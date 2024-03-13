// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google_protobuf_generated_message_tctable_gen.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_strings_str_cat.hpp>
#include "google_protobuf_descriptor.hpp"
#include "google_protobuf_descriptor.pb.hpp"
#include "google_protobuf_generated_message_tctable_decl.hpp"
#include "google_protobuf_generated_message_tctable_impl.hpp"
#include "google_protobuf_wire_format.hpp"

// Must come last:

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


namespace google {
namespace protobuf {
namespace internal {

namespace {

bool GetEnumValidationRange(const EnumDescriptor* enum_type, int16_t& start,
                            uint16_t& size) {
  ABSL_CHECK_GT(enum_type->value_count(), 0) << enum_type->DebugString();

  // Check if the enum values are a single, contiguous range.
  std::vector<int> enum_values;
  for (int i = 0, N = static_cast<int>(enum_type->value_count()); i < N; ++i) {
    enum_values.push_back(enum_type->value(i)->number());
  }
  auto values_begin = enum_values.begin();
  auto values_end = enum_values.end();
  std::sort(values_begin, values_end);
  enum_values.erase(std::unique(values_begin, values_end), values_end);

  if (std::numeric_limits<int16_t>::min() <= enum_values[0] &&
      enum_values[0] <= std::numeric_limits<int16_t>::max() &&
      enum_values.size() <= std::numeric_limits<uint16_t>::max() &&
      static_cast<int>(enum_values[0] + enum_values.size() - 1) ==
          enum_values.back()) {
    start = static_cast<int16_t>(enum_values[0]);
    size = static_cast<uint16_t>(enum_values.size());
    return true;
  } else {
    return false;
  }
}

enum class EnumRangeInfo {
  kNone,         // No contiguous range
  kContiguous,   // Has a contiguous range
  kContiguous0,  // Has a small contiguous range starting at 0
  kContiguous1,  // Has a small contiguous range starting at 1
};

// Returns enum validation range info, and sets `rmax_value` iff
// the returned range is a small range. `rmax_value` is guaranteed
// to remain unchanged if the enum range is not small.
EnumRangeInfo GetEnumRangeInfo(const FieldDescriptor* field,
                               uint8_t& rmax_value) {
  int16_t start;
  uint16_t size;
  if (!GetEnumValidationRange(field->enum_type(), start, size)) {
    return EnumRangeInfo::kNone;
  }
  int max_value = start + size - 1;
  if (max_value <= 127 && (start == 0 || start == 1)) {
    rmax_value = static_cast<uint8_t>(max_value);
    return start == 0 ? EnumRangeInfo::kContiguous0
                      : EnumRangeInfo::kContiguous1;
  }
  return EnumRangeInfo::kContiguous;
}

// options.lazy_opt might be on for fields that don't really support lazy, so we
// make sure we only use lazy rep for singular TYPE_MESSAGE fields.
// We can't trust the `lazy=true` annotation.
bool HasLazyRep(const FieldDescriptor* field,
                const TailCallTableInfo::PerFieldOptions options) {
  return field->type() == field->TYPE_MESSAGE && !field->is_repeated() &&
         options.lazy_opt != 0;
}

TailCallTableInfo::FastFieldInfo::Field MakeFastFieldEntry(
    const TailCallTableInfo::FieldEntryInfo& entry,
    const TailCallTableInfo::MessageOptions& message_options,
    const TailCallTableInfo::PerFieldOptions& options) {
  TailCallTableInfo::FastFieldInfo::Field info{};
#define PROTOBUF_PICK_FUNCTION(fn) \
  (field->number() < 16 ? TcParseFunction::fn##1 : TcParseFunction::fn##2)

#define PROTOBUF_PICK_SINGLE_FUNCTION(fn) PROTOBUF_PICK_FUNCTION(fn##S)

#define PROTOBUF_PICK_REPEATABLE_FUNCTION(fn)           \
  (field->is_repeated() ? PROTOBUF_PICK_FUNCTION(fn##R) \
                        : PROTOBUF_PICK_FUNCTION(fn##S))

#define PROTOBUF_PICK_PACKABLE_FUNCTION(fn)               \
  (field->is_packed()     ? PROTOBUF_PICK_FUNCTION(fn##P) \
   : field->is_repeated() ? PROTOBUF_PICK_FUNCTION(fn##R) \
                          : PROTOBUF_PICK_FUNCTION(fn##S))

#define PROTOBUF_PICK_STRING_FUNCTION(fn)                       \
  (field->options().ctype() == FieldOptions::CORD               \
       ? PROTOBUF_PICK_FUNCTION(fn##cS)                         \
   : options.is_string_inlined ? PROTOBUF_PICK_FUNCTION(fn##iS) \
                               : PROTOBUF_PICK_REPEATABLE_FUNCTION(fn))

  const FieldDescriptor* field = entry.field;
  info.aux_idx = static_cast<uint8_t>(entry.aux_idx);
  if (field->type() == FieldDescriptor::TYPE_BYTES ||
      field->type() == FieldDescriptor::TYPE_STRING) {
    if (options.is_string_inlined) {
      ABSL_CHECK(!field->is_repeated());
      info.aux_idx = static_cast<uint8_t>(entry.inlined_string_idx);
    }
  }

  TcParseFunction picked = TcParseFunction::kNone;
  switch (field->type()) {
    case FieldDescriptor::TYPE_BOOL:
      picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastV8);
      break;
    case FieldDescriptor::TYPE_INT32:
    case FieldDescriptor::TYPE_UINT32:
      picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastV32);
      break;
    case FieldDescriptor::TYPE_SINT32:
      picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastZ32);
      break;
    case FieldDescriptor::TYPE_INT64:
    case FieldDescriptor::TYPE_UINT64:
      picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastV64);
      break;
    case FieldDescriptor::TYPE_SINT64:
      picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastZ64);
      break;
    case FieldDescriptor::TYPE_FLOAT:
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_SFIXED32:
      picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastF32);
      break;
    case FieldDescriptor::TYPE_DOUBLE:
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_SFIXED64:
      picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastF64);
      break;
    case FieldDescriptor::TYPE_ENUM:
      if (cpp::HasPreservingUnknownEnumSemantics(field)) {
        picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastV32);
      } else {
        switch (GetEnumRangeInfo(field, info.aux_idx)) {
          case EnumRangeInfo::kNone:
            picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastEv);
            break;
          case EnumRangeInfo::kContiguous:
            picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastEr);
            break;
          case EnumRangeInfo::kContiguous0:
            picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastEr0);
            break;
          case EnumRangeInfo::kContiguous1:
            picked = PROTOBUF_PICK_PACKABLE_FUNCTION(kFastEr1);
            break;
        }
      }
      break;
    case FieldDescriptor::TYPE_BYTES:
      picked = PROTOBUF_PICK_STRING_FUNCTION(kFastB);
      break;
    case FieldDescriptor::TYPE_STRING:
      switch (internal::cpp::GetUtf8CheckMode(field, message_options.is_lite)) {
        case internal::cpp::Utf8CheckMode::kStrict:
          picked = PROTOBUF_PICK_STRING_FUNCTION(kFastU);
          break;
        case internal::cpp::Utf8CheckMode::kVerify:
          picked = PROTOBUF_PICK_STRING_FUNCTION(kFastS);
          break;
        case internal::cpp::Utf8CheckMode::kNone:
          picked = PROTOBUF_PICK_STRING_FUNCTION(kFastB);
          break;
      }
      break;
    case FieldDescriptor::TYPE_MESSAGE:
      picked =
          (HasLazyRep(field, options) ? PROTOBUF_PICK_SINGLE_FUNCTION(kFastMl)
           : options.use_direct_tcparser_table
               ? PROTOBUF_PICK_REPEATABLE_FUNCTION(kFastMt)
               : PROTOBUF_PICK_REPEATABLE_FUNCTION(kFastMd));
      break;
    case FieldDescriptor::TYPE_GROUP:
      picked = (options.use_direct_tcparser_table
                    ? PROTOBUF_PICK_REPEATABLE_FUNCTION(kFastGt)
                    : PROTOBUF_PICK_REPEATABLE_FUNCTION(kFastGd));
      break;
  }

