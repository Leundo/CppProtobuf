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
// Utility class for writing text to a ZeroCopyOutputStream.

#ifndef GOOGLE_PROTOBUF_IO_PRINTER_H__
#define GOOGLE_PROTOBUF_IO_PRINTER_H__

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <CppAbseil/absl_cleanup_cleanup.hpp>
#include <CppAbseil/absl_container_flat_hash_map.hpp>
#include <CppAbseil/absl_functional_function_ref.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_meta_type_traits.hpp>
#include <CppAbseil/absl_strings_str_cat.hpp>
#include <CppAbseil/absl_strings_str_format.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include <CppAbseil/absl_types_optional.hpp>
#include <CppAbseil/absl_types_variant.hpp>
#include <CppProtobuf/google_protobuf_io_zero_copy_sink.hpp>


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

// Records annotations about a Printer's output.
class PROTOBUF_EXPORT AnnotationCollector {
 public:
  // Annotation is a offset range and a payload pair. This payload's layout is
  // specific to derived types of AnnotationCollector.
  using Annotation = std::pair<std::pair<size_t, size_t>, std::string>;

  // The semantic meaning of an annotation. This enum mirrors
  // google.protobuf.GeneratedCodeInfo.Annotation.Semantic, and the enumerator values
  // should match it.
  enum Semantic {
    kNone = 0,
    kSet = 1,
    kAlias = 2,
  };

  virtual ~AnnotationCollector() = default;

  // Records that the bytes in file_path beginning with begin_offset and ending
  // before end_offset are associated with the SourceCodeInfo-style path.
  virtual void AddAnnotation(size_t begin_offset, size_t end_offset,
                             const std::string& file_path,
                             const std::vector<int>& path) = 0;

  virtual void AddAnnotation(size_t begin_offset, size_t end_offset,
                             const std::string& file_path,
                             const std::vector<int>& path,
                             std::optional<Semantic> semantic) {
    AddAnnotation(begin_offset, end_offset, file_path, path);
  }

  // TODO I don't see why we need virtuals here. Just a vector of
  // range, payload pairs stored in a context should suffice.
  virtual void AddAnnotationNew(Annotation&) {}
};

// Records annotations about a Printer's output to a Protobuf message,
// assuming that it has a repeated submessage field named `annotation` with
// fields matching
//
// message ??? {
//   repeated int32 path = 1;
//   optional string source_file = 2;
//   optional int32 begin = 3;
//   optional int32 end = 4;
//   optional int32 semantic = 5;
// }
template <typename AnnotationProto>
class AnnotationProtoCollector : public AnnotationCollector {
 private:
  // Some users of this type use it with a proto that does not have a
  // "semantic" field. Therefore, we need to detect it with SFINAE.

  // go/ranked-overloads
  struct Rank0 {};
  struct Rank1 : Rank0 {};

  template <typename Proto>
  static auto SetSemantic(Proto* p, int semantic, Rank1)
      -> decltype(p->set_semantic(
          static_cast<typename Proto::Semantic>(semantic))) {
    return p->set_semantic(static_cast<typename Proto::Semantic>(semantic));
  }

  template <typename Proto>
  static void SetSemantic(Proto*, int, Rank0) {}

 public:
  explicit AnnotationProtoCollector(AnnotationProto* annotation_proto)
      : annotation_proto_(annotation_proto) {}

  void AddAnnotation(size_t begin_offset, size_t end_offset,
                     const std::string& file_path,
                     const std::vector<int>& path) override {
    AddAnnotation(begin_offset, end_offset, file_path, path, absl::nullopt);
  }

  void AddAnnotation(size_t begin_offset, size_t end_offset,
                     const std::string& file_path, const std::vector<int>& path,
                     std::optional<Semantic> semantic) override {
    auto* annotation = annotation_proto_->add_annotation();
    for (int i = 0; i < path.size(); ++i) {
      annotation->add_path(path[i]);
    }
    annotation->set_source_file(file_path);
    annotation->set_begin(begin_offset);
    annotation->set_end(end_offset);

    if (semantic.has_value()) {
      SetSemantic(annotation, *semantic, Rank1{});
    }
  }

  void AddAnnotationNew(Annotation& a) override {
    auto* annotation = annotation_proto_->add_annotation();
    annotation->ParseFromString(a.second);
    annotation->set_begin(a.first.first);
    annotation->set_end(a.first.second);
  }

 private:
  AnnotationProto* annotation_proto_;
};

