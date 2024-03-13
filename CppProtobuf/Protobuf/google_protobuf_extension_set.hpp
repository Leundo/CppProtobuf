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
// This header is logically internal, but is made public because it is used
// from protocol-compiler-generated code, which may reside in other components.

#ifndef GOOGLE_PROTOBUF_EXTENSION_SET_H__
#define GOOGLE_PROTOBUF_EXTENSION_SET_H__

#include <algorithm>
#include <cassert>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <CppProtobuf/google_protobuf_stubs_common.hpp>
#include <CppAbseil/absl_base_call_once.hpp>
#include <CppAbseil/absl_container_btree_map.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppProtobuf/google_protobuf_internal_visibility.hpp>
#include <CppProtobuf/google_protobuf_port.hpp>
#include <CppProtobuf/google_protobuf_io_coded_stream.hpp>
#include <CppProtobuf/google_protobuf_parse_context.hpp>
#include <CppProtobuf/google_protobuf_repeated_field.hpp>
#include <CppProtobuf/google_protobuf_wire_format_lite.hpp>

// clang-format off

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

// clang-format on

#ifdef SWIG
#error "You cannot SWIG proto headers"
#endif


namespace google {
namespace protobuf {
class Arena;
class Descriptor;       // descriptor.h
class FieldDescriptor;  // descriptor.h
class DescriptorPool;   // descriptor.h
class MessageLite;      // message_lite.h
class Message;          // message.h
class MessageFactory;   // message.h
class Reflection;       // message.h
class UnknownFieldSet;  // unknown_field_set.h
class FeatureSet;
namespace internal {
class FieldSkipper;  // wire_format_lite.h
class WireFormat;
}  // namespace internal
}  // namespace protobuf
}  // namespace google
namespace pb {
class CppFeatures;
}  // namespace pb

namespace google {
namespace protobuf {
namespace internal {

class InternalMetadata;

// Used to store values of type WireFormatLite::FieldType without having to
// #include wire_format_lite.h.  Also, ensures that we use only one byte to
// store these values, which is important to keep the layout of
// ExtensionSet::Extension small.
typedef uint8_t FieldType;

// A function which, given an integer value, returns true if the number
// matches one of the defined values for the corresponding enum type.  This
// is used with RegisterEnumExtension, below.
typedef bool EnumValidityFunc(int number);

// Version of the above which takes an argument.  This is needed to deal with
// extensions that are not compiled in.
typedef bool EnumValidityFuncWithArg(const void* arg, int number);

// Information about a registered extension.
struct ExtensionInfo {
  constexpr ExtensionInfo() : enum_validity_check() {}
  constexpr ExtensionInfo(const MessageLite* extendee, int param_number,
                          FieldType type_param, bool isrepeated, bool ispacked,
                          LazyEagerVerifyFnType verify_func)
      : message(extendee),
        number(param_number),
        type(type_param),
        is_repeated(isrepeated),
        is_packed(ispacked),
        enum_validity_check(),
        lazy_eager_verify_func(verify_func) {}

  const MessageLite* message = nullptr;
  int number = 0;

  FieldType type = 0;
  bool is_repeated = false;
  bool is_packed = false;

  struct EnumValidityCheck {
    EnumValidityFuncWithArg* func;
    const void* arg;
  };

  struct MessageInfo {
    const MessageLite* prototype;
  };

  union {
    EnumValidityCheck enum_validity_check;
    MessageInfo message_info;
  };

  // The descriptor for this extension, if one exists and is known.  May be
  // nullptr.  Must not be nullptr if the descriptor for the extension does not
  // live in the same pool as the descriptor for the containing type.
  const FieldDescriptor* descriptor = nullptr;

  // If this field is potentially lazy this function can be used as a cheap
  // verification of the raw bytes.
  // If nullptr then no verification is performed.
  LazyEagerVerifyFnType lazy_eager_verify_func = nullptr;
};

// An ExtensionFinder is an object which looks up extension definitions.  It
// must implement this method:
//
// bool Find(int number, ExtensionInfo* output);

// GeneratedExtensionFinder is an ExtensionFinder which finds extensions
// defined in .proto files which have been compiled into the binary.
class PROTOBUF_EXPORT GeneratedExtensionFinder {
 public:
  explicit GeneratedExtensionFinder(const MessageLite* extendee)
      : extendee_(extendee) {}

  // Returns true and fills in *output if found, otherwise returns false.
  bool Find(int number, ExtensionInfo* output);

 private:
  const MessageLite* extendee_;
};

// Note:  extension_set_heavy.cc defines DescriptorPoolExtensionFinder for
// finding extensions from a DescriptorPool.

// This is an internal helper class intended for use within the protocol buffer
// library and generated classes.  Clients should not use it directly.  Instead,
// use the generated accessors such as GetExtension() of the class being
// extended.
//
// This class manages extensions for a protocol message object.  The
// message's HasExtension(), GetExtension(), MutableExtension(), and
// ClearExtension() methods are just thin wrappers around the embedded
// ExtensionSet.  When parsing, if a tag number is encountered which is
// inside one of the message type's extension ranges, the tag is passed
// off to the ExtensionSet for parsing.  Etc.
class PROTOBUF_EXPORT ExtensionSet {
 public:
  constexpr ExtensionSet() : ExtensionSet(nullptr) {}
  ExtensionSet(const ExtensionSet& rhs) = delete;

  // Arena enabled constructors: for internal use only.
  ExtensionSet(internal::InternalVisibility, Arena* arena)
      : ExtensionSet(arena) {}

  // TODO: make constructor private, and migrate `ArenaInitialized`
  // to `InternalVisibility` overloaded constructor(s).
  explicit constexpr ExtensionSet(Arena* arena);
  ExtensionSet(ArenaInitialized, Arena* arena) : ExtensionSet(arena) {}

  ExtensionSet& operator=(const ExtensionSet&) = delete;
  ~ExtensionSet();

  // These are called at startup by protocol-compiler-generated code to
  // register known extensions.  The registrations are used by ParseField()
  // to look up extensions for parsed field numbers.  Note that dynamic parsing
  // does not use ParseField(); only protocol-compiler-generated parsing
  // methods do.
  static void RegisterExtension(const MessageLite* extendee, int number,
                                FieldType type, bool is_repeated,
                                bool is_packed,
                                LazyEagerVerifyFnType verify_func);
  static void RegisterEnumExtension(const MessageLite* extendee, int number,
                                    FieldType type, bool is_repeated,
                                    bool is_packed, EnumValidityFunc* is_valid);
  static void RegisterMessageExtension(const MessageLite* extendee, int number,
                                       FieldType type, bool is_repeated,
                                       bool is_packed,
                                       const MessageLite* prototype,
                                       LazyEagerVerifyFnType verify_func);

  // =================================================================

  // Add all fields which are currently present to the given vector.  This
  // is useful to implement Reflection::ListFields(). Descriptors are appended
  // in increasing tag order.
  void AppendToList(const Descriptor* extendee, const DescriptorPool* pool,
                    std::vector<const FieldDescriptor*>* output) const;

  // =================================================================
  // Accessors
  //
  // Generated message classes include type-safe templated wrappers around
  // these methods.  Generally you should use those rather than call these
  // directly, unless you are doing low-level memory management.
  //
  // When calling any of these accessors, the extension number requested
  // MUST exist in the DescriptorPool provided to the constructor.  Otherwise,
  // the method will fail an assert.  Normally, though, you would not call
  // these directly; you would either call the generated accessors of your
  // message class (e.g. GetExtension()) or you would call the accessors
  // of the reflection interface.  In both cases, it is impossible to
  // trigger this assert failure:  the generated accessors only accept
  // linked-in extension types as parameters, while the Reflection interface
  // requires you to provide the FieldDescriptor describing the extension.
  //
  // When calling any of these accessors, a protocol-compiler-generated
  // implementation of the extension corresponding to the number MUST
  // be linked in, and the FieldDescriptor used to refer to it MUST be
  // the one generated by that linked-in code.  Otherwise, the method will
  // die on an assert failure.  The message objects returned by the message
  // accessors are guaranteed to be of the correct linked-in type.
  //
  // These methods pretty much match Reflection except that:
  // - They're not virtual.
  // - They identify fields by number rather than FieldDescriptors.
  // - They identify enum values using integers rather than descriptors.
  // - Strings provide Mutable() in addition to Set() accessors.

