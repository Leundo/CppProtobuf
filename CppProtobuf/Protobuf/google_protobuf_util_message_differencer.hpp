// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: jschorr@google.com (Joseph Schorr)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// This file defines static methods and classes for comparing Protocol
// Messages.
//
// Aug. 2008: Added Unknown Fields Comparison for messages.
// Aug. 2009: Added different options to compare repeated fields.
// Apr. 2010: Moved field comparison to FieldComparator
// Sep. 2020: Added option to output map keys in path

#ifndef GOOGLE_PROTOBUF_UTIL_MESSAGE_DIFFERENCER_H__
#define GOOGLE_PROTOBUF_UTIL_MESSAGE_DIFFERENCER_H__

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <CppProtobuf/google_protobuf_stubs_common.hpp>
#include <CppAbseil/absl_container_fixed_array.hpp>
#include <CppAbseil/absl_container_flat_hash_map.hpp>
#include <CppAbseil/absl_container_flat_hash_set.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppProtobuf/google_protobuf_descriptor.hpp>
#include <CppProtobuf/google_protobuf_message.hpp>
#include <CppProtobuf/google_protobuf_unknown_field_set.hpp>
#include <CppProtobuf/google_protobuf_util_field_comparator.hpp>

// Always include as last one, otherwise it can break compilation

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

class DynamicMessageFactory;
class FieldDescriptor;

namespace io {
class ZeroCopyOutputStream;
class Printer;
}  // namespace io

namespace util {

class DefaultFieldComparator;
class FieldContext;  // declared below MessageDifferencer

// A basic differencer that can be used to determine
// the differences between two specified Protocol Messages. If any differences
// are found, the Compare method will return false, and any differencer reporter
// specified via ReportDifferencesTo will have its reporting methods called (see
// below for implementation of the report). Based off of the original
// ProtocolDifferencer implementation in //net/proto/protocol-differencer.h
// (Thanks Todd!).
//
// MessageDifferencer REQUIRES that compared messages be the same type, defined
// as messages that share the same descriptor.  If not, the behavior of this
// class is undefined.
//
// People disagree on what MessageDifferencer should do when asked to compare
// messages with different descriptors.  Some people think it should always
// return false.  Others expect it to try to look for similar fields and
// compare them anyway -- especially if the descriptors happen to be identical.
// If we chose either of these behaviors, some set of people would find it
// surprising, and could end up writing code expecting the other behavior
// without realizing their error.  Therefore, we forbid that usage.
//
// This class is implemented based on the proto2 reflection. The performance
// should be good enough for normal usages. However, for places where the
// performance is extremely sensitive, there are several alternatives:
// - Comparing serialized string
// Downside: false negatives (there are messages that are the same but their
// serialized strings are different).
// - Equals code generator by compiler plugin (net/proto2/contrib/equals_plugin)
// Downside: more generated code; maintenance overhead for the additional rule
// (must be in sync with the original proto_library).
//
// Note on handling of google.protobuf.Any: MessageDifferencer automatically
// unpacks Any::value into a Message and compares its individual fields.
// Messages encoded in a repeated Any cannot be compared using TreatAsMap.
//
// Note on thread-safety: MessageDifferencer is *not* thread-safe. You need to
// guard it with a lock to use the same MessageDifferencer instance from
// multiple threads. Note that it's fine to call static comparison methods
// (like MessageDifferencer::Equals) concurrently, but it's not recommended for
// performance critical code as it leads to extra allocations.
class PROTOBUF_EXPORT MessageDifferencer {
 public:
  // Determines whether the supplied messages are equal. Equality is defined as
  // all fields within the two messages being set to the same value. Primitive
  // fields and strings are compared by value while embedded messages/groups
  // are compared as if via a recursive call. Use Compare() with IgnoreField()
  // if some fields should be ignored in the comparison. Use Compare() with
  // TreatAsSet() if there are repeated fields where ordering does not matter.
  //
  // This method REQUIRES that the two messages have the same
  // Descriptor (message1.GetDescriptor() == message2.GetDescriptor()).
  static bool Equals(const Message& message1, const Message& message2);

  // Determines whether the supplied messages are equivalent. Equivalency is
  // defined as all fields within the two messages having the same value. This
  // differs from the Equals method above in that fields with default values
  // are considered set to said value automatically. For details on how default
  // values are defined for each field type, see:
  // https://developers.google.com/protocol-buffers/docs/proto?csw=1#optional.
  // Also, Equivalent() ignores unknown fields. Use IgnoreField() and Compare()
  // if some fields should be ignored in the comparison.
  //
  // This method REQUIRES that the two messages have the same
  // Descriptor (message1.GetDescriptor() == message2.GetDescriptor()).
  static bool Equivalent(const Message& message1, const Message& message2);

  // Determines whether the supplied messages are approximately equal.
  // Approximate equality is defined as all fields within the two messages
  // being approximately equal.  Primitive (non-float) fields and strings are
  // compared by value, floats are compared using MathUtil::AlmostEquals() and
  // embedded messages/groups are compared as if via a recursive call. Use
  // IgnoreField() and Compare() if some fields should be ignored in the
  // comparison.
  //
  // This method REQUIRES that the two messages have the same
  // Descriptor (message1.GetDescriptor() == message2.GetDescriptor()).
  static bool ApproximatelyEquals(const Message& message1,
                                  const Message& message2);

