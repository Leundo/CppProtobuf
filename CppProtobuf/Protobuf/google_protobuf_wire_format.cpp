// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include "google_protobuf_wire_format.hpp"

#include <stack>
#include <string>
#include <vector>

#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_strings_cord.hpp>
#include "google_protobuf_descriptor.hpp"
#include "google_protobuf_descriptor.pb.hpp"
#include "google_protobuf_dynamic_message.hpp"
#include "google_protobuf_io_coded_stream.hpp"
#include "google_protobuf_io_zero_copy_stream.hpp"
#include "google_protobuf_io_zero_copy_stream_impl.hpp"
#include "google_protobuf_map_field.hpp"
#include "google_protobuf_map_field_inl.hpp"
#include "google_protobuf_message.hpp"
#include "google_protobuf_message_lite.hpp"
#include "google_protobuf_parse_context.hpp"
#include "google_protobuf_unknown_field_set.hpp"


// Must be included last.

// MARK: - BEGIN google_protobuf_port_def.inc
// -*- c++ -*-
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// This file defines common macros that are used in protobuf.
//
// To hide these definitions from the outside world (and to prevent collisions
// if more than one version of protobuf is #included in the same project) you
// must follow this pattern when #including port_def.inc in a header file:
//
// #include "other_header.h"
// #include "message.h"
// // etc.
//
// #include "port_def.inc"  // MUST be last header included
//
// // Definitions for this header.
//
// #include "port_undef.inc"
//
// This is a textual header with no include guard, because we want to
// detect/prohibit anytime it is #included twice without a corresponding
// #undef.

#ifdef PROTOBUF_PORT_
#error "port_def.inc included multiple times"
#endif
#define PROTOBUF_PORT_
#include <CppAbseil/absl_base_attributes.hpp>
#include <CppAbseil/absl_base_config.hpp>

// The definitions in this file are intended to be portable across Clang,
// GCC, and MSVC. Function-like macros are usable without an #ifdef guard.
// Syntax macros (for example, attributes) are always defined, although
// they may be empty.
//
// Some definitions rely on the NDEBUG macro and/or (in MSVC) _DEBUG:
// - https://en.cppreference.com/w/c/error/assert
// - https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros#microsoft-specific-predefined-macros
//
// References for predefined macros:
// - Standard: https://en.cppreference.com/w/cpp/preprocessor/replace
// - Clang: https://clang.llvm.org/docs/LanguageExtensions.html
//          (see also GCC predefined macros)
// - GCC: https://gcc.gnu.org/onlinedocs/cpp/Predefined-Macros.html
// - MSVC: https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
// - Interactive (Clang/GCC only): https://www.compiler-explorer.com/z/hc6jKd3sj
//
// References for attributes (and extension attributes):
// - Standard: https://en.cppreference.com/w/cpp/language/attributes
// - Clang: https://clang.llvm.org/docs/AttributeReference.html
// - GCC: https://gcc.gnu.org/onlinedocs/gcc/Attribute-Syntax.html
//        (see Clang attribute docs as well)
//
// References for standard C++ language conformance (and minimum versions):
// - Clang: https://clang.llvm.org/cxx_status.html
// - GCC: https://gcc.gnu.org/projects/cxx-status.html
// - MSVC: https://docs.microsoft.com/en-us/cpp/overview/visual-cpp-language-conformance
//
// Historical release notes (which can help to determine minimum versions):
// - Clang: https://releases.llvm.org/
// - GCC: https://gcc.gnu.org/releases.html
// - MSVC: https://docs.microsoft.com/en-us/visualstudio/releases/2019/release-notes-history
//         https://docs.microsoft.com/en-us/visualstudio/releasenotes/vs2017-relnotes-history

// Portable fallback for Clang's __has_warning macro:
#ifndef __has_warning
#define __has_warning(x) 0
#define PROTOBUF_has_warning_DEFINED_
#endif

#ifdef ADDRESS_SANITIZER
#include <sanitizer/asan_interface.h>
#define PROTOBUF_POISON_MEMORY_REGION(p, n) ASAN_POISON_MEMORY_REGION(p, n)
#define PROTOBUF_UNPOISON_MEMORY_REGION(p, n) ASAN_UNPOISON_MEMORY_REGION(p, n)
#else  // ADDRESS_SANITIZER
#define PROTOBUF_POISON_MEMORY_REGION(p, n)
#define PROTOBUF_UNPOISON_MEMORY_REGION(p, n)
#endif  // ADDRESS_SANITIZER

// Portable PROTOBUF_BUILTIN_BSWAPxx definitions
// Code must check for availability, e.g.: `defined(PROTOBUF_BUILTIN_BSWAP32)`
#ifdef PROTOBUF_BUILTIN_BSWAP16
#error PROTOBUF_BUILTIN_BSWAP16 was previously defined
#endif
#ifdef PROTOBUF_BUILTIN_BSWAP32
#error PROTOBUF_BUILTIN_BSWAP32 was previously defined
#endif
#ifdef PROTOBUF_BUILTIN_BSWAP64
#error PROTOBUF_BUILTIN_BSWAP64 was previously defined
#endif
#if defined(__GNUC__) || ABSL_HAVE_BUILTIN(__builtin_bswap16)
#define PROTOBUF_BUILTIN_BSWAP16(x) __builtin_bswap16(x)
#endif
#if defined(__GNUC__) || ABSL_HAVE_BUILTIN(__builtin_bswap32)
#define PROTOBUF_BUILTIN_BSWAP32(x) __builtin_bswap32(x)
#endif
#if defined(__GNUC__) || ABSL_HAVE_BUILTIN(__builtin_bswap64)
#define PROTOBUF_BUILTIN_BSWAP64(x) __builtin_bswap64(x)
#endif

// Portable check for __builtin_mul_overflow.
#if ABSL_HAVE_BUILTIN(__builtin_mul_overflow)
#define PROTOBUF_HAS_BUILTIN_MUL_OVERFLOW 1
#endif

// Portable check for gcc-style atomic built-ins
#if ABSL_HAVE_BUILTIN(__atomic_load_n)
#define PROTOBUF_BUILTIN_ATOMIC 1
#endif

// Portable check for GCC minimum version:
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__GNUC__) && defined(__GNUC_MINOR__) \
    && defined(__GNUC_PATCHLEVEL__)
#  define PROTOBUF_GNUC_MIN(x, y) \
  (__GNUC__ > (x) || (__GNUC__ == (x) && __GNUC_MINOR__ >= (y)))
#else
#  define PROTOBUF_GNUC_MIN(x, y) 0
#endif

#if defined(__clang__) && defined(__clang_major__) && defined(__clang_minor__)
#define PROTOBUF_CLANG_MIN(x, y) \
  (__clang_major__ > (x) || (__clang_major__ == (x) && __clang_minor__ >= (y)))
#else
#define PROTOBUF_CLANG_MIN(x, y) 0
#endif

// Portable check for MSVC minimum version:
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
#if defined(_MSC_VER)
#define PROTOBUF_MSC_VER_MIN(x) (_MSC_VER >= x)
#else
#define PROTOBUF_MSC_VER_MIN(x) 0
#endif

// Portable check for minimum C++ language version:
// https://en.cppreference.com/w/cpp/preprocessor/replace
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
#if !defined(_MSVC_LANG)
#define PROTOBUF_CPLUSPLUS_MIN(x) (__cplusplus >= x)
#else
#define PROTOBUF_CPLUSPLUS_MIN(x) (_MSVC_LANG >= x)
#endif

// Check minimum Protobuf support defined at:
// https://github.com/google/oss-policies-info/blob/main/foundational-cxx-support-matrix.md
#ifdef __clang__
static_assert(PROTOBUF_CLANG_MIN(6, 0), "Protobuf only supports Clang 6.0 and newer.");
#elif defined(__GNUC__)
static_assert(PROTOBUF_GNUC_MIN(7, 3), "Protobuf only supports GCC 7.3 and newer.");
#elif defined(_MSVC_LANG)
static_assert(PROTOBUF_MSC_VER_MIN(1910), "Protobuf only supports MSVC 2017 and newer.");
#endif
static_assert(PROTOBUF_CPLUSPLUS_MIN(201402L), "Protobuf only supports C++14 and newer.");

// Check minimum Abseil version.
#if defined(ABSL_LTS_RELEASE_VERSION) && defined(ABSL_LTS_RELEASE_PATCH_LEVEL)
#define PROTOBUF_ABSL_MIN(x, y)      \
  (ABSL_LTS_RELEASE_VERSION > (x) || \
   (ABSL_LTS_RELEASE_VERSION == (x) && ABSL_LTS_RELEASE_PATCH_LEVEL >= (y)))
#else
// If we can't find an Abseil version, it's either not installed at all (which
// should trigger cmake errors), using a non-LTS release, or just isn't used in
// this file.
#define PROTOBUF_ABSL_MIN(x, y) 1
#endif

static_assert(PROTOBUF_ABSL_MIN(20230125, 3),
              "Protobuf only supports Abseil version 20230125.3 and newer.");

// Future versions of protobuf will include breaking changes to some APIs.
// This macro can be set to enable these API changes ahead of time, so that
// user code can be updated before upgrading versions of protobuf.

#ifdef PROTOBUF_FUTURE_BREAKING_CHANGES

// Used to remove the manipulation of cleared elements in RepeatedPtrField.
// Owner: mkruskal@
#define PROTOBUF_FUTURE_REMOVE_CLEARED_API 1

// Used for descriptor proto extension declarations.
// Owner: shaod@, gberg@
#define PROTOBUF_FUTURE_DESCRIPTOR_EXTENSION_DECL 1

// Used to remove `RepeatedPtrField::GetArena() const`.
// Owner: ezb@
#define PROTOBUF_FUTURE_REMOVE_CONST_REPEATEDFIELD_GETARENA_API 1

// Used to make ExtensionRange into a fully-fledged descriptor class.
// Owner: mkruskal@
#define PROTOBUF_FUTURE_EXTENSION_RANGE_CLASS 1

// Used to lock down wrong ctype usages in proto file.
// Owner: jieluo@
#define PROTOBUF_FUTURE_REMOVE_WRONG_CTYPE 1

#endif

#ifdef PROTOBUF_VERSION
#error PROTOBUF_VERSION was previously defined
#endif
#define PROTOBUF_VERSION 4025002

#ifdef PROTOBUF_MIN_HEADER_VERSION_FOR_PROTOC
#error PROTOBUF_MIN_HEADER_VERSION_FOR_PROTOC was previously defined
#endif
#define PROTOBUF_MIN_HEADER_VERSION_FOR_PROTOC 4025000

#ifdef PROTOBUF_MIN_PROTOC_VERSION
#error PROTOBUF_MIN_PROTOC_VERSION was previously defined
#endif
#define PROTOBUF_MIN_PROTOC_VERSION 4025000

#ifdef PROTOBUF_VERSION_SUFFIX
#error PROTOBUF_VERSION_SUFFIX was previously defined
#endif
#define PROTOBUF_VERSION_SUFFIX ""

#ifdef PROTOBUF_MINIMUM_EDITION
#error PROTOBUF_MINIMUM_EDITION was previously defined
#endif
#define PROTOBUF_MINIMUM_EDITION EDITION_PROTO2

#ifdef PROTOBUF_MAXIMUM_EDITION
#error PROTOBUF_MAXIMUM_EDITION was previously defined
#endif
#define PROTOBUF_MAXIMUM_EDITION EDITION_2023

#ifdef PROTOBUF_ALWAYS_INLINE
#error PROTOBUF_ALWAYS_INLINE was previously defined
#endif
// For functions we want to force inline.
#if defined(PROTOBUF_NO_INLINE)
# define PROTOBUF_ALWAYS_INLINE
#elif defined(__GNUC__)
# define PROTOBUF_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
# define PROTOBUF_ALWAYS_INLINE __forceinline
#else
# define PROTOBUF_ALWAYS_INLINE
#endif

#ifdef PROTOBUF_ALWAYS_INLINE_CALL
#error PROTOBUF_ALWAYS_INLINE_CALL was previously defined
#endif
// For functions we want to force inline from the caller, instead of in the
// declaration of the callee.
// This is useful for lambdas where it is not easy to specify ALWAYS_INLINE.
// Use like:
//   PROTOBUF_ALWAYS_INLINE_CALL res = SomeFunc(args...);
#if defined(__clang__) && !defined(PROTOBUF_NO_INLINE_CALL) && \
    ABSL_HAVE_CPP_ATTRIBUTE(clang::always_inline)
#define PROTOBUF_ALWAYS_INLINE_CALL [[clang::always_inline]]
#else
#define PROTOBUF_ALWAYS_INLINE_CALL
#endif

#ifdef PROTOBUF_NDEBUG_INLINE
#error PROTOBUF_NDEBUG_INLINE was previously defined
#endif
// Avoid excessive inlining in non-optimized builds. Without other optimizations
// the inlining is not going to provide benefits anyway and the huge resulting
// functions, especially in the proto-generated serialization functions, produce
// stack frames so large that many tests run into stack overflows (b/32192897).
#if defined(NDEBUG) || (defined(_MSC_VER) && !defined(_DEBUG))
# define PROTOBUF_NDEBUG_INLINE PROTOBUF_ALWAYS_INLINE
#else
# define PROTOBUF_NDEBUG_INLINE
#endif

// Note that PROTOBUF_NOINLINE is an attribute applied to functions, to prevent
// them from being inlined by the compiler. This is different from
// PROTOBUF_NO_INLINE, which is a user-supplied macro that disables forced
// inlining by PROTOBUF_(ALWAYS|NDEBUG)_INLINE.
//
// For forward-declared functions, make sure to apply PROTOBUF_NOINLINE to all
// the declarations, not just the definition, or else whole-program optimization
// may not honor the attribute.
#ifdef PROTOBUF_NOINLINE
#error PROTOBUF_NOINLINE was previously defined
#endif
#if defined(__GNUC__)
# define PROTOBUF_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
// Seems to have been around since at least Visual Studio 2005
# define PROTOBUF_NOINLINE __declspec(noinline)
#endif

#ifdef PROTOBUF_MUSTTAIL
#error PROTOBUF_MUSTTAIL was previously defined
#endif
#ifdef PROTOBUF_TAILCALL
#error PROTOBUF_TAILCALL was previously defined
#endif
#if ABSL_HAVE_CPP_ATTRIBUTE(clang::musttail) && !defined(__arm__) &&  \
    !defined(_ARCH_PPC) && !defined(__wasm__) &&                      \
    !(defined(_MSC_VER) && defined(_M_IX86)) && !defined(__i386__)
// Compilation fails on ARM32: b/195943306
// Compilation fails on powerpc64le: b/187985113
// Compilation fails on X86 Windows:
// https://github.com/llvm/llvm-project/issues/53271
#define PROTOBUF_MUSTTAIL [[clang::musttail]]
#define PROTOBUF_TAILCALL true
#else
#define PROTOBUF_MUSTTAIL
#define PROTOBUF_TAILCALL false
#endif

#ifdef PROTOBUF_EXCLUSIVE_LOCKS_REQUIRED
#error PROTOBUF_EXCLUSIVE_LOCKS_REQUIRED was previously defined
#endif
#if ABSL_HAVE_ATTRIBUTE(exclusive_locks_required)
#define PROTOBUF_EXCLUSIVE_LOCKS_REQUIRED(...) \
  __attribute__((exclusive_locks_required(__VA_ARGS__)))
#else
#define PROTOBUF_EXCLUSIVE_LOCKS_REQUIRED(...)
#endif

#ifdef PROTOBUF_NO_THREAD_SAFETY_ANALYSIS
#error PROTOBUF_NO_THREAD_SAFETY_ANALYSIS was previously defined
#endif
#if ABSL_HAVE_ATTRIBUTE(no_thread_safety_analysis)
#define PROTOBUF_NO_THREAD_SAFETY_ANALYSIS \
  __attribute__((no_thread_safety_analysis))
#else
#define PROTOBUF_NO_THREAD_SAFETY_ANALYSIS
#endif