  bool Has(int number) const;
  int ExtensionSize(int number) const;  // Size of a repeated extension.
  int NumExtensions() const;            // The number of extensions
  FieldType ExtensionType(int number) const;
  void ClearExtension(int number);

  // singular fields -------------------------------------------------

  int32_t GetInt32(int number, int32_t default_value) const;
  int64_t GetInt64(int number, int64_t default_value) const;
  uint32_t GetUInt32(int number, uint32_t default_value) const;
  uint64_t GetUInt64(int number, uint64_t default_value) const;
  float GetFloat(int number, float default_value) const;
  double GetDouble(int number, double default_value) const;
  bool GetBool(int number, bool default_value) const;
  int GetEnum(int number, int default_value) const;
  const std::string& GetString(int number,
                               const std::string& default_value) const;
  const MessageLite& GetMessage(int number,
                                const MessageLite& default_value) const;
  const MessageLite& GetMessage(int number, const Descriptor* message_type,
                                MessageFactory* factory) const;

  // |descriptor| may be nullptr so long as it is known that the descriptor for
  // the extension lives in the same pool as the descriptor for the containing
  // type.
#define desc const FieldDescriptor* descriptor  // avoid line wrapping
  void SetInt32(int number, FieldType type, int32_t value, desc);
  void SetInt64(int number, FieldType type, int64_t value, desc);
  void SetUInt32(int number, FieldType type, uint32_t value, desc);
  void SetUInt64(int number, FieldType type, uint64_t value, desc);
  void SetFloat(int number, FieldType type, float value, desc);
  void SetDouble(int number, FieldType type, double value, desc);
  void SetBool(int number, FieldType type, bool value, desc);
  void SetEnum(int number, FieldType type, int value, desc);
  void SetString(int number, FieldType type, std::string value, desc);
  std::string* MutableString(int number, FieldType type, desc);
  MessageLite* MutableMessage(int number, FieldType type,
                              const MessageLite& prototype, desc);
  MessageLite* MutableMessage(const FieldDescriptor* descriptor,
                              MessageFactory* factory);
  // Adds the given message to the ExtensionSet, taking ownership of the
  // message object. Existing message with the same number will be deleted.
  // If "message" is nullptr, this is equivalent to "ClearExtension(number)".
  void SetAllocatedMessage(int number, FieldType type,
                           const FieldDescriptor* descriptor,
                           MessageLite* message);
  void UnsafeArenaSetAllocatedMessage(int number, FieldType type,
                                      const FieldDescriptor* descriptor,
                                      MessageLite* message);
  PROTOBUF_NODISCARD MessageLite* ReleaseMessage(int number,
                                                 const MessageLite& prototype);
  MessageLite* UnsafeArenaReleaseMessage(int number,
                                         const MessageLite& prototype);

  PROTOBUF_NODISCARD MessageLite* ReleaseMessage(
      const FieldDescriptor* descriptor, MessageFactory* factory);
  MessageLite* UnsafeArenaReleaseMessage(const FieldDescriptor* descriptor,
                                         MessageFactory* factory);
#undef desc
  Arena* GetArena() const { return arena_; }

  // repeated fields -------------------------------------------------

  // Fetches a RepeatedField extension by number; returns |default_value|
  // if no such extension exists. User should not touch this directly; it is
  // used by the GetRepeatedExtension() method.
  const void* GetRawRepeatedField(int number, const void* default_value) const;
  // Fetches a mutable version of a RepeatedField extension by number,
  // instantiating one if none exists. Similar to above, user should not use
  // this directly; it underlies MutableRepeatedExtension().
  void* MutableRawRepeatedField(int number, FieldType field_type, bool packed,
                                const FieldDescriptor* desc);

  // This is an overload of MutableRawRepeatedField to maintain compatibility
  // with old code using a previous API. This version of
  // MutableRawRepeatedField() will ABSL_CHECK-fail on a missing extension.
  // (E.g.: borg/clients/internal/proto1/proto2_reflection.cc.)
  void* MutableRawRepeatedField(int number);

  int32_t GetRepeatedInt32(int number, int index) const;
  int64_t GetRepeatedInt64(int number, int index) const;
  uint32_t GetRepeatedUInt32(int number, int index) const;
  uint64_t GetRepeatedUInt64(int number, int index) const;
  float GetRepeatedFloat(int number, int index) const;
  double GetRepeatedDouble(int number, int index) const;
  bool GetRepeatedBool(int number, int index) const;
  int GetRepeatedEnum(int number, int index) const;
  const std::string& GetRepeatedString(int number, int index) const;
  const MessageLite& GetRepeatedMessage(int number, int index) const;

  void SetRepeatedInt32(int number, int index, int32_t value);
  void SetRepeatedInt64(int number, int index, int64_t value);
  void SetRepeatedUInt32(int number, int index, uint32_t value);
  void SetRepeatedUInt64(int number, int index, uint64_t value);
  void SetRepeatedFloat(int number, int index, float value);
  void SetRepeatedDouble(int number, int index, double value);
  void SetRepeatedBool(int number, int index, bool value);
  void SetRepeatedEnum(int number, int index, int value);
  void SetRepeatedString(int number, int index, std::string value);
  std::string* MutableRepeatedString(int number, int index);
  MessageLite* MutableRepeatedMessage(int number, int index);

#define desc const FieldDescriptor* descriptor  // avoid line wrapping
  void AddInt32(int number, FieldType type, bool packed, int32_t value, desc);
  void AddInt64(int number, FieldType type, bool packed, int64_t value, desc);
  void AddUInt32(int number, FieldType type, bool packed, uint32_t value, desc);
  void AddUInt64(int number, FieldType type, bool packed, uint64_t value, desc);
  void AddFloat(int number, FieldType type, bool packed, float value, desc);
  void AddDouble(int number, FieldType type, bool packed, double value, desc);
  void AddBool(int number, FieldType type, bool packed, bool value, desc);
  void AddEnum(int number, FieldType type, bool packed, int value, desc);
  void AddString(int number, FieldType type, std::string value, desc);
  std::string* AddString(int number, FieldType type, desc);
  MessageLite* AddMessage(int number, FieldType type,
                          const MessageLite& prototype, desc);
  MessageLite* AddMessage(const FieldDescriptor* descriptor,
                          MessageFactory* factory);
  void AddAllocatedMessage(const FieldDescriptor* descriptor,
                           MessageLite* new_entry);
  void UnsafeArenaAddAllocatedMessage(const FieldDescriptor* descriptor,
                                      MessageLite* new_entry);
#undef desc

  void RemoveLast(int number);
  PROTOBUF_NODISCARD MessageLite* ReleaseLast(int number);
  MessageLite* UnsafeArenaReleaseLast(int number);
  void SwapElements(int number, int index1, int index2);

  // =================================================================
  // convenience methods for implementing methods of Message
  //
  // These could all be implemented in terms of the other methods of this
  // class, but providing them here helps keep the generated code size down.

  void Clear();
  void MergeFrom(const MessageLite* extendee, const ExtensionSet& other);
  void Swap(const MessageLite* extendee, ExtensionSet* other);
  void InternalSwap(ExtensionSet* other);
  void SwapExtension(const MessageLite* extendee, ExtensionSet* other,
                     int number);
  void UnsafeShallowSwapExtension(ExtensionSet* other, int number);
  bool IsInitialized(const MessageLite* extendee) const;

  // Lite parser
  const char* ParseField(uint64_t tag, const char* ptr,
                         const MessageLite* extendee,
                         internal::InternalMetadata* metadata,
                         internal::ParseContext* ctx);
  // Full parser
  const char* ParseField(uint64_t tag, const char* ptr, const Message* extendee,
                         internal::InternalMetadata* metadata,
                         internal::ParseContext* ctx);
  template <typename Msg>
  const char* ParseMessageSet(const char* ptr, const Msg* extendee,
                              InternalMetadata* metadata,
                              internal::ParseContext* ctx) {
    struct MessageSetItem {
      const char* _InternalParse(const char* ptr, ParseContext* ctx) {
        return me->ParseMessageSetItem(ptr, extendee, metadata, ctx);
      }
      ExtensionSet* me;
      const Msg* extendee;
      InternalMetadata* metadata;
    } item{this, extendee, metadata};
    while (!ctx->Done(&ptr)) {
      uint32_t tag;
      ptr = ReadTag(ptr, &tag);
      GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
      if (tag == WireFormatLite::kMessageSetItemStartTag) {
        ptr = ctx->ParseGroup(&item, ptr, tag);
        GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
      } else {
        if (tag == 0 || (tag & 7) == 4) {
          ctx->SetLastTag(tag);
          return ptr;
        }
        ptr = ParseField(tag, ptr, extendee, metadata, ctx);
        GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
      }
    }
    return ptr;
  }

