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
// RepeatedField and RepeatedPtrField are used by generated protocol message
// classes to manipulate repeated fields.  These classes are very similar to
// STL's vector, but include a number of optimizations found to be useful
// specifically in the case of Protocol Buffers.  RepeatedPtrField is
// particularly different from STL vector as it manages ownership of the
// pointers that it contains.
//
// This header covers RepeatedPtrField.

#ifndef GOOGLE_PROTOBUF_REPEATED_PTR_FIELD_H__
#define GOOGLE_PROTOBUF_REPEATED_PTR_FIELD_H__

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <CppAbseil/absl_base_attributes.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppProtobuf/google_protobuf_arena.hpp>
#include <CppProtobuf/google_protobuf_internal_visibility.hpp>
#include <CppProtobuf/google_protobuf_message_lite.hpp>
#include <CppProtobuf/google_protobuf_port.hpp>


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


#ifdef SWIG
#error "You cannot SWIG proto headers"
#endif

namespace google {
namespace protobuf {

class Message;
class Reflection;

template <typename T>
struct WeakRepeatedPtrField;

namespace internal {

class MergePartialFromCodedStreamHelper;
class SwapFieldHelper;


}  // namespace internal

namespace internal {
template <typename It>
class RepeatedPtrIterator;
template <typename It, typename VoidPtr>
class RepeatedPtrOverPtrsIterator;
}  // namespace internal

namespace internal {

template <typename Element>
inline void* NewT(Arena* a) {
  return GenericTypeHandler<Element>::New(a);
}

// Swaps two non-overlapping blocks of memory of size `N`
template <size_t N>
inline void memswap(char* PROTOBUF_RESTRICT a, char* PROTOBUF_RESTRICT b) {
  // `PROTOBUF_RESTRICT` tells compiler that blocks do not overlapping which
  // allows it to generate optimized code for swap_ranges.
  std::swap_ranges(a, a + N, b);
}

// type-traits helper for RepeatedPtrFieldBase: we only want to invoke
// arena-related "copy if on different arena" behavior if the necessary methods
// exist on the contained type. In particular, we rely on MergeFrom() existing
// as a general proxy for the fact that a copy will work, and we also provide a
// specific override for std::string*.
template <typename T>
struct TypeImplementsMergeBehaviorProbeForMergeFrom {
  typedef char HasMerge;
  typedef long HasNoMerge;

  // We accept either of:
  // - void MergeFrom(const T& other)
  // - bool MergeFrom(const T& other)
  //
  // We mangle these names a bit to avoid compatibility issues in 'unclean'
  // include environments that may have, e.g., "#define test ..." (yes, this
  // exists).
  template <typename U, typename RetType, RetType (U::*)(const U& arg)>
  struct CheckType;
  template <typename U>
  static HasMerge Check(CheckType<U, void, &U::MergeFrom>*);
  template <typename U>
  static HasMerge Check(CheckType<U, bool, &U::MergeFrom>*);
  template <typename U>
  static HasNoMerge Check(...);

  // Resolves to either std::true_type or std::false_type.
  typedef std::integral_constant<bool,
                                 (sizeof(Check<T>(0)) == sizeof(HasMerge))>
      type;
};

template <typename T, typename = void>
struct TypeImplementsMergeBehavior
    : TypeImplementsMergeBehaviorProbeForMergeFrom<T> {};


template <>
struct TypeImplementsMergeBehavior<std::string> {
  typedef std::true_type type;
};

template <typename T>
struct IsMovable
    : std::integral_constant<bool, std::is_move_constructible<T>::value &&
                                       std::is_move_assignable<T>::value> {};

// A trait that tells offset of `T::arena_`.
//
// Do not use this struct - it exists for internal use only.
template <typename T>
struct ArenaOffsetHelper {
  constexpr static size_t value = offsetof(T, arena_);
};

// This is the common base class for RepeatedPtrFields.  It deals only in void*
// pointers.  Users should not use this interface directly.
//
// The methods of this interface correspond to the methods of RepeatedPtrField,
// but may have a template argument called TypeHandler.  Its signature is:
//   class TypeHandler {
//    public:
//     typedef MyType Type;
//     static Type* New();
//     static Type* NewFromPrototype(const Type* prototype,
//                                       Arena* arena);
//     static void Delete(Type*);
//     static void Clear(Type*);
//     static void Merge(const Type& from, Type* to);
//
//     // Only needs to be implemented if SpaceUsedExcludingSelf() is called.
//     static int SpaceUsedLong(const Type&);
//   };
class PROTOBUF_EXPORT RepeatedPtrFieldBase {
  template <typename Handler>
  using Value = typename Handler::Type;

  static constexpr int kSSOCapacity = 1;

  using ElementFactory = void* (*)(Arena*);

 protected:
  // We use the same Handler for all Message types to deduplicate generated
  // code.
  template <typename Handler>
  using CommonHandler = typename std::conditional<
      std::is_base_of<MessageLite, Value<Handler>>::value,
      internal::GenericTypeHandler<MessageLite>, Handler>::type;

  constexpr RepeatedPtrFieldBase()
      : tagged_rep_or_elem_(nullptr),
        current_size_(0),
        total_size_(kSSOCapacity),
        arena_(nullptr) {}
  explicit RepeatedPtrFieldBase(Arena* arena)
      : tagged_rep_or_elem_(nullptr),
        current_size_(0),
        total_size_(kSSOCapacity),
        arena_(arena) {}

  RepeatedPtrFieldBase(const RepeatedPtrFieldBase&) = delete;
  RepeatedPtrFieldBase& operator=(const RepeatedPtrFieldBase&) = delete;

  ~RepeatedPtrFieldBase() {
#ifndef NDEBUG
    // Try to trigger segfault / asan failure in non-opt builds. If arena_
    // lifetime has ended before the destructor.
    if (arena_) (void)arena_->SpaceAllocated();
#endif
  }

  bool empty() const { return current_size_ == 0; }
  int size() const { return current_size_; }
  int Capacity() const { return total_size_; }

  template <typename TypeHandler>
  const Value<TypeHandler>& at(int index) const {
    ABSL_CHECK_GE(index, 0);
    ABSL_CHECK_LT(index, current_size_);
    return *cast<TypeHandler>(element_at(index));
  }

  template <typename TypeHandler>
  Value<TypeHandler>& at(int index) {
    ABSL_CHECK_GE(index, 0);
    ABSL_CHECK_LT(index, current_size_);
    return *cast<TypeHandler>(element_at(index));
  }

  template <typename TypeHandler>
  Value<TypeHandler>* Mutable(int index) {
    ABSL_DCHECK_GE(index, 0);
    ABSL_DCHECK_LT(index, current_size_);
    return cast<TypeHandler>(element_at(index));
  }

  template <typename Handler>
  Value<Handler>* Add() {
    return cast<Handler>(AddOutOfLineHelper(NewT<Value<Handler>>));
  }

  template <typename TypeHandler>
  Value<TypeHandler>* Add(const Value<TypeHandler>* prototype) {
    if (current_size_ < allocated_size()) {
      return cast<TypeHandler>(
          element_at(ExchangeCurrentSize(current_size_ + 1)));
    }
    auto* result = TypeHandler::NewFromPrototype(prototype, arena_);
    return cast<TypeHandler>(AddOutOfLineHelper(result));
  }

  template <
      typename TypeHandler,
      typename std::enable_if<TypeHandler::Movable::value>::type* = nullptr>
  inline void Add(Value<TypeHandler>&& value) {
    if (current_size_ < allocated_size()) {
      *cast<TypeHandler>(element_at(ExchangeCurrentSize(current_size_ + 1))) =
          std::move(value);
      return;
    }
    MaybeExtend();
    if (!using_sso()) ++rep()->allocated_size;
    auto* result = TypeHandler::New(arena_, std::move(value));
    element_at(ExchangeCurrentSize(current_size_ + 1)) = result;
  }

  template <typename TypeHandler>
  void Delete(int index) {
    ABSL_DCHECK_GE(index, 0);
    ABSL_DCHECK_LT(index, current_size_);
    using H = CommonHandler<TypeHandler>;
    Delete<H>(element_at(index), arena_);
  }

  // Must be called from destructor.
  template <typename TypeHandler>
  void Destroy() {
    using H = CommonHandler<TypeHandler>;
    if (arena_ != nullptr) return;
    int n = allocated_size();
    void** elems = elements();
    for (int i = 0; i < n; i++) {
      Delete<H>(elems[i], nullptr);
    }
    if (!using_sso()) {
      internal::SizedDelete(rep(),
                            total_size_ * sizeof(elems[0]) + kRepHeaderSize);
    }
  }

  bool NeedsDestroy() const {
    return tagged_rep_or_elem_ != nullptr && arena_ == nullptr;
  }
  void DestroyProtos();  // implemented in the cc file

 public:
  // The next few methods are public so that they can be called from generated
  // code when implicit weak fields are used, but they should never be called by
  // application code.

  template <typename TypeHandler>
  const Value<TypeHandler>& Get(int index) const {
    ABSL_DCHECK_GE(index, 0);
    ABSL_DCHECK_LT(index, current_size_);
    return *cast<TypeHandler>(element_at(index));
  }

  // Creates and adds an element using the given prototype, without introducing
  // a link-time dependency on the concrete message type. This method is used to
  // implement implicit weak fields. The prototype may be nullptr, in which case
  // an ImplicitWeakMessage will be used as a placeholder.
  MessageLite* AddWeak(const MessageLite* prototype);

  template <typename TypeHandler>
  void Clear() {
    const int n = current_size_;
    ABSL_DCHECK_GE(n, 0);
    if (n > 0) {
      using H = CommonHandler<TypeHandler>;
      ClearNonEmpty<H>();
    }
  }