// A source code printer for assisting in code generation.
//
// This type implements a simple templating language for substiting variables
// into static, user-provided strings, and also tracks indentation
// automatically.
//
// The main entry-point for this type is the Emit function, which can be used
// thus:
//
//   Printer p(output);
//   p.Emit({{"class", my_class_name}}, R"cc(
//     class $class$ {
//      public:
//       $class$(int x) : x_(x) {}
//      private:
//       int x_;
//     };
//   )cc");
//
// Substitutions are of the form $var$, which is looked up in the map passed in
// as the first argument. The variable delimiter character, $, can be chosen to
// be something convenient for the target language. For example, in PHP, which
// makes heavy use of $, it can be made into something like # instead.
//
// A literal $ can be emitted by writing $$.
//
// Substitutions may contain spaces around the name of the variable, which will
// be ignored for the purposes of looking up the variable to substitute in, but
// which will be reproduced in the output:
//
//   p.Emit({{"foo", "bar"}}, "$ foo $");
//
// emits the string " bar ". If the substituted-in variable is the empty string,
// then the surrounding spaces are *not* printed:
//
//   p.Emit({{"xzy", xyz}}, "$xyz $Thing");
//
// If xyz is "Foo", this will become "Foo Thing", but if it is "", this becomes
// "Thing", rather than " Thing". This helps minimize awkward whitespace in the
// output.
//
// The value may be any type that can be stringified with `absl::StrCat`:
//
//   p.Emit({{"num", 5}}, "x = $num$;");
//
// If a variable is referenced in the format string that is missing, the program
// will crash. Callers must statically know that every variable reference is
// valid, and MUST NOT pass user-provided strings directly into Emit().
//
// Substitutions can be configured to "chomp" a single character after them, to
// help make indentation work out. This can be configured by passing a
// io::Printer::Sub().WithSuffix() into Emit's substitution map:
//   p.Emit({io::Printer::Sub("var", var_decl).WithSuffix(";")}, R"cc(
//     class $class$ {
//      public:
//       $var$;
//     };
//   )cc");
//
// This will delete the ; after $var$, regardless of whether it was an empty
// declaration or not. It will also intelligently attempt to clean up
// empty lines that follow, if it was on an empty line; this promotes cleaner
// formatting of the output.
//
// Any number of different characters can be potentially skipped, but only one
// will actually be skipped. For example, callback substitutions (see below) use
// ";," by default as their "chomping set".
//
// # Callback Substitution
//
// Instead of passing a string into Emit(), it is possible to pass in a callback
// as a variable mapping. This will take indentation into account, which allows
// factoring out parts of a formatting string while ensuring braces are
// balanced:
//
//   p.Emit(
//     {{"methods", [&] {
//       p.Emit(R"cc(
//         int Bar() {
//            return 42;
//         }
//       )cc");
//     }}},
//     R"cc(
//       class Foo {
//        public:
//         $methods$;
//       };
//     )cc"
//   );
//
// This emits
//
//   class Foo {
//    public:
//     int Bar() {
//       return 42;
//     }
//   };
//
// # Comments
//
// It may be desirable to place comments in a raw string that are stripped out
// before printing. The prefix for Printer-ignored comments can be configured
// in Options. By default, this is `//~`.
//
//   p.Emit(R"cc(
//     // Will be printed in the output.
//     //~ Won't be.
//   )cc");
//
// # Lookup Frames
//
// If many calls to Emit() use the same set of variables, they can be stored
// in a *variable lookup frame*, like so:
//
//   auto vars = p.WithVars({{"class_name", my_class_name}});
//   p.Emit(R"cc(
//     class $class_name$ {
//      public:
//       $class_name$(int x);
//       // Etc.
//     };
//   )cc");
//
// WithVars() returns an RAII object that will "pop" the lookup frame on scope
// exit, ensuring that the variables remain local. There are a few different
// overloads of WithVars(); it accepts a map type, like absl::flat_hash_map,
// either by-value (which will cause the Printer to store a copy), or by
// pointer (which will cause the Printer to store a pointer, potentially
// avoiding a copy.)
//
// p.Emit(vars, "..."); is effectively syntax sugar for
//
//  { auto v = p.WithVars(vars); p.Emit("..."); }
//
// NOTE: callbacks are *not* allowed with WithVars; callbacks should be local
// to a specific Emit() call.
//
// # Annotations
//
// If Printer is given an AnnotationCollector, it will use it to record which
// spans of generated code correspond to user-indicated descriptors. There are
// a few different ways of indicating when to emit annotations.
//
// The WithAnnotations() function is like WithVars(), but accepts maps with
// string keys and descriptor values. It adds an annotation variable frame and
// returns an RAII object that pops the frame.
//
// There are two different ways to annotate code. In the first, when
// substituting a variable, if there is an annotation with the same name, then
// the resulting expanded value's span will be annotated with that annotation.
// For example:
//
//   auto v = p.WithVars({{"class_name", my_class_name}});
//   auto a = p.WithAnnotations({{"class_name", message_descriptor}});
//   p.Emit(R"cc(
//     class $class_name$ {
//      public:
//       $class_name$(int x);
//       // Etc.
//     };
//   )cc");
//
// The span corresponding to whatever $class_name$ expands to will be annotated
// as having come from message_descriptor.
//
// For convenience, this can be done with a single WithVars(), using the special
// three-argument form:
//
//   auto v = p.WithVars({{"class_name", my_class_name, message_descriptor}});
//   p.Emit(R"cc(
//     class $class_name$ {
//      public:
//       $class_name$(int x);
//       // Etc.
//     };
//   )cc");
//
//
// Alternatively, a range may be given explicitly:
//
//   auto a = p.WithAnnotations({{"my_desc", message_descriptor}});
//   p.Emit(R"cc(
//     $_start$my_desc$
//     class Foo {
//       // Etc.
//     };
//     $_end$my_desc$
//   )cc");
//
// The special $_start$ and $_end$ variables indicate the start and end of an
// annotated span, which is annotated with the variable that follows. This
// form can produce somewhat unreadable format strings and is not recommended.
//
// Note that whitespace after a $_start$ and before an $_end$ is not printed.
//
// # Indentation
//
// Printer tracks an indentation amount to add to each new line, independent
// from indentation in an Emit() call's literal. The amount of indentation to
// add is controlled by the WithIndent() function:
//
//   p.Emit("class $class_name$ {");
//   {
//     auto indent = p.WithIndent();
//     p.Emit(R"cc(
//       public:
//        $class_name$(int x);
//     )cc");
//   }
//   p.Emit("};");
//
// This will automatically add one level of indentation to all code in scope of
// `indent`, which is an RAII object much like the return value of `WithVars()`.
//
// # Old API
// TODO: Delete this documentation.
//
// Printer supports an older-style API that is in the process of being
// re-written. The old documentation is reproduced here until all use-cases are
// handled.
//
// This simple utility class assists in code generation.  It basically
// allows the caller to define a set of variables and then output some
// text with variable substitutions.  Example usage:
//
//   Printer printer(output, '$');
//   map<string, string> vars;
//   vars["name"] = "Bob";
//   printer.Print(vars, "My name is $name$.");
//
// The above writes "My name is Bob." to the output stream.
//
// Printer aggressively enforces correct usage, crashing (with assert failures)
// in the case of undefined variables in debug builds. This helps greatly in
// debugging code which uses it.
//
// If a Printer is constructed with an AnnotationCollector, it will provide it
// with annotations that connect the Printer's output to paths that can identify
// various descriptors.  In the above example, if person_ is a descriptor that
// identifies Bob, we can associate the output string "My name is Bob." with
// a source path pointing to that descriptor with:
//
//   printer.Annotate("name", person_);
//
// The AnnotationCollector will be sent an annotation linking the output range
// covering "Bob" to the logical path provided by person_.  Tools may use
// this association to (for example) link "Bob" in the output back to the
// source file that defined the person_ descriptor identifying Bob.
//
// Annotate can only examine variables substituted during the last call to
// Print.  It is invalid to refer to a variable that was used multiple times
// in a single Print call.
//
// In full generality, one may specify a range of output text using a beginning
// substitution variable and an ending variable.  The resulting annotation will
// span from the first character of the substituted value for the beginning
// variable to the last character of the substituted value for the ending
// variable.  For example, the Annotate call above is equivalent to this one:
//
//   printer.Annotate("name", "name", person_);
//
// This is useful if multiple variables combine to form a single span of output
// that should be annotated with the same source path.  For example:
//
//   Printer printer(output, '$');
//   map<string, string> vars;
//   vars["first"] = "Alice";
//   vars["last"] = "Smith";
//   printer.Print(vars, "My name is $first$ $last$.");
//   printer.Annotate("first", "last", person_);
//
// This code would associate the span covering "Alice Smith" in the output with
// the person_ descriptor.
//
// Note that the beginning variable must come before (or overlap with, in the
// case of zero-sized substitution values) the ending variable.
//
// It is also sometimes useful to use variables with zero-sized values as
// markers.  This avoids issues with multiple references to the same variable
// and also allows annotation ranges to span literal text from the Print
// templates:
//
//   Printer printer(output, '$');
//   map<string, string> vars;
//   vars["foo"] = "bar";
//   vars["function"] = "call";
//   vars["mark"] = "";
//   printer.Print(vars, "$function$($foo$,$foo$)$mark$");
//   printer.Annotate("function", "mark", call_);
//
// This code associates the span covering "call(bar,bar)" in the output with the
// call_ descriptor.
class PROTOBUF_EXPORT Printer {
 private:
  struct AnnotationRecord;