  // Write all extension fields with field numbers in the range
  //   [start_field_number, end_field_number)
  // to the output stream, using the cached sizes computed when ByteSize() was
  // last called.  Note that the range bounds are inclusive-exclusive.
  void SerializeWithCachedSizes(const MessageLite* extendee,
                                int start_field_number, int end_field_number,
                                io::CodedOutputStream* output) const {
    output->SetCur(_InternalSerialize(extendee, start_field_number,
                                      end_field_number, output->Cur(),
                                      output->EpsCopy()));
  }

  // Same as SerializeWithCachedSizes, but without any bounds checking.
  // The caller must ensure that target has sufficient capacity for the
  // serialized extensions.
  //
  // Returns a pointer past the last written byte.

  uint8_t* _InternalSerialize(const MessageLite* extendee,
                              int start_field_number, int end_field_number,
                              uint8_t* target,
                              io::EpsCopyOutputStream* stream) const {
    if (flat_size_ == 0) {
      assert(!is_large());
      return target;
    }
    return _InternalSerializeImpl(extendee, start_field_number,
                                  end_field_number, target, stream);
  }

  // Like above but serializes in MessageSet format.
  void SerializeMessageSetWithCachedSizes(const MessageLite* extendee,
                                          io::CodedOutputStream* output) const {
    output->SetCur(InternalSerializeMessageSetWithCachedSizesToArray(
        extendee, output->Cur(), output->EpsCopy()));
  }
  uint8_t* InternalSerializeMessageSetWithCachedSizesToArray(
      const MessageLite* extendee, uint8_t* target,
      io::EpsCopyOutputStream* stream) const;

  // For backward-compatibility, versions of two of the above methods that
  // serialize deterministically iff SetDefaultSerializationDeterministic()
  // has been called.
  uint8_t* SerializeWithCachedSizesToArray(int start_field_number,
                                           int end_field_number,
                                           uint8_t* target) const;
  uint8_t* SerializeMessageSetWithCachedSizesToArray(
      const MessageLite* extendee, uint8_t* target) const;

  // Returns the total serialized size of all the extensions.
  size_t ByteSize() const;

  // Like ByteSize() but uses MessageSet format.
  size_t MessageSetByteSize() const;

  // Returns (an estimate of) the total number of bytes used for storing the
  // extensions in memory, excluding sizeof(*this).  If the ExtensionSet is
  // for a lite message (and thus possibly contains lite messages), the results
  // are undefined (might work, might crash, might corrupt data, might not even
  // be linked in).  It's up to the protocol compiler to avoid calling this on
  // such ExtensionSets (easy enough since lite messages don't implement
  // SpaceUsed()).
  size_t SpaceUsedExcludingSelfLong() const;

  // This method just calls SpaceUsedExcludingSelfLong() but it can not be
  // inlined because the definition of SpaceUsedExcludingSelfLong() is not
  // included in lite runtime and when an inline method refers to it MSVC
  // will complain about unresolved symbols when building the lite runtime
  // as .dll.
  int SpaceUsedExcludingSelf() const;

 private:
  template <typename Type>
  friend class PrimitiveTypeTraits;

  template <typename Type>
  friend class RepeatedPrimitiveTypeTraits;

  template <typename Type, bool IsValid(int)>
  friend class EnumTypeTraits;

  template <typename Type, bool IsValid(int)>
  friend class RepeatedEnumTypeTraits;

  friend class google::protobuf::Reflection;
  friend class google::protobuf::internal::WireFormat;

  const int32_t& GetRefInt32(int number, const int32_t& default_value) const;
  const int64_t& GetRefInt64(int number, const int64_t& default_value) const;
  const uint32_t& GetRefUInt32(int number, const uint32_t& default_value) const;
  const uint64_t& GetRefUInt64(int number, const uint64_t& default_value) const;
  const float& GetRefFloat(int number, const float& default_value) const;
  const double& GetRefDouble(int number, const double& default_value) const;
  const bool& GetRefBool(int number, const bool& default_value) const;
  const int& GetRefEnum(int number, const int& default_value) const;
  const int32_t& GetRefRepeatedInt32(int number, int index) const;
  const int64_t& GetRefRepeatedInt64(int number, int index) const;
  const uint32_t& GetRefRepeatedUInt32(int number, int index) const;
  const uint64_t& GetRefRepeatedUInt64(int number, int index) const;
  const float& GetRefRepeatedFloat(int number, int index) const;
  const double& GetRefRepeatedDouble(int number, int index) const;
  const bool& GetRefRepeatedBool(int number, int index) const;
  const int& GetRefRepeatedEnum(int number, int index) const;

  // Implementation of _InternalSerialize for non-empty map_.
  uint8_t* _InternalSerializeImpl(const MessageLite* extendee,
                                  int start_field_number, int end_field_number,
                                  uint8_t* target,
                                  io::EpsCopyOutputStream* stream) const;
  // Interface of a lazily parsed singular message extension.
  class PROTOBUF_EXPORT LazyMessageExtension {
   public:
    LazyMessageExtension() = default;
    LazyMessageExtension(const LazyMessageExtension&) = delete;
    LazyMessageExtension& operator=(const LazyMessageExtension&) = delete;
    virtual ~LazyMessageExtension() = default;

    virtual LazyMessageExtension* New(Arena* arena) const = 0;
    virtual const MessageLite& GetMessage(const MessageLite& prototype,
                                          Arena* arena) const = 0;
    virtual MessageLite* MutableMessage(const MessageLite& prototype,
                                        Arena* arena) = 0;
    virtual void SetAllocatedMessage(MessageLite* message, Arena* arena) = 0;
    virtual void UnsafeArenaSetAllocatedMessage(MessageLite* message,
                                                Arena* arena) = 0;
    PROTOBUF_NODISCARD virtual MessageLite* ReleaseMessage(
        const MessageLite& prototype, Arena* arena) = 0;
    virtual MessageLite* UnsafeArenaReleaseMessage(const MessageLite& prototype,
                                                   Arena* arena) = 0;

    virtual bool IsInitialized(const MessageLite* prototype,
                               Arena* arena) const = 0;
    virtual bool IsEagerSerializeSafe(const MessageLite* prototype,
                                      Arena* arena) const = 0;

    [[deprecated("Please use ByteSizeLong() instead")]] virtual int ByteSize()
        const {
      return internal::ToIntSize(ByteSizeLong());
    }
    virtual size_t ByteSizeLong() const = 0;
    virtual size_t SpaceUsedLong() const = 0;

    virtual void MergeFrom(const MessageLite* prototype,
                           const LazyMessageExtension& other, Arena* arena) = 0;
    virtual void MergeFromMessage(const MessageLite& msg, Arena* arena) = 0;
    virtual void Clear() = 0;

    virtual const char* _InternalParse(const MessageLite& prototype,
                                       Arena* arena, const char* ptr,
                                       ParseContext* ctx) = 0;
    virtual uint8_t* WriteMessageToArray(
        const MessageLite* prototype, int number, uint8_t* target,
        io::EpsCopyOutputStream* stream) const = 0;

   private:
    virtual void UnusedKeyMethod();  // Dummy key method to avoid weak vtable.
  };
  // Give access to function defined below to see LazyMessageExtension.
  friend LazyMessageExtension* MaybeCreateLazyExtension(Arena* arena);
  struct Extension {
    // The order of these fields packs Extension into 24 bytes when using 8
    // byte alignment. Consider this when adding or removing fields here.
    union {
      int32_t int32_t_value;
      int64_t int64_t_value;
      uint32_t uint32_t_value;
      uint64_t uint64_t_value;
      float float_value;
      double double_value;
      bool bool_value;
      int enum_value;
      std::string* string_value;
      MessageLite* message_value;
      LazyMessageExtension* lazymessage_value;