  // Message creating functor: used in MergeFrom<T>()
  template <typename T>
  static MessageLite* CopyMessage(Arena* arena, const MessageLite& src) {
    T* msg = Arena::CreateMaybeMessage<T>(arena);
    msg->MergeFrom(static_cast<const T&>(src));
    return msg;
  }

  // Appends all message values from `from` to this instance.
  template <typename T>
  void MergeFrom(const RepeatedPtrFieldBase& from) {
    static_assert(std::is_base_of<MessageLite, T>::value, "");
    MergeFromConcreteMessage(from, CopyMessage<T>);
  }

  inline void InternalSwap(RepeatedPtrFieldBase* PROTOBUF_RESTRICT rhs) {
    ABSL_DCHECK(this != rhs);

    // Swap all fields except arena pointer at once.
    internal::memswap<ArenaOffsetHelper<RepeatedPtrFieldBase>::value>(
        reinterpret_cast<char*>(this), reinterpret_cast<char*>(rhs));
  }

  // Prepares the container for adding elements via `AddAllocatedForParse`.
  // It ensures we have no preallocated elements in the array.
  //  Returns true if the invariants hold and `AddAllocatedForParse` can be
  //  used.
  bool PrepareForParse() { return allocated_size() == current_size_; }

  // Similar to `AddAllocated` but faster.
  // Can only be invoked after a call to `PrepareForParse` that returned `true`,
  // or other calls to `AddAllocatedForParse`.
  template <typename TypeHandler>
  void AddAllocatedForParse(Value<TypeHandler>* value) {
    ABSL_DCHECK_EQ(current_size_, allocated_size());
    MaybeExtend();
    element_at(current_size_++) = value;
    if (!using_sso()) ++rep()->allocated_size;
  }

 protected:
  template <typename TypeHandler>
  void RemoveLast() {
    ABSL_DCHECK_GT(current_size_, 0);
    ExchangeCurrentSize(current_size_ - 1);
    using H = CommonHandler<TypeHandler>;
    H::Clear(cast<H>(element_at(current_size_)));
  }

  template <typename TypeHandler>
  void CopyFrom(const RepeatedPtrFieldBase& other) {
    if (&other == this) return;
    RepeatedPtrFieldBase::Clear<TypeHandler>();
    if (other.empty()) return;
    RepeatedPtrFieldBase::MergeFrom<typename TypeHandler::Type>(other);
  }

  void CloseGap(int start, int num);

  void Reserve(int capacity);

  template <typename TypeHandler>
  static inline Value<TypeHandler>* copy(const Value<TypeHandler>* value) {
    using H = CommonHandler<TypeHandler>;
    auto* new_value = H::NewFromPrototype(value, nullptr);
    H::Merge(*value, new_value);
    return cast<TypeHandler>(new_value);
  }

  // Used for constructing iterators.
  void* const* raw_data() const { return elements(); }
  void** raw_mutable_data() { return elements(); }

  template <typename TypeHandler>
  Value<TypeHandler>** mutable_data() {
    // TODO:  Breaks C++ aliasing rules.  We should probably remove this
    //   method entirely.
    return reinterpret_cast<Value<TypeHandler>**>(raw_mutable_data());
  }

  template <typename TypeHandler>
  const Value<TypeHandler>* const* data() const {
    // TODO:  Breaks C++ aliasing rules.  We should probably remove this
    //   method entirely.
    return reinterpret_cast<const Value<TypeHandler>* const*>(raw_data());
  }

  template <typename TypeHandler>
  PROTOBUF_NDEBUG_INLINE void Swap(RepeatedPtrFieldBase* other) {
#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() != nullptr && GetArena() == other->GetArena())
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() == other->GetArena())
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
    {
      InternalSwap(other);
    } else {
      SwapFallback<TypeHandler>(other);
    }
  }

  void SwapElements(int index1, int index2) {
    using std::swap;  // enable ADL with fallback
    swap(element_at(index1), element_at(index2));
  }

  template <typename TypeHandler>
  PROTOBUF_NOINLINE size_t SpaceUsedExcludingSelfLong() const {
    size_t allocated_bytes =
        using_sso()
            ? 0
            : static_cast<size_t>(total_size_) * sizeof(void*) + kRepHeaderSize;
    const int n = allocated_size();
    void* const* elems = elements();
    for (int i = 0; i < n; ++i) {
      allocated_bytes +=
          TypeHandler::SpaceUsedLong(*cast<TypeHandler>(elems[i]));
    }
    return allocated_bytes;
  }

  // Advanced memory management --------------------------------------

  // Like Add(), but if there are no cleared objects to use, returns nullptr.
  template <typename TypeHandler>
  Value<TypeHandler>* AddFromCleared() {
    if (current_size_ < allocated_size()) {
      return cast<TypeHandler>(
          element_at(ExchangeCurrentSize(current_size_ + 1)));
    } else {
      return nullptr;
    }
  }

  template <typename TypeHandler>
  void AddAllocated(Value<TypeHandler>* value) {
    typename TypeImplementsMergeBehavior<Value<TypeHandler>>::type t;
    AddAllocatedInternal<TypeHandler>(value, t);
  }

  template <typename TypeHandler>
  void UnsafeArenaAddAllocated(Value<TypeHandler>* value) {
    // Make room for the new pointer.
    if (current_size_ == total_size_) {
      // The array is completely full with no cleared objects, so grow it.
      Reserve(total_size_ + 1);
      ++rep()->allocated_size;
    } else if (allocated_size() == total_size_) {
      // There is no more space in the pointer array because it contains some
      // cleared objects awaiting reuse.  We don't want to grow the array in
      // this case because otherwise a loop calling AddAllocated() followed by
      // Clear() would leak memory.
      using H = CommonHandler<TypeHandler>;
      Delete<H>(element_at(current_size_), arena_);
    } else if (current_size_ < allocated_size()) {
      // We have some cleared objects.  We don't care about their order, so we
      // can just move the first one to the end to make space.
      element_at(allocated_size()) = element_at(current_size_);
      ++rep()->allocated_size;
    } else {
      // There are no cleared objects.
      if (!using_sso()) ++rep()->allocated_size;
    }

    element_at(ExchangeCurrentSize(current_size_ + 1)) = value;
  }

  template <typename TypeHandler>
  PROTOBUF_NODISCARD Value<TypeHandler>* ReleaseLast() {
    typename TypeImplementsMergeBehavior<Value<TypeHandler>>::type t;
    return ReleaseLastInternal<TypeHandler>(t);
  }

  // Releases and returns the last element, but does not do out-of-arena copy.
  // Instead, just returns the raw pointer to the contained element in the
  // arena.
  template <typename TypeHandler>
  Value<TypeHandler>* UnsafeArenaReleaseLast() {
    ABSL_DCHECK_GT(current_size_, 0);
    ExchangeCurrentSize(current_size_ - 1);
    auto* result = cast<TypeHandler>(element_at(current_size_));
    if (using_sso()) {
      tagged_rep_or_elem_ = nullptr;
    } else {
      --rep()->allocated_size;
      if (current_size_ < allocated_size()) {
        // There are cleared elements on the end; replace the removed element
        // with the last allocated element.
        element_at(current_size_) = element_at(allocated_size());
      }
    }
    return result;
  }

  int ClearedCount() const { return allocated_size() - current_size_; }

  template <typename TypeHandler>
  void AddCleared(Value<TypeHandler>* value) {
    ABSL_DCHECK(GetArena() == nullptr) << "AddCleared() can only be used on a "
                                          "RepeatedPtrField not on an arena.";
    ABSL_DCHECK(TypeHandler::GetArena(value) == nullptr)
        << "AddCleared() can only accept values not on an arena.";
    MaybeExtend();
    if (using_sso()) {
      tagged_rep_or_elem_ = value;
    } else {
      element_at(rep()->allocated_size++) = value;
    }
  }

  template <typename TypeHandler>
  PROTOBUF_NODISCARD Value<TypeHandler>* ReleaseCleared() {
    ABSL_DCHECK(GetArena() == nullptr)
        << "ReleaseCleared() can only be used on a RepeatedPtrField not on "
        << "an arena.";
    ABSL_DCHECK(tagged_rep_or_elem_ != nullptr);
    ABSL_DCHECK_GT(allocated_size(), current_size_);
    if (using_sso()) {
      auto* result = cast<TypeHandler>(tagged_rep_or_elem_);
      tagged_rep_or_elem_ = nullptr;
      return result;
    } else {
      return cast<TypeHandler>(element_at(--rep()->allocated_size));
    }
  }

  // AddAllocated version that implements arena-safe copying behavior.
  template <typename TypeHandler>
  void AddAllocatedInternal(Value<TypeHandler>* value, std::true_type) {
    Arena* element_arena = TypeHandler::GetArena(value);
    Arena* arena = GetArena();
    if (arena == element_arena && allocated_size() < total_size_) {
      // Fast path: underlying arena representation (tagged pointer) is equal to
      // our arena pointer, and we can add to array without resizing it (at
      // least one slot that is not allocated).
      void** elems = elements();
      if (current_size_ < allocated_size()) {
        // Make space at [current] by moving first allocated element to end of
        // allocated list.
        elems[allocated_size()] = elems[current_size_];
      }
      elems[ExchangeCurrentSize(current_size_ + 1)] = value;
      if (!using_sso()) ++rep()->allocated_size;
    } else {
      AddAllocatedSlowWithCopy<TypeHandler>(value, element_arena, arena);
    }
  }

