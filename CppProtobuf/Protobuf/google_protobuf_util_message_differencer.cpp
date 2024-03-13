// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: jschorr@google.com (Joseph Schorr)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include "google_protobuf_util_message_differencer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <utility>

#include "google_protobuf_descriptor.pb.hpp"
#include <CppAbseil/absl_container_fixed_array.hpp>
#include <CppAbseil/absl_container_flat_hash_map.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_strings_escaping.hpp>
#include <CppAbseil/absl_strings_match.hpp>
#include <CppAbseil/absl_strings_str_cat.hpp>
#include <CppAbseil/absl_strings_str_format.hpp>
#include "google_protobuf_descriptor.hpp"
#include "google_protobuf_dynamic_message.hpp"
#include "google_protobuf_generated_enum_reflection.hpp"
#include "google_protobuf_io_printer.hpp"
#include "google_protobuf_io_zero_copy_stream.hpp"
#include "google_protobuf_io_zero_copy_stream_impl.hpp"
#include "google_protobuf_map_field.hpp"
#include "google_protobuf_message.hpp"
#include "google_protobuf_text_format.hpp"
#include "google_protobuf_util_field_comparator.hpp"

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

namespace util {

namespace {

std::string PrintShortTextFormat(const google::protobuf::Message& message) {
  std::string debug_string;

  google::protobuf::TextFormat::Printer printer;
  printer.SetSingleLineMode(true);
  printer.SetExpandAny(true);

  printer.PrintToString(message, &debug_string);
  // Single line mode currently might have an extra space at the end.
  if (!debug_string.empty() && debug_string[debug_string.size() - 1] == ' ') {
    debug_string.resize(debug_string.size() - 1);
  }

  return debug_string;
}

}  // namespace

// A reporter to report the total number of diffs.
// TODO: we can improve this to take into account the value differencers.
class NumDiffsReporter : public google::protobuf::util::MessageDifferencer::Reporter {
 public:
  NumDiffsReporter() : num_diffs_(0) {}

  // Returns the total number of diffs.
  int32_t GetNumDiffs() const { return num_diffs_; }
  void Reset() { num_diffs_ = 0; }

  // Report that a field has been added into Message2.
  void ReportAdded(
      const google::protobuf::Message& /* message1 */,
      const google::protobuf::Message& /* message2 */,
      const std::vector<google::protobuf::util::MessageDifferencer::SpecificField>&
      /*field_path*/) override {
    ++num_diffs_;
  }

  // Report that a field has been deleted from Message1.
  void ReportDeleted(
      const google::protobuf::Message& /* message1 */,
      const google::protobuf::Message& /* message2 */,
      const std::vector<google::protobuf::util::MessageDifferencer::SpecificField>&
      /*field_path*/) override {
    ++num_diffs_;
  }

  // Report that the value of a field has been modified.
  void ReportModified(
      const google::protobuf::Message& /* message1 */,
      const google::protobuf::Message& /* message2 */,
      const std::vector<google::protobuf::util::MessageDifferencer::SpecificField>&
      /*field_path*/) override {
    ++num_diffs_;
  }