  // Determines whether the supplied messages are approximately equivalent.
  // Approximate equivalency is defined as all fields within the two messages
  // being approximately equivalent. As in
  // MessageDifferencer::ApproximatelyEquals, primitive (non-float) fields and
  // strings are compared by value, floats are compared using
  // MathUtil::AlmostEquals() and embedded messages/groups are compared as if
  // via a recursive call. However, fields with default values are considered
  // set to said value, as per MessageDiffencer::Equivalent. Use IgnoreField()
  // and Compare() if some fields should be ignored in the comparison.
  //
  // This method REQUIRES that the two messages have the same
  // Descriptor (message1.GetDescriptor() == message2.GetDescriptor()).
  static bool ApproximatelyEquivalent(const Message& message1,
                                      const Message& message2);

  // Identifies an individual field in a message instance.  Used for field_path,
  // below.
  struct SpecificField {
    // The messages that contain this field. They are always set. They are valid
    // only during a call to Reporter::Report* methods.
    //
    // If the original messages are of type google.protobuf.Any, these fields
    // will store the unpacked payloads, and unpacked_any will become > 0.  More
    // precisely, unpacked_any defines the nesting level of Any.  For example,
    // if the original message packs another Any, then unpacked_any=2, assuming
    // the differencer unpacked both of them.
    //
    // When an Any object packs a non-Any proto object whose field includes
    // Any, then unpacked_any=1. Thus, in most practical applications,
    // unpacked_any will be 0 or 1.
    const Message* message1 = nullptr;
    const Message* message2 = nullptr;
    int unpacked_any = 0;

    // For known fields, "field" is filled in and "unknown_field_number" is -1.
    // For unknown fields, "field" is NULL, "unknown_field_number" is the field
    // number, and "unknown_field_type" is its type.
    const FieldDescriptor* field = nullptr;
    int unknown_field_number = -1;
    UnknownField::Type unknown_field_type = UnknownField::Type::TYPE_VARINT;

    // If this a repeated field, "index" is the index within it.  For unknown
    // fields, this is the index of the field among all unknown fields of the
    // same field number and type.
    int index = -1;

    // If "field" is a repeated field which is being treated as a map or
    // a set (see TreatAsMap() and TreatAsSet(), below), new_index indicates
    // the index the position to which the element has moved.  If the element
    // has not moved, "new_index" will have the same value as "index".
    int new_index = -1;

    // If "field" is a map field, point to the map entry.
    const Message* map_entry1 = nullptr;
    const Message* map_entry2 = nullptr;

    // For unknown fields, these are the pointers to the UnknownFieldSet
    // containing the unknown fields. In certain cases (e.g. proto1's
    // MessageSet, or nested groups of unknown fields), these may differ from
    // the messages' internal UnknownFieldSets.
    const UnknownFieldSet* unknown_field_set1 = nullptr;
    const UnknownFieldSet* unknown_field_set2 = nullptr;

    // For unknown fields, these are the index of the field within the
    // UnknownFieldSets. One or the other will be -1 when
    // reporting an addition or deletion.
    int unknown_field_index1 = -1;
    int unknown_field_index2 = -1;

    // Was this field added to the diffing because set_force_compare_no_presence
    // was called on the MessageDifferencer object.
    bool forced_compare_no_presence_ = false;
  };

  // Abstract base class from which all MessageDifferencer
  // reporters derive. The five Report* methods below will be called when
  // a field has been added, deleted, modified, moved, or matched. The third
  // argument is a vector of FieldDescriptor pointers which describes the chain
  // of fields that was taken to find the current field. For example, for a
  // field found in an embedded message, the vector will contain two
  // FieldDescriptors. The first will be the field of the embedded message
  // itself and the second will be the actual field in the embedded message
  // that was added/deleted/modified.
  // Fields will be reported in PostTraversalOrder.
  // For example, given following proto, if both baz and mooo are changed.
  // foo {
  //   bar {
  //     baz: 1
  //     mooo: 2
  //   }
  // }
  // ReportModified will be invoked with following order:
  // 1. foo.bar.baz or foo.bar.mooo
  // 2. foo.bar.mooo or foo.bar.baz
  // 2. foo.bar
  // 3. foo
  class PROTOBUF_EXPORT Reporter {
   public:
    Reporter();
    Reporter(const Reporter&) = delete;
    Reporter& operator=(const Reporter&) = delete;
    virtual ~Reporter();

    // Reports that a field has been added into Message2.
    virtual void ReportAdded(const Message& message1, const Message& message2,
                             const std::vector<SpecificField>& field_path) {}

    // Reports that a field has been deleted from Message1.
    virtual void ReportDeleted(const Message& message1, const Message& message2,
                               const std::vector<SpecificField>& field_path) {}

    // Reports that the value of a field has been modified.
    virtual void ReportModified(const Message& message1,
                                const Message& message2,
                                const std::vector<SpecificField>& field_path) {}

    // Reports that a repeated field has been moved to another location.  This
    // only applies when using TreatAsSet or TreatAsMap()  -- see below. Also
    // note that for any given field, ReportModified and ReportMoved are
    // mutually exclusive. If a field has been both moved and modified, then
    // only ReportModified will be called.
    virtual void ReportMoved(
        const Message& /* message1 */, const Message& /* message2 */,
        const std::vector<SpecificField>& /* field_path */) {}