#ifdef PROTOBUF_GUARDED_BY
#error PROTOBUF_GUARDED_BY was previously defined
#endif
#if ABSL_HAVE_ATTRIBUTE(guarded_by)
#define PROTOBUF_GUARDED_BY(x) __attribute__((guarded_by(x)))
#else
#define PROTOBUF_GUARDED_BY(x)
#endif

#ifdef PROTOBUF_LOCKS_EXCLUDED
#error PROTOBUF_LOCKS_EXCLUDED was previously defined
#endif
#if ABSL_HAVE_ATTRIBUTE(locks_excluded)
#define PROTOBUF_LOCKS_EXCLUDED(...) \
  __attribute__((locks_excluded(__VA_ARGS__)))
#else
#define PROTOBUF_LOCKS_EXCLUDED(...)
#endif

#ifdef PROTOBUF_COLD
#error PROTOBUF_COLD was previously defined
#endif
#if ABSL_HAVE_ATTRIBUTE(cold) || defined(__GNUC__)
# define PROTOBUF_COLD __attribute__((cold))
#else
# define PROTOBUF_COLD
#endif

#ifdef PROTOBUF_SECTION_VARIABLE
#error PROTOBUF_SECTION_VARIABLE was previously defined
#endif
#if (ABSL_HAVE_ATTRIBUTE(section) || defined(__GNUC__)) && defined(__ELF__)
// Place a variable in the given ELF section.
# define PROTOBUF_SECTION_VARIABLE(x) __attribute__((section(#x)))
#else
# define PROTOBUF_SECTION_VARIABLE(x)
#endif

#if defined(__clang__)
#define PROTOBUF_IGNORE_DEPRECATION_START                     \
  _Pragma("clang diagnostic push")                                  \
  _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define PROTOBUF_IGNORE_DEPRECATION_STOP                     \
  _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define PROTOBUF_IGNORE_DEPRECATION_START                     \
  _Pragma("GCC diagnostic push")                                  \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define PROTOBUF_IGNORE_DEPRECATION_STOP                     \
  _Pragma("GCC diagnostic pop")
#else
#define PROTOBUF_IGNORE_DEPRECATION_START
#define PROTOBUF_IGNORE_DEPRECATION_STOP
#endif

#ifdef PROTOBUF_RETURNS_NONNULL
#error PROTOBUF_RETURNS_NONNULL was previously defined
#endif
#if ABSL_HAVE_ATTRIBUTE(returns_nonnull) || defined(__GNUC__)
#define PROTOBUF_RETURNS_NONNULL __attribute__((returns_nonnull))
#else
#define PROTOBUF_RETURNS_NONNULL
#endif

#ifdef PROTOBUF_ATTRIBUTE_REINITIALIZES
#error PROTOBUF_ATTRIBUTE_REINITIALIZES was previously defined
#endif
#if ABSL_HAVE_CPP_ATTRIBUTE(clang::reinitializes)
#define PROTOBUF_ATTRIBUTE_REINITIALIZES [[clang::reinitializes]]
#else
#define PROTOBUF_ATTRIBUTE_REINITIALIZES
#endif

// The minimum library version which works with the current version of the
// headers.
#define GOOGLE_PROTOBUF_MIN_LIBRARY_VERSION 4025000

#ifdef PROTOBUF_RTTI
#error PROTOBUF_RTTI was previously defined
#endif
#if defined(GOOGLE_PROTOBUF_NO_RTTI) && GOOGLE_PROTOBUF_NO_RTTI
// A user-provided definition GOOGLE_PROTOBUF_NO_RTTI=1 disables RTTI.
#define PROTOBUF_RTTI 0
#elif defined(__cpp_rtti)
// https://en.cppreference.com/w/cpp/feature_test
#define PROTOBUF_RTTI 1
#elif ABSL_HAVE_FEATURE(cxx_rtti)
// https://clang.llvm.org/docs/LanguageExtensions.html#c-rtti
#define PROTOBUF_RTTI 1
#elif defined(__GXX_RTTI)
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#define PROTOBUF_RTTI 1
#elif defined(_CPPRTTI)
// https://docs.microsoft.com/en-us/cpp/build/reference/gr-enable-run-time-type-information
#define PROTOBUF_RTTI 1
#else
#define PROTOBUF_RTTI 0
#endif

// Returns the offset of the given field within the given aggregate type.
// This is equivalent to the ANSI C offsetof() macro.  However, according
// to the C++ standard, offsetof() only works on POD types, and GCC
// enforces this requirement with a warning.  In practice, this rule is
// unnecessarily strict; there is probably no compiler or platform on
// which the offsets of the direct fields of a class are non-constant.
// Fields inherited from superclasses *can* have non-constant offsets,
// but that's not what this macro will be used for.
#ifdef PROTOBUF_FIELD_OFFSET
#error PROTOBUF_FIELD_OFFSET was previously defined
#endif
#if defined(__clang__)
// For Clang we use __builtin_offsetof() and suppress the warning,
// to avoid Control Flow Integrity and UBSan vptr sanitizers from
// crashing while trying to validate the invalid reinterpret_casts.
#define PROTOBUF_FIELD_OFFSET(TYPE, FIELD)                   \
  _Pragma("clang diagnostic push")                           \
  _Pragma("clang diagnostic ignored \"-Winvalid-offsetof\"") \
  __builtin_offsetof(TYPE, FIELD)                            \
  _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define PROTOBUF_FIELD_OFFSET(TYPE, FIELD) __builtin_offsetof(TYPE, FIELD)
#else  // defined(__clang__)
// Note that we calculate relative to the pointer value 16 here since if we
// just use zero, GCC complains about dereferencing a NULL pointer.  We
// choose 16 rather than some other number just in case the compiler would
// be confused by an unaligned pointer.
#define PROTOBUF_FIELD_OFFSET(TYPE, FIELD)                                \
  static_cast< ::uint32_t>(reinterpret_cast<const char*>(                   \
                             &reinterpret_cast<const TYPE*>(16)->FIELD) - \
                         reinterpret_cast<const char*>(16))
#endif

#ifdef PROTOBUF_EXPORT
#error PROTOBUF_EXPORT was previously defined
#endif

#if defined(PROTOBUF_USE_DLLS) && defined(_MSC_VER)
# if defined(LIBPROTOBUF_EXPORTS)
#  define PROTOBUF_EXPORT __declspec(dllexport)
#  define PROTOBUF_EXPORT_TEMPLATE_DECLARE
#  define PROTOBUF_EXPORT_TEMPLATE_DEFINE __declspec(dllexport)
# else
#  define PROTOBUF_EXPORT __declspec(dllimport)
#  define PROTOBUF_EXPORT_TEMPLATE_DECLARE
#  define PROTOBUF_EXPORT_TEMPLATE_DEFINE __declspec(dllimport)
# endif  // defined(LIBPROTOBUF_EXPORTS)
#elif defined(PROTOBUF_USE_DLLS) && defined(LIBPROTOBUF_EXPORTS)
# define PROTOBUF_EXPORT __attribute__((visibility("default")))
# define PROTOBUF_EXPORT_TEMPLATE_DECLARE __attribute__((visibility("default")))
# define PROTOBUF_EXPORT_TEMPLATE_DEFINE
#else
# define PROTOBUF_EXPORT
# define PROTOBUF_EXPORT_TEMPLATE_DECLARE
# define PROTOBUF_EXPORT_TEMPLATE_DEFINE
#endif

#ifdef PROTOC_EXPORT
#error PROTOC_EXPORT was previously defined
#endif

#if defined(PROTOBUF_USE_DLLS) && defined(_MSC_VER)
# if defined(LIBPROTOC_EXPORTS)
#  define PROTOC_EXPORT __declspec(dllexport)
# else
#  define PROTOC_EXPORT __declspec(dllimport)
# endif  // defined(LIBPROTOC_EXPORTS)
#elif defined(PROTOBUF_USE_DLLS) && defined(LIBPROTOC_EXPORTS)
# define PROTOC_EXPORT __attribute__((visibility("default")))
#else
# define PROTOC_EXPORT
#endif

#if defined(PROTOBUF_USE_DLLS) && defined(_MSC_VER)
# if defined(LIBPROTOBUF_TEST_EXPORTS)
#  define PROTOBUF_TEST_EXPORTS __declspec(dllexport)
# else
#  define PROTOBUF_TEST_EXPORTS __declspec(dllimport)
# endif  // defined(LIBPROTOBUF_TEST_EXPORTS)
#elif defined(PROTOBUF_USE_DLLS) && defined(LIBPROTOBUF_TEST_EXPORTS)
# define PROTOBUF_TEST_EXPORTS __attribute__((visibility("default")))
#else
# define PROTOBUF_TEST_EXPORTS
#endif

#if defined(PROTOBUF_PREDICT_TRUE) || defined(PROTOBUF_PREDICT_FALSE)
#error PROTOBUF_PREDICT_(TRUE|FALSE) was previously defined
#endif
#if defined(__GNUC__)
# define PROTOBUF_PREDICT_TRUE(x) (__builtin_expect(false || (x), true))
# define PROTOBUF_PREDICT_FALSE(x) (__builtin_expect(false || (x), false))
#else
# define PROTOBUF_PREDICT_TRUE(x) (x)
# define PROTOBUF_PREDICT_FALSE(x) (x)
#endif

#ifdef PROTOBUF_NODISCARD
#error PROTOBUF_NODISCARD was previously defined
#endif
#if ABSL_HAVE_CPP_ATTRIBUTE(nodiscard) && PROTOBUF_CPLUSPLUS_MIN(201703L)
#define PROTOBUF_NODISCARD [[nodiscard]]
#elif ABSL_HAVE_ATTRIBUTE(warn_unused_result) || defined(__GNUC__)
#define PROTOBUF_NODISCARD __attribute__((warn_unused_result))
#else
#define PROTOBUF_NODISCARD
#endif

#ifdef PROTOBUF_RESTRICT
#error PROTOBUF_RESTRICT was previously defined
#endif
#if defined( __clang__) || defined(__GNUC__)
#define PROTOBUF_RESTRICT __restrict
#else
#define PROTOBUF_RESTRICT
#endif

#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
#error PROTOBUF_FORCE_COPY_IN_RELEASE was previously defined
#endif

#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
#error PROTOBUF_FORCE_COPY_IN_SWAP was previously defined
#endif

#ifdef PROTOBUF_FORCE_COPY_IN_MOVE
#error PROTOBUF_FORCE_COPY_IN_MOVE was previously defined
#endif

#ifdef PROTOBUF_FORCE_RESET_IN_CLEAR
#error PROTOBUF_FORCE_RESET_IN_CLEAR was previously defined
#endif

#ifdef PROTOBUF_FUZZ_MESSAGE_SPACE_USED_LONG
#error PROTOBUF_FUZZ_MESSAGE_SPACE_USED_LONG was previously defined
#endif

// Force copy the default string to a string field so that non-optimized builds
// have harder-to-rely-on address stability.
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
#error PROTOBUF_FORCE_COPY_DEFAULT_STRING was previously defined
#endif

// Set (and leave set) PROTOBUF_EXPLICIT_CONSTRUCTORS
// This flag changes the internal implementation class and its uses from using
// aggregate initialization to using explicit constructor based initialization.
// TODO: remove this flag and make the code permanent.
#ifndef PROTOBUF_EXPLICIT_CONSTRUCTORS
#define PROTOBUF_EXPLICIT_CONSTRUCTORS
#endif

#ifdef PROTOBUF_FORCE_ALLOCATION_ON_CONSTRUCTION
#error PROTOBUF_FORCE_ALLOCATION_ON_CONSTRUCTION was previously defined
#endif

#ifdef PROTOBUF_FALLTHROUGH_INTENDED
#error PROTOBUF_FALLTHROUGH_INTENDED was previously defined
#endif
#if ABSL_HAVE_CPP_ATTRIBUTE(fallthrough)
#define PROTOBUF_FALLTHROUGH_INTENDED [[fallthrough]]
#elif ABSL_HAVE_FEATURE(cxx_attributes) && \
    __has_warning("-Wimplicit-fallthrough")
#define PROTOBUF_FALLTHROUGH_INTENDED [[clang::fallthrough]]
#elif defined(__GNUC__)
#define PROTOBUF_FALLTHROUGH_INTENDED [[gnu::fallthrough]]
#else
#define PROTOBUF_FALLTHROUGH_INTENDED
#endif

// Specify memory alignment for structs, classes, etc.
// Use like:
//   class PROTOBUF_ALIGNAS(16) MyClass { ... }
//   PROTOBUF_ALIGNAS(16) int array[4];
//
// In most places you can use the C++11 keyword "alignas", which is preferred.
//
// But compilers have trouble mixing __attribute__((...)) syntax with
// alignas(...) syntax.
//
// Doesn't work in clang or gcc:
//   struct alignas(16) __attribute__((packed)) S { char c; };
// Works in clang but not gcc:
//   struct __attribute__((packed)) alignas(16) S2 { char c; };
// Works in clang and gcc:
//   struct alignas(16) S3 { char c; } __attribute__((packed));
//
// There are also some attributes that must be specified *before* a class
// definition: visibility (used for exporting functions/classes) is one of
// these attributes. This means that it is not possible to use alignas() with a
// class that is marked as exported.
#ifdef PROTOBUF_ALIGNAS
#error PROTOBUF_ALIGNAS was previously defined
#endif
#if defined(_MSC_VER)
#define PROTOBUF_ALIGNAS(byte_alignment) __declspec(align(byte_alignment))
#elif defined(__GNUC__)
#define PROTOBUF_ALIGNAS(byte_alignment) \
  __attribute__((aligned(byte_alignment)))
#else
#define PROTOBUF_ALIGNAS(byte_alignment) alignas(byte_alignment)
#endif

#ifdef PROTOBUF_FINAL
#error PROTOBUF_FINAL was previously defined
#endif
#define PROTOBUF_FINAL final

#ifdef PROTOBUF_THREAD_LOCAL
#error PROTOBUF_THREAD_LOCAL was previously defined
#endif
#if defined(_MSC_VER)
#define PROTOBUF_THREAD_LOCAL __declspec(thread)
#else
#define PROTOBUF_THREAD_LOCAL __thread
#endif

// TODO: cleanup PROTOBUF_LITTLE_ENDIAN in various 3p forks.
#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define PROTOBUF_LITTLE_ENDIAN 1
#ifdef PROTOBUF_BIG_ENDIAN
#error Conflicting PROTOBUF_BIG_ENDIAN was previously defined
#endif
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define PROTOBUF_BIG_ENDIAN 1
#elif defined(_WIN32) || defined(__x86_64__) || defined(__aarch64__)
#define PROTOBUF_LITTLE_ENDIAN 1
#else
#error "endian detection failed for current compiler"
#endif

#ifdef PROTOBUF_CONSTINIT
#error PROTOBUF_CONSTINIT was previously defined
#endif

// Lexan sets both MSV_VER and clang, so handle it with the clang path.
#if defined(_MSC_VER) && !defined(__clang__)
// MSVC 17 currently seems to raise an error about constant-initialized pointers.
# if PROTOBUF_MSC_VER_MIN(1930)
#  define PROTOBUF_CONSTINIT
#  define PROTOBUF_CONSTEXPR constexpr
# endif
#elif defined(__GNUC__) && !defined(__clang__)
// GCC doesn't support constinit aggregate initialization of absl::Cord.
# if PROTOBUF_GNUC_MIN(12, 2)
#  define PROTOBUF_CONSTINIT
#  define PROTOBUF_CONSTEXPR constexpr
# endif
#else
# if defined(__cpp_constinit) && !defined(__CYGWIN__)
#  define PROTOBUF_CONSTINIT constinit
#  define PROTOBUF_CONSTEXPR constexpr
#  define PROTOBUF_CONSTINIT_DEFAULT_INSTANCES
// Some older Clang versions incorrectly raise an error about
// constant-initializing weak default instance pointers. Versions 12.0 and
// higher seem to work, except that XCode 12.5.1 shows the error even though it
// uses Clang 12.0.5.
#elif !defined(__CYGWIN__) && !defined(__MINGW32__) &&                 \
    ABSL_HAVE_CPP_ATTRIBUTE(clang::require_constant_initialization) && \
    ((defined(__APPLE__) && PROTOBUF_CLANG_MIN(13, 0)) ||              \
     (!defined(__APPLE__) && PROTOBUF_CLANG_MIN(12, 0)))
