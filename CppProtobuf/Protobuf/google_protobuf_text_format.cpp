// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: jschorr@google.com (Joseph Schorr)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include "google_protobuf_text_format.hpp"

#include <float.h>
#include <stdio.h>

#include <algorithm>
#include <atomic>
#include <climits>
#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <CppAbseil/absl_container_btree_set.hpp>
#include <CppAbseil/absl_strings_ascii.hpp>
#include <CppAbseil/absl_strings_cord.hpp>
#include <CppAbseil/absl_strings_escaping.hpp>
#include <CppAbseil/absl_strings_numbers.hpp>
#include <CppAbseil/absl_strings_str_cat.hpp>
#include <CppAbseil/absl_strings_str_join.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include "google_protobuf_any.hpp"
#include "google_protobuf_descriptor.hpp"
#include "google_protobuf_descriptor.pb.hpp"
#include "google_protobuf_dynamic_message.hpp"
#include "google_protobuf_io_coded_stream.hpp"
#include "google_protobuf_io_strtod.hpp"
#include "google_protobuf_io_tokenizer.hpp"
#include "google_protobuf_io_zero_copy_stream.hpp"
#include "google_protobuf_io_zero_copy_stream_impl.hpp"
#include "google_protobuf_io_zero_copy_stream_impl_lite.hpp"
#include "google_protobuf_map_field.hpp"
#include "google_protobuf_message.hpp"
#include "google_protobuf_reflection_mode.hpp"
#include "google_protobuf_repeated_field.hpp"
#include "google_protobuf_unknown_field_set.hpp"
#include "google_protobuf_wire_format_lite.hpp"

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

using internal::FieldReporterLevel;
using internal::ReflectionMode;
using internal::ScopedReflectionMode;

namespace {

const absl::string_view kFieldValueReplacement = "[REDACTED]";

inline bool IsHexNumber(const std::string& str) {
  return (str.length() >= 2 && str[0] == '0' &&
          (str[1] == 'x' || str[1] == 'X'));
}

inline bool IsOctNumber(const std::string& str) {
  return (str.length() >= 2 && str[0] == '0' &&
          (str[1] >= '0' && str[1] < '8'));
}

// The number of fields that are redacted in AbslStringify.
std::atomic<int64_t> num_redacted_field{0};

inline void IncrementRedactedFieldCounter() {
  num_redacted_field.fetch_add(1, std::memory_order_relaxed);
}

inline void TrimTrailingSpace(std::string& debug_string) {
  // Single line mode currently might have an extra space at the end.
  if (!debug_string.empty() && debug_string.back() == ' ') {
    debug_string.pop_back();
  }
}

}  // namespace

namespace internal {
const char kDebugStringSilentMarker[] = "";
const char kDebugStringSilentMarkerForDetection[] = "\t ";

// Controls insertion of kDebugStringSilentMarker into DebugString() output.
PROTOBUF_EXPORT std::atomic<bool> enable_debug_text_format_marker;

// Controls insertion of a marker making debug strings non-parseable, and
// redacting annotated fields.
PROTOBUF_EXPORT std::atomic<bool> enable_debug_text_redaction{true};

int64_t GetRedactedFieldCount() {
  return num_redacted_field.load(std::memory_order_relaxed);
}
}  // namespace internal

std::string Message::DebugString() const {
  // Indicate all scoped reflection calls are from DebugString function.
  ScopedReflectionMode scope(ReflectionMode::kDebugString);
  std::string debug_string;

  TextFormat::Printer printer;
  printer.SetExpandAny(true);
  printer.SetInsertSilentMarker(internal::enable_debug_text_format_marker.load(
      std::memory_order_relaxed));
  printer.SetReportSensitiveFields(FieldReporterLevel::kDebugString);

  printer.PrintToString(*this, &debug_string);

  return debug_string;
}

std::string Message::ShortDebugString() const {
  // Indicate all scoped reflection calls are from DebugString function.
  ScopedReflectionMode scope(ReflectionMode::kDebugString);
  std::string debug_string;

  TextFormat::Printer printer;
  printer.SetSingleLineMode(true);
  printer.SetExpandAny(true);
  printer.SetInsertSilentMarker(internal::enable_debug_text_format_marker.load(
      std::memory_order_relaxed));
  printer.SetReportSensitiveFields(FieldReporterLevel::kShortDebugString);

  printer.PrintToString(*this, &debug_string);
  TrimTrailingSpace(debug_string);

  return debug_string;
}

std::string Message::Utf8DebugString() const {
  // Indicate all scoped reflection calls are from DebugString function.
  ScopedReflectionMode scope(ReflectionMode::kDebugString);
  std::string debug_string;

  TextFormat::Printer printer;
  printer.SetUseUtf8StringEscaping(true);
  printer.SetExpandAny(true);
  printer.SetInsertSilentMarker(internal::enable_debug_text_format_marker.load(
      std::memory_order_relaxed));
  printer.SetReportSensitiveFields(FieldReporterLevel::kUtf8DebugString);

  printer.PrintToString(*this, &debug_string);

  return debug_string;
}

void Message::PrintDebugString() const { printf("%s", DebugString().c_str()); }

namespace internal {

enum class Option { kNone, kShort, kUTF8 };

std::string StringifyMessage(const Message& message, Option option) {
  // Indicate all scoped reflection calls are from DebugString function.
  ScopedReflectionMode scope(ReflectionMode::kDebugString);

  TextFormat::Printer printer;
  internal::FieldReporterLevel reporter = FieldReporterLevel::kAbslStringify;
  switch (option) {
    case Option::kShort:
      printer.SetSingleLineMode(true);
      reporter = FieldReporterLevel::kShortFormat;
      break;
    case Option::kUTF8:
      printer.SetUseUtf8StringEscaping(true);
      reporter = FieldReporterLevel::kUtf8Format;
      break;
    case Option::kNone:
      break;
  }
  printer.SetExpandAny(true);
  printer.SetRedactDebugString(
      internal::enable_debug_text_redaction.load(std::memory_order_relaxed));
  printer.SetRandomizeDebugString(true);
  printer.SetReportSensitiveFields(reporter);
  std::string result;
  printer.PrintToString(message, &result);

  if (option == Option::kShort) {
    TrimTrailingSpace(result);
  }

  return result;
}

PROTOBUF_EXPORT std::string StringifyMessage(const Message& message) {
  return StringifyMessage(message, Option::kNone);
}

}  // namespace internal

PROTOBUF_EXPORT std::string ShortFormat(const Message& message) {
  return internal::StringifyMessage(message, internal::Option::kShort);
}

PROTOBUF_EXPORT std::string Utf8Format(const Message& message) {
  return internal::StringifyMessage(message, internal::Option::kUTF8);
}


// ===========================================================================
// Implementation of the parse information tree class.
void TextFormat::ParseInfoTree::RecordLocation(
    const FieldDescriptor* field, TextFormat::ParseLocationRange range) {
  locations_[field].push_back(range);
}

TextFormat::ParseInfoTree* TextFormat::ParseInfoTree::CreateNested(
    const FieldDescriptor* field) {
  // Owned by us in the map.
  auto& vec = nested_[field];
  vec.emplace_back(new TextFormat::ParseInfoTree());
  return vec.back().get();
}

void CheckFieldIndex(const FieldDescriptor* field, int index) {
  if (field == nullptr) {
    return;
  }

  if (field->is_repeated() && index == -1) {
    ABSL_DLOG(FATAL) << "Index must be in range of repeated field values. "
                     << "Field: " << field->name();
  } else if (!field->is_repeated() && index != -1) {
    ABSL_DLOG(FATAL) << "Index must be -1 for singular fields."
                     << "Field: " << field->name();
  }
}

TextFormat::ParseLocationRange TextFormat::ParseInfoTree::GetLocationRange(
    const FieldDescriptor* field, int index) const {
  CheckFieldIndex(field, index);
  if (index == -1) {
    index = 0;
  }

  auto it = locations_.find(field);
  if (it == locations_.end() ||
      index >= static_cast<int64_t>(it->second.size())) {
    return TextFormat::ParseLocationRange();
  }

  return it->second[static_cast<size_t>(index)];
}

TextFormat::ParseInfoTree* TextFormat::ParseInfoTree::GetTreeForNested(
    const FieldDescriptor* field, int index) const {
  CheckFieldIndex(field, index);
  if (index == -1) {
    index = 0;
  }

  auto it = nested_.find(field);
  if (it == nested_.end() || index >= static_cast<int64_t>(it->second.size())) {
    return nullptr;
  }

  return it->second[static_cast<size_t>(index)].get();
}

namespace {
// These functions implement the behavior of the "default" TextFormat::Finder,
// they are defined as standalone to be called when finder_ is nullptr.
const FieldDescriptor* DefaultFinderFindExtension(Message* message,
                                                  const std::string& name) {
  const Descriptor* descriptor = message->GetDescriptor();
  return descriptor->file()->pool()->FindExtensionByPrintableName(descriptor,
                                                                  name);
}

const FieldDescriptor* DefaultFinderFindExtensionByNumber(
    const Descriptor* descriptor, int number) {
  return descriptor->file()->pool()->FindExtensionByNumber(descriptor, number);
}

const Descriptor* DefaultFinderFindAnyType(const Message& message,
                                           const std::string& prefix,
                                           const std::string& name) {
  if (prefix != internal::kTypeGoogleApisComPrefix &&
      prefix != internal::kTypeGoogleProdComPrefix) {
    return nullptr;
  }
  return message.GetDescriptor()->file()->pool()->FindMessageTypeByName(name);
}
}  // namespace

// ===========================================================================
// Internal class for parsing an ASCII representation of a Protocol Message.
// This class makes use of the Protocol Message compiler's tokenizer found
// in //third_party/protobuf/io/tokenizer.h. Note that class's Parse
// method is *not* thread-safe and should only be used in a single thread at
// a time.

// Makes code slightly more readable.  The meaning of "DO(foo)" is
// "Execute foo and fail if it fails.", where failure is indicated by
// returning false. Borrowed from parser.cc (Thanks Kenton!).
#define DO(STATEMENT) \
  if (STATEMENT) {    \
  } else {            \
    return false;     \
  }

class TextFormat::Parser::ParserImpl {
 public:
  // Determines if repeated values for non-repeated fields and
  // oneofs are permitted, e.g., the string "foo: 1 foo: 2" for a
  // required/optional field named "foo", or "baz: 1 bar: 2"
  // where "baz" and "bar" are members of the same oneof.
  enum SingularOverwritePolicy {
    ALLOW_SINGULAR_OVERWRITES = 0,   // the last value is retained
    FORBID_SINGULAR_OVERWRITES = 1,  // an error is issued
  };

  ParserImpl(const Descriptor* root_message_type,
             io::ZeroCopyInputStream* input_stream,
             io::ErrorCollector* error_collector,
             const TextFormat::Finder* finder, ParseInfoTree* parse_info_tree,
             SingularOverwritePolicy singular_overwrite_policy,
             bool allow_case_insensitive_field, bool allow_unknown_field,
             bool allow_unknown_extension, bool allow_unknown_enum,
             bool allow_field_number, bool allow_relaxed_whitespace,
             bool allow_partial, int recursion_limit,
             bool error_on_no_op_fields)
      : error_collector_(error_collector),
        finder_(finder),
        parse_info_tree_(parse_info_tree),
        tokenizer_error_collector_(this),
        tokenizer_(input_stream, &tokenizer_error_collector_),
        root_message_type_(root_message_type),
        singular_overwrite_policy_(singular_overwrite_policy),
        allow_case_insensitive_field_(allow_case_insensitive_field),
        allow_unknown_field_(allow_unknown_field),
        allow_unknown_extension_(allow_unknown_extension),
        allow_unknown_enum_(allow_unknown_enum),
        allow_field_number_(allow_field_number),
        allow_partial_(allow_partial),
        initial_recursion_limit_(recursion_limit),
        recursion_limit_(recursion_limit),
        had_silent_marker_(false),
        had_errors_(false),
        error_on_no_op_fields_(error_on_no_op_fields) {
    // For backwards-compatibility with proto1, we need to allow the 'f' suffix
    // for floats.
    tokenizer_.set_allow_f_after_float(true);

    // '#' starts a comment.
    tokenizer_.set_comment_style(io::Tokenizer::SH_COMMENT_STYLE);

    if (allow_relaxed_whitespace) {
      tokenizer_.set_require_space_after_number(false);
      tokenizer_.set_allow_multiline_strings(true);
    }

    // Consume the starting token.
    tokenizer_.Next();
  }
  ParserImpl(const ParserImpl&) = delete;
  ParserImpl& operator=(const ParserImpl&) = delete;
  ~ParserImpl() {}