    // Reports that two fields match. Useful for doing side-by-side diffs.
    // This function is mutually exclusive with ReportModified and ReportMoved.
    // Note that you must call set_report_matches(true) before calling Compare
    // to make use of this function.
    virtual void ReportMatched(
        const Message& /* message1 */, const Message& /* message2 */,
        const std::vector<SpecificField>& /* field_path */) {}

    // Reports that two fields would have been compared, but the
    // comparison has been skipped because the field was marked as
    // 'ignored' using IgnoreField().  This function is mutually
    // exclusive with all the other Report() functions.
    //
    // The contract of ReportIgnored is slightly different than the
    // other Report() functions, in that |field_path.back().index| is
    // always equal to -1, even if the last field is repeated. This is
    // because while the other Report() functions indicate where in a
    // repeated field the action (Addition, Deletion, etc...)
    // happened, when a repeated field is 'ignored', the differencer
    // simply calls ReportIgnored on the repeated field as a whole and
    // moves on without looking at its individual elements.
    //
    // Furthermore, ReportIgnored() does not indicate whether the
    // fields were in fact equal or not, as Compare() does not inspect
    // these fields at all. It is up to the Reporter to decide whether
    // the fields are equal or not (perhaps with a second call to
    // Compare()), if it cares.
    virtual void ReportIgnored(
        const Message& /* message1 */, const Message& /* message2 */,
        const std::vector<SpecificField>& /* field_path */) {}

    // Report that an unknown field is ignored. (see comment above).
    // Note this is a different function since the last SpecificField in field
    // path has a null field.  This could break existing Reporter.
    virtual void ReportUnknownFieldIgnored(
        const Message& /* message1 */, const Message& /* message2 */,
        const std::vector<SpecificField>& /* field_path */) {}
  };

  // MapKeyComparator is used to determine if two elements have the same key
  // when comparing elements of a repeated field as a map.
  class PROTOBUF_EXPORT MapKeyComparator {
   public:
    MapKeyComparator();
    MapKeyComparator(const MapKeyComparator&) = delete;
    MapKeyComparator& operator=(const MapKeyComparator&) = delete;
    virtual ~MapKeyComparator();

    // This method should be overridden by every implementation.  The arg
    // unmapped_any is nonzero the original messages provided by the user are of
    // type google.protobuf.Any.
    //
    // More precisely, unpacked_any defines the nesting level of Any.  For
    // example, if Any packs another Any then unpacked_any=2, assuming the
    // patcher unpacked both.  Note that when an Any object packs a non-Any
    // proto object whose field includes Any, then unpacked_any=1. Thus, in most
    // practical applications, unpacked_any will be 0 or 1.
    virtual bool IsMatch(const Message& message1, const Message& message2,
                         int /* unmapped_any */,
                         const std::vector<SpecificField>& fields) const {
      ABSL_CHECK(false) << "IsMatch() is not implemented.";
      return false;
    }
  };

  // Abstract base class from which all IgnoreCriteria derive.
  // By adding IgnoreCriteria more complex ignore logic can be implemented.
  // IgnoreCriteria are registered with AddIgnoreCriteria. For each compared
  // field IsIgnored is called on each added IgnoreCriteria until one returns
  // true or all return false.
  // IsIgnored is called for fields where at least one side has a value.
  class PROTOBUF_EXPORT IgnoreCriteria {
   public:
    IgnoreCriteria();
    virtual ~IgnoreCriteria();

    // Returns true if the field should be ignored.
    virtual bool IsIgnored(
        const Message& /* message1 */, const Message& /* message2 */,
        const FieldDescriptor* /* field */,
        const std::vector<SpecificField>& /* parent_fields */) = 0;

    // Returns true if the unknown field should be ignored.
    // Note: This will be called for unknown fields as well in which case
    //       field.field will be null.
    virtual bool IsUnknownFieldIgnored(
        const Message& /* message1 */, const Message& /* message2 */,
        const SpecificField& /* field */,
        const std::vector<SpecificField>& /* parent_fields */) {
      return false;
    }
  };

  // To add a Reporter, construct default here, then use ReportDifferencesTo or
  // ReportDifferencesToString.
  explicit MessageDifferencer();
  MessageDifferencer(const MessageDifferencer&) = delete;
  MessageDifferencer& operator=(const MessageDifferencer&) = delete;

  ~MessageDifferencer();

  enum MessageFieldComparison {
    EQUAL,       // Fields must be present in both messages
                 // for the messages to be considered the same.
    EQUIVALENT,  // Fields with default values are considered set
                 // for comparison purposes even if not explicitly
                 // set in the messages themselves.  Unknown fields
                 // are ignored.
  };

  enum Scope {
    FULL,    // All fields of both messages are considered in the comparison.
    PARTIAL  // Only fields present in the first message are considered; fields
             // set only in the second message will be skipped during
             // comparison.
  };

  // DEPRECATED. Use FieldComparator::FloatComparison instead.
  enum FloatComparison {
    EXACT,       // Floats and doubles are compared exactly.
    APPROXIMATE  // Floats and doubles are compared using the
                 // MathUtil::AlmostEquals method.
  };