#  define PROTOBUF_CONSTINIT [[clang::require_constant_initialization]]
#  define PROTOBUF_CONSTEXPR constexpr
#  define PROTOBUF_CONSTINIT_DEFAULT_INSTANCES
# endif
#endif

#ifndef PROTOBUF_CONSTINIT
#define PROTOBUF_CONSTINIT
#define PROTOBUF_CONSTEXPR
#endif

// Some globals with an empty non-trivial destructor are annotated with
// no_destroy for performance reasons. It reduces the cost of these globals in
// non-opt mode and under sanitizers.
#ifdef PROTOBUF_ATTRIBUTE_NO_DESTROY
#error PROTOBUF_ATTRIBUTE_NO_DESTROY was previously defined
#endif
#if ABSL_HAVE_CPP_ATTRIBUTE(clang::no_destroy)
#define PROTOBUF_ATTRIBUTE_NO_DESTROY [[clang::no_destroy]]
#else
#define PROTOBUF_ATTRIBUTE_NO_DESTROY
#endif

// Force clang to always emit complete debug info for a type.
// Clang uses constructor homing to determine when to emit debug info for a
// type. If the constructor of a type is never used, which can happen in some
// cases where member variables are constructed in place for optimization
// purposes (see b/208803175 for an example), the type will have incomplete
// debug info unless this attribute is used.
#ifdef PROTOBUF_ATTRIBUTE_STANDALONE_DEBUG
#error PROTOBUF_ATTRIBUTE_STANDALONE_DEBUG was previously defined
#endif
#if ABSL_HAVE_CPP_ATTRIBUTE(clang::standalone_debug)
#define PROTOBUF_ATTRIBUTE_STANDALONE_DEBUG [[clang::standalone_debug]]
#else
#define PROTOBUF_ATTRIBUTE_STANDALONE_DEBUG
#endif

// Protobuf extensions and reflection require registration of the protos linked
// in the binary. Not until everything is registered does the runtime have a
// complete view on all protos. When code is using reflection or extensions
// in between registration calls this can lead to surprising behavior. By
// having the registration run first we mitigate this scenario.
// Highest priority is 101. We use 102 for registration, to allow code that
// really wants to higher priority to still beat us. Some initialization happens
// at higher priority, though, since it is needed before registration.
#ifdef PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
#error PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 was previously defined
#endif
#ifdef PROTOBUF_ATTRIBUTE_INIT_PRIORITY2
#error PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 was previously defined
#endif
#if defined(__GNUC__) && (!defined(__APPLE__) || defined(__clang__)) && \
    !((defined(sun) || defined(__sun)) &&                                     \
      (defined(__SVR4) || defined(__svr4__)))
#define PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 __attribute__((init_priority((101))))
#define PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 __attribute__((init_priority((102))))
#else
#define PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
#define PROTOBUF_ATTRIBUTE_INIT_PRIORITY2
#endif

#ifdef PROTOBUF_PRAGMA_INIT_SEG
#error PROTOBUF_PRAGMA_INIT_SEG was previously defined
#endif
#ifdef _MSC_VER
#define PROTOBUF_PRAGMA_INIT_SEG __pragma(init_seg(lib))
#else
#define PROTOBUF_PRAGMA_INIT_SEG
#endif

#ifdef PROTOBUF_ATTRIBUTE_WEAK
#error PROTOBUF_ATTRIBUTE_WEAK was previously defined
#endif
#if ABSL_HAVE_ATTRIBUTE(weak) && !defined(__APPLE__) && \
    (!defined(_WIN32) || __clang_major__ < 9) && !defined(__MINGW32__)
#define PROTOBUF_ATTRIBUTE_WEAK __attribute__((weak))
#define PROTOBUF_HAVE_ATTRIBUTE_WEAK 1
#else
#define PROTOBUF_ATTRIBUTE_WEAK
#define PROTOBUF_HAVE_ATTRIBUTE_WEAK 0
#endif

// Macros to detect sanitizers.
#ifdef PROTOBUF_ASAN
#error PROTOBUF_ASAN was previously defined
#endif
#ifdef PROTOBUF_MSAN
#error PROTOBUF_MSAN was previously defined
#endif
#ifdef PROTOBUF_TSAN
#error PROTOBUF_TSAN was previously defined
#endif
#if defined(__clang__)
#if ABSL_HAVE_FEATURE(address_sanitizer)
#    define PROTOBUF_ASAN 1
#  endif
#if ABSL_HAVE_FEATURE(thread_sanitizer)
#    define PROTOBUF_TSAN 1
#  endif
#if ABSL_HAVE_FEATURE(memory_sanitizer)
#    define PROTOBUF_MSAN 1
#  endif
#elif defined(__GNUC__)
// Double-guard is needed for -Wundef:
#  ifdef __SANITIZE_ADDRESS__
#  if    __SANITIZE_ADDRESS__
#    define PROTOBUF_ASAN 1
#  endif
#  endif
#  ifdef __SANITIZE_THREAD__
#  if    __SANITIZE_THREAD__
#    define PROTOBUF_TSAN 1
#  endif
#  endif
#endif

#ifdef PROTOBUF_TSAN_READ
#error PROTOBUF_TSAN_READ was previously defined
#endif
#ifdef PROTOBUF_TSAN_WRITE
#error PROTOBUF_TSAN_WRITE was previously defined
#endif
#ifdef PROTOBUF_TSAN
// TODO: it would be preferable to use __tsan_external_read/
// __tsan_external_write, but they can cause dlopen issues.
#define PROTOBUF_TSAN_READ(addr)                                      \
  do {                                                                \
    char protobuf_tsan_dummy = *reinterpret_cast<const char *>(addr); \
    asm volatile("" : "+r"(protobuf_tsan_dummy));                     \
  } while (0)
// We currently use a dedicated member for TSan checking so the value of this
// member is not important. We can unconditionally write to it without affecting
// correctness of the rest of the class.
#define PROTOBUF_TSAN_WRITE(addr)        \
  do {                                   \
    *reinterpret_cast<char *>(addr) = 0; \
  } while (0)
#else  // PROTOBUF_TSAN
#define PROTOBUF_TSAN_READ(addr) (void)0
#define PROTOBUF_TSAN_WRITE(addr) (void)0
#endif  // PROTOBUF_TSAN

#ifdef PROTOBUF_TSAN
#define PROTOBUF_TSAN_DECLARE_MEMBER ::uint32_t _tsan_detect_race = 0;
#else
#define PROTOBUF_TSAN_DECLARE_MEMBER
#endif

#ifdef PROTOBUF_USE_TABLE_PARSER_ON_REFLECTION
#error PROTOBUF_USE_TABLE_PARSER_ON_REFLECTION was previously defined
#endif
#if !defined(PROTOBUF_TEMPORARY_DISABLE_TABLE_PARSER_ON_REFLECTION)
#define PROTOBUF_USE_TABLE_PARSER_ON_REFLECTION 1
#endif  // PROTOBUF_ENABLE_FORCE_ALLOCATION_ON_CONSTRUCTION

// Note that this is performance sensitive: changing the parameters will change
// the registers used by the ABI calling convention, which subsequently affects
// register selection logic inside the function.
// Arguments `msg`, `ptr` and `ctx` are the 1st/2nd/3rd argument to match the
// signature of ParseLoop.
//
// Note for x86_64: `data` must be the third or fourth argument for performance
// reasons. In order to efficiently read the second byte of `data` we need it to
// be passed in RDX or RCX.
#define PROTOBUF_TC_PARAM_DECL                 \
  ::google::protobuf::MessageLite *msg, const char *ptr, \
      ::google::protobuf::internal::ParseContext *ctx,   \
      ::google::protobuf::internal::TcFieldData data,    \
      const ::google::protobuf::internal::TcParseTableBase *table, uint64_t hasbits
// PROTOBUF_TC_PARAM_PASS passes values to match PROTOBUF_TC_PARAM_DECL.
#define PROTOBUF_TC_PARAM_PASS msg, ptr, ctx, data, table, hasbits

// PROTOBUF_TC_PARAM_NO_DATA_DECL and PROTOBUF_TC_PARAM_NO_DATA_PASS provide the
// exact same ABI as above, except that they don't name or pass the `data`
// argument. Specific functions such as `Error() and `ToTagDispatch()` don't
// use the `data` argument. By not passing `data` down the call stack, we free
// up the register holding that value, which may matter in highly optimized
// functions such as varint parsing.
#define PROTOBUF_TC_PARAM_NO_DATA_DECL                                        \
  ::google::protobuf::MessageLite *msg, const char *ptr,                                \
      ::google::protobuf::internal::ParseContext *ctx, ::google::protobuf::internal::TcFieldData, \
      const ::google::protobuf::internal::TcParseTableBase *table, uint64_t hasbits
#define PROTOBUF_TC_PARAM_NO_DATA_PASS \
  msg, ptr, ctx, ::google::protobuf::internal::TcFieldData::DefaultInit(), table, hasbits

#ifdef PROTOBUF_UNUSED
#error PROTOBUF_UNUSED was previously defined
#endif
#if ABSL_HAVE_CPP_ATTRIBUTE(maybe_unused) || \
    (PROTOBUF_MSC_VER_MIN(1911) && PROTOBUF_CPLUSPLUS_MIN(201703L))
#define PROTOBUF_UNUSED [[maybe_unused]]
#elif ABSL_HAVE_ATTRIBUTE(unused) || defined(__GNUC__)
#define PROTOBUF_UNUSED __attribute__((__unused__))
#else
#define PROTOBUF_UNUSED
#endif

// For the toolchains that support it, evalutes to `true` if the input is
// determined to be a compile time constant after optimizations have happened
// (like inlining and constant propagation). A `false` result means that the
// compiler could not determine it is constant (it might still be but couldn't
// figure it out).
#ifdef PROTOBUF_BUILTIN_CONSTANT_P
#error PROTOBUF_BUILTIN_CONSTANT_P was previously defined
#endif
#if ABSL_HAVE_BUILTIN(__builtin_constant_p)
#define PROTOBUF_BUILTIN_CONSTANT_P(x) __builtin_constant_p(x)
#else
#define PROTOBUF_BUILTIN_CONSTANT_P(x) false
#endif

// ThreadSafeArenaz is turned off completely in opensource builds.

// autoheader defines this in some circumstances
#ifdef PACKAGE
#define PROTOBUF_DID_UNDEF_PACKAGE
#pragma push_macro("PACKAGE")
#undef PACKAGE
#endif

// a few common headers define this
#ifdef PACKED
#define PROTOBUF_DID_UNDEF_PACKED
#pragma push_macro("PACKED")
#undef PACKED
#endif

// This is a macro on Windows, macOS, and some variants of GCC.
#ifdef DOMAIN
#define PROTOBUF_DID_UNDEF_DOMAIN
#pragma push_macro("DOMAIN")
#undef DOMAIN
#endif

// linux is a legacy MACRO defined in most popular C++ standards.
#ifdef linux
#pragma push_macro("linux")
#undef linux
#define PROTOBUF_DID_UNDEF_LINUX
#endif

// Windows declares several inconvenient macro names.  We #undef them and then
// restore them in port_undef.inc.
#ifdef _WIN32
#pragma push_macro("CompareString")
#undef CompareString
#pragma push_macro("CREATE_NEW")
#undef CREATE_NEW
#pragma push_macro("DELETE")
#undef DELETE
#pragma push_macro("DOUBLE_CLICK")
#undef DOUBLE_CLICK
#pragma push_macro("ERROR")
#undef ERROR
#pragma push_macro("ERROR_BUSY")
#undef ERROR_BUSY
#pragma push_macro("ERROR_INSTALL_FAILED")
#undef ERROR_INSTALL_FAILED
#pragma push_macro("ERROR_NOT_FOUND")
#undef ERROR_NOT_FOUND
#pragma push_macro("GetClassName")
#undef GetClassName
#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime
#pragma push_macro("GetMessage")
#undef GetMessage
#pragma push_macro("GetObject")
#undef GetObject
#pragma push_macro("IGNORE")
#undef IGNORE
#pragma push_macro("IN")
#undef IN
#pragma push_macro("INPUT_KEYBOARD")
#undef INPUT_KEYBOARD
#pragma push_macro("NO_ERROR")
#undef NO_ERROR
#pragma push_macro("OUT")
#undef OUT
#pragma push_macro("OPTIONAL")
#undef OPTIONAL
#pragma push_macro("min")
#undef min
#pragma push_macro("max")
#undef max
#pragma push_macro("NEAR")
#undef NEAR
#pragma push_macro("NO_DATA")
#undef NO_DATA
#pragma push_macro("REASON_UNKNOWN")
#undef REASON_UNKNOWN
#pragma push_macro("SERVICE_DISABLED")
#undef SERVICE_DISABLED
#pragma push_macro("SEVERITY_ERROR")
#undef SEVERITY_ERROR
#pragma push_macro("STATUS_PENDING")
#undef STATUS_PENDING
#pragma push_macro("STRICT")
#undef STRICT
#pragma push_macro("timezone")
#undef timezone
#pragma push_macro("TRUE")
#undef TRUE
#pragma push_macro("FALSE")
#undef FALSE
#endif  // _WIN32

#ifdef __APPLE__
// Inconvenient macro names from /usr/include/mach/boolean.h in some macOS SDKs.
#pragma push_macro("TRUE")
#undef TRUE
#pragma push_macro("FALSE")
#undef FALSE
// Inconvenient macro names from usr/include/sys/syslimits.h in some macOS SDKs.
#pragma push_macro("UID_MAX")
#undef UID_MAX
#pragma push_macro("GID_MAX")
#undef GID_MAX
// TYPE_BOOL is defined in the MacOS's ConditionalMacros.h.
#pragma push_macro("TYPE_BOOL")
#undef TYPE_BOOL
#endif  // __APPLE__

#ifdef __FreeBSD__
// Inconvenient macro names from /usr/include/sys/param.h.
#pragma push_macro("TRUE")
#undef TRUE
#pragma push_macro("FALSE")
#undef FALSE
// Inconvenient macro names from /usr/include/sys/limits.h.
#pragma push_macro("UID_MAX")
#undef UID_MAX
#pragma push_macro("GID_MAX")
#undef GID_MAX
#endif  // __FreeBSD__

#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
// Don't let Objective-C Macros interfere with proto identifiers with the same
// name.
#pragma push_macro("DEBUG")
#undef DEBUG
#endif // defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)