  // AddAllocated version that does not implement arena-safe copying behavior.
  template <typename TypeHandler>
  void AddAllocatedInternal(Value<TypeHandler>* value, std::false_type) {
    if (allocated_size() < total_size_) {
      // Fast path: underlying arena representation (tagged pointer) is equal to
      // our arena pointer, and we can add to array without resizing it (at
      // least one slot that is not allocated).
      void** elems = elements();
      if (current_size_ < allocated_size()) {
        // Make space at [current] by moving first allocated element to end of
        // allocated list.
        elems[allocated_size()] = elems[current_size_];
      }
      elems[ExchangeCurrentSize(current_size_ + 1)] = value;
      if (!using_sso()) ++rep()->allocated_size;
    } else {
      UnsafeArenaAddAllocated<TypeHandler>(value);
    }
  }

  // Slowpath handles all cases, copying if necessary.
  template <typename TypeHandler>
  PROTOBUF_NOINLINE void AddAllocatedSlowWithCopy(
      // Pass value_arena and my_arena to avoid duplicate virtual call (value)
      // or load (mine).
      Value<TypeHandler>* value, Arena* value_arena, Arena* my_arena) {
    // Ensure that either the value is in the same arena, or if not, we do the
    // appropriate thing: Own() it (if it's on heap and we're in an arena) or
    // copy it to our arena/heap (otherwise).
    if (my_arena != nullptr && value_arena == nullptr) {
      my_arena->Own(value);
    } else if (my_arena != value_arena) {
      auto* new_value = TypeHandler::NewFromPrototype(value, my_arena);
      using H = CommonHandler<TypeHandler>;
      H::Merge(*value, new_value);
      H::Delete(value, value_arena);
      value = new_value;
    }

    UnsafeArenaAddAllocated<TypeHandler>(value);
  }

  template <typename TypeHandler>
  Value<TypeHandler>* ReleaseLastInternal(std::true_type) {
    // ReleaseLast() for types that implement merge/copy behavior.
    // First, release an element.
    Value<TypeHandler>* result = UnsafeArenaReleaseLast<TypeHandler>();
    // Now perform a copy if we're on an arena.
    Arena* arena = GetArena();

#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
    auto* new_result = copy<TypeHandler>(result);
    if (arena == nullptr) delete result;
#else   // PROTOBUF_FORCE_COPY_IN_RELEASE
    auto* new_result = (arena == nullptr) ? result : copy<TypeHandler>(result);
#endif  // !PROTOBUF_FORCE_COPY_IN_RELEASE
    return new_result;
  }

  template <typename TypeHandler>
  Value<TypeHandler>* ReleaseLastInternal(std::false_type) {
    // ReleaseLast() for types that *do not* implement merge/copy behavior --
    // this is the same as UnsafeArenaReleaseLast(). Note that we
    // ABSL_DCHECK-fail if we're on an arena, since the user really should
    // implement the copy operation in this case.
    ABSL_DCHECK(GetArena() == nullptr)
        << "ReleaseLast() called on a RepeatedPtrField that is on an arena, "
        << "with a type that does not implement MergeFrom. This is unsafe; "
        << "please implement MergeFrom for your type.";
    return UnsafeArenaReleaseLast<TypeHandler>();
  }

  template <typename TypeHandler>
  PROTOBUF_NOINLINE void SwapFallback(RepeatedPtrFieldBase* other) {
#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    ABSL_DCHECK(GetArena() == nullptr || other->GetArena() != GetArena());
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
    ABSL_DCHECK(other->GetArena() != GetArena());
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP

    // Copy semantics in this case. We try to improve efficiency by placing the
    // temporary on |other|'s arena so that messages are copied twice rather
    // than three times.
    RepeatedPtrFieldBase temp(other->GetArena());
    if (!this->empty()) {
      temp.MergeFrom<typename TypeHandler::Type>(*this);
    }
    this->CopyFrom<TypeHandler>(*other);
    other->InternalSwap(&temp);
    temp.Destroy<TypeHandler>();  // Frees rep_ if `other` had no arena.
  }

  // Gets the Arena on which this RepeatedPtrField stores its elements.
  inline Arena* GetArena() const { return arena_; }

  inline Arena* GetOwningArena() const { return arena_; }

 private:
  using InternalArenaConstructable_ = void;
  using DestructorSkippable_ = void;

  template <typename T>
  friend class Arena::InternalHelper;

  // ExtensionSet stores repeated message extensions as
  // RepeatedPtrField<MessageLite>, but non-lite ExtensionSets need to implement
  // SpaceUsedLong(), and thus need to call SpaceUsedExcludingSelfLong()
  // reinterpreting MessageLite as Message.  ExtensionSet also needs to make use
  // of AddFromCleared(), which is not part of the public interface.
  friend class ExtensionSet;

  // The MapFieldBase implementation needs to call protected methods directly,
  // reinterpreting pointers as being to Message instead of a specific Message
  // subclass.
  friend class MapFieldBase;
  friend struct MapFieldTestPeer;

  // The table-driven MergePartialFromCodedStream implementation needs to
  // operate on RepeatedPtrField<MessageLite>.
  friend class MergePartialFromCodedStreamHelper;

  friend class AccessorHelper;

  template <typename T>
  friend struct google::protobuf::WeakRepeatedPtrField;

  friend class internal::TcParser;  // TODO: Remove this friend.

  // Expose offset of `arena_` without exposing the member itself.
  // Used to optimize code size of `InternalSwap` method.
  template <typename T>
  friend struct ArenaOffsetHelper;

  // The reflection implementation needs to call protected methods directly,
  // reinterpreting pointers as being to Message instead of a specific Message
  // subclass.
  friend class google::protobuf::Reflection;
  friend class internal::SwapFieldHelper;

  // Concrete Arena enabled copy function used to copy messages instances.
  // This follows the `Arena::CreateMaybeMessage` signature so that the compiler
  // can have the inlined call into the out of line copy function(s) simply pass
  // the address of `Arena::CreateMaybeMessage` 'as is'.
  using CopyFn = MessageLite* (*)(Arena*, const MessageLite&);

  struct Rep {
    int allocated_size;
    // Here we declare a huge array as a way of approximating C's "flexible
    // array member" feature without relying on undefined behavior.
    void* elements[(std::numeric_limits<int>::max() - 2 * sizeof(int)) /
                   sizeof(void*)];
  };

  static constexpr size_t kRepHeaderSize = offsetof(Rep, elements);

  // Replaces current_size_ with new_size and returns the previous value of
  // current_size_. This function is intended to be the only place where
  // current_size_ is modified.
  inline int ExchangeCurrentSize(int new_size) {
    return std::exchange(current_size_, new_size);
  }

  void* const* elements() const {
    return using_sso() ? &tagged_rep_or_elem_ : +rep()->elements;
  }
  void** elements() {
    return using_sso() ? &tagged_rep_or_elem_ : +rep()->elements;
  }

  void*& element_at(int index) {
    if (using_sso()) {
      ABSL_DCHECK_EQ(index, 0);
      return tagged_rep_or_elem_;
    }
    return rep()->elements[index];
  }
  const void* element_at(int index) const {
    return const_cast<RepeatedPtrFieldBase*>(this)->element_at(index);
  }

  int allocated_size() const {
    return using_sso() ? (tagged_rep_or_elem_ != nullptr ? 1 : 0)
                       : rep()->allocated_size;
  }
  Rep* rep() {
    ABSL_DCHECK(!using_sso());
    return reinterpret_cast<Rep*>(
        reinterpret_cast<uintptr_t>(tagged_rep_or_elem_) - 1);
  }
  const Rep* rep() const {
    return const_cast<RepeatedPtrFieldBase*>(this)->rep();
  }

  bool using_sso() const {
    return (reinterpret_cast<uintptr_t>(tagged_rep_or_elem_) & 1) == 0;
  }

  template <typename TypeHandler>
  static inline Value<TypeHandler>* cast(void* element) {
    return reinterpret_cast<Value<TypeHandler>*>(element);
  }
  template <typename TypeHandler>
  static inline const Value<TypeHandler>* cast(const void* element) {
    return reinterpret_cast<const Value<TypeHandler>*>(element);
  }
  template <typename TypeHandler>
  static inline void Delete(void* obj, Arena* arena) {
    TypeHandler::Delete(cast<TypeHandler>(obj), arena);
  }

  // Out-of-line helper routine for Clear() once the inlined check has
  // determined the container is non-empty
  template <typename TypeHandler>
  PROTOBUF_NOINLINE void ClearNonEmpty() {
    const int n = current_size_;
    void* const* elems = elements();
    int i = 0;
    ABSL_DCHECK_GT(n, 0);
    // do/while loop to avoid initial test because we know n > 0
    do {
      TypeHandler::Clear(cast<TypeHandler>(elems[i++]));
    } while (i < n);
    ExchangeCurrentSize(0);
  }

  // Merges messages from `from` into available, cleared messages sitting in the
  // range `[size(), allocated_size())`. Returns the number of message merged
  // which is `ClearedCount(), from.size())`.
  // Note that this function does explicitly NOT update `current_size_`.
  // This function is out of line as it should be the slow path: this scenario
  // only happens when a caller constructs and fills a repeated field, then
  // shrinks it, and then merges additional messages into it.
  int MergeIntoClearedMessages(const RepeatedPtrFieldBase& from);

  // Appends all messages from `from` to this instance, using the
  // provided `copy_fn` copy function to copy existing messages.
  void MergeFromConcreteMessage(const RepeatedPtrFieldBase& from,
                                CopyFn copy_fn);

  // Extends capacity by at least |extend_amount|.
  //
  // Pre-condition: |extend_amount| must be > 0.
  void** InternalExtend(int extend_amount);

  // Ensures that capacity is big enough to store one more allocated element.
  inline void MaybeExtend() {
    if (using_sso() ? (tagged_rep_or_elem_ != nullptr)
                    : (rep()->allocated_size == total_size_)) {
      ABSL_DCHECK_EQ(allocated_size(), Capacity());
      InternalExtend(1);
    } else {
      ABSL_DCHECK_NE(allocated_size(), Capacity());
    }
  }

