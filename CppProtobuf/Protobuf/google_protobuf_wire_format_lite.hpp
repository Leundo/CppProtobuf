// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: kenton@google.com (Kenton Varda)
//         atenasio@google.com (Chris Atenasio) (ZigZag transform)
//         wink@google.com (Wink Saville) (refactored from wire_format.h)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// This header is logically internal, but is made public because it is used
// from protocol-compiler-generated code, which may reside in other components.

#ifndef GOOGLE_PROTOBUF_WIRE_FORMAT_LITE_H__
#define GOOGLE_PROTOBUF_WIRE_FORMAT_LITE_H__

#include <limits>
#include <string>

#include <CppProtobuf/google_protobuf_stubs_common.hpp>
#include <CppAbseil/absl_base_casts.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include <CppProtobuf/google_protobuf_arenastring.hpp>
#include <CppProtobuf/google_protobuf_io_coded_stream.hpp>
#include <CppProtobuf/google_protobuf_message_lite.hpp>
#include <CppProtobuf/google_protobuf_port.hpp>
#include <CppProtobuf/google_protobuf_repeated_field.hpp>

#ifndef NDEBUG
#define GOOGLE_PROTOBUF_UTF8_VALIDATION_ENABLED
#endif

// Avoid conflict with iOS where <ConditionalMacros.h> #defines TYPE_BOOL.
//
// If some one needs the macro TYPE_BOOL in a file that includes this header,
// it's possible to bring it back using push/pop_macro as follows.
//
// #pragma push_macro("TYPE_BOOL")
// #include this header and/or all headers that need the macro to be undefined.
// #pragma pop_macro("TYPE_BOOL")
#undef TYPE_BOOL


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
namespace internal {

// This class is for internal use by the protocol buffer library and by
// protocol-compiler-generated message classes.  It must not be called
// directly by clients.
//
// This class contains helpers for implementing the binary protocol buffer
// wire format without the need for reflection. Use WireFormat when using
// reflection.
//
// This class is really a namespace that contains only static methods.
class PROTOBUF_EXPORT WireFormatLite {
 public:
  WireFormatLite() = delete;
  // -----------------------------------------------------------------
  // Helper constants and functions related to the format.  These are
  // mostly meant for internal and generated code to use.

  // The wire format is composed of a sequence of tag/value pairs, each
  // of which contains the value of one field (or one element of a repeated
  // field).  Each tag is encoded as a varint.  The lower bits of the tag
  // identify its wire type, which specifies the format of the data to follow.
  // The rest of the bits contain the field number.  Each type of field (as
  // declared by FieldDescriptor::Type, in descriptor.h) maps to one of
  // these wire types.  Immediately following each tag is the field's value,
  // encoded in the format specified by the wire type.  Because the tag
  // identifies the encoding of this data, it is possible to skip
  // unrecognized fields for forwards compatibility.

  enum WireType
#ifndef SWIG
      : int
#endif  // !SWIG
  {
    WIRETYPE_VARINT = 0,
    WIRETYPE_FIXED64 = 1,
    WIRETYPE_LENGTH_DELIMITED = 2,
    WIRETYPE_START_GROUP = 3,
    WIRETYPE_END_GROUP = 4,
    WIRETYPE_FIXED32 = 5,
  };

  // Lite alternative to FieldDescriptor::Type.  Must be kept in sync.
  enum FieldType {
    TYPE_DOUBLE = 1,
    TYPE_FLOAT = 2,
    TYPE_INT64 = 3,
    TYPE_UINT64 = 4,
    TYPE_INT32 = 5,
    TYPE_FIXED64 = 6,
    TYPE_FIXED32 = 7,
    TYPE_BOOL = 8,
    TYPE_STRING = 9,
    TYPE_GROUP = 10,
    TYPE_MESSAGE = 11,
    TYPE_BYTES = 12,
    TYPE_UINT32 = 13,
    TYPE_ENUM = 14,
    TYPE_SFIXED32 = 15,
    TYPE_SFIXED64 = 16,
    TYPE_SINT32 = 17,
    TYPE_SINT64 = 18,
    MAX_FIELD_TYPE = 18,
  };

  // Lite alternative to FieldDescriptor::CppType.  Must be kept in sync.
  enum CppType {
    CPPTYPE_INT32 = 1,
    CPPTYPE_INT64 = 2,
    CPPTYPE_UINT32 = 3,
    CPPTYPE_UINT64 = 4,
    CPPTYPE_DOUBLE = 5,
    CPPTYPE_FLOAT = 6,
    CPPTYPE_BOOL = 7,
    CPPTYPE_ENUM = 8,
    CPPTYPE_STRING = 9,
    CPPTYPE_MESSAGE = 10,
    MAX_CPPTYPE = 10,
  };

  // Helper method to get the CppType for a particular Type.
  static CppType FieldTypeToCppType(FieldType type);

  // Given a FieldDescriptor::Type return its WireType
  static inline WireFormatLite::WireType WireTypeForFieldType(
      WireFormatLite::FieldType type) {
    return kWireTypeForFieldType[type];
  }

  // Number of bits in a tag which identify the wire type.
  static constexpr int kTagTypeBits = 3;
  // Mask for those bits.
  static constexpr uint32_t kTagTypeMask = (1 << kTagTypeBits) - 1;

  // Helper functions for encoding and decoding tags.  (Inlined below and in
  // _inl.h)
  //
  // This is different from MakeTag(field->number(), field->type()) in the
  // case of packed repeated fields.
  constexpr static uint32_t MakeTag(int field_number, WireType type);
  static WireType GetTagWireType(uint32_t tag);
  static int GetTagFieldNumber(uint32_t tag);

  // Compute the byte size of a tag.  For groups, this includes both the start
  // and end tags.
  static inline size_t TagSize(int field_number,
                               WireFormatLite::FieldType type);

  // Skips a field value with the given tag.  The input should start
  // positioned immediately after the tag.  Skipped values are simply
  // discarded, not recorded anywhere.  See WireFormat::SkipField() for a
  // version that records to an UnknownFieldSet.
  static bool SkipField(io::CodedInputStream* input, uint32_t tag);

  // Skips a field value with the given tag.  The input should start
  // positioned immediately after the tag. Skipped values are recorded to a
  // CodedOutputStream.
  static bool SkipField(io::CodedInputStream* input, uint32_t tag,
                        io::CodedOutputStream* output);

  // Reads and ignores a message from the input.  Skipped values are simply
  // discarded, not recorded anywhere.  See WireFormat::SkipMessage() for a
  // version that records to an UnknownFieldSet.
  static bool SkipMessage(io::CodedInputStream* input);

  // Reads and ignores a message from the input.  Skipped values are recorded
  // to a CodedOutputStream.
  static bool SkipMessage(io::CodedInputStream* input,
                          io::CodedOutputStream* output);

  // This macro does the same thing as WireFormatLite::MakeTag(), but the
  // result is usable as a compile-time constant, which makes it usable
  // as a switch case or a template input.  WireFormatLite::MakeTag() is more
  // type-safe, though, so prefer it if possible.
#define GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(FIELD_NUMBER, TYPE) \
  static_cast<uint32_t>((static_cast<uint32_t>(FIELD_NUMBER) << 3) | (TYPE))

  // These are the tags for the old MessageSet format, which was defined as:
  //   message MessageSet {
  //     repeated group Item = 1 {
  //       required int32 type_id = 2;
  //       required string message = 3;
  //     }
  //   }
  static constexpr int kMessageSetItemNumber = 1;
  static constexpr int kMessageSetTypeIdNumber = 2;
  static constexpr int kMessageSetMessageNumber = 3;
  static const int kMessageSetItemStartTag = GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(
      kMessageSetItemNumber, WireFormatLite::WIRETYPE_START_GROUP);
  static const int kMessageSetItemEndTag = GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(
      kMessageSetItemNumber, WireFormatLite::WIRETYPE_END_GROUP);
  static const int kMessageSetTypeIdTag = GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(
      kMessageSetTypeIdNumber, WireFormatLite::WIRETYPE_VARINT);
  static const int kMessageSetMessageTag = GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(
      kMessageSetMessageNumber, WireFormatLite::WIRETYPE_LENGTH_DELIMITED);

  // Byte size of all tags of a MessageSet::Item combined.
  static const size_t kMessageSetItemTagsSize;

  // Helper functions for converting between floats/doubles and IEEE-754
  // uint32s/uint64s so that they can be written.  (Assumes your platform
  // uses IEEE-754 floats.)
  static uint32_t EncodeFloat(float value);
  static float DecodeFloat(uint32_t value);
  static uint64_t EncodeDouble(double value);
  static double DecodeDouble(uint64_t value);

  // Helper functions for mapping signed integers to unsigned integers in
  // such a way that numbers with small magnitudes will encode to smaller
  // varints.  If you simply static_cast a negative number to an unsigned
  // number and varint-encode it, it will always take 10 bytes, defeating
  // the purpose of varint.  So, for the "sint32" and "sint64" field types,
  // we ZigZag-encode the values.
  static uint32_t ZigZagEncode32(int32_t n);
  static int32_t ZigZagDecode32(uint32_t n);
  static uint64_t ZigZagEncode64(int64_t n);
  static int64_t ZigZagDecode64(uint64_t n);