 private:
  int32_t num_diffs_;
};

// When comparing a repeated field as map, MultipleFieldMapKeyComparator can
// be used to specify multiple fields as key for key comparison.
// Two elements of a repeated field will be regarded as having the same key
// iff they have the same value for every specified key field.
// Note that you can also specify only one field as key.
class MessageDifferencer::MultipleFieldsMapKeyComparator
    : public MessageDifferencer::MapKeyComparator {
 public:
  MultipleFieldsMapKeyComparator(
      MessageDifferencer* message_differencer,
      const std::vector<std::vector<const FieldDescriptor*> >& key_field_paths)
      : message_differencer_(message_differencer),
        key_field_paths_(key_field_paths) {
    ABSL_CHECK(!key_field_paths_.empty());
    for (const auto& path : key_field_paths_) {
      ABSL_CHECK(!path.empty());
    }
  }
  MultipleFieldsMapKeyComparator(MessageDifferencer* message_differencer,
                                 const FieldDescriptor* key)
      : message_differencer_(message_differencer) {
    std::vector<const FieldDescriptor*> key_field_path;
    key_field_path.push_back(key);
    key_field_paths_.push_back(key_field_path);
  }
  MultipleFieldsMapKeyComparator(const MultipleFieldsMapKeyComparator&) =
      delete;
  MultipleFieldsMapKeyComparator& operator=(
      const MultipleFieldsMapKeyComparator&) = delete;
  bool IsMatch(const Message& message1, const Message& message2,
               int unpacked_any,
               const std::vector<SpecificField>& parent_fields) const override {
    for (const auto& path : key_field_paths_) {
      if (!IsMatchInternal(message1, message2, unpacked_any, parent_fields,
                           path, 0)) {
        return false;
      }
    }
    return true;
  }

 private:
  bool IsMatchInternal(
      const Message& message1, const Message& message2, int unpacked_any,
      const std::vector<SpecificField>& parent_fields,
      const std::vector<const FieldDescriptor*>& key_field_path,
      int path_index) const {
    const FieldDescriptor* field = key_field_path[path_index];
    std::vector<SpecificField> current_parent_fields(parent_fields);
    if (path_index == static_cast<int64_t>(key_field_path.size() - 1)) {
      if (field->is_map()) {
        return message_differencer_->CompareMapField(
            message1, message2, unpacked_any, field, &current_parent_fields);
      } else if (field->is_repeated()) {
        return message_differencer_->CompareRepeatedField(
            message1, message2, unpacked_any, field, &current_parent_fields);
      } else {
        return message_differencer_->CompareFieldValueUsingParentFields(
            message1, message2, unpacked_any, field, -1, -1,
            &current_parent_fields);
      }
    } else {
      const Reflection* reflection1 = message1.GetReflection();
      const Reflection* reflection2 = message2.GetReflection();
      bool has_field1 = reflection1->HasField(message1, field);
      bool has_field2 = reflection2->HasField(message2, field);
      if (!has_field1 && !has_field2) {
        return true;
      }
      if (has_field1 != has_field2) {
        return false;
      }
      SpecificField specific_field;
      specific_field.message1 = &message1;
      specific_field.message2 = &message2;
      specific_field.unpacked_any = unpacked_any;
      specific_field.field = field;
      current_parent_fields.push_back(specific_field);
      return IsMatchInternal(reflection1->GetMessage(message1, field),
                             reflection2->GetMessage(message2, field),
                             false /*key is never Any*/, current_parent_fields,
                             key_field_path, path_index + 1);
    }
  }
  MessageDifferencer* message_differencer_;
  std::vector<std::vector<const FieldDescriptor*> > key_field_paths_;
};

// Preserve the order when treating repeated field as SMART_LIST. The current
// implementation is to find the longest matching sequence from the first
// element. The optimal solution requires to use //util/diff/lcs.h, which is
// not open sourced yet. Overwrite this method if you want to have that.
// TODO: change to use LCS once it is open sourced.
void MatchIndicesPostProcessorForSmartList(std::vector<int>* match_list1,
                                           std::vector<int>* match_list2) {
  int last_matched_index = -1;
  for (size_t i = 0; i < match_list1->size(); ++i) {
    if (match_list1->at(i) < 0) {
      continue;
    }
    if (last_matched_index < 0 || match_list1->at(i) > last_matched_index) {
      last_matched_index = match_list1->at(i);
    } else {
      match_list2->at(match_list1->at(i)) = -1;
      match_list1->at(i) = -1;
    }
  }
}

void AddSpecificIndex(
    google::protobuf::util::MessageDifferencer::SpecificField* specific_field,
    const Message& message, const FieldDescriptor* field, int index) {
  if (field->is_map()) {
    const Reflection* reflection = message.GetReflection();
    specific_field->map_entry1 =
        &reflection->GetRepeatedMessage(message, field, index);
  }
  specific_field->index = index;
}

void AddSpecificNewIndex(
    google::protobuf::util::MessageDifferencer::SpecificField* specific_field,
    const Message& message, const FieldDescriptor* field, int index) {
  if (field->is_map()) {
    const Reflection* reflection = message.GetReflection();
    specific_field->map_entry2 =
        &reflection->GetRepeatedMessage(message, field, index);
  }
  specific_field->new_index = index;
}

MessageDifferencer::MapEntryKeyComparator::MapEntryKeyComparator(
    MessageDifferencer* message_differencer)
    : message_differencer_(message_differencer) {}

bool MessageDifferencer::MapEntryKeyComparator::IsMatch(
    const Message& message1, const Message& message2, int unpacked_any,
    const std::vector<SpecificField>& parent_fields) const {
  // Map entry has its key in the field with tag 1.  See the comment for
  // map_entry in MessageOptions.
  const FieldDescriptor* key = message1.GetDescriptor()->FindFieldByNumber(1);
  // If key is not present in message1 and we're doing partial comparison or if
  // map key is explicitly ignored treat the field as set instead,
  const bool treat_as_set =
      (message_differencer_->scope() == PARTIAL &&
       !message1.GetReflection()->HasField(message1, key)) ||
      message_differencer_->IsIgnored(message1, message2, key, parent_fields);

  std::vector<SpecificField> current_parent_fields(parent_fields);
  if (treat_as_set) {
    return message_differencer_->Compare(message1, message2, unpacked_any,
                                         &current_parent_fields);
  }
  return message_differencer_->CompareFieldValueUsingParentFields(
      message1, message2, unpacked_any, key, -1, -1, &current_parent_fields);
}

bool MessageDifferencer::Equals(const Message& message1,
                                const Message& message2) {
  MessageDifferencer differencer;

  return differencer.Compare(message1, message2);
}

bool MessageDifferencer::Equivalent(const Message& message1,
                                    const Message& message2) {
  MessageDifferencer differencer;
  differencer.set_message_field_comparison(MessageDifferencer::EQUIVALENT);

  return differencer.Compare(message1, message2);
}

bool MessageDifferencer::ApproximatelyEquals(const Message& message1,
                                             const Message& message2) {
  MessageDifferencer differencer;
  differencer.set_float_comparison(MessageDifferencer::APPROXIMATE);

  return differencer.Compare(message1, message2);
}

bool MessageDifferencer::ApproximatelyEquivalent(const Message& message1,
                                                 const Message& message2) {
  MessageDifferencer differencer;
  differencer.set_message_field_comparison(MessageDifferencer::EQUIVALENT);
  differencer.set_float_comparison(MessageDifferencer::APPROXIMATE);

  return differencer.Compare(message1, message2);
}

// ===========================================================================

MessageDifferencer::MessageDifferencer()
    : reporter_(NULL),
      message_field_comparison_(EQUAL),
      scope_(FULL),
      repeated_field_comparison_(AS_LIST),
      map_entry_key_comparator_(this),
      report_matches_(false),
      report_moves_(true),
      report_ignores_(true),
      output_string_(nullptr),
      match_indices_for_smart_list_callback_(
          MatchIndicesPostProcessorForSmartList) {}

MessageDifferencer::~MessageDifferencer() {
  for (MapKeyComparator* comparator : owned_key_comparators_) {
    delete comparator;
  }
}

void MessageDifferencer::set_field_comparator(FieldComparator* comparator) {
  ABSL_CHECK(comparator) << "Field comparator can't be NULL.";
  field_comparator_kind_ = kFCBase;
  field_comparator_.base = comparator;
}

void MessageDifferencer::set_field_comparator(
    DefaultFieldComparator* comparator) {
  ABSL_CHECK(comparator) << "Field comparator can't be NULL.";
  field_comparator_kind_ = kFCDefault;
  field_comparator_.default_impl = comparator;
}

void MessageDifferencer::set_message_field_comparison(
    MessageFieldComparison comparison) {
  message_field_comparison_ = comparison;
}

MessageDifferencer::MessageFieldComparison
MessageDifferencer::message_field_comparison() const {
  return message_field_comparison_;
}

void MessageDifferencer::set_scope(Scope scope) { scope_ = scope; }

MessageDifferencer::Scope MessageDifferencer::scope() const { return scope_; }

void MessageDifferencer::set_force_compare_no_presence(bool value) {
  force_compare_no_presence_ = value;
}

void MessageDifferencer::set_float_comparison(FloatComparison comparison) {
  default_field_comparator_.set_float_comparison(
      comparison == EXACT ? DefaultFieldComparator::EXACT
                          : DefaultFieldComparator::APPROXIMATE);
}

void MessageDifferencer::set_repeated_field_comparison(
    RepeatedFieldComparison comparison) {
  repeated_field_comparison_ = comparison;
}

MessageDifferencer::RepeatedFieldComparison
MessageDifferencer::repeated_field_comparison() const {
  return repeated_field_comparison_;
}

void MessageDifferencer::CheckRepeatedFieldComparisons(
    const FieldDescriptor* field,
    const RepeatedFieldComparison& new_comparison) {
  ABSL_CHECK(field->is_repeated())
      << "Field must be repeated: " << field->full_name();
  const MapKeyComparator* key_comparator = GetMapKeyComparator(field);
  ABSL_CHECK(key_comparator == NULL)
      << "Cannot treat this repeated field as both MAP and " << new_comparison
      << " for comparison.  Field name is: " << field->full_name();
}

void MessageDifferencer::TreatAsSet(const FieldDescriptor* field) {
  CheckRepeatedFieldComparisons(field, AS_SET);
  repeated_field_comparisons_[field] = AS_SET;
}

void MessageDifferencer::TreatAsSmartSet(const FieldDescriptor* field) {
  CheckRepeatedFieldComparisons(field, AS_SMART_SET);
  repeated_field_comparisons_[field] = AS_SMART_SET;
}

void MessageDifferencer::SetMatchIndicesForSmartListCallback(
    std::function<void(std::vector<int>*, std::vector<int>*)> callback) {
  match_indices_for_smart_list_callback_ = callback;
}

void MessageDifferencer::TreatAsList(const FieldDescriptor* field) {
  CheckRepeatedFieldComparisons(field, AS_LIST);
  repeated_field_comparisons_[field] = AS_LIST;
}

void MessageDifferencer::TreatAsSmartList(const FieldDescriptor* field) {
  CheckRepeatedFieldComparisons(field, AS_SMART_LIST);
  repeated_field_comparisons_[field] = AS_SMART_LIST;
}

void MessageDifferencer::TreatAsMap(const FieldDescriptor* field,
                                    const FieldDescriptor* key) {
  ABSL_CHECK_EQ(FieldDescriptor::CPPTYPE_MESSAGE, field->cpp_type())
      << "Field has to be message type.  Field name is: " << field->full_name();
  ABSL_CHECK(key->containing_type() == field->message_type())
      << key->full_name()
      << " must be a direct subfield within the repeated field "
      << field->full_name() << ", not " << key->containing_type()->full_name();
  ABSL_CHECK(repeated_field_comparisons_.find(field) ==
             repeated_field_comparisons_.end())
      << "Cannot treat the same field as both "
      << repeated_field_comparisons_[field]
      << " and MAP. Field name is: " << field->full_name();
  MapKeyComparator* key_comparator =
      new MultipleFieldsMapKeyComparator(this, key);
  owned_key_comparators_.push_back(key_comparator);
  map_field_key_comparator_[field] = key_comparator;
}

void MessageDifferencer::TreatAsMapWithMultipleFieldsAsKey(
    const FieldDescriptor* field,
    const std::vector<const FieldDescriptor*>& key_fields) {
  std::vector<std::vector<const FieldDescriptor*> > key_field_paths;
  for (const FieldDescriptor* key_filed : key_fields) {
    std::vector<const FieldDescriptor*> key_field_path;
    key_field_path.push_back(key_filed);
    key_field_paths.push_back(key_field_path);
  }
  TreatAsMapWithMultipleFieldPathsAsKey(field, key_field_paths);
}

void MessageDifferencer::TreatAsMapWithMultipleFieldPathsAsKey(
    const FieldDescriptor* field,
    const std::vector<std::vector<const FieldDescriptor*> >& key_field_paths) {
  ABSL_CHECK(field->is_repeated())
      << "Field must be repeated: " << field->full_name();
  ABSL_CHECK_EQ(FieldDescriptor::CPPTYPE_MESSAGE, field->cpp_type())
      << "Field has to be message type.  Field name is: " << field->full_name();
  for (const auto& key_field_path : key_field_paths) {
    for (size_t j = 0; j < key_field_path.size(); ++j) {
      const FieldDescriptor* parent_field =
          j == 0 ? field : key_field_path[j - 1];
      const FieldDescriptor* child_field = key_field_path[j];
      ABSL_CHECK(child_field->containing_type() == parent_field->message_type())
          << child_field->full_name()
          << " must be a direct subfield within the field: "
          << parent_field->full_name();
      if (j != 0) {
        ABSL_CHECK_EQ(FieldDescriptor::CPPTYPE_MESSAGE,
                      parent_field->cpp_type())
            << parent_field->full_name() << " has to be of type message.";
        ABSL_CHECK(!parent_field->is_repeated())
            << parent_field->full_name() << " cannot be a repeated field.";
      }
    }
  }
  ABSL_CHECK(repeated_field_comparisons_.find(field) ==
             repeated_field_comparisons_.end())
      << "Cannot treat the same field as both "
      << repeated_field_comparisons_[field]
      << " and MAP. Field name is: " << field->full_name();
  MapKeyComparator* key_comparator =
      new MultipleFieldsMapKeyComparator(this, key_field_paths);
  owned_key_comparators_.push_back(key_comparator);
  map_field_key_comparator_[field] = key_comparator;
}

void MessageDifferencer::TreatAsMapUsingKeyComparator(
    const FieldDescriptor* field, const MapKeyComparator* key_comparator) {
  ABSL_CHECK(field->is_repeated())
      << "Field must be repeated: " << field->full_name();
  ABSL_CHECK(repeated_field_comparisons_.find(field) ==
             repeated_field_comparisons_.end())
      << "Cannot treat the same field as both "
      << repeated_field_comparisons_[field]
      << " and MAP. Field name is: " << field->full_name();
  map_field_key_comparator_[field] = key_comparator;
}

void MessageDifferencer::AddIgnoreCriteria(
    std::unique_ptr<IgnoreCriteria> ignore_criteria) {
  ignore_criteria_.push_back(std::move(ignore_criteria));
}

void MessageDifferencer::IgnoreField(const FieldDescriptor* field) {
  ignored_fields_.insert(field);
}

void MessageDifferencer::SetFractionAndMargin(const FieldDescriptor* field,
                                              double fraction, double margin) {
  default_field_comparator_.SetFractionAndMargin(field, fraction, margin);
}

void MessageDifferencer::ReportDifferencesToString(std::string* output) {
  ABSL_DCHECK(output) << "Specified output string was NULL";

  output_string_ = output;
  output_string_->clear();
}

void MessageDifferencer::ReportDifferencesTo(Reporter* reporter) {
  // If an output string is set, clear it to prevent
  // it superseding the specified reporter.
  if (output_string_) {
    output_string_ = NULL;
  }

  reporter_ = reporter;
}

bool MessageDifferencer::FieldBefore(const FieldDescriptor* field1,
                                     const FieldDescriptor* field2) {
  // Handle sentinel values (i.e. make sure NULLs are always ordered
  // at the end of the list).
  if (field1 == NULL) {
    return false;
  }

  if (field2 == NULL) {
    return true;
  }

  // Always order fields by their tag number
  return (field1->number() < field2->number());
}

bool MessageDifferencer::Compare(const Message& message1,
                                 const Message& message2) {
  std::vector<SpecificField> parent_fields;
  force_compare_no_presence_fields_.clear();
  force_compare_failure_triggering_fields_.clear();

  bool result = false;
  // Setup the internal reporter if need be.
  if (output_string_) {
    io::StringOutputStream output_stream(output_string_);
    StreamReporter reporter(&output_stream);
    reporter.SetMessages(message1, message2);
    reporter_ = &reporter;
    result = Compare(message1, message2, false, &parent_fields);
    reporter_ = NULL;
  } else {
    result = Compare(message1, message2, false, &parent_fields);
  }
  return result;
}

bool MessageDifferencer::CompareWithFields(
    const Message& message1, const Message& message2,
    const std::vector<const FieldDescriptor*>& message1_fields_arg,
    const std::vector<const FieldDescriptor*>& message2_fields_arg) {
  if (message1.GetDescriptor() != message2.GetDescriptor()) {
    ABSL_DLOG(FATAL) << "Comparison between two messages with different "
                     << "descriptors.";
    return false;
  }

  std::vector<SpecificField> parent_fields;
  force_compare_no_presence_fields_.clear();
  force_compare_failure_triggering_fields_.clear();

  bool result = false;

  std::vector<const FieldDescriptor*> message1_fields(
      message1_fields_arg.size() + 1);
  std::vector<const FieldDescriptor*> message2_fields(
      message2_fields_arg.size() + 1);

  std::copy(message1_fields_arg.cbegin(), message1_fields_arg.cend(),
            message1_fields.begin());
  std::copy(message2_fields_arg.cbegin(), message2_fields_arg.cend(),
            message2_fields.begin());

  // Append sentinel values.
  message1_fields[message1_fields_arg.size()] = nullptr;
  message2_fields[message2_fields_arg.size()] = nullptr;

  std::sort(message1_fields.begin(), message1_fields.end(), FieldBefore);
  std::sort(message2_fields.begin(), message2_fields.end(), FieldBefore);

  // Setup the internal reporter if need be.
  if (output_string_) {
    io::StringOutputStream output_stream(output_string_);
    StreamReporter reporter(&output_stream);
    reporter_ = &reporter;
    result = CompareRequestedFieldsUsingSettings(
        message1, message2, false, message1_fields, message2_fields,
        &parent_fields);
    reporter_ = NULL;
  } else {
    result = CompareRequestedFieldsUsingSettings(
        message1, message2, false, message1_fields, message2_fields,
        &parent_fields);
  }

  return result;
}

bool MessageDifferencer::Compare(const Message& message1,
                                 const Message& message2, int unpacked_any,
                                 std::vector<SpecificField>* parent_fields) {
  const Descriptor* descriptor1 = message1.GetDescriptor();
  const Descriptor* descriptor2 = message2.GetDescriptor();
  if (descriptor1 != descriptor2) {
    ABSL_DLOG(FATAL) << "Comparison between two messages with different "
                     << "descriptors. " << descriptor1->full_name() << " vs "
                     << descriptor2->full_name();
    return false;
  }

  // Expand google.protobuf.Any payload if possible.
  if (descriptor1->full_name() == internal::kAnyFullTypeName) {
    std::unique_ptr<Message> data1;
    std::unique_ptr<Message> data2;
    if (unpack_any_field_.UnpackAny(message1, &data1) &&
        unpack_any_field_.UnpackAny(message2, &data2) &&
        data1->GetDescriptor() == data2->GetDescriptor()) {
      return Compare(*data1, *data2, unpacked_any + 1, parent_fields);
    }
    // If the Any payload is unparsable, or the payload types are different
    // between message1 and message2, fall through and treat Any as a regular
    // proto.
  }

  bool unknown_compare_result = true;
  // Ignore unknown fields in EQUIVALENT mode
  if (message_field_comparison_ != EQUIVALENT) {
    const Reflection* reflection1 = message1.GetReflection();
    const Reflection* reflection2 = message2.GetReflection();
    const UnknownFieldSet& unknown_field_set1 =
        reflection1->GetUnknownFields(message1);
    const UnknownFieldSet& unknown_field_set2 =
        reflection2->GetUnknownFields(message2);
    if (!CompareUnknownFields(message1, message2, unknown_field_set1,
                              unknown_field_set2, parent_fields)) {
      if (reporter_ == NULL) {
        return false;
      }
      unknown_compare_result = false;
    }
  }

  std::vector<const FieldDescriptor*> message1_fields =
      RetrieveFields(message1, true);
  std::vector<const FieldDescriptor*> message2_fields =
      RetrieveFields(message2, false);

  return CompareRequestedFieldsUsingSettings(message1, message2, unpacked_any,
                                             message1_fields, message2_fields,
                                             parent_fields) &&
         unknown_compare_result;
}

std::vector<const FieldDescriptor*> MessageDifferencer::RetrieveFields(
    const Message& message, bool base_message) {
  const Descriptor* descriptor = message.GetDescriptor();

  tmp_message_fields_.clear();
  tmp_message_fields_.reserve(descriptor->field_count() + 1);

  const Reflection* reflection = message.GetReflection();
  if (descriptor->options().map_entry()) {
    if (this->scope_ == PARTIAL && base_message) {
      reflection->ListFields(message, &tmp_message_fields_);
    } else {
      // Map entry fields are always considered present.
      for (int i = 0; i < descriptor->field_count(); i++) {
        tmp_message_fields_.push_back(descriptor->field(i));
      }
    }
  } else {
    reflection->ListFields(message, &tmp_message_fields_);
  }
  // Add sentinel values to deal with the
  // case where the number of the fields in
  // each list are different.
  tmp_message_fields_.push_back(nullptr);

  std::vector<const FieldDescriptor*> message_fields(
      tmp_message_fields_.begin(), tmp_message_fields_.end());

  return message_fields;
}

bool MessageDifferencer::CompareRequestedFieldsUsingSettings(
    const Message& message1, const Message& message2, int unpacked_any,
    const std::vector<const FieldDescriptor*>& message1_fields,
    const std::vector<const FieldDescriptor*>& message2_fields,
    std::vector<SpecificField>* parent_fields) {
  if (scope_ == FULL) {
    if (message_field_comparison_ == EQUIVALENT) {
      // We need to merge the field lists of both messages (i.e.
      // we are merely checking for a difference in field values,
      // rather than the addition or deletion of fields).
      std::vector<const FieldDescriptor*> fields_union =
          CombineFields(message1_fields, FULL, message2_fields, FULL);
      return CompareWithFieldsInternal(message1, message2, unpacked_any,
                                       fields_union, fields_union,
                                       parent_fields);
    } else {
      // Simple equality comparison, use the unaltered field lists.
      return CompareWithFieldsInternal(message1, message2, unpacked_any,
                                       message1_fields, message2_fields,
                                       parent_fields);
    }
  } else {
    if (message_field_comparison_ == EQUIVALENT) {
      // We use the list of fields for message1 for both messages when
      // comparing.  This way, extra fields in message2 are ignored,
      // and missing fields in message2 use their default value.
      return CompareWithFieldsInternal(message1, message2, unpacked_any,
                                       message1_fields, message1_fields,
                                       parent_fields);
    } else {
      // We need to consider the full list of fields for message1
      // but only the intersection for message2.  This way, any fields
      // only present in message2 will be ignored, but any fields only
      // present in message1 will be marked as a difference.
      std::vector<const FieldDescriptor*> fields_intersection =
          CombineFields(message1_fields, PARTIAL, message2_fields, PARTIAL);
      return CompareWithFieldsInternal(message1, message2, unpacked_any,
                                       message1_fields, fields_intersection,
                                       parent_fields);
    }
  }
}

std::vector<const FieldDescriptor*> MessageDifferencer::CombineFields(
    const std::vector<const FieldDescriptor*>& fields1, Scope fields1_scope,
    const std::vector<const FieldDescriptor*>& fields2, Scope fields2_scope) {
  size_t index1 = 0;
  size_t index2 = 0;

  tmp_message_fields_.clear();

  while (index1 < fields1.size() && index2 < fields2.size()) {
    const FieldDescriptor* field1 = fields1[index1];
    const FieldDescriptor* field2 = fields2[index2];

    if (FieldBefore(field1, field2)) {
      if (fields1_scope == FULL) {
        tmp_message_fields_.push_back(field1);
      }
      ++index1;
    } else if (FieldBefore(field2, field1)) {
      if (fields2_scope == FULL) {
        tmp_message_fields_.push_back(field2);
      } else if (fields2_scope == PARTIAL && force_compare_no_presence_ &&
                 !field2->has_presence() && !field2->is_repeated()) {
        // In order to make MessageDifferencer play nicely with no-presence
        // fields in unit tests, we want to check if the expected proto
        // (message1) has some fields which are set to their default value but
        // are not set to their default value in message2 (the actual message).
        // Those fields will appear in fields2 (since they have non default
        // value) but will not appear in fields1 (since they have the default
        // value or were never set).
        force_compare_no_presence_fields_.insert(field2);
        tmp_message_fields_.push_back(field2);
      }
      ++index2;
    } else {
      tmp_message_fields_.push_back(field1);
      ++index1;
      ++index2;
    }
  }

  tmp_message_fields_.push_back(nullptr);

  std::vector<const FieldDescriptor*> combined_fields(
      tmp_message_fields_.begin(), tmp_message_fields_.end());

  return combined_fields;
}

// We push an element via a NOINLINE function to avoid using stack space on
// the caller for a temporary SpecificField object. They are quite large.
static PROTOBUF_NOINLINE MessageDifferencer::SpecificField& PushSpecificField(
    std::vector<MessageDifferencer::SpecificField>* fields) {
  fields->emplace_back();
  return fields->back();
}

bool MessageDifferencer::CompareWithFieldsInternal(
    const Message& message1, const Message& message2, int unpacked_any,
    const std::vector<const FieldDescriptor*>& message1_fields,
    const std::vector<const FieldDescriptor*>& message2_fields,
    std::vector<SpecificField>* parent_fields) {
  bool isDifferent = false;
  int field_index1 = 0;
  int field_index2 = 0;

  const Reflection* reflection1 = message1.GetReflection();
  const Reflection* reflection2 = message2.GetReflection();

  while (true) {
    const FieldDescriptor* field1 = message1_fields[field_index1];
    const FieldDescriptor* field2 = message2_fields[field_index2];

    // Once we have reached sentinel values, we are done the comparison.
    if (field1 == NULL && field2 == NULL) {
      break;
    }

    // Check for differences in the field itself.
    if (FieldBefore(field1, field2)) {
      // Field 1 is not in the field list for message 2.
      if (IsIgnored(message1, message2, field1, *parent_fields)) {
        // We are ignoring field1. Report the ignore and move on to
        // the next field in message1_fields.
        if (reporter_ != NULL) {
          SpecificField& specific_field = PushSpecificField(parent_fields);
          specific_field.message1 = &message1;
          specific_field.message2 = &message2;
          specific_field.unpacked_any = unpacked_any;
          specific_field.field = field1;
          if (report_ignores_) {
            reporter_->ReportIgnored(message1, message2, *parent_fields);
          }
          parent_fields->pop_back();
        }
        ++field_index1;
        continue;
      }

      if (reporter_ != NULL) {
        assert(field1 != NULL);
        int count = field1->is_repeated()
                        ? reflection1->FieldSize(message1, field1)
                        : 1;

        for (int i = 0; i < count; ++i) {
          SpecificField& specific_field = PushSpecificField(parent_fields);
          specific_field.message1 = &message1;
          specific_field.message2 = &message2;
          specific_field.unpacked_any = unpacked_any;
          specific_field.field = field1;
          if (field1->is_repeated()) {
            AddSpecificIndex(&specific_field, message1, field1, i);
          } else {
            specific_field.index = -1;
          }

          reporter_->ReportDeleted(message1, message2, *parent_fields);
          parent_fields->pop_back();
        }

        isDifferent = true;
      } else {
        return false;
      }

      ++field_index1;
      continue;
    } else if (FieldBefore(field2, field1)) {
      const bool ignore_field =
          IsIgnored(message1, message2, field2, *parent_fields);
      if (!ignore_field && force_compare_no_presence_fields_.contains(field2)) {
        force_compare_failure_triggering_fields_.insert(field2->full_name());
      }

      // Field 2 is not in the field list for message 1.
      if (ignore_field) {
        // We are ignoring field2. Report the ignore and move on to
        // the next field in message2_fields.
        if (reporter_ != NULL) {
          SpecificField& specific_field = PushSpecificField(parent_fields);
          specific_field.message1 = &message1;
          specific_field.message2 = &message2;
          specific_field.unpacked_any = unpacked_any;
          specific_field.field = field2;
          if (report_ignores_) {
            reporter_->ReportIgnored(message1, message2, *parent_fields);
          }
          parent_fields->pop_back();
        }
        ++field_index2;
        continue;
      }

      if (reporter_ != NULL) {
        int count = field2->is_repeated()
                        ? reflection2->FieldSize(message2, field2)
                        : 1;

        for (int i = 0; i < count; ++i) {
          SpecificField& specific_field = PushSpecificField(parent_fields);
          specific_field.message1 = &message1,
          specific_field.message2 = &message2;
          specific_field.unpacked_any = unpacked_any;
          specific_field.field = field2;
          if (field2->is_repeated()) {
            specific_field.index = i;
            AddSpecificNewIndex(&specific_field, message2, field2, i);
          } else {
            specific_field.index = -1;
            specific_field.new_index = -1;
          }

          specific_field.forced_compare_no_presence_ =
              force_compare_no_presence_ &&
              force_compare_no_presence_fields_.contains(specific_field.field);

          reporter_->ReportAdded(message1, message2, *parent_fields);
          parent_fields->pop_back();
        }

        isDifferent = true;
      } else {
        return false;
      }

      ++field_index2;
      continue;
    }

    // By this point, field1 and field2 are guaranteed to point to the same
    // field, so we can now compare the values.
    if (IsIgnored(message1, message2, field1, *parent_fields)) {
      // Ignore this field. Report and move on.
      if (reporter_ != NULL) {
        SpecificField& specific_field = PushSpecificField(parent_fields);
        specific_field.message1 = &message1;
        specific_field.message2 = &message2;
        specific_field.unpacked_any = unpacked_any;
        specific_field.field = field1;
        if (report_ignores_) {
          reporter_->ReportIgnored(message1, message2, *parent_fields);
        }
        parent_fields->pop_back();
      }

      ++field_index1;
      ++field_index2;
      continue;
    }

    bool fieldDifferent = false;
    assert(field1 != NULL);
    if (field1->is_map()) {
      fieldDifferent = !CompareMapField(message1, message2, unpacked_any,
                                        field1, parent_fields);
    } else if (field1->is_repeated()) {
      fieldDifferent = !CompareRepeatedField(message1, message2, unpacked_any,
                                             field1, parent_fields);
    } else {
      fieldDifferent = !CompareFieldValueUsingParentFields(
          message1, message2, unpacked_any, field1, -1, -1, parent_fields);

      if (force_compare_no_presence_fields_.contains(field1)) {
        force_compare_failure_triggering_fields_.insert(field1->full_name());
      }

      if (reporter_ != nullptr) {
        SpecificField& specific_field = PushSpecificField(parent_fields);
        specific_field.message1 = &message1;
        specific_field.message2 = &message2;
        specific_field.unpacked_any = unpacked_any;
        specific_field.field = field1;
        specific_field.forced_compare_no_presence_ =
            force_compare_no_presence_ &&
            force_compare_no_presence_fields_.contains(field1);

        if (fieldDifferent) {
          reporter_->ReportModified(message1, message2, *parent_fields);
          isDifferent = true;
        } else if (report_matches_) {
          reporter_->ReportMatched(message1, message2, *parent_fields);
        }
        parent_fields->pop_back();
      }
    }
    if (fieldDifferent) {
      if (reporter_ == nullptr) return false;
      isDifferent = true;
    }
    // Increment the field indices.
    ++field_index1;
    ++field_index2;
  }

  return !isDifferent;
}

bool MessageDifferencer::IsMatch(
    const FieldDescriptor* repeated_field,
    const MapKeyComparator* key_comparator, const Message* message1,
    const Message* message2, int unpacked_any,
    const std::vector<SpecificField>& parent_fields, Reporter* reporter,
    int index1, int index2) {
  std::vector<SpecificField> current_parent_fields(parent_fields);
  if (repeated_field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) {
    return CompareFieldValueUsingParentFields(
        *message1, *message2, unpacked_any, repeated_field, index1, index2,
        &current_parent_fields);
  }
  // Back up the Reporter and output_string_.  They will be reset in the
  // following code.
  Reporter* backup_reporter = reporter_;
  std::string* output_string = output_string_;
  reporter_ = reporter;
  output_string_ = NULL;
  bool match;

  if (key_comparator == NULL) {
    match = CompareFieldValueUsingParentFields(
        *message1, *message2, unpacked_any, repeated_field, index1, index2,
        &current_parent_fields);
  } else {
    const Reflection* reflection1 = message1->GetReflection();
    const Reflection* reflection2 = message2->GetReflection();
    const Message& m1 =
        reflection1->GetRepeatedMessage(*message1, repeated_field, index1);
    const Message& m2 =
        reflection2->GetRepeatedMessage(*message2, repeated_field, index2);
    SpecificField specific_field;
    specific_field.message1 = message1;
    specific_field.message2 = message2;
    specific_field.unpacked_any = unpacked_any;
    specific_field.field = repeated_field;
    if (repeated_field->is_map()) {
      specific_field.map_entry1 = &m1;
      specific_field.map_entry2 = &m2;
    }
    specific_field.index = index1;
    specific_field.new_index = index2;
    current_parent_fields.push_back(specific_field);
    match = key_comparator->IsMatch(m1, m2, false, current_parent_fields);
  }

  reporter_ = backup_reporter;
  output_string_ = output_string;
  return match;
}

bool MessageDifferencer::CompareMapFieldByMapReflection(
    const Message& message1, const Message& message2, int unpacked_any,
    const FieldDescriptor* map_field, std::vector<SpecificField>* parent_fields,
    DefaultFieldComparator* comparator) {
  ABSL_DCHECK_EQ(nullptr, reporter_);
  ABSL_DCHECK(map_field->is_map());
  ABSL_DCHECK(map_field_key_comparator_.find(map_field) ==
              map_field_key_comparator_.end());
  ABSL_DCHECK_EQ(repeated_field_comparison_, AS_LIST);
  const Reflection* reflection1 = message1.GetReflection();
  const Reflection* reflection2 = message2.GetReflection();
  const int count1 = reflection1->MapSize(message1, map_field);
  const int count2 = reflection2->MapSize(message2, map_field);
  const bool treated_as_subset = IsTreatedAsSubset(map_field);
  if (count1 != count2 && !treated_as_subset) {
    return false;
  }
  if (count1 > count2) {
    return false;
  }

  // First pass: check whether the same keys are present.
  for (MapIterator it = reflection1->MapBegin(const_cast<Message*>(&message1),
                                              map_field),
                   it_end = reflection1->MapEnd(const_cast<Message*>(&message1),
                                                map_field);
       it != it_end; ++it) {
    if (!reflection2->ContainsMapKey(message2, map_field, it.GetKey())) {
      return false;
    }
  }

  // Second pass: compare values for matching keys.
  const FieldDescriptor* val_des = map_field->message_type()->map_value();
  switch (val_des->cpp_type()) {
#define HANDLE_TYPE(CPPTYPE, METHOD, COMPAREMETHOD)                           \
  case FieldDescriptor::CPPTYPE_##CPPTYPE: {                                  \
    for (MapIterator it = reflection1->MapBegin(                              \
                         const_cast<Message*>(&message1), map_field),         \
                     it_end = reflection1->MapEnd(                            \
                         const_cast<Message*>(&message1), map_field);         \
         it != it_end; ++it) {                                                \
      MapValueConstRef value2;                                                \
      reflection2->LookupMapValue(message2, map_field, it.GetKey(), &value2); \
      if (!comparator->Compare##COMPAREMETHOD(*val_des,                       \
                                              it.GetValueRef().Get##METHOD(), \
                                              value2.Get##METHOD())) {        \
        return false;                                                         \
      }                                                                       \
    }                                                                         \
    break;                                                                    \
  }
    HANDLE_TYPE(INT32, Int32Value, Int32);
    HANDLE_TYPE(INT64, Int64Value, Int64);
    HANDLE_TYPE(UINT32, UInt32Value, UInt32);
    HANDLE_TYPE(UINT64, UInt64Value, UInt64);
    HANDLE_TYPE(DOUBLE, DoubleValue, Double);
    HANDLE_TYPE(FLOAT, FloatValue, Float);
    HANDLE_TYPE(BOOL, BoolValue, Bool);
    HANDLE_TYPE(STRING, StringValue, String);
    HANDLE_TYPE(ENUM, EnumValue, Int32);
#undef HANDLE_TYPE
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      for (MapIterator it = reflection1->MapBegin(
               const_cast<Message*>(&message1), map_field);
           it !=
           reflection1->MapEnd(const_cast<Message*>(&message1), map_field);
           ++it) {
        if (!reflection2->ContainsMapKey(message2, map_field, it.GetKey())) {
          return false;
        }
        bool compare_result;
        MapValueConstRef value2;
        reflection2->LookupMapValue(message2, map_field, it.GetKey(), &value2);
        // Append currently compared field to the end of parent_fields.
        SpecificField specific_value_field;
        specific_value_field.message1 = &message1;
        specific_value_field.message2 = &message2;
        specific_value_field.unpacked_any = unpacked_any;
        specific_value_field.field = val_des;
        parent_fields->push_back(specific_value_field);
        compare_result =
            Compare(it.GetValueRef().GetMessageValue(),
                    value2.GetMessageValue(), false, parent_fields);
        parent_fields->pop_back();
        if (!compare_result) {
          return false;
        }
      }
      break;
    }
  }
  return true;
}

