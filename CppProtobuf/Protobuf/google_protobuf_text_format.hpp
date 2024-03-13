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
// Utilities for printing and parsing protocol messages in a human-readable,
// text-based format.

#ifndef GOOGLE_PROTOBUF_TEXT_FORMAT_H__
#define GOOGLE_PROTOBUF_TEXT_FORMAT_H__

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <CppAbseil/absl_container_flat_hash_map.hpp>
#include <CppAbseil/absl_strings_cord.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include <CppProtobuf/google_protobuf_descriptor.hpp>
#include <CppProtobuf/google_protobuf_message.hpp>
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

namespace internal {
PROTOBUF_EXPORT extern const char kDebugStringSilentMarker[1];
PROTOBUF_EXPORT extern const char kDebugStringSilentMarkerForDetection[3];

PROTOBUF_EXPORT extern std::atomic<bool> enable_debug_text_format_marker;
PROTOBUF_EXPORT extern std::atomic<bool> enable_debug_text_detection;
PROTOBUF_EXPORT extern std::atomic<bool> enable_debug_text_redaction;
PROTOBUF_EXPORT int64_t GetRedactedFieldCount();
PROTOBUF_EXPORT bool ShouldRedactField(const FieldDescriptor* field);

// This enum contains all the APIs that convert protos to human-readable
// formats. A higher-level API must correspond to a greater number than any
// lower-level APIs it calls under the hood (e.g kDebugString >
// kMemberPrintToString > kPrintWithStream).
PROTOBUF_EXPORT enum class FieldReporterLevel {
  kNoReport = 0,
  kPrintMessage = 1,
  kPrintWithGenerator = 2,
  kPrintWithStream = 3,
  kMemberPrintToString = 4,
  kStaticPrintToString = 5,
  kAbslStringify = 6,
  kShortFormat = 7,
  kUtf8Format = 8,
  kDebugString = 12,
  kShortDebugString = 13,
  kUtf8DebugString = 14
};

}  // namespace internal

namespace io {
class ErrorCollector;  // tokenizer.h
}

namespace internal {
// Enum used to set printing options for StringifyMessage.
PROTOBUF_EXPORT enum class Option;

// Converts a protobuf message to a string with redaction enabled.
PROTOBUF_EXPORT std::string StringifyMessage(const Message& message,
                                             Option option);
}  // namespace internal

// This class implements protocol buffer text format, colloquially known as text
// proto.  Printing and parsing protocol messages in text format is useful for
// debugging and human editing of messages.
//
// This class is really a namespace that contains only static methods.
class PROTOBUF_EXPORT TextFormat {
 public:
  TextFormat(const TextFormat&) = delete;
  TextFormat& operator=(const TextFormat&) = delete;

  // Outputs a textual representation of the given message to the given
  // output stream. Returns false if printing fails.
  static bool Print(const Message& message, io::ZeroCopyOutputStream* output);

  // Print the fields in an UnknownFieldSet.  They are printed by tag number
  // only.  Embedded messages are heuristically identified by attempting to
  // parse them. Returns false if printing fails.
  static bool PrintUnknownFields(const UnknownFieldSet& unknown_fields,
                                 io::ZeroCopyOutputStream* output);

  // Like Print(), but outputs directly to a string.
  // Note: output will be cleared prior to printing, and will be left empty
  // even if printing fails. Returns false if printing fails.
  static bool PrintToString(const Message& message, std::string* output);

  // Like PrintUnknownFields(), but outputs directly to a string. Returns
  // false if printing fails.
  static bool PrintUnknownFieldsToString(const UnknownFieldSet& unknown_fields,
                                         std::string* output);

  // Outputs a textual representation of the value of the field supplied on
  // the message supplied. For non-repeated fields, an index of -1 must
  // be supplied. Note that this method will print the default value for a
  // field if it is not set.
  static void PrintFieldValueToString(const Message& message,
                                      const FieldDescriptor* field, int index,
                                      std::string* output);

  // Forward declare `Printer` for `BaseTextGenerator::MarkerToken` which
  // restricts some methods of `BaseTextGenerator` to the class `Printer`.
  class Printer;

  class PROTOBUF_EXPORT BaseTextGenerator {
   private:
    // Passkey (go/totw/134#what-about-stdshared-ptr) that allows `Printer`
    // (but not derived classes) to call `PrintMaybeWithMarker` and its
    // `Printer::TextGenerator` to overload it.
    // This prevents users from bypassing the marker generation.
    class MarkerToken {
     private:
      explicit MarkerToken() = default;  // 'explicit' prevents aggregate init.
      friend class Printer;
    };