// Protobuf does not support building with a number of warnings that are noisy
// (and of variable quality across compiler versions) or impossible to implement
// effectively but which people turn on anyways.
#ifdef __clang__
#pragma clang diagnostic push
// -Wshorten-64-to-32 is a typical pain where we diff pointers.
//   char* p = strchr(s, '\n');
//   return p ? p - s : -1;
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
// Turn on -Wdeprecated-enum-enum-conversion. This deprecation comes in C++20
// via http://wg21.link/p1120r0.
#pragma clang diagnostic error "-Wdeprecated-enum-enum-conversion"
// This error has been generally flaky, but we need to disable it specifically
// to fix https://github.com/protocolbuffers/protobuf/issues/12313
#pragma clang diagnostic ignored "-Wunused-parameter"
// -Wshadow means that declaration shadows a variable.
// for (int i = 0; i < file_->public_dependency_count(); ++i)
//   for (int i = 0; i < public_dep->message_type_count(); ++i)
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wextra-semi"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
// Some of the warnings below only exist in some GCC versions; those version
// ranges are poorly documented.
#pragma GCC diagnostic ignored "-Wpragmas"
// GCC does not allow disabling diagnostics within an expression:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60875, so we disable this one
// globally even though it's only used for PROTOBUF_FIELD_OFFSET.
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
// Some versions of GCC seem to think that
//  [this] { Foo(); }
// leaves `this` unused, even though `Foo();` is a member function of the
// captured `this`.
// https://bugzilla.mozilla.org/show_bug.cgi?id=1373625
#pragma GCC diagnostic ignored "-Wunused-lambda-capture"
// -Wsign-conversion causes a lot of warnings on mostly code like:
//   int index = ...
//   int value = vec[index];
#pragma GCC diagnostic ignored "-Wsign-conversion"
// This error has been generally flaky, but we need to disable it specifically
// to fix https://github.com/protocolbuffers/protobuf/issues/12313
#pragma GCC diagnostic ignored "-Wunused-parameter"
// -Wshadow means that declaration shadows a variable.
// for (int i = 0; i < file_->public_dependency_count(); ++i)
//   for (int i = 0; i < public_dep->message_type_count(); ++i)
#pragma GCC diagnostic ignored "-Wshadow"
#ifndef __clang__
// This causes spurious warnings in GCC 13.
#pragma GCC diagnostic ignored "-Wstringop-overflow"
// This causes spurious warnings in GCC 13.
#pragma GCC diagnostic ignored "-Wself-move"
// Ignore warning: extra ';' [-Wpedantic]
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#if __GNUC__ == 12 && __GNUC_MINOR__ < 4
// Wrong warning emitted when assigning a single char c-string to a std::string
// in c++20 mode and optimization on.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105329
// Planned to be fixed by 12.3 but widen window to 12.4.
#pragma GCC diagnostic ignored "-Wrestrict"
#endif
#endif  // __GNUC__

// Silence some MSVC warnings in all our code.
#ifdef _MSC_VER
#pragma warning(push)
// For non-trivial unions
#pragma warning(disable : 4582)
#pragma warning(disable : 4583)
// For init_seg(lib)
#pragma warning(disable : 4073)
// To silence the fact that we will pop this push from another file
#pragma warning(disable : 5031)
// Conditional expression is constant
#pragma warning(disable: 4127)
// decimal digit terminates octal escape sequence
#pragma warning(disable: 4125)
#endif

#if PROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII
#define PROTOBUF_DEBUG true
#else
#define PROTOBUF_DEBUG false
#endif

#define PROTO2_IS_OSS true

#ifdef PROTOBUF_NO_THREADLOCAL
#error PROTOBUF_NO_THREADLOCAL was previously defined
#endif