bool MessageDifferencer::CompareMapField(
    const Message& message1, const Message& message2, int unpacked_any,
    const FieldDescriptor* repeated_field,
    std::vector<SpecificField>* parent_fields) {
  ABSL_DCHECK(repeated_field->is_map());

  // the input FieldDescriptor is guaranteed to be repeated field.
  const Reflection* reflection1 = message1.GetReflection();
  const Reflection* reflection2 = message2.GetReflection();

  // When both map fields are on map, do not sync to repeated field.
  if (reflection1->GetMapData(message1, repeated_field)->IsMapValid() &&
      reflection2->GetMapData(message2, repeated_field)->IsMapValid() &&
      // TODO: Add support for reporter
      reporter_ == nullptr &&
      // Users didn't set custom map field key comparator
      map_field_key_comparator_.find(repeated_field) ==
          map_field_key_comparator_.end() &&
      // Users didn't set repeated field comparison
      repeated_field_comparison_ == AS_LIST &&
      // Users didn't set their own FieldComparator implementation
      field_comparator_kind_ == kFCDefault) {
    const FieldDescriptor* key_des = repeated_field->message_type()->map_key();
    const FieldDescriptor* val_des =
        repeated_field->message_type()->map_value();
    std::vector<SpecificField> current_parent_fields(*parent_fields);
    SpecificField specific_field;
    specific_field.message1 = &message1;
    specific_field.message2 = &message2;
    specific_field.unpacked_any = unpacked_any;
    specific_field.field = repeated_field;
    current_parent_fields.push_back(specific_field);
    if (!IsIgnored(message1, message2, key_des, current_parent_fields) &&
        !IsIgnored(message1, message2, val_des, current_parent_fields)) {
      return CompareMapFieldByMapReflection(
          message1, message2, unpacked_any, repeated_field,
          &current_parent_fields, field_comparator_.default_impl);
    }
  }

  return CompareRepeatedRep(message1, message2, unpacked_any, repeated_field,
                            parent_fields);
}