  enum RepeatedFieldComparison {
    AS_LIST,  // Repeated fields are compared in order.  Differing values at
              // the same index are reported using ReportModified().  If the
              // repeated fields have different numbers of elements, the
              // unpaired elements are reported using ReportAdded() or
              // ReportDeleted().
    AS_SET,   // Treat all the repeated fields as sets.
              // See TreatAsSet(), as below.
    AS_SMART_LIST,  // Similar to AS_SET, but preserve the order and find the
                    // longest matching sequence from the first matching
                    // element. To use an optimal solution, call
                    // SetMatchIndicesForSmartListCallback() to pass it in.
    AS_SMART_SET,   // Similar to AS_SET, but match elements with fewest diffs.
  };

  // The elements of the given repeated field will be treated as a set for
  // diffing purposes, so different orderings of the same elements will be
  // considered equal.  Elements which are present on both sides of the
  // comparison but which have changed position will be reported with
  // ReportMoved().  Elements which only exist on one side or the other are
  // reported with ReportAdded() and ReportDeleted() regardless of their
  // positions.  ReportModified() is never used for this repeated field.  If
  // the only differences between the compared messages is that some fields
  // have been moved, then the comparison returns true.
  //
  // Note that despite the name of this method, this is really
  // comparison as multisets: if one side of the comparison has a duplicate
  // in the repeated field but the other side doesn't, this will count as
  // a mismatch.
  //
  // If the scope of comparison is set to PARTIAL, then in addition to what's
  // above, extra values added to repeated fields of the second message will
  // not cause the comparison to fail.
  //
  // Note that set comparison is currently O(k * n^2) (where n is the total
  // number of elements, and k is the average size of each element). In theory
  // it could be made O(n * k) with a more complex hashing implementation. Feel
  // free to contribute one if the current implementation is too slow for you.
  // If partial matching is also enabled, the time complexity will be O(k * n^2
  // + n^3) in which n^3 is the time complexity of the maximum matching
  // algorithm.
  //
  // REQUIRES: field->is_repeated() and field not registered with TreatAsMap*
  void TreatAsSet(const FieldDescriptor* field);
  void TreatAsSmartSet(const FieldDescriptor* field);

  // The elements of the given repeated field will be treated as a list for
  // diffing purposes, so different orderings of the same elements will NOT be
  // considered equal.
  //
  // REQUIRES: field->is_repeated() and field not registered with TreatAsMap*
  void TreatAsList(const FieldDescriptor* field);
  // Note that the complexity is similar to treating as SET.
  void TreatAsSmartList(const FieldDescriptor* field);

  // The elements of the given repeated field will be treated as a map for
  // diffing purposes, with |key| being the map key.  Thus, elements with the
  // same key will be compared even if they do not appear at the same index.
  // Differences are reported similarly to TreatAsSet(), except that
  // ReportModified() is used to report elements with the same key but
  // different values.  Note that if an element is both moved and modified,
  // only ReportModified() will be called.  As with TreatAsSet, if the only
  // differences between the compared messages is that some fields have been
  // moved, then the comparison returns true. See TreatAsSet for notes on
  // performance.
  //
  // REQUIRES:  field->is_repeated()
  // REQUIRES:  field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE
  // REQUIRES:  key->containing_type() == field->message_type()
  void TreatAsMap(const FieldDescriptor* field, const FieldDescriptor* key);
  // Same as TreatAsMap except that this method will use multiple fields as
  // the key in comparison. All specified fields in 'key_fields' should be
  // present in the compared elements. Two elements will be treated as having
  // the same key iff they have the same value for every specified field. There
  // are two steps in the comparison process. The first one is key matching.
  // Every element from one message will be compared to every element from
  // the other message. Only fields in 'key_fields' are compared in this step
  // to decide if two elements have the same key. The second step is value
  // comparison. Those pairs of elements with the same key (with equal value
  // for every field in 'key_fields') will be compared in this step.
  // Time complexity of the first step is O(s * m * n ^ 2) where s is the
  // average size of the fields specified in 'key_fields', m is the number of
  // fields in 'key_fields' and n is the number of elements. If partial
  // matching is enabled, an extra O(n^3) will be incured by the maximum
  // matching algorithm. The second step is O(k * n) where k is the average
  // size of each element.
  void TreatAsMapWithMultipleFieldsAsKey(
      const FieldDescriptor* field,
      const std::vector<const FieldDescriptor*>& key_fields);
  // Same as TreatAsMapWithMultipleFieldsAsKey, except that each of the field
  // do not necessarily need to be a direct subfield. Each element in
  // key_field_paths indicate a path from the message being compared, listing
  // successive subfield to reach the key field.
  //
  // REQUIRES:
  //   for key_field_path in key_field_paths:
  //     key_field_path[0]->containing_type() == field->message_type()
  //     for i in [0, key_field_path.size() - 1):
  //       key_field_path[i+1]->containing_type() ==
  //           key_field_path[i]->message_type()
  //       key_field_path[i]->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE
  //       !key_field_path[i]->is_repeated()
  void TreatAsMapWithMultipleFieldPathsAsKey(
      const FieldDescriptor* field,
      const std::vector<std::vector<const FieldDescriptor*> >& key_field_paths);

  // Uses a custom MapKeyComparator to determine if two elements have the same
  // key when comparing a repeated field as a map.
  // The caller is responsible to delete the key_comparator.
  // This method varies from TreatAsMapWithMultipleFieldsAsKey only in the
  // first key matching step. Rather than comparing some specified fields, it
  // will invoke the IsMatch method of the given 'key_comparator' to decide if
  // two elements have the same key.
  void TreatAsMapUsingKeyComparator(const FieldDescriptor* field,
                                    const MapKeyComparator* key_comparator);