      RepeatedField<int32_t>* repeated_int32_t_value;
      RepeatedField<int64_t>* repeated_int64_t_value;
      RepeatedField<uint32_t>* repeated_uint32_t_value;
      RepeatedField<uint64_t>* repeated_uint64_t_value;
      RepeatedField<float>* repeated_float_value;
      RepeatedField<double>* repeated_double_value;
      RepeatedField<bool>* repeated_bool_value;
      RepeatedField<int>* repeated_enum_value;
      RepeatedPtrField<std::string>* repeated_string_value;
      RepeatedPtrField<MessageLite>* repeated_message_value;
    };

    FieldType type;
    bool is_repeated;

    // For singular types, indicates if the extension is "cleared".  This
    // happens when an extension is set and then later cleared by the caller.
    // We want to keep the Extension object around for reuse, so instead of
    // removing it from the map, we just set is_cleared = true.  This has no
    // meaning for repeated types; for those, the size of the RepeatedField
    // simply becomes zero when cleared.
    bool is_cleared : 4;

    // For singular message types, indicates whether lazy parsing is enabled
    // for this extension. This field is only valid when type == TYPE_MESSAGE
    // and !is_repeated because we only support lazy parsing for singular
    // message types currently. If is_lazy = true, the extension is stored in
    // lazymessage_value. Otherwise, the extension will be message_value.
    bool is_lazy : 4;

    // For repeated types, this indicates if the [packed=true] option is set.
    bool is_packed;

    // For packed fields, the size of the packed data is recorded here when
    // ByteSize() is called then used during serialization.
    // TODO:  Use atomic<int> when C++ supports it.
    mutable int cached_size;

    // The descriptor for this extension, if one exists and is known.  May be
    // nullptr.  Must not be nullptr if the descriptor for the extension does
    // not live in the same pool as the descriptor for the containing type.
    const FieldDescriptor* descriptor;

    // Some helper methods for operations on a single Extension.
    uint8_t* InternalSerializeFieldWithCachedSizesToArray(
        const MessageLite* extendee, const ExtensionSet* extension_set,
        int number, uint8_t* target, io::EpsCopyOutputStream* stream) const;
    uint8_t* InternalSerializeMessageSetItemWithCachedSizesToArray(
        const MessageLite* extendee, const ExtensionSet* extension_set,
        int number, uint8_t* target, io::EpsCopyOutputStream* stream) const;
    size_t ByteSize(int number) const;
    size_t MessageSetItemByteSize(int number) const;
    void Clear();
    int GetSize() const;
    void Free();
    size_t SpaceUsedExcludingSelfLong() const;
    bool IsInitialized(const ExtensionSet* ext_set, const MessageLite* extendee,
                       int number, Arena* arena) const;
  };

  // The Extension struct is small enough to be passed by value, so we use it
  // directly as the value type in mappings rather than use pointers.  We use
  // sorted maps rather than hash-maps because we expect most ExtensionSets will
  // only contain a small number of extension.  Also, we want AppendToList and
  // deterministic serialization to order fields by field number.

  struct KeyValue {
    int first;
    Extension second;

    struct FirstComparator {
      bool operator()(const KeyValue& lhs, const KeyValue& rhs) const {
        return lhs.first < rhs.first;
      }
      bool operator()(const KeyValue& lhs, int key) const {
        return lhs.first < key;
      }
      bool operator()(int key, const KeyValue& rhs) const {
        return key < rhs.first;
      }
    };
  };

  using LargeMap = absl::btree_map<int, Extension>;

  // Wrapper API that switches between flat-map and LargeMap.

  // Finds a key (if present) in the ExtensionSet.
  const Extension* FindOrNull(int key) const;
  Extension* FindOrNull(int key);

  // Helper-functions that only inspect the LargeMap.
  const Extension* FindOrNullInLargeMap(int key) const;
  Extension* FindOrNullInLargeMap(int key);

  // Inserts a new (key, Extension) into the ExtensionSet (and returns true), or
  // finds the already-existing Extension for that key (returns false).
  // The Extension* will point to the new-or-found Extension.
  std::pair<Extension*, bool> Insert(int key);

  // Grows the flat_capacity_.
  // If flat_capacity_ > kMaximumFlatCapacity, converts to LargeMap.
  void GrowCapacity(size_t minimum_new_capacity);
  static constexpr uint16_t kMaximumFlatCapacity = 256;
  bool is_large() const { return static_cast<int16_t>(flat_size_) < 0; }

  // Removes a key from the ExtensionSet.
  void Erase(int key);

  size_t Size() const {
    return PROTOBUF_PREDICT_FALSE(is_large()) ? map_.large->size() : flat_size_;
  }

  // Similar to std::for_each.
  // Each Iterator is decomposed into ->first and ->second fields, so
  // that the KeyValueFunctor can be agnostic vis-a-vis KeyValue-vs-std::pair.
  template <typename Iterator, typename KeyValueFunctor>
  static KeyValueFunctor ForEach(Iterator begin, Iterator end,
                                 KeyValueFunctor func) {
    for (Iterator it = begin; it != end; ++it) func(it->first, it->second);
    return std::move(func);
  }

  // Applies a functor to the <int, Extension&> pairs in sorted order.
  template <typename KeyValueFunctor>
  KeyValueFunctor ForEach(KeyValueFunctor func) {
    if (PROTOBUF_PREDICT_FALSE(is_large())) {
      return ForEach(map_.large->begin(), map_.large->end(), std::move(func));
    }
    return ForEach(flat_begin(), flat_end(), std::move(func));
  }

  // Applies a functor to the <int, const Extension&> pairs in sorted order.
  template <typename KeyValueFunctor>
  KeyValueFunctor ForEach(KeyValueFunctor func) const {
    if (PROTOBUF_PREDICT_FALSE(is_large())) {
      return ForEach(map_.large->begin(), map_.large->end(), std::move(func));
    }
    return ForEach(flat_begin(), flat_end(), std::move(func));
  }

  // Merges existing Extension from other_extension
  void InternalExtensionMergeFrom(const MessageLite* extendee, int number,
                                  const Extension& other_extension,
                                  Arena* other_arena);

  inline static bool is_packable(WireFormatLite::WireType type) {
    switch (type) {
      case WireFormatLite::WIRETYPE_VARINT:
      case WireFormatLite::WIRETYPE_FIXED64:
      case WireFormatLite::WIRETYPE_FIXED32:
        return true;
      case WireFormatLite::WIRETYPE_LENGTH_DELIMITED:
      case WireFormatLite::WIRETYPE_START_GROUP:
      case WireFormatLite::WIRETYPE_END_GROUP:
        return false;

        // Do not add a default statement. Let the compiler complain when
        // someone
        // adds a new wire type.
    }
    PROTOBUF_ASSUME(false);  // switch handles all possible enum values
    return false;
  }

  // Returns true and fills field_number and extension if extension is found.
  // Note to support packed repeated field compatibility, it also fills whether
  // the tag on wire is packed, which can be different from
  // extension->is_packed (whether packed=true is specified).
  template <typename ExtensionFinder>
  bool FindExtensionInfoFromTag(uint32_t tag, ExtensionFinder* extension_finder,
                                int* field_number, ExtensionInfo* extension,
                                bool* was_packed_on_wire) {
    *field_number = WireFormatLite::GetTagFieldNumber(tag);
    WireFormatLite::WireType wire_type = WireFormatLite::GetTagWireType(tag);
    return FindExtensionInfoFromFieldNumber(wire_type, *field_number,
                                            extension_finder, extension,
                                            was_packed_on_wire);
  }

  // Returns true and fills extension if extension is found.
  // Note to support packed repeated field compatibility, it also fills whether
  // the tag on wire is packed, which can be different from
  // extension->is_packed (whether packed=true is specified).
  template <typename ExtensionFinder>
  bool FindExtensionInfoFromFieldNumber(int wire_type, int field_number,
                                        ExtensionFinder* extension_finder,
                                        ExtensionInfo* extension,
                                        bool* was_packed_on_wire) const {
    if (!extension_finder->Find(field_number, extension)) {
      return false;
    }

    ABSL_DCHECK(extension->type > 0 &&
                extension->type <= WireFormatLite::MAX_FIELD_TYPE);
    auto real_type = static_cast<WireFormatLite::FieldType>(extension->type);

    WireFormatLite::WireType expected_wire_type =
        WireFormatLite::WireTypeForFieldType(real_type);

    // Check if this is a packed field.
    *was_packed_on_wire = false;
    if (extension->is_repeated &&
        wire_type == WireFormatLite::WIRETYPE_LENGTH_DELIMITED &&
        is_packable(expected_wire_type)) {
      *was_packed_on_wire = true;
      return true;
    }
    // Otherwise the wire type must match.
    return expected_wire_type == wire_type;
  }