bool MessageDifferencer::CompareRepeatedField(
    const Message& message1, const Message& message2, int unpacked_any,
    const FieldDescriptor* repeated_field,
    std::vector<SpecificField>* parent_fields) {
  ABSL_DCHECK(!repeated_field->is_map());
  return CompareRepeatedRep(message1, message2, unpacked_any, repeated_field,
                            parent_fields);
}

bool MessageDifferencer::CompareRepeatedRep(
    const Message& message1, const Message& message2, int unpacked_any,
    const FieldDescriptor* repeated_field,
    std::vector<SpecificField>* parent_fields) {
  // the input FieldDescriptor is guaranteed to be repeated field.
  ABSL_DCHECK(repeated_field->is_repeated());
  const Reflection* reflection1 = message1.GetReflection();
  const Reflection* reflection2 = message2.GetReflection();

  const int count1 = reflection1->FieldSize(message1, repeated_field);
  const int count2 = reflection2->FieldSize(message2, repeated_field);
  const bool treated_as_subset = IsTreatedAsSubset(repeated_field);

  // If the field is not treated as subset and no detailed reports is needed,
  // we do a quick check on the number of the elements to avoid unnecessary
  // comparison.
  if (count1 != count2 && reporter_ == NULL && !treated_as_subset) {
    return false;
  }
  // A match can never be found if message1 has more items than message2.
  if (count1 > count2 && reporter_ == NULL) {
    return false;
  }

  // These two list are used for store the index of the correspondent
  // element in peer repeated field.
  std::vector<int> match_list1;
  std::vector<int> match_list2;

  const MapKeyComparator* key_comparator = GetMapKeyComparator(repeated_field);
  bool smart_list = IsTreatedAsSmartList(repeated_field);
  bool simple_list = key_comparator == nullptr &&
                     !IsTreatedAsSet(repeated_field) &&
                     !IsTreatedAsSmartSet(repeated_field) && !smart_list;

  // For simple lists, we avoid matching repeated field indices, saving the
  // memory allocations that would otherwise be needed for match_list1 and
  // match_list2.
  if (!simple_list) {
    // Try to match indices of the repeated fields. Return false if match fails.
    if (!MatchRepeatedFieldIndices(
            message1, message2, unpacked_any, repeated_field, key_comparator,
            *parent_fields, &match_list1, &match_list2) &&
        reporter_ == nullptr) {
      return false;
    }
  }

  bool fieldDifferent = false;
  SpecificField specific_field;
  specific_field.message1 = &message1;
  specific_field.message2 = &message2;
  specific_field.unpacked_any = unpacked_any;
  specific_field.field = repeated_field;

  // At this point, we have already matched pairs of fields (with the reporting
  // to be done later). Now to check if the paired elements are different.
  int next_unmatched_index = 0;
  for (int i = 0; i < count1; i++) {
    if (simple_list && i >= count2) {
      break;
    }
    if (!simple_list && match_list1[i] == -1) {
      if (smart_list) {
        if (reporter_ == nullptr) return false;
        AddSpecificIndex(&specific_field, message1, repeated_field, i);
        parent_fields->push_back(specific_field);
        reporter_->ReportDeleted(message1, message2, *parent_fields);
        parent_fields->pop_back();
        fieldDifferent = true;
        // Use -2 to mark this element has been reported.
        match_list1[i] = -2;
      }
      continue;
    }
    if (smart_list) {
      for (int j = next_unmatched_index; j < match_list1[i]; ++j) {
        ABSL_CHECK_LE(0, j);
        if (reporter_ == nullptr) return false;
        specific_field.index = j;
        AddSpecificNewIndex(&specific_field, message2, repeated_field, j);
        parent_fields->push_back(specific_field);
        reporter_->ReportAdded(message1, message2, *parent_fields);
        parent_fields->pop_back();
        fieldDifferent = true;
        // Use -2 to mark this element has been reported.
        match_list2[j] = -2;
      }
    }
    AddSpecificIndex(&specific_field, message1, repeated_field, i);
    if (simple_list) {
      AddSpecificNewIndex(&specific_field, message2, repeated_field, i);
    } else {
      AddSpecificNewIndex(&specific_field, message2, repeated_field,
                          match_list1[i]);
      next_unmatched_index = match_list1[i] + 1;
    }

    const bool result = CompareFieldValueUsingParentFields(
        message1, message2, unpacked_any, repeated_field, i,
        specific_field.new_index, parent_fields);

    // If we have found differences, either report them or terminate if
    // no reporter is present. Note that ReportModified, ReportMoved, and
    // ReportMatched are all mutually exclusive.
    if (!result) {
      if (reporter_ == NULL) return false;
      parent_fields->push_back(specific_field);
      reporter_->ReportModified(message1, message2, *parent_fields);
      parent_fields->pop_back();
      fieldDifferent = true;
    } else if (reporter_ != NULL &&
               specific_field.index != specific_field.new_index &&
               !specific_field.field->is_map() && report_moves_) {
      parent_fields->push_back(specific_field);
      reporter_->ReportMoved(message1, message2, *parent_fields);
      parent_fields->pop_back();
    } else if (report_matches_ && reporter_ != NULL) {
      parent_fields->push_back(specific_field);
      reporter_->ReportMatched(message1, message2, *parent_fields);
      parent_fields->pop_back();
    }
  }

  // Report any remaining additions or deletions.
  for (int i = 0; i < count2; ++i) {
    if (!simple_list && match_list2[i] != -1) continue;
    if (simple_list && i < count1) continue;
    if (!treated_as_subset) {
      fieldDifferent = true;
    }

    if (reporter_ == NULL) continue;
    specific_field.index = i;
    AddSpecificNewIndex(&specific_field, message2, repeated_field, i);
    parent_fields->push_back(specific_field);
    reporter_->ReportAdded(message1, message2, *parent_fields);
    parent_fields->pop_back();
  }

  for (int i = 0; i < count1; ++i) {
    if (!simple_list && match_list1[i] != -1) continue;
    if (simple_list && i < count2) continue;
    assert(reporter_ != NULL);
    AddSpecificIndex(&specific_field, message1, repeated_field, i);
    parent_fields->push_back(specific_field);
    reporter_->ReportDeleted(message1, message2, *parent_fields);
    parent_fields->pop_back();
    fieldDifferent = true;
  }
  return !fieldDifferent;
}