  // Ensures that capacity is at least `n` elements.
  // Returns a pointer to the element directly beyond the last element.
  inline void** InternalReserve(int n) {
    if (n <= total_size_) {
      void** elements = using_sso() ? &tagged_rep_or_elem_ : rep()->elements;
      return elements + current_size_;
    }
    return InternalExtend(n - total_size_);
  }

  // Internal helper for Add: adds "obj" as the next element in the
  // array, including potentially resizing the array with Reserve if
  // needed
  void* AddOutOfLineHelper(void* obj);
  void* AddOutOfLineHelper(ElementFactory factory);

  // A few notes on internal representation:
  //
  // We use an indirected approach, with struct Rep, to keep
  // sizeof(RepeatedPtrFieldBase) equivalent to what it was before arena support
  // was added; namely, 3 8-byte machine words on x86-64. An instance of Rep is
  // allocated only when the repeated field is non-empty, and it is a
  // dynamically-sized struct (the header is directly followed by elements[]).
  // We place arena_ and current_size_ directly in the object to avoid cache
  // misses due to the indirection, because these fields are checked frequently.
  // Placing all fields directly in the RepeatedPtrFieldBase instance would cost
  // significant performance for memory-sensitive workloads.
  void* tagged_rep_or_elem_;
  int current_size_;
  int total_size_;
  Arena* arena_;
};

// Appends all message values from `from` to this instance using the abstract
// message interface. This overload is used in places like reflection and
// other locations where the underlying type is unavailable
template <>
void RepeatedPtrFieldBase::MergeFrom<MessageLite>(
    const RepeatedPtrFieldBase& from);

template <>
inline void RepeatedPtrFieldBase::MergeFrom<Message>(
    const RepeatedPtrFieldBase& from) {
  return MergeFrom<MessageLite>(from);
}

// Appends all `std::string` values from `from` to this instance.
template <>
void RepeatedPtrFieldBase::MergeFrom<std::string>(
    const RepeatedPtrFieldBase& from);


PROTOBUF_EXPORT void InternalOutOfLineDeleteMessageLite(MessageLite* message);

template <typename GenericType>
class GenericTypeHandler {
 public:
  typedef GenericType Type;
  using Movable = IsMovable<GenericType>;

  static inline GenericType* New(Arena* arena) {
    return Arena::CreateMaybeMessage<Type>(arena);
  }
  static inline GenericType* New(Arena* arena, GenericType&& value) {
    return Arena::Create<GenericType>(arena, std::move(value));
  }
  static inline GenericType* NewFromPrototype(const GenericType* /*prototype*/,
                                              Arena* arena = nullptr) {
    return New(arena);
  }
  static inline void Delete(GenericType* value, Arena* arena) {
    if (arena != nullptr) return;
#ifdef __cpp_if_constexpr
    if constexpr (std::is_base_of<MessageLite, GenericType>::value) {
      // Using virtual destructor to reduce generated code size that would have
      // happened otherwise due to inlined `~GenericType`.
      InternalOutOfLineDeleteMessageLite(value);
    } else {
      delete value;
    }
#else
    delete value;
#endif
  }
  static inline Arena* GetArena(GenericType* value) {
    return Arena::InternalGetArena(value);
  }

  static inline void Clear(GenericType* value) { value->Clear(); }
  static void Merge(const GenericType& from, GenericType* to);
  static inline size_t SpaceUsedLong(const GenericType& value) {
    return value.SpaceUsedLong();
  }
};

// NewFromPrototypeHelper() is not defined inline here, as we will need to do a
// virtual function dispatch anyways to go from Message* to call New/Merge. (The
// additional helper is needed as a workaround for MSVC.)
PROTOBUF_EXPORT MessageLite* NewFromPrototypeHelper(
    const MessageLite* prototype, Arena* arena);

template <>
inline MessageLite* GenericTypeHandler<MessageLite>::NewFromPrototype(
    const MessageLite* prototype, Arena* arena) {
  return NewFromPrototypeHelper(prototype, arena);
}
template <>
inline Arena* GenericTypeHandler<MessageLite>::GetArena(MessageLite* value) {
  return value->GetArena();
}

template <typename GenericType>
PROTOBUF_NOINLINE inline void GenericTypeHandler<GenericType>::Merge(
    const GenericType& from, GenericType* to) {
  to->MergeFrom(from);
}
template <>
PROTOBUF_EXPORT void GenericTypeHandler<MessageLite>::Merge(
    const MessageLite& from, MessageLite* to);

template <>
inline void GenericTypeHandler<std::string>::Clear(std::string* value) {
  value->clear();
}
template <>
void GenericTypeHandler<std::string>::Merge(const std::string& from,
                                            std::string* to);

// Message specialization bodies defined in message.cc. This split is necessary
// to allow proto2-lite (which includes this header) to be independent of
// Message.
template <>
PROTOBUF_EXPORT Message* GenericTypeHandler<Message>::NewFromPrototype(
    const Message* prototype, Arena* arena);
template <>
PROTOBUF_EXPORT Arena* GenericTypeHandler<Message>::GetArena(Message* value);

class StringTypeHandler {
 public:
  typedef std::string Type;
  using Movable = IsMovable<Type>;

  static PROTOBUF_NOINLINE std::string* New(Arena* arena) {
    return Arena::Create<std::string>(arena);
  }
  static PROTOBUF_NOINLINE std::string* New(Arena* arena, std::string&& value) {
    return Arena::Create<std::string>(arena, std::move(value));
  }
  static inline std::string* NewFromPrototype(const std::string*,
                                              Arena* arena) {
    return New(arena);
  }
  static inline Arena* GetArena(std::string*) { return nullptr; }
  static inline void Delete(std::string* value, Arena* arena) {
    if (arena == nullptr) {
      delete value;
    }
  }
  static inline void Clear(std::string* value) { value->clear(); }
  static inline void Merge(const std::string& from, std::string* to) {
    *to = from;
  }
  static size_t SpaceUsedLong(const std::string& value) {
    return sizeof(value) + StringSpaceUsedExcludingSelfLong(value);
  }
};

}  // namespace internal

// RepeatedPtrField is like RepeatedField, but used for repeated strings or
// Messages.
template <typename Element>
class RepeatedPtrField final : private internal::RepeatedPtrFieldBase {
  static_assert(!std::is_const<Element>::value,
                "We do not support const value types.");
  static_assert(!std::is_volatile<Element>::value,
                "We do not support volatile value types.");
  static_assert(!std::is_pointer<Element>::value,
                "We do not support pointer value types.");
  static_assert(!std::is_reference<Element>::value,
                "We do not support reference value types.");
  static constexpr PROTOBUF_ALWAYS_INLINE void StaticValidityCheck() {
    static_assert(
        absl::disjunction<
            internal::is_supported_string_type<Element>,
            internal::is_supported_message_type<Element>>::value,
        "We only support string and Message types in RepeatedPtrField.");
  }

 public:
  using value_type = Element;
  using size_type = int;
  using difference_type = ptrdiff_t;
  using reference = Element&;
  using const_reference = const Element&;
  using pointer = Element*;
  using const_pointer = const Element*;
  using iterator = internal::RepeatedPtrIterator<Element>;
  using const_iterator = internal::RepeatedPtrIterator<const Element>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  // Custom STL-like iterator that iterates over and returns the underlying
  // pointers to Element rather than Element itself.
  using pointer_iterator =
      internal::RepeatedPtrOverPtrsIterator<Element*, void*>;
  using const_pointer_iterator =
      internal::RepeatedPtrOverPtrsIterator<const Element* const,
                                            const void* const>;

  constexpr RepeatedPtrField();

  // Arena enabled constructors: for internal use only.
  RepeatedPtrField(internal::InternalVisibility, Arena* arena)
      : RepeatedPtrField(arena) {}
  RepeatedPtrField(internal::InternalVisibility, Arena* arena,
                   const RepeatedPtrField& rhs)
      : RepeatedPtrField(arena, rhs) {}

  // TODO: make constructor private
  explicit RepeatedPtrField(Arena* arena);

  template <typename Iter,
            typename = typename std::enable_if<std::is_constructible<
                Element, decltype(*std::declval<Iter>())>::value>::type>
  RepeatedPtrField(Iter begin, Iter end);

  RepeatedPtrField(const RepeatedPtrField& other)
      : RepeatedPtrField(nullptr, other) {}
  RepeatedPtrField& operator=(const RepeatedPtrField& other)
      ABSL_ATTRIBUTE_LIFETIME_BOUND;

  RepeatedPtrField(RepeatedPtrField&& other) noexcept;
  RepeatedPtrField& operator=(RepeatedPtrField&& other) noexcept
      ABSL_ATTRIBUTE_LIFETIME_BOUND;

  ~RepeatedPtrField();

  bool empty() const;
  int size() const;

  const_reference Get(int index) const ABSL_ATTRIBUTE_LIFETIME_BOUND;
  pointer Mutable(int index) ABSL_ATTRIBUTE_LIFETIME_BOUND;

  // Unlike std::vector, adding an element to a RepeatedPtrField doesn't always
  // make a new element; it might re-use an element left over from when the
  // field was Clear()'d or resize()'d smaller.  For this reason, Add() is the
  // fastest API for adding a new element.
  pointer Add() ABSL_ATTRIBUTE_LIFETIME_BOUND;

  // `Add(std::move(value));` is equivalent to `*Add() = std::move(value);`
  // It will either move-construct to the end of this field, or swap value
  // with the new-or-recycled element at the end of this field.  Note that
  // this operation is very slow if this RepeatedPtrField is not on the
  // same Arena, if any, as `value`.
  void Add(Element&& value);

  // Copying to the end of this RepeatedPtrField is slowest of all; it can't
  // reliably copy-construct to the last element of this RepeatedPtrField, for
  // example (unlike std::vector).
  // We currently block this API.  The right way to add to the end is to call
  // Add() and modify the element it points to.
  // If you must add an existing value, call `*Add() = value;`
  void Add(const Element& value) = delete;