   public:
    virtual ~BaseTextGenerator();

    virtual void Indent() {}
    virtual void Outdent() {}
    // Returns the current indentation size in characters.
    virtual size_t GetCurrentIndentationSize() const { return 0; }

    // Print text to the output stream.
    virtual void Print(const char* text, size_t size) = 0;

    void PrintString(absl::string_view str) { Print(str.data(), str.size()); }

    template <size_t n>
    void PrintLiteral(const char (&text)[n]) {
      Print(text, n - 1);  // n includes the terminating zero character.
    }

    // Internal to Printer, access regulated by `MarkerToken`.
    virtual void PrintMaybeWithMarker(MarkerToken, absl::string_view text) {
      Print(text.data(), text.size());
    }

    // Internal to Printer, access regulated by `MarkerToken`.
    virtual void PrintMaybeWithMarker(MarkerToken, absl::string_view text_head,
                                      absl::string_view text_tail) {
      Print(text_head.data(), text_head.size());
      Print(text_tail.data(), text_tail.size());
    }

    friend class Printer;
  };

  // The default printer that converts scalar values from fields into their
  // string representation.
  // You can derive from this FastFieldValuePrinter if you want to have fields
  // to be printed in a different way and register it at the Printer.
  class PROTOBUF_EXPORT FastFieldValuePrinter {
   public:
    FastFieldValuePrinter();
    FastFieldValuePrinter(const FastFieldValuePrinter&) = delete;
    FastFieldValuePrinter& operator=(const FastFieldValuePrinter&) = delete;
    virtual ~FastFieldValuePrinter();
    virtual void PrintBool(bool val, BaseTextGenerator* generator) const;
    virtual void PrintInt32(int32_t val, BaseTextGenerator* generator) const;
    virtual void PrintUInt32(uint32_t val, BaseTextGenerator* generator) const;
    virtual void PrintInt64(int64_t val, BaseTextGenerator* generator) const;
    virtual void PrintUInt64(uint64_t val, BaseTextGenerator* generator) const;
    virtual void PrintFloat(float val, BaseTextGenerator* generator) const;
    virtual void PrintDouble(double val, BaseTextGenerator* generator) const;
    virtual void PrintString(const std::string& val,
                             BaseTextGenerator* generator) const;
    virtual void PrintBytes(const std::string& val,
                            BaseTextGenerator* generator) const;
    virtual void PrintEnum(int32_t val, const std::string& name,
                           BaseTextGenerator* generator) const;
    virtual void PrintFieldName(const Message& message, int field_index,
                                int field_count, const Reflection* reflection,
                                const FieldDescriptor* field,
                                BaseTextGenerator* generator) const;
    virtual void PrintFieldName(const Message& message,
                                const Reflection* reflection,
                                const FieldDescriptor* field,
                                BaseTextGenerator* generator) const;
    virtual void PrintMessageStart(const Message& message, int field_index,
                                   int field_count, bool single_line_mode,
                                   BaseTextGenerator* generator) const;
    // Allows to override the logic on how to print the content of a message.
    // Return false to use the default printing logic. Note that it is legal for
    // this function to print something and then return false to use the default
    // content printing (although at that point it would behave similarly to
    // PrintMessageStart).
    virtual bool PrintMessageContent(const Message& message, int field_index,
                                     int field_count, bool single_line_mode,
                                     BaseTextGenerator* generator) const;
    virtual void PrintMessageEnd(const Message& message, int field_index,
                                 int field_count, bool single_line_mode,
                                 BaseTextGenerator* generator) const;
  };

  // Deprecated: please use FastFieldValuePrinter instead.
  class PROTOBUF_EXPORT FieldValuePrinter {
   public:
    FieldValuePrinter();
    FieldValuePrinter(const FieldValuePrinter&) = delete;
    FieldValuePrinter& operator=(const FieldValuePrinter&) = delete;
    virtual ~FieldValuePrinter();
    virtual std::string PrintBool(bool val) const;
    virtual std::string PrintInt32(int32_t val) const;
    virtual std::string PrintUInt32(uint32_t val) const;
    virtual std::string PrintInt64(int64_t val) const;
    virtual std::string PrintUInt64(uint64_t val) const;
    virtual std::string PrintFloat(float val) const;
    virtual std::string PrintDouble(double val) const;
    virtual std::string PrintString(const std::string& val) const;
    virtual std::string PrintBytes(const std::string& val) const;
    virtual std::string PrintEnum(int32_t val, const std::string& name) const;
    virtual std::string PrintFieldName(const Message& message,
                                       const Reflection* reflection,
                                       const FieldDescriptor* field) const;
    virtual std::string PrintMessageStart(const Message& message,
                                          int field_index, int field_count,
                                          bool single_line_mode) const;
    virtual std::string PrintMessageEnd(const Message& message, int field_index,
                                        int field_count,
                                        bool single_line_mode) const;