  // =================================================================
  // Methods for reading/writing individual field.

  // Read fields, not including tags.  The assumption is that you already
  // read the tag to determine what field to read.

  // For primitive fields, we just use a templatized routine parameterized by
  // the represented type and the FieldType. These are specialized with the
  // appropriate definition for each declared type.
  template <typename CType, enum FieldType DeclaredType>
  PROTOBUF_NDEBUG_INLINE static bool ReadPrimitive(io::CodedInputStream* input,
                                                   CType* value);

  // Reads repeated primitive values, with optimizations for repeats.
  // tag_size and tag should both be compile-time constants provided by the
  // protocol compiler.
  template <typename CType, enum FieldType DeclaredType>
  PROTOBUF_NDEBUG_INLINE static bool ReadRepeatedPrimitive(
      int tag_size, uint32_t tag, io::CodedInputStream* input,
      RepeatedField<CType>* value);

  // Identical to ReadRepeatedPrimitive, except will not inline the
  // implementation.
  template <typename CType, enum FieldType DeclaredType>
  static bool ReadRepeatedPrimitiveNoInline(int tag_size, uint32_t tag,
                                            io::CodedInputStream* input,
                                            RepeatedField<CType>* value);

  // Reads a primitive value directly from the provided buffer. It returns a
  // pointer past the segment of data that was read.
  //
  // This is only implemented for the types with fixed wire size, e.g.
  // float, double, and the (s)fixed* types.
  template <typename CType, enum FieldType DeclaredType>
  PROTOBUF_NDEBUG_INLINE static const uint8_t* ReadPrimitiveFromArray(
      const uint8_t* buffer, CType* value);

  // Reads a primitive packed field.
  //
  // This is only implemented for packable types.
  template <typename CType, enum FieldType DeclaredType>
  PROTOBUF_NDEBUG_INLINE static bool ReadPackedPrimitive(
      io::CodedInputStream* input, RepeatedField<CType>* value);

  // Identical to ReadPackedPrimitive, except will not inline the
  // implementation.
  template <typename CType, enum FieldType DeclaredType>
  static bool ReadPackedPrimitiveNoInline(io::CodedInputStream* input,
                                          RepeatedField<CType>* value);

  // Read a packed enum field. If the is_valid function is not nullptr, values
  // for which is_valid(value) returns false are silently dropped.
  static bool ReadPackedEnumNoInline(io::CodedInputStream* input,
                                     bool (*is_valid)(int),
                                     RepeatedField<int>* values);

  // Read a packed enum field. If the is_valid function is not nullptr, values
  // for which is_valid(value) returns false are appended to
  // unknown_fields_stream.
  static bool ReadPackedEnumPreserveUnknowns(
      io::CodedInputStream* input, int field_number, bool (*is_valid)(int),
      io::CodedOutputStream* unknown_fields_stream, RepeatedField<int>* values);

  // Read a string.  ReadString(..., std::string* value) requires an
  // existing std::string.
  static inline bool ReadString(io::CodedInputStream* input,
                                std::string* value);
  // ReadString(..., std::string** p) is internal-only, and should only be
  // called from generated code. It starts by setting *p to "new std::string" if
  // *p == &GetEmptyStringAlreadyInited().  It then invokes
  // ReadString(io::CodedInputStream* input, *p).  This is useful for reducing
  // code size.
  static inline bool ReadString(io::CodedInputStream* input, std::string** p);
  // Analogous to ReadString().
  static bool ReadBytes(io::CodedInputStream* input, std::string* value);
  static bool ReadBytes(io::CodedInputStream* input, std::string** p);

  static inline bool ReadBytes(io::CodedInputStream* input, absl::Cord* value);
  static inline bool ReadBytes(io::CodedInputStream* input, absl::Cord** p);

  enum Operation {
    PARSE = 0,
    SERIALIZE = 1,
  };

  // Returns true if the data is valid UTF-8.
  static bool VerifyUtf8String(const char* data, int size, Operation op,
                               const char* field_name);

  template <typename MessageType>
  static inline bool ReadGroup(int field_number, io::CodedInputStream* input,
                               MessageType* value);

  template <typename MessageType>
  static inline bool ReadMessage(io::CodedInputStream* input,
                                 MessageType* value);

  template <typename MessageType>
  static inline bool ReadMessageNoVirtual(io::CodedInputStream* input,
                                          MessageType* value) {
    return ReadMessage(input, value);
  }

  // Write a tag.  The Write*() functions typically include the tag, so
  // normally there's no need to call this unless using the Write*NoTag()
  // variants.
  PROTOBUF_NDEBUG_INLINE static void WriteTag(int field_number, WireType type,
                                              io::CodedOutputStream* output);