  // Append elements in the range [begin, end) after reserving
  // the appropriate number of elements.
  template <typename Iter>
  void Add(Iter begin, Iter end);

  const_reference operator[](int index) const ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return Get(index);
  }
  reference operator[](int index) ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return *Mutable(index);
  }

  const_reference at(int index) const ABSL_ATTRIBUTE_LIFETIME_BOUND;
  reference at(int index) ABSL_ATTRIBUTE_LIFETIME_BOUND;

  // Removes the last element in the array.
  // Ownership of the element is retained by the array.
  void RemoveLast();

  // Deletes elements with indices in the range [start .. start+num-1].
  // Caution: moves all elements with indices [start+num .. ].
  // Calling this routine inside a loop can cause quadratic behavior.
  void DeleteSubrange(int start, int num);

  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear();
  void MergeFrom(const RepeatedPtrField& other);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void CopyFrom(const RepeatedPtrField& other);

  // Replaces the contents with RepeatedPtrField(begin, end).
  template <typename Iter>
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Assign(Iter begin, Iter end);

  // Reserves space to expand the field to at least the given size.  This only
  // resizes the pointer array; it doesn't allocate any objects.  If the
  // array is grown, it will always be at least doubled in size.
  void Reserve(int new_size);

  int Capacity() const;

  // Gets the underlying array.  This pointer is possibly invalidated by
  // any add or remove operation.
  Element**
  mutable_data() ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const Element* const* data() const ABSL_ATTRIBUTE_LIFETIME_BOUND;

  // Swaps entire contents with "other". If they are on separate arenas, then
  // copies data.
  void Swap(RepeatedPtrField* other);

  // Swaps entire contents with "other". Caller should guarantee that either
  // both fields are on the same arena or both are on the heap. Swapping between
  // different arenas with this function is disallowed and is caught via
  // ABSL_DCHECK.
  void UnsafeArenaSwap(RepeatedPtrField* other);

  // Swaps two elements.
  void SwapElements(int index1, int index2);

  iterator begin() ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const_iterator begin() const ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const_iterator cbegin() const ABSL_ATTRIBUTE_LIFETIME_BOUND;
  iterator end() ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const_iterator end() const ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const_iterator cend() const ABSL_ATTRIBUTE_LIFETIME_BOUND;

  reverse_iterator rbegin() ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return reverse_iterator(end());
  }
  const_reverse_iterator rbegin() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return const_reverse_iterator(end());
  }
  reverse_iterator rend() ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return reverse_iterator(begin());
  }
  const_reverse_iterator rend() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return const_reverse_iterator(begin());
  }

  pointer_iterator pointer_begin() ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const_pointer_iterator pointer_begin() const ABSL_ATTRIBUTE_LIFETIME_BOUND;
  pointer_iterator pointer_end() ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const_pointer_iterator pointer_end() const ABSL_ATTRIBUTE_LIFETIME_BOUND;

  // Returns (an estimate of) the number of bytes used by the repeated field,
  // excluding sizeof(*this).
  size_t SpaceUsedExcludingSelfLong() const;

  int SpaceUsedExcludingSelf() const {
    return internal::ToIntSize(SpaceUsedExcludingSelfLong());
  }

  // Advanced memory management --------------------------------------
  // When hardcore memory management becomes necessary -- as it sometimes
  // does here at Google -- the following methods may be useful.

  // Adds an already-allocated object, passing ownership to the
  // RepeatedPtrField.
  //
  // Note that some special behavior occurs with respect to arenas:
  //
  //   (i) if this field holds submessages, the new submessage will be copied if
  //   the original is in an arena and this RepeatedPtrField is either in a
  //   different arena, or on the heap.
  //   (ii) if this field holds strings, the passed-in string *must* be
  //   heap-allocated, not arena-allocated. There is no way to dynamically check
  //   this at runtime, so User Beware.
  void AddAllocated(Element* value);

  // Removes and returns the last element, passing ownership to the caller.
  // Requires:  size() > 0
  //
  // If this RepeatedPtrField is on an arena, an object copy is required to pass
  // ownership back to the user (for compatible semantics). Use
  // UnsafeArenaReleaseLast() if this behavior is undesired.
  PROTOBUF_NODISCARD Element* ReleaseLast();

  // Adds an already-allocated object, skipping arena-ownership checks. The user
  // must guarantee that the given object is in the same arena as this
  // RepeatedPtrField.
  // It is also useful in legacy code that uses temporary ownership to avoid
  // copies. Example:
  //   RepeatedPtrField<T> temp_field;
  //   temp_field.UnsafeArenaAddAllocated(new T);
  //   ... // Do something with temp_field
  //   temp_field.UnsafeArenaExtractSubrange(0, temp_field.size(), nullptr);
  // If you put temp_field on the arena this fails, because the ownership
  // transfers to the arena at the "AddAllocated" call and is not released
  // anymore, causing a double delete. UnsafeArenaAddAllocated prevents this.
  void UnsafeArenaAddAllocated(Element* value);

  // Removes and returns the last element.  Unlike ReleaseLast, the returned
  // pointer is always to the original object.  This may be in an arena, in
  // which case it would have the arena's lifetime.
  // Requires: current_size_ > 0
  pointer UnsafeArenaReleaseLast();

  // Extracts elements with indices in the range "[start .. start+num-1]".
  // The caller assumes ownership of the extracted elements and is responsible
  // for deleting them when they are no longer needed.
  // If "elements" is non-nullptr, then pointers to the extracted elements
  // are stored in "elements[0 .. num-1]" for the convenience of the caller.
  // If "elements" is nullptr, then the caller must use some other mechanism
  // to perform any further operations (like deletion) on these elements.
  // Caution: implementation also moves elements with indices [start+num ..].
  // Calling this routine inside a loop can cause quadratic behavior.
  //
  // Memory copying behavior is identical to ReleaseLast(), described above: if
  // this RepeatedPtrField is on an arena, an object copy is performed for each
  // returned element, so that all returned element pointers are to
  // heap-allocated copies. If this copy is not desired, the user should call
  // UnsafeArenaExtractSubrange().
  void ExtractSubrange(int start, int num, Element** elements);

  // Identical to ExtractSubrange() described above, except that no object
  // copies are ever performed. Instead, the raw object pointers are returned.
  // Thus, if on an arena, the returned objects must not be freed, because they
  // will not be heap-allocated objects.
  void UnsafeArenaExtractSubrange(int start, int num, Element** elements);

  // When elements are removed by calls to RemoveLast() or Clear(), they
  // are not actually freed.  Instead, they are cleared and kept so that
  // they can be reused later.  This can save lots of CPU time when
  // repeatedly reusing a protocol message for similar purposes.
  //
  // Hardcore programs may choose to manipulate these cleared objects
  // to better optimize memory management using the following routines.

  // Gets the number of cleared objects that are currently being kept
  // around for reuse.
  ABSL_DEPRECATED("This will be removed in a future release")
  int ClearedCount() const;

#ifndef PROTOBUF_FUTURE_REMOVE_CLEARED_API
  // Adds an element to the pool of cleared objects, passing ownership to
  // the RepeatedPtrField.  The element must be cleared prior to calling
  // this method.
  //
  // This method cannot be called when either the repeated field or |value| is
  // on an arena; both cases will trigger a ABSL_DCHECK-failure.
  ABSL_DEPRECATED("This will be removed in a future release")
  void AddCleared(Element* value);
  // Removes and returns a single element from the cleared pool, passing
  // ownership to the caller.  The element is guaranteed to be cleared.
  // Requires:  ClearedCount() > 0
  //
  // This method cannot be called when the repeated field is on an arena; doing
  // so will trigger a ABSL_DCHECK-failure.
  PROTOBUF_NODISCARD
  ABSL_DEPRECATED("This will be removed in a future release")
  pointer ReleaseCleared();
#endif  // !PROTOBUF_FUTURE_REMOVE_CLEARED_API

  // Removes the element referenced by position.
  //
  // Returns an iterator to the element immediately following the removed
  // element.
  //
  // Invalidates all iterators at or after the removed element, including end().
  iterator erase(const_iterator position) ABSL_ATTRIBUTE_LIFETIME_BOUND;

  // Removes the elements in the range [first, last).
  //
  // Returns an iterator to the element immediately following the removed range.
  //
  // Invalidates all iterators at or after the removed range, including end().
  iterator erase(const_iterator first,
                 const_iterator last) ABSL_ATTRIBUTE_LIFETIME_BOUND;

  // Gets the arena on which this RepeatedPtrField stores its elements.
  inline Arena* GetArena();

#ifndef PROTOBUF_FUTURE_REMOVE_CONST_REPEATEDFIELD_GETARENA_API
  ABSL_DEPRECATED("This will be removed in a future release")
  inline Arena* GetArena() const;
#endif  // !PROTOBUF_FUTURE_REMOVE_CONST_REPEATEDFIELD_GETARENA_API

  // For internal use only.
  //
  // This is public due to it being called by generated code.
  void InternalSwap(RepeatedPtrField* PROTOBUF_RESTRICT other) {
    internal::RepeatedPtrFieldBase::InternalSwap(other);
  }


 private:
  using InternalArenaConstructable_ = void;
  using DestructorSkippable_ = void;

  friend class Arena;

  friend class internal::TcParser;

  template <typename T>
  friend struct WeakRepeatedPtrField;

  // Note:  RepeatedPtrField SHOULD NOT be subclassed by users.
  class TypeHandler;

  RepeatedPtrField(Arena* arena, const RepeatedPtrField& rhs);

  // Internal version of GetArena().
  inline Arena* GetOwningArena() const;


  // Implementations for ExtractSubrange(). The copying behavior must be
  // included only if the type supports the necessary operations (e.g.,
  // MergeFrom()), so we must resolve this at compile time. ExtractSubrange()
  // uses SFINAE to choose one of the below implementations.
  void ExtractSubrangeInternal(int start, int num, Element** elements,
                               std::true_type);
  void ExtractSubrangeInternal(int start, int num, Element** elements,
                               std::false_type);

  void AddAllocatedForParse(Element* p) {
    return RepeatedPtrFieldBase::AddAllocatedForParse<TypeHandler>(p);
  }
};