bool MessageDifferencer::CompareFieldValue(const Message& message1,
                                           const Message& message2,
                                           int unpacked_any,
                                           const FieldDescriptor* field,
                                           int index1, int index2) {
  return CompareFieldValueUsingParentFields(message1, message2, unpacked_any,
                                            field, index1, index2, nullptr);
}

bool MessageDifferencer::CompareFieldValueUsingParentFields(
    const Message& message1, const Message& message2, int unpacked_any,
    const FieldDescriptor* field, int index1, int index2,
    std::vector<SpecificField>* parent_fields) {
  FieldContext field_context(parent_fields);
  FieldComparator::ComparisonResult result = GetFieldComparisonResult(
      message1, message2, field, index1, index2, &field_context);

  if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
      result == FieldComparator::RECURSE) {
    // Get the nested messages and compare them using one of the Compare
    // methods.
    const Reflection* reflection1 = message1.GetReflection();
    const Reflection* reflection2 = message2.GetReflection();
    const Message& m1 =
        field->is_repeated()
            ? reflection1->GetRepeatedMessage(message1, field, index1)
            : reflection1->GetMessage(message1, field);
    const Message& m2 =
        field->is_repeated()
            ? reflection2->GetRepeatedMessage(message2, field, index2)
            : reflection2->GetMessage(message2, field);

    // parent_fields is used in calls to Reporter methods.
    if (parent_fields != NULL) {
      // Append currently compared field to the end of parent_fields.
      SpecificField& specific_field = PushSpecificField(parent_fields);
      specific_field.message1 = &message1;
      specific_field.message2 = &message2;
      specific_field.unpacked_any = unpacked_any;
      specific_field.field = field;
      AddSpecificIndex(&specific_field, message1, field, index1);
      AddSpecificNewIndex(&specific_field, message2, field, index2);
      const bool compare_result = Compare(m1, m2, false, parent_fields);
      parent_fields->pop_back();
      return compare_result;
    } else {
      // Recreates parent_fields as if m1 and m2 had no parents.
      return Compare(m1, m2);
    }
  } else {
    return (result == FieldComparator::SAME);
  }
}

bool MessageDifferencer::CheckPathChanged(
    const std::vector<SpecificField>& field_path) {
  for (const SpecificField& specific_field : field_path) {
    // Don't check indexes for map entries -- maps are unordered.
    if (specific_field.field != nullptr && specific_field.field->is_map())
      continue;
    if (specific_field.index != specific_field.new_index) return true;
  }
  return false;
}

bool MessageDifferencer::IsTreatedAsSet(const FieldDescriptor* field) {
  if (!field->is_repeated()) return false;
  if (repeated_field_comparisons_.find(field) !=
      repeated_field_comparisons_.end()) {
    return repeated_field_comparisons_[field] == AS_SET;
  }
  return GetMapKeyComparator(field) == nullptr &&
         repeated_field_comparison_ == AS_SET;
}