   private:
    FastFieldValuePrinter delegate_;
  };

  class PROTOBUF_EXPORT MessagePrinter {
   public:
    MessagePrinter() {}
    MessagePrinter(const MessagePrinter&) = delete;
    MessagePrinter& operator=(const MessagePrinter&) = delete;
    virtual ~MessagePrinter() {}
    virtual void Print(const Message& message, bool single_line_mode,
                       BaseTextGenerator* generator) const = 0;
  };

  // Interface that Printers or Parsers can use to find extensions, or types
  // referenced in Any messages.
  class PROTOBUF_EXPORT Finder {
   public:
    virtual ~Finder();

    // Try to find an extension of *message by fully-qualified field
    // name.  Returns nullptr if no extension is known for this name or number.
    // The base implementation uses the extensions already known by the message.
    virtual const FieldDescriptor* FindExtension(Message* message,
                                                 const std::string& name) const;

    // Similar to FindExtension, but uses a Descriptor and the extension number
    // instead of using a Message and the name when doing the look up.
    virtual const FieldDescriptor* FindExtensionByNumber(
        const Descriptor* descriptor, int number) const;

    // Find the message type for an Any proto.
    // Returns nullptr if no message is known for this name.
    // The base implementation only accepts prefixes of type.googleprod.com/ or
    // type.googleapis.com/, and searches the DescriptorPool of the parent
    // message.
    virtual const Descriptor* FindAnyType(const Message& message,
                                          const std::string& prefix,
                                          const std::string& name) const;

    // Find the message factory for the given extension field. This can be used
    // to generalize the Parser to add extension fields to a message in the same
    // way as the "input" message for the Parser.
    virtual MessageFactory* FindExtensionFactory(
        const FieldDescriptor* field) const;
  };

  // Class for those users which require more fine-grained control over how
  // a protobuffer message is printed out.
  class PROTOBUF_EXPORT Printer {
   public:
    Printer();

    // Like TextFormat::Print
    bool Print(const Message& message, io::ZeroCopyOutputStream* output) const;
    // Like TextFormat::Printer::Print but takes an additional
    // internal::FieldReporterLevel
    bool Print(const Message& message, io::ZeroCopyOutputStream* output,
               internal::FieldReporterLevel reporter) const;
    // Like TextFormat::PrintUnknownFields
    bool PrintUnknownFields(const UnknownFieldSet& unknown_fields,
                            io::ZeroCopyOutputStream* output) const;
    // Like TextFormat::PrintToString
    bool PrintToString(const Message& message, std::string* output) const;
    // Like TextFormat::PrintUnknownFieldsToString
    bool PrintUnknownFieldsToString(const UnknownFieldSet& unknown_fields,
                                    std::string* output) const;
    // Like TextFormat::PrintFieldValueToString
    void PrintFieldValueToString(const Message& message,
                                 const FieldDescriptor* field, int index,
                                 std::string* output) const;

    // Adjust the initial indent level of all output.  Each indent level is
    // equal to two spaces.
    void SetInitialIndentLevel(int indent_level) {
      initial_indent_level_ = indent_level;
    }

    // If printing in single line mode, then the entire message will be output
    // on a single line with no line breaks.
    void SetSingleLineMode(bool single_line_mode) {
      single_line_mode_ = single_line_mode;
    }

    bool IsInSingleLineMode() const { return single_line_mode_; }

    // If use_field_number is true, uses field number instead of field name.
    void SetUseFieldNumber(bool use_field_number) {
      use_field_number_ = use_field_number;
    }

    // Set true to print repeated primitives in a format like:
    //   field_name: [1, 2, 3, 4]
    // instead of printing each value on its own line.  Short format applies
    // only to primitive values -- i.e. everything except strings and
    // sub-messages/groups.
    void SetUseShortRepeatedPrimitives(bool use_short_repeated_primitives) {
      use_short_repeated_primitives_ = use_short_repeated_primitives;
    }