// port_def.inc may be included in very large compilation targets, so we need to
// minimize adding symbol and source file information here. For this reason we
// implement our own simple `protobuf_assumption_failed()` function for
// asserting PROTOBUF_ASSUME predicates in debug builds.
namespace google {
namespace protobuf {
namespace internal {
PROTOBUF_EXPORT void protobuf_assumption_failed(const char *pred,
                                                const char *file, int line);
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// PROTOBUF_ASSUME(pred) tells the compiler that it can assume pred is true.
// To be safe, we also validate the assumption in debug builds, printing an
// assert style "Assumption failed: ..." message and aborting the program if
// the predicate is false. The macro does not do anything useful if the
// compiler does not support __builtin_assume.
#ifdef PROTOBUF_ASSUME
#error PROTOBUF_ASSUME was previously defined
#endif
#if ABSL_HAVE_BUILTIN(__builtin_assume)
#ifdef NDEBUG
#define PROTOBUF_ASSUME(pred) __builtin_assume(pred)
#else  // NDEBUG
#define PROTOBUF_ASSUME(pred)                                                  \
  if (!(pred)) {                                                               \
    ::google::protobuf::internal::protobuf_assumption_failed(#pred, __FILE__, __LINE__); \
  }                                                                            \
  __builtin_assume(pred)
#endif  // NDEBUG
#else   // has_builtin(__builtin_assume)
#ifndef NDEBUG
#define PROTOBUF_ASSUME(pred)                                                  \
  if (!(pred)) {                                                               \
    ::google::protobuf::internal::protobuf_assumption_failed(#pred, __FILE__, __LINE__); \
  }
#else  // !NDEBUG
#define PROTOBUF_ASSUME(pred)
#endif  // !NDEBUG
#endif  // has_builtin(__builtin_assume)

// We don't want code outside port_def doing complex testing, so
// remove our portable condition test macros to nudge folks away from
// using it themselves.
#ifdef PROTOBUF_has_warning_DEFINED_
#  undef __has_warning
#  undef PROTOBUF_has_warning_DEFINED_
#endif


// MARK: - END google_protobuf_port_def.inc


const size_t kMapEntryTagByteSize = 2;

namespace google {
namespace protobuf {
namespace internal {

// Forward declare static functions
static size_t MapValueRefDataOnlyByteSize(const FieldDescriptor* field,
                                          const MapValueConstRef& value);

// ===================================================================

bool UnknownFieldSetFieldSkipper::SkipField(io::CodedInputStream* input,
                                            uint32_t tag) {
  return WireFormat::SkipField(input, tag, unknown_fields_);
}

bool UnknownFieldSetFieldSkipper::SkipMessage(io::CodedInputStream* input) {
  return WireFormat::SkipMessage(input, unknown_fields_);
}

void UnknownFieldSetFieldSkipper::SkipUnknownEnum(int field_number, int value) {
  unknown_fields_->AddVarint(field_number, value);
}

bool WireFormat::SkipField(io::CodedInputStream* input, uint32_t tag,
                           UnknownFieldSet* unknown_fields) {
  int number = WireFormatLite::GetTagFieldNumber(tag);
  // Field number 0 is illegal.
  if (number == 0) return false;

  switch (WireFormatLite::GetTagWireType(tag)) {
    case WireFormatLite::WIRETYPE_VARINT: {
      uint64_t value;
      if (!input->ReadVarint64(&value)) return false;
      if (unknown_fields != nullptr) unknown_fields->AddVarint(number, value);
      return true;
    }
    case WireFormatLite::WIRETYPE_FIXED64: {
      uint64_t value;
      if (!input->ReadLittleEndian64(&value)) return false;
      if (unknown_fields != nullptr) unknown_fields->AddFixed64(number, value);
      return true;
    }
    case WireFormatLite::WIRETYPE_LENGTH_DELIMITED: {
      uint32_t length;
      if (!input->ReadVarint32(&length)) return false;
      if (unknown_fields == nullptr) {
        if (!input->Skip(length)) return false;
      } else {
        if (!input->ReadString(unknown_fields->AddLengthDelimited(number),
                               length)) {
          return false;
        }
      }
      return true;
    }
    case WireFormatLite::WIRETYPE_START_GROUP: {
      if (!input->IncrementRecursionDepth()) return false;
      if (!SkipMessage(input, (unknown_fields == nullptr)
                                  ? nullptr
                                  : unknown_fields->AddGroup(number))) {
        return false;
      }
      input->DecrementRecursionDepth();
      // Check that the ending tag matched the starting tag.
      if (!input->LastTagWas(
              WireFormatLite::MakeTag(WireFormatLite::GetTagFieldNumber(tag),
                                      WireFormatLite::WIRETYPE_END_GROUP))) {
        return false;
      }
      return true;
    }
    case WireFormatLite::WIRETYPE_END_GROUP: {
      return false;
    }
    case WireFormatLite::WIRETYPE_FIXED32: {
      uint32_t value;
      if (!input->ReadLittleEndian32(&value)) return false;
      if (unknown_fields != nullptr) unknown_fields->AddFixed32(number, value);
      return true;
    }
    default: {
      return false;
    }
  }
}

bool WireFormat::SkipMessage(io::CodedInputStream* input,
                             UnknownFieldSet* unknown_fields) {
  while (true) {
    uint32_t tag = input->ReadTag();
    if (tag == 0) {
      // End of input.  This is a valid place to end, so return true.
      return true;
    }

    WireFormatLite::WireType wire_type = WireFormatLite::GetTagWireType(tag);

    if (wire_type == WireFormatLite::WIRETYPE_END_GROUP) {
      // Must be the end of the message.
      return true;
    }

    if (!SkipField(input, tag, unknown_fields)) return false;
  }
}

bool WireFormat::ReadPackedEnumPreserveUnknowns(io::CodedInputStream* input,
                                                uint32_t field_number,
                                                bool (*is_valid)(int),
                                                UnknownFieldSet* unknown_fields,
                                                RepeatedField<int>* values) {
  uint32_t length;
  if (!input->ReadVarint32(&length)) return false;
  io::CodedInputStream::Limit limit = input->PushLimit(length);
  while (input->BytesUntilLimit() > 0) {
    int value;
    if (!WireFormatLite::ReadPrimitive<int, WireFormatLite::TYPE_ENUM>(
            input, &value)) {
      return false;
    }
    if (is_valid == nullptr || is_valid(value)) {
      values->Add(value);
    } else {
      unknown_fields->AddVarint(field_number, value);
    }
  }
  input->PopLimit(limit);
  return true;
}

uint8_t* WireFormat::InternalSerializeUnknownFieldsToArray(
    const UnknownFieldSet& unknown_fields, uint8_t* target,
    io::EpsCopyOutputStream* stream) {
  for (int i = 0; i < unknown_fields.field_count(); i++) {
    const UnknownField& field = unknown_fields.field(i);

    target = stream->EnsureSpace(target);
    switch (field.type()) {
      case UnknownField::TYPE_VARINT:
        target = WireFormatLite::WriteUInt64ToArray(field.number(),
                                                    field.varint(), target);
        break;
      case UnknownField::TYPE_FIXED32:
        target = WireFormatLite::WriteFixed32ToArray(field.number(),
                                                     field.fixed32(), target);
        break;
      case UnknownField::TYPE_FIXED64:
        target = WireFormatLite::WriteFixed64ToArray(field.number(),
                                                     field.fixed64(), target);
        break;
      case UnknownField::TYPE_LENGTH_DELIMITED:
        target = stream->WriteString(field.number(), field.length_delimited(),
                                     target);
        break;
      case UnknownField::TYPE_GROUP:
        target = WireFormatLite::WriteTagToArray(
            field.number(), WireFormatLite::WIRETYPE_START_GROUP, target);
        target = InternalSerializeUnknownFieldsToArray(field.group(), target,
                                                       stream);
        target = stream->EnsureSpace(target);
        target = WireFormatLite::WriteTagToArray(
            field.number(), WireFormatLite::WIRETYPE_END_GROUP, target);
        break;
    }
  }
  return target;
}

uint8_t* WireFormat::InternalSerializeUnknownMessageSetItemsToArray(
    const UnknownFieldSet& unknown_fields, uint8_t* target,
    io::EpsCopyOutputStream* stream) {
  for (int i = 0; i < unknown_fields.field_count(); i++) {
    const UnknownField& field = unknown_fields.field(i);

    // The only unknown fields that are allowed to exist in a MessageSet are
    // messages, which are length-delimited.
    if (field.type() == UnknownField::TYPE_LENGTH_DELIMITED) {
      target = stream->EnsureSpace(target);
      // Start group.
      target = io::CodedOutputStream::WriteTagToArray(
          WireFormatLite::kMessageSetItemStartTag, target);

      // Write type ID.
      target = io::CodedOutputStream::WriteTagToArray(
          WireFormatLite::kMessageSetTypeIdTag, target);
      target =
          io::CodedOutputStream::WriteVarint32ToArray(field.number(), target);

      // Write message.
      target = io::CodedOutputStream::WriteTagToArray(
          WireFormatLite::kMessageSetMessageTag, target);

      target = field.InternalSerializeLengthDelimitedNoTag(target, stream);

      target = stream->EnsureSpace(target);
      // End group.
      target = io::CodedOutputStream::WriteTagToArray(
          WireFormatLite::kMessageSetItemEndTag, target);
    }
  }

  return target;
}

size_t WireFormat::ComputeUnknownFieldsSize(
    const UnknownFieldSet& unknown_fields) {
  size_t size = 0;
  for (int i = 0; i < unknown_fields.field_count(); i++) {
    const UnknownField& field = unknown_fields.field(i);

    switch (field.type()) {
      case UnknownField::TYPE_VARINT:
        size += io::CodedOutputStream::VarintSize32(WireFormatLite::MakeTag(
            field.number(), WireFormatLite::WIRETYPE_VARINT));
        size += io::CodedOutputStream::VarintSize64(field.varint());
        break;
      case UnknownField::TYPE_FIXED32:
        size += io::CodedOutputStream::VarintSize32(WireFormatLite::MakeTag(
            field.number(), WireFormatLite::WIRETYPE_FIXED32));
        size += sizeof(int32_t);
        break;
      case UnknownField::TYPE_FIXED64:
        size += io::CodedOutputStream::VarintSize32(WireFormatLite::MakeTag(
            field.number(), WireFormatLite::WIRETYPE_FIXED64));
        size += sizeof(int64_t);
        break;
      case UnknownField::TYPE_LENGTH_DELIMITED:
        size += io::CodedOutputStream::VarintSize32(WireFormatLite::MakeTag(
            field.number(), WireFormatLite::WIRETYPE_LENGTH_DELIMITED));
        size += io::CodedOutputStream::VarintSize32(
            field.length_delimited().size());
        size += field.length_delimited().size();
        break;
      case UnknownField::TYPE_GROUP:
        size += io::CodedOutputStream::VarintSize32(WireFormatLite::MakeTag(
            field.number(), WireFormatLite::WIRETYPE_START_GROUP));
        size += ComputeUnknownFieldsSize(field.group());
        size += io::CodedOutputStream::VarintSize32(WireFormatLite::MakeTag(
            field.number(), WireFormatLite::WIRETYPE_END_GROUP));
        break;
    }
  }

  return size;
}

size_t WireFormat::ComputeUnknownMessageSetItemsSize(
    const UnknownFieldSet& unknown_fields) {
  size_t size = 0;
  for (int i = 0; i < unknown_fields.field_count(); i++) {
    const UnknownField& field = unknown_fields.field(i);

    // The only unknown fields that are allowed to exist in a MessageSet are
    // messages, which are length-delimited.
    if (field.type() == UnknownField::TYPE_LENGTH_DELIMITED) {
      size += WireFormatLite::kMessageSetItemTagsSize;
      size += io::CodedOutputStream::VarintSize32(field.number());

      int field_size = field.GetLengthDelimitedSize();
      size += io::CodedOutputStream::VarintSize32(field_size);
      size += field_size;
    }
  }

  return size;
}

// ===================================================================

bool WireFormat::ParseAndMergePartial(io::CodedInputStream* input,
                                      Message* message) {
  const Descriptor* descriptor = message->GetDescriptor();
  const Reflection* message_reflection = message->GetReflection();

  while (true) {
    uint32_t tag = input->ReadTag();
    if (tag == 0) {
      // End of input.  This is a valid place to end, so return true.
      return true;
    }

    if (WireFormatLite::GetTagWireType(tag) ==
        WireFormatLite::WIRETYPE_END_GROUP) {
      // Must be the end of the message.
      return true;
    }

    const FieldDescriptor* field = nullptr;

    if (descriptor != nullptr) {
      int field_number = WireFormatLite::GetTagFieldNumber(tag);
      field = descriptor->FindFieldByNumber(field_number);

      // If that failed, check if the field is an extension.
      if (field == nullptr && descriptor->IsExtensionNumber(field_number)) {
        if (input->GetExtensionPool() == nullptr) {
          field = message_reflection->FindKnownExtensionByNumber(field_number);
        } else {
          field = input->GetExtensionPool()->FindExtensionByNumber(
              descriptor, field_number);
        }
      }

      // If that failed, but we're a MessageSet, and this is the tag for a
      // MessageSet item, then parse that.
      if (field == nullptr && descriptor->options().message_set_wire_format() &&
          tag == WireFormatLite::kMessageSetItemStartTag) {
        if (!ParseAndMergeMessageSetItem(input, message)) {
          return false;
        }
        continue;  // Skip ParseAndMergeField(); already taken care of.
      }
    }

    if (!ParseAndMergeField(tag, field, message, input)) {
      return false;
    }
  }
}

bool WireFormat::SkipMessageSetField(io::CodedInputStream* input,
                                     uint32_t field_number,
                                     UnknownFieldSet* unknown_fields) {
  uint32_t length;
  if (!input->ReadVarint32(&length)) return false;
  return input->ReadString(unknown_fields->AddLengthDelimited(field_number),
                           length);
}

bool WireFormat::ParseAndMergeMessageSetField(uint32_t field_number,
                                              const FieldDescriptor* field,
                                              Message* message,
                                              io::CodedInputStream* input) {
  const Reflection* message_reflection = message->GetReflection();
  if (field == nullptr) {
    // We store unknown MessageSet extensions as groups.
    return SkipMessageSetField(
        input, field_number, message_reflection->MutableUnknownFields(message));
  } else if (field->is_repeated() ||
             field->type() != FieldDescriptor::TYPE_MESSAGE) {
    // This shouldn't happen as we only allow optional message extensions to
    // MessageSet.
    ABSL_LOG(ERROR) << "Extensions of MessageSets must be optional messages.";
    return false;
  } else {
    Message* sub_message = message_reflection->MutableMessage(
        message, field, input->GetExtensionFactory());
    return WireFormatLite::ReadMessage(input, sub_message);
  }
}

bool WireFormat::ParseAndMergeField(
    uint32_t tag,
    const FieldDescriptor* field,  // May be nullptr for unknown
    Message* message, io::CodedInputStream* input) {
  const Reflection* message_reflection = message->GetReflection();

  enum { UNKNOWN, NORMAL_FORMAT, PACKED_FORMAT } value_format;

  if (field == nullptr) {
    value_format = UNKNOWN;
  } else if (WireFormatLite::GetTagWireType(tag) ==
             WireTypeForFieldType(field->type())) {
    value_format = NORMAL_FORMAT;
  } else if (field->is_packable() &&
             WireFormatLite::GetTagWireType(tag) ==
                 WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    value_format = PACKED_FORMAT;
  } else {
    // We don't recognize this field. Either the field number is unknown
    // or the wire type doesn't match. Put it in our unknown field set.
    value_format = UNKNOWN;
  }

  if (value_format == UNKNOWN) {
    return SkipField(input, tag,
                     message_reflection->MutableUnknownFields(message));
  } else if (value_format == PACKED_FORMAT) {
    uint32_t length;
    if (!input->ReadVarint32(&length)) return false;
    io::CodedInputStream::Limit limit = input->PushLimit(length);

    switch (field->type()) {
#define HANDLE_PACKED_TYPE(TYPE, CPPTYPE, CPPTYPE_METHOD)                      \
  case FieldDescriptor::TYPE_##TYPE: {                                         \
    while (input->BytesUntilLimit() > 0) {                                     \
      CPPTYPE value;                                                           \
      if (!WireFormatLite::ReadPrimitive<CPPTYPE,                              \
                                         WireFormatLite::TYPE_##TYPE>(input,   \
                                                                      &value)) \
        return false;                                                          \
      message_reflection->Add##CPPTYPE_METHOD(message, field, value);          \
    }                                                                          \
    break;                                                                     \
  }

      HANDLE_PACKED_TYPE(INT32, int32_t, Int32)
      HANDLE_PACKED_TYPE(INT64, int64_t, Int64)
      HANDLE_PACKED_TYPE(SINT32, int32_t, Int32)
      HANDLE_PACKED_TYPE(SINT64, int64_t, Int64)
      HANDLE_PACKED_TYPE(UINT32, uint32_t, UInt32)
      HANDLE_PACKED_TYPE(UINT64, uint64_t, UInt64)

      HANDLE_PACKED_TYPE(FIXED32, uint32_t, UInt32)
      HANDLE_PACKED_TYPE(FIXED64, uint64_t, UInt64)
      HANDLE_PACKED_TYPE(SFIXED32, int32_t, Int32)
      HANDLE_PACKED_TYPE(SFIXED64, int64_t, Int64)

      HANDLE_PACKED_TYPE(FLOAT, float, Float)
      HANDLE_PACKED_TYPE(DOUBLE, double, Double)

      HANDLE_PACKED_TYPE(BOOL, bool, Bool)
#undef HANDLE_PACKED_TYPE

      case FieldDescriptor::TYPE_ENUM: {
        while (input->BytesUntilLimit() > 0) {
          int value;
          if (!WireFormatLite::ReadPrimitive<int, WireFormatLite::TYPE_ENUM>(
                  input, &value))
            return false;
          if (!field->legacy_enum_field_treated_as_closed()) {
            message_reflection->AddEnumValue(message, field, value);
          } else {
            const EnumValueDescriptor* enum_value =
                field->enum_type()->FindValueByNumber(value);
            if (enum_value != nullptr) {
              message_reflection->AddEnum(message, field, enum_value);
            } else {
              // The enum value is not one of the known values.  Add it to the
              // UnknownFieldSet.
              int64_t sign_extended_value = static_cast<int64_t>(value);
              message_reflection->MutableUnknownFields(message)->AddVarint(
                  WireFormatLite::GetTagFieldNumber(tag), sign_extended_value);
            }
          }
        }

        break;
      }

      case FieldDescriptor::TYPE_STRING:
      case FieldDescriptor::TYPE_GROUP:
      case FieldDescriptor::TYPE_MESSAGE:
      case FieldDescriptor::TYPE_BYTES:
        // Can't have packed fields of these types: these should be caught by
        // the protocol compiler.
        return false;
        break;
    }

    input->PopLimit(limit);
  } else {
    // Non-packed value (value_format == NORMAL_FORMAT)
    switch (field->type()) {
#define HANDLE_TYPE(TYPE, CPPTYPE, CPPTYPE_METHOD)                            \
  case FieldDescriptor::TYPE_##TYPE: {                                        \
    CPPTYPE value;                                                            \
    if (!WireFormatLite::ReadPrimitive<CPPTYPE, WireFormatLite::TYPE_##TYPE>( \
            input, &value))                                                   \
      return false;                                                           \
    if (field->is_repeated()) {                                               \
      message_reflection->Add##CPPTYPE_METHOD(message, field, value);         \
    } else {                                                                  \
      message_reflection->Set##CPPTYPE_METHOD(message, field, value);         \
    }                                                                         \
    break;                                                                    \
  }

      HANDLE_TYPE(INT32, int32_t, Int32)
      HANDLE_TYPE(INT64, int64_t, Int64)
      HANDLE_TYPE(SINT32, int32_t, Int32)
      HANDLE_TYPE(SINT64, int64_t, Int64)
      HANDLE_TYPE(UINT32, uint32_t, UInt32)
      HANDLE_TYPE(UINT64, uint64_t, UInt64)

      HANDLE_TYPE(FIXED32, uint32_t, UInt32)
      HANDLE_TYPE(FIXED64, uint64_t, UInt64)
      HANDLE_TYPE(SFIXED32, int32_t, Int32)
      HANDLE_TYPE(SFIXED64, int64_t, Int64)

      HANDLE_TYPE(FLOAT, float, Float)
      HANDLE_TYPE(DOUBLE, double, Double)

      HANDLE_TYPE(BOOL, bool, Bool)
#undef HANDLE_TYPE

      case FieldDescriptor::TYPE_ENUM: {
        int value;
        if (!WireFormatLite::ReadPrimitive<int, WireFormatLite::TYPE_ENUM>(
                input, &value))
          return false;
        if (field->is_repeated()) {
          message_reflection->AddEnumValue(message, field, value);
        } else {
          message_reflection->SetEnumValue(message, field, value);
        }
        break;
      }

      // Handle strings separately so that we can optimize the ctype=CORD case.
      case FieldDescriptor::TYPE_STRING: {
        bool strict_utf8_check = field->requires_utf8_validation();
        std::string value;
        if (!WireFormatLite::ReadString(input, &value)) return false;
        if (strict_utf8_check) {
          if (!WireFormatLite::VerifyUtf8String(value.data(), value.length(),
                                                WireFormatLite::PARSE,
                                                field->full_name().c_str())) {
            return false;
          }
        } else {
          VerifyUTF8StringNamedField(value.data(), value.length(), PARSE,
                                     field->full_name().c_str());
        }
        if (field->is_repeated()) {
          message_reflection->AddString(message, field, value);
        } else {
          message_reflection->SetString(message, field, value);
        }
        break;
      }

      case FieldDescriptor::TYPE_BYTES: {
        if (internal::cpp::EffectiveStringCType(field) == FieldOptions::CORD) {
          absl::Cord value;
          if (!WireFormatLite::ReadBytes(input, &value)) return false;
          message_reflection->SetString(message, field, value);
          break;
        }
        std::string value;
        if (!WireFormatLite::ReadBytes(input, &value)) return false;
        if (field->is_repeated()) {
          message_reflection->AddString(message, field, value);
        } else {
          message_reflection->SetString(message, field, value);
        }
        break;
      }

      case FieldDescriptor::TYPE_GROUP: {
        Message* sub_message;
        if (field->is_repeated()) {
          sub_message = message_reflection->AddMessage(
              message, field, input->GetExtensionFactory());
        } else {
          sub_message = message_reflection->MutableMessage(
              message, field, input->GetExtensionFactory());
        }

        if (!WireFormatLite::ReadGroup(WireFormatLite::GetTagFieldNumber(tag),
                                       input, sub_message))
          return false;
        break;
      }

      case FieldDescriptor::TYPE_MESSAGE: {
        Message* sub_message;
        if (field->is_repeated()) {
          sub_message = message_reflection->AddMessage(
              message, field, input->GetExtensionFactory());
        } else {
          sub_message = message_reflection->MutableMessage(
              message, field, input->GetExtensionFactory());
        }

        if (!WireFormatLite::ReadMessage(input, sub_message)) return false;
        break;
      }
    }
  }

  return true;
}

bool WireFormat::ParseAndMergeMessageSetItem(io::CodedInputStream* input,
                                             Message* message) {
  struct MSReflective {
    bool ParseField(int type_id, io::CodedInputStream* input) {
      const FieldDescriptor* field =
          message_reflection->FindKnownExtensionByNumber(type_id);
      return ParseAndMergeMessageSetField(type_id, field, message, input);
    }

    bool SkipField(uint32_t tag, io::CodedInputStream* input) {
      return WireFormat::SkipField(input, tag, nullptr);
    }

    const Reflection* message_reflection;
    Message* message;
  };

  return ParseMessageSetItemImpl(
      input, MSReflective{message->GetReflection(), message});
}

struct WireFormat::MessageSetParser {
  const char* _InternalParse(const char* ptr, internal::ParseContext* ctx) {
    // Parse a MessageSetItem
    auto metadata = reflection->MutableInternalMetadata(msg);
    enum class State { kNoTag, kHasType, kHasPayload, kDone };
    State state = State::kNoTag;

    std::string payload;
    uint32_t type_id = 0;
    while (!ctx->Done(&ptr)) {
      // We use 64 bit tags in order to allow typeid's that span the whole
      // range of 32 bit numbers.
      uint32_t tag = static_cast<uint8_t>(*ptr++);
      if (tag == WireFormatLite::kMessageSetTypeIdTag) {
        uint64_t tmp;
        ptr = ParseBigVarint(ptr, &tmp);
        // We should fail parsing if type id is 0 after cast to uint32.
        GOOGLE_PROTOBUF_PARSER_ASSERT(ptr != nullptr &&
                                       static_cast<uint32_t>(tmp) != 0);
        if (state == State::kNoTag) {
          type_id = static_cast<uint32_t>(tmp);
          state = State::kHasType;
        } else if (state == State::kHasPayload) {
          type_id = static_cast<uint32_t>(tmp);
          const FieldDescriptor* field;
          if (ctx->data().pool == nullptr) {
            field = reflection->FindKnownExtensionByNumber(type_id);
          } else {
            field =
                ctx->data().pool->FindExtensionByNumber(descriptor, type_id);
          }
          if (field == nullptr || field->message_type() == nullptr) {
            WriteLengthDelimited(
                type_id, payload,
                metadata->mutable_unknown_fields<UnknownFieldSet>());
          } else {
            Message* value =
                field->is_repeated()
                    ? reflection->AddMessage(msg, field, ctx->data().factory)
                    : reflection->MutableMessage(msg, field,
                                                 ctx->data().factory);
            const char* p;
            // We can't use regular parse from string as we have to track
            // proper recursion depth and descriptor pools. Spawn a new
            // ParseContext inheriting those attributes.
            ParseContext tmp_ctx(ParseContext::kSpawn, *ctx, &p, payload);
            GOOGLE_PROTOBUF_PARSER_ASSERT(value->_InternalParse(p, &tmp_ctx) &&
                                           tmp_ctx.EndedAtLimit());
          }
          state = State::kDone;
        }
        continue;
      } else if (tag == WireFormatLite::kMessageSetMessageTag) {
        if (state == State::kNoTag) {
          int32_t size = ReadSize(&ptr);
          GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
          ptr = ctx->ReadString(ptr, size, &payload);
          GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
          state = State::kHasPayload;
        } else if (state == State::kHasType) {
          // We're now parsing the payload
          const FieldDescriptor* field = nullptr;
          if (descriptor->IsExtensionNumber(type_id)) {
            if (ctx->data().pool == nullptr) {
              field = reflection->FindKnownExtensionByNumber(type_id);
            } else {
              field =
                  ctx->data().pool->FindExtensionByNumber(descriptor, type_id);
            }
          }
          ptr = WireFormat::_InternalParseAndMergeField(
              msg, ptr, ctx, static_cast<uint64_t>(type_id) * 8 + 2, reflection,
              field);
          state = State::kDone;
        } else {
          int32_t size = ReadSize(&ptr);
          GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
          ptr = ctx->Skip(ptr, size);
          GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
        }
      } else {
        // An unknown field in MessageSetItem.
        ptr = ReadTag(ptr - 1, &tag);
        if (tag == 0 || (tag & 7) == WireFormatLite::WIRETYPE_END_GROUP) {
          ctx->SetLastTag(tag);
          return ptr;
        }
        // Skip field.
        ptr = internal::UnknownFieldParse(
            tag, static_cast<std::string*>(nullptr), ptr, ctx);
      }
      GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
    }
    return ptr;
  }

  const char* ParseMessageSet(const char* ptr, internal::ParseContext* ctx) {
    while (!ctx->Done(&ptr)) {
      uint32_t tag;
      ptr = ReadTag(ptr, &tag);
      if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
      if (tag == 0 || (tag & 7) == WireFormatLite::WIRETYPE_END_GROUP) {
        ctx->SetLastTag(tag);
        break;
      }
      if (tag == WireFormatLite::kMessageSetItemStartTag) {
        // A message set item starts
        ptr = ctx->ParseGroup(this, ptr, tag);
      } else {
        // Parse other fields as normal extensions.
        int field_number = WireFormatLite::GetTagFieldNumber(tag);
        const FieldDescriptor* field = nullptr;
        if (descriptor->IsExtensionNumber(field_number)) {
          if (ctx->data().pool == nullptr) {
            field = reflection->FindKnownExtensionByNumber(field_number);
          } else {
            field = ctx->data().pool->FindExtensionByNumber(descriptor,
                                                            field_number);
          }
        }
        ptr = WireFormat::_InternalParseAndMergeField(msg, ptr, ctx, tag,
                                                      reflection, field);
      }
      if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
    }
    return ptr;
  }

  Message* msg;
  const Descriptor* descriptor;
  const Reflection* reflection;
};

const char* WireFormat::_InternalParse(Message* msg, const char* ptr,
                                       internal::ParseContext* ctx) {
  const Descriptor* descriptor = msg->GetDescriptor();
  const Reflection* reflection = msg->GetReflection();
  ABSL_DCHECK(descriptor);
  ABSL_DCHECK(reflection);
  if (descriptor->options().message_set_wire_format()) {
    MessageSetParser message_set{msg, descriptor, reflection};
    return message_set.ParseMessageSet(ptr, ctx);
  }
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ReadTag(ptr, &tag);
    if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
    if (tag == 0 || (tag & 7) == WireFormatLite::WIRETYPE_END_GROUP) {
      ctx->SetLastTag(tag);
      break;
    }
    const FieldDescriptor* field = nullptr;

    int field_number = WireFormatLite::GetTagFieldNumber(tag);
    field = descriptor->FindFieldByNumber(field_number);

    // If that failed, check if the field is an extension.
    if (field == nullptr && descriptor->IsExtensionNumber(field_number)) {
      if (ctx->data().pool == nullptr) {
        field = reflection->FindKnownExtensionByNumber(field_number);
      } else {
        field =
            ctx->data().pool->FindExtensionByNumber(descriptor, field_number);
      }
    }

    ptr = _InternalParseAndMergeField(msg, ptr, ctx, tag, reflection, field);
    if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
  }
  return ptr;
}

const char* WireFormat::_InternalParseAndMergeField(
    Message* msg, const char* ptr, internal::ParseContext* ctx, uint64_t tag,
    const Reflection* reflection, const FieldDescriptor* field) {
  if (field == nullptr) {
    // unknown field set parser takes 64bit tags, because message set type ids
    // span the full 32 bit range making the tag span [0, 2^35) range.
    return internal::UnknownFieldParse(
        tag, reflection->MutableUnknownFields(msg), ptr, ctx);
  }
  if (WireFormatLite::GetTagWireType(tag) !=
      WireTypeForFieldType(field->type())) {
    if (field->is_packable() && WireFormatLite::GetTagWireType(tag) ==
                                    WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
      switch (field->type()) {
#define HANDLE_PACKED_TYPE(TYPE, CPPTYPE, CPPTYPE_METHOD)                   \
  case FieldDescriptor::TYPE_##TYPE: {                                      \
    ptr = internal::Packed##CPPTYPE_METHOD##Parser(                         \
        reflection->MutableRepeatedFieldInternal<CPPTYPE>(msg, field), ptr, \
        ctx);                                                               \
    return ptr;                                                             \
  }

        HANDLE_PACKED_TYPE(INT32, int32_t, Int32)
        HANDLE_PACKED_TYPE(INT64, int64_t, Int64)
        HANDLE_PACKED_TYPE(SINT32, int32_t, SInt32)
        HANDLE_PACKED_TYPE(SINT64, int64_t, SInt64)
        HANDLE_PACKED_TYPE(UINT32, uint32_t, UInt32)
        HANDLE_PACKED_TYPE(UINT64, uint64_t, UInt64)

        HANDLE_PACKED_TYPE(FIXED32, uint32_t, Fixed32)
        HANDLE_PACKED_TYPE(FIXED64, uint64_t, Fixed64)
        HANDLE_PACKED_TYPE(SFIXED32, int32_t, SFixed32)
        HANDLE_PACKED_TYPE(SFIXED64, int64_t, SFixed64)

        HANDLE_PACKED_TYPE(FLOAT, float, Float)
        HANDLE_PACKED_TYPE(DOUBLE, double, Double)

        HANDLE_PACKED_TYPE(BOOL, bool, Bool)
#undef HANDLE_PACKED_TYPE

        case FieldDescriptor::TYPE_ENUM: {
          auto rep_enum =
              reflection->MutableRepeatedFieldInternal<int>(msg, field);
          if (!field->legacy_enum_field_treated_as_closed()) {
            ptr = internal::PackedEnumParser(rep_enum, ptr, ctx);
          } else {
            return ctx->ReadPackedVarint(
                ptr, [rep_enum, field, reflection, msg](int32_t val) {
                  if (field->enum_type()->FindValueByNumber(val) != nullptr) {
                    rep_enum->Add(val);
                  } else {
                    WriteVarint(field->number(), val,
                                reflection->MutableUnknownFields(msg));
                  }
                });
          }
          return ptr;
        }

        case FieldDescriptor::TYPE_STRING:
        case FieldDescriptor::TYPE_GROUP:
        case FieldDescriptor::TYPE_MESSAGE:
        case FieldDescriptor::TYPE_BYTES:
          ABSL_LOG(FATAL) << "Can't reach";
          return nullptr;
      }
    } else {
      // mismatched wiretype;
      return internal::UnknownFieldParse(
          tag, reflection->MutableUnknownFields(msg), ptr, ctx);
    }
  }