  // Initiates and returns a new instance of MultipleFieldsMapKeyComparator.
  MapKeyComparator* CreateMultipleFieldsMapKeyComparator(
      const std::vector<std::vector<const FieldDescriptor*> >& key_field_paths);

  // Add a custom ignore criteria that is evaluated in addition to the
  // ignored fields added with IgnoreField.
  // Takes ownership of ignore_criteria.
  void AddIgnoreCriteria(IgnoreCriteria* ignore_criteria) {
    AddIgnoreCriteria(absl::WrapUnique(ignore_criteria));
  }
  void AddIgnoreCriteria(std::unique_ptr<IgnoreCriteria> ignore_criteria);

  // Indicates that any field with the given descriptor should be
  // ignored for the purposes of comparing two messages. This applies
  // to fields nested in the message structure as well as top level
  // ones. When the MessageDifferencer encounters an ignored field,
  // ReportIgnored is called on the reporter, if one is specified.
  //
  // The only place where the field's 'ignored' status is not applied is when
  // it is being used as a key in a field passed to TreatAsMap or is one of
  // the fields passed to TreatAsMapWithMultipleFieldsAsKey.
  // In this case it is compared in key matching but after that it's ignored
  // in value comparison.
  void IgnoreField(const FieldDescriptor* field);

  // Sets the field comparator used to determine differences between protocol
  // buffer fields. By default it's set to a DefaultFieldComparator instance.
  // MessageDifferencer doesn't take ownership over the passed object.
  // Note that this method must be called before Compare for the comparator to
  // be used.
  void set_field_comparator(FieldComparator* comparator);
  void set_field_comparator(DefaultFieldComparator* comparator);

  // DEPRECATED. Pass a DefaultFieldComparator instance instead.
  // Sets the fraction and margin for the float comparison of a given field.
  // Uses MathUtil::WithinFractionOrMargin to compare the values.
  // NOTE: this method does nothing if differencer's field comparator has been
  //       set to a custom object.
  //
  // REQUIRES: field->cpp_type == FieldDescriptor::CPPTYPE_DOUBLE or
  //           field->cpp_type == FieldDescriptor::CPPTYPE_FLOAT
  // REQUIRES: float_comparison_ == APPROXIMATE
  void SetFractionAndMargin(const FieldDescriptor* field, double fraction,
                            double margin);

  // Sets the type of comparison (as defined in the MessageFieldComparison
  // enumeration above) that is used by this differencer when determining how
  // to compare fields in messages.
  void set_message_field_comparison(MessageFieldComparison comparison);

  // Returns the current message field comparison used in this differencer.
  MessageFieldComparison message_field_comparison() const;

  // Tells the differencer whether or not to report matches. This method must
  // be called before Compare. The default for a new differencer is false.
  void set_report_matches(bool report_matches) {
    report_matches_ = report_matches;
  }

  // Tells the differencer whether or not to report moves (in a set or map
  // repeated field). This method must be called before Compare. The default for
  // a new differencer is true.
  void set_report_moves(bool report_moves) { report_moves_ = report_moves; }

  // Tells the differencer whether or not to report ignored values. This method
  // must be called before Compare. The default for a new differencer is true.
  void set_report_ignores(bool report_ignores) {
    report_ignores_ = report_ignores;
  }

  // Sets the scope of the comparison (as defined in the Scope enumeration
  // above) that is used by this differencer when determining which fields to
  // compare between the messages.
  void set_scope(Scope scope);

  // Returns the current scope used by this differencer.
  Scope scope() const;

  // Only affects PARTIAL diffing. When set, all non-repeated no-presence fields
  // which are set to their default value (which is the same as being unset) in
  // message1 but are set to a non-default value in message2 will also be used
  // in the comparison.
  void set_force_compare_no_presence(bool value);

  // DEPRECATED. Pass a DefaultFieldComparator instance instead.
  // Sets the type of comparison (as defined in the FloatComparison enumeration
  // above) that is used by this differencer when comparing float (and double)
  // fields in messages.
  // NOTE: this method does nothing if differencer's field comparator has been
  //       set to a custom object.
  void set_float_comparison(FloatComparison comparison);

  // Sets the type of comparison for repeated field (as defined in the
  // RepeatedFieldComparison enumeration above) that is used by this
  // differencer when compare repeated fields in messages.
  void set_repeated_field_comparison(RepeatedFieldComparison comparison);

  // Returns the current repeated field comparison used by this differencer.
  RepeatedFieldComparison repeated_field_comparison() const;

  // Compares the two specified messages, returning true if they are the same,
  // false otherwise. If this method returns false, any changes between the
  // two messages will be reported if a Reporter was specified via
  // ReportDifferencesTo (see also ReportDifferencesToString).
  //
  // This method REQUIRES that the two messages have the same
  // Descriptor (message1.GetDescriptor() == message2.GetDescriptor()).
  bool Compare(const Message& message1, const Message& message2);

  // Same as above, except comparing only the list of fields specified by the
  // two vectors of FieldDescriptors.
  bool CompareWithFields(
      const Message& message1, const Message& message2,
      const std::vector<const FieldDescriptor*>& message1_fields,
      const std::vector<const FieldDescriptor*>& message2_fields);