    // Set true to output UTF-8 instead of ASCII.  The only difference
    // is that bytes >= 0x80 in string fields will not be escaped,
    // because they are assumed to be part of UTF-8 multi-byte
    // sequences. This will change the default FastFieldValuePrinter.
    void SetUseUtf8StringEscaping(bool as_utf8);

    // Set the default FastFieldValuePrinter that is used for all fields that
    // don't have a field-specific printer registered.
    // Takes ownership of the printer.
    void SetDefaultFieldValuePrinter(const FastFieldValuePrinter* printer);

    [[deprecated("Please use FastFieldValuePrinter")]] void
    SetDefaultFieldValuePrinter(const FieldValuePrinter* printer);

    // Sets whether we want to hide unknown fields or not.
    // Usually unknown fields are printed in a generic way that includes the
    // tag number of the field instead of field name. However, sometimes it
    // is useful to be able to print the message without unknown fields (e.g.
    // for the python protobuf version to maintain consistency between its pure
    // python and c++ implementations).
    void SetHideUnknownFields(bool hide) { hide_unknown_fields_ = hide; }

    // If print_message_fields_in_index_order is true, fields of a proto message
    // will be printed using the order defined in source code instead of the
    // field number, extensions will be printed at the end of the message
    // and their relative order is determined by the extension number.
    // By default, use the field number order.
    void SetPrintMessageFieldsInIndexOrder(
        bool print_message_fields_in_index_order) {
      print_message_fields_in_index_order_ =
          print_message_fields_in_index_order;
    }

    // If expand==true, expand google.protobuf.Any payloads. The output
    // will be of form
    //    [type_url] { <value_printed_in_text> }
    //
    // If expand==false, print Any using the default printer. The output will
    // look like
    //    type_url: "<type_url>"  value: "serialized_content"
    void SetExpandAny(bool expand) { expand_any_ = expand; }

    // Set how parser finds message for Any payloads.
    void SetFinder(const Finder* finder) { finder_ = finder; }

    // If non-zero, we truncate all string fields that are  longer than
    // this threshold.  This is useful when the proto message has very long
    // strings, e.g., dump of encoded image file.
    //
    // NOTE:  Setting a non-zero value breaks round-trip safe
    // property of TextFormat::Printer.  That is, from the printed message, we
    // cannot fully recover the original string field any more.
    void SetTruncateStringFieldLongerThan(
        const int64_t truncate_string_field_longer_than) {
      truncate_string_field_longer_than_ = truncate_string_field_longer_than;
    }

    // Sets whether sensitive fields found in the message will be reported or
    // not.
    void SetReportSensitiveFields(internal::FieldReporterLevel reporter) {
      if (report_sensitive_fields_ < reporter) {
        report_sensitive_fields_ = reporter;
      }
    }

    // Register a custom field-specific FastFieldValuePrinter for fields
    // with a particular FieldDescriptor.
    // Returns "true" if the registration succeeded, or "false", if there is
    // already a printer for that FieldDescriptor.
    // Takes ownership of the printer on successful registration.
    bool RegisterFieldValuePrinter(const FieldDescriptor* field,
                                   const FastFieldValuePrinter* printer);

    [[deprecated("Please use FastFieldValuePrinter")]] bool
    RegisterFieldValuePrinter(const FieldDescriptor* field,
                              const FieldValuePrinter* printer);

    // Register a custom message-specific MessagePrinter for messages with a
    // particular Descriptor.
    // Returns "true" if the registration succeeded, or "false" if there is
    // already a printer for that Descriptor.
    bool RegisterMessagePrinter(const Descriptor* descriptor,
                                const MessagePrinter* printer);

    // Default printing for messages, which allows registered message printers
    // to fall back to default printing without losing the ability to control
    // sub-messages or fields.
    // NOTE: If the passed in `text_generaor` is not actually the current
    // `TextGenerator`, then no output will be produced.
    void PrintMessage(const Message& message,
                      BaseTextGenerator* generator) const;

   private:
    friend std::string Message::DebugString() const;
    friend std::string Message::ShortDebugString() const;
    friend std::string Message::Utf8DebugString() const;
    friend std::string internal::StringifyMessage(const Message& message,
                                                  internal::Option option);