 public:
  // This type exists to work around an absl type that has not yet been
  // released.
  struct SourceLocation {
    static SourceLocation current() { return {}; }
    absl::string_view file_name() { return "<unknown>"; }
    int line() { return 0; }
  };

  static constexpr char kDefaultVariableDelimiter = '$';
  static constexpr absl::string_view kProtocCodegenTrace =
      "PROTOC_CODEGEN_TRACE";

  // Sink type for constructing substitutions to pass to WithVars() and Emit().
  class Sub;

  // Options for controlling how the output of a Printer is formatted.
  struct Options {
    Options() = default;
    Options(const Options&) = default;
    Options(Options&&) = default;
    Options(char variable_delimiter, AnnotationCollector* annotation_collector)
        : variable_delimiter(variable_delimiter),
          annotation_collector(annotation_collector) {}

    // The delimiter for variable substitutions, e.g. $foo$.
    char variable_delimiter = kDefaultVariableDelimiter;
    // An optional listener the Printer calls whenever it emits a source
    // annotation; may be null.
    AnnotationCollector* annotation_collector = nullptr;
    // The "comment start" token for the language being generated. This is used
    // to allow the Printer to emit debugging annotations in the source code
    // output.
    absl::string_view comment_start = "//";
    // The token for beginning comments that are discarded by Printer's internal
    // formatter.
    absl::string_view ignored_comment_start = "//~";
    // The number of spaces that a single level of indentation adds by default;
    // this is the amount that WithIndent() increases indentation by.
    size_t spaces_per_indent = 2;
    // Whether to emit a "codegen trace" for calls to Emit(). If true, each call
    // to Emit() will print a comment indicating where in the source of the
    // compiler the Emit() call occurred.
    //
    // If disengaged, defaults to whether or not the environment variable
    // `PROTOC_CODEGEN_TRACE` is set.
    std::optional<bool> enable_codegen_trace = absl::nullopt;
  };