bool MessageDifferencer::IsTreatedAsSmartSet(const FieldDescriptor* field) {
  if (!field->is_repeated()) return false;
  if (repeated_field_comparisons_.find(field) !=
      repeated_field_comparisons_.end()) {
    return repeated_field_comparisons_[field] == AS_SMART_SET;
  }
  return GetMapKeyComparator(field) == nullptr &&
         repeated_field_comparison_ == AS_SMART_SET;
}

bool MessageDifferencer::IsTreatedAsSmartList(const FieldDescriptor* field) {
  if (!field->is_repeated()) return false;
  if (repeated_field_comparisons_.find(field) !=
      repeated_field_comparisons_.end()) {
    return repeated_field_comparisons_[field] == AS_SMART_LIST;
  }
  return GetMapKeyComparator(field) == nullptr &&
         repeated_field_comparison_ == AS_SMART_LIST;
}

bool MessageDifferencer::IsTreatedAsSubset(const FieldDescriptor* field) {
  return scope_ == PARTIAL &&
         (IsTreatedAsSet(field) || GetMapKeyComparator(field) != NULL);
}

bool MessageDifferencer::IsIgnored(
    const Message& message1, const Message& message2,
    const FieldDescriptor* field,
    const std::vector<SpecificField>& parent_fields) {
  if (ignored_fields_.find(field) != ignored_fields_.end()) {
    return true;
  }
  for (const auto& criteria : ignore_criteria_) {
    if (criteria->IsIgnored(message1, message2, field, parent_fields)) {
      return true;
    }
  }
  return false;
}

bool MessageDifferencer::IsUnknownFieldIgnored(
    const Message& message1, const Message& message2,
    const SpecificField& field,
    const std::vector<SpecificField>& parent_fields) {
  for (const auto& criteria : ignore_criteria_) {
    if (criteria->IsUnknownFieldIgnored(message1, message2, field,
                                        parent_fields)) {
      return true;
    }
  }
  return false;
}

const MessageDifferencer::MapKeyComparator*
MessageDifferencer ::GetMapKeyComparator(const FieldDescriptor* field) const {
  if (!field->is_repeated()) return nullptr;
  auto it = map_field_key_comparator_.find(field);
  if (it != map_field_key_comparator_.end()) {
    return it->second;
  }
  if (field->is_map()) {
    // field cannot already be treated as list or set since TreatAsList() and
    // TreatAsSet() call GetMapKeyComparator() and fail if it returns non-NULL.
    return &map_entry_key_comparator_;
  }
  return nullptr;
}

namespace {

typedef std::pair<int, const UnknownField*> IndexUnknownFieldPair;

struct UnknownFieldOrdering {
  inline bool operator()(const IndexUnknownFieldPair& a,
                         const IndexUnknownFieldPair& b) const {
    if (a.second->number() < b.second->number()) return true;
    if (a.second->number() > b.second->number()) return false;
    return a.second->type() < b.second->type();
  }
};

}  // namespace

bool MessageDifferencer::UnpackAnyField::UnpackAny(
    const Message& any, std::unique_ptr<Message>* data) {
  const Reflection* reflection = any.GetReflection();
  const FieldDescriptor* type_url_field;
  const FieldDescriptor* value_field;
  if (!internal::GetAnyFieldDescriptors(any, &type_url_field, &value_field)) {
    return false;
  }
  const std::string& type_url = reflection->GetString(any, type_url_field);
  std::string full_type_name;
  if (!internal::ParseAnyTypeUrl(type_url, &full_type_name)) {
    return false;
  }

  const Descriptor* desc =
      any.GetDescriptor()->file()->pool()->FindMessageTypeByName(
          full_type_name);
  if (desc == NULL) {
    return false;
  }

  if (dynamic_message_factory_ == NULL) {
    dynamic_message_factory_.reset(new DynamicMessageFactory());
  }
  data->reset(dynamic_message_factory_->GetPrototype(desc)->New());
  std::string serialized_value = reflection->GetString(any, value_field);
  if (!(*data)->ParsePartialFromString(serialized_value)) {
    ABSL_DLOG(ERROR) << "Failed to parse value for " << full_type_name;
    return false;
  }
  return true;
}

bool MessageDifferencer::CompareUnknownFields(
    const Message& message1, const Message& message2,
    const UnknownFieldSet& unknown_field_set1,
    const UnknownFieldSet& unknown_field_set2,
    std::vector<SpecificField>* parent_field) {
  // Ignore unknown fields in EQUIVALENT mode.
  if (message_field_comparison_ == EQUIVALENT) return true;

  if (unknown_field_set1.empty() && unknown_field_set2.empty()) {
    return true;
  }

  bool is_different = false;

  // We first sort the unknown fields by field number and type (in other words,
  // in tag order), making sure to preserve ordering of values with the same
  // tag.  This allows us to report only meaningful differences between the
  // two sets -- that is, differing values for the same tag.  We use
  // IndexUnknownFieldPairs to keep track of the field's original index for
  // reporting purposes.
  std::vector<IndexUnknownFieldPair> fields1;  // unknown_field_set1, sorted
  std::vector<IndexUnknownFieldPair> fields2;  // unknown_field_set2, sorted
  fields1.reserve(unknown_field_set1.field_count());
  fields2.reserve(unknown_field_set2.field_count());

  for (int i = 0; i < unknown_field_set1.field_count(); i++) {
    fields1.push_back(std::make_pair(i, &unknown_field_set1.field(i)));
  }
  for (int i = 0; i < unknown_field_set2.field_count(); i++) {
    fields2.push_back(std::make_pair(i, &unknown_field_set2.field(i)));
  }

  UnknownFieldOrdering is_before;
  std::stable_sort(fields1.begin(), fields1.end(), is_before);
  std::stable_sort(fields2.begin(), fields2.end(), is_before);

  // In order to fill in SpecificField::index, we have to keep track of how
  // many values we've seen with the same field number and type.
  // current_repeated points at the first field in this range, and
  // current_repeated_start{1,2} are the indexes of the first field in the
  // range within fields1 and fields2.
  const UnknownField* current_repeated = NULL;
  int current_repeated_start1 = 0;
  int current_repeated_start2 = 0;

  // Now that we have two sorted lists, we can detect fields which appear only
  // in one list or the other by traversing them simultaneously.
  size_t index1 = 0;
  size_t index2 = 0;
  while (index1 < fields1.size() || index2 < fields2.size()) {
    enum {
      ADDITION,
      DELETION,
      MODIFICATION,
      COMPARE_GROUPS,
      NO_CHANGE
    } change_type;

    // focus_field is the field we're currently reporting on.  (In the case
    // of a modification, it's the field on the left side.)
    const UnknownField* focus_field;
    bool match = false;

    if (index2 == fields2.size() ||
        (index1 < fields1.size() &&
         is_before(fields1[index1], fields2[index2]))) {
      // fields1[index1] is not present in fields2.
      change_type = DELETION;
      focus_field = fields1[index1].second;
    } else if (index1 == fields1.size() ||
               is_before(fields2[index2], fields1[index1])) {
      // fields2[index2] is not present in fields1.
      if (scope_ == PARTIAL) {
        // Ignore.
        ++index2;
        continue;
      }
      change_type = ADDITION;
      focus_field = fields2[index2].second;
    } else {
      // Field type and number are the same.  See if the values differ.
      change_type = MODIFICATION;
      focus_field = fields1[index1].second;

      switch (focus_field->type()) {
        case UnknownField::TYPE_VARINT:
          match = fields1[index1].second->varint() ==
                  fields2[index2].second->varint();
          break;
        case UnknownField::TYPE_FIXED32:
          match = fields1[index1].second->fixed32() ==
                  fields2[index2].second->fixed32();
          break;
        case UnknownField::TYPE_FIXED64:
          match = fields1[index1].second->fixed64() ==
                  fields2[index2].second->fixed64();
          break;
        case UnknownField::TYPE_LENGTH_DELIMITED:
          match = fields1[index1].second->length_delimited() ==
                  fields2[index2].second->length_delimited();
          break;
        case UnknownField::TYPE_GROUP:
          // We must deal with this later, after building the SpecificField.
          change_type = COMPARE_GROUPS;
          break;
      }
      if (match && change_type != COMPARE_GROUPS) {
        change_type = NO_CHANGE;
      }
    }

    if (current_repeated == NULL ||
        focus_field->number() != current_repeated->number() ||
        focus_field->type() != current_repeated->type()) {
      // We've started a new repeated field.
      current_repeated = focus_field;
      current_repeated_start1 = index1;
      current_repeated_start2 = index2;
    }

    if (change_type == NO_CHANGE && reporter_ == NULL) {
      // Fields were already compared and matched and we have no reporter.
      ++index1;
      ++index2;
      continue;
    }

    // Build the SpecificField.  This is slightly complicated.
    SpecificField specific_field;
    specific_field.message1 = &message1;
    specific_field.message2 = &message2;
    specific_field.unknown_field_number = focus_field->number();
    specific_field.unknown_field_type = focus_field->type();

    specific_field.unknown_field_set1 = &unknown_field_set1;
    specific_field.unknown_field_set2 = &unknown_field_set2;

    if (change_type != ADDITION) {
      specific_field.unknown_field_index1 = fields1[index1].first;
    }
    if (change_type != DELETION) {
      specific_field.unknown_field_index2 = fields2[index2].first;
    }

    // Calculate the field index.
    if (change_type == ADDITION) {
      specific_field.index = index2 - current_repeated_start2;
      specific_field.new_index = index2 - current_repeated_start2;
    } else {
      specific_field.index = index1 - current_repeated_start1;
      specific_field.new_index = index2 - current_repeated_start2;
    }

    if (IsUnknownFieldIgnored(message1, message2, specific_field,
                              *parent_field)) {
      if (report_ignores_ && reporter_ != NULL) {
        parent_field->push_back(specific_field);
        reporter_->ReportUnknownFieldIgnored(message1, message2, *parent_field);
        parent_field->pop_back();
      }
      if (change_type != ADDITION) ++index1;
      if (change_type != DELETION) ++index2;
      continue;
    }

    if (change_type == ADDITION || change_type == DELETION ||
        change_type == MODIFICATION) {
      if (reporter_ == NULL) {
        // We found a difference and we have no reporter.
        return false;
      }
      is_different = true;
    }

    parent_field->push_back(specific_field);

    switch (change_type) {
      case ADDITION:
        reporter_->ReportAdded(message1, message2, *parent_field);
        ++index2;
        break;
      case DELETION:
        reporter_->ReportDeleted(message1, message2, *parent_field);
        ++index1;
        break;
      case MODIFICATION:
        reporter_->ReportModified(message1, message2, *parent_field);
        ++index1;
        ++index2;
        break;
      case COMPARE_GROUPS:
        if (!CompareUnknownFields(
                message1, message2, fields1[index1].second->group(),
                fields2[index2].second->group(), parent_field)) {
          if (reporter_ == NULL) return false;
          is_different = true;
          reporter_->ReportModified(message1, message2, *parent_field);
        }
        ++index1;
        ++index2;
        break;
      case NO_CHANGE:
        ++index1;
        ++index2;
        if (report_matches_) {
          reporter_->ReportMatched(message1, message2, *parent_field);
        }
    }

    parent_field->pop_back();
  }

  return !is_different;
}