// -------------------------------------------------------------------

template <typename Element>
class RepeatedPtrField<Element>::TypeHandler
    : public internal::GenericTypeHandler<Element> {};

template <>
class RepeatedPtrField<std::string>::TypeHandler
    : public internal::StringTypeHandler {};

template <typename Element>
constexpr RepeatedPtrField<Element>::RepeatedPtrField()
    : RepeatedPtrFieldBase() {
  StaticValidityCheck();
}

template <typename Element>
inline RepeatedPtrField<Element>::RepeatedPtrField(Arena* arena)
    : RepeatedPtrFieldBase(arena) {
  // We can't have StaticValidityCheck here because that requires Element to be
  // a complete type, and in split repeated fields cases, we call
  // CreateMaybeMessage<RepeatedPtrField<T>> for incomplete Ts.
}

template <typename Element>
inline RepeatedPtrField<Element>::RepeatedPtrField(Arena* arena,
                                                   const RepeatedPtrField& rhs)
    : RepeatedPtrFieldBase(arena) {
  StaticValidityCheck();
  MergeFrom(rhs);
}

template <typename Element>
template <typename Iter, typename>
inline RepeatedPtrField<Element>::RepeatedPtrField(Iter begin, Iter end) {
  StaticValidityCheck();
  Add(begin, end);
}

template <typename Element>
RepeatedPtrField<Element>::~RepeatedPtrField() {
  StaticValidityCheck();
#ifdef __cpp_if_constexpr
  if constexpr (std::is_base_of<MessageLite, Element>::value) {
#else
  if (std::is_base_of<MessageLite, Element>::value) {
#endif
    if (NeedsDestroy()) DestroyProtos();
  } else {
    Destroy<TypeHandler>();
  }
}

template <typename Element>
inline RepeatedPtrField<Element>& RepeatedPtrField<Element>::operator=(
    const RepeatedPtrField& other) ABSL_ATTRIBUTE_LIFETIME_BOUND {
  if (this != &other) CopyFrom(other);
  return *this;
}

template <typename Element>
inline RepeatedPtrField<Element>::RepeatedPtrField(
    RepeatedPtrField&& other) noexcept
    : RepeatedPtrField() {
#ifdef PROTOBUF_FORCE_COPY_IN_MOVE
  CopyFrom(other);
#else   // PROTOBUF_FORCE_COPY_IN_MOVE
  // We don't just call Swap(&other) here because it would perform 3 copies if
  // other is on an arena. This field can't be on an arena because arena
  // construction always uses the Arena* accepting constructor.
  if (other.GetArena()) {
    CopyFrom(other);
  } else {
    InternalSwap(&other);
  }
#endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
}

template <typename Element>
inline RepeatedPtrField<Element>& RepeatedPtrField<Element>::operator=(
    RepeatedPtrField&& other) noexcept ABSL_ATTRIBUTE_LIFETIME_BOUND {
  // We don't just call Swap(&other) here because it would perform 3 copies if
  // the two fields are on different arenas.
  if (this != &other) {
    if (GetArena() != other.GetArena()
#ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        || GetArena() == nullptr
#endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      CopyFrom(other);
    } else {
      InternalSwap(&other);
    }
  }
  return *this;
}

template <typename Element>
inline bool RepeatedPtrField<Element>::empty() const {
  return RepeatedPtrFieldBase::empty();
}

template <typename Element>
inline int RepeatedPtrField<Element>::size() const {
  return RepeatedPtrFieldBase::size();
}

template <typename Element>
inline const Element& RepeatedPtrField<Element>::Get(int index) const
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return RepeatedPtrFieldBase::Get<TypeHandler>(index);
}

template <typename Element>
inline const Element& RepeatedPtrField<Element>::at(int index) const
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return RepeatedPtrFieldBase::at<TypeHandler>(index);
}

template <typename Element>
inline Element& RepeatedPtrField<Element>::at(int index)
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return RepeatedPtrFieldBase::at<TypeHandler>(index);
}


template <typename Element>
inline Element* RepeatedPtrField<Element>::Mutable(int index)
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return RepeatedPtrFieldBase::Mutable<TypeHandler>(index);
}

template <typename Element>
PROTOBUF_NOINLINE Element* RepeatedPtrField<Element>::Add()
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return RepeatedPtrFieldBase::Add<TypeHandler>();
}

template <typename Element>
inline void RepeatedPtrField<Element>::Add(Element&& value) {
  RepeatedPtrFieldBase::Add<TypeHandler>(std::move(value));
}

template <typename Element>
template <typename Iter>
inline void RepeatedPtrField<Element>::Add(Iter begin, Iter end) {
  if (std::is_base_of<
          std::forward_iterator_tag,
          typename std::iterator_traits<Iter>::iterator_category>::value) {
    int reserve = static_cast<int>(std::distance(begin, end));
    Reserve(size() + reserve);
  }
  for (; begin != end; ++begin) {
    *Add() = *begin;
  }
}

template <typename Element>
inline void RepeatedPtrField<Element>::RemoveLast() {
  RepeatedPtrFieldBase::RemoveLast<TypeHandler>();
}

template <typename Element>
inline void RepeatedPtrField<Element>::DeleteSubrange(int start, int num) {
  ABSL_DCHECK_GE(start, 0);
  ABSL_DCHECK_GE(num, 0);
  ABSL_DCHECK_LE(start + num, size());
  void** subrange = raw_mutable_data() + start;
  Arena* arena = GetArena();
  for (int i = 0; i < num; ++i) {
    using H = CommonHandler<TypeHandler>;
    H::Delete(static_cast<Element*>(subrange[i]), arena);
  }
  UnsafeArenaExtractSubrange(start, num, nullptr);
}

template <typename Element>
inline void RepeatedPtrField<Element>::ExtractSubrange(int start, int num,
                                                       Element** elements) {
  typename internal::TypeImplementsMergeBehavior<
      typename TypeHandler::Type>::type t;
  ExtractSubrangeInternal(start, num, elements, t);
}

// ExtractSubrange() implementation for types that implement merge/copy
// behavior.
template <typename Element>
inline void RepeatedPtrField<Element>::ExtractSubrangeInternal(
    int start, int num, Element** elements, std::true_type) {
  ABSL_DCHECK_GE(start, 0);
  ABSL_DCHECK_GE(num, 0);
  ABSL_DCHECK_LE(start + num, size());

  if (num == 0) return;

  ABSL_DCHECK_NE(elements, nullptr)
      << "Releasing elements without transferring ownership is an unsafe "
         "operation.  Use UnsafeArenaExtractSubrange.";
  if (elements != nullptr) {
    Arena* arena = GetArena();
    auto* extracted = data() + start;
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
    // Always copy.
    for (int i = 0; i < num; ++i) {
      elements[i] = copy<TypeHandler>(extracted[i]);
    }
    if (arena == nullptr) {
      for (int i = 0; i < num; ++i) {
        delete extracted[i];
      }
    }
#else   // PROTOBUF_FORCE_COPY_IN_RELEASE
    // If we're on an arena, we perform a copy for each element so that the
    // returned elements are heap-allocated. Otherwise, just forward it.
    if (arena != nullptr) {
      for (int i = 0; i < num; ++i) {
        elements[i] = copy<TypeHandler>(extracted[i]);
      }
    } else {
      memcpy(elements, extracted, num * sizeof(Element*));
    }
#endif  // !PROTOBUF_FORCE_COPY_IN_RELEASE
  }
  CloseGap(start, num);
}

// ExtractSubrange() implementation for types that do not implement merge/copy
// behavior.
template <typename Element>
inline void RepeatedPtrField<Element>::ExtractSubrangeInternal(
    int start, int num, Element** elements, std::false_type) {
  // This case is identical to UnsafeArenaExtractSubrange(). However, since
  // ExtractSubrange() must return heap-allocated objects by contract, and we
  // cannot fulfill this contract if we are an on arena, we must ABSL_DCHECK()
  // that we are not on an arena.
  ABSL_DCHECK(GetArena() == nullptr)
      << "ExtractSubrange() when arena is non-nullptr is only supported when "
      << "the Element type supplies a MergeFrom() operation to make copies.";
  UnsafeArenaExtractSubrange(start, num, elements);
}

template <typename Element>
inline void RepeatedPtrField<Element>::UnsafeArenaExtractSubrange(
    int start, int num, Element** elements) {
  ABSL_DCHECK_GE(start, 0);
  ABSL_DCHECK_GE(num, 0);
  ABSL_DCHECK_LE(start + num, size());

  if (num > 0) {
    // Save the values of the removed elements if requested.
    if (elements != nullptr) {
      memcpy(elements, data() + start, num * sizeof(Element*));
    }
    CloseGap(start, num);
  }
}

template <typename Element>
inline void RepeatedPtrField<Element>::Clear() {
  RepeatedPtrFieldBase::Clear<TypeHandler>();
}

template <typename Element>
inline void RepeatedPtrField<Element>::MergeFrom(
    const RepeatedPtrField& other) {
  if (other.empty()) return;
  RepeatedPtrFieldBase::MergeFrom<Element>(other);
}

template <typename Element>
inline void RepeatedPtrField<Element>::CopyFrom(const RepeatedPtrField& other) {
  RepeatedPtrFieldBase::CopyFrom<TypeHandler>(other);
}