  // Write fields, without tags.
  PROTOBUF_NDEBUG_INLINE static void WriteInt32NoTag(
      int32_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteInt64NoTag(
      int64_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteUInt32NoTag(
      uint32_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteUInt64NoTag(
      uint64_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteSInt32NoTag(
      int32_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteSInt64NoTag(
      int64_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteFixed32NoTag(
      uint32_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteFixed64NoTag(
      uint64_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteSFixed32NoTag(
      int32_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteSFixed64NoTag(
      int64_t value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteFloatNoTag(
      float value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteDoubleNoTag(
      double value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteBoolNoTag(
      bool value, io::CodedOutputStream* output);
  PROTOBUF_NDEBUG_INLINE static void WriteEnumNoTag(
      int value, io::CodedOutputStream* output);

  // Write array of primitive fields, without tags
  static void WriteFloatArray(const float* a, int n,
                              io::CodedOutputStream* output);
  static void WriteDoubleArray(const double* a, int n,
                               io::CodedOutputStream* output);
  static void WriteFixed32Array(const uint32_t* a, int n,
                                io::CodedOutputStream* output);
  static void WriteFixed64Array(const uint64_t* a, int n,
                                io::CodedOutputStream* output);
  static void WriteSFixed32Array(const int32_t* a, int n,
                                 io::CodedOutputStream* output);
  static void WriteSFixed64Array(const int64_t* a, int n,
                                 io::CodedOutputStream* output);
  static void WriteBoolArray(const bool* a, int n,
                             io::CodedOutputStream* output);

  // Write fields, including tags.
  static void WriteInt32(int field_number, int32_t value,
                         io::CodedOutputStream* output);
  static void WriteInt64(int field_number, int64_t value,
                         io::CodedOutputStream* output);
  static void WriteUInt32(int field_number, uint32_t value,
                          io::CodedOutputStream* output);
  static void WriteUInt64(int field_number, uint64_t value,
                          io::CodedOutputStream* output);
  static void WriteSInt32(int field_number, int32_t value,
                          io::CodedOutputStream* output);
  static void WriteSInt64(int field_number, int64_t value,
                          io::CodedOutputStream* output);
  static void WriteFixed32(int field_number, uint32_t value,
                           io::CodedOutputStream* output);
  static void WriteFixed64(int field_number, uint64_t value,
                           io::CodedOutputStream* output);
  static void WriteSFixed32(int field_number, int32_t value,
                            io::CodedOutputStream* output);
  static void WriteSFixed64(int field_number, int64_t value,
                            io::CodedOutputStream* output);
  static void WriteFloat(int field_number, float value,
                         io::CodedOutputStream* output);
  static void WriteDouble(int field_number, double value,
                          io::CodedOutputStream* output);
  static void WriteBool(int field_number, bool value,
                        io::CodedOutputStream* output);
  static void WriteEnum(int field_number, int value,
                        io::CodedOutputStream* output);

  static void WriteString(int field_number, const std::string& value,
                          io::CodedOutputStream* output);
  static void WriteBytes(int field_number, const std::string& value,
                         io::CodedOutputStream* output);
  static void WriteStringMaybeAliased(int field_number,
                                      const std::string& value,
                                      io::CodedOutputStream* output);
  static void WriteBytesMaybeAliased(int field_number, const std::string& value,
                                     io::CodedOutputStream* output);

  static void WriteGroup(int field_number, const MessageLite& value,
                         io::CodedOutputStream* output);
  static void WriteMessage(int field_number, const MessageLite& value,
                           io::CodedOutputStream* output);
  // Like above, but these will check if the output stream has enough
  // space to write directly to a flat array.
  static void WriteGroupMaybeToArray(int field_number, const MessageLite& value,
                                     io::CodedOutputStream* output);
  static void WriteMessageMaybeToArray(int field_number,
                                       const MessageLite& value,
                                       io::CodedOutputStream* output);

  // Like above, but de-virtualize the call to SerializeWithCachedSizes().  The
  // pointer must point at an instance of MessageType, *not* a subclass (or
  // the subclass must not override SerializeWithCachedSizes()).
  template <typename MessageType>
  static inline void WriteGroupNoVirtual(int field_number,
                                         const MessageType& value,
                                         io::CodedOutputStream* output);
  template <typename MessageType>
  static inline void WriteMessageNoVirtual(int field_number,
                                           const MessageType& value,
                                           io::CodedOutputStream* output);

  // Like above, but use only *ToArray methods of CodedOutputStream.
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteTagToArray(int field_number,
                                                         WireType type,
                                                         uint8_t* target);

  // Write fields, without tags.
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt32NoTagToArray(
      int32_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt64NoTagToArray(
      int64_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt32NoTagToArray(
      uint32_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt64NoTagToArray(
      uint64_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt32NoTagToArray(
      int32_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt64NoTagToArray(
      int64_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed32NoTagToArray(
      uint32_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed64NoTagToArray(
      uint64_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed32NoTagToArray(
      int32_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed64NoTagToArray(
      int64_t value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFloatNoTagToArray(
      float value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteDoubleNoTagToArray(
      double value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteBoolNoTagToArray(bool value,
                                                               uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteEnumNoTagToArray(int value,
                                                               uint8_t* target);

  // Write fields, without tags.  These require that value.size() > 0.
  template <typename T>
  PROTOBUF_NDEBUG_INLINE static uint8_t* WritePrimitiveNoTagToArray(
      const RepeatedField<T>& value, uint8_t* (*Writer)(T, uint8_t*),
      uint8_t* target);
  template <typename T>
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixedNoTagToArray(
      const RepeatedField<T>& value, uint8_t* (*Writer)(T, uint8_t*),
      uint8_t* target);

  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt32NoTagToArray(
      const RepeatedField<int32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt64NoTagToArray(
      const RepeatedField<int64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt32NoTagToArray(
      const RepeatedField<uint32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt64NoTagToArray(
      const RepeatedField<uint64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt32NoTagToArray(
      const RepeatedField<int32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt64NoTagToArray(
      const RepeatedField<int64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed32NoTagToArray(
      const RepeatedField<uint32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed64NoTagToArray(
      const RepeatedField<uint64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed32NoTagToArray(
      const RepeatedField<int32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed64NoTagToArray(
      const RepeatedField<int64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFloatNoTagToArray(
      const RepeatedField<float>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteDoubleNoTagToArray(
      const RepeatedField<double>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteBoolNoTagToArray(
      const RepeatedField<bool>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteEnumNoTagToArray(
      const RepeatedField<int>& value, uint8_t* output);

  // Write fields, including tags.
  template <int field_number>
  PROTOBUF_NOINLINE static uint8_t* WriteInt32ToArrayWithField(
      ::google::protobuf::io::EpsCopyOutputStream* stream, int32_t value,
      uint8_t* target) {
    target = stream->EnsureSpace(target);
    return WriteInt32ToArray(field_number, value, target);
  }

  template <int field_number>
  PROTOBUF_NOINLINE static uint8_t* WriteInt64ToArrayWithField(
      ::google::protobuf::io::EpsCopyOutputStream* stream, int64_t value,
      uint8_t* target) {
    target = stream->EnsureSpace(target);
    return WriteInt64ToArray(field_number, value, target);
  }

  template <int field_number>
  PROTOBUF_NOINLINE static uint8_t* WriteEnumToArrayWithField(
      ::google::protobuf::io::EpsCopyOutputStream* stream, int value, uint8_t* target) {
    target = stream->EnsureSpace(target);
    return WriteEnumToArray(field_number, value, target);
  }

  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt32ToArray(int field_number,
                                                           int32_t value,
                                                           uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt64ToArray(int field_number,
                                                           int64_t value,
                                                           uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt32ToArray(int field_number,
                                                            uint32_t value,
                                                            uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt64ToArray(int field_number,
                                                            uint64_t value,
                                                            uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt32ToArray(int field_number,
                                                            int32_t value,
                                                            uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt64ToArray(int field_number,
                                                            int64_t value,
                                                            uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed32ToArray(int field_number,
                                                             uint32_t value,
                                                             uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed64ToArray(int field_number,
                                                             uint64_t value,
                                                             uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed32ToArray(int field_number,
                                                              int32_t value,
                                                              uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed64ToArray(int field_number,
                                                              int64_t value,
                                                              uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFloatToArray(int field_number,
                                                           float value,
                                                           uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteDoubleToArray(int field_number,
                                                            double value,
                                                            uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteBoolToArray(int field_number,
                                                          bool value,
                                                          uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteEnumToArray(int field_number,
                                                          int value,
                                                          uint8_t* target);

  template <typename T>
  PROTOBUF_NDEBUG_INLINE static uint8_t* WritePrimitiveToArray(
      int field_number, const RepeatedField<T>& value,
      uint8_t* (*Writer)(int, T, uint8_t*), uint8_t* target);

  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt32ToArray(
      int field_number, const RepeatedField<int32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteInt64ToArray(
      int field_number, const RepeatedField<int64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt32ToArray(
      int field_number, const RepeatedField<uint32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteUInt64ToArray(
      int field_number, const RepeatedField<uint64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt32ToArray(
      int field_number, const RepeatedField<int32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSInt64ToArray(
      int field_number, const RepeatedField<int64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed32ToArray(
      int field_number, const RepeatedField<uint32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFixed64ToArray(
      int field_number, const RepeatedField<uint64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed32ToArray(
      int field_number, const RepeatedField<int32_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteSFixed64ToArray(
      int field_number, const RepeatedField<int64_t>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteFloatToArray(
      int field_number, const RepeatedField<float>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteDoubleToArray(
      int field_number, const RepeatedField<double>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteBoolToArray(
      int field_number, const RepeatedField<bool>& value, uint8_t* output);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteEnumToArray(
      int field_number, const RepeatedField<int>& value, uint8_t* output);

  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteStringToArray(
      int field_number, const std::string& value, uint8_t* target);
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteBytesToArray(
      int field_number, const std::string& value, uint8_t* target);

  // Whether to serialize deterministically (e.g., map keys are
  // sorted) is a property of a CodedOutputStream, and in the process
  // of serialization, the "ToArray" variants may be invoked.  But they don't
  // have a CodedOutputStream available, so they get an additional parameter
  // telling them whether to serialize deterministically.
  static uint8_t* InternalWriteGroup(int field_number, const MessageLite& value,
                                     uint8_t* target,
                                     io::EpsCopyOutputStream* stream);
  static uint8_t* InternalWriteMessage(int field_number,
                                       const MessageLite& value,
                                       int cached_size, uint8_t* target,
                                       io::EpsCopyOutputStream* stream);

  // Like above, but de-virtualize the call to SerializeWithCachedSizes().
  template <typename MessageType>
  PROTOBUF_NDEBUG_INLINE static uint8_t* InternalWriteGroupNoVirtualToArray(
      int field_number, const MessageType& value, uint8_t* target);
  template <typename MessageType>
  PROTOBUF_NDEBUG_INLINE static uint8_t* InternalWriteMessageNoVirtualToArray(
      int field_number, const MessageType& value, uint8_t* target);

  // For backward-compatibility, the last four methods also have versions
  // that are non-deterministic always.
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteGroupToArray(
      int field_number, const MessageLite& value, uint8_t* target) {
    io::EpsCopyOutputStream stream(
        target,
        value.GetCachedSize() +
            static_cast<int>(2 * io::CodedOutputStream::VarintSize32(
                                     static_cast<uint32_t>(field_number) << 3)),
        io::CodedOutputStream::IsDefaultSerializationDeterministic());
    return InternalWriteGroup(field_number, value, target, &stream);
  }
  PROTOBUF_NDEBUG_INLINE static uint8_t* WriteMessageToArray(
      int field_number, const MessageLite& value, uint8_t* target) {
    int size = value.GetCachedSize();
    io::EpsCopyOutputStream stream(
        target,
        size + static_cast<int>(io::CodedOutputStream::VarintSize32(
                                    static_cast<uint32_t>(field_number) << 3) +
                                io::CodedOutputStream::VarintSize32(size)),
        io::CodedOutputStream::IsDefaultSerializationDeterministic());
    return InternalWriteMessage(field_number, value, value.GetCachedSize(),
                                target, &stream);
  }

  // Compute the byte size of a field.  The XxSize() functions do NOT include
  // the tag, so you must also call TagSize().  (This is because, for repeated
  // fields, you should only call TagSize() once and multiply it by the element
  // count, but you may have to call XxSize() for each individual element.)
  static inline size_t Int32Size(int32_t value);
  static inline size_t Int64Size(int64_t value);
  static inline size_t UInt32Size(uint32_t value);
  static inline size_t UInt64Size(uint64_t value);
  static inline size_t SInt32Size(int32_t value);
  static inline size_t SInt64Size(int64_t value);
  static inline size_t EnumSize(int value);
  static inline size_t Int32SizePlusOne(int32_t value);
  static inline size_t Int64SizePlusOne(int64_t value);
  static inline size_t UInt32SizePlusOne(uint32_t value);
  static inline size_t UInt64SizePlusOne(uint64_t value);
  static inline size_t SInt32SizePlusOne(int32_t value);
  static inline size_t SInt64SizePlusOne(int64_t value);
  static inline size_t EnumSizePlusOne(int value);

  static size_t Int32Size(const RepeatedField<int32_t>& value);
  static size_t Int64Size(const RepeatedField<int64_t>& value);
  static size_t UInt32Size(const RepeatedField<uint32_t>& value);
  static size_t UInt64Size(const RepeatedField<uint64_t>& value);
  static size_t SInt32Size(const RepeatedField<int32_t>& value);
  static size_t SInt64Size(const RepeatedField<int64_t>& value);
  static size_t EnumSize(const RepeatedField<int>& value);

  // These types always have the same size.
  static constexpr size_t kFixed32Size = 4;
  static constexpr size_t kFixed64Size = 8;
  static constexpr size_t kSFixed32Size = 4;
  static constexpr size_t kSFixed64Size = 8;
  static constexpr size_t kFloatSize = 4;
  static constexpr size_t kDoubleSize = 8;
  static constexpr size_t kBoolSize = 1;

  static inline size_t StringSize(const std::string& value);
  static inline size_t StringSize(const absl::Cord& value);
  static inline size_t BytesSize(const std::string& value);
  static inline size_t BytesSize(const absl::Cord& value);
  static inline size_t StringSize(absl::string_view value);
  static inline size_t BytesSize(absl::string_view value);

  template <typename MessageType>
  static inline size_t GroupSize(const MessageType& value);
  template <typename MessageType>
  static inline size_t MessageSize(const MessageType& value);

  // Like above, but de-virtualize the call to ByteSize().  The
  // pointer must point at an instance of MessageType, *not* a subclass (or
  // the subclass must not override ByteSize()).
  template <typename MessageType>
  static inline size_t GroupSizeNoVirtual(const MessageType& value);
  template <typename MessageType>
  static inline size_t MessageSizeNoVirtual(const MessageType& value);

  // Given the length of data, calculate the byte size of the data on the
  // wire if we encode the data as a length delimited field.
  static inline size_t LengthDelimitedSize(size_t length);

 private:
  // A helper method for the repeated primitive reader. This method has
  // optimizations for primitive types that have fixed size on the wire, and
  // can be read using potentially faster paths.
  template <typename CType, enum FieldType DeclaredType>
  PROTOBUF_NDEBUG_INLINE static bool ReadRepeatedFixedSizePrimitive(
      int tag_size, uint32_t tag, io::CodedInputStream* input,
      RepeatedField<CType>* value);

  // Like ReadRepeatedFixedSizePrimitive but for packed primitive fields.
  template <typename CType, enum FieldType DeclaredType>
  PROTOBUF_NDEBUG_INLINE static bool ReadPackedFixedSizePrimitive(
      io::CodedInputStream* input, RepeatedField<CType>* value);

  static const CppType kFieldTypeToCppTypeMap[];
  static const WireFormatLite::WireType kWireTypeForFieldType[];
  static void WriteSubMessageMaybeToArray(int size, const MessageLite& value,
                                          io::CodedOutputStream* output);
};

// A class which deals with unknown values.  The default implementation just
// discards them.  WireFormat defines a subclass which writes to an
// UnknownFieldSet.  This class is used by ExtensionSet::ParseField(), since
// ExtensionSet is part of the lite library but UnknownFieldSet is not.
class PROTOBUF_EXPORT FieldSkipper {
 public:
  FieldSkipper() {}
  virtual ~FieldSkipper() {}

  // Skip a field whose tag has already been consumed.
  virtual bool SkipField(io::CodedInputStream* input, uint32_t tag);

  // Skip an entire message or group, up to an end-group tag (which is consumed)
  // or end-of-stream.
  virtual bool SkipMessage(io::CodedInputStream* input);

  // Deal with an already-parsed unrecognized enum value.  The default
  // implementation does nothing, but the UnknownFieldSet-based implementation
  // saves it as an unknown varint.
  virtual void SkipUnknownEnum(int field_number, int value);
};

// Subclass of FieldSkipper which saves skipped fields to a CodedOutputStream.

class PROTOBUF_EXPORT CodedOutputStreamFieldSkipper : public FieldSkipper {
 public:
  explicit CodedOutputStreamFieldSkipper(io::CodedOutputStream* unknown_fields)
      : unknown_fields_(unknown_fields) {}
  ~CodedOutputStreamFieldSkipper() override {}

  // implements FieldSkipper -----------------------------------------
  bool SkipField(io::CodedInputStream* input, uint32_t tag) override;
  bool SkipMessage(io::CodedInputStream* input) override;
  void SkipUnknownEnum(int field_number, int value) override;

 protected:
  io::CodedOutputStream* unknown_fields_;
};

// inline methods ====================================================

inline WireFormatLite::CppType WireFormatLite::FieldTypeToCppType(
    FieldType type) {
  return kFieldTypeToCppTypeMap[type];
}

constexpr inline uint32_t WireFormatLite::MakeTag(int field_number,
                                                  WireType type) {
  return GOOGLE_PROTOBUF_WIRE_FORMAT_MAKE_TAG(field_number, type);
}

inline WireFormatLite::WireType WireFormatLite::GetTagWireType(uint32_t tag) {
  return static_cast<WireType>(tag & kTagTypeMask);
}

inline int WireFormatLite::GetTagFieldNumber(uint32_t tag) {
  return static_cast<int>(tag >> kTagTypeBits);
}

inline size_t WireFormatLite::TagSize(int field_number,
                                      WireFormatLite::FieldType type) {
  size_t result = io::CodedOutputStream::VarintSize32(
      static_cast<uint32_t>(field_number << kTagTypeBits));
  if (type == TYPE_GROUP) {
    // Groups have both a start and an end tag.
    return result * 2;
  } else {
    return result;
  }
}

inline uint32_t WireFormatLite::EncodeFloat(float value) {
  return absl::bit_cast<uint32_t>(value);
}

inline float WireFormatLite::DecodeFloat(uint32_t value) {
  return absl::bit_cast<float>(value);
}

inline uint64_t WireFormatLite::EncodeDouble(double value) {
  return absl::bit_cast<uint64_t>(value);
}

inline double WireFormatLite::DecodeDouble(uint64_t value) {
  return absl::bit_cast<double>(value);
}

// ZigZag Transform:  Encodes signed integers so that they can be
// effectively used with varint encoding.
//
// varint operates on unsigned integers, encoding smaller numbers into
// fewer bytes.  If you try to use it on a signed integer, it will treat
// this number as a very large unsigned integer, which means that even
// small signed numbers like -1 will take the maximum number of bytes
// (10) to encode.  ZigZagEncode() maps signed integers to unsigned
// in such a way that those with a small absolute value will have smaller
// encoded values, making them appropriate for encoding using varint.
//
//       int32_t ->     uint32_t
// -------------------------
//           0 ->          0
//          -1 ->          1
//           1 ->          2
//          -2 ->          3
//         ... ->        ...
//  2147483647 -> 4294967294
// -2147483648 -> 4294967295
//
//        >> encode >>
//        << decode <<

inline uint32_t WireFormatLite::ZigZagEncode32(int32_t n) {
  // Note:  the right-shift must be arithmetic
  // Note:  left shift must be unsigned because of overflow
  return (static_cast<uint32_t>(n) << 1) ^ static_cast<uint32_t>(n >> 31);
}

inline int32_t WireFormatLite::ZigZagDecode32(uint32_t n) {
  // Note:  Using unsigned types prevent undefined behavior
  return static_cast<int32_t>((n >> 1) ^ (~(n & 1) + 1));
}

inline uint64_t WireFormatLite::ZigZagEncode64(int64_t n) {
  // Note:  the right-shift must be arithmetic
  // Note:  left shift must be unsigned because of overflow
  return (static_cast<uint64_t>(n) << 1) ^ static_cast<uint64_t>(n >> 63);
}

inline int64_t WireFormatLite::ZigZagDecode64(uint64_t n) {
  // Note:  Using unsigned types prevent undefined behavior
  return static_cast<int64_t>((n >> 1) ^ (~(n & 1) + 1));
}

// String is for UTF-8 text only, but, even so, ReadString() can simply
// call ReadBytes().

inline bool WireFormatLite::ReadString(io::CodedInputStream* input,
                                       std::string* value) {
  return ReadBytes(input, value);
}

inline bool WireFormatLite::ReadString(io::CodedInputStream* input,
                                       std::string** p) {
  return ReadBytes(input, p);
}

inline uint8_t* InternalSerializeUnknownMessageSetItemsToArray(
    const std::string& unknown_fields, uint8_t* target,
    io::EpsCopyOutputStream* stream) {
  return stream->WriteRaw(unknown_fields.data(),
                          static_cast<int>(unknown_fields.size()), target);
}

inline size_t ComputeUnknownMessageSetItemsSize(
    const std::string& unknown_fields) {
  return unknown_fields.size();
}

// Implementation details of ReadPrimitive.

template <>
inline bool WireFormatLite::ReadPrimitive<int32_t, WireFormatLite::TYPE_INT32>(
    io::CodedInputStream* input, int32_t* value) {
  uint32_t temp;
  if (!input->ReadVarint32(&temp)) return false;
  *value = static_cast<int32_t>(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<int64_t, WireFormatLite::TYPE_INT64>(
    io::CodedInputStream* input, int64_t* value) {
  uint64_t temp;
  if (!input->ReadVarint64(&temp)) return false;
  *value = static_cast<int64_t>(temp);
  return true;
}
template <>
inline bool
WireFormatLite::ReadPrimitive<uint32_t, WireFormatLite::TYPE_UINT32>(
    io::CodedInputStream* input, uint32_t* value) {
  return input->ReadVarint32(value);
}
template <>
inline bool
WireFormatLite::ReadPrimitive<uint64_t, WireFormatLite::TYPE_UINT64>(
    io::CodedInputStream* input, uint64_t* value) {
  return input->ReadVarint64(value);
}
template <>
inline bool WireFormatLite::ReadPrimitive<int32_t, WireFormatLite::TYPE_SINT32>(
    io::CodedInputStream* input, int32_t* value) {
  uint32_t temp;
  if (!input->ReadVarint32(&temp)) return false;
  *value = ZigZagDecode32(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<int64_t, WireFormatLite::TYPE_SINT64>(
    io::CodedInputStream* input, int64_t* value) {
  uint64_t temp;
  if (!input->ReadVarint64(&temp)) return false;
  *value = ZigZagDecode64(temp);
  return true;
}
template <>
inline bool
WireFormatLite::ReadPrimitive<uint32_t, WireFormatLite::TYPE_FIXED32>(
    io::CodedInputStream* input, uint32_t* value) {
  return input->ReadLittleEndian32(value);
}
template <>
inline bool
WireFormatLite::ReadPrimitive<uint64_t, WireFormatLite::TYPE_FIXED64>(
    io::CodedInputStream* input, uint64_t* value) {
  return input->ReadLittleEndian64(value);
}
template <>
inline bool
WireFormatLite::ReadPrimitive<int32_t, WireFormatLite::TYPE_SFIXED32>(
    io::CodedInputStream* input, int32_t* value) {
  uint32_t temp;
  if (!input->ReadLittleEndian32(&temp)) return false;
  *value = static_cast<int32_t>(temp);
  return true;
}
template <>
inline bool
WireFormatLite::ReadPrimitive<int64_t, WireFormatLite::TYPE_SFIXED64>(
    io::CodedInputStream* input, int64_t* value) {
  uint64_t temp;
  if (!input->ReadLittleEndian64(&temp)) return false;
  *value = static_cast<int64_t>(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<float, WireFormatLite::TYPE_FLOAT>(
    io::CodedInputStream* input, float* value) {
  uint32_t temp;
  if (!input->ReadLittleEndian32(&temp)) return false;
  *value = DecodeFloat(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<double, WireFormatLite::TYPE_DOUBLE>(
    io::CodedInputStream* input, double* value) {
  uint64_t temp;
  if (!input->ReadLittleEndian64(&temp)) return false;
  *value = DecodeDouble(temp);
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<bool, WireFormatLite::TYPE_BOOL>(
    io::CodedInputStream* input, bool* value) {
  uint64_t temp;
  if (!input->ReadVarint64(&temp)) return false;
  *value = temp != 0;
  return true;
}
template <>
inline bool WireFormatLite::ReadPrimitive<int, WireFormatLite::TYPE_ENUM>(
    io::CodedInputStream* input, int* value) {
  uint32_t temp;
  if (!input->ReadVarint32(&temp)) return false;
  *value = static_cast<int>(temp);
  return true;
}

template <>
inline const uint8_t*
WireFormatLite::ReadPrimitiveFromArray<uint32_t, WireFormatLite::TYPE_FIXED32>(
    const uint8_t* buffer, uint32_t* value) {
  return io::CodedInputStream::ReadLittleEndian32FromArray(buffer, value);
}
template <>
inline const uint8_t*
WireFormatLite::ReadPrimitiveFromArray<uint64_t, WireFormatLite::TYPE_FIXED64>(
    const uint8_t* buffer, uint64_t* value) {
  return io::CodedInputStream::ReadLittleEndian64FromArray(buffer, value);
}
template <>
inline const uint8_t*
WireFormatLite::ReadPrimitiveFromArray<int32_t, WireFormatLite::TYPE_SFIXED32>(
    const uint8_t* buffer, int32_t* value) {
  uint32_t temp;
  buffer = io::CodedInputStream::ReadLittleEndian32FromArray(buffer, &temp);
  *value = static_cast<int32_t>(temp);
  return buffer;
}
template <>
inline const uint8_t*
WireFormatLite::ReadPrimitiveFromArray<int64_t, WireFormatLite::TYPE_SFIXED64>(
    const uint8_t* buffer, int64_t* value) {
  uint64_t temp;
  buffer = io::CodedInputStream::ReadLittleEndian64FromArray(buffer, &temp);
  *value = static_cast<int64_t>(temp);
  return buffer;
}
template <>
inline const uint8_t*
WireFormatLite::ReadPrimitiveFromArray<float, WireFormatLite::TYPE_FLOAT>(
    const uint8_t* buffer, float* value) {
  uint32_t temp;
  buffer = io::CodedInputStream::ReadLittleEndian32FromArray(buffer, &temp);
  *value = DecodeFloat(temp);
  return buffer;
}
template <>
inline const uint8_t*
WireFormatLite::ReadPrimitiveFromArray<double, WireFormatLite::TYPE_DOUBLE>(
    const uint8_t* buffer, double* value) {
  uint64_t temp;
  buffer = io::CodedInputStream::ReadLittleEndian64FromArray(buffer, &temp);
  *value = DecodeDouble(temp);
  return buffer;
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadRepeatedPrimitive(
    int,  // tag_size, unused.
    uint32_t tag, io::CodedInputStream* input, RepeatedField<CType>* values) {
  CType value;
  if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
  values->Add(value);
  int elements_already_reserved = values->Capacity() - values->size();
  while (elements_already_reserved > 0 && input->ExpectTag(tag)) {
    if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
    values->AddAlreadyReserved(value);
    elements_already_reserved--;
  }
  return true;
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadRepeatedFixedSizePrimitive(
    int tag_size, uint32_t tag, io::CodedInputStream* input,
    RepeatedField<CType>* values) {
  ABSL_DCHECK_EQ(UInt32Size(tag), static_cast<size_t>(tag_size));
  CType value;
  if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
  values->Add(value);

  // For fixed size values, repeated values can be read more quickly by
  // reading directly from a raw array.
  //
  // We can get a tight loop by only reading as many elements as can be
  // added to the RepeatedField without having to do any resizing. Additionally,
  // we only try to read as many elements as are available from the current
  // buffer space. Doing so avoids having to perform boundary checks when
  // reading the value: the maximum number of elements that can be read is
  // known outside of the loop.
  const void* void_pointer;
  int size;
  input->GetDirectBufferPointerInline(&void_pointer, &size);
  if (size > 0) {
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(void_pointer);
    // The number of bytes each type occupies on the wire.
    const int per_value_size = tag_size + static_cast<int>(sizeof(value));

    // parentheses around (std::min) prevents macro expansion of min(...)
    int elements_available =
        (std::min)(values->Capacity() - values->size(), size / per_value_size);
    int num_read = 0;
    while (num_read < elements_available &&
           (buffer = io::CodedInputStream::ExpectTagFromArray(buffer, tag)) !=
               nullptr) {
      buffer = ReadPrimitiveFromArray<CType, DeclaredType>(buffer, &value);
      values->AddAlreadyReserved(value);
      ++num_read;
    }
    const int read_bytes = num_read * per_value_size;
    if (read_bytes > 0) {
      input->Skip(read_bytes);
    }
  }
  return true;
}

// Specializations of ReadRepeatedPrimitive for the fixed size types, which use
// the optimized code path.
#define READ_REPEATED_FIXED_SIZE_PRIMITIVE(CPPTYPE, DECLARED_TYPE)        \
  template <>                                                             \
  inline bool WireFormatLite::ReadRepeatedPrimitive<                      \
      CPPTYPE, WireFormatLite::DECLARED_TYPE>(                            \
      int tag_size, uint32_t tag, io::CodedInputStream* input,            \
      RepeatedField<CPPTYPE>* values) {                                   \
    return ReadRepeatedFixedSizePrimitive<CPPTYPE,                        \
                                          WireFormatLite::DECLARED_TYPE>( \
        tag_size, tag, input, values);                                    \
  }

READ_REPEATED_FIXED_SIZE_PRIMITIVE(uint32_t, TYPE_FIXED32)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(uint64_t, TYPE_FIXED64)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(int32_t, TYPE_SFIXED32)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(int64_t, TYPE_SFIXED64)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(float, TYPE_FLOAT)
READ_REPEATED_FIXED_SIZE_PRIMITIVE(double, TYPE_DOUBLE)

#undef READ_REPEATED_FIXED_SIZE_PRIMITIVE

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
bool WireFormatLite::ReadRepeatedPrimitiveNoInline(
    int tag_size, uint32_t tag, io::CodedInputStream* input,
    RepeatedField<CType>* value) {
  return ReadRepeatedPrimitive<CType, DeclaredType>(tag_size, tag, input,
                                                    value);
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadPackedPrimitive(io::CodedInputStream* input,
                                                RepeatedField<CType>* values) {
  int length;
  if (!input->ReadVarintSizeAsInt(&length)) return false;
  io::CodedInputStream::Limit limit = input->PushLimit(length);
  while (input->BytesUntilLimit() > 0) {
    CType value;
    if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
    values->Add(value);
  }
  input->PopLimit(limit);
  return true;
}

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
inline bool WireFormatLite::ReadPackedFixedSizePrimitive(
    io::CodedInputStream* input, RepeatedField<CType>* values) {
  int length;
  if (!input->ReadVarintSizeAsInt(&length)) return false;
  const int old_entries = values->size();
  const int new_entries = length / static_cast<int>(sizeof(CType));
  const int new_bytes = new_entries * static_cast<int>(sizeof(CType));
  if (new_bytes != length) return false;
  // We would *like* to pre-allocate the buffer to write into (for
  // speed), but *must* avoid performing a very large allocation due
  // to a malicious user-supplied "length" above.  So we have a fast
  // path that pre-allocates when the "length" is less than a bound.
  // We determine the bound by calling BytesUntilTotalBytesLimit() and
  // BytesUntilLimit().  These return -1 to mean "no limit set".
  // There are four cases:
  // TotalBytesLimit  Limit
  // -1               -1     Use slow path.
  // -1               >= 0   Use fast path if length <= Limit.
  // >= 0             -1     Use slow path.
  // >= 0             >= 0   Use fast path if length <= min(both limits).
  int64_t bytes_limit = input->BytesUntilTotalBytesLimit();
  if (bytes_limit == -1) {
    bytes_limit = input->BytesUntilLimit();
  } else {
    // parentheses around (std::min) prevents macro expansion of min(...)
    bytes_limit =
        (std::min)(bytes_limit, static_cast<int64_t>(input->BytesUntilLimit()));
  }
  if (bytes_limit >= new_bytes) {
    // Fast-path that pre-allocates *values to the final size.
#if defined(PROTOBUF_LITTLE_ENDIAN)
    values->Resize(old_entries + new_entries, 0);
    // values->mutable_data() may change after Resize(), so do this after:
    void* dest = reinterpret_cast<void*>(values->mutable_data() + old_entries);
    if (!input->ReadRaw(dest, new_bytes)) {
      values->Truncate(old_entries);
      return false;
    }
#else
    values->Reserve(old_entries + new_entries);
    CType value;
    for (int i = 0; i < new_entries; ++i) {
      if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
      values->AddAlreadyReserved(value);
    }
#endif
  } else {
    // This is the slow-path case where "length" may be too large to
    // safely allocate.  We read as much as we can into *values
    // without pre-allocating "length" bytes.
    CType value;
    for (int i = 0; i < new_entries; ++i) {
      if (!ReadPrimitive<CType, DeclaredType>(input, &value)) return false;
      values->Add(value);
    }
  }
  return true;
}

// Specializations of ReadPackedPrimitive for the fixed size types, which use
// an optimized code path.
#define READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(CPPTYPE, DECLARED_TYPE)      \
  template <>                                                                  \
  inline bool                                                                  \
  WireFormatLite::ReadPackedPrimitive<CPPTYPE, WireFormatLite::DECLARED_TYPE>( \
      io::CodedInputStream * input, RepeatedField<CPPTYPE> * values) {         \
    return ReadPackedFixedSizePrimitive<CPPTYPE,                               \
                                        WireFormatLite::DECLARED_TYPE>(        \
        input, values);                                                        \
  }

READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(uint32_t, TYPE_FIXED32)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(uint64_t, TYPE_FIXED64)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(int32_t, TYPE_SFIXED32)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(int64_t, TYPE_SFIXED64)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(float, TYPE_FLOAT)
READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE(double, TYPE_DOUBLE)

#undef READ_REPEATED_PACKED_FIXED_SIZE_PRIMITIVE

template <typename CType, enum WireFormatLite::FieldType DeclaredType>
bool WireFormatLite::ReadPackedPrimitiveNoInline(io::CodedInputStream* input,
                                                 RepeatedField<CType>* values) {
  return ReadPackedPrimitive<CType, DeclaredType>(input, values);
}

inline bool WireFormatLite::ReadBytes(io::CodedInputStream* input,
                                      absl::Cord* value) {
  int length;
  return input->ReadVarintSizeAsInt(&length) && input->ReadCord(value, length);
}

inline bool WireFormatLite::ReadBytes(io::CodedInputStream* input,
                                      absl::Cord** p) {
  return ReadBytes(input, *p);
}


template <typename MessageType>
inline bool WireFormatLite::ReadGroup(int field_number,
                                      io::CodedInputStream* input,
                                      MessageType* value) {
  if (!input->IncrementRecursionDepth()) return false;
  if (!value->MergePartialFromCodedStream(input)) return false;
  input->UnsafeDecrementRecursionDepth();
  // Make sure the last thing read was an end tag for this group.
  if (!input->LastTagWas(MakeTag(field_number, WIRETYPE_END_GROUP))) {
    return false;
  }
  return true;
}
template <typename MessageType>
inline bool WireFormatLite::ReadMessage(io::CodedInputStream* input,
                                        MessageType* value) {
  int length;
  if (!input->ReadVarintSizeAsInt(&length)) return false;
  std::pair<io::CodedInputStream::Limit, int> p =
      input->IncrementRecursionDepthAndPushLimit(length);
  if (p.second < 0 || !value->MergePartialFromCodedStream(input)) return false;
  // Make sure that parsing stopped when the limit was hit, not at an endgroup
  // tag.
  return input->DecrementRecursionDepthAndPopLimit(p.first);
}

// ===================================================================

inline void WireFormatLite::WriteTag(int field_number, WireType type,
                                     io::CodedOutputStream* output) {
  output->WriteTag(MakeTag(field_number, type));
}

inline void WireFormatLite::WriteInt32NoTag(int32_t value,
                                            io::CodedOutputStream* output) {
  output->WriteVarint32SignExtended(value);
}
inline void WireFormatLite::WriteInt64NoTag(int64_t value,
                                            io::CodedOutputStream* output) {
  output->WriteVarint64(static_cast<uint64_t>(value));
}
inline void WireFormatLite::WriteUInt32NoTag(uint32_t value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint32(value);
}
inline void WireFormatLite::WriteUInt64NoTag(uint64_t value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint64(value);
}
inline void WireFormatLite::WriteSInt32NoTag(int32_t value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint32(ZigZagEncode32(value));
}
inline void WireFormatLite::WriteSInt64NoTag(int64_t value,
                                             io::CodedOutputStream* output) {
  output->WriteVarint64(ZigZagEncode64(value));
}
inline void WireFormatLite::WriteFixed32NoTag(uint32_t value,
                                              io::CodedOutputStream* output) {
  output->WriteLittleEndian32(value);
}
inline void WireFormatLite::WriteFixed64NoTag(uint64_t value,
                                              io::CodedOutputStream* output) {
  output->WriteLittleEndian64(value);
}
inline void WireFormatLite::WriteSFixed32NoTag(int32_t value,
                                               io::CodedOutputStream* output) {
  output->WriteLittleEndian32(static_cast<uint32_t>(value));
}
inline void WireFormatLite::WriteSFixed64NoTag(int64_t value,
                                               io::CodedOutputStream* output) {
  output->WriteLittleEndian64(static_cast<uint64_t>(value));
}
inline void WireFormatLite::WriteFloatNoTag(float value,
                                            io::CodedOutputStream* output) {
  output->WriteLittleEndian32(EncodeFloat(value));
}
inline void WireFormatLite::WriteDoubleNoTag(double value,
                                             io::CodedOutputStream* output) {
  output->WriteLittleEndian64(EncodeDouble(value));
}
inline void WireFormatLite::WriteBoolNoTag(bool value,
                                           io::CodedOutputStream* output) {
  output->WriteVarint32(value ? 1 : 0);
}
inline void WireFormatLite::WriteEnumNoTag(int value,
                                           io::CodedOutputStream* output) {
  output->WriteVarint32SignExtended(value);
}

// See comment on ReadGroupNoVirtual to understand the need for this template
// parameter name.
template <typename MessageType_WorkAroundCppLookupDefect>
inline void WireFormatLite::WriteGroupNoVirtual(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    io::CodedOutputStream* output) {
  WriteTag(field_number, WIRETYPE_START_GROUP, output);
  value.MessageType_WorkAroundCppLookupDefect::SerializeWithCachedSizes(output);
  WriteTag(field_number, WIRETYPE_END_GROUP, output);
}
template <typename MessageType_WorkAroundCppLookupDefect>
inline void WireFormatLite::WriteMessageNoVirtual(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    io::CodedOutputStream* output) {
  WriteTag(field_number, WIRETYPE_LENGTH_DELIMITED, output);
  output->WriteVarint32(
      value.MessageType_WorkAroundCppLookupDefect::GetCachedSize());
  value.MessageType_WorkAroundCppLookupDefect::SerializeWithCachedSizes(output);
}

// ===================================================================

inline uint8_t* WireFormatLite::WriteTagToArray(int field_number, WireType type,
                                                uint8_t* target) {
  return io::CodedOutputStream::WriteTagToArray(MakeTag(field_number, type),
                                                target);
}

inline uint8_t* WireFormatLite::WriteInt32NoTagToArray(int32_t value,
                                                       uint8_t* target) {
  return io::CodedOutputStream::WriteVarint32SignExtendedToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteInt64NoTagToArray(int64_t value,
                                                       uint8_t* target) {
  return io::CodedOutputStream::WriteVarint64ToArray(
      static_cast<uint64_t>(value), target);
}
inline uint8_t* WireFormatLite::WriteUInt32NoTagToArray(uint32_t value,
                                                        uint8_t* target) {
  return io::CodedOutputStream::WriteVarint32ToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteUInt64NoTagToArray(uint64_t value,
                                                        uint8_t* target) {
  return io::CodedOutputStream::WriteVarint64ToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteSInt32NoTagToArray(int32_t value,
                                                        uint8_t* target) {
  return io::CodedOutputStream::WriteVarint32ToArray(ZigZagEncode32(value),
                                                     target);
}
inline uint8_t* WireFormatLite::WriteSInt64NoTagToArray(int64_t value,
                                                        uint8_t* target) {
  return io::CodedOutputStream::WriteVarint64ToArray(ZigZagEncode64(value),
                                                     target);
}
inline uint8_t* WireFormatLite::WriteFixed32NoTagToArray(uint32_t value,
                                                         uint8_t* target) {
  return io::CodedOutputStream::WriteLittleEndian32ToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteFixed64NoTagToArray(uint64_t value,
                                                         uint8_t* target) {
  return io::CodedOutputStream::WriteLittleEndian64ToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteSFixed32NoTagToArray(int32_t value,
                                                          uint8_t* target) {
  return io::CodedOutputStream::WriteLittleEndian32ToArray(
      static_cast<uint32_t>(value), target);
}
inline uint8_t* WireFormatLite::WriteSFixed64NoTagToArray(int64_t value,
                                                          uint8_t* target) {
  return io::CodedOutputStream::WriteLittleEndian64ToArray(
      static_cast<uint64_t>(value), target);
}
inline uint8_t* WireFormatLite::WriteFloatNoTagToArray(float value,
                                                       uint8_t* target) {
  return io::CodedOutputStream::WriteLittleEndian32ToArray(EncodeFloat(value),
                                                           target);
}
inline uint8_t* WireFormatLite::WriteDoubleNoTagToArray(double value,
                                                        uint8_t* target) {
  return io::CodedOutputStream::WriteLittleEndian64ToArray(EncodeDouble(value),
                                                           target);
}
inline uint8_t* WireFormatLite::WriteBoolNoTagToArray(bool value,
                                                      uint8_t* target) {
  return io::CodedOutputStream::WriteVarint32ToArray(value ? 1 : 0, target);
}
inline uint8_t* WireFormatLite::WriteEnumNoTagToArray(int value,
                                                      uint8_t* target) {
  return io::CodedOutputStream::WriteVarint32SignExtendedToArray(value, target);
}

template <typename T>
inline uint8_t* WireFormatLite::WritePrimitiveNoTagToArray(
    const RepeatedField<T>& value, uint8_t* (*Writer)(T, uint8_t*),
    uint8_t* target) {
  const int n = value.size();
  ABSL_DCHECK_GT(n, 0);

  const T* ii = value.data();
  int i = 0;
  do {
    target = Writer(ii[i], target);
  } while (++i < n);

  return target;
}

template <typename T>
inline uint8_t* WireFormatLite::WriteFixedNoTagToArray(
    const RepeatedField<T>& value, uint8_t* (*Writer)(T, uint8_t*),
    uint8_t* target) {
#if defined(PROTOBUF_LITTLE_ENDIAN)
  (void)Writer;

  const int n = value.size();
  ABSL_DCHECK_GT(n, 0);

  const T* ii = value.data();
  const int bytes = n * static_cast<int>(sizeof(ii[0]));
  memcpy(target, ii, static_cast<size_t>(bytes));
  return target + bytes;
#else
  return WritePrimitiveNoTagToArray(value, Writer, target);
#endif
}

inline uint8_t* WireFormatLite::WriteInt32NoTagToArray(
    const RepeatedField<int32_t>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteInt32NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteInt64NoTagToArray(
    const RepeatedField<int64_t>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteInt64NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteUInt32NoTagToArray(
    const RepeatedField<uint32_t>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteUInt32NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteUInt64NoTagToArray(
    const RepeatedField<uint64_t>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteUInt64NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteSInt32NoTagToArray(
    const RepeatedField<int32_t>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteSInt32NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteSInt64NoTagToArray(
    const RepeatedField<int64_t>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteSInt64NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteFixed32NoTagToArray(
    const RepeatedField<uint32_t>& value, uint8_t* target) {
  return WriteFixedNoTagToArray(value, WriteFixed32NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteFixed64NoTagToArray(
    const RepeatedField<uint64_t>& value, uint8_t* target) {
  return WriteFixedNoTagToArray(value, WriteFixed64NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteSFixed32NoTagToArray(
    const RepeatedField<int32_t>& value, uint8_t* target) {
  return WriteFixedNoTagToArray(value, WriteSFixed32NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteSFixed64NoTagToArray(
    const RepeatedField<int64_t>& value, uint8_t* target) {
  return WriteFixedNoTagToArray(value, WriteSFixed64NoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteFloatNoTagToArray(
    const RepeatedField<float>& value, uint8_t* target) {
  return WriteFixedNoTagToArray(value, WriteFloatNoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteDoubleNoTagToArray(
    const RepeatedField<double>& value, uint8_t* target) {
  return WriteFixedNoTagToArray(value, WriteDoubleNoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteBoolNoTagToArray(
    const RepeatedField<bool>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteBoolNoTagToArray, target);
}
inline uint8_t* WireFormatLite::WriteEnumNoTagToArray(
    const RepeatedField<int>& value, uint8_t* target) {
  return WritePrimitiveNoTagToArray(value, WriteEnumNoTagToArray, target);
}

inline uint8_t* WireFormatLite::WriteInt32ToArray(int field_number,
                                                  int32_t value,
                                                  uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteInt32NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteInt64ToArray(int field_number,
                                                  int64_t value,
                                                  uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteInt64NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteUInt32ToArray(int field_number,
                                                   uint32_t value,
                                                   uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteUInt32NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteUInt64ToArray(int field_number,
                                                   uint64_t value,
                                                   uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteUInt64NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteSInt32ToArray(int field_number,
                                                   int32_t value,
                                                   uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteSInt32NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteSInt64ToArray(int field_number,
                                                   int64_t value,
                                                   uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteSInt64NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteFixed32ToArray(int field_number,
                                                    uint32_t value,
                                                    uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED32, target);
  return WriteFixed32NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteFixed64ToArray(int field_number,
                                                    uint64_t value,
                                                    uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED64, target);
  return WriteFixed64NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteSFixed32ToArray(int field_number,
                                                     int32_t value,
                                                     uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED32, target);
  return WriteSFixed32NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteSFixed64ToArray(int field_number,
                                                     int64_t value,
                                                     uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED64, target);
  return WriteSFixed64NoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteFloatToArray(int field_number, float value,
                                                  uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED32, target);
  return WriteFloatNoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteDoubleToArray(int field_number,
                                                   double value,
                                                   uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_FIXED64, target);
  return WriteDoubleNoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteBoolToArray(int field_number, bool value,
                                                 uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteBoolNoTagToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteEnumToArray(int field_number, int value,
                                                 uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_VARINT, target);
  return WriteEnumNoTagToArray(value, target);
}

template <typename T>
inline uint8_t* WireFormatLite::WritePrimitiveToArray(
    int field_number, const RepeatedField<T>& value,
    uint8_t* (*Writer)(int, T, uint8_t*), uint8_t* target) {
  const int n = value.size();
  if (n == 0) {
    return target;
  }

  const T* ii = value.data();
  int i = 0;
  do {
    target = Writer(field_number, ii[i], target);
  } while (++i < n);

  return target;
}

inline uint8_t* WireFormatLite::WriteInt32ToArray(
    int field_number, const RepeatedField<int32_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteInt32ToArray, target);
}
inline uint8_t* WireFormatLite::WriteInt64ToArray(
    int field_number, const RepeatedField<int64_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteInt64ToArray, target);
}
inline uint8_t* WireFormatLite::WriteUInt32ToArray(
    int field_number, const RepeatedField<uint32_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteUInt32ToArray, target);
}
inline uint8_t* WireFormatLite::WriteUInt64ToArray(
    int field_number, const RepeatedField<uint64_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteUInt64ToArray, target);
}
inline uint8_t* WireFormatLite::WriteSInt32ToArray(
    int field_number, const RepeatedField<int32_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteSInt32ToArray, target);
}
inline uint8_t* WireFormatLite::WriteSInt64ToArray(
    int field_number, const RepeatedField<int64_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteSInt64ToArray, target);
}
inline uint8_t* WireFormatLite::WriteFixed32ToArray(
    int field_number, const RepeatedField<uint32_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteFixed32ToArray,
                               target);
}
inline uint8_t* WireFormatLite::WriteFixed64ToArray(
    int field_number, const RepeatedField<uint64_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteFixed64ToArray,
                               target);
}
inline uint8_t* WireFormatLite::WriteSFixed32ToArray(
    int field_number, const RepeatedField<int32_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteSFixed32ToArray,
                               target);
}
inline uint8_t* WireFormatLite::WriteSFixed64ToArray(
    int field_number, const RepeatedField<int64_t>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteSFixed64ToArray,
                               target);
}
inline uint8_t* WireFormatLite::WriteFloatToArray(
    int field_number, const RepeatedField<float>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteFloatToArray, target);
}
inline uint8_t* WireFormatLite::WriteDoubleToArray(
    int field_number, const RepeatedField<double>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteDoubleToArray, target);
}
inline uint8_t* WireFormatLite::WriteBoolToArray(
    int field_number, const RepeatedField<bool>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteBoolToArray, target);
}
inline uint8_t* WireFormatLite::WriteEnumToArray(
    int field_number, const RepeatedField<int>& value, uint8_t* target) {
  return WritePrimitiveToArray(field_number, value, WriteEnumToArray, target);
}
inline uint8_t* WireFormatLite::WriteStringToArray(int field_number,
                                                   const std::string& value,
                                                   uint8_t* target) {
  // String is for UTF-8 text only
  // WARNING:  In wire_format.cc, both strings and bytes are handled by
  //   WriteString() to avoid code duplication.  If the implementations become
  //   different, you will need to update that usage.
  target = WriteTagToArray(field_number, WIRETYPE_LENGTH_DELIMITED, target);
  return io::CodedOutputStream::WriteStringWithSizeToArray(value, target);
}
inline uint8_t* WireFormatLite::WriteBytesToArray(int field_number,
                                                  const std::string& value,
                                                  uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_LENGTH_DELIMITED, target);
  return io::CodedOutputStream::WriteStringWithSizeToArray(value, target);
}


// See comment on ReadGroupNoVirtual to understand the need for this template
// parameter name.
template <typename MessageType_WorkAroundCppLookupDefect>
inline uint8_t* WireFormatLite::InternalWriteGroupNoVirtualToArray(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_START_GROUP, target);
  target = value.MessageType_WorkAroundCppLookupDefect::
               SerializeWithCachedSizesToArray(target);
  return WriteTagToArray(field_number, WIRETYPE_END_GROUP, target);
}
template <typename MessageType_WorkAroundCppLookupDefect>
inline uint8_t* WireFormatLite::InternalWriteMessageNoVirtualToArray(
    int field_number, const MessageType_WorkAroundCppLookupDefect& value,
    uint8_t* target) {
  target = WriteTagToArray(field_number, WIRETYPE_LENGTH_DELIMITED, target);
  target = io::CodedOutputStream::WriteVarint32ToArray(
      static_cast<uint32_t>(
          value.MessageType_WorkAroundCppLookupDefect::GetCachedSize()),
      target);
  return value
      .MessageType_WorkAroundCppLookupDefect::SerializeWithCachedSizesToArray(
          target);
}

// ===================================================================

inline size_t WireFormatLite::Int32Size(int32_t value) {
  return io::CodedOutputStream::VarintSize32SignExtended(value);
}
inline size_t WireFormatLite::Int64Size(int64_t value) {
  return io::CodedOutputStream::VarintSize64(static_cast<uint64_t>(value));
}
inline size_t WireFormatLite::UInt32Size(uint32_t value) {
  return io::CodedOutputStream::VarintSize32(value);
}
inline size_t WireFormatLite::UInt64Size(uint64_t value) {
  return io::CodedOutputStream::VarintSize64(value);
}
inline size_t WireFormatLite::SInt32Size(int32_t value) {
  return io::CodedOutputStream::VarintSize32(ZigZagEncode32(value));
}
inline size_t WireFormatLite::SInt64Size(int64_t value) {
  return io::CodedOutputStream::VarintSize64(ZigZagEncode64(value));
}
inline size_t WireFormatLite::EnumSize(int value) {
  return io::CodedOutputStream::VarintSize32SignExtended(value);
}
inline size_t WireFormatLite::Int32SizePlusOne(int32_t value) {
  return io::CodedOutputStream::VarintSize32SignExtendedPlusOne(value);
}
inline size_t WireFormatLite::Int64SizePlusOne(int64_t value) {
  return io::CodedOutputStream::VarintSize64PlusOne(
      static_cast<uint64_t>(value));
}
inline size_t WireFormatLite::UInt32SizePlusOne(uint32_t value) {
  return io::CodedOutputStream::VarintSize32PlusOne(value);
}
inline size_t WireFormatLite::UInt64SizePlusOne(uint64_t value) {
  return io::CodedOutputStream::VarintSize64PlusOne(value);
}
inline size_t WireFormatLite::SInt32SizePlusOne(int32_t value) {
  return io::CodedOutputStream::VarintSize32PlusOne(ZigZagEncode32(value));
}
inline size_t WireFormatLite::SInt64SizePlusOne(int64_t value) {
  return io::CodedOutputStream::VarintSize64PlusOne(ZigZagEncode64(value));
}
inline size_t WireFormatLite::EnumSizePlusOne(int value) {
  return io::CodedOutputStream::VarintSize32SignExtendedPlusOne(value);
}

inline size_t WireFormatLite::StringSize(const std::string& value) {
  return LengthDelimitedSize(value.size());
}
inline size_t WireFormatLite::BytesSize(const std::string& value) {
  return LengthDelimitedSize(value.size());
}

inline size_t WireFormatLite::BytesSize(const absl::Cord& value) {
  return LengthDelimitedSize(value.size());
}

inline size_t WireFormatLite::StringSize(const absl::Cord& value) {
  return LengthDelimitedSize(value.size());
}

inline size_t WireFormatLite::StringSize(const absl::string_view value) {
  // WARNING:  In wire_format.cc, both strings and bytes are handled by
  //   StringSize() to avoid code duplication.  If the implementations become
  //   different, you will need to update that usage.
  return LengthDelimitedSize(value.size());
}
inline size_t WireFormatLite::BytesSize(const absl::string_view value) {
  return LengthDelimitedSize(value.size());
}

template <typename MessageType>
inline size_t WireFormatLite::GroupSize(const MessageType& value) {
  return value.ByteSizeLong();
}
template <typename MessageType>
inline size_t WireFormatLite::MessageSize(const MessageType& value) {
  return LengthDelimitedSize(value.ByteSizeLong());
}

// See comment on ReadGroupNoVirtual to understand the need for this template
// parameter name.
template <typename MessageType_WorkAroundCppLookupDefect>
inline size_t WireFormatLite::GroupSizeNoVirtual(
    const MessageType_WorkAroundCppLookupDefect& value) {
  return value.MessageType_WorkAroundCppLookupDefect::ByteSizeLong();
}
template <typename MessageType_WorkAroundCppLookupDefect>
inline size_t WireFormatLite::MessageSizeNoVirtual(
    const MessageType_WorkAroundCppLookupDefect& value) {
  return LengthDelimitedSize(
      value.MessageType_WorkAroundCppLookupDefect::ByteSizeLong());
}

inline size_t WireFormatLite::LengthDelimitedSize(size_t length) {
  // The static_cast here prevents an error in certain compiler configurations
  // but is not technically correct--if length is too large to fit in a uint32_t
  // then it will be silently truncated. We will need to fix this if we ever
  // decide to start supporting serialized messages greater than 2 GiB in size.
  return length +
         io::CodedOutputStream::VarintSize32(static_cast<uint32_t>(length));
}

template <typename MS>
bool ParseMessageSetItemImpl(io::CodedInputStream* input, MS ms) {
  // This method parses a group which should contain two fields:
  //   required int32 type_id = 2;
  //   required data message = 3;

  uint32_t last_type_id = 0;

  // If we see message data before the type_id, we'll append it to this so
  // we can parse it later.
  std::string message_data;

  enum class State { kNoTag, kHasType, kHasPayload, kDone };
  State state = State::kNoTag;

  while (true) {
    const uint32_t tag = input->ReadTagNoLastTag();
    if (tag == 0) return false;

    switch (tag) {
      case WireFormatLite::kMessageSetTypeIdTag: {
        uint32_t type_id;
        // We should fail parsing if type id is 0.
        if (!input->ReadVarint32(&type_id) || type_id == 0) return false;
        if (state == State::kNoTag) {
          last_type_id = type_id;
          state = State::kHasType;
        } else if (state == State::kHasPayload) {
          // We saw some message data before the type_id.  Have to parse it
          // now.
          io::CodedInputStream sub_input(
              reinterpret_cast<const uint8_t*>(message_data.data()),
              static_cast<int>(message_data.size()));
          sub_input.SetRecursionLimit(input->RecursionBudget());
          if (!ms.ParseField(type_id, &sub_input)) {
            return false;
          }
          message_data.clear();
          state = State::kDone;
        }

        break;
      }

      case WireFormatLite::kMessageSetMessageTag: {
        if (state == State::kHasType) {
          // Already saw type_id, so we can parse this directly.
          if (!ms.ParseField(last_type_id, input)) {
            return false;
          }
          state = State::kDone;
        } else if (state == State::kNoTag) {
          // We haven't seen a type_id yet.  Append this data to message_data.
          uint32_t length;
          if (!input->ReadVarint32(&length)) return false;
          if (static_cast<int32_t>(length) < 0) return false;
          uint32_t size = static_cast<uint32_t>(
              length + io::CodedOutputStream::VarintSize32(length));
          message_data.resize(size);
          auto ptr = reinterpret_cast<uint8_t*>(&message_data[0]);
          ptr = io::CodedOutputStream::WriteVarint32ToArray(length, ptr);
          if (!input->ReadRaw(ptr, length)) return false;
          state = State::kHasPayload;
        } else {
          if (!ms.SkipField(tag, input)) return false;
        }

        break;
      }

      case WireFormatLite::kMessageSetItemEndTag: {
        return true;
      }

      default: {
        if (!ms.SkipField(tag, input)) return false;
      }
    }
  }
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


#endif  // GOOGLE_PROTOBUF_WIRE_FORMAT_LITE_H__