  // Find the prototype for a LazyMessage from the extension registry. Returns
  // null if the extension is not found.
  const MessageLite* GetPrototypeForLazyMessage(const MessageLite* extendee,
                                                int number) const;

  // Returns true if extension is present and lazy.
  bool HasLazy(int number) const;

  // Gets the extension with the given number, creating it if it does not
  // already exist.  Returns true if the extension did not already exist.
  bool MaybeNewExtension(int number, const FieldDescriptor* descriptor,
                         Extension** result);

  // Gets the repeated extension for the given descriptor, creating it if
  // it does not exist.
  Extension* MaybeNewRepeatedExtension(const FieldDescriptor* descriptor);

  bool FindExtension(int wire_type, uint32_t field, const MessageLite* extendee,
                     const internal::ParseContext* /*ctx*/,
                     ExtensionInfo* extension, bool* was_packed_on_wire) {
    GeneratedExtensionFinder finder(extendee);
    return FindExtensionInfoFromFieldNumber(wire_type, field, &finder,
                                            extension, was_packed_on_wire);
  }
  inline bool FindExtension(int wire_type, uint32_t field,
                            const Message* extendee,
                            const internal::ParseContext* ctx,
                            ExtensionInfo* extension, bool* was_packed_on_wire);
  // Used for MessageSet only
  const char* ParseFieldMaybeLazily(uint64_t tag, const char* ptr,
                                    const MessageLite* extendee,
                                    internal::InternalMetadata* metadata,
                                    internal::ParseContext* ctx) {
    // Lite MessageSet doesn't implement lazy.
    return ParseField(tag, ptr, extendee, metadata, ctx);
  }
  const char* ParseFieldMaybeLazily(uint64_t tag, const char* ptr,
                                    const Message* extendee,
                                    internal::InternalMetadata* metadata,
                                    internal::ParseContext* ctx);
  const char* ParseMessageSetItem(const char* ptr, const MessageLite* extendee,
                                  internal::InternalMetadata* metadata,
                                  internal::ParseContext* ctx);
  const char* ParseMessageSetItem(const char* ptr, const Message* extendee,
                                  internal::InternalMetadata* metadata,
                                  internal::ParseContext* ctx);

  // Implemented in extension_set_inl.h to keep code out of the header file.
  template <typename T>
  const char* ParseFieldWithExtensionInfo(int number, bool was_packed_on_wire,
                                          const ExtensionInfo& info,
                                          internal::InternalMetadata* metadata,
                                          const char* ptr,
                                          internal::ParseContext* ctx);
  template <typename Msg, typename T>
  const char* ParseMessageSetItemTmpl(const char* ptr, const Msg* extendee,
                                      internal::InternalMetadata* metadata,
                                      internal::ParseContext* ctx);

  // Hack:  RepeatedPtrFieldBase declares ExtensionSet as a friend.  This
  //   friendship should automatically extend to ExtensionSet::Extension, but
  //   unfortunately some older compilers (e.g. GCC 3.4.4) do not implement this
  //   correctly.  So, we must provide helpers for calling methods of that
  //   class.

  // Defined in extension_set_heavy.cc.
  static inline size_t RepeatedMessage_SpaceUsedExcludingSelfLong(
      RepeatedPtrFieldBase* field);

  KeyValue* flat_begin() {
    assert(!is_large());
    return map_.flat;
  }
  const KeyValue* flat_begin() const {
    assert(!is_large());
    return map_.flat;
  }
  KeyValue* flat_end() {
    assert(!is_large());
    return map_.flat + flat_size_;
  }
  const KeyValue* flat_end() const {
    assert(!is_large());
    return map_.flat + flat_size_;
  }

  Arena* arena_;

  // Manual memory-management:
  // map_.flat is an allocated array of flat_capacity_ elements.
  // [map_.flat, map_.flat + flat_size_) is the currently-in-use prefix.
  uint16_t flat_capacity_;
  uint16_t flat_size_;  // negative int16_t(flat_size_) indicates is_large()
  union AllocatedData {
    KeyValue* flat;

    // If flat_capacity_ > kMaximumFlatCapacity, switch to LargeMap,
    // which guarantees O(n lg n) CPU but larger constant factors.
    LargeMap* large;
  } map_;

  static void DeleteFlatMap(const KeyValue* flat, uint16_t flat_capacity);
};

constexpr ExtensionSet::ExtensionSet(Arena* arena)
    : arena_(arena), flat_capacity_(0), flat_size_(0), map_{nullptr} {}

// These are just for convenience...
inline void ExtensionSet::SetString(int number, FieldType type,
                                    std::string value,
                                    const FieldDescriptor* descriptor) {
  MutableString(number, type, descriptor)->assign(std::move(value));
}
inline void ExtensionSet::SetRepeatedString(int number, int index,
                                            std::string value) {
  MutableRepeatedString(number, index)->assign(std::move(value));
}
inline void ExtensionSet::AddString(int number, FieldType type,
                                    std::string value,
                                    const FieldDescriptor* descriptor) {
  AddString(number, type, descriptor)->assign(std::move(value));
}
// ===================================================================
// Glue for generated extension accessors

// -------------------------------------------------------------------
// Template magic

// First we have a set of classes representing "type traits" for different
// field types.  A type traits class knows how to implement basic accessors
// for extensions of a particular type given an ExtensionSet.  The signature
// for a type traits class looks like this:
//
//   class TypeTraits {
//    public:
//     typedef ? ConstType;
//     typedef ? MutableType;
//     // TypeTraits for singular fields and repeated fields will define the
//     // symbol "Singular" or "Repeated" respectively. These two symbols will
//     // be used in extension accessors to distinguish between singular
//     // extensions and repeated extensions. If the TypeTraits for the passed
//     // in extension doesn't have the expected symbol defined, it means the
//     // user is passing a repeated extension to a singular accessor, or the
//     // opposite. In that case the C++ compiler will generate an error
//     // message "no matching member function" to inform the user.
//     typedef ? Singular
//     typedef ? Repeated
//
//     static inline ConstType Get(int number, const ExtensionSet& set);
//     static inline void Set(int number, ConstType value, ExtensionSet* set);
//     static inline MutableType Mutable(int number, ExtensionSet* set);
//
//     // Variants for repeated fields.
//     static inline ConstType Get(int number, const ExtensionSet& set,
//                                 int index);
//     static inline void Set(int number, int index,
//                            ConstType value, ExtensionSet* set);
//     static inline MutableType Mutable(int number, int index,
//                                       ExtensionSet* set);
//     static inline void Add(int number, ConstType value, ExtensionSet* set);
//     static inline MutableType Add(int number, ExtensionSet* set);
//     This is used by the ExtensionIdentifier constructor to register
//     the extension at dynamic initialization.
//     template <typename ExtendeeT>
//     static void Register(int number, FieldType type, bool is_packed);
//   };
//
// Not all of these methods make sense for all field types.  For example, the
// "Mutable" methods only make sense for strings and messages, and the
// repeated methods only make sense for repeated types.  So, each type
// traits class implements only the set of methods from this signature that it
// actually supports.  This will cause a compiler error if the user tries to
// access an extension using a method that doesn't make sense for its type.
// For example, if "foo" is an extension of type "optional int32", then if you
// try to write code like:
//   my_message.MutableExtension(foo)
// you will get a compile error because PrimitiveTypeTraits<int32_t> does not
// have a "Mutable()" method.

// -------------------------------------------------------------------
// PrimitiveTypeTraits

// Since the ExtensionSet has different methods for each primitive type,
// we must explicitly define the methods of the type traits class for each
// known type.
template <typename Type>
class PrimitiveTypeTraits {
 public:
  typedef Type ConstType;
  typedef Type MutableType;
  typedef PrimitiveTypeTraits<Type> Singular;
  static constexpr bool kLifetimeBound = false;