  // Non-packed value
  bool utf8_check = false;
  bool strict_utf8_check = false;
  switch (field->type()) {
#define HANDLE_TYPE(TYPE, CPPTYPE, CPPTYPE_METHOD)        \
  case FieldDescriptor::TYPE_##TYPE: {                    \
    CPPTYPE value;                                        \
    ptr = VarintParse(ptr, &value);                       \
    if (ptr == nullptr) return nullptr;                   \
    if (field->is_repeated()) {                           \
      reflection->Add##CPPTYPE_METHOD(msg, field, value); \
    } else {                                              \
      reflection->Set##CPPTYPE_METHOD(msg, field, value); \
    }                                                     \
    return ptr;                                           \
  }

    HANDLE_TYPE(BOOL, uint64_t, Bool)
    HANDLE_TYPE(INT32, uint32_t, Int32)
    HANDLE_TYPE(INT64, uint64_t, Int64)
    HANDLE_TYPE(UINT32, uint32_t, UInt32)
    HANDLE_TYPE(UINT64, uint64_t, UInt64)

    case FieldDescriptor::TYPE_SINT32: {
      int32_t value = ReadVarintZigZag32(&ptr);
      if (ptr == nullptr) return nullptr;
      if (field->is_repeated()) {
        reflection->AddInt32(msg, field, value);
      } else {
        reflection->SetInt32(msg, field, value);
      }
      return ptr;
    }
    case FieldDescriptor::TYPE_SINT64: {
      int64_t value = ReadVarintZigZag64(&ptr);
      if (ptr == nullptr) return nullptr;
      if (field->is_repeated()) {
        reflection->AddInt64(msg, field, value);
      } else {
        reflection->SetInt64(msg, field, value);
      }
      return ptr;
    }
#undef HANDLE_TYPE
#define HANDLE_TYPE(TYPE, CPPTYPE, CPPTYPE_METHOD)        \
  case FieldDescriptor::TYPE_##TYPE: {                    \
    CPPTYPE value;                                        \
    value = UnalignedLoad<CPPTYPE>(ptr);                  \
    ptr += sizeof(CPPTYPE);                               \
    if (field->is_repeated()) {                           \
      reflection->Add##CPPTYPE_METHOD(msg, field, value); \
    } else {                                              \
      reflection->Set##CPPTYPE_METHOD(msg, field, value); \
    }                                                     \
    return ptr;                                           \
  }

      HANDLE_TYPE(FIXED32, uint32_t, UInt32)
      HANDLE_TYPE(FIXED64, uint64_t, UInt64)
      HANDLE_TYPE(SFIXED32, int32_t, Int32)
      HANDLE_TYPE(SFIXED64, int64_t, Int64)

      HANDLE_TYPE(FLOAT, float, Float)
      HANDLE_TYPE(DOUBLE, double, Double)

#undef HANDLE_TYPE

    case FieldDescriptor::TYPE_ENUM: {
      uint32_t value;
      ptr = VarintParse(ptr, &value);
      if (ptr == nullptr) return nullptr;
      if (field->is_repeated()) {
        reflection->AddEnumValue(msg, field, value);
      } else {
        reflection->SetEnumValue(msg, field, value);
      }
      return ptr;
    }

    // Handle strings separately so that we can optimize the ctype=CORD case.
    case FieldDescriptor::TYPE_STRING:
      utf8_check = true;
      strict_utf8_check = field->requires_utf8_validation();
      PROTOBUF_FALLTHROUGH_INTENDED;
    case FieldDescriptor::TYPE_BYTES: {
      int size = ReadSize(&ptr);
      if (ptr == nullptr) return nullptr;
      if (internal::cpp::EffectiveStringCType(field) == FieldOptions::CORD) {
        absl::Cord value;
        ptr = ctx->ReadCord(ptr, size, &value);
        if (ptr == nullptr) return nullptr;
        reflection->SetString(msg, field, value);
        return ptr;
      }
      std::string value;
      ptr = ctx->ReadString(ptr, size, &value);
      if (ptr == nullptr) return nullptr;
      if (utf8_check) {
        if (strict_utf8_check) {
          if (!WireFormatLite::VerifyUtf8String(value.data(), value.length(),
                                                WireFormatLite::PARSE,
                                                field->full_name().c_str())) {
            return nullptr;
          }
        } else {
          VerifyUTF8StringNamedField(value.data(), value.length(), PARSE,
                                     field->full_name().c_str());
        }
      }
      if (field->is_repeated()) {
        reflection->AddString(msg, field, std::move(value));
      } else {
        reflection->SetString(msg, field, std::move(value));
      }
      return ptr;
    }

    case FieldDescriptor::TYPE_GROUP: {
      Message* sub_message;
      if (field->is_repeated()) {
        sub_message = reflection->AddMessage(msg, field, ctx->data().factory);
      } else {
        sub_message =
            reflection->MutableMessage(msg, field, ctx->data().factory);
      }

      return ctx->ParseGroup(sub_message, ptr, tag);
    }

    case FieldDescriptor::TYPE_MESSAGE: {
      Message* sub_message;
      if (field->is_repeated()) {
        sub_message = reflection->AddMessage(msg, field, ctx->data().factory);
      } else {
        sub_message =
            reflection->MutableMessage(msg, field, ctx->data().factory);
      }
      ptr = ctx->ParseMessage(sub_message, ptr);

      // For map entries, if the value is an unknown enum we have to push it
      // into the unknown field set and remove it from the list.
      if (ptr != nullptr && field->is_map()) {
        auto* value_field = field->message_type()->map_value();
        auto* enum_type = value_field->enum_type();
        if (enum_type != nullptr &&
            !internal::cpp::HasPreservingUnknownEnumSemantics(value_field) &&
            enum_type->FindValueByNumber(
                sub_message->GetReflection()->GetEnumValue(
                    *sub_message, value_field)) == nullptr) {
          reflection->MutableUnknownFields(msg)->AddLengthDelimited(
              field->number(), sub_message->SerializeAsString());
          reflection->RemoveLast(msg, field);
        }
      }

      return ptr;
    }
  }

  // GCC 8 complains about control reaching end of non-void function here.
  // Let's keep it happy by returning a nullptr.
  return nullptr;
}

// ===================================================================

uint8_t* WireFormat::_InternalSerialize(const Message& message, uint8_t* target,
                                        io::EpsCopyOutputStream* stream) {
  const Descriptor* descriptor = message.GetDescriptor();
  const Reflection* message_reflection = message.GetReflection();

  std::vector<const FieldDescriptor*> fields;

  // Fields of map entry should always be serialized.
  if (descriptor->options().map_entry()) {
    for (int i = 0; i < descriptor->field_count(); i++) {
      fields.push_back(descriptor->field(i));
    }
  } else {
    message_reflection->ListFields(message, &fields);
  }

  for (auto field : fields) {
    target = InternalSerializeField(field, message, target, stream);
  }

  if (descriptor->options().message_set_wire_format()) {
    return InternalSerializeUnknownMessageSetItemsToArray(
        message_reflection->GetUnknownFields(message), target, stream);
  } else {
    return InternalSerializeUnknownFieldsToArray(
        message_reflection->GetUnknownFields(message), target, stream);
  }
}

uint8_t* SerializeMapKeyWithCachedSizes(const FieldDescriptor* field,
                                        const MapKey& value, uint8_t* target,
                                        io::EpsCopyOutputStream* stream) {
  target = stream->EnsureSpace(target);
  switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE:
    case FieldDescriptor::TYPE_FLOAT:
    case FieldDescriptor::TYPE_GROUP:
    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_ENUM:
      ABSL_LOG(FATAL) << "Unsupported";
      break;
#define CASE_TYPE(FieldType, CamelFieldType, CamelCppType)   \
  case FieldDescriptor::TYPE_##FieldType:                    \
    target = WireFormatLite::Write##CamelFieldType##ToArray( \
        1, value.Get##CamelCppType##Value(), target);        \
    break;
      CASE_TYPE(INT64, Int64, Int64)
      CASE_TYPE(UINT64, UInt64, UInt64)
      CASE_TYPE(INT32, Int32, Int32)
      CASE_TYPE(FIXED64, Fixed64, UInt64)
      CASE_TYPE(FIXED32, Fixed32, UInt32)
      CASE_TYPE(BOOL, Bool, Bool)
      CASE_TYPE(UINT32, UInt32, UInt32)
      CASE_TYPE(SFIXED32, SFixed32, Int32)
      CASE_TYPE(SFIXED64, SFixed64, Int64)
      CASE_TYPE(SINT32, SInt32, Int32)
      CASE_TYPE(SINT64, SInt64, Int64)
#undef CASE_TYPE
    case FieldDescriptor::TYPE_STRING:
      target = stream->WriteString(1, value.GetStringValue(), target);
      break;
  }
  return target;
}

static uint8_t* SerializeMapValueRefWithCachedSizes(
    const FieldDescriptor* field, const MapValueConstRef& value,
    uint8_t* target, io::EpsCopyOutputStream* stream) {
  target = stream->EnsureSpace(target);
  switch (field->type()) {
#define CASE_TYPE(FieldType, CamelFieldType, CamelCppType)   \
  case FieldDescriptor::TYPE_##FieldType:                    \
    target = WireFormatLite::Write##CamelFieldType##ToArray( \
        2, value.Get##CamelCppType##Value(), target);        \
    break;
    CASE_TYPE(INT64, Int64, Int64)
    CASE_TYPE(UINT64, UInt64, UInt64)
    CASE_TYPE(INT32, Int32, Int32)
    CASE_TYPE(FIXED64, Fixed64, UInt64)
    CASE_TYPE(FIXED32, Fixed32, UInt32)
    CASE_TYPE(BOOL, Bool, Bool)
    CASE_TYPE(UINT32, UInt32, UInt32)
    CASE_TYPE(SFIXED32, SFixed32, Int32)
    CASE_TYPE(SFIXED64, SFixed64, Int64)
    CASE_TYPE(SINT32, SInt32, Int32)
    CASE_TYPE(SINT64, SInt64, Int64)
    CASE_TYPE(ENUM, Enum, Enum)
    CASE_TYPE(DOUBLE, Double, Double)
    CASE_TYPE(FLOAT, Float, Float)
#undef CASE_TYPE
    case FieldDescriptor::TYPE_STRING:
    case FieldDescriptor::TYPE_BYTES:
      target = stream->WriteString(2, value.GetStringValue(), target);
      break;
    case FieldDescriptor::TYPE_MESSAGE: {
      auto& msg = value.GetMessageValue();
      target = WireFormatLite::InternalWriteMessage(2, msg, msg.GetCachedSize(),
                                                    target, stream);
    } break;
    case FieldDescriptor::TYPE_GROUP:
      target = WireFormatLite::InternalWriteGroup(2, value.GetMessageValue(),
                                                  target, stream);
      break;
  }
  return target;
}

class MapKeySorter {
 public:
  static std::vector<MapKey> SortKey(const Message& message,
                                     const Reflection* reflection,
                                     const FieldDescriptor* field) {
    std::vector<MapKey> sorted_key_list;
    for (MapIterator it =
             reflection->MapBegin(const_cast<Message*>(&message), field);
         it != reflection->MapEnd(const_cast<Message*>(&message), field);
         ++it) {
      sorted_key_list.push_back(it.GetKey());
    }
    MapKeyComparator comparator;
    std::sort(sorted_key_list.begin(), sorted_key_list.end(), comparator);
    return sorted_key_list;
  }

 private:
  class MapKeyComparator {
   public:
    bool operator()(const MapKey& a, const MapKey& b) const {
      ABSL_DCHECK(a.type() == b.type());
      switch (a.type()) {
#define CASE_TYPE(CppType, CamelCppType)                                \
  case FieldDescriptor::CPPTYPE_##CppType: {                            \
    return a.Get##CamelCppType##Value() < b.Get##CamelCppType##Value(); \
  }
        CASE_TYPE(STRING, String)
        CASE_TYPE(INT64, Int64)
        CASE_TYPE(INT32, Int32)
        CASE_TYPE(UINT64, UInt64)
        CASE_TYPE(UINT32, UInt32)
        CASE_TYPE(BOOL, Bool)
#undef CASE_TYPE

        default:
          ABSL_DLOG(FATAL) << "Invalid key for map field.";
          return true;
      }
    }
  };
};