  // Parses the ASCII representation specified in input and saves the
  // information into the output pointer (a Message). Returns
  // false if an error occurs (an error will also be logged to
  // ABSL_LOG(ERROR)).
  bool Parse(Message* output) {
    // Consume fields until we cannot do so anymore.
    while (true) {
      if (LookingAtType(io::Tokenizer::TYPE_END)) {
        // Ensures recursion limit properly unwinded, but only for success
        // cases. This implicitly avoids the check when `Parse` returns false
        // via `DO(...)`.
        ABSL_DCHECK(had_errors_ || recursion_limit_ == initial_recursion_limit_)
            << "Recursion limit at end of parse should be "
            << initial_recursion_limit_ << ", but was " << recursion_limit_
            << ". Difference of " << initial_recursion_limit_ - recursion_limit_
            << " stack frames not accounted for stack unwind.";

        return !had_errors_;
      }

      DO(ConsumeField(output));
    }
  }

  bool ParseField(const FieldDescriptor* field, Message* output) {
    bool suc;
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      suc = ConsumeFieldMessage(output, output->GetReflection(), field);
    } else {
      suc = ConsumeFieldValue(output, output->GetReflection(), field);
    }
    return suc && LookingAtType(io::Tokenizer::TYPE_END);
  }

  void ReportError(int line, int col, absl::string_view message) {
    had_errors_ = true;
    if (error_collector_ == nullptr) {
      if (line >= 0) {
        ABSL_LOG(ERROR) << "Error parsing text-format "
                        << root_message_type_->full_name() << ": " << (line + 1)
                        << ":" << (col + 1) << ": " << message;
      } else {
        ABSL_LOG(ERROR) << "Error parsing text-format "
                        << root_message_type_->full_name() << ": " << message;
      }
    } else {
      error_collector_->RecordError(line, col, message);
    }
  }

  void ReportWarning(int line, int col, const absl::string_view message) {
    if (error_collector_ == nullptr) {
      if (line >= 0) {
        ABSL_LOG_EVERY_POW_2(WARNING)
            << "Warning parsing text-format " << root_message_type_->full_name()
            << ": " << (line + 1) << ":" << (col + 1) << " (N = " << COUNTER
            << "): " << message;
      } else {
        ABSL_LOG_EVERY_POW_2(WARNING)
            << "Warning parsing text-format " << root_message_type_->full_name()
            << " (N = " << COUNTER << "): " << message;
      }
    } else {
      error_collector_->RecordWarning(line, col, message);
    }
  }

 private:
  static constexpr int32_t kint32max = std::numeric_limits<int32_t>::max();
  static constexpr uint32_t kuint32max = std::numeric_limits<uint32_t>::max();
  static constexpr int64_t kint64min = std::numeric_limits<int64_t>::min();
  static constexpr int64_t kint64max = std::numeric_limits<int64_t>::max();
  static constexpr uint64_t kuint64max = std::numeric_limits<uint64_t>::max();

  // Reports an error with the given message with information indicating
  // the position (as derived from the current token).
  void ReportError(absl::string_view message) {
    ReportError(tokenizer_.current().line, tokenizer_.current().column,
                message);
  }

  // Reports a warning with the given message with information indicating
  // the position (as derived from the current token).
  void ReportWarning(absl::string_view message) {
    ReportWarning(tokenizer_.current().line, tokenizer_.current().column,
                  message);
  }

  // Consumes the specified message with the given starting delimiter.
  // This method checks to see that the end delimiter at the conclusion of
  // the consumption matches the starting delimiter passed in here.
  bool ConsumeMessage(Message* message, const std::string delimiter) {
    while (!LookingAt(">") && !LookingAt("}")) {
      DO(ConsumeField(message));
    }

    // Confirm that we have a valid ending delimiter.
    DO(Consume(delimiter));
    return true;
  }

  // Consume either "<" or "{".
  bool ConsumeMessageDelimiter(std::string* delimiter) {
    if (TryConsume("<")) {
      *delimiter = ">";
    } else {
      DO(Consume("{"));
      *delimiter = "}";
    }
    return true;
  }


  // Consumes the current field (as returned by the tokenizer) on the
  // passed in message.
  bool ConsumeField(Message* message) {
    const Reflection* reflection = message->GetReflection();
    const Descriptor* descriptor = message->GetDescriptor();

    std::string field_name;
    bool reserved_field = false;
    const FieldDescriptor* field = nullptr;
    int start_line = tokenizer_.current().line;
    int start_column = tokenizer_.current().column;

    const FieldDescriptor* any_type_url_field;
    const FieldDescriptor* any_value_field;
    if (internal::GetAnyFieldDescriptors(*message, &any_type_url_field,
                                         &any_value_field) &&
        TryConsume("[")) {
      std::string full_type_name, prefix;
      DO(ConsumeAnyTypeUrl(&full_type_name, &prefix));
      std::string prefix_and_full_type_name =
          absl::StrCat(prefix, full_type_name);
      DO(ConsumeBeforeWhitespace("]"));
      TryConsumeWhitespace();
      // ':' is optional between message labels and values.
      if (TryConsumeBeforeWhitespace(":")) {
        TryConsumeWhitespace();
      }
      std::string serialized_value;
      const Descriptor* value_descriptor =
          finder_ ? finder_->FindAnyType(*message, prefix, full_type_name)
                  : DefaultFinderFindAnyType(*message, prefix, full_type_name);
      if (value_descriptor == nullptr) {
        ReportError(absl::StrCat("Could not find type \"",
                                 prefix_and_full_type_name,
                                 "\" stored in google.protobuf.Any."));
        return false;
      }
      DO(ConsumeAnyValue(value_descriptor, &serialized_value));
      if (singular_overwrite_policy_ == FORBID_SINGULAR_OVERWRITES) {
        // Fail if any_type_url_field has already been specified.
        if ((!any_type_url_field->is_repeated() &&
             reflection->HasField(*message, any_type_url_field)) ||
            (!any_value_field->is_repeated() &&
             reflection->HasField(*message, any_value_field))) {
          ReportError("Non-repeated Any specified multiple times.");
          return false;
        }
      }
      reflection->SetString(message, any_type_url_field,
                            std::move(prefix_and_full_type_name));
      reflection->SetString(message, any_value_field,
                            std::move(serialized_value));
      return true;
    }
    if (TryConsume("[")) {
      // Extension.
      DO(ConsumeFullTypeName(&field_name));
      DO(ConsumeBeforeWhitespace("]"));
      TryConsumeWhitespace();

      field = finder_ ? finder_->FindExtension(message, field_name)
                      : DefaultFinderFindExtension(message, field_name);

      if (field == nullptr) {
        if (!allow_unknown_field_ && !allow_unknown_extension_) {
          ReportError(absl::StrCat("Extension \"", field_name,
                                   "\" is not defined or "
                                   "is not an extension of \"",
                                   descriptor->full_name(), "\"."));
          return false;
        } else {
          ReportWarning(absl::StrCat(
              "Ignoring extension \"", field_name,
              "\" which is not defined or is not an extension of \"",
              descriptor->full_name(), "\"."));
        }
      }
    } else {
      DO(ConsumeIdentifierBeforeWhitespace(&field_name));
      TryConsumeWhitespace();

      int32_t field_number;
      if (allow_field_number_ && absl::SimpleAtoi(field_name, &field_number)) {
        if (descriptor->IsExtensionNumber(field_number)) {
          field = finder_
                      ? finder_->FindExtensionByNumber(descriptor, field_number)
                      : DefaultFinderFindExtensionByNumber(descriptor,
                                                           field_number);
        } else if (descriptor->IsReservedNumber(field_number)) {
          reserved_field = true;
        } else {
          field = descriptor->FindFieldByNumber(field_number);
        }
      } else {
        field = descriptor->FindFieldByName(field_name);
        // Group names are expected to be capitalized as they appear in the
        // .proto file, which actually matches their type names, not their
        // field names.
        if (field == nullptr) {
          std::string lower_field_name = field_name;
          absl::AsciiStrToLower(&lower_field_name);
          field = descriptor->FindFieldByName(lower_field_name);
          // If the case-insensitive match worked but the field is NOT a group,
          if (field != nullptr &&
              field->type() != FieldDescriptor::TYPE_GROUP) {
            field = nullptr;
          }
        }
        // Again, special-case group names as described above.
        if (field != nullptr && field->type() == FieldDescriptor::TYPE_GROUP &&
            field->message_type()->name() != field_name) {
          field = nullptr;
        }

        if (field == nullptr && allow_case_insensitive_field_) {
          std::string lower_field_name = field_name;
          absl::AsciiStrToLower(&lower_field_name);
          field = descriptor->FindFieldByLowercaseName(lower_field_name);
        }

        if (field == nullptr) {
          reserved_field = descriptor->IsReservedName(field_name);
        }
      }

      if (field == nullptr && !reserved_field) {
        if (!allow_unknown_field_) {
          ReportError(absl::StrCat("Message type \"", descriptor->full_name(),
                                   "\" has no field named \"", field_name,
                                   "\"."));
          return false;
        } else {
          ReportWarning(absl::StrCat("Message type \"", descriptor->full_name(),
                                     "\" has no field named \"", field_name,
                                     "\"."));
        }
      }
    }

    // Skips unknown or reserved fields.
    if (field == nullptr) {
      ABSL_CHECK(allow_unknown_field_ || allow_unknown_extension_ ||
                 reserved_field);

      // Try to guess the type of this field.
      // If this field is not a message, there should be a ":" between the
      // field name and the field value and also the field value should not
      // start with "{" or "<" which indicates the beginning of a message body.
      // If there is no ":" or there is a "{" or "<" after ":", this field has
      // to be a message or the input is ill-formed.
      if (TryConsumeBeforeWhitespace(":")) {
        TryConsumeWhitespace();
        if (!LookingAt("{") && !LookingAt("<")) {
          return SkipFieldValue();
        }
      }
      return SkipFieldMessage();
    }

    if (field->options().deprecated()) {
      ReportWarning(absl::StrCat("text format contains deprecated field \"",
                                 field_name, "\""));
    }

    if (singular_overwrite_policy_ == FORBID_SINGULAR_OVERWRITES) {
      // Fail if the field is not repeated and it has already been specified.
      if (!field->is_repeated() && reflection->HasField(*message, field)) {
        ReportError(absl::StrCat("Non-repeated field \"", field_name,
                                 "\" is specified multiple times."));
        return false;
      }
      // Fail if the field is a member of a oneof and another member has already
      // been specified.
      const OneofDescriptor* oneof = field->containing_oneof();
      if (oneof != nullptr && reflection->HasOneof(*message, oneof)) {
        const FieldDescriptor* other_field =
            reflection->GetOneofFieldDescriptor(*message, oneof);
        ReportError(absl::StrCat("Field \"", field_name,
                                 "\" is specified along with "
                                 "field \"",
                                 other_field->name(),
                                 "\", another member "
                                 "of oneof \"",
                                 oneof->name(), "\"."));
        return false;
      }
    }

    // Perform special handling for embedded message types.
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      // ':' is optional here.
      bool consumed_semicolon = TryConsumeBeforeWhitespace(":");
      if (consumed_semicolon) {
        TryConsumeWhitespace();
      }
      if (consumed_semicolon && field->options().weak() &&
          LookingAtType(io::Tokenizer::TYPE_STRING)) {
        // we are getting a bytes string for a weak field.
        std::string tmp;
        DO(ConsumeString(&tmp));
        MessageFactory* factory =
            finder_ ? finder_->FindExtensionFactory(field) : nullptr;
        reflection->MutableMessage(message, field, factory)
            ->ParseFromString(tmp);
        goto label_skip_parsing;
      }
    } else {
      // ':' is required here.
      DO(ConsumeBeforeWhitespace(":"));
      TryConsumeWhitespace();
    }

    if (field->is_repeated() && TryConsume("[")) {
      // Short repeated format, e.g.  "foo: [1, 2, 3]".
      if (!TryConsume("]")) {
        // "foo: []" is treated as empty.
        while (true) {
          if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
            // Perform special handling for embedded message types.
            DO(ConsumeFieldMessage(message, reflection, field));
          } else {
            DO(ConsumeFieldValue(message, reflection, field));
          }
          if (TryConsume("]")) {
            break;
          }
          DO(Consume(","));
        }
      }
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      DO(ConsumeFieldMessage(message, reflection, field));
    } else {
      DO(ConsumeFieldValue(message, reflection, field));
    }
  label_skip_parsing:
    // For historical reasons, fields may optionally be separated by commas or
    // semicolons.
    TryConsume(";") || TryConsume(",");

    // If a parse info tree exists, add the location for the parsed
    // field.
    if (parse_info_tree_ != nullptr) {
      int end_line = tokenizer_.previous().line;
      int end_column = tokenizer_.previous().end_column;

      RecordLocation(parse_info_tree_, field,
                     ParseLocationRange(ParseLocation(start_line, start_column),
                                        ParseLocation(end_line, end_column)));
    }

    return true;
  }

  // Skips the next field including the field's name and value.
  bool SkipField() {
    std::string field_name;
    if (TryConsume("[")) {
      // Extension name or type URL.
      DO(ConsumeTypeUrlOrFullTypeName(&field_name));
      DO(ConsumeBeforeWhitespace("]"));
    } else {
      DO(ConsumeIdentifierBeforeWhitespace(&field_name));
    }
    TryConsumeWhitespace();

    // Try to guess the type of this field.
    // If this field is not a message, there should be a ":" between the
    // field name and the field value and also the field value should not
    // start with "{" or "<" which indicates the beginning of a message body.
    // If there is no ":" or there is a "{" or "<" after ":", this field has
    // to be a message or the input is ill-formed.
    if (TryConsumeBeforeWhitespace(":")) {
      TryConsumeWhitespace();
      if (!LookingAt("{") && !LookingAt("<")) {
        DO(SkipFieldValue());
      } else {
        DO(SkipFieldMessage());
      }
    } else {
      DO(SkipFieldMessage());
    }
    // For historical reasons, fields may optionally be separated by commas or
    // semicolons.
    TryConsume(";") || TryConsume(",");
    return true;
  }

  bool ConsumeFieldMessage(Message* message, const Reflection* reflection,
                           const FieldDescriptor* field) {
    if (--recursion_limit_ < 0) {
      ReportError(
          absl::StrCat("Message is too deep, the parser exceeded the "
                       "configured recursion limit of ",
                       initial_recursion_limit_, "."));
      return false;
    }
    // If the parse information tree is not nullptr, create a nested one
    // for the nested message.
    ParseInfoTree* parent = parse_info_tree_;
    if (parent != nullptr) {
      parse_info_tree_ = CreateNested(parent, field);
    }

    std::string delimiter;
    DO(ConsumeMessageDelimiter(&delimiter));
    MessageFactory* factory =
        finder_ ? finder_->FindExtensionFactory(field) : nullptr;
    if (field->is_repeated()) {
      DO(ConsumeMessage(reflection->AddMessage(message, field, factory),
                        delimiter));
    } else {
      DO(ConsumeMessage(reflection->MutableMessage(message, field, factory),
                        delimiter));
    }

    ++recursion_limit_;

    // Reset the parse information tree.
    parse_info_tree_ = parent;
    return true;
  }

  // Skips the whole body of a message including the beginning delimiter and
  // the ending delimiter.
  bool SkipFieldMessage() {
    if (--recursion_limit_ < 0) {
      ReportError(
          absl::StrCat("Message is too deep, the parser exceeded the "
                       "configured recursion limit of ",
                       initial_recursion_limit_, "."));
      return false;
    }

    std::string delimiter;
    DO(ConsumeMessageDelimiter(&delimiter));
    while (!LookingAt(">") && !LookingAt("}")) {
      DO(SkipField());
    }
    DO(Consume(delimiter));

    ++recursion_limit_;
    return true;
  }

  bool ConsumeFieldValue(Message* message, const Reflection* reflection,
                         const FieldDescriptor* field) {
// Define an easy to use macro for setting fields. This macro checks
// to see if the field is repeated (in which case we need to use the Add
// methods or not (in which case we need to use the Set methods).
// When checking for no-op operations, We verify that both the existing value in
// the message and the new value are the default. If the existing field value is
// not the default, setting it to the default should not be treated as a no-op.
#define SET_FIELD(CPPTYPE, CPPTYPELCASE, VALUE)                   \
  if (field->is_repeated()) {                                     \
    reflection->Add##CPPTYPE(message, field, VALUE);              \
  } else {                                                        \
    if (error_on_no_op_fields_ && !field->has_presence() &&       \
        field->default_value_##CPPTYPELCASE() ==                  \
            reflection->Get##CPPTYPE(*message, field) &&          \
        field->default_value_##CPPTYPELCASE() == VALUE) {         \
      ReportError("Input field " + field->full_name() +           \
                  " did not change resulting proto.");            \
    } else {                                                      \
      reflection->Set##CPPTYPE(message, field, std::move(VALUE)); \
    }                                                             \
  }

    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32: {
        int64_t value;
        DO(ConsumeSignedInteger(&value, kint32max));
        SET_FIELD(Int32, int32, static_cast<int32_t>(value));
        break;
      }

      case FieldDescriptor::CPPTYPE_UINT32: {
        uint64_t value;
        DO(ConsumeUnsignedInteger(&value, kuint32max));
        SET_FIELD(UInt32, uint32, static_cast<uint32_t>(value));
        break;
      }

      case FieldDescriptor::CPPTYPE_INT64: {
        int64_t value;
        DO(ConsumeSignedInteger(&value, kint64max));
        SET_FIELD(Int64, int64, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_UINT64: {
        uint64_t value;
        DO(ConsumeUnsignedInteger(&value, kuint64max));
        SET_FIELD(UInt64, uint64, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_FLOAT: {
        double value;
        DO(ConsumeDouble(&value));
        SET_FIELD(Float, float, io::SafeDoubleToFloat(value));
        break;
      }

      case FieldDescriptor::CPPTYPE_DOUBLE: {
        double value;
        DO(ConsumeDouble(&value));
        SET_FIELD(Double, double, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_STRING: {
        std::string value;
        DO(ConsumeString(&value));
        SET_FIELD(String, string, value);
        break;
      }

      case FieldDescriptor::CPPTYPE_BOOL: {
        if (LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
          uint64_t value;
          DO(ConsumeUnsignedInteger(&value, 1));
          SET_FIELD(Bool, bool, value);
        } else {
          std::string value;
          DO(ConsumeIdentifier(&value));
          if (value == "true" || value == "True" || value == "t") {
            SET_FIELD(Bool, bool, true);
          } else if (value == "false" || value == "False" || value == "f") {
            SET_FIELD(Bool, bool, false);
          } else {
            ReportError(absl::StrCat("Invalid value for boolean field \"",
                                     field->name(), "\". Value: \"", value,
                                     "\"."));
            return false;
          }
        }
        break;
      }

      case FieldDescriptor::CPPTYPE_ENUM: {
        std::string value;
        int64_t int_value = kint64max;
        const EnumDescriptor* enum_type = field->enum_type();
        const EnumValueDescriptor* enum_value = nullptr;

        if (LookingAtType(io::Tokenizer::TYPE_IDENTIFIER)) {
          DO(ConsumeIdentifier(&value));
          // Find the enumeration value.
          enum_value = enum_type->FindValueByName(value);

        } else if (LookingAt("-") ||
                   LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
          DO(ConsumeSignedInteger(&int_value, kint32max));
          value = absl::StrCat(int_value);  // for error reporting
          enum_value = enum_type->FindValueByNumber(int_value);
        } else {
          ReportError(absl::StrCat("Expected integer or identifier, got: ",
                                   tokenizer_.current().text));
          return false;
        }

        if (enum_value == nullptr) {
          if (int_value != kint64max &&
              !field->legacy_enum_field_treated_as_closed()) {
            SET_FIELD(EnumValue, int64, int_value);
            return true;
          } else if (!allow_unknown_enum_) {
            ReportError(absl::StrCat("Unknown enumeration value of \"", value,
                                     "\" for field \"", field->name(), "\"."));
            return false;
          } else {
            ReportWarning(absl::StrCat("Unknown enumeration value of \"", value,
                                       "\" for field \"", field->name(),
                                       "\"."));
            return true;
          }
        }

        SET_FIELD(Enum, enum, enum_value);
        break;
      }

      case FieldDescriptor::CPPTYPE_MESSAGE: {
        // We should never get here. Put here instead of a default
        // so that if new types are added, we get a nice compiler warning.
        ABSL_LOG(FATAL) << "Reached an unintended state: CPPTYPE_MESSAGE";
        break;
      }
    }
#undef SET_FIELD
    return true;
  }

  bool SkipFieldValue() {
    if (--recursion_limit_ < 0) {
      ReportError(
          absl::StrCat("Message is too deep, the parser exceeded the "
                       "configured recursion limit of ",
                       initial_recursion_limit_, "."));
      return false;
    }

    if (LookingAtType(io::Tokenizer::TYPE_STRING)) {
      while (LookingAtType(io::Tokenizer::TYPE_STRING)) {
        tokenizer_.Next();
      }
      ++recursion_limit_;
      return true;
    }
    if (TryConsume("[")) {
      if (!TryConsume("]")) {
        while (true) {
          if (!LookingAt("{") && !LookingAt("<")) {
            DO(SkipFieldValue());
          } else {
            DO(SkipFieldMessage());
          }
          if (TryConsume("]")) {
            break;
          }
          DO(Consume(","));
        }
      }
      ++recursion_limit_;
      return true;
    }
    // Possible field values other than string:
    //   12345        => TYPE_INTEGER
    //   -12345       => TYPE_SYMBOL + TYPE_INTEGER
    //   1.2345       => TYPE_FLOAT
    //   -1.2345      => TYPE_SYMBOL + TYPE_FLOAT
    //   inf          => TYPE_IDENTIFIER
    //   -inf         => TYPE_SYMBOL + TYPE_IDENTIFIER
    //   TYPE_INTEGER => TYPE_IDENTIFIER
    // Divides them into two group, one with TYPE_SYMBOL
    // and the other without:
    //   Group one:
    //     12345        => TYPE_INTEGER
    //     1.2345       => TYPE_FLOAT
    //     inf          => TYPE_IDENTIFIER
    //     TYPE_INTEGER => TYPE_IDENTIFIER
    //   Group two:
    //     -12345       => TYPE_SYMBOL + TYPE_INTEGER
    //     -1.2345      => TYPE_SYMBOL + TYPE_FLOAT
    //     -inf         => TYPE_SYMBOL + TYPE_IDENTIFIER
    // As we can see, the field value consists of an optional '-' and one of
    // TYPE_INTEGER, TYPE_FLOAT and TYPE_IDENTIFIER.
    bool has_minus = TryConsume("-");
    if (!LookingAtType(io::Tokenizer::TYPE_INTEGER) &&
        !LookingAtType(io::Tokenizer::TYPE_FLOAT) &&
        !LookingAtType(io::Tokenizer::TYPE_IDENTIFIER)) {
      std::string text = tokenizer_.current().text;
      ReportError(
          absl::StrCat("Cannot skip field value, unexpected token: ", text));
      ++recursion_limit_;
      return false;
    }
    // Combination of '-' and TYPE_IDENTIFIER may result in an invalid field
    // value while other combinations all generate valid values.
    // We check if the value of this combination is valid here.
    // TYPE_IDENTIFIER after a '-' should be one of the float values listed
    // below:
    //   inf, inff, infinity, nan
    if (has_minus && LookingAtType(io::Tokenizer::TYPE_IDENTIFIER)) {
      std::string text = tokenizer_.current().text;
      absl::AsciiStrToLower(&text);
      if (text != "inf" &&
          text != "infinity" && text != "nan") {
        ReportError(absl::StrCat("Invalid float number: ", text));
        ++recursion_limit_;
        return false;
      }
    }
    tokenizer_.Next();
    ++recursion_limit_;
    return true;
  }

  // Returns true if the current token's text is equal to that specified.
  bool LookingAt(const std::string& text) {
    return tokenizer_.current().text == text;
  }

  // Returns true if the current token's type is equal to that specified.
  bool LookingAtType(io::Tokenizer::TokenType token_type) {
    return tokenizer_.current().type == token_type;
  }

  // Consumes an identifier and saves its value in the identifier parameter.
  // Returns false if the token is not of type IDENTIFIER.
  bool ConsumeIdentifier(std::string* identifier) {
    if (LookingAtType(io::Tokenizer::TYPE_IDENTIFIER)) {
      *identifier = tokenizer_.current().text;
      tokenizer_.Next();
      return true;
    }

    // If allow_field_numer_ or allow_unknown_field_ is true, we should able
    // to parse integer identifiers.
    if ((allow_field_number_ || allow_unknown_field_ ||
         allow_unknown_extension_) &&
        LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
      *identifier = tokenizer_.current().text;
      tokenizer_.Next();
      return true;
    }

    ReportError(
        absl::StrCat("Expected identifier, got: ", tokenizer_.current().text));
    return false;
  }

  // Similar to `ConsumeIdentifier`, but any following whitespace token may
  // be reported.
  bool ConsumeIdentifierBeforeWhitespace(std::string* identifier) {
    tokenizer_.set_report_whitespace(true);
    bool result = ConsumeIdentifier(identifier);
    tokenizer_.set_report_whitespace(false);
    return result;
  }

  // Consume a string of form "<id1>.<id2>....<idN>".
  bool ConsumeFullTypeName(std::string* name) {
    DO(ConsumeIdentifier(name));
    while (TryConsume(".")) {
      std::string part;
      DO(ConsumeIdentifier(&part));
      absl::StrAppend(name, ".", part);
    }
    return true;
  }

  bool ConsumeTypeUrlOrFullTypeName(std::string* name) {
    DO(ConsumeIdentifier(name));
    while (true) {
      std::string connector;
      if (TryConsume(".")) {
        connector = ".";
      } else if (TryConsume("/")) {
        connector = "/";
      } else {
        break;
      }
      std::string part;
      DO(ConsumeIdentifier(&part));
      *name += connector;
      *name += part;
    }
    return true;
  }

  // Consumes a string and saves its value in the text parameter.
  // Returns false if the token is not of type STRING.
  bool ConsumeString(std::string* text) {
    if (!LookingAtType(io::Tokenizer::TYPE_STRING)) {
      ReportError(
          absl::StrCat("Expected string, got: ", tokenizer_.current().text));
      return false;
    }

    text->clear();
    while (LookingAtType(io::Tokenizer::TYPE_STRING)) {
      io::Tokenizer::ParseStringAppend(tokenizer_.current().text, text);

      tokenizer_.Next();
    }

    return true;
  }

  // Consumes a uint64_t and saves its value in the value parameter.
  // Returns false if the token is not of type INTEGER.
  bool ConsumeUnsignedInteger(uint64_t* value, uint64_t max_value) {
    if (!LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
      ReportError(
          absl::StrCat("Expected integer, got: ", tokenizer_.current().text));
      return false;
    }

    if (!io::Tokenizer::ParseInteger(tokenizer_.current().text, max_value,
                                     value)) {
      ReportError(absl::StrCat("Integer out of range (",
                               tokenizer_.current().text, ")"));
      return false;
    }

    tokenizer_.Next();
    return true;
  }

  // Consumes an int64_t and saves its value in the value parameter.
  // Note that since the tokenizer does not support negative numbers,
  // we actually may consume an additional token (for the minus sign) in this
  // method. Returns false if the token is not an integer
  // (signed or otherwise).
  bool ConsumeSignedInteger(int64_t* value, uint64_t max_value) {
    bool negative = false;

    if (TryConsume("-")) {
      negative = true;
      // Two's complement always allows one more negative integer than
      // positive.
      ++max_value;
    }

    uint64_t unsigned_value;

    DO(ConsumeUnsignedInteger(&unsigned_value, max_value));

    if (negative) {
      if ((static_cast<uint64_t>(kint64max) + 1) == unsigned_value) {
        *value = kint64min;
      } else {
        *value = -static_cast<int64_t>(unsigned_value);
      }
    } else {
      *value = static_cast<int64_t>(unsigned_value);
    }

    return true;
  }

  // Consumes a double and saves its value in the value parameter.
  // Accepts decimal numbers only, rejects hex or oct numbers.
  bool ConsumeUnsignedDecimalAsDouble(double* value, uint64_t max_value) {
    if (!LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
      ReportError(
          absl::StrCat("Expected integer, got: ", tokenizer_.current().text));
      return false;
    }

    const std::string& text = tokenizer_.current().text;
    if (IsHexNumber(text) || IsOctNumber(text)) {
      ReportError(absl::StrCat("Expect a decimal number, got: ", text));
      return false;
    }

    uint64_t uint64_value;
    if (io::Tokenizer::ParseInteger(text, max_value, &uint64_value)) {
      *value = static_cast<double>(uint64_value);
    } else {
      // Uint64 overflow, attempt to parse as a double instead.
      *value = io::Tokenizer::ParseFloat(text);
    }

    tokenizer_.Next();
    return true;
  }

  // Consumes a double and saves its value in the value parameter.
  // Note that since the tokenizer does not support negative numbers,
  // we actually may consume an additional token (for the minus sign) in this
  // method. Returns false if the token is not a double
  // (signed or otherwise).
  bool ConsumeDouble(double* value) {
    bool negative = false;

    if (TryConsume("-")) {
      negative = true;
    }

    // A double can actually be an integer, according to the tokenizer.
    // Therefore, we must check both cases here.
    if (LookingAtType(io::Tokenizer::TYPE_INTEGER)) {
      // We have found an integer value for the double.
      DO(ConsumeUnsignedDecimalAsDouble(value, kuint64max));
    } else if (LookingAtType(io::Tokenizer::TYPE_FLOAT)) {
      // We have found a float value for the double.
      *value = io::Tokenizer::ParseFloat(tokenizer_.current().text);

      // Mark the current token as consumed.
      tokenizer_.Next();
    } else if (LookingAtType(io::Tokenizer::TYPE_IDENTIFIER)) {
      std::string text = tokenizer_.current().text;
      absl::AsciiStrToLower(&text);
      if (text == "inf" ||
          text == "infinity") {
        *value = std::numeric_limits<double>::infinity();
        tokenizer_.Next();
      } else if (text == "nan") {
        *value = std::numeric_limits<double>::quiet_NaN();
        tokenizer_.Next();
      } else {
        ReportError(absl::StrCat("Expected double, got: ", text));
        return false;
      }
    } else {
      ReportError(
          absl::StrCat("Expected double, got: ", tokenizer_.current().text));
      return false;
    }

    if (negative) {
      *value = -*value;
    }

    return true;
  }

  // Consumes Any::type_url value, of form "type.googleapis.com/full.type.Name"
  // or "type.googleprod.com/full.type.Name"
  bool ConsumeAnyTypeUrl(std::string* full_type_name, std::string* prefix) {
    // TODO Extend Consume() to consume multiple tokens at once, so that
    // this code can be written as just DO(Consume(kGoogleApisTypePrefix)).
    DO(ConsumeIdentifier(prefix));
    while (TryConsume(".")) {
      std::string url;
      DO(ConsumeIdentifier(&url));
      absl::StrAppend(prefix, ".", url);
    }
    DO(Consume("/"));
    absl::StrAppend(prefix, "/");
    DO(ConsumeFullTypeName(full_type_name));

    return true;
  }

  // A helper function for reconstructing Any::value. Consumes a text of
  // full_type_name, then serializes it into serialized_value.
  bool ConsumeAnyValue(const Descriptor* value_descriptor,
                       std::string* serialized_value) {
    DynamicMessageFactory factory;
    const Message* value_prototype = factory.GetPrototype(value_descriptor);
    if (value_prototype == nullptr) {
      return false;
    }
    std::unique_ptr<Message> value(value_prototype->New());
    std::string sub_delimiter;
    DO(ConsumeMessageDelimiter(&sub_delimiter));
    DO(ConsumeMessage(value.get(), sub_delimiter));

    if (allow_partial_) {
      value->AppendPartialToString(serialized_value);
    } else {
      if (!value->IsInitialized()) {
        ReportError(absl::StrCat(
            "Value of type \"", value_descriptor->full_name(),
            "\" stored in google.protobuf.Any has missing required fields"));
        return false;
      }
      value->AppendToString(serialized_value);
    }
    return true;
  }

  // Consumes a token and confirms that it matches that specified in the
  // value parameter. Returns false if the token found does not match that
  // which was specified.
  bool Consume(const std::string& value) {
    const std::string& current_value = tokenizer_.current().text;

    if (current_value != value) {
      ReportError(absl::StrCat("Expected \"", value, "\", found \"",
                               current_value, "\"."));
      return false;
    }

    tokenizer_.Next();

    return true;
  }

  // Similar to `Consume`, but the following token may be tokenized as
  // TYPE_WHITESPACE.
  bool ConsumeBeforeWhitespace(const std::string& value) {
    // Report whitespace after this token, but only once.
    tokenizer_.set_report_whitespace(true);
    bool result = Consume(value);
    tokenizer_.set_report_whitespace(false);
    return result;
  }

  // Attempts to consume the supplied value. Returns false if a the
  // token found does not match the value specified.
  bool TryConsume(const std::string& value) {
    if (tokenizer_.current().text == value) {
      tokenizer_.Next();
      return true;
    } else {
      return false;
    }
  }

  // Similar to `TryConsume`, but the following token may be tokenized as
  // TYPE_WHITESPACE.
  bool TryConsumeBeforeWhitespace(const std::string& value) {
    // Report whitespace after this token, but only once.
    tokenizer_.set_report_whitespace(true);
    bool result = TryConsume(value);
    tokenizer_.set_report_whitespace(false);
    return result;
  }

  bool TryConsumeWhitespace() {
    had_silent_marker_ = false;
    if (LookingAtType(io::Tokenizer::TYPE_WHITESPACE)) {
      if (tokenizer_.current().text ==
          absl::StrCat(" ", internal::kDebugStringSilentMarkerForDetection)) {
        had_silent_marker_ = true;
      }
      tokenizer_.Next();
      return true;
    }
    return false;
  }

  // An internal instance of the Tokenizer's error collector, used to
  // collect any base-level parse errors and feed them to the ParserImpl.
  class ParserErrorCollector : public io::ErrorCollector {
   public:
    explicit ParserErrorCollector(TextFormat::Parser::ParserImpl* parser)
        : parser_(parser) {}

    ParserErrorCollector(const ParserErrorCollector&) = delete;
    ParserErrorCollector& operator=(const ParserErrorCollector&) = delete;
    ~ParserErrorCollector() override {}

    void RecordError(int line, int column, absl::string_view message) override {
      parser_->ReportError(line, column, message);
    }

    void RecordWarning(int line, int column,
                       absl::string_view message) override {
      parser_->ReportWarning(line, column, message);
    }

   private:
    TextFormat::Parser::ParserImpl* parser_;
  };

  io::ErrorCollector* error_collector_;
  const TextFormat::Finder* finder_;
  ParseInfoTree* parse_info_tree_;
  ParserErrorCollector tokenizer_error_collector_;
  io::Tokenizer tokenizer_;
  const Descriptor* root_message_type_;
  SingularOverwritePolicy singular_overwrite_policy_;
  const bool allow_case_insensitive_field_;
  const bool allow_unknown_field_;
  const bool allow_unknown_extension_;
  const bool allow_unknown_enum_;
  const bool allow_field_number_;
  const bool allow_partial_;
  const int initial_recursion_limit_;
  int recursion_limit_;
  bool had_silent_marker_;
  bool had_errors_;
  bool error_on_no_op_fields_;

};

// ===========================================================================
// Internal class for writing text to the io::ZeroCopyOutputStream. Adapted
// from the Printer found in //third_party/protobuf/io/printer.h
class TextFormat::Printer::TextGenerator
    : public TextFormat::BaseTextGenerator {
 public:
  explicit TextGenerator(io::ZeroCopyOutputStream* output,
                         int initial_indent_level)
      : output_(output),
        buffer_(nullptr),
        buffer_size_(0),
        at_start_of_line_(true),
        failed_(false),
        insert_silent_marker_(false),
        indent_level_(initial_indent_level),
        initial_indent_level_(initial_indent_level) {}

  explicit TextGenerator(io::ZeroCopyOutputStream* output,
                         bool insert_silent_marker, int initial_indent_level)
      : output_(output),
        buffer_(nullptr),
        buffer_size_(0),
        at_start_of_line_(true),
        failed_(false),
        insert_silent_marker_(insert_silent_marker),
        indent_level_(initial_indent_level),
        initial_indent_level_(initial_indent_level) {}

  TextGenerator(const TextGenerator&) = delete;
  TextGenerator& operator=(const TextGenerator&) = delete;
  ~TextGenerator() override {
    // Only BackUp() if we're sure we've successfully called Next() at least
    // once.
    if (!failed_) {
      output_->BackUp(buffer_size_);
    }
  }

  // Indent text by two spaces.  After calling Indent(), two spaces will be
  // inserted at the beginning of each line of text.  Indent() may be called
  // multiple times to produce deeper indents.
  void Indent() override { ++indent_level_; }

  // Reduces the current indent level by two spaces, or crashes if the indent
  // level is zero.
  void Outdent() override {
    if (indent_level_ == 0 || indent_level_ < initial_indent_level_) {
      ABSL_DLOG(FATAL) << " Outdent() without matching Indent().";
      return;
    }

    --indent_level_;
  }

  size_t GetCurrentIndentationSize() const override {
    return 2 * indent_level_;
  }

  // Print text to the output stream.
  void Print(const char* text, size_t size) override {
    if (indent_level_ > 0) {
      size_t pos = 0;  // The number of bytes we've written so far.
      for (size_t i = 0; i < size; i++) {
        if (text[i] == '\n') {
          // Saw newline.  If there is more text, we may need to insert an
          // indent here.  So, write what we have so far, including the '\n'.
          Write(text + pos, i - pos + 1);
          pos = i + 1;

          // Setting this true will cause the next Write() to insert an indent
          // first.
          at_start_of_line_ = true;
        }
      }
      // Write the rest.
      Write(text + pos, size - pos);
    } else {
      Write(text, size);
      if (size > 0 && text[size - 1] == '\n') {
        at_start_of_line_ = true;
      }
    }
  }

  // True if any write to the underlying stream failed.  (We don't just
  // crash in this case because this is an I/O failure, not a programming
  // error.)
  bool failed() const { return failed_; }

  void PrintMaybeWithMarker(MarkerToken, absl::string_view text) override {
    Print(text.data(), text.size());
    if (ConsumeInsertSilentMarker()) {
      PrintLiteral(internal::kDebugStringSilentMarker);
    }
  }

  void PrintMaybeWithMarker(MarkerToken, absl::string_view text_head,
                            absl::string_view text_tail) override {
    Print(text_head.data(), text_head.size());
    if (ConsumeInsertSilentMarker()) {
      PrintLiteral(internal::kDebugStringSilentMarker);
    }
    Print(text_tail.data(), text_tail.size());
  }

 private:
  void Write(const char* data, size_t size) {
    if (failed_) return;
    if (size == 0) return;

    if (at_start_of_line_) {
      // Insert an indent.
      at_start_of_line_ = false;
      WriteIndent();
      if (failed_) return;
    }

    while (static_cast<int64_t>(size) > buffer_size_) {
      // Data exceeds space in the buffer.  Copy what we can and request a
      // new buffer.
      if (buffer_size_ > 0) {
        memcpy(buffer_, data, buffer_size_);
        data += buffer_size_;
        size -= buffer_size_;
      }
      void* void_buffer = nullptr;
      failed_ = !output_->Next(&void_buffer, &buffer_size_);
      if (failed_) return;
      buffer_ = reinterpret_cast<char*>(void_buffer);
    }

    // Buffer is big enough to receive the data; copy it.
    memcpy(buffer_, data, size);
    buffer_ += size;
    buffer_size_ -= size;
  }

  void WriteIndent() {
    if (indent_level_ == 0) {
      return;
    }
    ABSL_DCHECK(!failed_);
    int size = GetCurrentIndentationSize();

    while (size > buffer_size_) {
      // Data exceeds space in the buffer. Write what we can and request a new
      // buffer.
      if (buffer_size_ > 0) {
        memset(buffer_, ' ', buffer_size_);
      }
      size -= buffer_size_;
      void* void_buffer;
      failed_ = !output_->Next(&void_buffer, &buffer_size_);
      if (failed_) return;
      buffer_ = reinterpret_cast<char*>(void_buffer);
    }

    // Buffer is big enough to receive the data; copy it.
    memset(buffer_, ' ', size);
    buffer_ += size;
    buffer_size_ -= size;
  }

  // Return the current value of insert_silent_marker_. If it is true, set it
  // to false as we assume that a silent marker is inserted after a call to this
  // function.
  bool ConsumeInsertSilentMarker() {
    if (insert_silent_marker_) {
      insert_silent_marker_ = false;
      return true;
    }
    return false;
  }

  io::ZeroCopyOutputStream* const output_;
  char* buffer_;
  int buffer_size_;
  bool at_start_of_line_;
  bool failed_;
  // This flag is false when inserting silent marker is disabled or a silent
  // marker has been inserted.
  bool insert_silent_marker_;

  int indent_level_;
  int initial_indent_level_;
};

// ===========================================================================
//  An internal field value printer that may insert a silent marker in
//  DebugStrings.
class TextFormat::Printer::DebugStringFieldValuePrinter
    : public TextFormat::FastFieldValuePrinter {
 public:
  void PrintMessageStart(const Message& /*message*/, int /*field_index*/,
                         int /*field_count*/, bool single_line_mode,
                         BaseTextGenerator* generator) const override {
    if (single_line_mode) {
      generator->PrintMaybeWithMarker(MarkerToken(), " ", "{ ");
    } else {
      generator->PrintMaybeWithMarker(MarkerToken(), " ", "{\n");
    }
  }
};

// ===========================================================================
//  An internal field value printer that escape UTF8 strings.
class TextFormat::Printer::FastFieldValuePrinterUtf8Escaping
    : public TextFormat::Printer::DebugStringFieldValuePrinter {
 public:
  void PrintString(const std::string& val,
                   TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintLiteral("\"");
    generator->PrintString(absl::Utf8SafeCEscape(val));
    generator->PrintLiteral("\"");
  }
  void PrintBytes(const std::string& val,
                  TextFormat::BaseTextGenerator* generator) const override {
    return FastFieldValuePrinter::PrintString(val, generator);
  }
};

// ===========================================================================
// Implementation of the default Finder for extensions.
TextFormat::Finder::~Finder() {}

const FieldDescriptor* TextFormat::Finder::FindExtension(
    Message* message, const std::string& name) const {
  return DefaultFinderFindExtension(message, name);
}

const FieldDescriptor* TextFormat::Finder::FindExtensionByNumber(
    const Descriptor* descriptor, int number) const {
  return DefaultFinderFindExtensionByNumber(descriptor, number);
}

const Descriptor* TextFormat::Finder::FindAnyType(
    const Message& message, const std::string& prefix,
    const std::string& name) const {
  return DefaultFinderFindAnyType(message, prefix, name);
}

MessageFactory* TextFormat::Finder::FindExtensionFactory(
    const FieldDescriptor* /*field*/) const {
  return nullptr;
}

// ===========================================================================

TextFormat::Parser::Parser()
    : error_collector_(nullptr),
      finder_(nullptr),
      parse_info_tree_(nullptr),
      allow_partial_(false),
      allow_case_insensitive_field_(false),
      allow_unknown_field_(false),
      allow_unknown_extension_(false),
      allow_unknown_enum_(false),
      allow_field_number_(false),
      allow_relaxed_whitespace_(false),
      allow_singular_overwrites_(false),
      recursion_limit_(std::numeric_limits<int>::max()) {}

TextFormat::Parser::~Parser() {}

namespace {

template <typename T>
bool CheckParseInputSize(T& input, io::ErrorCollector* error_collector) {
  if (input.size() > INT_MAX) {
    error_collector->RecordError(
        -1, 0,
        absl::StrCat(
            "Input size too large: ", static_cast<int64_t>(input.size()),
            " bytes", " > ", INT_MAX, " bytes."));
    return false;
  }
  return true;
}

}  // namespace

bool TextFormat::Parser::Parse(io::ZeroCopyInputStream* input,
                               Message* output) {
  output->Clear();

  ParserImpl::SingularOverwritePolicy overwrites_policy =
      allow_singular_overwrites_ ? ParserImpl::ALLOW_SINGULAR_OVERWRITES
                                 : ParserImpl::FORBID_SINGULAR_OVERWRITES;

  ParserImpl parser(output->GetDescriptor(), input, error_collector_, finder_,
                    parse_info_tree_, overwrites_policy,
                    allow_case_insensitive_field_, allow_unknown_field_,
                    allow_unknown_extension_, allow_unknown_enum_,
                    allow_field_number_, allow_relaxed_whitespace_,
                    allow_partial_, recursion_limit_, error_on_no_op_fields_);
  return MergeUsingImpl(input, output, &parser);
}

bool TextFormat::Parser::ParseFromString(absl::string_view input,
                                         Message* output) {
  DO(CheckParseInputSize(input, error_collector_));
  io::ArrayInputStream input_stream(input.data(), input.size());
  return Parse(&input_stream, output);
}

bool TextFormat::Parser::ParseFromCord(const absl::Cord& input,
                                       Message* output) {
  DO(CheckParseInputSize(input, error_collector_));
  io::CordInputStream input_stream(&input);
  return Parse(&input_stream, output);
}

bool TextFormat::Parser::Merge(io::ZeroCopyInputStream* input,
                               Message* output) {
  ParserImpl parser(output->GetDescriptor(), input, error_collector_, finder_,
                    parse_info_tree_, ParserImpl::ALLOW_SINGULAR_OVERWRITES,
                    allow_case_insensitive_field_, allow_unknown_field_,
                    allow_unknown_extension_, allow_unknown_enum_,
                    allow_field_number_, allow_relaxed_whitespace_,
                    allow_partial_, recursion_limit_, error_on_no_op_fields_);
  return MergeUsingImpl(input, output, &parser);
}

bool TextFormat::Parser::MergeFromString(absl::string_view input,
                                         Message* output) {
  DO(CheckParseInputSize(input, error_collector_));
  io::ArrayInputStream input_stream(input.data(), input.size());
  return Merge(&input_stream, output);
}

bool TextFormat::Parser::MergeUsingImpl(io::ZeroCopyInputStream* /* input */,
                                        Message* output,
                                        ParserImpl* parser_impl) {
  if (!parser_impl->Parse(output)) return false;
  if (!allow_partial_ && !output->IsInitialized()) {
    std::vector<std::string> missing_fields;
    output->FindInitializationErrors(&missing_fields);
    parser_impl->ReportError(-1, 0,
                             absl::StrCat("Message missing required fields: ",
                                          absl::StrJoin(missing_fields, ", ")));
    return false;
  }
  return true;
}

bool TextFormat::Parser::ParseFieldValueFromString(absl::string_view input,
                                                   const FieldDescriptor* field,
                                                   Message* output) {
  io::ArrayInputStream input_stream(input.data(), input.size());
  ParserImpl parser(output->GetDescriptor(), &input_stream, error_collector_,
                    finder_, parse_info_tree_,
                    ParserImpl::ALLOW_SINGULAR_OVERWRITES,
                    allow_case_insensitive_field_, allow_unknown_field_,
                    allow_unknown_extension_, allow_unknown_enum_,
                    allow_field_number_, allow_relaxed_whitespace_,
                    allow_partial_, recursion_limit_, error_on_no_op_fields_);
  return parser.ParseField(field, output);
}

/* static */ bool TextFormat::Parse(io::ZeroCopyInputStream* input,
                                    Message* output) {
  return Parser().Parse(input, output);
}

/* static */ bool TextFormat::Merge(io::ZeroCopyInputStream* input,
                                    Message* output) {
  return Parser().Merge(input, output);
}

/* static */ bool TextFormat::ParseFromString(absl::string_view input,
                                              Message* output) {
  return Parser().ParseFromString(input, output);
}

/* static */ bool TextFormat::ParseFromCord(const absl::Cord& input,
                                            Message* output) {
  return Parser().ParseFromCord(input, output);
}

/* static */ bool TextFormat::MergeFromString(absl::string_view input,
                                              Message* output) {
  return Parser().MergeFromString(input, output);
}

#undef DO

// ===========================================================================

TextFormat::BaseTextGenerator::~BaseTextGenerator() {}

namespace {

// A BaseTextGenerator that writes to a string.
class StringBaseTextGenerator : public TextFormat::BaseTextGenerator {
 public:
  void Print(const char* text, size_t size) override {
    output_.append(text, size);
  }

  std::string Consume() && { return std::move(output_); }

 private:
  std::string output_;
};

}  // namespace

// The default implementation for FieldValuePrinter. We just delegate the
// implementation to the default FastFieldValuePrinter to avoid duplicating the
// logic.
TextFormat::FieldValuePrinter::FieldValuePrinter() {}
TextFormat::FieldValuePrinter::~FieldValuePrinter() {}

#define FORWARD_IMPL(fn, ...)            \
  StringBaseTextGenerator generator;     \
  delegate_.fn(__VA_ARGS__, &generator); \
  return std::move(generator).Consume()

std::string TextFormat::FieldValuePrinter::PrintBool(bool val) const {
  FORWARD_IMPL(PrintBool, val);
}
std::string TextFormat::FieldValuePrinter::PrintInt32(int32_t val) const {
  FORWARD_IMPL(PrintInt32, val);
}
std::string TextFormat::FieldValuePrinter::PrintUInt32(uint32_t val) const {
  FORWARD_IMPL(PrintUInt32, val);
}
std::string TextFormat::FieldValuePrinter::PrintInt64(int64_t val) const {
  FORWARD_IMPL(PrintInt64, val);
}
std::string TextFormat::FieldValuePrinter::PrintUInt64(uint64_t val) const {
  FORWARD_IMPL(PrintUInt64, val);
}
std::string TextFormat::FieldValuePrinter::PrintFloat(float val) const {
  FORWARD_IMPL(PrintFloat, val);
}
std::string TextFormat::FieldValuePrinter::PrintDouble(double val) const {
  FORWARD_IMPL(PrintDouble, val);
}
std::string TextFormat::FieldValuePrinter::PrintString(
    const std::string& val) const {
  FORWARD_IMPL(PrintString, val);
}
std::string TextFormat::FieldValuePrinter::PrintBytes(
    const std::string& val) const {
  return PrintString(val);
}
std::string TextFormat::FieldValuePrinter::PrintEnum(
    int32_t val, const std::string& name) const {
  FORWARD_IMPL(PrintEnum, val, name);
}
std::string TextFormat::FieldValuePrinter::PrintFieldName(
    const Message& message, const Reflection* reflection,
    const FieldDescriptor* field) const {
  FORWARD_IMPL(PrintFieldName, message, reflection, field);
}
std::string TextFormat::FieldValuePrinter::PrintMessageStart(
    const Message& message, int field_index, int field_count,
    bool single_line_mode) const {
  FORWARD_IMPL(PrintMessageStart, message, field_index, field_count,
               single_line_mode);
}
std::string TextFormat::FieldValuePrinter::PrintMessageEnd(
    const Message& message, int field_index, int field_count,
    bool single_line_mode) const {
  FORWARD_IMPL(PrintMessageEnd, message, field_index, field_count,
               single_line_mode);
}
#undef FORWARD_IMPL

TextFormat::FastFieldValuePrinter::FastFieldValuePrinter() {}
TextFormat::FastFieldValuePrinter::~FastFieldValuePrinter() {}
void TextFormat::FastFieldValuePrinter::PrintBool(
    bool val, BaseTextGenerator* generator) const {
  if (val) {
    generator->PrintLiteral("true");
  } else {
    generator->PrintLiteral("false");
  }
}
void TextFormat::FastFieldValuePrinter::PrintInt32(
    int32_t val, BaseTextGenerator* generator) const {
  generator->PrintString(absl::StrCat(val));
}
void TextFormat::FastFieldValuePrinter::PrintUInt32(
    uint32_t val, BaseTextGenerator* generator) const {
  generator->PrintString(absl::StrCat(val));
}
void TextFormat::FastFieldValuePrinter::PrintInt64(
    int64_t val, BaseTextGenerator* generator) const {
  generator->PrintString(absl::StrCat(val));
}
void TextFormat::FastFieldValuePrinter::PrintUInt64(
    uint64_t val, BaseTextGenerator* generator) const {
  generator->PrintString(absl::StrCat(val));
}
void TextFormat::FastFieldValuePrinter::PrintFloat(
    float val, BaseTextGenerator* generator) const {
  generator->PrintString(!std::isnan(val) ? io::SimpleFtoa(val) : "nan");
}
void TextFormat::FastFieldValuePrinter::PrintDouble(
    double val, BaseTextGenerator* generator) const {
  generator->PrintString(!std::isnan(val) ? io::SimpleDtoa(val) : "nan");
}
void TextFormat::FastFieldValuePrinter::PrintEnum(
    int32_t /*val*/, const std::string& name,
    BaseTextGenerator* generator) const {
  generator->PrintString(name);
}

void TextFormat::FastFieldValuePrinter::PrintString(
    const std::string& val, BaseTextGenerator* generator) const {
  generator->PrintLiteral("\"");
  generator->PrintString(absl::CEscape(val));
  generator->PrintLiteral("\"");
}
void TextFormat::FastFieldValuePrinter::PrintBytes(
    const std::string& val, BaseTextGenerator* generator) const {
  PrintString(val, generator);
}
void TextFormat::FastFieldValuePrinter::PrintFieldName(
    const Message& message, int /*field_index*/, int /*field_count*/,
    const Reflection* reflection, const FieldDescriptor* field,
    BaseTextGenerator* generator) const {
  PrintFieldName(message, reflection, field, generator);
}
void TextFormat::FastFieldValuePrinter::PrintFieldName(
    const Message& /*message*/, const Reflection* /*reflection*/,
    const FieldDescriptor* field, BaseTextGenerator* generator) const {
  if (field->is_extension()) {
    generator->PrintLiteral("[");
    generator->PrintString(field->PrintableNameForExtension());
    generator->PrintLiteral("]");
  } else if (field->type() == FieldDescriptor::TYPE_GROUP) {
    // Groups must be serialized with their original capitalization.
    generator->PrintString(field->message_type()->name());
  } else {
    generator->PrintString(field->name());
  }
}
void TextFormat::FastFieldValuePrinter::PrintMessageStart(
    const Message& /*message*/, int /*field_index*/, int /*field_count*/,
    bool single_line_mode, BaseTextGenerator* generator) const {
  if (single_line_mode) {
    generator->PrintLiteral(" { ");
  } else {
    generator->PrintLiteral(" {\n");
  }
}
bool TextFormat::FastFieldValuePrinter::PrintMessageContent(
    const Message& /*message*/, int /*field_index*/, int /*field_count*/,
    bool /*single_line_mode*/, BaseTextGenerator* /*generator*/) const {
  return false;  // Use the default printing function.
}
void TextFormat::FastFieldValuePrinter::PrintMessageEnd(
    const Message& /*message*/, int /*field_index*/, int /*field_count*/,
    bool single_line_mode, BaseTextGenerator* generator) const {
  if (single_line_mode) {
    generator->PrintLiteral("} ");
  } else {
    generator->PrintLiteral("}\n");
  }
}

namespace {

// A legacy compatibility wrapper. Takes ownership of the delegate.
class FieldValuePrinterWrapper : public TextFormat::FastFieldValuePrinter {
 public:
  explicit FieldValuePrinterWrapper(
      const TextFormat::FieldValuePrinter* delegate)
      : delegate_(delegate) {}

  void SetDelegate(const TextFormat::FieldValuePrinter* delegate) {
    delegate_.reset(delegate);
  }

  void PrintBool(bool val,
                 TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintBool(val));
  }
  void PrintInt32(int32_t val,
                  TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintInt32(val));
  }
  void PrintUInt32(uint32_t val,
                   TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintUInt32(val));
  }
  void PrintInt64(int64_t val,
                  TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintInt64(val));
  }
  void PrintUInt64(uint64_t val,
                   TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintUInt64(val));
  }
  void PrintFloat(float val,
                  TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintFloat(val));
  }
  void PrintDouble(double val,
                   TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintDouble(val));
  }
  void PrintString(const std::string& val,
                   TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintString(val));
  }
  void PrintBytes(const std::string& val,
                  TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintBytes(val));
  }
  void PrintEnum(int32_t val, const std::string& name,
                 TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintEnum(val, name));
  }
  void PrintFieldName(const Message& message, int /*field_index*/,
                      int /*field_count*/, const Reflection* reflection,
                      const FieldDescriptor* field,
                      TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(
        delegate_->PrintFieldName(message, reflection, field));
  }
  void PrintFieldName(const Message& message, const Reflection* reflection,
                      const FieldDescriptor* field,
                      TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(
        delegate_->PrintFieldName(message, reflection, field));
  }
  void PrintMessageStart(
      const Message& message, int field_index, int field_count,
      bool single_line_mode,
      TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintMessageStart(
        message, field_index, field_count, single_line_mode));
  }
  void PrintMessageEnd(
      const Message& message, int field_index, int field_count,
      bool single_line_mode,
      TextFormat::BaseTextGenerator* generator) const override {
    generator->PrintString(delegate_->PrintMessageEnd(
        message, field_index, field_count, single_line_mode));
  }

 private:
  std::unique_ptr<const TextFormat::FieldValuePrinter> delegate_;
};

}  // namespace