  ABSL_CHECK(picked != TcParseFunction::kNone);
  info.func = picked;
  return info;

#undef PROTOBUF_PICK_FUNCTION
#undef PROTOBUF_PICK_SINGLE_FUNCTION
#undef PROTOBUF_PICK_REPEATABLE_FUNCTION
#undef PROTOBUF_PICK_PACKABLE_FUNCTION
#undef PROTOBUF_PICK_STRING_FUNCTION
}

bool IsFieldEligibleForFastParsing(
    const TailCallTableInfo::FieldEntryInfo& entry,
    const TailCallTableInfo::MessageOptions& message_options,
    const TailCallTableInfo::OptionProvider& option_provider) {
  const auto* field = entry.field;
  const auto options = option_provider.GetForField(field);
  ABSL_CHECK(!field->options().weak());
  // Map, oneof, weak, and split fields are not handled on the fast path.
  if (field->is_map() || field->real_containing_oneof() ||
      options.is_implicitly_weak || options.should_split) {
    return false;
  }

  if (HasLazyRep(field, options) && !message_options.uses_codegen) {
    // Can't use TDP on lazy fields if we can't do codegen.
    return false;
  }

  if (HasLazyRep(field, options) && options.lazy_opt == field_layout::kTvLazy) {
    // We only support eagerly verified lazy fields in the fast path.
    return false;
  }

  // We will check for a valid auxiliary index range later. However, we might
  // want to change the value we check for inlined string fields.
  int aux_idx = entry.aux_idx;

  switch (field->type()) {
      // Some bytes fields can be handled on fast path.
    case FieldDescriptor::TYPE_STRING:
    case FieldDescriptor::TYPE_BYTES:
      if (field->options().ctype() == FieldOptions::STRING) {
        // strings are fine...
      } else if (field->options().ctype() == FieldOptions::CORD) {
        // Cords are worth putting into the fast table, if they're not repeated
        if (field->is_repeated()) return false;
      } else {
        return false;
      }
      if (options.is_string_inlined) {
        ABSL_CHECK(!field->is_repeated());
        // For inlined strings, the donation state index is stored in the
        // `aux_idx` field of the fast parsing info. We need to check the range
        // of that value instead of the auxiliary index.
        aux_idx = entry.inlined_string_idx;
      }
      break;

    case FieldDescriptor::TYPE_ENUM: {
      uint8_t rmax_value;
      if (!message_options.uses_codegen &&
          GetEnumRangeInfo(field, rmax_value) == EnumRangeInfo::kNone) {
        // We can't use fast parsing for these entries because we can't specify
        // the validator.
        // TODO: Implement a fast parser for these enums.
        return false;
      }
      break;
    }

    default:
      break;
  }

  if (cpp::HasHasbit(field)) {
    // The tailcall parser can only update the first 32 hasbits. Fields with
    // has-bits beyond the first 32 are handled by mini parsing/fallback.
    ABSL_CHECK_GE(entry.hasbit_idx, 0) << field->DebugString();
    if (entry.hasbit_idx >= 32) return false;
  }

  // If the field needs auxiliary data, then the aux index is needed. This
  // must fit in a uint8_t.
  if (aux_idx > std::numeric_limits<uint8_t>::max()) {
    return false;
  }

  // The largest tag that can be read by the tailcall parser is two bytes
  // when varint-coded. This allows 14 bits for the numeric tag value:
  //   byte 0   byte 1
  //   1nnnnttt 0nnnnnnn
  //    ^^^^^^^  ^^^^^^^
  if (field->number() >= 1 << 11) return false;

  return true;
}

std::optional<uint32_t> GetEndGroupTag(const Descriptor* descriptor) {
  auto* parent = descriptor->containing_type();
  if (parent == nullptr) return absl::nullopt;
  for (int i = 0; i < parent->field_count(); ++i) {
    auto* field = parent->field(i);
    if (field->type() == field->TYPE_GROUP &&
        field->message_type() == descriptor) {
      return WireFormatLite::MakeTag(field->number(),
                                     WireFormatLite::WIRETYPE_END_GROUP);
    }
  }
  return absl::nullopt;
}

uint32_t RecodeTagForFastParsing(uint32_t tag) {
  ABSL_DCHECK_LE(tag, 0x3FFF);
  // Construct the varint-coded tag. If it is more than 7 bits, we need to
  // shift the high bits and add a continue bit.
  if (uint32_t hibits = tag & 0xFFFFFF80) {
    // hi = tag & ~0x7F
    // lo = tag & 0x7F
    // This shifts hi to the left by 1 to the next byte and sets the
    // continuation bit.
    tag = tag + hibits + 128;
  }
  return tag;
}

std::vector<TailCallTableInfo::FastFieldInfo> SplitFastFieldsForSize(
    std::optional<uint32_t> end_group_tag,
    const std::vector<TailCallTableInfo::FieldEntryInfo>& field_entries,
    int table_size_log2,
    const TailCallTableInfo::MessageOptions& message_options,
    const TailCallTableInfo::OptionProvider& option_provider) {
  std::vector<TailCallTableInfo::FastFieldInfo> result(1 << table_size_log2);
  const uint32_t idx_mask = static_cast<uint32_t>(result.size() - 1);
  const auto tag_to_idx = [&](uint32_t tag) {
    // The field index is determined by the low bits of the field number, where
    // the table size determines the width of the mask. The largest table
    // supported is 32 entries. The parse loop uses these bits directly, so that
    // the dispatch does not require arithmetic:
    //        byte 0   byte 1
    //   tag: 1nnnnttt 0nnnnnnn
    //        ^^^^^
    //         idx (table_size_log2=5)
    // This means that any field number that does not fit in the lower 4 bits
    // will always have the top bit of its table index asserted.
    return (tag >> 3) & idx_mask;
  };

  if (end_group_tag.has_value() && (*end_group_tag >> 14) == 0) {
    // Fits in 1 or 2 varint bytes.
    const uint32_t tag = RecodeTagForFastParsing(*end_group_tag);
    const uint32_t fast_idx = tag_to_idx(tag);

    TailCallTableInfo::FastFieldInfo& info = result[fast_idx];
    info.data = TailCallTableInfo::FastFieldInfo::NonField{
        *end_group_tag < 128 ? TcParseFunction::kFastEndG1
                             : TcParseFunction::kFastEndG2,
        static_cast<uint16_t>(tag),
        static_cast<uint16_t>(*end_group_tag),
    };
  }

  for (const auto& entry : field_entries) {
    if (!IsFieldEligibleForFastParsing(entry, message_options,
                                       option_provider)) {
      continue;
    }

    const auto* field = entry.field;
    const auto options = option_provider.GetForField(field);
    const uint32_t tag = RecodeTagForFastParsing(WireFormat::MakeTag(field));
    const uint32_t fast_idx = tag_to_idx(tag);

    TailCallTableInfo::FastFieldInfo& info = result[fast_idx];
    if (info.AsNonField() != nullptr) {
      // Null field means END_GROUP which is guaranteed to be present.
      continue;
    }
    if (auto* as_field = info.AsField()) {
      // This field entry is already filled. Skip if previous entry is more
      // likely present.
      const auto prev_options = option_provider.GetForField(as_field->field);
      if (prev_options.presence_probability >= options.presence_probability) {
        continue;
      }
    }

    // We reset the entry even if it had a field already.
    // Fill in this field's entry:
    auto& fast_field =
        info.data.emplace<TailCallTableInfo::FastFieldInfo::Field>(
            MakeFastFieldEntry(entry, message_options, options));
    fast_field.field = field;
    fast_field.coded_tag = tag;
    // If this field does not have presence, then it can set an out-of-bounds
    // bit (tailcall parsing uses a uint64_t for hasbits, but only stores 32).
    fast_field.hasbit_idx = cpp::HasHasbit(field) ? entry.hasbit_idx : 63;
  }
  return result;
}

// We only need field names for reporting UTF-8 parsing errors, so we only
// emit them for string fields with Utf8 transform specified.
bool NeedsFieldNameForTable(const FieldDescriptor* field, bool is_lite) {
  if (cpp::GetUtf8CheckMode(field, is_lite) == cpp::Utf8CheckMode::kNone)
    return false;
  return field->type() == FieldDescriptor::TYPE_STRING ||
         (field->is_map() && (field->message_type()->map_key()->type() ==
                                  FieldDescriptor::TYPE_STRING ||
                              field->message_type()->map_value()->type() ==
                                  FieldDescriptor::TYPE_STRING));
}

absl::string_view FieldNameForTable(
    const TailCallTableInfo::FieldEntryInfo& entry,
    const TailCallTableInfo::MessageOptions& message_options) {
  if (NeedsFieldNameForTable(entry.field, message_options.is_lite)) {
    return entry.field->name();
  }
  return "";
}

std::vector<uint8_t> GenerateFieldNames(
    const Descriptor* descriptor,
    const std::vector<TailCallTableInfo::FieldEntryInfo>& entries,
    const TailCallTableInfo::MessageOptions& message_options,
    const TailCallTableInfo::OptionProvider& option_provider) {
  static constexpr int kMaxNameLength = 255;
  std::vector<uint8_t> out;

  std::vector<absl::string_view> names;
  bool found_needed_name = false;
  for (const auto& entry : entries) {
    names.push_back(FieldNameForTable(entry, message_options));
    if (!names.back().empty()) found_needed_name = true;
  }

  // No names needed. Omit the whole table.
  if (!found_needed_name) {
    return out;
  }

  // First, we output the size of each string, as an unsigned byte. The first
  // string is the message name.
  int count = 1;
  out.push_back(std::min(static_cast<int>(descriptor->full_name().size()),
                         kMaxNameLength));
  for (auto field_name : names) {
    out.push_back(field_name.size());
    ++count;
  }
  while (count & 7) {  // align to an 8-byte boundary
    out.push_back(0);
    ++count;
  }
  // The message name is stored at the beginning of the string
  std::string message_name = descriptor->full_name();
  if (message_name.size() > kMaxNameLength) {
    static constexpr int kNameHalfLength = (kMaxNameLength - 3) / 2;
    message_name = absl::StrCat(
        message_name.substr(0, kNameHalfLength), "...",
        message_name.substr(message_name.size() - kNameHalfLength));
  }
  out.insert(out.end(), message_name.begin(), message_name.end());
  // Then we output the actual field names
  for (auto field_name : names) {
    out.insert(out.end(), field_name.begin(), field_name.end());
  }

  return out;
}

TailCallTableInfo::NumToEntryTable MakeNumToEntryTable(
    const std::vector<const FieldDescriptor*>& field_descriptors) {
  TailCallTableInfo::NumToEntryTable num_to_entry_table;
  num_to_entry_table.skipmap32 = static_cast<uint32_t>(-1);

  // skip_entry_block is the current block of SkipEntries that we're
  // appending to.  cur_block_first_fnum is the number of the first
  // field represented by the block.
  uint16_t field_entry_index = 0;
  uint16_t N = field_descriptors.size();
  // First, handle field numbers 1-32, which affect only the initial
  // skipmap32 and don't generate additional skip-entry blocks.
  for (; field_entry_index != N; ++field_entry_index) {
    auto* field_descriptor = field_descriptors[field_entry_index];
    if (field_descriptor->number() > 32) break;
    auto skipmap32_index = field_descriptor->number() - 1;
    num_to_entry_table.skipmap32 -= 1 << skipmap32_index;
  }
  // If all the field numbers were less than or equal to 32, we will have
  // no further entries to process, and we are already done.
  if (field_entry_index == N) return num_to_entry_table;

  TailCallTableInfo::SkipEntryBlock* block = nullptr;
  bool start_new_block = true;
  // To determine sparseness, track the field number corresponding to
  // the start of the most recent skip entry.
  uint32_t last_skip_entry_start = 0;
  for (; field_entry_index != N; ++field_entry_index) {
    auto* field_descriptor = field_descriptors[field_entry_index];
    uint32_t fnum = static_cast<uint32_t>(field_descriptor->number());
    ABSL_CHECK_GT(fnum, last_skip_entry_start);
    if (start_new_block == false) {
      // If the next field number is within 15 of the last_skip_entry_start, we
      // continue writing just to that entry.  If it's between 16 and 31 more,
      // then we just extend the current block by one. If it's more than 31
      // more, we have to add empty skip entries in order to continue using the
      // existing block.  Obviously it's just 32 more, it doesn't make sense to
      // start a whole new block, since new blocks mean having to write out
      // their starting field number, which is 32 bits, as well as the size of
      // the additional block, which is 16... while an empty SkipEntry16 only
      // costs 32 bits.  So if it was 48 more, it's a slight space win; we save
      // 16 bits, but probably at the cost of slower run time.  We're choosing
      // 96 for now.
      if (fnum - last_skip_entry_start > 96) start_new_block = true;
    }
    if (start_new_block) {
      num_to_entry_table.blocks.push_back({fnum});
      block = &num_to_entry_table.blocks.back();
      start_new_block = false;
    }

    auto skip_entry_num = (fnum - block->first_fnum) / 16;
    auto skip_entry_index = (fnum - block->first_fnum) % 16;
    while (skip_entry_num >= block->entries.size())
      block->entries.push_back({0xFFFF, field_entry_index});
    block->entries[skip_entry_num].skipmap -= 1 << (skip_entry_index);

    last_skip_entry_start = fnum - skip_entry_index;
  }
  return num_to_entry_table;
}

uint16_t MakeTypeCardForField(
    const FieldDescriptor* field,
    const TailCallTableInfo::MessageOptions& message_options,
    const TailCallTableInfo::PerFieldOptions& options) {
  uint16_t type_card;
  namespace fl = internal::field_layout;
  if (internal::cpp::HasHasbit(field)) {
    type_card = fl::kFcOptional;
  } else if (field->is_repeated()) {
    type_card = fl::kFcRepeated;
  } else if (field->real_containing_oneof()) {
    type_card = fl::kFcOneof;
  } else {
    type_card = fl::kFcSingular;
  }

  // The rest of the type uses convenience aliases:
  switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedDouble
                       : fl::kDouble;
      break;
    case FieldDescriptor::TYPE_FLOAT:
      type_card |= field->is_repeated() && field->is_packed() ? fl::kPackedFloat
                                                              : fl::kFloat;
      break;
    case FieldDescriptor::TYPE_FIXED32:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedFixed32
                       : fl::kFixed32;
      break;
    case FieldDescriptor::TYPE_SFIXED32:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedSFixed32
                       : fl::kSFixed32;
      break;
    case FieldDescriptor::TYPE_FIXED64:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedFixed64
                       : fl::kFixed64;
      break;
    case FieldDescriptor::TYPE_SFIXED64:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedSFixed64
                       : fl::kSFixed64;
      break;
    case FieldDescriptor::TYPE_BOOL:
      type_card |= field->is_repeated() && field->is_packed() ? fl::kPackedBool
                                                              : fl::kBool;
      break;
    case FieldDescriptor::TYPE_ENUM:
      if (internal::cpp::HasPreservingUnknownEnumSemantics(field)) {
        // No validation is required.
        type_card |= field->is_repeated() && field->is_packed()
                         ? fl::kPackedOpenEnum
                         : fl::kOpenEnum;
      } else {
        int16_t start;
        uint16_t size;
        if (GetEnumValidationRange(field->enum_type(), start, size)) {
          // Validation is done by range check (start/length in FieldAux).
          type_card |= field->is_repeated() && field->is_packed()
                           ? fl::kPackedEnumRange
                           : fl::kEnumRange;
        } else {
          // Validation uses the generated _IsValid function.
          type_card |= field->is_repeated() && field->is_packed()
                           ? fl::kPackedEnum
                           : fl::kEnum;
        }
      }
      break;
    case FieldDescriptor::TYPE_UINT32:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedUInt32
                       : fl::kUInt32;
      break;
    case FieldDescriptor::TYPE_SINT32:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedSInt32
                       : fl::kSInt32;
      break;
    case FieldDescriptor::TYPE_INT32:
      type_card |= field->is_repeated() && field->is_packed() ? fl::kPackedInt32
                                                              : fl::kInt32;
      break;
    case FieldDescriptor::TYPE_UINT64:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedUInt64
                       : fl::kUInt64;
      break;
    case FieldDescriptor::TYPE_SINT64:
      type_card |= field->is_repeated() && field->is_packed()
                       ? fl::kPackedSInt64
                       : fl::kSInt64;
      break;
    case FieldDescriptor::TYPE_INT64:
      type_card |= field->is_repeated() && field->is_packed() ? fl::kPackedInt64
                                                              : fl::kInt64;
      break;

    case FieldDescriptor::TYPE_BYTES:
      type_card |= fl::kBytes;
      break;
    case FieldDescriptor::TYPE_STRING: {
      switch (internal::cpp::GetUtf8CheckMode(field, message_options.is_lite)) {
        case internal::cpp::Utf8CheckMode::kStrict:
          type_card |= fl::kUtf8String;
          break;
        case internal::cpp::Utf8CheckMode::kVerify:
          type_card |= fl::kRawString;
          break;
        case internal::cpp::Utf8CheckMode::kNone:
          type_card |= fl::kBytes;
          break;
      }
      break;
    }

    case FieldDescriptor::TYPE_GROUP:
      type_card |= 0 | fl::kMessage | fl::kRepGroup;
      if (options.is_implicitly_weak) {
        type_card |= fl::kTvWeakPtr;
      } else if (options.use_direct_tcparser_table) {
        type_card |= fl::kTvTable;
      } else {
        type_card |= fl::kTvDefault;
      }
      break;
    case FieldDescriptor::TYPE_MESSAGE:
      if (field->is_map()) {
        type_card |= fl::kMap;
      } else {
        type_card |= fl::kMessage;
        if (HasLazyRep(field, options)) {
          ABSL_CHECK(options.lazy_opt == field_layout::kTvEager ||
                     options.lazy_opt == field_layout::kTvLazy);
          type_card |= +fl::kRepLazy | options.lazy_opt;
        } else {
          if (options.is_implicitly_weak) {
            type_card |= fl::kTvWeakPtr;
          } else if (options.use_direct_tcparser_table) {
            type_card |= fl::kTvTable;
          } else {
            type_card |= fl::kTvDefault;
          }
        }
      }
      break;
  }