namespace {

// Find maximum bipartite matching using the argumenting path algorithm.
class MaximumMatcher {
 public:
  typedef std::function<bool(int, int)> NodeMatchCallback;
  // MaximumMatcher takes ownership of the passed in callback and uses it to
  // determine whether a node on the left side of the bipartial graph matches
  // a node on the right side. count1 is the number of nodes on the left side
  // of the graph and count2 to is the number of nodes on the right side.
  // Every node is referred to using 0-based indices.
  // If a maximum match is found, the result will be stored in match_list1 and
  // match_list2. match_list1[i] == j means the i-th node on the left side is
  // matched to the j-th node on the right side and match_list2[x] == y means
  // the x-th node on the right side is matched to y-th node on the left side.
  // match_list1[i] == -1 means the node is not matched. Same with match_list2.
  MaximumMatcher(int count1, int count2, NodeMatchCallback callback,
                 std::vector<int>* match_list1, std::vector<int>* match_list2);
  MaximumMatcher(const MaximumMatcher&) = delete;
  MaximumMatcher& operator=(const MaximumMatcher&) = delete;
  // Find a maximum match and return the number of matched node pairs.
  // If early_return is true, this method will return 0 immediately when it
  // finds that not all nodes on the left side can be matched.
  int FindMaximumMatch(bool early_return);

 private:
  // Determines whether the node on the left side of the bipartial graph
  // matches the one on the right side.
  bool Match(int left, int right);
  // Find an argumenting path starting from the node v on the left side. If a
  // path can be found, update match_list2_ to reflect the path and return
  // true.
  bool FindArgumentPathDFS(int v, std::vector<bool>* visited);

  int count1_;
  int count2_;
  NodeMatchCallback match_callback_;
  absl::flat_hash_map<std::pair<int, int>, bool> cached_match_results_;
  std::vector<int>* match_list1_;
  std::vector<int>* match_list2_;
};

MaximumMatcher::MaximumMatcher(int count1, int count2,
                               NodeMatchCallback callback,
                               std::vector<int>* match_list1,
                               std::vector<int>* match_list2)
    : count1_(count1),
      count2_(count2),
      match_callback_(std::move(callback)),
      match_list1_(match_list1),
      match_list2_(match_list2) {
  match_list1_->assign(count1, -1);
  match_list2_->assign(count2, -1);
}

int MaximumMatcher::FindMaximumMatch(bool early_return) {
  int result = 0;
  for (int i = 0; i < count1_; ++i) {
    std::vector<bool> visited(count1_);
    if (FindArgumentPathDFS(i, &visited)) {
      ++result;
    } else if (early_return) {
      return 0;
    }
  }
  // Backfill match_list1_ as we only filled match_list2_ when finding
  // argumenting paths.
  for (int i = 0; i < count2_; ++i) {
    if ((*match_list2_)[i] != -1) {
      (*match_list1_)[(*match_list2_)[i]] = i;
    }
  }
  return result;
}

bool MaximumMatcher::Match(int left, int right) {
  std::pair<int, int> p(left, right);
  auto it = cached_match_results_.find(p);
  if (it != cached_match_results_.end()) {
    return it->second;
  }
  it = cached_match_results_.emplace_hint(it, p, match_callback_(left, right));
  return it->second;
}

bool MaximumMatcher::FindArgumentPathDFS(int v, std::vector<bool>* visited) {
  (*visited)[v] = true;
  // We try to match those un-matched nodes on the right side first. This is
  // the step that the naive greedy matching algorithm uses. In the best cases
  // where the greedy algorithm can find a maximum matching, we will always
  // find a match in this step and the performance will be identical to the
  // greedy algorithm.
  for (int i = 0; i < count2_; ++i) {
    int matched = (*match_list2_)[i];
    if (matched == -1 && Match(v, i)) {
      (*match_list2_)[i] = v;
      return true;
    }
  }
  // Then we try those already matched nodes and see if we can find an
  // alternative match for the node matched to them.
  // The greedy algorithm will stop before this and fail to produce the
  // correct result.
  for (int i = 0; i < count2_; ++i) {
    int matched = (*match_list2_)[i];
    if (matched != -1 && Match(v, i)) {
      if (!(*visited)[matched] && FindArgumentPathDFS(matched, visited)) {
        (*match_list2_)[i] = v;
        return true;
      }
    }
  }
  return false;
}

}  // namespace

bool MessageDifferencer::MatchRepeatedFieldIndices(
    const Message& message1, const Message& message2, int unpacked_any,
    const FieldDescriptor* repeated_field,
    const MapKeyComparator* key_comparator,
    const std::vector<SpecificField>& parent_fields,
    std::vector<int>* match_list1, std::vector<int>* match_list2) {
  const int count1 =
      message1.GetReflection()->FieldSize(message1, repeated_field);
  const int count2 =
      message2.GetReflection()->FieldSize(message2, repeated_field);
  const bool is_treated_as_smart_set = IsTreatedAsSmartSet(repeated_field);

  match_list1->assign(count1, -1);
  match_list2->assign(count2, -1);
  // Ensure that we don't report differences during the matching process. Since
  // field comparators could potentially use this message differencer object to
  // perform further comparisons, turn off reporting here and re-enable it
  // before returning.
  Reporter* reporter = reporter_;
  reporter_ = NULL;
  NumDiffsReporter num_diffs_reporter;
  std::vector<int32_t> num_diffs_list1;
  if (is_treated_as_smart_set) {
    num_diffs_list1.assign(count1, std::numeric_limits<int32_t>::max());
  }

  bool success = true;
  // Find potential match if this is a special repeated field.
  if (scope_ == PARTIAL) {
    // When partial matching is enabled, Compare(a, b) && Compare(a, c)
    // doesn't necessarily imply Compare(b, c). Therefore a naive greedy
    // algorithm will fail to find a maximum matching.
    // Here we use the augmenting path algorithm.
    auto callback = [&](int i1, int i2) {
      return IsMatch(repeated_field, key_comparator, &message1, &message2,
                     unpacked_any, parent_fields, nullptr, i1, i2);
    };
    MaximumMatcher matcher(count1, count2, std::move(callback), match_list1,
                           match_list2);
    // If diff info is not needed, we should end the matching process as
    // soon as possible if not all items can be matched.
    bool early_return = (reporter == nullptr);
    int match_count = matcher.FindMaximumMatch(early_return);
    if (match_count != count1 && early_return) return false;
    success = success && (match_count == count1);
  } else {
    int start_offset = 0;
    // If the two repeated fields are treated as sets, optimize for the case
    // where both start with same items stored in the same order.
    if (IsTreatedAsSet(repeated_field) || is_treated_as_smart_set ||
        IsTreatedAsSmartList(repeated_field)) {
      start_offset = std::min(count1, count2);
      for (int i = 0; i < count1 && i < count2; i++) {
        if (IsMatch(repeated_field, key_comparator, &message1, &message2,
                    unpacked_any, parent_fields, nullptr, i, i)) {
          match_list1->at(i) = i;
          match_list2->at(i) = i;
        } else {
          start_offset = i;
          break;
        }
      }
    }
    for (int i = start_offset; i < count1; ++i) {
      // Indicates any matched elements for this repeated field.
      bool match = false;
      int matched_j = -1;

      for (int j = start_offset; j < count2; j++) {
        if (match_list2->at(j) != -1) {
          if (!is_treated_as_smart_set || num_diffs_list1[i] == 0 ||
              num_diffs_list1[match_list2->at(j)] == 0) {
            continue;
          }
        }

        if (is_treated_as_smart_set) {
          num_diffs_reporter.Reset();
          match =
              IsMatch(repeated_field, key_comparator, &message1, &message2,
                      unpacked_any, parent_fields, &num_diffs_reporter, i, j);
        } else {
          match = IsMatch(repeated_field, key_comparator, &message1, &message2,
                          unpacked_any, parent_fields, nullptr, i, j);
        }

        if (is_treated_as_smart_set) {
          if (match) {
            num_diffs_list1[i] = 0;
          } else if (repeated_field->cpp_type() ==
                     FieldDescriptor::CPPTYPE_MESSAGE) {
            // Replace with the one with fewer diffs.
            const int32_t num_diffs = num_diffs_reporter.GetNumDiffs();
            if (num_diffs < num_diffs_list1[i]) {
              // If j has been already matched to some element, ensure the
              // current num_diffs is smaller.
              if (match_list2->at(j) == -1 ||
                  num_diffs < num_diffs_list1[match_list2->at(j)]) {
                num_diffs_list1[i] = num_diffs;
                match = true;
              }
            }
          }
        }

        if (match) {
          matched_j = j;
          if (!is_treated_as_smart_set || num_diffs_list1[i] == 0) {
            break;
          }
        }
      }

      match = (matched_j != -1);
      if (match) {
        if (is_treated_as_smart_set && match_list2->at(matched_j) != -1) {
          // This is to revert the previously matched index in list2.
          match_list1->at(match_list2->at(matched_j)) = -1;
          match = false;
        }
        match_list1->at(i) = matched_j;
        match_list2->at(matched_j) = i;
      }
      if (!match && reporter == nullptr) return false;
      success = success && match;
    }
  }

  if (IsTreatedAsSmartList(repeated_field)) {
    match_indices_for_smart_list_callback_(match_list1, match_list2);
  }

  reporter_ = reporter;

  return success;
}

FieldComparator::ComparisonResult MessageDifferencer::GetFieldComparisonResult(
    const Message& message1, const Message& message2,
    const FieldDescriptor* field, int index1, int index2,
    const FieldContext* field_context) {
  FieldComparator* comparator = field_comparator_kind_ == kFCBase
                                    ? field_comparator_.base
                                    : field_comparator_.default_impl;
  return comparator->Compare(message1, message2, field, index1, index2,
                             field_context);
}

// ===========================================================================

MessageDifferencer::Reporter::Reporter() {}
MessageDifferencer::Reporter::~Reporter() {}

// ===========================================================================

MessageDifferencer::MapKeyComparator::MapKeyComparator() {}
MessageDifferencer::MapKeyComparator::~MapKeyComparator() {}