TextFormat::Printer::Printer()
    : initial_indent_level_(0),
      single_line_mode_(false),
      use_field_number_(false),
      use_short_repeated_primitives_(false),
      insert_silent_marker_(false),
      redact_debug_string_(false),
      randomize_debug_string_(false),
      report_sensitive_fields_(internal::FieldReporterLevel::kNoReport),
      hide_unknown_fields_(false),
      print_message_fields_in_index_order_(false),
      expand_any_(false),
      truncate_string_field_longer_than_(0LL),
      finder_(nullptr) {
  SetUseUtf8StringEscaping(false);
}

void TextFormat::Printer::SetUseUtf8StringEscaping(bool as_utf8) {
  SetDefaultFieldValuePrinter(as_utf8 ? new FastFieldValuePrinterUtf8Escaping()
                                      : new DebugStringFieldValuePrinter());
}

void TextFormat::Printer::SetDefaultFieldValuePrinter(
    const FieldValuePrinter* printer) {
  default_field_value_printer_.reset(new FieldValuePrinterWrapper(printer));
}

void TextFormat::Printer::SetDefaultFieldValuePrinter(
    const FastFieldValuePrinter* printer) {
  default_field_value_printer_.reset(printer);
}

bool TextFormat::Printer::RegisterFieldValuePrinter(
    const FieldDescriptor* field, const FieldValuePrinter* printer) {
  if (field == nullptr || printer == nullptr) {
    return false;
  }
  std::unique_ptr<FieldValuePrinterWrapper> wrapper(
      new FieldValuePrinterWrapper(nullptr));
  auto pair = custom_printers_.emplace(field, nullptr);
  if (pair.second) {
    wrapper->SetDelegate(printer);
    pair.first->second = std::move(wrapper);
    return true;
  } else {
    return false;
  }
}