  // Automatically creates a reporter that will output the differences
  // found (if any) to the specified output string pointer. Note that this
  // method must be called before Compare.
  void ReportDifferencesToString(std::string* output);

  // Tells the MessageDifferencer to report differences via the specified
  // reporter. Note that this method must be called before Compare for
  // the reporter to be used. It is the responsibility of the caller to delete
  // this object.
  // If the provided pointer equals NULL, the MessageDifferencer stops reporting
  // differences to any previously set reporters or output strings.
  void ReportDifferencesTo(Reporter* reporter);

  // Returns the list of fields which was automatically added to the list of
  // compared fields by calling set_force_compare_no_presence and caused the
  // last call to Compare to fail.
  const absl::flat_hash_set<std::string>& NoPresenceFieldsCausingFailure() {
    return force_compare_failure_triggering_fields_;
  }

 private:
  // Class for processing Any deserialization.  This logic is used by both the
  // MessageDifferencer and StreamReporter classes.
  class UnpackAnyField {
   private:
    std::unique_ptr<DynamicMessageFactory> dynamic_message_factory_;

   public:
    UnpackAnyField() = default;
    ~UnpackAnyField() = default;
    // If "any" is of type google.protobuf.Any, extract its payload using
    // DynamicMessageFactory and store in "data".
    bool UnpackAny(const Message& any, std::unique_ptr<Message>* data);
  };

 public:
  // An implementation of the MessageDifferencer Reporter that outputs
  // any differences found in human-readable form to the supplied
  // ZeroCopyOutputStream or Printer. If a printer is used, the delimiter
  // *must* be '$'.
  //
  // WARNING: this reporter does not necessarily flush its output until it is
  // destroyed. As a result, it is not safe to assume the output is valid or
  // complete until after you destroy the reporter. For example, if you use a
  // StreamReporter to write to a StringOutputStream, the target string may
  // contain uninitialized data until the reporter is destroyed.
  class PROTOBUF_EXPORT StreamReporter : public Reporter {
   public:
    explicit StreamReporter(io::ZeroCopyOutputStream* output);
    explicit StreamReporter(io::Printer* printer);  // delimiter '$'
    StreamReporter(const StreamReporter&) = delete;
    StreamReporter& operator=(const StreamReporter&) = delete;
    ~StreamReporter() override;

    // When set to true, the stream reporter will also output aggregates nodes
    // (i.e. messages and groups) whose subfields have been modified. When
    // false, will only report the individual subfields. Defaults to false.
    void set_report_modified_aggregates(bool report) {
      report_modified_aggregates_ = report;
    }

    // The following are implementations of the methods described above.

    void ReportAdded(const Message& message1, const Message& message2,
                     const std::vector<SpecificField>& field_path) override;

    void ReportDeleted(const Message& message1, const Message& message2,
                       const std::vector<SpecificField>& field_path) override;

    void ReportModified(const Message& message1, const Message& message2,
                        const std::vector<SpecificField>& field_path) override;

    void ReportMoved(const Message& message1, const Message& message2,
                     const std::vector<SpecificField>& field_path) override;

    void ReportMatched(const Message& message1, const Message& message2,
                       const std::vector<SpecificField>& field_path) override;

    void ReportIgnored(const Message& message1, const Message& message2,
                       const std::vector<SpecificField>& field_path) override;

    void ReportUnknownFieldIgnored(
        const Message& message1, const Message& message2,
        const std::vector<SpecificField>& field_path) override;

    // Messages that are being compared must be provided to StreamReporter prior
    // to processing
    void SetMessages(const Message& message1, const Message& message2);

   protected:
    // Prints the specified path of fields to the buffer.
    virtual void PrintPath(const std::vector<SpecificField>& field_path,
                           bool left_side);

    // Prints the value of fields to the buffer.  left_side is true if the
    // given message is from the left side of the comparison, false if it
    // was the right.  This is relevant only to decide whether to follow
    // unknown_field_index1 or unknown_field_index2 when an unknown field
    // is encountered in field_path.
    virtual void PrintValue(const Message& message,
                            const std::vector<SpecificField>& field_path,
                            bool left_side);

    // Prints the specified path of unknown fields to the buffer.
    virtual void PrintUnknownFieldValue(const UnknownField* unknown_field);

    // Just print a string
    void Print(const std::string& str);

   private:
    // helper function for PrintPath that contains logic for printing maps
    void PrintMapKey(bool left_side, const SpecificField& specific_field);

    io::Printer* printer_;
    bool delete_printer_;
    bool report_modified_aggregates_;
    const Message* message1_;
    const Message* message2_;
    MessageDifferencer::UnpackAnyField unpack_any_field_;
  };

 private:
  friend class SimpleFieldComparator;

  // A MapKeyComparator to be used in TreatAsMapUsingKeyComparator.
  // Implementation of this class needs to do field value comparison which
  // relies on some private methods of MessageDifferencer. That's why this
  // class is declared as a nested class of MessageDifferencer.
  class MultipleFieldsMapKeyComparator;

  // A MapKeyComparator for use with map_entries.
  class PROTOBUF_EXPORT MapEntryKeyComparator : public MapKeyComparator {
   public:
    explicit MapEntryKeyComparator(MessageDifferencer* message_differencer);
    bool IsMatch(
        const Message& message1, const Message& message2, int unpacked_any,
        const std::vector<SpecificField>& parent_fields) const override;