  // Fill in extra information about string and bytes field representations.
  if (field->type() == FieldDescriptor::TYPE_BYTES ||
      field->type() == FieldDescriptor::TYPE_STRING) {
    switch (internal::cpp::EffectiveStringCType(field)) {
      case FieldOptions::CORD:
        // `Cord` is always used, even for repeated fields.
        type_card |= fl::kRepCord;
        break;
      case FieldOptions::STRING:
        if (field->is_repeated()) {
          // A repeated string field uses RepeatedPtrField<std::string>
          // (unless it has a ctype option; see above).
          type_card |= fl::kRepSString;
        } else {
          // Otherwise, non-repeated string fields use ArenaStringPtr.
          type_card |= fl::kRepAString;
        }
        break;
      default:
        PROTOBUF_ASSUME(false);
    }
  }

  if (options.should_split) {
    type_card |= fl::kSplitTrue;
  }

  return type_card;
}

}  // namespace

TailCallTableInfo::TailCallTableInfo(
    const Descriptor* descriptor,
    const std::vector<const FieldDescriptor*>& ordered_fields,
    const MessageOptions& message_options,
    const OptionProvider& option_provider,
    const std::vector<int>& has_bit_indices,
    const std::vector<int>& inlined_string_indices) {
  ABSL_DCHECK(std::is_sorted(ordered_fields.begin(), ordered_fields.end(),
                             [](const auto* lhs, const auto* rhs) {
                               return lhs->number() < rhs->number();
                             }));
  // If this message has any inlined string fields, store the donation state
  // offset in the first auxiliary entry, which is kInlinedStringAuxIdx.
  if (!inlined_string_indices.empty()) {
    aux_entries.resize(kInlinedStringAuxIdx + 1);  // Allocate our slot
    aux_entries[kInlinedStringAuxIdx] = {kInlinedStringDonatedOffset};
  }

  // If this message is split, store the split pointer offset in the second
  // and third auxiliary entries, which are kSplitOffsetAuxIdx and
  // kSplitSizeAuxIdx.
  for (auto* field : ordered_fields) {
    if (option_provider.GetForField(field).should_split) {
      static_assert(kSplitOffsetAuxIdx + 1 == kSplitSizeAuxIdx, "");
      aux_entries.resize(kSplitSizeAuxIdx + 1);  // Allocate our 2 slots
      aux_entries[kSplitOffsetAuxIdx] = {kSplitOffset};
      aux_entries[kSplitSizeAuxIdx] = {kSplitSizeof};
      break;
    }
  }

  // Fill in mini table entries.
  for (const FieldDescriptor* field : ordered_fields) {
    auto options = option_provider.GetForField(field);
    field_entries.push_back(
        {field, internal::cpp ::HasHasbit(field)
                    ? has_bit_indices[static_cast<size_t>(field->index())]
                    : -1});
    auto& entry = field_entries.back();
    entry.type_card = MakeTypeCardForField(field, message_options, options);

    if (field->type() == FieldDescriptor::TYPE_MESSAGE ||
        field->type() == FieldDescriptor::TYPE_GROUP) {
      // Message-typed fields have a FieldAux with the default instance pointer.
      if (field->is_map()) {
        field_entries.back().aux_idx = aux_entries.size();
        aux_entries.push_back({kMapAuxInfo, {field}});
        if (message_options.uses_codegen) {
          // If we don't use codegen we can't add these.
          auto* map_value = field->message_type()->map_value();
          if (auto* sub = map_value->message_type()) {
            aux_entries.push_back({kCreateInArena});
            aux_entries.back().desc = sub;
          } else if (map_value->type() == FieldDescriptor::TYPE_ENUM &&
                     !cpp::HasPreservingUnknownEnumSemantics(map_value)) {
            aux_entries.push_back({kEnumValidator, {map_value}});
          }
        }
      } else if (HasLazyRep(field, options)) {
        if (message_options.uses_codegen) {
          field_entries.back().aux_idx = aux_entries.size();
          aux_entries.push_back({kSubMessage, {field}});
          if (options.lazy_opt == field_layout::kTvEager) {
            aux_entries.push_back({kMessageVerifyFunc, {field}});
          } else {
            aux_entries.push_back({kNothing});
          }
        } else {
          field_entries.back().aux_idx =
              TcParseTableBase::FieldEntry::kNoAuxIdx;
        }
      } else {
        field_entries.back().aux_idx = aux_entries.size();
        aux_entries.push_back({options.is_implicitly_weak ? kSubMessageWeak
                               : options.use_direct_tcparser_table
                                   ? kSubTable
                                   : kSubMessage,
                               {field}});
      }
    } else if (field->type() == FieldDescriptor::TYPE_ENUM &&
               !cpp::HasPreservingUnknownEnumSemantics(field)) {
      // Enum fields which preserve unknown values (proto3 behavior) are
      // effectively int32 fields with respect to parsing -- i.e., the value
      // does not need to be validated at parse time.
      //
      // Enum fields which do not preserve unknown values (proto2 behavior) use
      // a FieldAux to store validation information. If the enum values are
      // sequential (and within a range we can represent), then the FieldAux
      // entry represents the range using the minimum value (which must fit in
      // an int16_t) and count (a uint16_t). Otherwise, the entry holds a
      // pointer to the generated Name_IsValid function.

      entry.aux_idx = aux_entries.size();
      aux_entries.push_back({});
      auto& aux_entry = aux_entries.back();

      if (GetEnumValidationRange(field->enum_type(), aux_entry.enum_range.start,
                                 aux_entry.enum_range.size)) {
        aux_entry.type = kEnumRange;
      } else {
        aux_entry.type = kEnumValidator;
        aux_entry.field = field;
      }

    } else if ((field->type() == FieldDescriptor::TYPE_STRING ||
                field->type() == FieldDescriptor::TYPE_BYTES) &&
               options.is_string_inlined) {
      ABSL_CHECK(!field->is_repeated());
      // Inlined strings have an extra marker to represent their donation state.
      int idx = inlined_string_indices[static_cast<size_t>(field->index())];
      // For mini parsing, the donation state index is stored as an `offset`
      // auxiliary entry.
      entry.aux_idx = aux_entries.size();
      aux_entries.push_back({kNumericOffset});
      aux_entries.back().offset = idx;
      // For fast table parsing, the donation state index is stored instead of
      // the aux_idx (this will limit the range to 8 bits).
      entry.inlined_string_idx = idx;
    }
  }

  table_size_log2 = 0;  // fallback value
  int num_fast_fields = -1;
  auto end_group_tag = GetEndGroupTag(descriptor);
  for (int try_size_log2 : {0, 1, 2, 3, 4, 5}) {
    size_t try_size = 1 << try_size_log2;
    auto split_fields =
        SplitFastFieldsForSize(end_group_tag, field_entries, try_size_log2,
                               message_options, option_provider);
    ABSL_CHECK_EQ(split_fields.size(), try_size);
    int try_num_fast_fields = 0;
    for (const auto& info : split_fields) {
      if (info.is_empty()) continue;

      if (info.AsNonField() != nullptr) {
        ++try_num_fast_fields;
        continue;
      }

      auto* as_field = info.AsField();
      const auto option = option_provider.GetForField(as_field->field);
      // 0.05 was selected based on load tests where 0.1 and 0.01 were also
      // evaluated and worse.
      constexpr float kMinPresence = 0.05f;
      if (option.presence_probability >= kMinPresence) {
        ++try_num_fast_fields;
      }
    }
    // Use this size if (and only if) it covers more fields.
    if (try_num_fast_fields > num_fast_fields) {
      fast_path_fields = std::move(split_fields);
      table_size_log2 = try_size_log2;
      num_fast_fields = try_num_fast_fields;
    }
    // The largest table we allow has the same number of entries as the
    // message has fields, rounded up to the next power of 2 (e.g., a message
    // with 5 fields can have a fast table of size 8). A larger table *might*
    // cover more fields in certain cases, but a larger table in that case
    // would have mostly empty entries; so, we cap the size to avoid
    // pathologically sparse tables.
    if (end_group_tag.has_value()) {
      // If this message uses group encoding, the tables are sometimes very
      // sparse because the fields in the group avoid using the same field
      // numbering as the parent message (even though currently, the proto
      // compiler allows the overlap, and there is no possible conflict.)
      // As such, this test produces a false negative as far as whether the
      // large table will be worth it.  So we disable the test in this case.
    } else {
      if (try_size > ordered_fields.size()) {
        break;
      }
    }
  }

  num_to_entry_table = MakeNumToEntryTable(ordered_fields);
  ABSL_CHECK_EQ(field_entries.size(), ordered_fields.size());
  field_name_data = GenerateFieldNames(descriptor, field_entries,
                                       message_options, option_provider);
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