bool TextFormat::Printer::RegisterFieldValuePrinter(
    const FieldDescriptor* field, const FastFieldValuePrinter* printer) {
  if (field == nullptr || printer == nullptr) {
    return false;
  }
  auto pair = custom_printers_.emplace(field, nullptr);
  if (pair.second) {
    pair.first->second.reset(printer);
    return true;
  } else {
    return false;
  }
}

bool TextFormat::Printer::RegisterMessagePrinter(
    const Descriptor* descriptor, const MessagePrinter* printer) {
  if (descriptor == nullptr || printer == nullptr) {
    return false;
  }
  auto pair = custom_message_printers_.emplace(descriptor, nullptr);
  if (pair.second) {
    pair.first->second.reset(printer);
    return true;
  } else {
    return false;
  }
}

bool TextFormat::Printer::PrintToString(const Message& message,
                                        std::string* output) const {
  ABSL_DCHECK(output) << "output specified is nullptr";

  output->clear();
  io::StringOutputStream output_stream(output);

  return Print(message, &output_stream,
               internal::FieldReporterLevel::kMemberPrintToString);
}

bool TextFormat::Printer::PrintUnknownFieldsToString(
    const UnknownFieldSet& unknown_fields, std::string* output) const {
  ABSL_DCHECK(output) << "output specified is nullptr";

  output->clear();
  io::StringOutputStream output_stream(output);
  return PrintUnknownFields(unknown_fields, &output_stream);
}