   private:
    MessageDifferencer* message_differencer_;
  };

  // Returns true if field1's number() is less than field2's.
  static bool FieldBefore(const FieldDescriptor* field1,
                          const FieldDescriptor* field2);

  // Retrieve all the set fields, including extensions.
  std::vector<const FieldDescriptor*> RetrieveFields(const Message& message,
                                                     bool base_message);

  // Combine the two lists of fields into the combined_fields output vector.
  // All fields present in both lists will always be included in the combined
  // list.  Fields only present in one of the lists will only appear in the
  // combined list if the corresponding fields_scope option is set to FULL.
  std::vector<const FieldDescriptor*> CombineFields(
      const std::vector<const FieldDescriptor*>& fields1, Scope fields1_scope,
      const std::vector<const FieldDescriptor*>& fields2, Scope fields2_scope);

  // Internal version of the Compare method which performs the actual
  // comparison. The parent_fields vector is a vector containing field
  // descriptors of all fields accessed to get to this comparison operation
  // (i.e. if the current message is an embedded message, the parent_fields
  // vector will contain the field that has this embedded message).
  bool Compare(const Message& message1, const Message& message2,
               int unpacked_any, std::vector<SpecificField>* parent_fields);

  // Compares all the unknown fields in two messages.
  bool CompareUnknownFields(const Message& message1, const Message& message2,
                            const UnknownFieldSet&, const UnknownFieldSet&,
                            std::vector<SpecificField>* parent_fields);

  // Compares the specified messages for the requested field lists. The field
  // lists are modified depending on comparison settings, and then passed to
  // CompareWithFieldsInternal.
  bool CompareRequestedFieldsUsingSettings(
      const Message& message1, const Message& message2, int unpacked_any,
      const std::vector<const FieldDescriptor*>& message1_fields,
      const std::vector<const FieldDescriptor*>& message2_fields,
      std::vector<SpecificField>* parent_fields);

  // Compares the specified messages with the specified field lists.
  bool CompareWithFieldsInternal(
      const Message& message1, const Message& message2, int unpacked_any,
      const std::vector<const FieldDescriptor*>& message1_fields,
      const std::vector<const FieldDescriptor*>& message2_fields,
      std::vector<SpecificField>* parent_fields);

  // Compares the repeated fields, and report the error.
  bool CompareRepeatedField(const Message& message1, const Message& message2,
                            int unpacked_any, const FieldDescriptor* field,
                            std::vector<SpecificField>* parent_fields);

  // Compares map fields, and report the error.
  bool CompareMapField(const Message& message1, const Message& message2,
                       int unpacked_any, const FieldDescriptor* field,
                       std::vector<SpecificField>* parent_fields);

  // Helper for CompareRepeatedField and CompareMapField: compares and reports
  // differences element-wise. This is the implementation for non-map fields,
  // and can also compare map fields by using the underlying representation.
  bool CompareRepeatedRep(const Message& message1, const Message& message2,
                          int unpacked_any, const FieldDescriptor* field,
                          std::vector<SpecificField>* parent_fields);

  // Helper for CompareMapField: compare the map fields using map reflection
  // instead of sync to repeated.
  bool CompareMapFieldByMapReflection(const Message& message1,
                                      const Message& message2, int unpacked_any,
                                      const FieldDescriptor* field,
                                      std::vector<SpecificField>* parent_fields,
                                      DefaultFieldComparator* comparator);

  // Shorthand for CompareFieldValueUsingParentFields with NULL parent_fields.
  bool CompareFieldValue(const Message& message1, const Message& message2,
                         int unpacked_any, const FieldDescriptor* field,
                         int index1, int index2);

  // Compares the specified field on the two messages, returning
  // true if they are the same, false otherwise. For repeated fields,
  // this method only compares the value in the specified index. This method
  // uses Compare functions to recurse into submessages.
  // The parent_fields vector is used in calls to a Reporter instance calls.
  // It can be NULL, in which case the MessageDifferencer will create new
  // list of parent messages if it needs to recursively compare the given field.
  // To avoid confusing users you should not set it to NULL unless you modified
  // Reporter to handle the change of parent_fields correctly.
  bool CompareFieldValueUsingParentFields(
      const Message& message1, const Message& message2, int unpacked_any,
      const FieldDescriptor* field, int index1, int index2,
      std::vector<SpecificField>* parent_fields);

  // Compares the specified field on the two messages, returning comparison
  // result, as returned by appropriate FieldComparator.
  FieldComparator::ComparisonResult GetFieldComparisonResult(
      const Message& message1, const Message& message2,
      const FieldDescriptor* field, int index1, int index2,
      const FieldContext* field_context);

  // Check if the two elements in the repeated field are match to each other.
  // if the key_comprator is NULL, this function returns true when the two
  // elements are equal.
  bool IsMatch(const FieldDescriptor* repeated_field,
               const MapKeyComparator* key_comparator, const Message* message1,
               const Message* message2, int unpacked_any,
               const std::vector<SpecificField>& parent_fields,
               Reporter* reporter, int index1, int index2);

  // Returns true when this repeated field has been configured to be treated
  // as a Set / SmartSet / SmartList.
  bool IsTreatedAsSet(const FieldDescriptor* field);
  bool IsTreatedAsSmartSet(const FieldDescriptor* field);