    // Sets whether silent markers will be inserted.
    void SetInsertSilentMarker(bool v) { insert_silent_marker_ = v; }

    // Sets whether strings will be redacted and thus unparsable.
    void SetRedactDebugString(bool redact) { redact_debug_string_ = redact; }

    // Sets whether the output string should be made non-deterministic.
    // This discourages equality checks based on serialized string comparisons.
    void SetRandomizeDebugString(bool randomize) {
      randomize_debug_string_ = randomize;
    }

    // Forward declaration of an internal class used to print the text
    // output to the OutputStream (see text_format.cc for implementation).
    class TextGenerator;
    using MarkerToken = BaseTextGenerator::MarkerToken;

    // Forward declaration of an internal class used to print field values for
    // DebugString APIs (see text_format.cc for implementation).
    class DebugStringFieldValuePrinter;

    // Forward declaration of an internal class used to print UTF-8 escaped
    // strings (see text_format.cc for implementation).
    class FastFieldValuePrinterUtf8Escaping;

    // Internal Print method, used for writing to the OutputStream via
    // the TextGenerator class.
    void Print(const Message& message, BaseTextGenerator* generator) const;

    // Print a single field.
    void PrintField(const Message& message, const Reflection* reflection,
                    const FieldDescriptor* field,
                    BaseTextGenerator* generator) const;

    // Print a repeated primitive field in short form.
    void PrintShortRepeatedField(const Message& message,
                                 const Reflection* reflection,
                                 const FieldDescriptor* field,
                                 BaseTextGenerator* generator) const;

    // Print the name of a field -- i.e. everything that comes before the
    // ':' for a single name/value pair.
    void PrintFieldName(const Message& message, int field_index,
                        int field_count, const Reflection* reflection,
                        const FieldDescriptor* field,
                        BaseTextGenerator* generator) const;

    // Outputs a textual representation of the value of the field supplied on
    // the message supplied or the default value if not set.
    void PrintFieldValue(const Message& message, const Reflection* reflection,
                         const FieldDescriptor* field, int index,
                         BaseTextGenerator* generator) const;

    // Print the fields in an UnknownFieldSet.  They are printed by tag number
    // only.  Embedded messages are heuristically identified by attempting to
    // parse them (subject to the recursion budget).
    void PrintUnknownFields(const UnknownFieldSet& unknown_fields,
                            BaseTextGenerator* generator,
                            int recursion_budget) const;

    bool PrintAny(const Message& message, BaseTextGenerator* generator) const;

    // Try to redact a field value based on the annotations associated with
    // the field. This function returns true if it redacts the field value.
    bool TryRedactFieldValue(const Message& message,
                             const FieldDescriptor* field,
                             BaseTextGenerator* generator,
                             bool insert_value_separator) const;

    const FastFieldValuePrinter* GetFieldPrinter(
        const FieldDescriptor* field) const {
      auto it = custom_printers_.find(field);
      return it == custom_printers_.end() ? default_field_value_printer_.get()
                                          : it->second.get();
    }

    int initial_indent_level_;
    bool single_line_mode_;
    bool use_field_number_;
    bool use_short_repeated_primitives_;
    bool insert_silent_marker_;
    bool redact_debug_string_;
    bool randomize_debug_string_;
    internal::FieldReporterLevel report_sensitive_fields_;
    bool hide_unknown_fields_;
    bool print_message_fields_in_index_order_;
    bool expand_any_;
    int64_t truncate_string_field_longer_than_;

    std::unique_ptr<const FastFieldValuePrinter> default_field_value_printer_;
    absl::flat_hash_map<const FieldDescriptor*,
                        std::unique_ptr<const FastFieldValuePrinter>>
        custom_printers_;

    absl::flat_hash_map<const Descriptor*,
                        std::unique_ptr<const MessagePrinter>>
        custom_message_printers_;

    const Finder* finder_;
  };

  // Parses a text-format protocol message from the given input stream to
  // the given message object. This function parses the human-readable
  // serialization format written by Print(). Returns true on success. The
  // message is cleared first, even if the function fails -- See Merge() to
  // avoid this behavior.
  //
  // Example input: "user {\n id: 123 extra { gender: MALE language: 'en' }\n}"
  //
  // One common use for this function is parsing handwritten strings in test
  // code.
  //
  // If you would like to read a protocol buffer serialized in the
  // (non-human-readable) binary wire format, see
  // google::protobuf::MessageLite::ParseFromString().
  static bool Parse(io::ZeroCopyInputStream* input, Message* output);
  // Like Parse(), but reads directly from a string.
  static bool ParseFromString(absl::string_view input, Message* output);
  // Like Parse(), but reads directly from a Cord.
  static bool ParseFromCord(const absl::Cord& input, Message* output);