bool TextFormat::Printer::Print(const Message& message,
                                io::ZeroCopyOutputStream* output) const {
  return Print(message, output, internal::FieldReporterLevel::kPrintWithStream);
}

bool TextFormat::Printer::Print(const Message& message,
                                io::ZeroCopyOutputStream* output,
                                internal::FieldReporterLevel reporter) const {
  TextGenerator generator(output, insert_silent_marker_, initial_indent_level_);


  Print(message, &generator);

  // Output false if the generator failed internally.
  return !generator.failed();
}

// Maximum recursion depth for heuristically printing out length-delimited
// unknown fields as messages.
static constexpr int kUnknownFieldRecursionLimit = 10;

bool TextFormat::Printer::PrintUnknownFields(
    const UnknownFieldSet& unknown_fields,
    io::ZeroCopyOutputStream* output) const {
  TextGenerator generator(output, initial_indent_level_);

  PrintUnknownFields(unknown_fields, &generator, kUnknownFieldRecursionLimit);

  // Output false if the generator failed internally.
  return !generator.failed();
}

namespace {
// Comparison functor for sorting FieldDescriptors by field index.
// Normal fields have higher precedence than extensions.
struct FieldIndexSorter {
  bool operator()(const FieldDescriptor* left,
                  const FieldDescriptor* right) const {
    if (left->is_extension() && right->is_extension()) {
      return left->number() < right->number();
    } else if (left->is_extension()) {
      return false;
    } else if (right->is_extension()) {
      return true;
    } else {
      return left->index() < right->index();
    }
  }
};

}  // namespace