  bool IsTreatedAsSmartList(const FieldDescriptor* field);
  // When treating as SMART_LIST, it uses MatchIndicesPostProcessorForSmartList
  // by default to find the longest matching sequence from the first matching
  // element. The callback takes two vectors showing the matching indices from
  // the other vector, where -1 means an unmatch.
  void SetMatchIndicesForSmartListCallback(
      std::function<void(std::vector<int>*, std::vector<int>*)> callback);

  // Returns true when this repeated field is to be compared as a subset, ie.
  // has been configured to be treated as a set or map and scope is set to
  // PARTIAL.
  bool IsTreatedAsSubset(const FieldDescriptor* field);

  // Returns true if this field is to be ignored when this
  // MessageDifferencer compares messages.
  bool IsIgnored(const Message& message1, const Message& message2,
                 const FieldDescriptor* field,
                 const std::vector<SpecificField>& parent_fields);

  // Returns true if this unknown field is to be ignored when this
  // MessageDifferencer compares messages.
  bool IsUnknownFieldIgnored(const Message& message1, const Message& message2,
                             const SpecificField& field,
                             const std::vector<SpecificField>& parent_fields);

  // Returns MapKeyComparator* when this field has been configured to be treated
  // as a map or its is_map() return true.  If not, returns NULL.
  const MapKeyComparator* GetMapKeyComparator(
      const FieldDescriptor* field) const;

  // Attempts to match indices of a repeated field, so that the contained values
  // match. Clears output vectors and sets their values to indices of paired
  // messages, ie. if message1[0] matches message2[1], then match_list1[0] == 1
  // and match_list2[1] == 0. The unmatched indices are indicated by -1.
  // Assumes the repeated field is not treated as a simple list.
  // This method returns false if the match failed. However, it doesn't mean
  // that the comparison succeeds when this method returns true (you need to
  // double-check in this case).
  bool MatchRepeatedFieldIndices(
      const Message& message1, const Message& message2, int unpacked_any,
      const FieldDescriptor* repeated_field,
      const MapKeyComparator* key_comparator,
      const std::vector<SpecificField>& parent_fields,
      std::vector<int>* match_list1, std::vector<int>* match_list2);

  // Checks if index is equal to new_index in all the specific fields.
  static bool CheckPathChanged(const std::vector<SpecificField>& parent_fields);

  // ABSL_CHECKs that the given repeated field can be compared according to
  // new_comparison.
  void CheckRepeatedFieldComparisons(
      const FieldDescriptor* field,
      const RepeatedFieldComparison& new_comparison);

  Reporter* reporter_;
  DefaultFieldComparator default_field_comparator_;
  MessageFieldComparison message_field_comparison_;
  Scope scope_;
  absl::flat_hash_set<const FieldDescriptor*> force_compare_no_presence_fields_;
  absl::flat_hash_set<std::string> force_compare_failure_triggering_fields_;
  RepeatedFieldComparison repeated_field_comparison_;

  absl::flat_hash_map<const FieldDescriptor*, RepeatedFieldComparison>
      repeated_field_comparisons_;
  // Keeps track of MapKeyComparators that are created within
  // MessageDifferencer. These MapKeyComparators should be deleted
  // before MessageDifferencer is destroyed.
  // When TreatAsMap or TreatAsMapWithMultipleFieldsAsKey is called, we don't
  // store the supplied FieldDescriptors directly. Instead, a new
  // MapKeyComparator is created for comparison purpose.
  std::vector<MapKeyComparator*> owned_key_comparators_;
  absl::flat_hash_map<const FieldDescriptor*, const MapKeyComparator*>
      map_field_key_comparator_;
  MapEntryKeyComparator map_entry_key_comparator_;
  std::vector<std::unique_ptr<IgnoreCriteria>> ignore_criteria_;
  // Reused multiple times in RetrieveFields to avoid extra allocations
  std::vector<const FieldDescriptor*> tmp_message_fields_;

  absl::flat_hash_set<const FieldDescriptor*> ignored_fields_;

  union {
    DefaultFieldComparator* default_impl;
    FieldComparator* base;
  } field_comparator_ = {&default_field_comparator_};
  enum { kFCDefault, kFCBase } field_comparator_kind_ = kFCDefault;

  bool report_matches_;
  bool report_moves_;
  bool report_ignores_;
  bool force_compare_no_presence_ = false;

  std::string* output_string_;

  // Callback to post-process the matched indices to support SMART_LIST.
  std::function<void(std::vector<int>*, std::vector<int>*)>
      match_indices_for_smart_list_callback_;

  MessageDifferencer::UnpackAnyField unpack_any_field_;
};

// This class provides extra information to the FieldComparator::Compare
// function.
class PROTOBUF_EXPORT FieldContext {
 public:
  explicit FieldContext(
      std::vector<MessageDifferencer::SpecificField>* parent_fields)
      : parent_fields_(parent_fields) {}

  std::vector<MessageDifferencer::SpecificField>* parent_fields() const {
    return parent_fields_;
  }

 private:
  std::vector<MessageDifferencer::SpecificField>* parent_fields_;
};

}  // namespace util
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


#endif  // GOOGLE_PROTOBUF_UTIL_MESSAGE_DIFFERENCER_H__