static uint8_t* InternalSerializeMapEntry(const FieldDescriptor* field,
                                          const MapKey& key,
                                          const MapValueConstRef& value,
                                          uint8_t* target,
                                          io::EpsCopyOutputStream* stream) {
  const FieldDescriptor* key_field = field->message_type()->field(0);
  const FieldDescriptor* value_field = field->message_type()->field(1);

  size_t size = kMapEntryTagByteSize;
  size += MapKeyDataOnlyByteSize(key_field, key);
  size += MapValueRefDataOnlyByteSize(value_field, value);
  target = stream->EnsureSpace(target);
  target = WireFormatLite::WriteTagToArray(
      field->number(), WireFormatLite::WIRETYPE_LENGTH_DELIMITED, target);
  target = io::CodedOutputStream::WriteVarint32ToArray(size, target);
  target = SerializeMapKeyWithCachedSizes(key_field, key, target, stream);
  target =
      SerializeMapValueRefWithCachedSizes(value_field, value, target, stream);
  return target;
}

uint8_t* WireFormat::InternalSerializeField(const FieldDescriptor* field,
                                            const Message& message,
                                            uint8_t* target,
                                            io::EpsCopyOutputStream* stream) {
  const Reflection* message_reflection = message.GetReflection();

  if (field->is_extension() &&
      field->containing_type()->options().message_set_wire_format() &&
      field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
      !field->is_repeated()) {
    return InternalSerializeMessageSetItem(field, message, target, stream);
  }

  // For map fields, we can use either repeated field reflection or map
  // reflection.  Our choice has some subtle effects.  If we use repeated field
  // reflection here, then the repeated field representation becomes
  // authoritative for this field: any existing references that came from map
  // reflection remain valid for reading, but mutations to them are lost and
  // will be overwritten next time we call map reflection!
  //
  // So far this mainly affects Python, which keeps long-term references to map
  // values around, and always uses map reflection.  See: b/35918691
  //
  // Here we choose to use map reflection API as long as the internal
  // map is valid. In this way, the serialization doesn't change map field's
  // internal state and existing references that came from map reflection remain
  // valid for both reading and writing.
  if (field->is_map()) {
    const MapFieldBase* map_field =
        message_reflection->GetMapData(message, field);
    if (map_field->IsMapValid()) {
      if (stream->IsSerializationDeterministic()) {
        std::vector<MapKey> sorted_key_list =
            MapKeySorter::SortKey(message, message_reflection, field);
        for (std::vector<MapKey>::iterator it = sorted_key_list.begin();
             it != sorted_key_list.end(); ++it) {
          MapValueConstRef map_value;
          message_reflection->LookupMapValue(message, field, *it, &map_value);
          target =
              InternalSerializeMapEntry(field, *it, map_value, target, stream);
        }
      } else {
        for (MapIterator it = message_reflection->MapBegin(
                 const_cast<Message*>(&message), field);
             it !=
             message_reflection->MapEnd(const_cast<Message*>(&message), field);
             ++it) {
          target = InternalSerializeMapEntry(field, it.GetKey(),
                                             it.GetValueRef(), target, stream);
        }
      }

      return target;
    }
  }
  int count = 0;

  if (field->is_repeated()) {
    count = message_reflection->FieldSize(message, field);
  } else if (field->containing_type()->options().map_entry()) {
    // Map entry fields always need to be serialized.
    count = 1;
  } else if (message_reflection->HasField(message, field)) {
    count = 1;
  }

  // map_entries is for maps that'll be deterministically serialized.
  std::vector<const Message*> map_entries;
  if (count > 1 && field->is_map() && stream->IsSerializationDeterministic()) {
    map_entries =
        DynamicMapSorter::Sort(message, count, message_reflection, field);
  }

  if (field->is_packed()) {
    if (count == 0) return target;
    target = stream->EnsureSpace(target);
    switch (field->type()) {
#define HANDLE_PRIMITIVE_TYPE(TYPE, CPPTYPE, TYPE_METHOD, CPPTYPE_METHOD)      \
  case FieldDescriptor::TYPE_##TYPE: {                                         \
    auto r =                                                                   \
        message_reflection->GetRepeatedFieldInternal<CPPTYPE>(message, field); \
    target = stream->Write##TYPE_METHOD##Packed(                               \
        field->number(), r, FieldDataOnlyByteSize(field, message), target);    \
    break;                                                                     \
  }

      HANDLE_PRIMITIVE_TYPE(INT32, int32_t, Int32, Int32)
      HANDLE_PRIMITIVE_TYPE(INT64, int64_t, Int64, Int64)
      HANDLE_PRIMITIVE_TYPE(SINT32, int32_t, SInt32, Int32)
      HANDLE_PRIMITIVE_TYPE(SINT64, int64_t, SInt64, Int64)
      HANDLE_PRIMITIVE_TYPE(UINT32, uint32_t, UInt32, UInt32)
      HANDLE_PRIMITIVE_TYPE(UINT64, uint64_t, UInt64, UInt64)
      HANDLE_PRIMITIVE_TYPE(ENUM, int, Enum, Enum)

#undef HANDLE_PRIMITIVE_TYPE
#define HANDLE_PRIMITIVE_TYPE(TYPE, CPPTYPE, TYPE_METHOD, CPPTYPE_METHOD)      \
  case FieldDescriptor::TYPE_##TYPE: {                                         \
    auto r =                                                                   \
        message_reflection->GetRepeatedFieldInternal<CPPTYPE>(message, field); \
    target = stream->WriteFixedPacked(field->number(), r, target);             \
    break;                                                                     \
  }

      HANDLE_PRIMITIVE_TYPE(FIXED32, uint32_t, Fixed32, UInt32)
      HANDLE_PRIMITIVE_TYPE(FIXED64, uint64_t, Fixed64, UInt64)
      HANDLE_PRIMITIVE_TYPE(SFIXED32, int32_t, SFixed32, Int32)
      HANDLE_PRIMITIVE_TYPE(SFIXED64, int64_t, SFixed64, Int64)

      HANDLE_PRIMITIVE_TYPE(FLOAT, float, Float, Float)
      HANDLE_PRIMITIVE_TYPE(DOUBLE, double, Double, Double)

      HANDLE_PRIMITIVE_TYPE(BOOL, bool, Bool, Bool)
#undef HANDLE_PRIMITIVE_TYPE
      default:
        ABSL_LOG(FATAL) << "Invalid descriptor";
    }
    return target;
  }

  auto get_message_from_field = [&message, &map_entries, message_reflection](
                                    const FieldDescriptor* field, int j) {
    if (!field->is_repeated()) {
      return &message_reflection->GetMessage(message, field);
    }
    if (!map_entries.empty()) {
      return map_entries[j];
    }
    return &message_reflection->GetRepeatedMessage(message, field, j);
  };
  for (int j = 0; j < count; j++) {
    target = stream->EnsureSpace(target);
    switch (field->type()) {
#define HANDLE_PRIMITIVE_TYPE(TYPE, CPPTYPE, TYPE_METHOD, CPPTYPE_METHOD)     \
  case FieldDescriptor::TYPE_##TYPE: {                                        \
    const CPPTYPE value =                                                     \
        field->is_repeated()                                                  \
            ? message_reflection->GetRepeated##CPPTYPE_METHOD(message, field, \
                                                              j)              \
            : message_reflection->Get##CPPTYPE_METHOD(message, field);        \
    target = WireFormatLite::Write##TYPE_METHOD##ToArray(field->number(),     \
                                                         value, target);      \
    break;                                                                    \
  }

      HANDLE_PRIMITIVE_TYPE(INT32, int32_t, Int32, Int32)
      HANDLE_PRIMITIVE_TYPE(INT64, int64_t, Int64, Int64)
      HANDLE_PRIMITIVE_TYPE(SINT32, int32_t, SInt32, Int32)
      HANDLE_PRIMITIVE_TYPE(SINT64, int64_t, SInt64, Int64)
      HANDLE_PRIMITIVE_TYPE(UINT32, uint32_t, UInt32, UInt32)
      HANDLE_PRIMITIVE_TYPE(UINT64, uint64_t, UInt64, UInt64)

      HANDLE_PRIMITIVE_TYPE(FIXED32, uint32_t, Fixed32, UInt32)
      HANDLE_PRIMITIVE_TYPE(FIXED64, uint64_t, Fixed64, UInt64)
      HANDLE_PRIMITIVE_TYPE(SFIXED32, int32_t, SFixed32, Int32)
      HANDLE_PRIMITIVE_TYPE(SFIXED64, int64_t, SFixed64, Int64)

      HANDLE_PRIMITIVE_TYPE(FLOAT, float, Float, Float)
      HANDLE_PRIMITIVE_TYPE(DOUBLE, double, Double, Double)

      HANDLE_PRIMITIVE_TYPE(BOOL, bool, Bool, Bool)
#undef HANDLE_PRIMITIVE_TYPE

      case FieldDescriptor::TYPE_GROUP: {
        auto* msg = get_message_from_field(field, j);
        target = WireFormatLite::InternalWriteGroup(field->number(), *msg,
                                                    target, stream);
      } break;

      case FieldDescriptor::TYPE_MESSAGE: {
        auto* msg = get_message_from_field(field, j);
        target = WireFormatLite::InternalWriteMessage(
            field->number(), *msg, msg->GetCachedSize(), target, stream);
      } break;

      case FieldDescriptor::TYPE_ENUM: {
        const EnumValueDescriptor* value =
            field->is_repeated()
                ? message_reflection->GetRepeatedEnum(message, field, j)
                : message_reflection->GetEnum(message, field);
        target = WireFormatLite::WriteEnumToArray(field->number(),
                                                  value->number(), target);
        break;
      }

      // Handle strings separately so that we can get string references
      // instead of copying.
      case FieldDescriptor::TYPE_STRING: {
        bool strict_utf8_check = field->requires_utf8_validation();
        std::string scratch;
        const std::string& value =
            field->is_repeated()
                ? message_reflection->GetRepeatedStringReference(message, field,
                                                                 j, &scratch)
                : message_reflection->GetStringReference(message, field,
                                                         &scratch);
        if (strict_utf8_check) {
          WireFormatLite::VerifyUtf8String(value.data(), value.length(),
                                           WireFormatLite::SERIALIZE,
                                           field->full_name().c_str());
        } else {
          VerifyUTF8StringNamedField(value.data(), value.length(), SERIALIZE,
                                     field->full_name().c_str());
        }
        target = stream->WriteString(field->number(), value, target);
        break;
      }

      case FieldDescriptor::TYPE_BYTES: {
        if (internal::cpp::EffectiveStringCType(field) == FieldOptions::CORD) {
          absl::Cord value = message_reflection->GetCord(message, field);
          target = stream->WriteString(field->number(), value, target);
          break;
        }
        std::string scratch;
        const std::string& value =
            field->is_repeated()
                ? message_reflection->GetRepeatedStringReference(message, field,
                                                                 j, &scratch)
                : message_reflection->GetStringReference(message, field,
                                                         &scratch);
        target = stream->WriteString(field->number(), value, target);
        break;
      }
    }
  }
  return target;
}

uint8_t* WireFormat::InternalSerializeMessageSetItem(
    const FieldDescriptor* field, const Message& message, uint8_t* target,
    io::EpsCopyOutputStream* stream) {
  const Reflection* message_reflection = message.GetReflection();

  target = stream->EnsureSpace(target);
  // Start group.
  target = io::CodedOutputStream::WriteTagToArray(
      WireFormatLite::kMessageSetItemStartTag, target);
  // Write type ID.
  target = WireFormatLite::WriteUInt32ToArray(
      WireFormatLite::kMessageSetTypeIdNumber, field->number(), target);
    // Write message.
    auto& msg = message_reflection->GetMessage(message, field);
    target = WireFormatLite::InternalWriteMessage(
        WireFormatLite::kMessageSetMessageNumber, msg, msg.GetCachedSize(),
        target, stream);
  // End group.
  target = stream->EnsureSpace(target);
  target = io::CodedOutputStream::WriteTagToArray(
      WireFormatLite::kMessageSetItemEndTag, target);
  return target;
}

// ===================================================================

size_t WireFormat::ByteSize(const Message& message) {
  const Descriptor* descriptor = message.GetDescriptor();
  const Reflection* message_reflection = message.GetReflection();

  size_t our_size = 0;

  std::vector<const FieldDescriptor*> fields;

  // Fields of map entry should always be serialized.
  if (descriptor->options().map_entry()) {
    for (int i = 0; i < descriptor->field_count(); i++) {
      fields.push_back(descriptor->field(i));
    }
  } else {
    message_reflection->ListFields(message, &fields);
  }

  for (const FieldDescriptor* field : fields) {
    our_size += FieldByteSize(field, message);
  }

  if (descriptor->options().message_set_wire_format()) {
    our_size += ComputeUnknownMessageSetItemsSize(
        message_reflection->GetUnknownFields(message));
  } else {
    our_size +=
        ComputeUnknownFieldsSize(message_reflection->GetUnknownFields(message));
  }

  return our_size;
}

size_t WireFormat::FieldByteSize(const FieldDescriptor* field,
                                 const Message& message) {
  const Reflection* message_reflection = message.GetReflection();

  if (field->is_extension() &&
      field->containing_type()->options().message_set_wire_format() &&
      field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
      !field->is_repeated()) {
    return MessageSetItemByteSize(field, message);
  }

  size_t count = 0;
  if (field->is_repeated()) {
    if (field->is_map()) {
      const MapFieldBase* map_field =
          message_reflection->GetMapData(message, field);
      if (map_field->IsMapValid()) {
        count = FromIntSize(map_field->size());
      } else {
        count = FromIntSize(message_reflection->FieldSize(message, field));
      }
    } else {
      count = FromIntSize(message_reflection->FieldSize(message, field));
    }
  } else if (field->containing_type()->options().map_entry()) {
    // Map entry fields always need to be serialized.
    count = 1;
  } else if (message_reflection->HasField(message, field)) {
    count = 1;
  }

  const size_t data_size = FieldDataOnlyByteSize(field, message);
  size_t our_size = data_size;
  if (field->is_packed()) {
    if (data_size > 0) {
      // Packed fields get serialized like a string, not their native type.
      // Technically this doesn't really matter; the size only changes if it's
      // a GROUP
      our_size += TagSize(field->number(), FieldDescriptor::TYPE_STRING);
      our_size += io::CodedOutputStream::VarintSize32(data_size);
    }
  } else {
    our_size += count * TagSize(field->number(), field->type());
  }
  return our_size;
}

size_t MapKeyDataOnlyByteSize(const FieldDescriptor* field,
                              const MapKey& value) {
  ABSL_DCHECK_EQ(FieldDescriptor::TypeToCppType(field->type()), value.type());
  switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE:
    case FieldDescriptor::TYPE_FLOAT:
    case FieldDescriptor::TYPE_GROUP:
    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_ENUM:
      ABSL_LOG(FATAL) << "Unsupported";
      return 0;
#define CASE_TYPE(FieldType, CamelFieldType, CamelCppType) \
  case FieldDescriptor::TYPE_##FieldType:                  \
    return WireFormatLite::CamelFieldType##Size(           \
        value.Get##CamelCppType##Value());

#define FIXED_CASE_TYPE(FieldType, CamelFieldType) \
  case FieldDescriptor::TYPE_##FieldType:          \
    return WireFormatLite::k##CamelFieldType##Size;

      CASE_TYPE(INT32, Int32, Int32);
      CASE_TYPE(INT64, Int64, Int64);
      CASE_TYPE(UINT32, UInt32, UInt32);
      CASE_TYPE(UINT64, UInt64, UInt64);
      CASE_TYPE(SINT32, SInt32, Int32);
      CASE_TYPE(SINT64, SInt64, Int64);
      CASE_TYPE(STRING, String, String);
      FIXED_CASE_TYPE(FIXED32, Fixed32);
      FIXED_CASE_TYPE(FIXED64, Fixed64);
      FIXED_CASE_TYPE(SFIXED32, SFixed32);
      FIXED_CASE_TYPE(SFIXED64, SFixed64);
      FIXED_CASE_TYPE(BOOL, Bool);

#undef CASE_TYPE
#undef FIXED_CASE_TYPE
  }
  ABSL_LOG(FATAL) << "Cannot get here";
  return 0;
}