bool TextFormat::Printer::PrintAny(const Message& message,
                                   BaseTextGenerator* generator) const {
  const FieldDescriptor* type_url_field;
  const FieldDescriptor* value_field;
  if (!internal::GetAnyFieldDescriptors(message, &type_url_field,
                                        &value_field)) {
    return false;
  }

  const Reflection* reflection = message.GetReflection();

  // Extract the full type name from the type_url field.
  const std::string& type_url = reflection->GetString(message, type_url_field);
  std::string url_prefix;
  std::string full_type_name;
  if (!internal::ParseAnyTypeUrl(type_url, &url_prefix, &full_type_name)) {
    return false;
  }

  // Print the "value" in text.
  const Descriptor* value_descriptor =
      finder_ ? finder_->FindAnyType(message, url_prefix, full_type_name)
              : DefaultFinderFindAnyType(message, url_prefix, full_type_name);
  if (value_descriptor == nullptr) {
    ABSL_LOG(WARNING) << "Can't print proto content: proto type " << type_url
                      << " not found";
    return false;
  }
  DynamicMessageFactory factory;
  std::unique_ptr<Message> value_message(
      factory.GetPrototype(value_descriptor)->New());
  std::string serialized_value = reflection->GetString(message, value_field);
  if (!value_message->ParseFromString(serialized_value)) {
    ABSL_LOG(WARNING) << type_url << ": failed to parse contents";
    return false;
  }
  generator->PrintLiteral("[");
  generator->PrintString(type_url);
  generator->PrintLiteral("]");
  const FastFieldValuePrinter* printer = GetFieldPrinter(value_field);
  printer->PrintMessageStart(message, -1, 0, single_line_mode_, generator);
  generator->Indent();
  Print(*value_message, generator);
  generator->Outdent();
  printer->PrintMessageEnd(message, -1, 0, single_line_mode_, generator);
  return true;
}