template <typename Element>
template <typename Iter>
inline void RepeatedPtrField<Element>::Assign(Iter begin, Iter end) {
  Clear();
  Add(begin, end);
}

template <typename Element>
inline typename RepeatedPtrField<Element>::iterator
RepeatedPtrField<Element>::erase(const_iterator position)
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return erase(position, position + 1);
}

template <typename Element>
inline typename RepeatedPtrField<Element>::iterator
RepeatedPtrField<Element>::erase(const_iterator first, const_iterator last)
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  size_type pos_offset = static_cast<size_type>(std::distance(cbegin(), first));
  size_type last_offset = static_cast<size_type>(std::distance(cbegin(), last));
  DeleteSubrange(pos_offset, last_offset - pos_offset);
  return begin() + pos_offset;
}

template <typename Element>
inline Element** RepeatedPtrField<Element>::mutable_data()
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return RepeatedPtrFieldBase::mutable_data<TypeHandler>();
}

template <typename Element>
inline const Element* const* RepeatedPtrField<Element>::data() const
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return RepeatedPtrFieldBase::data<TypeHandler>();
}

template <typename Element>
inline void RepeatedPtrField<Element>::Swap(RepeatedPtrField* other) {
  if (this == other) return;
  RepeatedPtrFieldBase::Swap<TypeHandler>(other);
}

template <typename Element>
inline void RepeatedPtrField<Element>::UnsafeArenaSwap(
    RepeatedPtrField* other) {
  if (this == other) return;
  ABSL_DCHECK_EQ(GetArena(), other->GetArena());
  RepeatedPtrFieldBase::InternalSwap(other);
}

template <typename Element>
inline void RepeatedPtrField<Element>::SwapElements(int index1, int index2) {
  RepeatedPtrFieldBase::SwapElements(index1, index2);
}

template <typename Element>
inline Arena* RepeatedPtrField<Element>::GetArena() {
  return RepeatedPtrFieldBase::GetArena();
}

#ifndef PROTOBUF_FUTURE_REMOVE_CONST_REPEATEDFIELD_GETARENA_API
template <typename Element>
inline Arena* RepeatedPtrField<Element>::GetArena() const {
  return RepeatedPtrFieldBase::GetArena();
}
#endif  // !PROTOBUF_FUTURE_REMOVE_CONST_REPEATEDFIELD_GETARENA_API

template <typename Element>
inline Arena* RepeatedPtrField<Element>::GetOwningArena() const {
  return RepeatedPtrFieldBase::GetArena();
}

template <typename Element>
inline size_t RepeatedPtrField<Element>::SpaceUsedExcludingSelfLong() const {
  // `google::protobuf::Message` has a virtual method `SpaceUsedLong`, hence we can
  // instantiate just one function for all protobuf messages.
  // Note: std::is_base_of requires that `Element` is a concrete class.
  using H = typename std::conditional<std::is_base_of<Message, Element>::value,
                                      internal::GenericTypeHandler<Message>,
                                      TypeHandler>::type;
  return RepeatedPtrFieldBase::SpaceUsedExcludingSelfLong<H>();
}

template <typename Element>
inline void RepeatedPtrField<Element>::AddAllocated(Element* value) {
  RepeatedPtrFieldBase::AddAllocated<TypeHandler>(value);
}

template <typename Element>
inline void RepeatedPtrField<Element>::UnsafeArenaAddAllocated(Element* value) {
  RepeatedPtrFieldBase::UnsafeArenaAddAllocated<TypeHandler>(value);
}

template <typename Element>
inline Element* RepeatedPtrField<Element>::ReleaseLast() {
  return RepeatedPtrFieldBase::ReleaseLast<TypeHandler>();
}

template <typename Element>
inline Element* RepeatedPtrField<Element>::UnsafeArenaReleaseLast() {
  return RepeatedPtrFieldBase::UnsafeArenaReleaseLast<TypeHandler>();
}

template <typename Element>
inline int RepeatedPtrField<Element>::ClearedCount() const {
  return RepeatedPtrFieldBase::ClearedCount();
}

#ifndef PROTOBUF_FUTURE_REMOVE_CLEARED_API
template <typename Element>
inline void RepeatedPtrField<Element>::AddCleared(Element* value) {
  return RepeatedPtrFieldBase::AddCleared<TypeHandler>(value);
}

template <typename Element>
inline Element* RepeatedPtrField<Element>::ReleaseCleared() {
  return RepeatedPtrFieldBase::ReleaseCleared<TypeHandler>();
}
#endif  // !PROTOBUF_FUTURE_REMOVE_CLEARED_API

template <typename Element>
inline void RepeatedPtrField<Element>::Reserve(int new_size) {
  return RepeatedPtrFieldBase::Reserve(new_size);
}

template <typename Element>
inline int RepeatedPtrField<Element>::Capacity() const {
  return RepeatedPtrFieldBase::Capacity();
}

// -------------------------------------------------------------------

namespace internal {

// STL-like iterator implementation for RepeatedPtrField.  You should not
// refer to this class directly; use RepeatedPtrField<T>::iterator instead.
//
// The iterator for RepeatedPtrField<T>, RepeatedPtrIterator<T>, is
// very similar to iterator_ptr<T**> in util/gtl/iterator_adaptors.h,
// but adds random-access operators and is modified to wrap a void** base
// iterator (since RepeatedPtrField stores its array as a void* array and
// casting void** to T** would violate C++ aliasing rules).
//
// This code based on net/proto/proto-array-internal.h by Jeffrey Yasskin
// (jyasskin@google.com).
template <typename Element>
class RepeatedPtrIterator {
 public:
  using iterator = RepeatedPtrIterator<Element>;
  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename std::remove_const<Element>::type;
  using difference_type = std::ptrdiff_t;
  using pointer = Element*;
  using reference = Element&;

  RepeatedPtrIterator() : it_(nullptr) {}
  explicit RepeatedPtrIterator(void* const* it) : it_(it) {}

  // Allows "upcasting" from RepeatedPtrIterator<T**> to
  // RepeatedPtrIterator<const T*const*>.
  template <typename OtherElement,
            typename std::enable_if<std::is_convertible<
                OtherElement*, pointer>::value>::type* = nullptr>
  RepeatedPtrIterator(const RepeatedPtrIterator<OtherElement>& other)
      : it_(other.it_) {}

  // dereferenceable
  reference operator*() const { return *reinterpret_cast<Element*>(*it_); }
  pointer operator->() const { return &(operator*()); }

  // {inc,dec}rementable
  iterator& operator++() {
    ++it_;
    return *this;
  }
  iterator operator++(int) { return iterator(it_++); }
  iterator& operator--() {
    --it_;
    return *this;
  }
  iterator operator--(int) { return iterator(it_--); }

  // equality_comparable
  friend bool operator==(const iterator& x, const iterator& y) {
    return x.it_ == y.it_;
  }
  friend bool operator!=(const iterator& x, const iterator& y) {
    return x.it_ != y.it_;
  }

  // less_than_comparable
  friend bool operator<(const iterator& x, const iterator& y) {
    return x.it_ < y.it_;
  }
  friend bool operator<=(const iterator& x, const iterator& y) {
    return x.it_ <= y.it_;
  }
  friend bool operator>(const iterator& x, const iterator& y) {
    return x.it_ > y.it_;
  }
  friend bool operator>=(const iterator& x, const iterator& y) {
    return x.it_ >= y.it_;
  }

  // addable, subtractable
  iterator& operator+=(difference_type d) {
    it_ += d;
    return *this;
  }
  friend iterator operator+(iterator it, const difference_type d) {
    it += d;
    return it;
  }
  friend iterator operator+(const difference_type d, iterator it) {
    it += d;
    return it;
  }
  iterator& operator-=(difference_type d) {
    it_ -= d;
    return *this;
  }
  friend iterator operator-(iterator it, difference_type d) {
    it -= d;
    return it;
  }

  // indexable
  reference operator[](difference_type d) const { return *(*this + d); }

  // random access iterator
  friend difference_type operator-(iterator it1, iterator it2) {
    return it1.it_ - it2.it_;
  }

 private:
  template <typename OtherElement>
  friend class RepeatedPtrIterator;

  // The internal iterator.
  void* const* it_;
};

// Provides an iterator that operates on pointers to the underlying objects
// rather than the objects themselves as RepeatedPtrIterator does.
// Consider using this when working with stl algorithms that change
// the array.
// The VoidPtr template parameter holds the type-agnostic pointer value
// referenced by the iterator.  It should either be "void *" for a mutable
// iterator, or "const void* const" for a constant iterator.
template <typename Element, typename VoidPtr>
class RepeatedPtrOverPtrsIterator {
 public:
  using iterator = RepeatedPtrOverPtrsIterator<Element, VoidPtr>;
  using iterator_category = std::random_access_iterator_tag;
  using value_type = typename std::remove_const<Element>::type;
  using difference_type = std::ptrdiff_t;
  using pointer = Element*;
  using reference = Element&;

  RepeatedPtrOverPtrsIterator() : it_(nullptr) {}
  explicit RepeatedPtrOverPtrsIterator(VoidPtr* it) : it_(it) {}

  // Allows "upcasting" from RepeatedPtrOverPtrsIterator<T**> to
  // RepeatedPtrOverPtrsIterator<const T*const*>.
  template <
      typename OtherElement, typename OtherVoidPtr,
      typename std::enable_if<
          std::is_convertible<OtherElement*, pointer>::value &&
          std::is_convertible<OtherVoidPtr*, VoidPtr>::value>::type* = nullptr>
  RepeatedPtrOverPtrsIterator(
      const RepeatedPtrOverPtrsIterator<OtherElement, OtherVoidPtr>& other)
      : it_(other.it_) {}

  // dereferenceable
  reference operator*() const { return *reinterpret_cast<Element*>(it_); }
  pointer operator->() const { return &(operator*()); }