static size_t MapValueRefDataOnlyByteSize(const FieldDescriptor* field,
                                          const MapValueConstRef& value) {
  switch (field->type()) {
    case FieldDescriptor::TYPE_GROUP:
      ABSL_LOG(FATAL) << "Unsupported";
      return 0;
#define CASE_TYPE(FieldType, CamelFieldType, CamelCppType) \
  case FieldDescriptor::TYPE_##FieldType:                  \
    return WireFormatLite::CamelFieldType##Size(           \
        value.Get##CamelCppType##Value());

#define FIXED_CASE_TYPE(FieldType, CamelFieldType) \
  case FieldDescriptor::TYPE_##FieldType:          \
    return WireFormatLite::k##CamelFieldType##Size;

      CASE_TYPE(INT32, Int32, Int32);
      CASE_TYPE(INT64, Int64, Int64);
      CASE_TYPE(UINT32, UInt32, UInt32);
      CASE_TYPE(UINT64, UInt64, UInt64);
      CASE_TYPE(SINT32, SInt32, Int32);
      CASE_TYPE(SINT64, SInt64, Int64);
      CASE_TYPE(STRING, String, String);
      CASE_TYPE(BYTES, Bytes, String);
      CASE_TYPE(ENUM, Enum, Enum);
      CASE_TYPE(MESSAGE, Message, Message);
      FIXED_CASE_TYPE(FIXED32, Fixed32);
      FIXED_CASE_TYPE(FIXED64, Fixed64);
      FIXED_CASE_TYPE(SFIXED32, SFixed32);
      FIXED_CASE_TYPE(SFIXED64, SFixed64);
      FIXED_CASE_TYPE(DOUBLE, Double);
      FIXED_CASE_TYPE(FLOAT, Float);
      FIXED_CASE_TYPE(BOOL, Bool);

#undef CASE_TYPE
#undef FIXED_CASE_TYPE
  }
  ABSL_LOG(FATAL) << "Cannot get here";
  return 0;
}

size_t WireFormat::FieldDataOnlyByteSize(const FieldDescriptor* field,
                                         const Message& message) {
  const Reflection* message_reflection = message.GetReflection();

  size_t data_size = 0;

  if (field->is_map()) {
    const MapFieldBase* map_field =
        message_reflection->GetMapData(message, field);
    if (map_field->IsMapValid()) {
      MapIterator iter(const_cast<Message*>(&message), field);
      MapIterator end(const_cast<Message*>(&message), field);
      const FieldDescriptor* key_field = field->message_type()->field(0);
      const FieldDescriptor* value_field = field->message_type()->field(1);
      for (map_field->MapBegin(&iter), map_field->MapEnd(&end); iter != end;
           ++iter) {
        size_t size = kMapEntryTagByteSize;
        size += MapKeyDataOnlyByteSize(key_field, iter.GetKey());
        size += MapValueRefDataOnlyByteSize(value_field, iter.GetValueRef());
        data_size += WireFormatLite::LengthDelimitedSize(size);
      }
      return data_size;
    }
  }

  size_t count = 0;
  if (field->is_repeated()) {
    count =
        internal::FromIntSize(message_reflection->FieldSize(message, field));
  } else if (field->containing_type()->options().map_entry()) {
    // Map entry fields always need to be serialized.
    count = 1;
  } else if (message_reflection->HasField(message, field)) {
    count = 1;
  }

  switch (field->type()) {
#define HANDLE_TYPE(TYPE, TYPE_METHOD, CPPTYPE_METHOD)                      \
  case FieldDescriptor::TYPE_##TYPE:                                        \
    if (field->is_repeated()) {                                             \
      for (size_t j = 0; j < count; j++) {                                  \
        data_size += WireFormatLite::TYPE_METHOD##Size(                     \
            message_reflection->GetRepeated##CPPTYPE_METHOD(message, field, \
                                                            j));            \
      }                                                                     \
    } else {                                                                \
      data_size += WireFormatLite::TYPE_METHOD##Size(                       \
          message_reflection->Get##CPPTYPE_METHOD(message, field));         \
    }                                                                       \
    break;

#define HANDLE_FIXED_TYPE(TYPE, TYPE_METHOD)                   \
  case FieldDescriptor::TYPE_##TYPE:                           \
    data_size += count * WireFormatLite::k##TYPE_METHOD##Size; \
    break;

    HANDLE_TYPE(INT32, Int32, Int32)
    HANDLE_TYPE(INT64, Int64, Int64)
    HANDLE_TYPE(SINT32, SInt32, Int32)
    HANDLE_TYPE(SINT64, SInt64, Int64)
    HANDLE_TYPE(UINT32, UInt32, UInt32)
    HANDLE_TYPE(UINT64, UInt64, UInt64)

    HANDLE_FIXED_TYPE(FIXED32, Fixed32)
    HANDLE_FIXED_TYPE(FIXED64, Fixed64)
    HANDLE_FIXED_TYPE(SFIXED32, SFixed32)
    HANDLE_FIXED_TYPE(SFIXED64, SFixed64)

    HANDLE_FIXED_TYPE(FLOAT, Float)
    HANDLE_FIXED_TYPE(DOUBLE, Double)

    HANDLE_FIXED_TYPE(BOOL, Bool)

    HANDLE_TYPE(GROUP, Group, Message)
    HANDLE_TYPE(MESSAGE, Message, Message)
#undef HANDLE_TYPE
#undef HANDLE_FIXED_TYPE

    case FieldDescriptor::TYPE_ENUM: {
      if (field->is_repeated()) {
        for (size_t j = 0; j < count; j++) {
          data_size += WireFormatLite::EnumSize(
              message_reflection->GetRepeatedEnum(message, field, j)->number());
        }
      } else {
        data_size += WireFormatLite::EnumSize(
            message_reflection->GetEnum(message, field)->number());
      }
      break;
    }

    // Handle strings separately so that we can get string references
    // instead of copying.
    case FieldDescriptor::TYPE_STRING:
    case FieldDescriptor::TYPE_BYTES: {
      if (internal::cpp::EffectiveStringCType(field) == FieldOptions::CORD) {
        for (size_t j = 0; j < count; j++) {
          absl::Cord value = message_reflection->GetCord(message, field);
          data_size += WireFormatLite::StringSize(value);
        }
        break;
      }
      for (size_t j = 0; j < count; j++) {
        std::string scratch;
        const std::string& value =
            field->is_repeated()
                ? message_reflection->GetRepeatedStringReference(message, field,
                                                                 j, &scratch)
                : message_reflection->GetStringReference(message, field,
                                                         &scratch);
        data_size += WireFormatLite::StringSize(value);
      }
      break;
    }
  }
  return data_size;
}

size_t WireFormat::MessageSetItemByteSize(const FieldDescriptor* field,
                                          const Message& message) {
  const Reflection* message_reflection = message.GetReflection();

  size_t our_size = WireFormatLite::kMessageSetItemTagsSize;

  // type_id
  our_size += io::CodedOutputStream::VarintSize32(field->number());

  // message
  const Message& sub_message = message_reflection->GetMessage(message, field);
  size_t message_size = sub_message.ByteSizeLong();

  our_size += io::CodedOutputStream::VarintSize32(message_size);
  our_size += message_size;

  return our_size;
}

// Compute the size of the UnknownFieldSet on the wire.
size_t ComputeUnknownFieldsSize(const InternalMetadata& metadata,
                                size_t total_size, CachedSize* cached_size) {
  total_size += WireFormat::ComputeUnknownFieldsSize(
      metadata.unknown_fields<UnknownFieldSet>(
          UnknownFieldSet::default_instance));
  cached_size->Set(ToCachedSize(total_size));
  return total_size;
}

}  // namespace internal
}  // namespace protobuf
}  // namespace google


// MARK: - BEGIN google_protobuf_port_undef.inc
// -*- c++ -*-
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// #undefs all macros defined in port_def.inc.  See comments in port_def.inc
// for more info.

#ifndef PROTOBUF_PORT_
#error "port_undef.inc must be included after port_def.inc"
#endif
#undef PROTOBUF_PORT_

#undef PROTOBUF_POISON_MEMORY_REGION
#undef PROTOBUF_UNPOISON_MEMORY_REGION
#undef PROTOBUF_BUILTIN_BSWAP16
#undef PROTOBUF_BUILTIN_BSWAP32
#undef PROTOBUF_BUILTIN_BSWAP64
#undef PROTOBUF_HAS_BUILTIN_MUL_OVERFLOW
#undef PROTOBUF_BUILTIN_ATOMIC
#undef PROTOBUF_GNUC_MIN
#undef PROTOBUF_CLANG_MIN
#undef PROTOBUF_MSC_VER_MIN
#undef PROTOBUF_CPLUSPLUS_MIN
#undef PROTOBUF_ABSL_MIN
#undef PROTOBUF_ALWAYS_INLINE
#undef PROTOBUF_ALWAYS_INLINE_CALL
#undef PROTOBUF_NDEBUG_INLINE
#undef PROTOBUF_MUSTTAIL
#undef PROTOBUF_TAILCALL
#undef PROTOBUF_COLD
#undef PROTOBUF_NOINLINE
#undef PROTOBUF_SECTION_VARIABLE
#undef PROTOBUF_IGNORE_DEPRECATION_START
#undef PROTOBUF_IGNORE_DEPRECATION_STOP
#undef PROTOBUF_RETURNS_NONNULL
#undef PROTOBUF_ATTRIBUTE_REINITIALIZES
#undef PROTOBUF_RTTI
#undef PROTOBUF_VERSION
#undef PROTOBUF_VERSION_SUFFIX
#undef PROTOBUF_MINIMUM_EDITION
#undef PROTOBUF_MAXIMUM_EDITION
#undef PROTOBUF_FIELD_OFFSET
#undef PROTOBUF_MIN_HEADER_VERSION_FOR_PROTOC
#undef PROTOBUF_MIN_PROTOC_VERSION
#undef PROTOBUF_PREDICT_TRUE
#undef PROTOBUF_PREDICT_FALSE
#undef PROTOBUF_FALLTHROUGH_INTENDED
#undef PROTOBUF_EXPORT
#undef PROTOC_EXPORT
#undef PROTOBUF_NODISCARD
#undef PROTOBUF_RESTRICT
#undef PROTOBUF_FORCE_COPY_IN_RELEASE
#undef PROTOBUF_FORCE_COPY_IN_SWAP
#undef PROTOBUF_FORCE_COPY_IN_MOVE
#undef PROTOBUF_FORCE_RESET_IN_CLEAR
#undef PROTOBUF_FUZZ_MESSAGE_SPACE_USED_LONG
#undef PROTOBUF_FORCE_COPY_DEFAULT_STRING
#undef PROTOBUF_FORCE_ALLOCATION_ON_CONSTRUCTION
#undef PROTOBUF_UNUSED
#undef PROTOBUF_ASSUME
#undef PROTOBUF_EXPORT_TEMPLATE_DECLARE
#undef PROTOBUF_EXPORT_TEMPLATE_DEFINE
#undef PROTOBUF_ALIGNAS
#undef PROTOBUF_FINAL
#undef PROTOBUF_THREAD_LOCAL
#undef PROTOBUF_LITTLE_ENDIAN
#undef PROTOBUF_BIG_ENDIAN
#undef PROTOBUF_CONSTINIT
#undef PROTOBUF_CONSTEXPR
#undef PROTOBUF_CONSTINIT_DEFAULT_INSTANCES
#undef PROTOBUF_ATTRIBUTE_WEAK
#undef PROTOBUF_HAVE_ATTRIBUTE_WEAK
#undef PROTOBUF_ATTRIBUTE_NO_DESTROY
#undef PROTOBUF_ATTRIBUTE_STANDALONE_DEBUG
#undef PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
#undef PROTOBUF_ATTRIBUTE_INIT_PRIORITY2
#undef PROTOBUF_PRAGMA_INIT_SEG
#undef PROTOBUF_ASAN
#undef PROTOBUF_MSAN
#undef PROTOBUF_TSAN
#undef PROTOBUF_TSAN_DECLARE_MEMBER
#undef PROTOBUF_TSAN_READ
#undef PROTOBUF_TSAN_WRITE
#undef PROTOBUF_USE_TABLE_PARSER_ON_REFLECTION
#undef PROTOBUF_BUILTIN_CONSTANT_P
#undef PROTOBUF_TC_PARAM_DECL
#undef PROTOBUF_EXCLUSIVE_LOCKS_REQUIRED
#undef PROTOBUF_LOCKS_EXCLUDED
#undef PROTOBUF_NO_THREAD_SAFETY_ANALYSIS
#undef PROTOBUF_GUARDED_BY
#undef PROTOBUF_DEBUG
#undef PROTO2_IS_OSS
#undef PROTOBUF_NO_THREADLOCAL

#ifdef PROTOBUF_FUTURE_BREAKING_CHANGES
#undef PROTOBUF_FUTURE_BREAKING_CHANGES
#undef PROTOBUF_FUTURE_REMOVE_CLEARED_API
#undef PROTOBUF_FUTURE_DESCRIPTOR_EXTENSION_DECL
#endif

// Restore macros that may have been #undef'd in port_def.inc.

#ifdef PROTOBUF_DID_UNDEF_PACKAGE
#pragma pop_macro("PACKAGE")
#undef PROTOBUF_DID_UNDEF_PACKAGE
#endif

#ifdef PROTOBUF_DID_UNDEF_PACKED
#pragma pop_macro("PACKED")
#undef PROTOBUF_DID_UNDEF_PACKED
#endif

#ifdef PROTOBUF_DID_UNDEF_DOMAIN
#pragma pop_macro("DOMAIN")
#undef PROTOBUF_DID_UNDEF_DOMAIN
#endif

#ifdef PROTOBUF_DID_UNDEF_LINUX
#pragma pop_macro("linux")
#endif

#ifdef _WIN32
#pragma pop_macro("CompareString")
#pragma pop_macro("CREATE_NEW")
#pragma pop_macro("DELETE")
#pragma pop_macro("DOUBLE_CLICK")
#pragma pop_macro("ERROR")
#pragma pop_macro("ERROR_BUSY")
#pragma pop_macro("ERROR_INSTALL_FAILED")
#pragma pop_macro("ERROR_NOT_FOUND")
#pragma pop_macro("GetClassName")
#pragma pop_macro("GetCurrentTime")
#pragma pop_macro("GetMessage")
#pragma pop_macro("GetObject")
#pragma pop_macro("IGNORE")
#pragma pop_macro("IN")
#pragma pop_macro("INPUT_KEYBOARD")
#pragma pop_macro("OUT")
#pragma pop_macro("OPTIONAL")
#pragma pop_macro("min")
#pragma pop_macro("max")
#pragma pop_macro("NEAR")
#pragma pop_macro("NO_DATA")
#pragma pop_macro("NO_ERROR")
#pragma pop_macro("REASON_UNKNOWN")
#pragma pop_macro("SERVICE_DISABLED")
#pragma pop_macro("SEVERITY_ERROR")
#pragma pop_macro("STRICT")
#pragma pop_macro("STATUS_PENDING")
#pragma pop_macro("timezone")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#endif

#ifdef __APPLE__
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma pop_macro("UID_MAX")
#pragma pop_macro("GID_MAX")
#pragma pop_macro("TYPE_BOOL")
#endif  // __APPLE__

#ifdef __FreeBSD__
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")
#pragma pop_macro("UID_MAX")
#pragma pop_macro("GID_MAX")
#endif  // __FreeBSD__

#if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
#pragma pop_macro("DEBUG")
#endif // defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

// Pop the warning(push) from port_def.inc
#ifdef _MSC_VER
#pragma warning(pop)
#endif


// MARK: - END google_protobuf_port_undef.inc