void TextFormat::Printer::Print(const Message& message,
                                BaseTextGenerator* generator) const {
  const Reflection* reflection = message.GetReflection();
  if (!reflection) {
    // This message does not provide any way to describe its structure.
    // Parse it again in an UnknownFieldSet, and display this instead.
    UnknownFieldSet unknown_fields;
    {
      std::string serialized = message.SerializeAsString();
      io::ArrayInputStream input(serialized.data(), serialized.size());
      unknown_fields.ParseFromZeroCopyStream(&input);
    }
    PrintUnknownFields(unknown_fields, generator, kUnknownFieldRecursionLimit);
    return;
  }
  const Descriptor* descriptor = message.GetDescriptor();
  auto itr = custom_message_printers_.find(descriptor);
  if (itr != custom_message_printers_.end()) {
    itr->second->Print(message, single_line_mode_, generator);
    return;
  }
  PrintMessage(message, generator);
}

void TextFormat::Printer::PrintMessage(const Message& message,
                                       BaseTextGenerator* generator) const {
  if (generator == nullptr) {
    return;
  }
  const Descriptor* descriptor = message.GetDescriptor();
  if (descriptor->full_name() == internal::kAnyFullTypeName && expand_any_ &&
      PrintAny(message, generator)) {
    return;
  }
  const Reflection* reflection = message.GetReflection();
  std::vector<const FieldDescriptor*> fields;
  if (descriptor->options().map_entry()) {
    fields.push_back(descriptor->field(0));
    fields.push_back(descriptor->field(1));
  } else {
    reflection->ListFields(message, &fields);
  }

  if (print_message_fields_in_index_order_) {
    std::sort(fields.begin(), fields.end(), FieldIndexSorter());
  }
  for (const FieldDescriptor* field : fields) {
    PrintField(message, reflection, field, generator);
  }
  if (!hide_unknown_fields_) {
    PrintUnknownFields(reflection->GetUnknownFields(message), generator,
                       kUnknownFieldRecursionLimit);
  }
}

void TextFormat::Printer::PrintFieldValueToString(const Message& message,
                                                  const FieldDescriptor* field,
                                                  int index,
                                                  std::string* output) const {
  ABSL_DCHECK(output) << "output specified is nullptr";

  output->clear();
  io::StringOutputStream output_stream(output);
  TextGenerator generator(&output_stream, initial_indent_level_);

  PrintFieldValue(message, message.GetReflection(), field, index, &generator);
}

class MapEntryMessageComparator {
 public:
  explicit MapEntryMessageComparator(const Descriptor* descriptor)
      : field_(descriptor->field(0)) {}

  bool operator()(const Message* a, const Message* b) {
    const Reflection* reflection = a->GetReflection();
    switch (field_->cpp_type()) {
      case FieldDescriptor::CPPTYPE_BOOL: {
        bool first = reflection->GetBool(*a, field_);
        bool second = reflection->GetBool(*b, field_);
        return first < second;
      }
      case FieldDescriptor::CPPTYPE_INT32: {
        int32_t first = reflection->GetInt32(*a, field_);
        int32_t second = reflection->GetInt32(*b, field_);
        return first < second;
      }
      case FieldDescriptor::CPPTYPE_INT64: {
        int64_t first = reflection->GetInt64(*a, field_);
        int64_t second = reflection->GetInt64(*b, field_);
        return first < second;
      }
      case FieldDescriptor::CPPTYPE_UINT32: {
        uint32_t first = reflection->GetUInt32(*a, field_);
        uint32_t second = reflection->GetUInt32(*b, field_);
        return first < second;
      }
      case FieldDescriptor::CPPTYPE_UINT64: {
        uint64_t first = reflection->GetUInt64(*a, field_);
        uint64_t second = reflection->GetUInt64(*b, field_);
        return first < second;
      }
      case FieldDescriptor::CPPTYPE_STRING: {
        std::string first = reflection->GetString(*a, field_);
        std::string second = reflection->GetString(*b, field_);
        return first < second;
      }
      default:
        ABSL_DLOG(FATAL) << "Invalid key for map field.";
        return true;
    }
  }

 private:
  const FieldDescriptor* field_;
};

namespace internal {
class MapFieldPrinterHelper {
 public:
  // DynamicMapSorter::Sort cannot be used because it enforces syncing with
  // repeated field.
  static bool SortMap(const Message& message, const Reflection* reflection,
                      const FieldDescriptor* field,
                      std::vector<const Message*>* sorted_map_field);
  static void CopyKey(const MapKey& key, Message* message,
                      const FieldDescriptor* field_desc);
  static void CopyValue(const MapValueRef& value, Message* message,
                        const FieldDescriptor* field_desc);
};

// Returns true if elements contained in sorted_map_field need to be released.
bool MapFieldPrinterHelper::SortMap(
    const Message& message, const Reflection* reflection,
    const FieldDescriptor* field,
    std::vector<const Message*>* sorted_map_field) {
  bool need_release = false;
  const MapFieldBase& base = *reflection->GetMapData(message, field);

  if (base.IsRepeatedFieldValid()) {
    const RepeatedPtrField<Message>& map_field =
        reflection->GetRepeatedPtrFieldInternal<Message>(message, field);
    for (int i = 0; i < map_field.size(); ++i) {
      sorted_map_field->push_back(
          const_cast<RepeatedPtrField<Message>*>(&map_field)->Mutable(i));
    }
  } else {
    // TODO: For performance, instead of creating map entry message
    // for each element, just store map keys and sort them.
    const Descriptor* map_entry_desc = field->message_type();
    const Message* prototype =
        reflection->GetMessageFactory()->GetPrototype(map_entry_desc);
    for (MapIterator iter =
             reflection->MapBegin(const_cast<Message*>(&message), field);
         iter != reflection->MapEnd(const_cast<Message*>(&message), field);
         ++iter) {
      Message* map_entry_message = prototype->New();
      CopyKey(iter.GetKey(), map_entry_message, map_entry_desc->field(0));
      CopyValue(iter.GetValueRef(), map_entry_message,
                map_entry_desc->field(1));
      sorted_map_field->push_back(map_entry_message);
    }
    need_release = true;
  }

  MapEntryMessageComparator comparator(field->message_type());
  std::stable_sort(sorted_map_field->begin(), sorted_map_field->end(),
                   comparator);
  return need_release;
}

void MapFieldPrinterHelper::CopyKey(const MapKey& key, Message* message,
                                    const FieldDescriptor* field_desc) {
  const Reflection* reflection = message->GetReflection();
  switch (field_desc->cpp_type()) {
    case FieldDescriptor::CPPTYPE_DOUBLE:
    case FieldDescriptor::CPPTYPE_FLOAT:
    case FieldDescriptor::CPPTYPE_ENUM:
    case FieldDescriptor::CPPTYPE_MESSAGE:
      ABSL_LOG(ERROR) << "Not supported.";
      break;
    case FieldDescriptor::CPPTYPE_STRING:
      reflection->SetString(message, field_desc, key.GetStringValue());
      return;
    case FieldDescriptor::CPPTYPE_INT64:
      reflection->SetInt64(message, field_desc, key.GetInt64Value());
      return;
    case FieldDescriptor::CPPTYPE_INT32:
      reflection->SetInt32(message, field_desc, key.GetInt32Value());
      return;
    case FieldDescriptor::CPPTYPE_UINT64:
      reflection->SetUInt64(message, field_desc, key.GetUInt64Value());
      return;
    case FieldDescriptor::CPPTYPE_UINT32:
      reflection->SetUInt32(message, field_desc, key.GetUInt32Value());
      return;
    case FieldDescriptor::CPPTYPE_BOOL:
      reflection->SetBool(message, field_desc, key.GetBoolValue());
      return;
  }
}

void MapFieldPrinterHelper::CopyValue(const MapValueRef& value,
                                      Message* message,
                                      const FieldDescriptor* field_desc) {
  const Reflection* reflection = message->GetReflection();
  switch (field_desc->cpp_type()) {
    case FieldDescriptor::CPPTYPE_DOUBLE:
      reflection->SetDouble(message, field_desc, value.GetDoubleValue());
      return;
    case FieldDescriptor::CPPTYPE_FLOAT:
      reflection->SetFloat(message, field_desc, value.GetFloatValue());
      return;
    case FieldDescriptor::CPPTYPE_ENUM:
      reflection->SetEnumValue(message, field_desc, value.GetEnumValue());
      return;
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      Message* sub_message = value.GetMessageValue().New();
      sub_message->CopyFrom(value.GetMessageValue());
      reflection->SetAllocatedMessage(message, sub_message, field_desc);
      return;
    }
    case FieldDescriptor::CPPTYPE_STRING:
      reflection->SetString(message, field_desc, value.GetStringValue());
      return;
    case FieldDescriptor::CPPTYPE_INT64:
      reflection->SetInt64(message, field_desc, value.GetInt64Value());
      return;
    case FieldDescriptor::CPPTYPE_INT32:
      reflection->SetInt32(message, field_desc, value.GetInt32Value());
      return;
    case FieldDescriptor::CPPTYPE_UINT64:
      reflection->SetUInt64(message, field_desc, value.GetUInt64Value());
      return;
    case FieldDescriptor::CPPTYPE_UINT32:
      reflection->SetUInt32(message, field_desc, value.GetUInt32Value());
      return;
    case FieldDescriptor::CPPTYPE_BOOL:
      reflection->SetBool(message, field_desc, value.GetBoolValue());
      return;
  }
}
}  // namespace internal