  // {inc,dec}rementable
  iterator& operator++() {
    ++it_;
    return *this;
  }
  iterator operator++(int) { return iterator(it_++); }
  iterator& operator--() {
    --it_;
    return *this;
  }
  iterator operator--(int) { return iterator(it_--); }

  // equality_comparable
  friend bool operator==(const iterator& x, const iterator& y) {
    return x.it_ == y.it_;
  }
  friend bool operator!=(const iterator& x, const iterator& y) {
    return x.it_ != y.it_;
  }

  // less_than_comparable
  friend bool operator<(const iterator& x, const iterator& y) {
    return x.it_ < y.it_;
  }
  friend bool operator<=(const iterator& x, const iterator& y) {
    return x.it_ <= y.it_;
  }
  friend bool operator>(const iterator& x, const iterator& y) {
    return x.it_ > y.it_;
  }
  friend bool operator>=(const iterator& x, const iterator& y) {
    return x.it_ >= y.it_;
  }

  // addable, subtractable
  iterator& operator+=(difference_type d) {
    it_ += d;
    return *this;
  }
  friend iterator operator+(iterator it, difference_type d) {
    it += d;
    return it;
  }
  friend iterator operator+(difference_type d, iterator it) {
    it += d;
    return it;
  }
  iterator& operator-=(difference_type d) {
    it_ -= d;
    return *this;
  }
  friend iterator operator-(iterator it, difference_type d) {
    it -= d;
    return it;
  }

  // indexable
  reference operator[](difference_type d) const { return *(*this + d); }

  // random access iterator
  friend difference_type operator-(iterator it1, iterator it2) {
    return it1.it_ - it2.it_;
  }

 private:
  template <typename OtherElement, typename OtherVoidPtr>
  friend class RepeatedPtrOverPtrsIterator;

  // The internal iterator.
  VoidPtr* it_;
};

}  // namespace internal

template <typename Element>
inline typename RepeatedPtrField<Element>::iterator
RepeatedPtrField<Element>::begin() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return iterator(raw_data());
}
template <typename Element>
inline typename RepeatedPtrField<Element>::const_iterator
RepeatedPtrField<Element>::begin() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return iterator(raw_data());
}
template <typename Element>
inline typename RepeatedPtrField<Element>::const_iterator
RepeatedPtrField<Element>::cbegin() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return begin();
}
template <typename Element>
inline typename RepeatedPtrField<Element>::iterator
RepeatedPtrField<Element>::end() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return iterator(raw_data() + size());
}
template <typename Element>
inline typename RepeatedPtrField<Element>::const_iterator
RepeatedPtrField<Element>::end() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return iterator(raw_data() + size());
}
template <typename Element>
inline typename RepeatedPtrField<Element>::const_iterator
RepeatedPtrField<Element>::cend() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return end();
}

template <typename Element>
inline typename RepeatedPtrField<Element>::pointer_iterator
RepeatedPtrField<Element>::pointer_begin() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return pointer_iterator(raw_mutable_data());
}
template <typename Element>
inline typename RepeatedPtrField<Element>::const_pointer_iterator
RepeatedPtrField<Element>::pointer_begin() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return const_pointer_iterator(const_cast<const void* const*>(raw_data()));
}
template <typename Element>
inline typename RepeatedPtrField<Element>::pointer_iterator
RepeatedPtrField<Element>::pointer_end() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return pointer_iterator(raw_mutable_data() + size());
}
template <typename Element>
inline typename RepeatedPtrField<Element>::const_pointer_iterator
RepeatedPtrField<Element>::pointer_end() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
  return const_pointer_iterator(
      const_cast<const void* const*>(raw_data() + size()));
}

// Iterators and helper functions that follow the spirit of the STL
// std::back_insert_iterator and std::back_inserter but are tailor-made
// for RepeatedField and RepeatedPtrField. Typical usage would be:
//
//   std::copy(some_sequence.begin(), some_sequence.end(),
//             RepeatedFieldBackInserter(proto.mutable_sequence()));
//
// Ported by johannes from util/gtl/proto-array-iterators.h

namespace internal {

// A back inserter for RepeatedPtrField objects.
template <typename T>
class RepeatedPtrFieldBackInsertIterator {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = T;
  using pointer = void;
  using reference = void;
  using difference_type = std::ptrdiff_t;

  RepeatedPtrFieldBackInsertIterator(RepeatedPtrField<T>* const mutable_field)
      : field_(mutable_field) {}
  RepeatedPtrFieldBackInsertIterator<T>& operator=(const T& value) {
    *field_->Add() = value;
    return *this;
  }
  RepeatedPtrFieldBackInsertIterator<T>& operator=(
      const T* const ptr_to_value) {
    *field_->Add() = *ptr_to_value;
    return *this;
  }
  RepeatedPtrFieldBackInsertIterator<T>& operator=(T&& value) {
    *field_->Add() = std::move(value);
    return *this;
  }
  RepeatedPtrFieldBackInsertIterator<T>& operator*() { return *this; }
  RepeatedPtrFieldBackInsertIterator<T>& operator++() { return *this; }
  RepeatedPtrFieldBackInsertIterator<T>& operator++(int /* unused */) {
    return *this;
  }

 private:
  RepeatedPtrField<T>* field_;
};

// A back inserter for RepeatedPtrFields that inserts by transferring ownership
// of a pointer.
template <typename T>
class AllocatedRepeatedPtrFieldBackInsertIterator {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = T;
  using pointer = void;
  using reference = void;
  using difference_type = std::ptrdiff_t;

  explicit AllocatedRepeatedPtrFieldBackInsertIterator(
      RepeatedPtrField<T>* const mutable_field)
      : field_(mutable_field) {}
  AllocatedRepeatedPtrFieldBackInsertIterator<T>& operator=(
      T* const ptr_to_value) {
    field_->AddAllocated(ptr_to_value);
    return *this;
  }
  AllocatedRepeatedPtrFieldBackInsertIterator<T>& operator*() { return *this; }
  AllocatedRepeatedPtrFieldBackInsertIterator<T>& operator++() { return *this; }
  AllocatedRepeatedPtrFieldBackInsertIterator<T>& operator++(int /* unused */) {
    return *this;
  }

 private:
  RepeatedPtrField<T>* field_;
};

// Almost identical to AllocatedRepeatedPtrFieldBackInsertIterator. This one
// uses the UnsafeArenaAddAllocated instead.
template <typename T>
class UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = T;
  using pointer = void;
  using reference = void;
  using difference_type = std::ptrdiff_t;

  explicit UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator(
      RepeatedPtrField<T>* const mutable_field)
      : field_(mutable_field) {}
  UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator<T>& operator=(
      T const* const ptr_to_value) {
    field_->UnsafeArenaAddAllocated(const_cast<T*>(ptr_to_value));
    return *this;
  }
  UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator<T>& operator*() {
    return *this;
  }
  UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator<T>& operator++() {
    return *this;
  }
  UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator<T>& operator++(
      int /* unused */) {
    return *this;
  }

 private:
  RepeatedPtrField<T>* field_;
};

}  // namespace internal

// Provides a back insert iterator for RepeatedPtrField instances,
// similar to std::back_inserter().
template <typename T>
internal::RepeatedPtrFieldBackInsertIterator<T> RepeatedPtrFieldBackInserter(
    RepeatedPtrField<T>* const mutable_field) {
  return internal::RepeatedPtrFieldBackInsertIterator<T>(mutable_field);
}

// Special back insert iterator for RepeatedPtrField instances, just in
// case someone wants to write generic template code that can access both
// RepeatedFields and RepeatedPtrFields using a common name.
template <typename T>
internal::RepeatedPtrFieldBackInsertIterator<T> RepeatedFieldBackInserter(
    RepeatedPtrField<T>* const mutable_field) {
  return internal::RepeatedPtrFieldBackInsertIterator<T>(mutable_field);
}

// Provides a back insert iterator for RepeatedPtrField instances
// similar to std::back_inserter() which transfers the ownership while
// copying elements.
template <typename T>
internal::AllocatedRepeatedPtrFieldBackInsertIterator<T>
AllocatedRepeatedPtrFieldBackInserter(
    RepeatedPtrField<T>* const mutable_field) {
  return internal::AllocatedRepeatedPtrFieldBackInsertIterator<T>(
      mutable_field);
}

// Similar to AllocatedRepeatedPtrFieldBackInserter, using
// UnsafeArenaAddAllocated instead of AddAllocated.
// This is slightly faster if that matters. It is also useful in legacy code
// that uses temporary ownership to avoid copies. Example:
//   RepeatedPtrField<T> temp_field;
//   temp_field.UnsafeArenaAddAllocated(new T);
//   ... // Do something with temp_field
//   temp_field.UnsafeArenaExtractSubrange(0, temp_field.size(), nullptr);
// Putting temp_field on the arena fails because the ownership transfers to the
// arena at the "AddAllocated" call and is not released anymore causing a
// double delete. This function uses UnsafeArenaAddAllocated to prevent this.
template <typename T>
internal::UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator<T>
UnsafeArenaAllocatedRepeatedPtrFieldBackInserter(
    RepeatedPtrField<T>* const mutable_field) {
  return internal::UnsafeArenaAllocatedRepeatedPtrFieldBackInsertIterator<T>(
      mutable_field);
}


namespace internal {
// Size optimization for `memswap<N>` - supplied below N is used by every
// `RepeatedPtrField<T>`.
extern template PROTOBUF_EXPORT_TEMPLATE_DECLARE void
memswap<ArenaOffsetHelper<RepeatedPtrFieldBase>::value>(
    char* PROTOBUF_RESTRICT, char* PROTOBUF_RESTRICT);
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


#endif  // GOOGLE_PROTOBUF_REPEATED_PTR_FIELD_H__