// ===========================================================================

MessageDifferencer::IgnoreCriteria::IgnoreCriteria() {}
MessageDifferencer::IgnoreCriteria::~IgnoreCriteria() {}

// ===========================================================================

// Note that the printer's delimiter is not used, because if we are given a
// printer, we don't know its delimiter.
MessageDifferencer::StreamReporter::StreamReporter(
    io::ZeroCopyOutputStream* output)
    : printer_(new io::Printer(output, '$')),
      delete_printer_(true),
      report_modified_aggregates_(false),
      message1_(nullptr),
      message2_(nullptr) {}

MessageDifferencer::StreamReporter::StreamReporter(io::Printer* printer)
    : printer_(printer),
      delete_printer_(false),
      report_modified_aggregates_(false),
      message1_(nullptr),
      message2_(nullptr) {}

MessageDifferencer::StreamReporter::~StreamReporter() {
  if (delete_printer_) delete printer_;
}

void MessageDifferencer::StreamReporter::PrintPath(
    const std::vector<SpecificField>& field_path, bool left_side) {
  for (size_t i = 0; i < field_path.size(); ++i) {
    SpecificField specific_field = field_path[i];

    if (specific_field.field != nullptr &&
        specific_field.field->name() == "value") {
      // check to see if this the value label of a map value.  If so, skip it
      // because it isn't meaningful
      if (i > 0 && field_path[i - 1].field->is_map()) {
        continue;
      }
    }
    if (i > 0) {
      printer_->Print(".");
    }
    if (specific_field.field != NULL) {
      if (specific_field.field->is_extension()) {
        printer_->Print("($name$)", "name", specific_field.field->full_name());
      } else {
        printer_->PrintRaw(specific_field.field->name());
        if (specific_field.forced_compare_no_presence_) {
          printer_->Print(" (added for better PARTIAL comparison)");
        }
      }

      if (specific_field.field->is_map()) {
        PrintMapKey(left_side, specific_field);
        continue;
      }
    } else {
      printer_->PrintRaw(absl::StrCat(specific_field.unknown_field_number));
    }
    if (left_side && specific_field.index >= 0) {
      printer_->Print("[$name$]", "name", absl::StrCat(specific_field.index));
    }
    if (!left_side && specific_field.new_index >= 0) {
      printer_->Print("[$name$]", "name",
                      absl::StrCat(specific_field.new_index));
    }
  }
}

void MessageDifferencer::StreamReporter::PrintValue(
    const Message& message, const std::vector<SpecificField>& field_path,
    bool left_side) {
  const SpecificField& specific_field = field_path.back();
  const FieldDescriptor* field = specific_field.field;
  if (field != NULL) {
    std::string output;
    int index = left_side ? specific_field.index : specific_field.new_index;
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      const Reflection* reflection = message.GetReflection();
      const Message& field_message =
          field->is_repeated()
              ? reflection->GetRepeatedMessage(message, field, index)
              : reflection->GetMessage(message, field);
      const FieldDescriptor* fd = nullptr;

      if (field->is_map() && message1_ != nullptr && message2_ != nullptr) {
        fd = field_message.GetDescriptor()->field(1);
        if (fd->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
          output = PrintShortTextFormat(
              field_message.GetReflection()->GetMessage(field_message, fd));
        } else {
          TextFormat::PrintFieldValueToString(field_message, fd, -1, &output);
        }
      } else {
        output = PrintShortTextFormat(field_message);
      }
      if (output.empty()) {
        printer_->Print("{ }");
      } else {
        if ((fd != nullptr) &&
            (fd->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE)) {
          printer_->PrintRaw(output);
        } else {
          printer_->Print("{ $name$ }", "name", output);
        }
      }
    } else {
      TextFormat::PrintFieldValueToString(message, field, index, &output);
      printer_->PrintRaw(output);
    }
  } else {
    const UnknownFieldSet* unknown_fields =
        (left_side ? specific_field.unknown_field_set1
                   : specific_field.unknown_field_set2);
    const UnknownField* unknown_field =
        &unknown_fields->field(left_side ? specific_field.unknown_field_index1
                                         : specific_field.unknown_field_index2);
    PrintUnknownFieldValue(unknown_field);
  }
}

void MessageDifferencer::StreamReporter::PrintUnknownFieldValue(
    const UnknownField* unknown_field) {
  ABSL_CHECK(unknown_field != NULL) << " Cannot print NULL unknown_field.";

  std::string output;
  switch (unknown_field->type()) {
    case UnknownField::TYPE_VARINT:
      output = absl::StrCat(unknown_field->varint());
      break;
    case UnknownField::TYPE_FIXED32:
      output = absl::StrCat(
          "0x", absl::Hex(unknown_field->fixed32(), absl::kZeroPad8));
      break;
    case UnknownField::TYPE_FIXED64:
      output = absl::StrCat(
          "0x", absl::Hex(unknown_field->fixed64(), absl::kZeroPad16));
      break;
    case UnknownField::TYPE_LENGTH_DELIMITED:
      output = absl::StrFormat(
          "\"%s\"", absl::CEscape(unknown_field->length_delimited()).c_str());
      break;
    case UnknownField::TYPE_GROUP:
      // TODO:  Print the contents of the group like we do for
      //   messages.  Requires an equivalent of ShortDebugString() for
      //   UnknownFieldSet.
      output = "{ ... }";
      break;
  }
  printer_->PrintRaw(output);
}

void MessageDifferencer::StreamReporter::Print(const std::string& str) {
  printer_->Print(str.c_str());
}

void MessageDifferencer::StreamReporter::PrintMapKey(
    bool left_side, const SpecificField& specific_field) {
  if (message1_ == nullptr || message2_ == nullptr) {
    ABSL_LOG(INFO) << "PrintPath cannot log map keys; "
                      "use SetMessages to provide the messages "
                      "being compared prior to any processing.";
    return;
  }

  const Message* found_message =
      left_side ? specific_field.map_entry1 : specific_field.map_entry2;
  std::string key_string = "";
  if (found_message != nullptr) {
    // NB: the map key is always the first field
    const FieldDescriptor* fd = found_message->GetDescriptor()->field(0);
    if (fd->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
      // Not using PrintFieldValueToString for strings to avoid extra
      // characters
      key_string = found_message->GetReflection()->GetString(
          *found_message, found_message->GetDescriptor()->field(0));
    } else {
      TextFormat::PrintFieldValueToString(*found_message, fd, -1, &key_string);
    }
    if (key_string.empty()) {
      key_string = "''";
    }
    printer_->PrintRaw(absl::StrCat("[", key_string, "]"));
  }
}

void MessageDifferencer::StreamReporter::ReportAdded(
    const Message& /*message1*/, const Message& message2,
    const std::vector<SpecificField>& field_path) {
  printer_->Print("added: ");
  PrintPath(field_path, false);
  printer_->Print(": ");
  PrintValue(message2, field_path, false);
  printer_->Print("\n");  // Print for newlines.
}

void MessageDifferencer::StreamReporter::ReportDeleted(
    const Message& message1, const Message& /*message2*/,
    const std::vector<SpecificField>& field_path) {
  printer_->Print("deleted: ");
  PrintPath(field_path, true);
  printer_->Print(": ");
  PrintValue(message1, field_path, true);
  printer_->Print("\n");  // Print for newlines
}

void MessageDifferencer::StreamReporter::ReportModified(
    const Message& message1, const Message& message2,
    const std::vector<SpecificField>& field_path) {
  if (!report_modified_aggregates_ && field_path.back().field == NULL) {
    if (field_path.back().unknown_field_type == UnknownField::TYPE_GROUP) {
      // Any changes to the subfields have already been printed.
      return;
    }
  } else if (!report_modified_aggregates_) {
    if (field_path.back().field->cpp_type() ==
        FieldDescriptor::CPPTYPE_MESSAGE) {
      // Any changes to the subfields have already been printed.
      return;
    }
  }

  printer_->Print("modified: ");
  PrintPath(field_path, true);
  if (CheckPathChanged(field_path)) {
    printer_->Print(" -> ");
    PrintPath(field_path, false);
  }
  printer_->Print(": ");
  PrintValue(message1, field_path, true);
  printer_->Print(" -> ");
  PrintValue(message2, field_path, false);
  printer_->Print("\n");  // Print for newlines.
}

void MessageDifferencer::StreamReporter::ReportMoved(
    const Message& message1, const Message& /*message2*/,
    const std::vector<SpecificField>& field_path) {
  printer_->Print("moved: ");
  PrintPath(field_path, true);
  printer_->Print(" -> ");
  PrintPath(field_path, false);
  printer_->Print(" : ");
  PrintValue(message1, field_path, true);
  printer_->Print("\n");  // Print for newlines.
}

void MessageDifferencer::StreamReporter::ReportMatched(
    const Message& message1, const Message& /*message2*/,
    const std::vector<SpecificField>& field_path) {
  printer_->Print("matched: ");
  PrintPath(field_path, true);
  if (CheckPathChanged(field_path)) {
    printer_->Print(" -> ");
    PrintPath(field_path, false);
  }
  printer_->Print(" : ");
  PrintValue(message1, field_path, true);
  printer_->Print("\n");  // Print for newlines.
}

void MessageDifferencer::StreamReporter::ReportIgnored(
    const Message& /*message1*/, const Message& /*message2*/,
    const std::vector<SpecificField>& field_path) {
  printer_->Print("ignored: ");
  PrintPath(field_path, true);
  if (CheckPathChanged(field_path)) {
    printer_->Print(" -> ");
    PrintPath(field_path, false);
  }
  printer_->Print("\n");  // Print for newlines.
}

void MessageDifferencer::StreamReporter::SetMessages(const Message& message1,
                                                     const Message& message2) {
  message1_ = &message1;
  message2_ = &message2;
}

void MessageDifferencer::StreamReporter::ReportUnknownFieldIgnored(
    const Message& /*message1*/, const Message& /*message2*/,
    const std::vector<SpecificField>& field_path) {
  printer_->Print("ignored: ");
  PrintPath(field_path, true);
  if (CheckPathChanged(field_path)) {
    printer_->Print(" -> ");
    PrintPath(field_path, false);
  }
  printer_->Print("\n");  // Print for newlines.
}

MessageDifferencer::MapKeyComparator*
MessageDifferencer::CreateMultipleFieldsMapKeyComparator(
    const std::vector<std::vector<const FieldDescriptor*> >& key_field_paths) {
  return new MultipleFieldsMapKeyComparator(this, key_field_paths);
}

}  // namespace util
}  // namespace protobuf
}  // namespace google