  static inline ConstType Get(int number, const ExtensionSet& set,
                              ConstType default_value);

  static inline const ConstType* GetPtr(int number, const ExtensionSet& set,
                                        const ConstType& default_value);
  static inline void Set(int number, FieldType field_type, ConstType value,
                         ExtensionSet* set);
  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType verify_func) {
    ExtensionSet::RegisterExtension(&ExtendeeT::default_instance(), number,
                                    type, false, is_packed, verify_func);
  }
};

template <typename Type>
class RepeatedPrimitiveTypeTraits {
 public:
  typedef Type ConstType;
  typedef Type MutableType;
  typedef RepeatedPrimitiveTypeTraits<Type> Repeated;
  static constexpr bool kLifetimeBound = false;

  typedef RepeatedField<Type> RepeatedFieldType;

  static inline Type Get(int number, const ExtensionSet& set, int index);
  static inline const Type* GetPtr(int number, const ExtensionSet& set,
                                   int index);
  static inline const RepeatedField<ConstType>* GetRepeatedPtr(
      int number, const ExtensionSet& set);
  static inline void Set(int number, int index, Type value, ExtensionSet* set);
  static inline void Add(int number, FieldType field_type, bool is_packed,
                         Type value, ExtensionSet* set);

  static inline const RepeatedField<ConstType>& GetRepeated(
      int number, const ExtensionSet& set);
  static inline RepeatedField<Type>* MutableRepeated(int number,
                                                     FieldType field_type,
                                                     bool is_packed,
                                                     ExtensionSet* set);

  static const RepeatedFieldType* GetDefaultRepeatedField();
  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType verify_func) {
    ExtensionSet::RegisterExtension(&ExtendeeT::default_instance(), number,
                                    type, true, is_packed, verify_func);
  }
};

class PROTOBUF_EXPORT RepeatedPrimitiveDefaults {
 private:
  template <typename Type>
  friend class RepeatedPrimitiveTypeTraits;
  static const RepeatedPrimitiveDefaults* default_instance();
  RepeatedField<int32_t> default_repeated_field_int32_t_;
  RepeatedField<int64_t> default_repeated_field_int64_t_;
  RepeatedField<uint32_t> default_repeated_field_uint32_t_;
  RepeatedField<uint64_t> default_repeated_field_uint64_t_;
  RepeatedField<double> default_repeated_field_double_;
  RepeatedField<float> default_repeated_field_float_;
  RepeatedField<bool> default_repeated_field_bool_;
};

#define PROTOBUF_DEFINE_PRIMITIVE_TYPE(TYPE, METHOD)                           \
  template <>                                                                  \
  inline TYPE PrimitiveTypeTraits<TYPE>::Get(                                  \
      int number, const ExtensionSet& set, TYPE default_value) {               \
    return set.Get##METHOD(number, default_value);                             \
  }                                                                            \
  template <>                                                                  \
  inline const TYPE* PrimitiveTypeTraits<TYPE>::GetPtr(                        \
      int number, const ExtensionSet& set, const TYPE& default_value) {        \
    return &set.GetRef##METHOD(number, default_value);                         \
  }                                                                            \
  template <>                                                                  \
  inline void PrimitiveTypeTraits<TYPE>::Set(int number, FieldType field_type, \
                                             TYPE value, ExtensionSet* set) {  \
    set->Set##METHOD(number, field_type, value, nullptr);                      \
  }                                                                            \
                                                                               \
  template <>                                                                  \
  inline TYPE RepeatedPrimitiveTypeTraits<TYPE>::Get(                          \
      int number, const ExtensionSet& set, int index) {                        \
    return set.GetRepeated##METHOD(number, index);                             \
  }                                                                            \
  template <>                                                                  \
  inline const TYPE* RepeatedPrimitiveTypeTraits<TYPE>::GetPtr(                \
      int number, const ExtensionSet& set, int index) {                        \
    return &set.GetRefRepeated##METHOD(number, index);                         \
  }                                                                            \
  template <>                                                                  \
  inline void RepeatedPrimitiveTypeTraits<TYPE>::Set(                          \
      int number, int index, TYPE value, ExtensionSet* set) {                  \
    set->SetRepeated##METHOD(number, index, value);                            \
  }                                                                            \
  template <>                                                                  \
  inline void RepeatedPrimitiveTypeTraits<TYPE>::Add(                          \
      int number, FieldType field_type, bool is_packed, TYPE value,            \
      ExtensionSet* set) {                                                     \
    set->Add##METHOD(number, field_type, is_packed, value, nullptr);           \
  }                                                                            \
  template <>                                                                  \
  inline const RepeatedField<TYPE>*                                            \
  RepeatedPrimitiveTypeTraits<TYPE>::GetDefaultRepeatedField() {               \
    return &RepeatedPrimitiveDefaults::default_instance()                      \
                ->default_repeated_field_##TYPE##_;                            \
  }                                                                            \
  template <>                                                                  \
  inline const RepeatedField<TYPE>&                                            \
  RepeatedPrimitiveTypeTraits<TYPE>::GetRepeated(int number,                   \
                                                 const ExtensionSet& set) {    \
    return *reinterpret_cast<const RepeatedField<TYPE>*>(                      \
        set.GetRawRepeatedField(number, GetDefaultRepeatedField()));           \
  }                                                                            \
  template <>                                                                  \
  inline const RepeatedField<TYPE>*                                            \
  RepeatedPrimitiveTypeTraits<TYPE>::GetRepeatedPtr(int number,                \
                                                    const ExtensionSet& set) { \
    return &GetRepeated(number, set);                                          \
  }                                                                            \
  template <>                                                                  \
  inline RepeatedField<TYPE>*                                                  \
  RepeatedPrimitiveTypeTraits<TYPE>::MutableRepeated(                          \
      int number, FieldType field_type, bool is_packed, ExtensionSet* set) {   \
    return reinterpret_cast<RepeatedField<TYPE>*>(                             \
        set->MutableRawRepeatedField(number, field_type, is_packed, nullptr)); \
  }

PROTOBUF_DEFINE_PRIMITIVE_TYPE(int32_t, Int32)
PROTOBUF_DEFINE_PRIMITIVE_TYPE(int64_t, Int64)
PROTOBUF_DEFINE_PRIMITIVE_TYPE(uint32_t, UInt32)
PROTOBUF_DEFINE_PRIMITIVE_TYPE(uint64_t, UInt64)
PROTOBUF_DEFINE_PRIMITIVE_TYPE(float, Float)
PROTOBUF_DEFINE_PRIMITIVE_TYPE(double, Double)
PROTOBUF_DEFINE_PRIMITIVE_TYPE(bool, Bool)

#undef PROTOBUF_DEFINE_PRIMITIVE_TYPE

// -------------------------------------------------------------------
// StringTypeTraits

// Strings support both Set() and Mutable().
class PROTOBUF_EXPORT StringTypeTraits {
 public:
  typedef const std::string& ConstType;
  typedef std::string* MutableType;
  typedef StringTypeTraits Singular;
  static constexpr bool kLifetimeBound = true;

  static inline const std::string& Get(int number, const ExtensionSet& set,
                                       ConstType default_value) {
    return set.GetString(number, default_value);
  }
  static inline const std::string* GetPtr(int number, const ExtensionSet& set,
                                          ConstType default_value) {
    return &Get(number, set, default_value);
  }
  static inline void Set(int number, FieldType field_type,
                         const std::string& value, ExtensionSet* set) {
    set->SetString(number, field_type, value, nullptr);
  }
  static inline std::string* Mutable(int number, FieldType field_type,
                                     ExtensionSet* set) {
    return set->MutableString(number, field_type, nullptr);
  }
  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType verify_func) {
    ExtensionSet::RegisterExtension(&ExtendeeT::default_instance(), number,
                                    type, false, is_packed, verify_func);
  }
};

class PROTOBUF_EXPORT RepeatedStringTypeTraits {
 public:
  typedef const std::string& ConstType;
  typedef std::string* MutableType;
  typedef RepeatedStringTypeTraits Repeated;
  static constexpr bool kLifetimeBound = true;

  typedef RepeatedPtrField<std::string> RepeatedFieldType;