  // Constructs a new Printer with the default options to output to
  // `output`.
  explicit Printer(ZeroCopyOutputStream* output);

  // Constructs a new printer with the given set of options to output to
  // `output`.
  Printer(ZeroCopyOutputStream* output, Options options);

  // Old-style constructor. Avoid in preference to the two constructors above.
  //
  // Will eventually be marked as deprecated.
  Printer(ZeroCopyOutputStream* output, char variable_delimiter,
          AnnotationCollector* annotation_collector = nullptr);

  Printer(const Printer&) = delete;
  Printer& operator=(const Printer&) = delete;

  // Pushes a new variable lookup frame that stores `vars` by reference.
  //
  // Returns an RAII object that pops the lookup frame.
  template <typename Map>
  auto WithVars(const Map* vars);

  // Pushes a new variable lookup frame that stores `vars` by value.
  //
  // Returns an RAII object that pops the lookup frame.
  template <typename Map = absl::flat_hash_map<std::string, std::string>,
            typename = std::enable_if_t<!std::is_pointer<Map>::value>,
            // Prefer the more specific span impl if this could be turned into
            // a span.
            typename = std::enable_if_t<
                !std::is_convertible<Map, absl::Span<const Sub>>::value>>
  auto WithVars(Map&& vars);

  // Pushes a new variable lookup frame that stores `vars` by value.
  //
  // Returns an RAII object that pops the lookup frame.
  auto WithVars(absl::Span<const Sub> vars);

  // Looks up a variable set with WithVars().
  //
  // Will crash if:
  // - `var` is not present in the lookup frame table.
  // - `var` is a callback, rather than a string.
  absl::string_view LookupVar(absl::string_view var);

  // Pushes a new annotation lookup frame that stores `vars` by reference.
  //
  // Returns an RAII object that pops the lookup frame.
  template <typename Map>
  auto WithAnnotations(const Map* vars);

  // Pushes a new variable lookup frame that stores `vars` by value.
  //
  // When writing `WithAnnotations({...})`, this is the overload that will be
  // called, and it will synthesize an `absl::flat_hash_map`.
  //
  // Returns an RAII object that pops the lookup frame.
  template <typename Map = absl::flat_hash_map<std::string, AnnotationRecord>>
  auto WithAnnotations(Map&& vars);

  // Increases the indentation by `indent` spaces; when nullopt, increments
  // indentation by the configured default spaces_per_indent.
  //
  // Returns an RAII object that removes this indentation.
  auto WithIndent(std::optional<size_t> indent = absl::nullopt) {
    size_t delta = indent.value_or(options_.spaces_per_indent);
    indent_ += delta;
    return absl::MakeCleanup([this, delta] { indent_ -= delta; });
  }

  // Emits formatted source code to the underlying output. See the class
  // documentation for more details.
  //
  // `format` MUST be a string constant.
  void Emit(absl::string_view format,
            SourceLocation loc = SourceLocation::current());

  // Emits formatted source code to the underlying output, injecting
  // additional variables as a lookup frame for just this call. See the class
  // documentation for more details.
  //
  // `format` MUST be a string constant.
  void Emit(absl::Span<const Sub> vars, absl::string_view format,
            SourceLocation loc = SourceLocation::current());

  // Write a string directly to the underlying output, performing no formatting
  // of any sort.
  void PrintRaw(absl::string_view data) { WriteRaw(data.data(), data.size()); }

  // Write a string directly to the underlying output, performing no formatting
  // of any sort.
  void WriteRaw(const char* data, size_t size);

