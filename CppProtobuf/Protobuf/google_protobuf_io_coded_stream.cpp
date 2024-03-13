// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// This implementation is heavily optimized to make reads and writes
// of small values (especially varints) as fast as possible.  In
// particular, we optimize for the common case that a read or a write
// will not cross the end of the buffer, since we can avoid a lot
// of branching in this case.

#include "google_protobuf_io_coded_stream.hpp"

#include <limits.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_strings_cord.hpp>
#include <CppAbseil/absl_strings_internal_resize_uninitialized.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include "google_protobuf_arena.hpp"
#include "google_protobuf_io_zero_copy_stream.hpp"
#include "google_protobuf_io_zero_copy_stream_impl_lite.hpp"
#include "google_protobuf_port.hpp"


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


namespace google {
namespace protobuf {
namespace io {

namespace {

static const int kMaxVarintBytes = 10;
static const int kMaxVarint32Bytes = 5;

// When reading / writing Cords, if we have fewer than this many bytes we
// won't bother trying to avoid coping the contents.
static const int kMaxCordBytesToCopy = 512;

inline bool NextNonEmpty(ZeroCopyInputStream* input, const void** data,
                         int* size) {
  bool success;
  do {
    success = input->Next(data, size);
  } while (success && *size == 0);
  return success;
}

inline uint8_t* CopyCordToArray(const absl::Cord& cord, uint8_t* target) {
  for (absl::string_view sv : cord.Chunks()) {
    memcpy(target, sv.data(), sv.size());
    target += sv.size();
  }
  return target;
}

}  // namespace

// CodedInputStream ==================================================

CodedInputStream::~CodedInputStream() {
  if (input_ != NULL) {
    BackUpInputToCurrentPosition();
  }
}

// Static.
int CodedInputStream::default_recursion_limit_ = 100;


void CodedInputStream::BackUpInputToCurrentPosition() {
  int backup_bytes = BufferSize() + buffer_size_after_limit_ + overflow_bytes_;
  if (backup_bytes > 0) {
    input_->BackUp(backup_bytes);

    // total_bytes_read_ doesn't include overflow_bytes_.
    total_bytes_read_ -= BufferSize() + buffer_size_after_limit_;
    buffer_end_ = buffer_;
    buffer_size_after_limit_ = 0;
    overflow_bytes_ = 0;
  }
}

inline void CodedInputStream::RecomputeBufferLimits() {
  buffer_end_ += buffer_size_after_limit_;
  int closest_limit = std::min(current_limit_, total_bytes_limit_);
  if (closest_limit < total_bytes_read_) {
    // The limit position is in the current buffer.  We must adjust
    // the buffer size accordingly.
    buffer_size_after_limit_ = total_bytes_read_ - closest_limit;
    buffer_end_ -= buffer_size_after_limit_;
  } else {
    buffer_size_after_limit_ = 0;
  }
}

CodedInputStream::Limit CodedInputStream::PushLimit(int byte_limit) {
  // Current position relative to the beginning of the stream.
  int current_position = CurrentPosition();

  Limit old_limit = current_limit_;

  // security: byte_limit is possibly evil, so check for negative values
  // and overflow. Also check that the new requested limit is before the
  // previous limit; otherwise we continue to enforce the previous limit.
  if (PROTOBUF_PREDICT_TRUE(byte_limit >= 0 &&
                            byte_limit <= INT_MAX - current_position &&
                            byte_limit < current_limit_ - current_position)) {
    current_limit_ = current_position + byte_limit;
    RecomputeBufferLimits();
  }

  return old_limit;
}

void CodedInputStream::PopLimit(Limit limit) {
  // The limit passed in is actually the *old* limit, which we returned from
  // PushLimit().
  current_limit_ = limit;
  RecomputeBufferLimits();

  // We may no longer be at a legitimate message end.  ReadTag() needs to be
  // called again to find out.
  legitimate_message_end_ = false;
}

std::pair<CodedInputStream::Limit, int>
CodedInputStream::IncrementRecursionDepthAndPushLimit(int byte_limit) {
  return std::make_pair(PushLimit(byte_limit), --recursion_budget_);
}

CodedInputStream::Limit CodedInputStream::ReadLengthAndPushLimit() {
  uint32_t length;
  return PushLimit(ReadVarint32(&length) ? length : 0);
}

bool CodedInputStream::DecrementRecursionDepthAndPopLimit(Limit limit) {
  bool result = ConsumedEntireMessage();
  PopLimit(limit);
  ABSL_DCHECK_LT(recursion_budget_, recursion_limit_);
  ++recursion_budget_;
  return result;
}

bool CodedInputStream::CheckEntireMessageConsumedAndPopLimit(Limit limit) {
  bool result = ConsumedEntireMessage();
  PopLimit(limit);
  return result;
}

int CodedInputStream::BytesUntilLimit() const {
  if (current_limit_ == INT_MAX) return -1;
  int current_position = CurrentPosition();

  return current_limit_ - current_position;
}

void CodedInputStream::SetTotalBytesLimit(int total_bytes_limit) {
  // Make sure the limit isn't already past, since this could confuse other
  // code.
  int current_position = CurrentPosition();
  total_bytes_limit_ = std::max(current_position, total_bytes_limit);
  RecomputeBufferLimits();
}

int CodedInputStream::BytesUntilTotalBytesLimit() const {
  if (total_bytes_limit_ == INT_MAX) return -1;
  return total_bytes_limit_ - CurrentPosition();
}

void CodedInputStream::PrintTotalBytesLimitError() {
  ABSL_LOG(ERROR) << "A protocol message was rejected because it was too "
                     "big (more than "
                  << total_bytes_limit_
                  << " bytes).  To increase the limit (or to disable these "
                     "warnings), see CodedInputStream::SetTotalBytesLimit() "
                     "in third_party/protobuf/io/coded_stream.h.";
}

bool CodedInputStream::SkipFallback(int count, int original_buffer_size) {
  if (buffer_size_after_limit_ > 0) {
    // We hit a limit inside this buffer.  Advance to the limit and fail.
    Advance(original_buffer_size);
    return false;
  }

  count -= original_buffer_size;
  buffer_ = NULL;
  buffer_end_ = buffer_;

  // Make sure this skip doesn't try to skip past the current limit.
  int closest_limit = std::min(current_limit_, total_bytes_limit_);
  int bytes_until_limit = closest_limit - total_bytes_read_;
  if (bytes_until_limit < count) {
    // We hit the limit.  Skip up to it then fail.
    if (bytes_until_limit > 0) {
      total_bytes_read_ = closest_limit;
      input_->Skip(bytes_until_limit);
    }
    return false;
  }

  if (!input_->Skip(count)) {
    total_bytes_read_ = input_->ByteCount();
    return false;
  }
  total_bytes_read_ += count;
  return true;
}

bool CodedInputStream::GetDirectBufferPointer(const void** data, int* size) {
  if (BufferSize() == 0 && !Refresh()) return false;

  *data = buffer_;
  *size = BufferSize();
  return true;
}

bool CodedInputStream::ReadRaw(void* buffer, int size) {
  int current_buffer_size;
  while ((current_buffer_size = BufferSize()) < size) {
    // Reading past end of buffer.  Copy what we have, then refresh.
    memcpy(buffer, buffer_, current_buffer_size);
    buffer = reinterpret_cast<uint8_t*>(buffer) + current_buffer_size;
    size -= current_buffer_size;
    Advance(current_buffer_size);
    if (!Refresh()) return false;
  }

  memcpy(buffer, buffer_, size);
  Advance(size);

  return true;
}

bool CodedInputStream::ReadString(std::string* buffer, int size) {
  if (size < 0) return false;  // security: size is often user-supplied

  if (BufferSize() >= size) {
    absl::strings_internal::STLStringResizeUninitialized(buffer, size);
    std::pair<char*, bool> z = as_string_data(buffer);
    if (z.second) {
      // Oddly enough, memcpy() requires its first two args to be non-NULL even
      // if we copy 0 bytes.  So, we have ensured that z.first is non-NULL here.
      ABSL_DCHECK(z.first != NULL);
      memcpy(z.first, buffer_, size);
      Advance(size);
    }
    return true;
  }

  return ReadStringFallback(buffer, size);
}

bool CodedInputStream::ReadStringFallback(std::string* buffer, int size) {
  if (!buffer->empty()) {
    buffer->clear();
  }

  int closest_limit = std::min(current_limit_, total_bytes_limit_);
  if (closest_limit != INT_MAX) {
    int bytes_to_limit = closest_limit - CurrentPosition();
    if (bytes_to_limit > 0 && size > 0 && size <= bytes_to_limit) {
      buffer->reserve(size);
    }
  }

  int current_buffer_size;
  while ((current_buffer_size = BufferSize()) < size) {
    // Some STL implementations "helpfully" crash on buffer->append(NULL, 0).
    if (current_buffer_size != 0) {
      // Note:  string1.append(string2) is O(string2.size()) (as opposed to
      //   O(string1.size() + string2.size()), which would be bad).
      buffer->append(reinterpret_cast<const char*>(buffer_),
                     current_buffer_size);
    }
    size -= current_buffer_size;
    Advance(current_buffer_size);
    if (!Refresh()) return false;
  }

  buffer->append(reinterpret_cast<const char*>(buffer_), size);
  Advance(size);

  return true;
}

bool CodedInputStream::ReadCord(absl::Cord* output, int size) {
  ABSL_DCHECK_NE(output, nullptr);

  // security: size is often user-supplied
  if (size < 0) {
    output->Clear();
    return false;  // security: size is often user-supplied
  }

  // Grab whatever is in the current output if `size` is relatively small,
  // or if we are not sourcing data from an input stream.
  if (input_ == nullptr || size < kMaxCordBytesToCopy) {
    // Just copy the current buffer into the output rather than backing up.
    absl::string_view buffer(reinterpret_cast<const char*>(buffer_),
                             static_cast<size_t>(std::min(size, BufferSize())));
    *output = buffer;
    Advance(static_cast<int>(buffer.size()));
    size -= static_cast<int>(buffer.size());
    if (size == 0) return true;
    if (input_ == nullptr || buffer_size_after_limit_ + overflow_bytes_ > 0) {
      // We hit a limit.
      return false;
    }
  } else {
    output->Clear();
    BackUpInputToCurrentPosition();
  }

  // Make sure to not cross a limit set by PushLimit() or SetTotalBytesLimit().
  const int closest_limit = std::min(current_limit_, total_bytes_limit_);
  const int available = closest_limit - total_bytes_read_;
  if (PROTOBUF_PREDICT_FALSE(size > available)) {
    total_bytes_read_ = closest_limit;
    input_->ReadCord(output, available);
    return false;
  }
  total_bytes_read_ += size;
  return input_->ReadCord(output, size);
}


bool CodedInputStream::ReadLittleEndian32Fallback(uint32_t* value) {
  uint8_t bytes[sizeof(*value)];

  const uint8_t* ptr;
  if (BufferSize() >= static_cast<int64_t>(sizeof(*value))) {
    // Fast path:  Enough bytes in the buffer to read directly.
    ptr = buffer_;
    Advance(sizeof(*value));
  } else {
    // Slow path:  Had to read past the end of the buffer.
    if (!ReadRaw(bytes, sizeof(*value))) return false;
    ptr = bytes;
  }
  ReadLittleEndian32FromArray(ptr, value);
  return true;
}

bool CodedInputStream::ReadLittleEndian64Fallback(uint64_t* value) {
  uint8_t bytes[sizeof(*value)];

  const uint8_t* ptr;
  if (BufferSize() >= static_cast<int64_t>(sizeof(*value))) {
    // Fast path:  Enough bytes in the buffer to read directly.
    ptr = buffer_;
    Advance(sizeof(*value));
  } else {
    // Slow path:  Had to read past the end of the buffer.
    if (!ReadRaw(bytes, sizeof(*value))) return false;
    ptr = bytes;
  }
  ReadLittleEndian64FromArray(ptr, value);
  return true;
}

namespace {

// Decodes varint64 with known size, N, and returns next pointer. Knowing N at
// compile time, compiler can generate optimal code. For example, instead of
// subtracting 0x80 at each iteration, it subtracts properly shifted mask once.
template <size_t N>
const uint8_t* DecodeVarint64KnownSize(const uint8_t* buffer, uint64_t* value) {
  ABSL_DCHECK_GT(N, 0);
  uint64_t result = static_cast<uint64_t>(buffer[N - 1]) << (7 * (N - 1));
  for (size_t i = 0, offset = 0; i < N - 1; i++, offset += 7) {
    result += static_cast<uint64_t>(buffer[i] - 0x80) << offset;
  }
  *value = result;
  return buffer + N;
}

// Read a varint from the given buffer, write it to *value, and return a pair.
// The first part of the pair is true iff the read was successful.  The second
// part is buffer + (number of bytes read).  This function is always inlined,
// so returning a pair is costless.
PROTOBUF_ALWAYS_INLINE
::std::pair<bool, const uint8_t*> ReadVarint32FromArray(uint32_t first_byte,
                                                      const uint8_t* buffer,
                                                      uint32_t* value);
inline ::std::pair<bool, const uint8_t*> ReadVarint32FromArray(
    uint32_t first_byte, const uint8_t* buffer, uint32_t* value) {
  // Fast path:  We have enough bytes left in the buffer to guarantee that
  // this read won't cross the end, so we can skip the checks.
  ABSL_DCHECK_EQ(*buffer, first_byte);
  ABSL_DCHECK_EQ(first_byte & 0x80, 0x80) << first_byte;
  const uint8_t* ptr = buffer;
  uint32_t b;
  uint32_t result = first_byte - 0x80;
  ++ptr;  // We just processed the first byte.  Move on to the second.
  b = *(ptr++);
  result += b << 7;
  if (!(b & 0x80)) goto done;
  result -= 0x80 << 7;
  b = *(ptr++);
  result += b << 14;
  if (!(b & 0x80)) goto done;
  result -= 0x80 << 14;
  b = *(ptr++);
  result += b << 21;
  if (!(b & 0x80)) goto done;
  result -= 0x80 << 21;
  b = *(ptr++);
  result += b << 28;
  if (!(b & 0x80)) goto done;
  // "result -= 0x80 << 28" is irrelevant.

  // If the input is larger than 32 bits, we still need to read it all
  // and discard the high-order bits.
  for (int i = 0; i < kMaxVarintBytes - kMaxVarint32Bytes; i++) {
    b = *(ptr++);
    if (!(b & 0x80)) goto done;
  }

  // We have overrun the maximum size of a varint (10 bytes).  Assume
  // the data is corrupt.
  return std::make_pair(false, ptr);

done:
  *value = result;
  return std::make_pair(true, ptr);
}

PROTOBUF_ALWAYS_INLINE::std::pair<bool, const uint8_t*> ReadVarint64FromArray(
    const uint8_t* buffer, uint64_t* value);
inline ::std::pair<bool, const uint8_t*> ReadVarint64FromArray(
    const uint8_t* buffer, uint64_t* value) {
  // Assumes varint64 is at least 2 bytes.
  ABSL_DCHECK_GE(buffer[0], 128);

  const uint8_t* next;
  if (buffer[1] < 128) {
    next = DecodeVarint64KnownSize<2>(buffer, value);
  } else if (buffer[2] < 128) {
    next = DecodeVarint64KnownSize<3>(buffer, value);
  } else if (buffer[3] < 128) {
    next = DecodeVarint64KnownSize<4>(buffer, value);
  } else if (buffer[4] < 128) {
    next = DecodeVarint64KnownSize<5>(buffer, value);
  } else if (buffer[5] < 128) {
    next = DecodeVarint64KnownSize<6>(buffer, value);
  } else if (buffer[6] < 128) {
    next = DecodeVarint64KnownSize<7>(buffer, value);
  } else if (buffer[7] < 128) {
    next = DecodeVarint64KnownSize<8>(buffer, value);
  } else if (buffer[8] < 128) {
    next = DecodeVarint64KnownSize<9>(buffer, value);
  } else if (buffer[9] < 128) {
    next = DecodeVarint64KnownSize<10>(buffer, value);
  } else {
    // We have overrun the maximum size of a varint (10 bytes). Assume
    // the data is corrupt.
    return std::make_pair(false, buffer + 11);
  }

  return std::make_pair(true, next);
}

}  // namespace

bool CodedInputStream::ReadVarint32Slow(uint32_t* value) {
  // Directly invoke ReadVarint64Fallback, since we already tried to optimize
  // for one-byte varints.
  std::pair<uint64_t, bool> p = ReadVarint64Fallback();
  *value = static_cast<uint32_t>(p.first);
  return p.second;
}

int64_t CodedInputStream::ReadVarint32Fallback(uint32_t first_byte_or_zero) {
  if (BufferSize() >= kMaxVarintBytes ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (buffer_end_ > buffer_ && !(buffer_end_[-1] & 0x80))) {
    ABSL_DCHECK_NE(first_byte_or_zero, 0)
        << "Caller should provide us with *buffer_ when buffer is non-empty";
    uint32_t temp;
    ::std::pair<bool, const uint8_t*> p =
        ReadVarint32FromArray(first_byte_or_zero, buffer_, &temp);
    if (!p.first) return -1;
    buffer_ = p.second;
    return temp;
  } else {
    // Really slow case: we will incur the cost of an extra function call here,
    // but moving this out of line reduces the size of this function, which
    // improves the common case. In micro benchmarks, this is worth about 10-15%
    uint32_t temp;
    return ReadVarint32Slow(&temp) ? static_cast<int64_t>(temp) : -1;
  }
}

int CodedInputStream::ReadVarintSizeAsIntSlow() {
  // Directly invoke ReadVarint64Fallback, since we already tried to optimize
  // for one-byte varints.
  std::pair<uint64_t, bool> p = ReadVarint64Fallback();
  if (!p.second || p.first > static_cast<uint64_t>(INT_MAX)) return -1;
  return p.first;
}

int CodedInputStream::ReadVarintSizeAsIntFallback() {
  if (BufferSize() >= kMaxVarintBytes ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (buffer_end_ > buffer_ && !(buffer_end_[-1] & 0x80))) {
    uint64_t temp;
    ::std::pair<bool, const uint8_t*> p = ReadVarint64FromArray(buffer_, &temp);
    if (!p.first || temp > static_cast<uint64_t>(INT_MAX)) return -1;
    buffer_ = p.second;
    return temp;
  } else {
    // Really slow case: we will incur the cost of an extra function call here,
    // but moving this out of line reduces the size of this function, which
    // improves the common case. In micro benchmarks, this is worth about 10-15%
    return ReadVarintSizeAsIntSlow();
  }
}

uint32_t CodedInputStream::ReadTagSlow() {
  if (buffer_ == buffer_end_) {
    // Call refresh.
    if (!Refresh()) {
      // Refresh failed.  Make sure that it failed due to EOF, not because
      // we hit total_bytes_limit_, which, unlike normal limits, is not a
      // valid place to end a message.
      int current_position = total_bytes_read_ - buffer_size_after_limit_;
      if (current_position >= total_bytes_limit_) {
        // Hit total_bytes_limit_.  But if we also hit the normal limit,
        // we're still OK.
        legitimate_message_end_ = current_limit_ == total_bytes_limit_;
      } else {
        legitimate_message_end_ = true;
      }
      return 0;
    }
  }

  // For the slow path, just do a 64-bit read. Try to optimize for one-byte tags
  // again, since we have now refreshed the buffer.
  uint64_t result = 0;
  if (!ReadVarint64(&result)) return 0;
  return static_cast<uint32_t>(result);
}

uint32_t CodedInputStream::ReadTagFallback(uint32_t first_byte_or_zero) {
  const int buf_size = BufferSize();
  if (buf_size >= kMaxVarintBytes ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (buf_size > 0 && !(buffer_end_[-1] & 0x80))) {
    ABSL_DCHECK_EQ(first_byte_or_zero, buffer_[0]);
    if (first_byte_or_zero == 0) {
      ++buffer_;
      return 0;
    }
    uint32_t tag;
    ::std::pair<bool, const uint8_t*> p =
        ReadVarint32FromArray(first_byte_or_zero, buffer_, &tag);
    if (!p.first) {
      return 0;
    }
    buffer_ = p.second;
    return tag;
  } else {
    // We are commonly at a limit when attempting to read tags. Try to quickly
    // detect this case without making another function call.
    if ((buf_size == 0) &&
        ((buffer_size_after_limit_ > 0) ||
         (total_bytes_read_ == current_limit_)) &&
        // Make sure that the limit we hit is not total_bytes_limit_, since
        // in that case we still need to call Refresh() so that it prints an
        // error.
        total_bytes_read_ - buffer_size_after_limit_ < total_bytes_limit_) {
      // We hit a byte limit.
      legitimate_message_end_ = true;
      return 0;
    }
    return ReadTagSlow();
  }
}

bool CodedInputStream::ReadVarint64Slow(uint64_t* value) {
  // Slow path:  This read might cross the end of the buffer, so we
  // need to check and refresh the buffer if and when it does.

  uint64_t result = 0;
  int count = 0;
  uint32_t b;

  do {
    if (count == kMaxVarintBytes) {
      *value = 0;
      return false;
    }
    while (buffer_ == buffer_end_) {
      if (!Refresh()) {
        *value = 0;
        return false;
      }
    }
    b = *buffer_;
    result |= static_cast<uint64_t>(b & 0x7F) << (7 * count);
    Advance(1);
    ++count;
  } while (b & 0x80);

  *value = result;
  return true;
}

std::pair<uint64_t, bool> CodedInputStream::ReadVarint64Fallback() {
  if (BufferSize() >= kMaxVarintBytes ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (buffer_end_ > buffer_ && !(buffer_end_[-1] & 0x80))) {
    uint64_t temp;
    ::std::pair<bool, const uint8_t*> p = ReadVarint64FromArray(buffer_, &temp);
    if (!p.first) {
      return std::make_pair(0, false);
    }
    buffer_ = p.second;
    return std::make_pair(temp, true);
  } else {
    uint64_t temp;
    bool success = ReadVarint64Slow(&temp);
    return std::make_pair(temp, success);
  }
}

bool CodedInputStream::Refresh() {
  ABSL_DCHECK_EQ(0, BufferSize());

  if (buffer_size_after_limit_ > 0 || overflow_bytes_ > 0 ||
      total_bytes_read_ == current_limit_) {
    // We've hit a limit.  Stop.
    int current_position = total_bytes_read_ - buffer_size_after_limit_;

    if (current_position >= total_bytes_limit_ &&
        total_bytes_limit_ != current_limit_) {
      // Hit total_bytes_limit_.
      PrintTotalBytesLimitError();
    }

    return false;
  }

  const void* void_buffer;
  int buffer_size;
  if (NextNonEmpty(input_, &void_buffer, &buffer_size)) {
    buffer_ = reinterpret_cast<const uint8_t*>(void_buffer);
    buffer_end_ = buffer_ + buffer_size;
    ABSL_CHECK_GE(buffer_size, 0);

    if (total_bytes_read_ <= INT_MAX - buffer_size) {
      total_bytes_read_ += buffer_size;
    } else {
      // Overflow.  Reset buffer_end_ to not include the bytes beyond INT_MAX.
      // We can't get that far anyway, because total_bytes_limit_ is guaranteed
      // to be less than it.  We need to keep track of the number of bytes
      // we discarded, though, so that we can call input_->BackUp() to back
      // up over them on destruction.

      // The following line is equivalent to:
      //   overflow_bytes_ = total_bytes_read_ + buffer_size - INT_MAX;
      // except that it avoids overflows.  Signed integer overflow has
      // undefined results according to the C standard.
      overflow_bytes_ = total_bytes_read_ - (INT_MAX - buffer_size);
      buffer_end_ -= overflow_bytes_;
      total_bytes_read_ = INT_MAX;
    }

    RecomputeBufferLimits();
    return true;
  } else {
    buffer_ = NULL;
    buffer_end_ = NULL;
    return false;
  }
}

// CodedOutputStream =================================================

void EpsCopyOutputStream::EnableAliasing(bool enabled) {
  aliasing_enabled_ = enabled && stream_->AllowsAliasing();
}

int64_t EpsCopyOutputStream::ByteCount(uint8_t* ptr) const {
  // Calculate the current offset relative to the end of the stream buffer.
  int delta = (end_ - ptr) + (buffer_end_ ? 0 : kSlopBytes);
  return stream_->ByteCount() - delta;
}

// Flushes what's written out to the underlying ZeroCopyOutputStream buffers.
// Returns the size remaining in the buffer and sets buffer_end_ to the start
// of the remaining buffer, ie. [buffer_end_, buffer_end_ + return value)
int EpsCopyOutputStream::Flush(uint8_t* ptr) {
  while (buffer_end_ && ptr > end_) {
    int overrun = ptr - end_;
    ABSL_DCHECK(!had_error_);
    ABSL_DCHECK(overrun <= kSlopBytes);  // NOLINT
    ptr = Next() + overrun;
    if (had_error_) return 0;
  }
  int s;
  if (buffer_end_) {
    std::memcpy(buffer_end_, buffer_, ptr - buffer_);
    buffer_end_ += ptr - buffer_;
    s = end_ - ptr;
  } else {
    // The stream is writing directly in the ZeroCopyOutputStream buffer.
    s = end_ + kSlopBytes - ptr;
    buffer_end_ = ptr;
  }
  ABSL_DCHECK(s >= 0);  // NOLINT
  return s;
}

uint8_t* EpsCopyOutputStream::Trim(uint8_t* ptr) {
  if (had_error_) return ptr;
  int s = Flush(ptr);
  stream_->BackUp(s);
  // Reset to initial state (expecting new buffer)
  buffer_end_ = end_ = buffer_;
  return buffer_;
}


uint8_t* EpsCopyOutputStream::FlushAndResetBuffer(uint8_t* ptr) {
  if (had_error_) return buffer_;
  int s = Flush(ptr);
  if (had_error_) return buffer_;
  return SetInitialBuffer(buffer_end_, s);
}

bool EpsCopyOutputStream::Skip(int count, uint8_t** pp) {
  if (count < 0) return false;
  if (had_error_) {
    *pp = buffer_;
    return false;
  }
  int size = Flush(*pp);
  if (had_error_) {
    *pp = buffer_;
    return false;
  }
  void* data = buffer_end_;
  while (count > size) {
    count -= size;
    if (!stream_->Next(&data, &size)) {
      *pp = Error();
      return false;
    }
  }
  *pp = SetInitialBuffer(static_cast<uint8_t*>(data) + count, size - count);
  return true;
}

bool EpsCopyOutputStream::GetDirectBufferPointer(void** data, int* size,
                                                 uint8_t** pp) {
  if (had_error_) {
    *pp = buffer_;
    return false;
  }
  *size = Flush(*pp);
  if (had_error_) {
    *pp = buffer_;
    return false;
  }
  *data = buffer_end_;
  while (*size == 0) {
    if (!stream_->Next(data, size)) {
      *pp = Error();
      return false;
    }
  }
  *pp = SetInitialBuffer(*data, *size);
  return true;
}

uint8_t* EpsCopyOutputStream::GetDirectBufferForNBytesAndAdvance(int size,
                                                               uint8_t** pp) {
  if (had_error_) {
    *pp = buffer_;
    return nullptr;
  }
  int s = Flush(*pp);
  if (had_error_) {
    *pp = buffer_;
    return nullptr;
  }
  if (s >= size) {
    auto res = buffer_end_;
    *pp = SetInitialBuffer(buffer_end_ + size, s - size);
    return res;
  } else {
    *pp = SetInitialBuffer(buffer_end_, s);
    return nullptr;
  }
}

uint8_t* EpsCopyOutputStream::Next() {
  ABSL_DCHECK(!had_error_);  // NOLINT
  if (PROTOBUF_PREDICT_FALSE(stream_ == nullptr)) return Error();
  if (buffer_end_) {
    // We're in the patch buffer and need to fill up the previous buffer.
    std::memcpy(buffer_end_, buffer_, end_ - buffer_);
    uint8_t* ptr;
    int size;
    do {
      void* data;
      if (PROTOBUF_PREDICT_FALSE(!stream_->Next(&data, &size))) {
        // Stream has an error, we use the patch buffer to continue to be
        // able to write.
        return Error();
      }
      ptr = static_cast<uint8_t*>(data);
    } while (size == 0);
    if (PROTOBUF_PREDICT_TRUE(size > kSlopBytes)) {
      std::memcpy(ptr, end_, kSlopBytes);
      end_ = ptr + size - kSlopBytes;
      buffer_end_ = nullptr;
      return ptr;
    } else {
      ABSL_DCHECK(size > 0);  // NOLINT
      // Buffer to small
      std::memmove(buffer_, end_, kSlopBytes);
      buffer_end_ = ptr;
      end_ = buffer_ + size;
      return buffer_;
    }
  } else {
    std::memcpy(buffer_, end_, kSlopBytes);
    buffer_end_ = end_;
    end_ = buffer_ + kSlopBytes;
    return buffer_;
  }
}

uint8_t* EpsCopyOutputStream::EnsureSpaceFallback(uint8_t* ptr) {
  do {
    if (PROTOBUF_PREDICT_FALSE(had_error_)) return buffer_;
    int overrun = ptr - end_;
    ABSL_DCHECK(overrun >= 0);           // NOLINT
    ABSL_DCHECK(overrun <= kSlopBytes);  // NOLINT
    ptr = Next() + overrun;
  } while (ptr >= end_);
  ABSL_DCHECK(ptr < end_);  // NOLINT
  return ptr;
}

uint8_t* EpsCopyOutputStream::WriteRawFallback(const void* data, int size,
                                             uint8_t* ptr) {
  int s = GetSize(ptr);
  while (s < size) {
    std::memcpy(ptr, data, s);
    size -= s;
    data = static_cast<const uint8_t*>(data) + s;
    ptr = EnsureSpaceFallback(ptr + s);
    s = GetSize(ptr);
  }
  std::memcpy(ptr, data, size);
  return ptr + size;
}

uint8_t* EpsCopyOutputStream::WriteAliasedRaw(const void* data, int size,
                                            uint8_t* ptr) {
  if (size < GetSize(ptr)
  ) {
    return WriteRaw(data, size, ptr);
  } else {
    ptr = Trim(ptr);
    if (stream_->WriteAliasedRaw(data, size)) return ptr;
    return Error();
  }
}

#ifndef PROTOBUF_LITTLE_ENDIAN
uint8_t* EpsCopyOutputStream::WriteRawLittleEndian32(const void* data, int size,
                                                   uint8_t* ptr) {
  auto p = static_cast<const uint8_t*>(data);
  auto end = p + size;
  while (end - p >= kSlopBytes) {
    ptr = EnsureSpace(ptr);
    uint32_t buffer[4];
    static_assert(sizeof(buffer) == kSlopBytes, "Buffer must be kSlopBytes");
    std::memcpy(buffer, p, kSlopBytes);
    p += kSlopBytes;
    for (auto x : buffer)
      ptr = CodedOutputStream::WriteLittleEndian32ToArray(x, ptr);
  }
  while (p < end) {
    ptr = EnsureSpace(ptr);
    uint32_t buffer;
    std::memcpy(&buffer, p, 4);
    p += 4;
    ptr = CodedOutputStream::WriteLittleEndian32ToArray(buffer, ptr);
  }
  return ptr;
}

uint8_t* EpsCopyOutputStream::WriteRawLittleEndian64(const void* data, int size,
                                                   uint8_t* ptr) {
  auto p = static_cast<const uint8_t*>(data);
  auto end = p + size;
  while (end - p >= kSlopBytes) {
    ptr = EnsureSpace(ptr);
    uint64_t buffer[2];
    static_assert(sizeof(buffer) == kSlopBytes, "Buffer must be kSlopBytes");
    std::memcpy(buffer, p, kSlopBytes);
    p += kSlopBytes;
    for (auto x : buffer)
      ptr = CodedOutputStream::WriteLittleEndian64ToArray(x, ptr);
  }
  while (p < end) {
    ptr = EnsureSpace(ptr);
    uint64_t buffer;
    std::memcpy(&buffer, p, 8);
    p += 8;
    ptr = CodedOutputStream::WriteLittleEndian64ToArray(buffer, ptr);
  }
  return ptr;
}
#endif

uint8_t* EpsCopyOutputStream::WriteCord(const absl::Cord& cord, uint8_t* ptr) {
  int s = GetSize(ptr);
  if (stream_ == nullptr) {
    if (static_cast<int64_t>(cord.size()) <= s) {
      // Just copy it to the current buffer.
      return CopyCordToArray(cord, ptr);
    } else {
      return Error();
    }
  } else if (static_cast<int64_t>(cord.size()) <= s &&
             static_cast<int64_t>(cord.size()) < kMaxCordBytesToCopy) {
    // Just copy it to the current buffer.
    return CopyCordToArray(cord, ptr);
  } else {
    // Back up to the position where the Cord should start.
    ptr = Trim(ptr);
    if (!stream_->WriteCord(cord)) return Error();
    return ptr;
  }
}

uint8_t* EpsCopyOutputStream::WriteStringMaybeAliasedOutline(uint32_t num,
                                                           const std::string& s,
                                                           uint8_t* ptr) {
  ptr = EnsureSpace(ptr);
  uint32_t size = s.size();
  ptr = WriteLengthDelim(num, size, ptr);
  return WriteRawMaybeAliased(s.data(), size, ptr);
}

uint8_t* EpsCopyOutputStream::WriteStringOutline(uint32_t num, const std::string& s,
                                               uint8_t* ptr) {
  ptr = EnsureSpace(ptr);
  uint32_t size = s.size();
  ptr = WriteLengthDelim(num, size, ptr);
  return WriteRaw(s.data(), size, ptr);
}

uint8_t* EpsCopyOutputStream::WriteStringOutline(uint32_t num, absl::string_view s,
                                               uint8_t* ptr) {
  ptr = EnsureSpace(ptr);
  uint32_t size = s.size();
  ptr = WriteLengthDelim(num, size, ptr);
  return WriteRaw(s.data(), size, ptr);
}

uint8_t* EpsCopyOutputStream::WriteCordOutline(const absl::Cord& c, uint8_t* ptr) {
  uint32_t size = c.size();
  ptr = UnsafeWriteSize(size, ptr);
  return WriteCord(c, ptr);
}

std::atomic<bool> CodedOutputStream::default_serialization_deterministic_{
    false};

CodedOutputStream::~CodedOutputStream() { Trim(); }

uint8_t* CodedOutputStream::WriteCordToArray(const absl::Cord& cord,
                                             uint8_t* target) {
  return CopyCordToArray(cord, target);
}


uint8_t* CodedOutputStream::WriteStringWithSizeToArray(const std::string& str,
                                                     uint8_t* target) {
  ABSL_DCHECK_LE(str.size(), std::numeric_limits<uint32_t>::max());
  target = WriteVarint32ToArray(str.size(), target);
  return WriteStringToArray(str, target);
}

}  // namespace io
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