void TextFormat::Printer::PrintField(const Message& message,
                                     const Reflection* reflection,
                                     const FieldDescriptor* field,
                                     BaseTextGenerator* generator) const {
  if (use_short_repeated_primitives_ && field->is_repeated() &&
      field->cpp_type() != FieldDescriptor::CPPTYPE_STRING &&
      field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    PrintShortRepeatedField(message, reflection, field, generator);
    return;
  }

  int count = 0;

  if (field->is_repeated()) {
    count = reflection->FieldSize(message, field);
  } else if (reflection->HasField(message, field) ||
             field->containing_type()->options().map_entry()) {
    count = 1;
  }

  std::vector<const Message*> sorted_map_field;
  bool need_release = false;
  bool is_map = field->is_map();
  if (is_map) {
    need_release = internal::MapFieldPrinterHelper::SortMap(
        message, reflection, field, &sorted_map_field);
  }

  for (int j = 0; j < count; ++j) {
    const int field_index = field->is_repeated() ? j : -1;

    PrintFieldName(message, field_index, count, reflection, field, generator);

    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      if (TryRedactFieldValue(message, field, generator,
                              /*insert_value_separator=*/true)) {
        break;
      }
      const FastFieldValuePrinter* printer = GetFieldPrinter(field);
      const Message& sub_message =
          field->is_repeated()
              ? (is_map ? *sorted_map_field[j]
                        : reflection->GetRepeatedMessage(message, field, j))
              : reflection->GetMessage(message, field);
      printer->PrintMessageStart(sub_message, field_index, count,
                                 single_line_mode_, generator);
      generator->Indent();
      if (!printer->PrintMessageContent(sub_message, field_index, count,
                                        single_line_mode_, generator)) {
        Print(sub_message, generator);
      }
      generator->Outdent();
      printer->PrintMessageEnd(sub_message, field_index, count,
                               single_line_mode_, generator);
    } else {
      generator->PrintMaybeWithMarker(MarkerToken(), ": ");
      // Write the field value.
      PrintFieldValue(message, reflection, field, field_index, generator);
      if (single_line_mode_) {
        generator->PrintLiteral(" ");
      } else {
        generator->PrintLiteral("\n");
      }
    }
  }

  if (need_release) {
    for (const Message* message_to_delete : sorted_map_field) {
      delete message_to_delete;
    }
  }
}

void TextFormat::Printer::PrintShortRepeatedField(
    const Message& message, const Reflection* reflection,
    const FieldDescriptor* field, BaseTextGenerator* generator) const {
  // Print primitive repeated field in short form.
  int size = reflection->FieldSize(message, field);
  PrintFieldName(message, /*field_index=*/-1, /*field_count=*/size, reflection,
                 field, generator);
  generator->PrintMaybeWithMarker(MarkerToken(), ": ", "[");
  for (int i = 0; i < size; i++) {
    if (i > 0) generator->PrintLiteral(", ");
    PrintFieldValue(message, reflection, field, i, generator);
  }
  if (single_line_mode_) {
    generator->PrintLiteral("] ");
  } else {
    generator->PrintLiteral("]\n");
  }
}

void TextFormat::Printer::PrintFieldName(const Message& message,
                                         int field_index, int field_count,
                                         const Reflection* reflection,
                                         const FieldDescriptor* field,
                                         BaseTextGenerator* generator) const {
  // if use_field_number_ is true, prints field number instead
  // of field name.
  if (use_field_number_) {
    generator->PrintString(absl::StrCat(field->number()));
    return;
  }

  const FastFieldValuePrinter* printer = GetFieldPrinter(field);
  printer->PrintFieldName(message, field_index, field_count, reflection, field,
                          generator);
}

void TextFormat::Printer::PrintFieldValue(const Message& message,
                                          const Reflection* reflection,
                                          const FieldDescriptor* field,
                                          int index,
                                          BaseTextGenerator* generator) const {
  ABSL_DCHECK(field->is_repeated() || (index == -1))
      << "Index must be -1 for non-repeated fields";

  const FastFieldValuePrinter* printer = GetFieldPrinter(field);
  if (TryRedactFieldValue(message, field, generator,
                          /*insert_value_separator=*/false)) {
    return;
  }

  switch (field->cpp_type()) {
#define OUTPUT_FIELD(CPPTYPE, METHOD)                                \
  case FieldDescriptor::CPPTYPE_##CPPTYPE:                           \
    printer->Print##METHOD(                                          \
        field->is_repeated()                                         \
            ? reflection->GetRepeated##METHOD(message, field, index) \
            : reflection->Get##METHOD(message, field),               \
        generator);                                                  \
    break

    OUTPUT_FIELD(INT32, Int32);
    OUTPUT_FIELD(INT64, Int64);
    OUTPUT_FIELD(UINT32, UInt32);
    OUTPUT_FIELD(UINT64, UInt64);
    OUTPUT_FIELD(FLOAT, Float);
    OUTPUT_FIELD(DOUBLE, Double);
    OUTPUT_FIELD(BOOL, Bool);
#undef OUTPUT_FIELD

    case FieldDescriptor::CPPTYPE_STRING: {
      std::string scratch;
      const std::string& value =
          field->is_repeated()
              ? reflection->GetRepeatedStringReference(message, field, index,
                                                       &scratch)
              : reflection->GetStringReference(message, field, &scratch);
      const std::string* value_to_print = &value;
      std::string truncated_value;
      if (truncate_string_field_longer_than_ > 0 &&
          static_cast<size_t>(truncate_string_field_longer_than_) <
              value.size()) {
        truncated_value = value.substr(0, truncate_string_field_longer_than_) +
                          "...<truncated>...";
        value_to_print = &truncated_value;
      }
      if (field->type() == FieldDescriptor::TYPE_STRING) {
        printer->PrintString(*value_to_print, generator);
      } else {
        ABSL_DCHECK_EQ(field->type(), FieldDescriptor::TYPE_BYTES);
        printer->PrintBytes(*value_to_print, generator);
      }
      break;
    }

    case FieldDescriptor::CPPTYPE_ENUM: {
      int enum_value =
          field->is_repeated()
              ? reflection->GetRepeatedEnumValue(message, field, index)
              : reflection->GetEnumValue(message, field);
      const EnumValueDescriptor* enum_desc =
          field->enum_type()->FindValueByNumber(enum_value);
      if (enum_desc != nullptr) {
        printer->PrintEnum(enum_value, enum_desc->name(), generator);
      } else {
        // Ordinarily, enum_desc should not be null, because proto2 has the
        // invariant that set enum field values must be in-range, but with the
        // new integer-based API for enums (or the RepeatedField<int> loophole),
        // it is possible for the user to force an unknown integer value.  So we
        // simply use the integer value itself as the enum value name in this
        // case.
        printer->PrintEnum(enum_value, absl::StrCat(enum_value), generator);
      }
      break;
    }

    case FieldDescriptor::CPPTYPE_MESSAGE:
      Print(field->is_repeated()
                ? reflection->GetRepeatedMessage(message, field, index)
                : reflection->GetMessage(message, field),
            generator);
      break;
  }
}

/* static */ bool TextFormat::Print(const Message& message,
                                    io::ZeroCopyOutputStream* output) {
  return Printer().Print(message, output);
}

/* static */ bool TextFormat::PrintUnknownFields(
    const UnknownFieldSet& unknown_fields, io::ZeroCopyOutputStream* output) {
  return Printer().PrintUnknownFields(unknown_fields, output);
}

/* static */ bool TextFormat::PrintToString(const Message& message,
                                            std::string* output) {
  auto printer = Printer();
  return printer.PrintToString(message, output);
}

/* static */ bool TextFormat::PrintUnknownFieldsToString(
    const UnknownFieldSet& unknown_fields, std::string* output) {
  return Printer().PrintUnknownFieldsToString(unknown_fields, output);
}

/* static */ void TextFormat::PrintFieldValueToString(
    const Message& message, const FieldDescriptor* field, int index,
    std::string* output) {
  return Printer().PrintFieldValueToString(message, field, index, output);
}

/* static */ bool TextFormat::ParseFieldValueFromString(
    absl::string_view input, const FieldDescriptor* field, Message* message) {
  return Parser().ParseFieldValueFromString(input, field, message);
}

template <typename... T>
PROTOBUF_NOINLINE void TextFormat::OutOfLinePrintString(
    BaseTextGenerator* generator, const T&... values) {
  generator->PrintString(absl::StrCat(values...));
}

void TextFormat::Printer::PrintUnknownFields(
    const UnknownFieldSet& unknown_fields, BaseTextGenerator* generator,
    int recursion_budget) const {
  for (int i = 0; i < unknown_fields.field_count(); i++) {
    const UnknownField& field = unknown_fields.field(i);

    switch (field.type()) {
      case UnknownField::TYPE_VARINT:
        OutOfLinePrintString(generator, field.number());
        generator->PrintMaybeWithMarker(MarkerToken(), ": ");
        OutOfLinePrintString(generator, field.varint());
        if (single_line_mode_) {
          generator->PrintLiteral(" ");
        } else {
          generator->PrintLiteral("\n");
        }
        break;
      case UnknownField::TYPE_FIXED32: {
        OutOfLinePrintString(generator, field.number());
        generator->PrintMaybeWithMarker(MarkerToken(), ": ", "0x");
        OutOfLinePrintString(generator,
                             absl::Hex(field.fixed32(), absl::kZeroPad8));
        if (single_line_mode_) {
          generator->PrintLiteral(" ");
        } else {
          generator->PrintLiteral("\n");
        }
        break;
      }
      case UnknownField::TYPE_FIXED64: {
        OutOfLinePrintString(generator, field.number());
        generator->PrintMaybeWithMarker(MarkerToken(), ": ", "0x");
        OutOfLinePrintString(generator,
                             absl::Hex(field.fixed64(), absl::kZeroPad16));
        if (single_line_mode_) {
          generator->PrintLiteral(" ");
        } else {
          generator->PrintLiteral("\n");
        }
        break;
      }
      case UnknownField::TYPE_LENGTH_DELIMITED: {
        OutOfLinePrintString(generator, field.number());
        const std::string& value = field.length_delimited();
        // We create a CodedInputStream so that we can adhere to our recursion
        // budget when we attempt to parse the data. UnknownFieldSet parsing is
        // recursive because of groups.
        io::CodedInputStream input_stream(
            reinterpret_cast<const uint8_t*>(value.data()), value.size());
        input_stream.SetRecursionLimit(recursion_budget);
        UnknownFieldSet embedded_unknown_fields;
        if (!value.empty() && recursion_budget > 0 &&
            embedded_unknown_fields.ParseFromCodedStream(&input_stream)) {
          // This field is parseable as a Message.
          // So it is probably an embedded message.
          if (single_line_mode_) {
            generator->PrintMaybeWithMarker(MarkerToken(), " ", "{ ");
          } else {
            generator->PrintMaybeWithMarker(MarkerToken(), " ", "{\n");
            generator->Indent();
          }
          PrintUnknownFields(embedded_unknown_fields, generator,
                             recursion_budget - 1);
          if (single_line_mode_) {
            generator->PrintLiteral("} ");
          } else {
            generator->Outdent();
            generator->PrintLiteral("}\n");
          }
        } else {
          // This field is not parseable as a Message (or we ran out of
          // recursion budget). So it is probably just a plain string.
          generator->PrintMaybeWithMarker(MarkerToken(), ": ", "\"");
          generator->PrintString(absl::CEscape(value));
          if (single_line_mode_) {
            generator->PrintLiteral("\" ");
          } else {
            generator->PrintLiteral("\"\n");
          }
        }
        break;
      }
      case UnknownField::TYPE_GROUP:
        OutOfLinePrintString(generator, field.number());
        if (single_line_mode_) {
          generator->PrintMaybeWithMarker(MarkerToken(), " ", "{ ");
        } else {
          generator->PrintMaybeWithMarker(MarkerToken(), " ", "{\n");
          generator->Indent();
        }
        // For groups, we recurse without checking the budget. This is OK,
        // because if the groups were too deeply nested then we would have
        // already rejected the message when we originally parsed it.
        PrintUnknownFields(field.group(), generator, recursion_budget - 1);
        if (single_line_mode_) {
          generator->PrintLiteral("} ");
        } else {
          generator->Outdent();
          generator->PrintLiteral("}\n");
        }
        break;
    }
  }
}

namespace internal {

// Check if the field is sensitive and should be redacted.
bool ShouldRedactField(const FieldDescriptor* field) {
  if (field->options().debug_redact()) return true;
  return false;
}

}  // namespace internal

bool TextFormat::Printer::TryRedactFieldValue(
    const Message& message, const FieldDescriptor* field,
    BaseTextGenerator* generator, bool insert_value_separator) const {
  if (internal::ShouldRedactField(field)) {
    if (redact_debug_string_) {
      IncrementRedactedFieldCounter();
      if (insert_value_separator) {
        generator->PrintMaybeWithMarker(MarkerToken(), ": ");
      }
      generator->PrintString(kFieldValueReplacement);
      if (insert_value_separator) {
        if (single_line_mode_) {
          generator->PrintLiteral(" ");
        } else {
          generator->PrintLiteral("\n");
        }
      }
      return true;
    }
  }
  return false;
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