  // True if any write to the underlying stream failed.  (We don't just
  // crash in this case because this is an I/O failure, not a programming
  // error.)
  bool failed() const { return failed_; }

  // -- Old-style API below; to be deprecated and removed. --
  // TODO: Deprecate these APIs.

  template <typename Map = absl::flat_hash_map<std::string, std::string>>
  void Print(const Map& vars, absl::string_view text);

  template <typename... Args>
  void Print(absl::string_view text, const Args&... args);

  // Link a substitution variable emitted by the last call to Print to the
  // object described by descriptor.
  template <typename SomeDescriptor>
  void Annotate(
      absl::string_view varname, const SomeDescriptor* descriptor,
      std::optional<AnnotationCollector::Semantic> semantic = absl::nullopt) {
    Annotate(varname, varname, descriptor, semantic);
  }

  // Link the output range defined by the substitution variables as emitted by
  // the last call to Print to the object described by descriptor. The range
  // begins at begin_varname's value and ends after the last character of the
  // value substituted for end_varname.
  template <typename Desc>
  void Annotate(
      absl::string_view begin_varname, absl::string_view end_varname,
      const Desc* descriptor,
      std::optional<AnnotationCollector::Semantic> semantic = absl::nullopt);

  // Link a substitution variable emitted by the last call to Print to the file
  // with path file_name.
  void Annotate(
      absl::string_view varname, absl::string_view file_name,
      std::optional<AnnotationCollector::Semantic> semantic = absl::nullopt) {
    Annotate(varname, varname, file_name, semantic);
  }

  // Link the output range defined by the substitution variables as emitted by
  // the last call to Print to the file with path file_name. The range begins
  // at begin_varname's value and ends after the last character of the value
  // substituted for end_varname.
  void Annotate(
      absl::string_view begin_varname, absl::string_view end_varname,
      absl::string_view file_name,
      std::optional<AnnotationCollector::Semantic> semantic = absl::nullopt) {
    if (options_.annotation_collector == nullptr) {
      return;
    }

    Annotate(begin_varname, end_varname, file_name, {}, semantic);
  }

  // Indent text by `options.spaces_per_indent`; undone by Outdent().
  void Indent() { indent_ += options_.spaces_per_indent; }

  // Undoes a call to Indent().
  void Outdent();

  // FormatInternal is a helper function not meant to use directly, use
  // compiler::cpp::Formatter instead.
  template <typename Map = absl::flat_hash_map<std::string, std::string>>
  void FormatInternal(absl::Span<const std::string> args, const Map& vars,
                      absl::string_view format);

 private:
  struct PrintOptions;
  struct Format;

  // Helper type for wrapping a variable substitution expansion result.
  template <bool owned>
  struct ValueImpl;

  using ValueView = ValueImpl</*owned=*/false>;
  using Value = ValueImpl</*owned=*/true>;

  // Provide a helper to use heterogeneous lookup when it's available.
  template <typename...>
  using Void = void;

  template <typename Map, typename = void>
  struct HasHeteroLookup : std::false_type {};
  template <typename Map>
  struct HasHeteroLookup<Map, Void<decltype(std::declval<Map>().find(
                                  std::declval<absl::string_view>()))>>
      : std::true_type {};

  template <typename Map,
            typename = std::enable_if_t<HasHeteroLookup<Map>::value>>
  static absl::string_view ToStringKey(absl::string_view x) {
    return x;
  }

  template <typename Map,
            typename = std::enable_if_t<!HasHeteroLookup<Map>::value>>
  static std::string ToStringKey(absl::string_view x) {
    return std::string(x);
  }

  Format TokenizeFormat(absl::string_view format_string,
                        const PrintOptions& options);

  // Emit an annotation for the range defined by the given substitution
  // variables, as set by the most recent call to PrintImpl() that set
  // `use_substitution_map` to true.
  //
  // The range begins at the start of `begin_varname`'s value and ends after the
  // last byte of `end_varname`'s value.
  //
  // `begin_varname` and `end_varname may` refer to the same variable.
  void Annotate(absl::string_view begin_varname, absl::string_view end_varname,
                absl::string_view file_path, const std::vector<int>& path,
                std::optional<AnnotationCollector::Semantic> semantic);

  // The core printing implementation. There are three public entry points,
  // which enable different slices of functionality that are controlled by the
  // `opts` argument.
  void PrintImpl(absl::string_view format, absl::Span<const std::string> args,
                 PrintOptions opts);

  // This is a private function only so that it can see PrintOptions.
  static bool Validate(bool cond, PrintOptions opts,
                       absl::FunctionRef<std::string()> message);
  static bool Validate(bool cond, PrintOptions opts, absl::string_view message);

  // Performs calls to `Validate()` to check that `index < current_arg_index`
  // and `index < args_len`, producing appropriate log lines if the checks fail,
  // and crashing if necessary.
  bool ValidateIndexLookupInBounds(size_t index, size_t current_arg_index,
                                   size_t args_len, PrintOptions opts);