  // Like Parse(), but the data is merged into the given message, as if
  // using Message::MergeFrom().
  static bool Merge(io::ZeroCopyInputStream* input, Message* output);
  // Like Merge(), but reads directly from a string.
  static bool MergeFromString(absl::string_view input, Message* output);

  // Parse the given text as a single field value and store it into the
  // given field of the given message. If the field is a repeated field,
  // the new value will be added to the end
  static bool ParseFieldValueFromString(absl::string_view input,
                                        const FieldDescriptor* field,
                                        Message* message);

  // A location in the parsed text.
  struct ParseLocation {
    int line;
    int column;

    ParseLocation() : line(-1), column(-1) {}
    ParseLocation(int line_param, int column_param)
        : line(line_param), column(column_param) {}
  };

  // A range of locations in the parsed text, including `start` and excluding
  // `end`.
  struct ParseLocationRange {
    ParseLocation start;
    ParseLocation end;
    ParseLocationRange() : start(), end() {}
    ParseLocationRange(ParseLocation start_param, ParseLocation end_param)
        : start(start_param), end(end_param) {}
  };

  // Data structure which is populated with the locations of each field
  // value parsed from the text.
  class PROTOBUF_EXPORT ParseInfoTree {
   public:
    ParseInfoTree() = default;
    ParseInfoTree(const ParseInfoTree&) = delete;
    ParseInfoTree& operator=(const ParseInfoTree&) = delete;

    // Returns the parse location range for index-th value of the field in
    // the parsed text. If none exists, returns a location with start and end
    // line -1. Index should be -1 for not-repeated fields.
    ParseLocationRange GetLocationRange(const FieldDescriptor* field,
                                        int index) const;

    // Returns the starting parse location for index-th value of the field in
    // the parsed text. If none exists, returns a location with line = -1. Index
    // should be -1 for not-repeated fields.
    ParseLocation GetLocation(const FieldDescriptor* field, int index) const {
      return GetLocationRange(field, index).start;
    }

    // Returns the parse info tree for the given field, which must be a message
    // type. The nested information tree is owned by the root tree and will be
    // deleted when it is deleted.
    ParseInfoTree* GetTreeForNested(const FieldDescriptor* field,
                                    int index) const;

   private:
    // Allow the text format parser to record information into the tree.
    friend class TextFormat;

    // Records the starting and ending locations of a single value for a field.
    void RecordLocation(const FieldDescriptor* field, ParseLocationRange range);

    // Create and records a nested tree for a nested message field.
    ParseInfoTree* CreateNested(const FieldDescriptor* field);

    // Defines the map from the index-th field descriptor to its parse location.
    absl::flat_hash_map<const FieldDescriptor*, std::vector<ParseLocationRange>>
        locations_;
    // Defines the map from the index-th field descriptor to the nested parse
    // info tree.
    absl::flat_hash_map<const FieldDescriptor*,
                        std::vector<std::unique_ptr<ParseInfoTree>>>
        nested_;
  };

  // For more control over parsing, use this class.
  class PROTOBUF_EXPORT Parser {
   public:
    Parser();
    ~Parser();

    // Like TextFormat::Parse().
    bool Parse(io::ZeroCopyInputStream* input, Message* output);
    // Like TextFormat::ParseFromString().
    bool ParseFromString(absl::string_view input, Message* output);
    // Like TextFormat::ParseFromCord().
    bool ParseFromCord(const absl::Cord& input, Message* output);
    // Like TextFormat::Merge().
    bool Merge(io::ZeroCopyInputStream* input, Message* output);
    // Like TextFormat::MergeFromString().
    bool MergeFromString(absl::string_view input, Message* output);

    // Set where to report parse errors.  If nullptr (the default), errors will
    // be printed to stderr.
    void RecordErrorsTo(io::ErrorCollector* error_collector) {
      error_collector_ = error_collector;
    }

    // Set how parser finds extensions.  If nullptr (the default), the
    // parser will use the standard Reflection object associated with
    // the message being parsed.
    void SetFinder(const Finder* finder) { finder_ = finder; }