  static inline const std::string& Get(int number, const ExtensionSet& set,
                                       int index) {
    return set.GetRepeatedString(number, index);
  }
  static inline const std::string* GetPtr(int number, const ExtensionSet& set,
                                          int index) {
    return &Get(number, set, index);
  }
  static inline const RepeatedPtrField<std::string>* GetRepeatedPtr(
      int number, const ExtensionSet& set) {
    return &GetRepeated(number, set);
  }
  static inline void Set(int number, int index, const std::string& value,
                         ExtensionSet* set) {
    set->SetRepeatedString(number, index, value);
  }
  static inline std::string* Mutable(int number, int index, ExtensionSet* set) {
    return set->MutableRepeatedString(number, index);
  }
  static inline void Add(int number, FieldType field_type, bool /*is_packed*/,
                         const std::string& value, ExtensionSet* set) {
    set->AddString(number, field_type, value, nullptr);
  }
  static inline std::string* Add(int number, FieldType field_type,
                                 ExtensionSet* set) {
    return set->AddString(number, field_type, nullptr);
  }
  static inline const RepeatedPtrField<std::string>& GetRepeated(
      int number, const ExtensionSet& set) {
    return *reinterpret_cast<const RepeatedPtrField<std::string>*>(
        set.GetRawRepeatedField(number, GetDefaultRepeatedField()));
  }

  static inline RepeatedPtrField<std::string>* MutableRepeated(
      int number, FieldType field_type, bool is_packed, ExtensionSet* set) {
    return reinterpret_cast<RepeatedPtrField<std::string>*>(
        set->MutableRawRepeatedField(number, field_type, is_packed, nullptr));
  }

  static const RepeatedFieldType* GetDefaultRepeatedField();

  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType fn) {
    ExtensionSet::RegisterExtension(&ExtendeeT::default_instance(), number,
                                    type, true, is_packed, fn);
  }

 private:
  static void InitializeDefaultRepeatedFields();
  static void DestroyDefaultRepeatedFields();
};

// -------------------------------------------------------------------
// EnumTypeTraits

// ExtensionSet represents enums using integers internally, so we have to
// static_cast around.
template <typename Type, bool IsValid(int)>
class EnumTypeTraits {
 public:
  typedef Type ConstType;
  typedef Type MutableType;
  typedef EnumTypeTraits<Type, IsValid> Singular;
  static constexpr bool kLifetimeBound = false;

  static inline ConstType Get(int number, const ExtensionSet& set,
                              ConstType default_value) {
    return static_cast<Type>(set.GetEnum(number, default_value));
  }
  static inline const ConstType* GetPtr(int number, const ExtensionSet& set,
                                        const ConstType& default_value) {
    return reinterpret_cast<const Type*>(
        &set.GetRefEnum(number, default_value));
  }
  static inline void Set(int number, FieldType field_type, ConstType value,
                         ExtensionSet* set) {
    ABSL_DCHECK(IsValid(value));
    set->SetEnum(number, field_type, value, nullptr);
  }
  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType fn) {
    // Avoid -Wunused-parameter
    (void)fn;
    ExtensionSet::RegisterEnumExtension(&ExtendeeT::default_instance(), number,
                                        type, false, is_packed, IsValid);
  }
};

template <typename Type, bool IsValid(int)>
class RepeatedEnumTypeTraits {
 public:
  typedef Type ConstType;
  typedef Type MutableType;
  typedef RepeatedEnumTypeTraits<Type, IsValid> Repeated;
  static constexpr bool kLifetimeBound = false;

  typedef RepeatedField<Type> RepeatedFieldType;

  static inline ConstType Get(int number, const ExtensionSet& set, int index) {
    return static_cast<Type>(set.GetRepeatedEnum(number, index));
  }
  static inline const ConstType* GetPtr(int number, const ExtensionSet& set,
                                        int index) {
    return reinterpret_cast<const Type*>(
        &set.GetRefRepeatedEnum(number, index));
  }
  static inline void Set(int number, int index, ConstType value,
                         ExtensionSet* set) {
    ABSL_DCHECK(IsValid(value));
    set->SetRepeatedEnum(number, index, value);
  }
  static inline void Add(int number, FieldType field_type, bool is_packed,
                         ConstType value, ExtensionSet* set) {
    ABSL_DCHECK(IsValid(value));
    set->AddEnum(number, field_type, is_packed, value, nullptr);
  }
  static inline const RepeatedField<Type>& GetRepeated(
      int number, const ExtensionSet& set) {
    // Hack: the `Extension` struct stores a RepeatedField<int> for enums.
    // RepeatedField<int> cannot implicitly convert to RepeatedField<EnumType>
    // so we need to do some casting magic. See message.h for similar
    // contortions for non-extension fields.
    return *reinterpret_cast<const RepeatedField<Type>*>(
        set.GetRawRepeatedField(number, GetDefaultRepeatedField()));
  }
  static inline const RepeatedField<Type>* GetRepeatedPtr(
      int number, const ExtensionSet& set) {
    return &GetRepeated(number, set);
  }
  static inline RepeatedField<Type>* MutableRepeated(int number,
                                                     FieldType field_type,
                                                     bool is_packed,
                                                     ExtensionSet* set) {
    return reinterpret_cast<RepeatedField<Type>*>(
        set->MutableRawRepeatedField(number, field_type, is_packed, nullptr));
  }

  static const RepeatedFieldType* GetDefaultRepeatedField() {
    // Hack: as noted above, repeated enum fields are internally stored as a
    // RepeatedField<int>. We need to be able to instantiate global static
    // objects to return as default (empty) repeated fields on non-existent
    // extensions. We would not be able to know a-priori all of the enum types
    // (values of |Type|) to instantiate all of these, so we just re-use
    // int32_t's default repeated field object.
    return reinterpret_cast<const RepeatedField<Type>*>(
        RepeatedPrimitiveTypeTraits<int32_t>::GetDefaultRepeatedField());
  }
  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType fn) {
    // Avoid -Wunused-parameter
    (void)fn;
    ExtensionSet::RegisterEnumExtension(&ExtendeeT::default_instance(), number,
                                        type, true, is_packed, IsValid);
  }
};

// -------------------------------------------------------------------
// MessageTypeTraits

// ExtensionSet guarantees that when manipulating extensions with message
// types, the implementation used will be the compiled-in class representing
// that type.  So, we can static_cast down to the exact type we expect.
template <typename Type>
class MessageTypeTraits {
 public:
  typedef const Type& ConstType;
  typedef Type* MutableType;
  typedef MessageTypeTraits<Type> Singular;
  static constexpr bool kLifetimeBound = true;

  static inline ConstType Get(int number, const ExtensionSet& set,
                              ConstType default_value) {
    return static_cast<const Type&>(set.GetMessage(number, default_value));
  }
  static inline std::nullptr_t GetPtr(int /* number */,
                                      const ExtensionSet& /* set */,
                                      ConstType /* default_value */) {
    // Cannot be implemented because of forward declared messages?
    return nullptr;
  }
  static inline MutableType Mutable(int number, FieldType field_type,
                                    ExtensionSet* set) {
    return static_cast<Type*>(set->MutableMessage(
        number, field_type, Type::default_instance(), nullptr));
  }
  static inline void SetAllocated(int number, FieldType field_type,
                                  MutableType message, ExtensionSet* set) {
    set->SetAllocatedMessage(number, field_type, nullptr, message);
  }
  static inline void UnsafeArenaSetAllocated(int number, FieldType field_type,
                                             MutableType message,
                                             ExtensionSet* set) {
    set->UnsafeArenaSetAllocatedMessage(number, field_type, nullptr, message);
  }
  PROTOBUF_NODISCARD static inline MutableType Release(
      int number, FieldType /* field_type */, ExtensionSet* set) {
    return static_cast<Type*>(
        set->ReleaseMessage(number, Type::default_instance()));
  }
  static inline MutableType UnsafeArenaRelease(int number,
                                               FieldType /* field_type */,
                                               ExtensionSet* set) {
    return static_cast<Type*>(
        set->UnsafeArenaReleaseMessage(number, Type::default_instance()));
  }
  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType fn) {
    ExtensionSet::RegisterMessageExtension(&ExtendeeT::default_instance(),
                                           number, type, false, is_packed,
                                           &Type::default_instance(), fn);
  }
};