  // Prints indentation if `at_start_of_line_` is true.
  void IndentIfAtStart();

  // Prints a codegen trace, for the given location in the compiler's source.
  void PrintCodegenTrace(std::optional<SourceLocation> loc);

  // The core implementation for "fully-elaborated" variable definitions.
  auto WithDefs(absl::Span<const Sub> vars, bool allow_callbacks);

  // Returns the start and end of the value that was substituted in place of
  // the variable `varname` in the last call to PrintImpl() (with
  // `use_substitution_map` set), if such a variable was substituted exactly
  // once.
  std::optional<std::pair<size_t, size_t>> GetSubstitutionRange(
      absl::string_view varname, PrintOptions opts);

  google::protobuf::io::zc_sink_internal::ZeroCopyStreamByteSink sink_;
  Options options_;
  size_t indent_ = 0;
  bool at_start_of_line_ = true;
  bool failed_ = false;

  size_t paren_depth_ = 0;
  std::vector<size_t> paren_depth_to_omit_;

  std::vector<std::function<std::optional<ValueView>(absl::string_view)>>
      var_lookups_;

  std::vector<
      std::function<std::optional<AnnotationRecord>(absl::string_view)>>
      annotation_lookups_;

  // A map from variable name to [start, end) offsets in the output buffer.
  //
  // This stores the data looked up by GetSubstitutionRange().
  absl::flat_hash_map<std::string, std::pair<size_t, size_t>> substitutions_;
  // Keeps track of the keys in `substitutions_` that need to be updated when
  // indents are inserted. These are keys that refer to the beginning of the
  // current line.
  std::vector<std::string> line_start_variables_;
};

// Options for PrintImpl().
struct Printer::PrintOptions {
  // The callsite of the public entry-point. Only Emit() sets this.
  std::optional<SourceLocation> loc;
  // If set, Validate() calls will not crash the program.
  bool checks_are_debug_only = false;
  // If set, the `substitutions_` map will be populated as variables are
  // substituted.
  bool use_substitution_map = false;
  // If set, the ${1$ and $}$ forms will be substituted. These are used for
  // a slightly janky annotation-insertion mechanism in FormatInternal, that
  // requires that passed-in substitution variables be serialized protos.
  bool use_curly_brace_substitutions = false;
  // If set, the $n$ forms will be substituted, pulling from the `args`
  // argument to PrintImpl().
  bool allow_digit_substitutions = true;
  // If set, when a variable substitution with spaces in it, such as $ var$,
  // is encountered, the spaces are stripped, so that it is as if it was
  // $var$. If $var$ substitutes to a non-empty string, the removed spaces are
  // printed around the substituted value.
  //
  // See the class documentation for more information on this behavior.
  bool strip_spaces_around_vars = true;
  // If set, leading whitespace will be stripped from the format string to
  // determine the "extraneous indentation" that is produced when the format
  // string is a C++ raw string. This is used to remove leading spaces from
  // a raw string that would otherwise result in erratic indentation in the
  // output.
  bool strip_raw_string_indentation = false;
  // If set, the annotation lookup frames are searched, per the annotation
  // semantics of Emit() described in the class documentation.
  bool use_annotation_frames = true;
};

// Helper type for wrapping a variable substitution expansion result.
template <bool owned>
struct Printer::ValueImpl {
 private:
  template <typename T>
  struct IsSubImpl : std::false_type {};
  template <bool a>
  struct IsSubImpl<ValueImpl<a>> : std::true_type {};

 public:
  using StringType = std::conditional_t<owned, std::string, absl::string_view>;
  // These callbacks return false if this is a recursive call.
  using Callback = std::function<bool()>;
  using StringOrCallback = absl::variant<StringType, Callback>;

  ValueImpl() = default;

  // This is a template to avoid colliding with the copy constructor below.
  template <typename Value,
            typename = std::enable_if_t<
                !IsSubImpl<absl::remove_cvref_t<Value>>::value>>
  ValueImpl(Value&& value)  // NOLINT
      : value(ToStringOrCallback(std::forward<Value>(value), Rank2{})) {
    if (absl::holds_alternative<Callback>(this->value)) {
      consume_after = ";,";
    }
  }

  // Copy ctor/assign allow interconversion of the two template parameters.
  template <bool that_owned>
  ValueImpl(const ValueImpl<that_owned>& that) {  // NOLINT
    *this = that;
  }

  template <bool that_owned>
  ValueImpl& operator=(const ValueImpl<that_owned>& that);

  const StringType* AsString() const {
    return absl::get_if<StringType>(&value);
  }

  const Callback* AsCallback() const { return absl::get_if<Callback>(&value); }

  StringOrCallback value;
  std::string consume_after;
  bool consume_parens_if_empty = false;