    // Sets where location information about the parse will be written. If
    // nullptr
    // (the default), then no location will be written.
    void WriteLocationsTo(ParseInfoTree* tree) { parse_info_tree_ = tree; }

    // Normally parsing fails if, after parsing, output->IsInitialized()
    // returns false.  Call AllowPartialMessage(true) to skip this check.
    void AllowPartialMessage(bool allow) { allow_partial_ = allow; }

    // Allow field names to be matched case-insensitively.
    // This is not advisable if there are fields that only differ in case, or
    // if you want to enforce writing in the canonical form.
    // This is 'false' by default.
    void AllowCaseInsensitiveField(bool allow) {
      allow_case_insensitive_field_ = allow;
    }

    // Like TextFormat::ParseFieldValueFromString
    bool ParseFieldValueFromString(absl::string_view input,
                                   const FieldDescriptor* field,
                                   Message* output);

    // When an unknown extension is met, parsing will fail if this option is
    // set to false (the default). If true, unknown extensions will be ignored
    // and a warning message will be generated.
    // Beware! Setting this option true may hide some errors (e.g. spelling
    // error on extension name).  This allows data loss; unlike binary format,
    // text format cannot preserve unknown extensions.  Avoid using this option
    // if possible.
    void AllowUnknownExtension(bool allow) { allow_unknown_extension_ = allow; }

    // When an unknown field is met, parsing will fail if this option is set
    // to false (the default). If true, unknown fields will be ignored and
    // a warning message will be generated.
    // Beware! Setting this option true may hide some errors (e.g. spelling
    // error on field name). This allows data loss; unlike binary format, text
    // format cannot preserve unknown fields.  Avoid using this option
    // if possible.
    void AllowUnknownField(bool allow) { allow_unknown_field_ = allow; }


    void AllowFieldNumber(bool allow) { allow_field_number_ = allow; }

    // Sets maximum recursion depth which parser can use. This is effectively
    // the maximum allowed nesting of proto messages.
    void SetRecursionLimit(int limit) { recursion_limit_ = limit; }

    // If called, the parser will report an error if a parsed field had no
    // effect on the resulting proto (for example, fields with no presence that
    // were set to their default value).
    void ErrorOnNoOpFields(bool return_error) {
      error_on_no_op_fields_ = return_error;
    }

   private:
    // Forward declaration of an internal class used to parse text
    // representations (see text_format.cc for implementation).
    class ParserImpl;

    // Like TextFormat::Merge().  The provided implementation is used
    // to do the parsing.
    bool MergeUsingImpl(io::ZeroCopyInputStream* input, Message* output,
                        ParserImpl* parser_impl);

    io::ErrorCollector* error_collector_;
    const Finder* finder_;
    ParseInfoTree* parse_info_tree_;
    bool allow_partial_;
    bool allow_case_insensitive_field_;
    bool allow_unknown_field_;
    bool allow_unknown_extension_;
    bool allow_unknown_enum_;
    bool allow_field_number_;
    bool allow_relaxed_whitespace_;
    bool allow_singular_overwrites_;
    int recursion_limit_;
    bool error_on_no_op_fields_ = false;
  };


 private:
  // Hack: ParseInfoTree declares TextFormat as a friend which should extend
  // the friendship to TextFormat::Parser::ParserImpl, but unfortunately some
  // old compilers (e.g. GCC 3.4.6) don't implement this correctly. We provide
  // helpers for ParserImpl to call methods of ParseInfoTree.
  static inline void RecordLocation(ParseInfoTree* info_tree,
                                    const FieldDescriptor* field,
                                    ParseLocationRange location);
  static inline ParseInfoTree* CreateNested(ParseInfoTree* info_tree,
                                            const FieldDescriptor* field);
  // To reduce stack frame bloat we use an out-of-line function to print
  // strings. This avoid local std::string temporaries.
  template <typename... T>
  static void OutOfLinePrintString(BaseTextGenerator* generator,
                                   const T&... values);
};


inline void TextFormat::RecordLocation(ParseInfoTree* info_tree,
                                       const FieldDescriptor* field,
                                       ParseLocationRange location) {
  info_tree->RecordLocation(field, location);
}

inline TextFormat::ParseInfoTree* TextFormat::CreateNested(
    ParseInfoTree* info_tree, const FieldDescriptor* field) {
  return info_tree->CreateNested(field);
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


#endif  // GOOGLE_PROTOBUF_TEXT_FORMAT_H__