// Used by WireFormatVerify to extract the verify function from the registry.
LazyEagerVerifyFnType FindExtensionLazyEagerVerifyFn(
    const MessageLite* extendee, int number);

// forward declaration.
class RepeatedMessageGenericTypeTraits;

template <typename Type>
class RepeatedMessageTypeTraits {
 public:
  typedef const Type& ConstType;
  typedef Type* MutableType;
  typedef RepeatedMessageTypeTraits<Type> Repeated;
  static constexpr bool kLifetimeBound = true;

  typedef RepeatedPtrField<Type> RepeatedFieldType;

  static inline ConstType Get(int number, const ExtensionSet& set, int index) {
    return static_cast<const Type&>(set.GetRepeatedMessage(number, index));
  }
  static inline std::nullptr_t GetPtr(int /* number */,
                                      const ExtensionSet& /* set */,
                                      int /* index */) {
    // Cannot be implemented because of forward declared messages?
    return nullptr;
  }
  static inline std::nullptr_t GetRepeatedPtr(int /* number */,
                                              const ExtensionSet& /* set */) {
    // Cannot be implemented because of forward declared messages?
    return nullptr;
  }
  static inline MutableType Mutable(int number, int index, ExtensionSet* set) {
    return static_cast<Type*>(set->MutableRepeatedMessage(number, index));
  }
  static inline MutableType Add(int number, FieldType field_type,
                                ExtensionSet* set) {
    return static_cast<Type*>(
        set->AddMessage(number, field_type, Type::default_instance(), nullptr));
  }
  static inline const RepeatedPtrField<Type>& GetRepeated(
      int number, const ExtensionSet& set) {
    // See notes above in RepeatedEnumTypeTraits::GetRepeated(): same
    // casting hack applies here, because a RepeatedPtrField<MessageLite>
    // cannot naturally become a RepeatedPtrType<Type> even though Type is
    // presumably a message. google::protobuf::Message goes through similar contortions
    // with a reinterpret_cast<>.
    return *reinterpret_cast<const RepeatedPtrField<Type>*>(
        set.GetRawRepeatedField(number, GetDefaultRepeatedField()));
  }
  static inline RepeatedPtrField<Type>* MutableRepeated(int number,
                                                        FieldType field_type,
                                                        bool is_packed,
                                                        ExtensionSet* set) {
    return reinterpret_cast<RepeatedPtrField<Type>*>(
        set->MutableRawRepeatedField(number, field_type, is_packed, nullptr));
  }

  static const RepeatedFieldType* GetDefaultRepeatedField();
  template <typename ExtendeeT>
  static void Register(int number, FieldType type, bool is_packed,
                       LazyEagerVerifyFnType fn) {
    ExtensionSet::RegisterMessageExtension(&ExtendeeT::default_instance(),
                                           number, type, true, is_packed,
                                           &Type::default_instance(), fn);
  }
};

template <typename Type>
inline const typename RepeatedMessageTypeTraits<Type>::RepeatedFieldType*
RepeatedMessageTypeTraits<Type>::GetDefaultRepeatedField() {
  static auto instance = OnShutdownDelete(new RepeatedFieldType);
  return instance;
}

// -------------------------------------------------------------------
// ExtensionIdentifier

// This is the type of actual extension objects.  E.g. if you have:
//   extend Foo {
//     optional int32 bar = 1234;
//   }
// then "bar" will be defined in C++ as:
//   ExtensionIdentifier<Foo, PrimitiveTypeTraits<int32_t>, 5, false> bar(1234);
//
// Note that we could, in theory, supply the field number as a template
// parameter, and thus make an instance of ExtensionIdentifier have no
// actual contents.  However, if we did that, then using an extension
// identifier would not necessarily cause the compiler to output any sort
// of reference to any symbol defined in the extension's .pb.o file.  Some
// linkers will actually drop object files that are not explicitly referenced,
// but that would be bad because it would cause this extension to not be
// registered at static initialization, and therefore using it would crash.

template <typename ExtendeeType, typename TypeTraitsType, FieldType field_type,
          bool is_packed>
class ExtensionIdentifier {
 public:
  typedef TypeTraitsType TypeTraits;
  typedef ExtendeeType Extendee;

  ExtensionIdentifier(int number, typename TypeTraits::ConstType default_value,
                      LazyEagerVerifyFnType verify_func = nullptr)
      : number_(number), default_value_(default_value) {
    Register(number, verify_func);
  }
  inline int number() const { return number_; }
  typename TypeTraits::ConstType default_value() const {
    return default_value_;
  }

  static void Register(int number, LazyEagerVerifyFnType verify_func) {
    TypeTraits::template Register<ExtendeeType>(number, field_type, is_packed,
                                                verify_func);
  }

  typename TypeTraits::ConstType const& default_value_ref() const {
    return default_value_;
  }

 private:
  const int number_;
  typename TypeTraits::ConstType default_value_;
};

// -------------------------------------------------------------------
// Generated accessors


// Used to retrieve a lazy extension, may return nullptr in some environments.
extern PROTOBUF_ATTRIBUTE_WEAK ExtensionSet::LazyMessageExtension*
MaybeCreateLazyExtension(Arena* arena);

// Define a specialization of ExtensionIdentifier for bootstrapped extensions
// that we need to register lazily.
template <>
class ExtensionIdentifier<FeatureSet, MessageTypeTraits<::pb::CppFeatures>, 11,
                          false> {
 public:
  using TypeTraits = MessageTypeTraits<::pb::CppFeatures>;
  using Extendee = FeatureSet;

  explicit constexpr ExtensionIdentifier(int number) : number_(number) {}

  int number() const { return number_; }
  const ::pb::CppFeatures& default_value() const { return *default_value_; }

  template <typename MessageType = ::pb::CppFeatures,
            typename ExtendeeType = FeatureSet>
  void LazyRegister(
      const MessageType& default_instance = MessageType::default_instance(),
      LazyEagerVerifyFnType verify_func = nullptr) const {
    absl::call_once(once_, [&] {
      default_value_ = &default_instance;
      MessageTypeTraits<MessageType>::template Register<ExtendeeType>(
          number_, 11, false, verify_func);
    });
  }

  const ::pb::CppFeatures& default_value_ref() const { return *default_value_; }

 private:
  const int number_;
  mutable const ::pb::CppFeatures* default_value_ = nullptr;
  mutable absl::once_flag once_;
};


}  // namespace internal

// Call this function to ensure that this extensions's reflection is linked into
// the binary:
//
//   google::protobuf::LinkExtensionReflection(Foo::my_extension);
//
// This will ensure that the following lookup will succeed:
//
//   DescriptorPool::generated_pool()->FindExtensionByName("Foo.my_extension");
//
// This is often relevant for parsing extensions in text mode.
//
// As a side-effect, it will also guarantee that anything else from the same
// .proto file will also be available for lookup in the generated pool.
//
// This function does not actually register the extension, so it does not need
// to be called before the lookup.  However it does need to occur in a function
// that cannot be stripped from the binary (ie. it must be reachable from main).
//
// Best practice is to call this function as close as possible to where the
// reflection is actually needed.  This function is very cheap to call, so you
// should not need to worry about its runtime overhead except in tight loops (on
// x86-64 it compiles into two "mov" instructions).
template <typename ExtendeeType, typename TypeTraitsType,
          internal::FieldType field_type, bool is_packed>
void LinkExtensionReflection(
    const google::protobuf::internal::ExtensionIdentifier<
        ExtendeeType, TypeTraitsType, field_type, is_packed>& extension) {
  internal::StrongReference(extension);
}

// Returns the field descriptor for a generated extension identifier.  This is
// useful when doing reflection over generated extensions.
template <typename ExtendeeType, typename TypeTraitsType,
          internal::FieldType field_type, bool is_packed,
          typename PoolType = DescriptorPool>
const FieldDescriptor* GetExtensionReflection(
    const google::protobuf::internal::ExtensionIdentifier<
        ExtendeeType, TypeTraitsType, field_type, is_packed>& extension) {
  return PoolType::generated_pool()->FindExtensionByNumber(
      google::protobuf::internal::ExtensionIdentifier<ExtendeeType, TypeTraitsType,
                                            field_type,
                                            is_packed>::Extendee::descriptor(),
      extension.number());
}

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


#endif  // GOOGLE_PROTOBUF_EXTENSION_SET_H__