 private:
  // go/ranked-overloads
  struct Rank0 {};
  struct Rank1 : Rank0 {};
  struct Rank2 : Rank1 {};

  // Dummy template for delayed instantiation, which is required for the
  // static assert below to kick in only when this function is called when it
  // shouldn't.
  //
  // This is done to produce a better error message than the "candidate does
  // not match" SFINAE errors.
  template <typename Cb, typename = decltype(std::declval<Cb&&>()())>
  StringOrCallback ToStringOrCallback(Cb&& cb, Rank2);

  // Separate from the AlphaNum overload to avoid copies when taking strings
  // by value when in `owned` mode.
  StringOrCallback ToStringOrCallback(StringType s, Rank1) { return s; }

  StringOrCallback ToStringOrCallback(const absl::AlphaNum& s, Rank0) {
    return StringType(s.Piece());
  }
};

template <bool owned>
template <bool that_owned>
Printer::ValueImpl<owned>& Printer::ValueImpl<owned>::operator=(
    const ValueImpl<that_owned>& that) {
  // Cast to void* is required, since this and that may potentially be of
  // different types (due to the `that_owned` parameter).
  if (static_cast<const void*>(this) == static_cast<const void*>(&that)) {
    return *this;
  }

  using ThatStringType = typename ValueImpl<that_owned>::StringType;

  if (auto* str = absl::get_if<ThatStringType>(&that.value)) {
    value = StringType(*str);
  } else {
    value = absl::get<Callback>(that.value);
  }

  consume_after = that.consume_after;
  consume_parens_if_empty = that.consume_parens_if_empty;
  return *this;
}

template <bool owned>
template <typename Cb, typename /*Sfinae*/>
auto Printer::ValueImpl<owned>::ToStringOrCallback(Cb&& cb, Rank2)
    -> StringOrCallback {
  return Callback(
      [cb = std::forward<Cb>(cb), is_called = false]() mutable -> bool {
        if (is_called) {
          // Catch whether or not this function is being called recursively.
          return false;
        }
        is_called = true;
        cb();
        is_called = false;
        return true;
      });
}

struct Printer::AnnotationRecord {
  std::vector<int> path;
  std::string file_path;
  std::optional<AnnotationCollector::Semantic> semantic;

  // AnnotationRecord's constructors are *not* marked as explicit,
  // specifically so that it is possible to construct a
  // map<string, AnnotationRecord> by writing
  //
  // {{"foo", my_cool_descriptor}, {"bar", "file.proto"}}

  template <
      typename String,
      std::enable_if_t<std::is_convertible<const String&, std::string>::value,
                       int> = 0>
  AnnotationRecord(  // NOLINT(google-explicit-constructor)
      const String& file_path,
      std::optional<AnnotationCollector::Semantic> semantic = absl::nullopt)
      : file_path(file_path), semantic(semantic) {}

  template <typename Desc,
            // This SFINAE clause excludes char* from matching this
            // constructor.
            std::enable_if_t<std::is_class<Desc>::value, int> = 0>
  AnnotationRecord(  // NOLINT(google-explicit-constructor)
      const Desc* desc,
      std::optional<AnnotationCollector::Semantic> semantic = absl::nullopt)
      : file_path(desc->file()->name()), semantic(semantic) {
    desc->GetLocationPath(&path);
  }
};

class Printer::Sub {
 public:
  template <typename Value>
  Sub(std::string key, Value&& value)
      : key_(std::move(key)),
        value_(std::forward<Value>(value)),
        annotation_(absl::nullopt) {}

  Sub AnnotatedAs(AnnotationRecord annotation) && {
    annotation_ = std::move(annotation);
    return std::move(*this);
  }

  Sub WithSuffix(std::string sub_suffix) && {
    value_.consume_after = std::move(sub_suffix);
    return std::move(*this);
  }

  Sub ConditionalFunctionCall() && {
    value_.consume_parens_if_empty = true;
    return std::move(*this);
  }

  absl::string_view key() const { return key_; }

  absl::string_view value() const {
    const auto* str = value_.AsString();
    ABSL_CHECK(str != nullptr)
        << "could not find " << key() << "; found callback instead";
    return *str;
  }

 private:
  friend class Printer;

  std::string key_;
  Value value_;
  std::optional<AnnotationRecord> annotation_;
};

template <typename Map>
auto Printer::WithVars(const Map* vars) {
  var_lookups_.emplace_back(
      [vars](absl::string_view var) -> std::optional<ValueView> {
        auto it = vars->find(ToStringKey<Map>(var));
        if (it == vars->end()) {
          return absl::nullopt;
        }
        return ValueView(it->second);
      });
  return absl::MakeCleanup([this] { var_lookups_.pop_back(); });
}

template <typename Map, typename, typename /*Sfinae*/>
auto Printer::WithVars(Map&& vars) {
  var_lookups_.emplace_back(
      [vars = std::forward<Map>(vars)](
          absl::string_view var) -> std::optional<ValueView> {
        auto it = vars.find(ToStringKey<Map>(var));
        if (it == vars.end()) {
          return absl::nullopt;
        }
        return ValueView(it->second);
      });
  return absl::MakeCleanup([this] { var_lookups_.pop_back(); });
}

template <typename Map>
auto Printer::WithAnnotations(const Map* vars) {
  annotation_lookups_.emplace_back(
      [vars](absl::string_view var) -> std::optional<AnnotationRecord> {
        auto it = vars->find(ToStringKey<Map>(var));
        if (it == vars->end()) {
          return absl::nullopt;
        }
        return AnnotationRecord(it->second);
      });
  return absl::MakeCleanup([this] { annotation_lookups_.pop_back(); });
}

template <typename Map>
auto Printer::WithAnnotations(Map&& vars) {
  annotation_lookups_.emplace_back(
      [vars = std::forward<Map>(vars)](
          absl::string_view var) -> std::optional<AnnotationRecord> {
        auto it = vars.find(ToStringKey<Map>(var));
        if (it == vars.end()) {
          return absl::nullopt;
        }
        return AnnotationRecord(it->second);
      });
  return absl::MakeCleanup([this] { annotation_lookups_.pop_back(); });
}

inline void Printer::Emit(absl::string_view format, SourceLocation loc) {
  Emit({}, format, loc);
}

template <typename Map>
void Printer::Print(const Map& vars, absl::string_view text) {
  PrintOptions opts;
  opts.checks_are_debug_only = true;
  opts.use_substitution_map = true;
  opts.allow_digit_substitutions = false;

  auto pop = WithVars(&vars);
  PrintImpl(text, {}, opts);
}

template <typename... Args>
void Printer::Print(absl::string_view text, const Args&... args) {
  static_assert(sizeof...(args) % 2 == 0, "");

  // Include an extra arg, since a zero-length array is ill-formed, and
  // MSVC complains.
  absl::string_view vars[] = {args..., ""};
  absl::flat_hash_map<absl::string_view, absl::string_view> map;
  map.reserve(sizeof...(args) / 2);
  for (size_t i = 0; i < sizeof...(args); i += 2) {
    map.emplace(vars[i], vars[i + 1]);
  }

  Print(map, text);
}

template <typename Desc>
void Printer::Annotate(absl::string_view begin_varname,
                       absl::string_view end_varname, const Desc* descriptor,
                       std::optional<AnnotationCollector::Semantic> semantic) {
  if (options_.annotation_collector == nullptr) {
    return;
  }

  std::vector<int> path;
  descriptor->GetLocationPath(&path);
  Annotate(begin_varname, end_varname, descriptor->file()->name(), path,
           semantic);
}

template <typename Map>
void Printer::FormatInternal(absl::Span<const std::string> args,
                             const Map& vars, absl::string_view format) {
  PrintOptions opts;
  opts.use_curly_brace_substitutions = true;
  opts.strip_spaces_around_vars = true;

  auto pop = WithVars(&vars);
  PrintImpl(format, args, opts);
}

inline auto Printer::WithDefs(absl::Span<const Sub> vars,
                              bool allow_callbacks) {
  absl::flat_hash_map<std::string, Value> var_map;
  var_map.reserve(vars.size());

  absl::flat_hash_map<std::string, AnnotationRecord> annotation_map;

  for (const auto& var : vars) {
    ABSL_CHECK(allow_callbacks || var.value_.AsCallback() == nullptr)
        << "callback arguments are not permitted in this position";
    auto result = var_map.insert({var.key_, var.value_});
    ABSL_CHECK(result.second)
        << "repeated variable in Emit() or WithVars() call: \"" << var.key_
        << "\"";
    if (var.annotation_.has_value()) {
      annotation_map.insert({var.key_, *var.annotation_});
    }
  }

  var_lookups_.emplace_back([map = std::move(var_map)](absl::string_view var)
                                -> std::optional<ValueView> {
    auto it = map.find(var);
    if (it == map.end()) {
      return absl::nullopt;
    }
    return ValueView(it->second);
  });

  bool has_annotations = !annotation_map.empty();
  if (has_annotations) {
    annotation_lookups_.emplace_back(
        [map = std::move(annotation_map)](
            absl::string_view var) -> std::optional<AnnotationRecord> {
          auto it = map.find(var);
          if (it == map.end()) {
            return absl::nullopt;
          }
          return it->second;
        });
  }

  return absl::MakeCleanup([this, has_annotations] {
    var_lookups_.pop_back();
    if (has_annotations) {
      annotation_lookups_.pop_back();
    }
  });
}

inline auto Printer::WithVars(absl::Span<const Sub> vars) {
  return WithDefs(vars, /*allow_callbacks=*/false);
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


#endif  // GOOGLE_PROTOBUF_IO_PRINTER_H__

