// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.

#include "google_protobuf_generated_message_reflection.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <string>

#include <CppAbseil/absl_base_call_once.hpp>
#include <CppAbseil/absl_base_casts.hpp>
#include <CppAbseil/absl_container_flat_hash_map.hpp>
#include <CppAbseil/absl_container_flat_hash_set.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_strings_match.hpp>
#include <CppAbseil/absl_strings_str_format.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include <CppAbseil/absl_synchronization_mutex.hpp>
#include "google_protobuf_descriptor.hpp"
#include "google_protobuf_descriptor.pb.hpp"
#include "google_protobuf_descriptor_legacy.hpp"
#include "google_protobuf_extension_set.hpp"
#include "google_protobuf_generated_message_tctable_decl.hpp"
#include "google_protobuf_generated_message_tctable_gen.hpp"
#include "google_protobuf_generated_message_tctable_impl.hpp"
#include "google_protobuf_generated_message_util.hpp"
#include "google_protobuf_inlined_string_field.hpp"
#include "google_protobuf_map_field.hpp"
#include "google_protobuf_map_field_inl.hpp"
#include "google_protobuf_raw_ptr.hpp"
#include "google_protobuf_repeated_field.hpp"
#include "google_protobuf_unknown_field_set.hpp"


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

#define GOOGLE_PROTOBUF_HAS_ONEOF

using google::protobuf::internal::ArenaStringPtr;
using google::protobuf::internal::DescriptorTable;
using google::protobuf::internal::ExtensionSet;
using google::protobuf::internal::GenericTypeHandler;
using google::protobuf::internal::GetEmptyString;
using google::protobuf::internal::InlinedStringField;
using google::protobuf::internal::InternalMetadata;
using google::protobuf::internal::LazyField;
using google::protobuf::internal::MapFieldBase;
using google::protobuf::internal::MigrationSchema;
using google::protobuf::internal::OnShutdownDelete;
using google::protobuf::internal::ReflectionSchema;
using google::protobuf::internal::RepeatedPtrFieldBase;
using google::protobuf::internal::StringSpaceUsedExcludingSelfLong;
using google::protobuf::internal::cpp::IsLazilyInitializedFile;

namespace google {
namespace protobuf {

namespace {
bool IsMapFieldInApi(const FieldDescriptor* field) { return field->is_map(); }

#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
Message* MaybeForceCopy(Arena* arena, Message* msg) {
  if (arena != nullptr || msg == nullptr) return msg;

  Message* copy = msg->New();
  copy->MergeFrom(*msg);
  delete msg;
  return copy;
}
#endif  // PROTOBUF_FORCE_COPY_IN_RELEASE
}  // anonymous namespace

namespace internal {

void InitializeFileDescriptorDefaultInstances() {
#if !defined(PROTOBUF_CONSTINIT_DEFAULT_INSTANCES)
  static std::true_type init =
      (InitializeFileDescriptorDefaultInstancesSlow(), std::true_type{});
  (void)init;
#endif  // !defined(PROTOBUF_CONSTINIT_DEFAULT_INSTANCES)
}

bool ParseNamedEnum(const EnumDescriptor* descriptor, absl::string_view name,
                    int* value) {
  const EnumValueDescriptor* d = descriptor->FindValueByName(name);
  if (d == nullptr) return false;
  *value = d->number();
  return true;
}

const std::string& NameOfEnum(const EnumDescriptor* descriptor, int value) {
  const EnumValueDescriptor* d = descriptor->FindValueByNumber(value);
  return (d == nullptr ? GetEmptyString() : d->name());
}

// Internal helper routine for NameOfDenseEnum in the header file.
// Allocates and fills a simple array of string pointers, based on
// reflection information about the names of the enums.  This routine
// allocates max_val + 1 entries, under the assumption that all the enums
// fall in the range [min_val .. max_val].
const std::string** MakeDenseEnumCache(const EnumDescriptor* desc, int min_val,
                                       int max_val) {
  auto* str_ptrs =
      new const std::string*[static_cast<size_t>(max_val - min_val + 1)]();
  const int count = desc->value_count();
  for (int i = 0; i < count; ++i) {
    const int num = desc->value(i)->number();
    if (str_ptrs[num - min_val] == nullptr) {
      // Don't over-write an existing entry, because in case of duplication, the
      // first one wins.
      str_ptrs[num - min_val] = &desc->value(i)->name();
    }
  }
  // Change any unfilled entries to point to the empty string.
  for (int i = 0; i < max_val - min_val + 1; ++i) {
    if (str_ptrs[i] == nullptr) str_ptrs[i] = &GetEmptyStringAlreadyInited();
  }
  return str_ptrs;
}

PROTOBUF_NOINLINE const std::string& NameOfDenseEnumSlow(
    int v, DenseEnumCacheInfo* deci) {
  if (v < deci->min_val || v > deci->max_val)
    return GetEmptyStringAlreadyInited();

  const std::string** new_cache =
      MakeDenseEnumCache(deci->descriptor_fn(), deci->min_val, deci->max_val);
  const std::string** old_cache = nullptr;

  if (deci->cache.compare_exchange_strong(old_cache, new_cache,
                                          std::memory_order_release,
                                          std::memory_order_acquire)) {
    // We successfully stored our new cache, and the old value was nullptr.
    return *new_cache[v - deci->min_val];
  } else {
    // In the time it took to create our enum cache, another thread also
    //  created one, and put it into deci->cache.  So delete ours, and
    // use theirs instead.
    delete[] new_cache;
    return *old_cache[v - deci->min_val];
  }
}

}  // namespace internal

// ===================================================================
// Helpers for reporting usage errors (e.g. trying to use GetInt32() on
// a string field).

namespace {

using internal::GetConstPointerAtOffset;
using internal::GetConstRefAtOffset;
using internal::GetPointerAtOffset;

void ReportReflectionUsageError(const Descriptor* descriptor,
                                const FieldDescriptor* field,
                                const char* method, const char* description) {
  ABSL_LOG(FATAL) << "Protocol Buffer reflection usage error:\n"
                     "  Method      : google::protobuf::Reflection::"
                  << method
                  << "\n"
                     "  Message type: "
                  << descriptor->full_name()
                  << "\n"
                     "  Field       : "
                  << field->full_name()
                  << "\n"
                     "  Problem     : "
                  << description;
}

#ifndef NDEBUG
void ReportReflectionUsageMessageError(const Descriptor* expected,
                                       const Descriptor* actual,
                                       const FieldDescriptor* field,
                                       const char* method) {
  ABSL_LOG(FATAL) << absl::StrFormat(
      "Protocol Buffer reflection usage error:\n"
      "  Method       : google::protobuf::Reflection::%s\n"
      "  Expected type: %s\n"
      "  Actual type  : %s\n"
      "  Field        : %s\n"
      "  Problem      : Message is not the right object for reflection",
      method, expected->full_name(), actual->full_name(), field->full_name());
}
#endif

const char* cpptype_names_[FieldDescriptor::MAX_CPPTYPE + 1] = {
    "INVALID_CPPTYPE", "CPPTYPE_INT32",  "CPPTYPE_INT64",  "CPPTYPE_UINT32",
    "CPPTYPE_UINT64",  "CPPTYPE_DOUBLE", "CPPTYPE_FLOAT",  "CPPTYPE_BOOL",
    "CPPTYPE_ENUM",    "CPPTYPE_STRING", "CPPTYPE_MESSAGE"};

static void ReportReflectionUsageTypeError(
    const Descriptor* descriptor, const FieldDescriptor* field,
    const char* method, FieldDescriptor::CppType expected_type) {
  ABSL_LOG(FATAL)
      << "Protocol Buffer reflection usage error:\n"
         "  Method      : google::protobuf::Reflection::"
      << method
      << "\n"
         "  Message type: "
      << descriptor->full_name()
      << "\n"
         "  Field       : "
      << field->full_name()
      << "\n"
         "  Problem     : Field is not the right type for this message:\n"
         "    Expected  : "
      << cpptype_names_[expected_type]
      << "\n"
         "    Field type: "
      << cpptype_names_[field->cpp_type()];
}

static void ReportReflectionUsageEnumTypeError(
    const Descriptor* descriptor, const FieldDescriptor* field,
    const char* method, const EnumValueDescriptor* value) {
  ABSL_LOG(FATAL) << "Protocol Buffer reflection usage error:\n"
                     "  Method      : google::protobuf::Reflection::"
                  << method
                  << "\n"
                     "  Message type: "
                  << descriptor->full_name()
                  << "\n"
                     "  Field       : "
                  << field->full_name()
                  << "\n"
                     "  Problem     : Enum value did not match field type:\n"
                     "    Expected  : "
                  << field->enum_type()->full_name()
                  << "\n"
                     "    Actual    : "
                  << value->full_name();
}

#define USAGE_CHECK(CONDITION, METHOD, ERROR_DESCRIPTION) \
  if (!(CONDITION))                                       \
  ReportReflectionUsageError(descriptor_, field, #METHOD, ERROR_DESCRIPTION)
#define USAGE_CHECK_EQ(A, B, METHOD, ERROR_DESCRIPTION) \
  USAGE_CHECK((A) == (B), METHOD, ERROR_DESCRIPTION)
#define USAGE_CHECK_NE(A, B, METHOD, ERROR_DESCRIPTION) \
  USAGE_CHECK((A) != (B), METHOD, ERROR_DESCRIPTION)

#define USAGE_CHECK_TYPE(METHOD, CPPTYPE)                      \
  if (field->cpp_type() != FieldDescriptor::CPPTYPE_##CPPTYPE) \
  ReportReflectionUsageTypeError(descriptor_, field, #METHOD,  \
                                 FieldDescriptor::CPPTYPE_##CPPTYPE)

#define USAGE_CHECK_ENUM_VALUE(METHOD)     \
  if (value->type() != field->enum_type()) \
  ReportReflectionUsageEnumTypeError(descriptor_, field, #METHOD, value)

#ifdef NDEBUG
// Avoid a virtual method call in optimized builds.
#define USAGE_CHECK_MESSAGE(METHOD, MESSAGE)
#else
#define USAGE_CHECK_MESSAGE(METHOD, MESSAGE)                                 \
  if (this != (MESSAGE)->GetReflection())                                    \
  ReportReflectionUsageMessageError(descriptor_, (MESSAGE)->GetDescriptor(), \
                                    field, #METHOD)
#endif

#define USAGE_CHECK_MESSAGE_TYPE(METHOD)                        \
  USAGE_CHECK_EQ(field->containing_type(), descriptor_, METHOD, \
                 "Field does not match message type.");
#define USAGE_CHECK_SINGULAR(METHOD)                                      \
  USAGE_CHECK_NE(field->label(), FieldDescriptor::LABEL_REPEATED, METHOD, \
                 "Field is repeated; the method requires a singular field.")
#define USAGE_CHECK_REPEATED(METHOD)                                      \
  USAGE_CHECK_EQ(field->label(), FieldDescriptor::LABEL_REPEATED, METHOD, \
                 "Field is singular; the method requires a repeated field.")

#define USAGE_CHECK_ALL(METHOD, LABEL, CPPTYPE) \
  USAGE_CHECK_MESSAGE(METHOD, &message);        \
  USAGE_CHECK_MESSAGE_TYPE(METHOD);             \
  USAGE_CHECK_##LABEL(METHOD);                  \
  USAGE_CHECK_TYPE(METHOD, CPPTYPE)

#define USAGE_MUTABLE_CHECK_ALL(METHOD, LABEL, CPPTYPE) \
  USAGE_CHECK_MESSAGE(METHOD, message);                 \
  USAGE_CHECK_MESSAGE_TYPE(METHOD);                     \
  USAGE_CHECK_##LABEL(METHOD);                          \
  USAGE_CHECK_TYPE(METHOD, CPPTYPE)

}  // namespace

// ===================================================================

Reflection::Reflection(const Descriptor* descriptor,
                       const internal::ReflectionSchema& schema,
                       const DescriptorPool* pool, MessageFactory* factory)
    : descriptor_(descriptor),
      schema_(schema),
      descriptor_pool_(
          (pool == nullptr) ? DescriptorPool::internal_generated_pool() : pool),
      message_factory_(factory),
      last_non_weak_field_index_(-1) {
  last_non_weak_field_index_ = descriptor_->field_count() - 1;
}

Reflection::~Reflection() {
  // No need to use sized delete. This code path is uncommon and it would not be
  // worth saving or recalculating the size.
  ::operator delete(const_cast<internal::TcParseTableBase*>(tcparse_table_));
}

const UnknownFieldSet& Reflection::GetUnknownFields(
    const Message& message) const {
  return GetInternalMetadata(message).unknown_fields<UnknownFieldSet>(
      UnknownFieldSet::default_instance);
}

UnknownFieldSet* Reflection::MutableUnknownFields(Message* message) const {
  return MutableInternalMetadata(message)
      ->mutable_unknown_fields<UnknownFieldSet>();
}

bool Reflection::IsLazyExtension(const Message& message,
                                 const FieldDescriptor* field) const {
  return field->is_extension() &&
         GetExtensionSet(message).HasLazy(field->number());
}

bool Reflection::IsLazilyVerifiedLazyField(const FieldDescriptor* field) const {
  if (field->options().unverified_lazy()) return true;

  // Message fields with [lazy=true] will be eagerly verified
  // (go/verified-lazy).
  return field->options().lazy() && !IsEagerlyVerifiedLazyField(field);
}

bool Reflection::IsEagerlyVerifiedLazyField(
    const FieldDescriptor* field) const {
  return (field->type() == FieldDescriptor::TYPE_MESSAGE &&
          schema_.IsEagerlyVerifiedLazyField(field));
}

bool Reflection::IsInlined(const FieldDescriptor* field) const {
  return schema_.IsFieldInlined(field);
}

size_t Reflection::SpaceUsedLong(const Message& message) const {
  // object_size_ already includes the in-memory representation of each field
  // in the message, so we only need to account for additional memory used by
  // the fields.
  size_t total_size = schema_.GetObjectSize();

  total_size += GetUnknownFields(message).SpaceUsedExcludingSelfLong();

  if (schema_.HasExtensionSet()) {
    total_size += GetExtensionSet(message).SpaceUsedExcludingSelfLong();
  }
  for (int i = 0; i <= last_non_weak_field_index_; i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (field->is_repeated()) {
      switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, LOWERCASE)                           \
  case FieldDescriptor::CPPTYPE_##UPPERCASE:                        \
    total_size += GetRaw<RepeatedField<LOWERCASE> >(message, field) \
                      .SpaceUsedExcludingSelfLong();                \
    break

        HANDLE_TYPE(INT32, int32_t);
        HANDLE_TYPE(INT64, int64_t);
        HANDLE_TYPE(UINT32, uint32_t);
        HANDLE_TYPE(UINT64, uint64_t);
        HANDLE_TYPE(DOUBLE, double);
        HANDLE_TYPE(FLOAT, float);
        HANDLE_TYPE(BOOL, bool);
        HANDLE_TYPE(ENUM, int);
#undef HANDLE_TYPE

        case FieldDescriptor::CPPTYPE_STRING:
          switch (field->options().ctype()) {
            default:  // TODO:  Support other string reps.
            case FieldOptions::STRING:
              total_size +=
                  GetRaw<RepeatedPtrField<std::string> >(message, field)
                      .SpaceUsedExcludingSelfLong();
              break;
          }
          break;

        case FieldDescriptor::CPPTYPE_MESSAGE:
          if (IsMapFieldInApi(field)) {
            total_size += GetRaw<internal::MapFieldBase>(message, field)
                              .SpaceUsedExcludingSelfLong();
          } else {
            // We don't know which subclass of RepeatedPtrFieldBase the type is,
            // so we use RepeatedPtrFieldBase directly.
            total_size +=
                GetRaw<RepeatedPtrFieldBase>(message, field)
                    .SpaceUsedExcludingSelfLong<GenericTypeHandler<Message> >();
          }

          break;
      }
    } else {
      if (schema_.InRealOneof(field) && !HasOneofField(message, field)) {
        continue;
      }
      switch (field->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
        case FieldDescriptor::CPPTYPE_INT64:
        case FieldDescriptor::CPPTYPE_UINT32:
        case FieldDescriptor::CPPTYPE_UINT64:
        case FieldDescriptor::CPPTYPE_DOUBLE:
        case FieldDescriptor::CPPTYPE_FLOAT:
        case FieldDescriptor::CPPTYPE_BOOL:
        case FieldDescriptor::CPPTYPE_ENUM:
          // Field is inline, so we've already counted it.
          break;

        case FieldDescriptor::CPPTYPE_STRING: {
          switch (internal::cpp::EffectiveStringCType(field)) {
            case FieldOptions::CORD:
              if (schema_.InRealOneof(field)) {
                total_size += GetField<absl::Cord*>(message, field)
                                  ->EstimatedMemoryUsage();

              } else {
                // sizeof(absl::Cord) is included to self.
                total_size += GetField<absl::Cord>(message, field)
                                  .EstimatedMemoryUsage() -
                              sizeof(absl::Cord);
              }
              break;
            default:
            case FieldOptions::STRING:
              if (IsInlined(field)) {
                const std::string* ptr =
                    &GetField<InlinedStringField>(message, field).GetNoArena();
                total_size += StringSpaceUsedExcludingSelfLong(*ptr);
              } else {
                // Initially, the string points to the default value stored
                // in the prototype. Only count the string if it has been
                // changed from the default value.
                // Except oneof fields, those never point to a default instance,
                // and there is no default instance to point to.
                const auto& str = GetField<ArenaStringPtr>(message, field);
                if (!str.IsDefault() || schema_.InRealOneof(field)) {
                  // string fields are represented by just a pointer, so also
                  // include sizeof(string) as well.
                  total_size += sizeof(std::string) +
                                StringSpaceUsedExcludingSelfLong(str.Get());
                }
              }
              break;
          }
          break;
        }

        case FieldDescriptor::CPPTYPE_MESSAGE:
          if (schema_.IsDefaultInstance(message)) {
            // For singular fields, the prototype just stores a pointer to the
            // external type's prototype, so there is no extra memory usage.
          } else {
            const Message* sub_message = GetRaw<const Message*>(message, field);
            if (sub_message != nullptr) {
              total_size += sub_message->SpaceUsedLong();
            }
          }
          break;
      }
    }
  }
#ifndef PROTOBUF_FUZZ_MESSAGE_SPACE_USED_LONG
  return total_size;
#else
  // Use both `this` and `dummy` to generate the seed so that the scale factor
  // is both per-object and non-predictable, but consistent across multiple
  // calls in the same binary.
  static bool dummy;
  uintptr_t seed =
      reinterpret_cast<uintptr_t>(&dummy) ^ reinterpret_cast<uintptr_t>(this);
  // Fuzz the size by +/- 50%.
  double scale = (static_cast<double>(seed % 10000) / 10000) + 0.5;
  return total_size * scale;
#endif
}

namespace {

template <bool unsafe_shallow_swap>
struct OneofFieldMover {
  template <typename FromType, typename ToType>
  void operator()(const FieldDescriptor* field, FromType* from, ToType* to) {
    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32:
        to->SetInt32(from->GetInt32());
        break;
      case FieldDescriptor::CPPTYPE_INT64:
        to->SetInt64(from->GetInt64());
        break;
      case FieldDescriptor::CPPTYPE_UINT32:
        to->SetUint32(from->GetUint32());
        break;
      case FieldDescriptor::CPPTYPE_UINT64:
        to->SetUint64(from->GetUint64());
        break;
      case FieldDescriptor::CPPTYPE_FLOAT:
        to->SetFloat(from->GetFloat());
        break;
      case FieldDescriptor::CPPTYPE_DOUBLE:
        to->SetDouble(from->GetDouble());
        break;
      case FieldDescriptor::CPPTYPE_BOOL:
        to->SetBool(from->GetBool());
        break;
      case FieldDescriptor::CPPTYPE_ENUM:
        to->SetEnum(from->GetEnum());
        break;
      case FieldDescriptor::CPPTYPE_MESSAGE:
        if (!unsafe_shallow_swap) {
          to->SetMessage(from->GetMessage());
        } else {
          to->UnsafeSetMessage(from->UnsafeGetMessage());
        }
        break;
      case FieldDescriptor::CPPTYPE_STRING:
        if (!unsafe_shallow_swap) {
          to->SetString(from->GetString());
          break;
        }
        switch (internal::cpp::EffectiveStringCType(field)) {
          case FieldOptions::CORD:
            to->SetCord(from->GetCord());
            break;
          default:
          case FieldOptions::STRING: {
            to->SetArenaStringPtr(from->GetArenaStringPtr());
            break;
          }
        }
        break;
      default:
        ABSL_LOG(FATAL) << "unimplemented type: " << field->cpp_type();
    }
    if (unsafe_shallow_swap) {
      // Not clearing oneof case after move may cause unwanted "ClearOneof"
      // where the residual message or string value is deleted and causes
      // use-after-free (only for unsafe swap).
      from->ClearOneofCase();
    }
  }
};

}  // namespace

namespace internal {

class SwapFieldHelper {
 public:
  template <bool unsafe_shallow_swap>
  static void SwapRepeatedStringField(const Reflection* r, Message* lhs,
                                      Message* rhs,
                                      const FieldDescriptor* field);

  template <bool unsafe_shallow_swap>
  static void SwapInlinedStrings(const Reflection* r, Message* lhs,
                                 Message* rhs, const FieldDescriptor* field);

  template <bool unsafe_shallow_swap>
  static void SwapNonInlinedStrings(const Reflection* r, Message* lhs,
                                    Message* rhs, const FieldDescriptor* field);

  template <bool unsafe_shallow_swap>
  static void SwapStringField(const Reflection* r, Message* lhs, Message* rhs,
                              const FieldDescriptor* field);

  static void SwapArenaStringPtr(ArenaStringPtr* lhs, Arena* lhs_arena,
                                 ArenaStringPtr* rhs, Arena* rhs_arena);

  template <bool unsafe_shallow_swap>
  static void SwapRepeatedMessageField(const Reflection* r, Message* lhs,
                                       Message* rhs,
                                       const FieldDescriptor* field);

  template <bool unsafe_shallow_swap>
  static void SwapMessageField(const Reflection* r, Message* lhs, Message* rhs,
                               const FieldDescriptor* field);

  static void SwapMessage(const Reflection* r, Message* lhs, Arena* lhs_arena,
                          Message* rhs, Arena* rhs_arena,
                          const FieldDescriptor* field);

  static void SwapNonMessageNonStringField(const Reflection* r, Message* lhs,
                                           Message* rhs,
                                           const FieldDescriptor* field);
};

template <bool unsafe_shallow_swap>
void SwapFieldHelper::SwapRepeatedStringField(const Reflection* r, Message* lhs,
                                              Message* rhs,
                                              const FieldDescriptor* field) {
  switch (field->options().ctype()) {
    default:
    case FieldOptions::STRING: {
      auto* lhs_string = r->MutableRaw<RepeatedPtrFieldBase>(lhs, field);
      auto* rhs_string = r->MutableRaw<RepeatedPtrFieldBase>(rhs, field);
      if (unsafe_shallow_swap) {
        lhs_string->InternalSwap(rhs_string);
      } else {
        lhs_string->Swap<GenericTypeHandler<std::string>>(rhs_string);
      }
      break;
    }
  }
}

template <bool unsafe_shallow_swap>
void SwapFieldHelper::SwapInlinedStrings(const Reflection* r, Message* lhs,
                                         Message* rhs,
                                         const FieldDescriptor* field) {
  // Inlined string field.
  Arena* lhs_arena = lhs->GetArena();
  Arena* rhs_arena = rhs->GetArena();
  auto* lhs_string = r->MutableRaw<InlinedStringField>(lhs, field);
  auto* rhs_string = r->MutableRaw<InlinedStringField>(rhs, field);
  uint32_t index = r->schema_.InlinedStringIndex(field);
  ABSL_DCHECK_GT(index, 0);
  uint32_t* lhs_array = r->MutableInlinedStringDonatedArray(lhs);
  uint32_t* rhs_array = r->MutableInlinedStringDonatedArray(rhs);
  uint32_t* lhs_state = &lhs_array[index / 32];
  uint32_t* rhs_state = &rhs_array[index / 32];
  bool lhs_arena_dtor_registered = (lhs_array[0] & 0x1u) == 0;
  bool rhs_arena_dtor_registered = (rhs_array[0] & 0x1u) == 0;
  const uint32_t mask = ~(static_cast<uint32_t>(1) << (index % 32));
  if (unsafe_shallow_swap) {
    ABSL_DCHECK_EQ(lhs_arena, rhs_arena);
    InlinedStringField::InternalSwap(lhs_string, lhs_arena_dtor_registered, lhs,
                                     rhs_string, rhs_arena_dtor_registered, rhs,
                                     lhs_arena);
  } else {
    const std::string temp = lhs_string->Get();
    lhs_string->Set(rhs_string->Get(), lhs_arena,
                    r->IsInlinedStringDonated(*lhs, field), lhs_state, mask,
                    lhs);
    rhs_string->Set(temp, rhs_arena, r->IsInlinedStringDonated(*rhs, field),
                    rhs_state, mask, rhs);
  }
}

template <bool unsafe_shallow_swap>
void SwapFieldHelper::SwapNonInlinedStrings(const Reflection* r, Message* lhs,
                                            Message* rhs,
                                            const FieldDescriptor* field) {
  ArenaStringPtr* lhs_string = r->MutableRaw<ArenaStringPtr>(lhs, field);
  ArenaStringPtr* rhs_string = r->MutableRaw<ArenaStringPtr>(rhs, field);
  if (unsafe_shallow_swap) {
    ArenaStringPtr::UnsafeShallowSwap(lhs_string, rhs_string);
  } else {
    SwapFieldHelper::SwapArenaStringPtr(lhs_string, lhs->GetArena(),  //
                                        rhs_string, rhs->GetArena());
  }
}

template <bool unsafe_shallow_swap>
void SwapFieldHelper::SwapStringField(const Reflection* r, Message* lhs,
                                      Message* rhs,
                                      const FieldDescriptor* field) {
  switch (internal::cpp::EffectiveStringCType(field)) {
    case FieldOptions::CORD:
      // Always shallow swap for Cord.
      std::swap(*r->MutableRaw<absl::Cord>(lhs, field),
                *r->MutableRaw<absl::Cord>(rhs, field));
      break;
    default:
    case FieldOptions::STRING: {
      if (r->IsInlined(field)) {
        SwapFieldHelper::SwapInlinedStrings<unsafe_shallow_swap>(r, lhs, rhs,
                                                                 field);
      } else {
        SwapFieldHelper::SwapNonInlinedStrings<unsafe_shallow_swap>(r, lhs, rhs,
                                                                    field);
      }
      break;
    }
  }
}

void SwapFieldHelper::SwapArenaStringPtr(ArenaStringPtr* lhs, Arena* lhs_arena,
                                         ArenaStringPtr* rhs,
                                         Arena* rhs_arena) {
  if (lhs_arena == rhs_arena) {
    ArenaStringPtr::InternalSwap(lhs, rhs, lhs_arena);
  } else if (lhs->IsDefault() && rhs->IsDefault()) {
    // Nothing to do.
  } else if (lhs->IsDefault()) {
    lhs->Set(rhs->Get(), lhs_arena);
    // rhs needs to be destroyed before overwritten.
    rhs->Destroy();
    rhs->InitDefault();
  } else if (rhs->IsDefault()) {
    rhs->Set(lhs->Get(), rhs_arena);
    // lhs needs to be destroyed before overwritten.
    lhs->Destroy();
    lhs->InitDefault();
  } else {
    std::string temp = lhs->Get();
    lhs->Set(rhs->Get(), lhs_arena);
    rhs->Set(std::move(temp), rhs_arena);
  }
}

template <bool unsafe_shallow_swap>
void SwapFieldHelper::SwapRepeatedMessageField(const Reflection* r,
                                               Message* lhs, Message* rhs,
                                               const FieldDescriptor* field) {
  if (IsMapFieldInApi(field)) {
    auto* lhs_map = r->MutableRaw<MapFieldBase>(lhs, field);
    auto* rhs_map = r->MutableRaw<MapFieldBase>(rhs, field);
    if (unsafe_shallow_swap) {
      lhs_map->UnsafeShallowSwap(rhs_map);
    } else {
      lhs_map->Swap(rhs_map);
    }
  } else {
    auto* lhs_rm = r->MutableRaw<RepeatedPtrFieldBase>(lhs, field);
    auto* rhs_rm = r->MutableRaw<RepeatedPtrFieldBase>(rhs, field);
    if (unsafe_shallow_swap) {
      lhs_rm->InternalSwap(rhs_rm);
    } else {
      lhs_rm->Swap<GenericTypeHandler<Message>>(rhs_rm);
    }
  }
}

template <bool unsafe_shallow_swap>
void SwapFieldHelper::SwapMessageField(const Reflection* r, Message* lhs,
                                       Message* rhs,
                                       const FieldDescriptor* field) {
  if (unsafe_shallow_swap) {
    std::swap(*r->MutableRaw<Message*>(lhs, field),
              *r->MutableRaw<Message*>(rhs, field));
  } else {
    SwapMessage(r, lhs, lhs->GetArena(), rhs, rhs->GetArena(), field);
  }
}

void SwapFieldHelper::SwapMessage(const Reflection* r, Message* lhs,
                                  Arena* lhs_arena, Message* rhs,
                                  Arena* rhs_arena,
                                  const FieldDescriptor* field) {
  Message** lhs_sub = r->MutableRaw<Message*>(lhs, field);
  Message** rhs_sub = r->MutableRaw<Message*>(rhs, field);

  if (*lhs_sub == *rhs_sub) return;

#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
  if (lhs_arena != nullptr && lhs_arena == rhs_arena) {
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
  if (lhs_arena == rhs_arena) {
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
    std::swap(*lhs_sub, *rhs_sub);
    return;
  }

  if (*lhs_sub != nullptr && *rhs_sub != nullptr) {
    (*lhs_sub)->GetReflection()->Swap(*lhs_sub, *rhs_sub);
  } else if (*lhs_sub == nullptr && r->HasBit(*rhs, field)) {
    *lhs_sub = (*rhs_sub)->New(lhs_arena);
    (*lhs_sub)->CopyFrom(**rhs_sub);
    r->ClearField(rhs, field);
    // Ensures has bit is unchanged after ClearField.
    r->SetBit(rhs, field);
  } else if (*rhs_sub == nullptr && r->HasBit(*lhs, field)) {
    *rhs_sub = (*lhs_sub)->New(rhs_arena);
    (*rhs_sub)->CopyFrom(**lhs_sub);
    r->ClearField(lhs, field);
    // Ensures has bit is unchanged after ClearField.
    r->SetBit(lhs, field);
  }
}

void SwapFieldHelper::SwapNonMessageNonStringField(
    const Reflection* r, Message* lhs, Message* rhs,
    const FieldDescriptor* field) {
  switch (field->cpp_type()) {
#define SWAP_VALUES(CPPTYPE, TYPE)               \
  case FieldDescriptor::CPPTYPE_##CPPTYPE:       \
    std::swap(*r->MutableRaw<TYPE>(lhs, field),  \
              *r->MutableRaw<TYPE>(rhs, field)); \
    break;

    SWAP_VALUES(INT32, int32_t);
    SWAP_VALUES(INT64, int64_t);
    SWAP_VALUES(UINT32, uint32_t);
    SWAP_VALUES(UINT64, uint64_t);
    SWAP_VALUES(FLOAT, float);
    SWAP_VALUES(DOUBLE, double);
    SWAP_VALUES(BOOL, bool);
    SWAP_VALUES(ENUM, int);
#undef SWAP_VALUES
    default:
      ABSL_LOG(FATAL) << "Unimplemented type: " << field->cpp_type();
  }
}

}  // namespace internal

void Reflection::SwapField(Message* message1, Message* message2,
                           const FieldDescriptor* field) const {
  if (field->is_repeated()) {
    switch (field->cpp_type()) {
#define SWAP_ARRAYS(CPPTYPE, TYPE)                                 \
  case FieldDescriptor::CPPTYPE_##CPPTYPE:                         \
    MutableRaw<RepeatedField<TYPE> >(message1, field)              \
        ->Swap(MutableRaw<RepeatedField<TYPE> >(message2, field)); \
    break;

      SWAP_ARRAYS(INT32, int32_t);
      SWAP_ARRAYS(INT64, int64_t);
      SWAP_ARRAYS(UINT32, uint32_t);
      SWAP_ARRAYS(UINT64, uint64_t);
      SWAP_ARRAYS(FLOAT, float);
      SWAP_ARRAYS(DOUBLE, double);
      SWAP_ARRAYS(BOOL, bool);
      SWAP_ARRAYS(ENUM, int);
#undef SWAP_ARRAYS

      case FieldDescriptor::CPPTYPE_STRING:
        internal::SwapFieldHelper::SwapRepeatedStringField<false>(
            this, message1, message2, field);
        break;
      case FieldDescriptor::CPPTYPE_MESSAGE:
        internal::SwapFieldHelper::SwapRepeatedMessageField<false>(
            this, message1, message2, field);
        break;

      default:
        ABSL_LOG(FATAL) << "Unimplemented type: " << field->cpp_type();
    }
  } else {
    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_MESSAGE:
        internal::SwapFieldHelper::SwapMessageField<false>(this, message1,
                                                           message2, field);
        break;

      case FieldDescriptor::CPPTYPE_STRING:
        internal::SwapFieldHelper::SwapStringField<false>(this, message1,
                                                          message2, field);
        break;
      default:
        internal::SwapFieldHelper::SwapNonMessageNonStringField(
            this, message1, message2, field);
    }
  }
}

void Reflection::UnsafeShallowSwapField(Message* message1, Message* message2,
                                        const FieldDescriptor* field) const {
  if (!field->is_repeated()) {
    if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
      internal::SwapFieldHelper::SwapMessageField<true>(this, message1,
                                                        message2, field);
    } else if (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING) {
      internal::SwapFieldHelper::SwapStringField<true>(this, message1, message2,
                                                       field);
    } else {
      internal::SwapFieldHelper::SwapNonMessageNonStringField(this, message1,
                                                              message2, field);
    }
    return;
  }

  switch (field->cpp_type()) {
#define SHALLOW_SWAP_ARRAYS(CPPTYPE, TYPE)                                \
  case FieldDescriptor::CPPTYPE_##CPPTYPE:                                \
    MutableRaw<RepeatedField<TYPE>>(message1, field)                      \
        ->InternalSwap(MutableRaw<RepeatedField<TYPE>>(message2, field)); \
    break;

    SHALLOW_SWAP_ARRAYS(INT32, int32_t);
    SHALLOW_SWAP_ARRAYS(INT64, int64_t);
    SHALLOW_SWAP_ARRAYS(UINT32, uint32_t);
    SHALLOW_SWAP_ARRAYS(UINT64, uint64_t);
    SHALLOW_SWAP_ARRAYS(FLOAT, float);
    SHALLOW_SWAP_ARRAYS(DOUBLE, double);
    SHALLOW_SWAP_ARRAYS(BOOL, bool);
    SHALLOW_SWAP_ARRAYS(ENUM, int);
#undef SHALLOW_SWAP_ARRAYS

    case FieldDescriptor::CPPTYPE_STRING:
      internal::SwapFieldHelper::SwapRepeatedStringField<true>(this, message1,
                                                               message2, field);
      break;
    case FieldDescriptor::CPPTYPE_MESSAGE:
      internal::SwapFieldHelper::SwapRepeatedMessageField<true>(
          this, message1, message2, field);
      break;

    default:
      ABSL_LOG(FATAL) << "Unimplemented type: " << field->cpp_type();
  }
}

// Swaps oneof field between lhs and rhs. If unsafe_shallow_swap is true, it
// directly swaps oneof values; otherwise, it may involve copy/delete. Note that
// two messages may have different oneof cases. So, it has to be done in three
// steps (i.e. lhs -> temp, rhs -> lhs, temp -> rhs).
template <bool unsafe_shallow_swap>
void Reflection::SwapOneofField(Message* lhs, Message* rhs,
                                const OneofDescriptor* oneof_descriptor) const {
  // Wraps a local variable to temporarily store oneof value.
  struct LocalVarWrapper {
#define LOCAL_VAR_ACCESSOR(type, var, name)               \
  type Get##name() const { return oneof_val.type_##var; } \
  void Set##name(type v) { oneof_val.type_##var = v; }

    LOCAL_VAR_ACCESSOR(int32_t, int32, Int32);
    LOCAL_VAR_ACCESSOR(int64_t, int64, Int64);
    LOCAL_VAR_ACCESSOR(uint32_t, uint32, Uint32);
    LOCAL_VAR_ACCESSOR(uint64_t, uint64, Uint64);
    LOCAL_VAR_ACCESSOR(float, float, Float);
    LOCAL_VAR_ACCESSOR(double, double, Double);
    LOCAL_VAR_ACCESSOR(bool, bool, Bool);
    LOCAL_VAR_ACCESSOR(int, enum, Enum);
    LOCAL_VAR_ACCESSOR(Message*, message, Message);
    LOCAL_VAR_ACCESSOR(ArenaStringPtr, arena_string_ptr, ArenaStringPtr);
    LOCAL_VAR_ACCESSOR(absl::Cord*, cord, Cord);
    const std::string& GetString() const { return string_val; }
    void SetString(const std::string& v) { string_val = v; }
    Message* UnsafeGetMessage() const { return GetMessage(); }
    void UnsafeSetMessage(Message* v) { SetMessage(v); }
    void ClearOneofCase() {}

    union {
      int32_t type_int32;
      int64_t type_int64;
      uint32_t type_uint32;
      uint64_t type_uint64;
      float type_float;
      double type_double;
      bool type_bool;
      int type_enum;
      Message* type_message;
      internal::ArenaStringPtr type_arena_string_ptr;
      absl::Cord* type_cord;
    } oneof_val;

    // std::string cannot be in union.
    std::string string_val;
  };

  // Wraps a message pointer to read and write a field.
  struct MessageWrapper {
#define MESSAGE_FIELD_ACCESSOR(type, var, name)         \
  type Get##name() const {                              \
    return reflection->GetField<type>(*message, field); \
  }                                                     \
  void Set##name(type v) { reflection->SetField<type>(message, field, v); }

    MESSAGE_FIELD_ACCESSOR(int32_t, int32, Int32);
    MESSAGE_FIELD_ACCESSOR(int64_t, int64, Int64);
    MESSAGE_FIELD_ACCESSOR(uint32_t, uint32, Uint32);
    MESSAGE_FIELD_ACCESSOR(uint64_t, uint64, Uint64);
    MESSAGE_FIELD_ACCESSOR(float, float, Float);
    MESSAGE_FIELD_ACCESSOR(double, double, Double);
    MESSAGE_FIELD_ACCESSOR(bool, bool, Bool);
    MESSAGE_FIELD_ACCESSOR(int, enum, Enum);
    MESSAGE_FIELD_ACCESSOR(ArenaStringPtr, arena_string_ptr, ArenaStringPtr);
    MESSAGE_FIELD_ACCESSOR(absl::Cord*, cord, Cord);
    std::string GetString() const {
      return reflection->GetString(*message, field);
    }
    void SetString(const std::string& v) {
      reflection->SetString(message, field, v);
    }
    Message* GetMessage() const {
      return reflection->ReleaseMessage(message, field);
    }
    void SetMessage(Message* v) {
      reflection->SetAllocatedMessage(message, v, field);
    }
    Message* UnsafeGetMessage() const {
      return reflection->UnsafeArenaReleaseMessage(message, field);
    }
    void UnsafeSetMessage(Message* v) {
      reflection->UnsafeArenaSetAllocatedMessage(message, v, field);
    }
    void ClearOneofCase() {
      *reflection->MutableOneofCase(message, field->containing_oneof()) = 0;
    }

    const Reflection* reflection;
    Message* message;
    const FieldDescriptor* field;
  };

  ABSL_DCHECK(!OneofDescriptorLegacy(oneof_descriptor).is_synthetic());
  uint32_t oneof_case_lhs = GetOneofCase(*lhs, oneof_descriptor);
  uint32_t oneof_case_rhs = GetOneofCase(*rhs, oneof_descriptor);

  LocalVarWrapper temp;
  MessageWrapper lhs_wrapper, rhs_wrapper;
  const FieldDescriptor* field_lhs = nullptr;
  OneofFieldMover<unsafe_shallow_swap> mover;
  // lhs --> temp
  if (oneof_case_lhs > 0) {
    field_lhs = descriptor_->FindFieldByNumber(oneof_case_lhs);
    lhs_wrapper = {this, lhs, field_lhs};
    mover(field_lhs, &lhs_wrapper, &temp);
  }
  // rhs --> lhs
  if (oneof_case_rhs > 0) {
    const FieldDescriptor* f = descriptor_->FindFieldByNumber(oneof_case_rhs);
    lhs_wrapper = {this, lhs, f};
    rhs_wrapper = {this, rhs, f};
    mover(f, &rhs_wrapper, &lhs_wrapper);
  } else if (!unsafe_shallow_swap) {
    ClearOneof(lhs, oneof_descriptor);
  }
  // temp --> rhs
  if (oneof_case_lhs > 0) {
    rhs_wrapper = {this, rhs, field_lhs};
    mover(field_lhs, &temp, &rhs_wrapper);
  } else if (!unsafe_shallow_swap) {
    ClearOneof(rhs, oneof_descriptor);
  }

  if (unsafe_shallow_swap) {
    *MutableOneofCase(lhs, oneof_descriptor) = oneof_case_rhs;
    *MutableOneofCase(rhs, oneof_descriptor) = oneof_case_lhs;
  }
}

void Reflection::Swap(Message* message1, Message* message2) const {
  if (message1 == message2) return;

  // TODO:  Other Reflection methods should probably check this too.
  ABSL_CHECK_EQ(message1->GetReflection(), this)
      << "First argument to Swap() (of type \""
      << message1->GetDescriptor()->full_name()
      << "\") is not compatible with this reflection object (which is for type "
         "\""
      << descriptor_->full_name()
      << "\").  Note that the exact same class is required; not just the same "
         "descriptor.";
  ABSL_CHECK_EQ(message2->GetReflection(), this)
      << "Second argument to Swap() (of type \""
      << message2->GetDescriptor()->full_name()
      << "\") is not compatible with this reflection object (which is for type "
         "\""
      << descriptor_->full_name()
      << "\").  Note that the exact same class is required; not just the same "
         "descriptor.";

  // Check that both messages are in the same arena (or both on the heap). We
  // need to copy all data if not, due to ownership semantics.
#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
  if (message1->GetArena() == nullptr ||
      message1->GetArena() != message2->GetArena()) {
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
  if (message1->GetArena() != message2->GetArena()) {
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
    // One of the two is guaranteed to have an arena.  Switch things around
    // to guarantee that message1 has an arena.
    Arena* arena = message1->GetArena();
    if (arena == nullptr) {
      arena = message2->GetArena();
      std::swap(message1, message2);  // Swapping names for pointers!
    }

    Message* temp = message1->New(arena);
    temp->MergeFrom(*message2);
    message2->CopyFrom(*message1);
#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    message1->CopyFrom(*temp);
    if (arena == nullptr) delete temp;
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
    Swap(message1, temp);
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
    return;
  }

  UnsafeArenaSwap(message1, message2);
}

template <bool unsafe_shallow_swap>
void Reflection::SwapFieldsImpl(
    Message* message1, Message* message2,
    const std::vector<const FieldDescriptor*>& fields) const {
  if (message1 == message2) return;

  // TODO:  Other Reflection methods should probably check this too.
  ABSL_CHECK_EQ(message1->GetReflection(), this)
      << "First argument to SwapFields() (of type \""
      << message1->GetDescriptor()->full_name()
      << "\") is not compatible with this reflection object (which is for type "
         "\""
      << descriptor_->full_name()
      << "\").  Note that the exact same class is required; not just the same "
         "descriptor.";
  ABSL_CHECK_EQ(message2->GetReflection(), this)
      << "Second argument to SwapFields() (of type \""
      << message2->GetDescriptor()->full_name()
      << "\") is not compatible with this reflection object (which is for type "
         "\""
      << descriptor_->full_name()
      << "\").  Note that the exact same class is required; not just the same "
         "descriptor.";

  absl::flat_hash_set<int> swapped_oneof;

  const Message* prototype =
      message_factory_->GetPrototype(message1->GetDescriptor());
  for (const auto* field : fields) {
    if (field->is_extension()) {
      if (unsafe_shallow_swap) {
        MutableExtensionSet(message1)->UnsafeShallowSwapExtension(
            MutableExtensionSet(message2), field->number());
      } else {
        MutableExtensionSet(message1)->SwapExtension(
            prototype, MutableExtensionSet(message2), field->number());
      }
    } else {
      if (schema_.InRealOneof(field)) {
        int oneof_index = field->containing_oneof()->index();
        // Only swap the oneof field once.
        if (!swapped_oneof.insert(oneof_index).second) {
          continue;
        }
        SwapOneofField<unsafe_shallow_swap>(message1, message2,
                                            field->containing_oneof());
      } else {
        // Swap field.
        if (unsafe_shallow_swap) {
          UnsafeShallowSwapField(message1, message2, field);
        } else {
          SwapField(message1, message2, field);
        }
        // Swap has bit for non-repeated fields.  We have already checked for
        // oneof already. This has to be done after SwapField, because SwapField
        // may depend on the information in has bits.
        if (!field->is_repeated()) {
          SwapBit(message1, message2, field);
          if (field->options().ctype() == FieldOptions::STRING &&
              IsInlined(field)) {
            ABSL_DCHECK(!unsafe_shallow_swap ||
                        message1->GetArena() == message2->GetArena());
            SwapInlinedStringDonated(message1, message2, field);
          }
        }
      }
    }
  }
}

void Reflection::SwapFields(
    Message* message1, Message* message2,
    const std::vector<const FieldDescriptor*>& fields) const {
  SwapFieldsImpl<false>(message1, message2, fields);
}

void Reflection::UnsafeShallowSwapFields(
    Message* message1, Message* message2,
    const std::vector<const FieldDescriptor*>& fields) const {
  ABSL_DCHECK_EQ(message1->GetArena(), message2->GetArena());

  SwapFieldsImpl<true>(message1, message2, fields);
}

void Reflection::UnsafeArenaSwapFields(
    Message* lhs, Message* rhs,
    const std::vector<const FieldDescriptor*>& fields) const {
  ABSL_DCHECK_EQ(lhs->GetArena(), rhs->GetArena());
  UnsafeShallowSwapFields(lhs, rhs, fields);
}

// -------------------------------------------------------------------

bool Reflection::HasField(const Message& message,
                          const FieldDescriptor* field) const {
  USAGE_CHECK_MESSAGE(HasField, &message);
  USAGE_CHECK_MESSAGE_TYPE(HasField);
  USAGE_CHECK_SINGULAR(HasField);

  if (field->is_extension()) {
    return GetExtensionSet(message).Has(field->number());
  } else {
    if (schema_.InRealOneof(field)) {
      return HasOneofField(message, field);
    } else {
      return HasBit(message, field);
    }
  }
}

void Reflection::UnsafeArenaSwap(Message* lhs, Message* rhs) const {
  ABSL_DCHECK_EQ(lhs->GetArena(), rhs->GetArena());
  InternalSwap(lhs, rhs);
}

void Reflection::InternalSwap(Message* lhs, Message* rhs) const {
  if (lhs == rhs) return;

  MutableInternalMetadata(lhs)->InternalSwap(MutableInternalMetadata(rhs));

  for (int i = 0; i <= last_non_weak_field_index_; i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (schema_.InRealOneof(field)) continue;
    if (schema_.IsSplit(field)) {
      continue;
    }
    UnsafeShallowSwapField(lhs, rhs, field);
  }
  if (schema_.IsSplit()) {
    std::swap(*MutableSplitField(lhs), *MutableSplitField(rhs));
  }
  const int oneof_decl_count = descriptor_->oneof_decl_count();
  for (int i = 0; i < oneof_decl_count; i++) {
    const OneofDescriptor* oneof = descriptor_->oneof_decl(i);
    if (!OneofDescriptorLegacy(oneof).is_synthetic()) {
      SwapOneofField<true>(lhs, rhs, oneof);
    }
  }

  // Swapping bits need to happen after swapping fields, because the latter may
  // depend on the has bit information.
  if (schema_.HasHasbits()) {
    uint32_t* lhs_has_bits = MutableHasBits(lhs);
    uint32_t* rhs_has_bits = MutableHasBits(rhs);

    int fields_with_has_bits = 0;
    for (int i = 0; i < descriptor_->field_count(); i++) {
      const FieldDescriptor* field = descriptor_->field(i);
      if (field->is_repeated() || schema_.InRealOneof(field)) {
        continue;
      }
      fields_with_has_bits++;
    }

    int has_bits_size = (fields_with_has_bits + 31) / 32;

    for (int i = 0; i < has_bits_size; i++) {
      std::swap(lhs_has_bits[i], rhs_has_bits[i]);
    }
  }

  if (schema_.HasInlinedString()) {
    uint32_t* lhs_donated_array = MutableInlinedStringDonatedArray(lhs);
    uint32_t* rhs_donated_array = MutableInlinedStringDonatedArray(rhs);
    int inlined_string_count = 0;
    for (int i = 0; i < descriptor_->field_count(); i++) {
      const FieldDescriptor* field = descriptor_->field(i);
      if (field->is_extension() || field->is_repeated() ||
          schema_.InRealOneof(field) ||
          field->options().ctype() != FieldOptions::STRING ||
          !IsInlined(field)) {
        continue;
      }
      inlined_string_count++;
    }

    int donated_array_size = inlined_string_count == 0
                                 ? 0
                                 // One extra bit for the arena dtor tracking.
                                 : (inlined_string_count + 1 + 31) / 32;
    ABSL_CHECK_EQ((lhs_donated_array[0] & 0x1u) == 0,
                  (rhs_donated_array[0] & 0x1u) == 0);
    for (int i = 0; i < donated_array_size; i++) {
      std::swap(lhs_donated_array[i], rhs_donated_array[i]);
    }
  }

  if (schema_.HasExtensionSet()) {
    MutableExtensionSet(lhs)->InternalSwap(MutableExtensionSet(rhs));
  }
}

int Reflection::FieldSize(const Message& message,
                          const FieldDescriptor* field) const {
  USAGE_CHECK_MESSAGE(FieldSize, &message);
  USAGE_CHECK_MESSAGE_TYPE(FieldSize);
  USAGE_CHECK_REPEATED(FieldSize);

  if (field->is_extension()) {
    return GetExtensionSet(message).ExtensionSize(field->number());
  } else {
    switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, LOWERCASE)    \
  case FieldDescriptor::CPPTYPE_##UPPERCASE: \
    return GetRaw<RepeatedField<LOWERCASE> >(message, field).size()

      HANDLE_TYPE(INT32, int32_t);
      HANDLE_TYPE(INT64, int64_t);
      HANDLE_TYPE(UINT32, uint32_t);
      HANDLE_TYPE(UINT64, uint64_t);
      HANDLE_TYPE(DOUBLE, double);
      HANDLE_TYPE(FLOAT, float);
      HANDLE_TYPE(BOOL, bool);
      HANDLE_TYPE(ENUM, int);
#undef HANDLE_TYPE

      case FieldDescriptor::CPPTYPE_STRING:
      case FieldDescriptor::CPPTYPE_MESSAGE:
        if (IsMapFieldInApi(field)) {
          const internal::MapFieldBase& map =
              GetRaw<MapFieldBase>(message, field);
          if (map.IsRepeatedFieldValid()) {
            return map.GetRepeatedField().size();
          } else {
            // No need to materialize the repeated field if it is out of sync:
            // its size will be the same as the map's size.
            return map.size();
          }
        } else {
          return GetRaw<RepeatedPtrFieldBase>(message, field).size();
        }
    }

    ABSL_LOG(FATAL) << "Can't get here.";
    return 0;
  }
}

void Reflection::ClearField(Message* message,
                            const FieldDescriptor* field) const {
  USAGE_CHECK_MESSAGE(ClearField, message);
  USAGE_CHECK_MESSAGE_TYPE(ClearField);

  if (field->is_extension()) {
    MutableExtensionSet(message)->ClearExtension(field->number());
  } else if (!field->is_repeated()) {
    if (schema_.InRealOneof(field)) {
      ClearOneofField(message, field);
      return;
    }
    if (HasBit(*message, field)) {
      ClearBit(message, field);

      // We need to set the field back to its default value.
      switch (field->cpp_type()) {
#define CLEAR_TYPE(CPPTYPE, TYPE)                                      \
  case FieldDescriptor::CPPTYPE_##CPPTYPE:                             \
    *MutableRaw<TYPE>(message, field) = field->default_value_##TYPE(); \
    break;

        CLEAR_TYPE(INT32, int32_t);
        CLEAR_TYPE(INT64, int64_t);
        CLEAR_TYPE(UINT32, uint32_t);
        CLEAR_TYPE(UINT64, uint64_t);
        CLEAR_TYPE(FLOAT, float);
        CLEAR_TYPE(DOUBLE, double);
        CLEAR_TYPE(BOOL, bool);
#undef CLEAR_TYPE

        case FieldDescriptor::CPPTYPE_ENUM:
          *MutableRaw<int>(message, field) =
              field->default_value_enum()->number();
          break;

        case FieldDescriptor::CPPTYPE_STRING: {
          switch (internal::cpp::EffectiveStringCType(field)) {
            case FieldOptions::CORD:
              if (field->has_default_value()) {
                *MutableRaw<absl::Cord>(message, field) =
                    field->default_value_string();
              } else {
                MutableRaw<absl::Cord>(message, field)->Clear();
              }
              break;
            default:
            case FieldOptions::STRING:
              if (IsInlined(field)) {
                // Currently, string with default value can't be inlined. So we
                // don't have to handle default value here.
                MutableRaw<InlinedStringField>(message, field)->ClearToEmpty();
              } else {
                auto* str = MutableRaw<ArenaStringPtr>(message, field);
                str->Destroy();
                str->InitDefault();
              }
              break;
          }
          break;
        }

        case FieldDescriptor::CPPTYPE_MESSAGE:
          if (schema_.HasBitIndex(field) == static_cast<uint32_t>(-1)) {
            // Proto3 does not have has-bits and we need to set a message field
            // to nullptr in order to indicate its un-presence.
            if (message->GetArena() == nullptr) {
              delete *MutableRaw<Message*>(message, field);
            }
            *MutableRaw<Message*>(message, field) = nullptr;
          } else {
            (*MutableRaw<Message*>(message, field))->Clear();
          }
          break;
      }
    }
  } else {
    switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, LOWERCASE)                           \
  case FieldDescriptor::CPPTYPE_##UPPERCASE:                        \
    MutableRaw<RepeatedField<LOWERCASE> >(message, field)->Clear(); \
    break

      HANDLE_TYPE(INT32, int32_t);
      HANDLE_TYPE(INT64, int64_t);
      HANDLE_TYPE(UINT32, uint32_t);
      HANDLE_TYPE(UINT64, uint64_t);
      HANDLE_TYPE(DOUBLE, double);
      HANDLE_TYPE(FLOAT, float);
      HANDLE_TYPE(BOOL, bool);
      HANDLE_TYPE(ENUM, int);
#undef HANDLE_TYPE

      case FieldDescriptor::CPPTYPE_STRING: {
        switch (field->options().ctype()) {
          default:  // TODO:  Support other string reps.
          case FieldOptions::STRING:
            MutableRaw<RepeatedPtrField<std::string> >(message, field)->Clear();
            break;
        }
        break;
      }

      case FieldDescriptor::CPPTYPE_MESSAGE: {
        if (IsMapFieldInApi(field)) {
          MutableRaw<MapFieldBase>(message, field)->Clear();
        } else {
          // We don't know which subclass of RepeatedPtrFieldBase the type is,
          // so we use RepeatedPtrFieldBase directly.
          MutableRaw<RepeatedPtrFieldBase>(message, field)
              ->Clear<GenericTypeHandler<Message> >();
        }
        break;
      }
    }
  }
}

void Reflection::RemoveLast(Message* message,
                            const FieldDescriptor* field) const {
  USAGE_CHECK_MESSAGE(RemoveLast, message);
  USAGE_CHECK_MESSAGE_TYPE(RemoveLast);
  USAGE_CHECK_REPEATED(RemoveLast);

  if (field->is_extension()) {
    MutableExtensionSet(message)->RemoveLast(field->number());
  } else {
    switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, LOWERCASE)                                \
  case FieldDescriptor::CPPTYPE_##UPPERCASE:                             \
    MutableRaw<RepeatedField<LOWERCASE> >(message, field)->RemoveLast(); \
    break

      HANDLE_TYPE(INT32, int32_t);
      HANDLE_TYPE(INT64, int64_t);
      HANDLE_TYPE(UINT32, uint32_t);
      HANDLE_TYPE(UINT64, uint64_t);
      HANDLE_TYPE(DOUBLE, double);
      HANDLE_TYPE(FLOAT, float);
      HANDLE_TYPE(BOOL, bool);
      HANDLE_TYPE(ENUM, int);
#undef HANDLE_TYPE

      case FieldDescriptor::CPPTYPE_STRING:
        switch (field->options().ctype()) {
          default:  // TODO:  Support other string reps.
          case FieldOptions::STRING:
            MutableRaw<RepeatedPtrField<std::string> >(message, field)
                ->RemoveLast();
            break;
        }
        break;

      case FieldDescriptor::CPPTYPE_MESSAGE:
        if (IsMapFieldInApi(field)) {
          MutableRaw<MapFieldBase>(message, field)
              ->MutableRepeatedField()
              ->RemoveLast<GenericTypeHandler<Message> >();
        } else {
          MutableRaw<RepeatedPtrFieldBase>(message, field)
              ->RemoveLast<GenericTypeHandler<Message> >();
        }
        break;
    }
  }
}

Message* Reflection::ReleaseLast(Message* message,
                                 const FieldDescriptor* field) const {
  USAGE_MUTABLE_CHECK_ALL(ReleaseLast, REPEATED, MESSAGE);

  Message* released;
  if (field->is_extension()) {
    released = static_cast<Message*>(
        MutableExtensionSet(message)->ReleaseLast(field->number()));
  } else {
    if (IsMapFieldInApi(field)) {
      released = MutableRaw<MapFieldBase>(message, field)
                     ->MutableRepeatedField()
                     ->ReleaseLast<GenericTypeHandler<Message>>();
    } else {
      released = MutableRaw<RepeatedPtrFieldBase>(message, field)
                     ->ReleaseLast<GenericTypeHandler<Message>>();
    }
  }
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
  return MaybeForceCopy(message->GetArena(), released);
#else   // PROTOBUF_FORCE_COPY_IN_RELEASE
  return released;
#endif  // !PROTOBUF_FORCE_COPY_IN_RELEASE
}

Message* Reflection::UnsafeArenaReleaseLast(
    Message* message, const FieldDescriptor* field) const {
  USAGE_MUTABLE_CHECK_ALL(UnsafeArenaReleaseLast, REPEATED, MESSAGE);

  if (field->is_extension()) {
    return static_cast<Message*>(
        MutableExtensionSet(message)->UnsafeArenaReleaseLast(field->number()));
  } else {
    if (IsMapFieldInApi(field)) {
      return MutableRaw<MapFieldBase>(message, field)
          ->MutableRepeatedField()
          ->UnsafeArenaReleaseLast<GenericTypeHandler<Message>>();
    } else {
      return MutableRaw<RepeatedPtrFieldBase>(message, field)
          ->UnsafeArenaReleaseLast<GenericTypeHandler<Message>>();
    }
  }
}

void Reflection::SwapElements(Message* message, const FieldDescriptor* field,
                              int index1, int index2) const {
  USAGE_CHECK_MESSAGE(Swap, message);
  USAGE_CHECK_MESSAGE_TYPE(Swap);
  USAGE_CHECK_REPEATED(Swap);

  if (field->is_extension()) {
    MutableExtensionSet(message)->SwapElements(field->number(), index1, index2);
  } else {
    switch (field->cpp_type()) {
#define HANDLE_TYPE(UPPERCASE, LOWERCASE)                 \
  case FieldDescriptor::CPPTYPE_##UPPERCASE:              \
    MutableRaw<RepeatedField<LOWERCASE> >(message, field) \
        ->SwapElements(index1, index2);                   \
    break

      HANDLE_TYPE(INT32, int32_t);
      HANDLE_TYPE(INT64, int64_t);
      HANDLE_TYPE(UINT32, uint32_t);
      HANDLE_TYPE(UINT64, uint64_t);
      HANDLE_TYPE(DOUBLE, double);
      HANDLE_TYPE(FLOAT, float);
      HANDLE_TYPE(BOOL, bool);
      HANDLE_TYPE(ENUM, int);
#undef HANDLE_TYPE

      case FieldDescriptor::CPPTYPE_STRING:
      case FieldDescriptor::CPPTYPE_MESSAGE:
        if (IsMapFieldInApi(field)) {
          MutableRaw<MapFieldBase>(message, field)
              ->MutableRepeatedField()
              ->SwapElements(index1, index2);
        } else {
          MutableRaw<RepeatedPtrFieldBase>(message, field)
              ->SwapElements(index1, index2);
        }
        break;
    }
  }
}

namespace {
// Comparison functor for sorting FieldDescriptors by field number.
struct FieldNumberSorter {
  bool operator()(const FieldDescriptor* left,
                  const FieldDescriptor* right) const {
    return left->number() < right->number();
  }
};

bool IsIndexInHasBitSet(const uint32_t* has_bit_set, uint32_t has_bit_index) {
  ABSL_DCHECK_NE(has_bit_index, ~0u);
  return ((has_bit_set[has_bit_index / 32] >> (has_bit_index % 32)) &
          static_cast<uint32_t>(1)) != 0;
}

void CheckInOrder(const FieldDescriptor* field, uint32_t* last) {
  *last = *last <= static_cast<uint32_t>(field->number())
              ? static_cast<uint32_t>(field->number())
              : UINT32_MAX;
}

}  // namespace

namespace internal {
bool CreateUnknownEnumValues(const FieldDescriptor* field) {
  bool open_enum = false;
  return !field->legacy_enum_field_treated_as_closed() || open_enum;
}
}  // namespace internal
using internal::CreateUnknownEnumValues;

void Reflection::ListFields(const Message& message,
                            std::vector<const FieldDescriptor*>* output) const {
  output->clear();

  // Optimization:  The default instance never has any fields set.
  if (schema_.IsDefaultInstance(message)) return;

  // Optimization: Avoid calling GetHasBits() and HasOneofField() many times
  // within the field loop.  We allow this violation of ReflectionSchema
  // encapsulation because this function takes a noticeable about of CPU
  // fleetwide and properly allowing this optimization through public interfaces
  // seems more trouble than it is worth.
  const uint32_t* const has_bits =
      schema_.HasHasbits() ? GetHasBits(message) : nullptr;
  const uint32_t* const has_bits_indices = schema_.has_bit_indices_;
  output->reserve(descriptor_->field_count());
  const int last_non_weak_field_index = last_non_weak_field_index_;
  // Fields in messages are usually added with the increasing tags.
  uint32_t last = 0;  // UINT32_MAX if out-of-order
  auto append_to_output = [&last, &output](const FieldDescriptor* field) {
    CheckInOrder(field, &last);
    output->push_back(field);
  };
  for (int i = 0; i <= last_non_weak_field_index; i++) {
    const FieldDescriptor* field = descriptor_->field(i);
    if (field->is_repeated()) {
      if (FieldSize(message, field) > 0) {
        append_to_output(field);
      }
    } else {
      const OneofDescriptor* containing_oneof = field->containing_oneof();
      if (schema_.InRealOneof(field)) {
        const uint32_t* const oneof_case_array =
            GetConstPointerAtOffset<uint32_t>(&message,
                                              schema_.oneof_case_offset_);
        // Equivalent to: HasOneofField(message, field)
        if (static_cast<int64_t>(oneof_case_array[containing_oneof->index()]) ==
            field->number()) {
          append_to_output(field);
        }
      } else if (has_bits && has_bits_indices[i] != static_cast<uint32_t>(-1)) {
        // Equivalent to: HasBit(message, field)
        if (IsIndexInHasBitSet(has_bits, has_bits_indices[i])) {
          append_to_output(field);
        }
      } else if (HasBit(message, field)) {  // Fall back on proto3-style HasBit.
        append_to_output(field);
      }
    }
  }
  // Descriptors of ExtensionSet are appended in their increasing tag
  // order and they are usually bigger than the field tags so if all fields are
  // not sorted, let them be sorted.
  if (last == UINT32_MAX) {
    std::sort(output->begin(), output->end(), FieldNumberSorter());
    last = output->back()->number();
  }
  size_t last_size = output->size();
  if (schema_.HasExtensionSet()) {
    // Descriptors of ExtensionSet are appended in their increasing order.
    GetExtensionSet(message).AppendToList(descriptor_, descriptor_pool_,
                                          output);
    ABSL_DCHECK(std::is_sorted(output->begin() + last_size, output->end(),
                               FieldNumberSorter()));
    if (output->size() != last_size) {
      CheckInOrder((*output)[last_size], &last);
    }
  }
  if (last != UINT32_MAX) {
    ABSL_DCHECK(
        std::is_sorted(output->begin(), output->end(), FieldNumberSorter()));
  } else {
    // ListFields() must sort output by field number.
    std::sort(output->begin(), output->end(), FieldNumberSorter());
  }
}

// -------------------------------------------------------------------

#undef DEFINE_PRIMITIVE_ACCESSORS
#define DEFINE_PRIMITIVE_ACCESSORS(TYPENAME, TYPE, PASSTYPE, CPPTYPE)          \
  PASSTYPE Reflection::Get##TYPENAME(const Message& message,                   \
                                     const FieldDescriptor* field) const {     \
    USAGE_CHECK_ALL(Get##TYPENAME, SINGULAR, CPPTYPE);                         \
    if (field->is_extension()) {                                               \
      return GetExtensionSet(message).Get##TYPENAME(                           \
          field->number(), field->default_value_##PASSTYPE());                 \
    } else if (schema_.InRealOneof(field) && !HasOneofField(message, field)) { \
      return field->default_value_##PASSTYPE();                                \
    } else {                                                                   \
      return GetField<TYPE>(message, field);                                   \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Reflection::Set##TYPENAME(                                              \
      Message* message, const FieldDescriptor* field, PASSTYPE value) const {  \
    USAGE_MUTABLE_CHECK_ALL(Set##TYPENAME, SINGULAR, CPPTYPE);                 \
    if (field->is_extension()) {                                               \
      return MutableExtensionSet(message)->Set##TYPENAME(                      \
          field->number(), field->type(), value, field);                       \
    } else {                                                                   \
      SetField<TYPE>(message, field, value);                                   \
    }                                                                          \
  }                                                                            \
                                                                               \
  PASSTYPE Reflection::GetRepeated##TYPENAME(                                  \
      const Message& message, const FieldDescriptor* field, int index) const { \
    USAGE_CHECK_ALL(GetRepeated##TYPENAME, REPEATED, CPPTYPE);                 \
    if (field->is_extension()) {                                               \
      return GetExtensionSet(message).GetRepeated##TYPENAME(field->number(),   \
                                                            index);            \
    } else {                                                                   \
      return GetRepeatedField<TYPE>(message, field, index);                    \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Reflection::SetRepeated##TYPENAME(Message* message,                     \
                                         const FieldDescriptor* field,         \
                                         int index, PASSTYPE value) const {    \
    USAGE_MUTABLE_CHECK_ALL(SetRepeated##TYPENAME, REPEATED, CPPTYPE);         \
    if (field->is_extension()) {                                               \
      MutableExtensionSet(message)->SetRepeated##TYPENAME(field->number(),     \
                                                          index, value);       \
    } else {                                                                   \
      SetRepeatedField<TYPE>(message, field, index, value);                    \
    }                                                                          \
  }                                                                            \
                                                                               \
  void Reflection::Add##TYPENAME(                                              \
      Message* message, const FieldDescriptor* field, PASSTYPE value) const {  \
    USAGE_MUTABLE_CHECK_ALL(Add##TYPENAME, REPEATED, CPPTYPE);                 \
    if (field->is_extension()) {                                               \
      MutableExtensionSet(message)->Add##TYPENAME(                             \
          field->number(), field->type(), field->options().packed(), value,    \
          field);                                                              \
    } else {                                                                   \
      AddField<TYPE>(message, field, value);                                   \
    }                                                                          \
  }

DEFINE_PRIMITIVE_ACCESSORS(Int32, int32_t, int32_t, INT32)
DEFINE_PRIMITIVE_ACCESSORS(Int64, int64_t, int64_t, INT64)
DEFINE_PRIMITIVE_ACCESSORS(UInt32, uint32_t, uint32_t, UINT32)
DEFINE_PRIMITIVE_ACCESSORS(UInt64, uint64_t, uint64_t, UINT64)
DEFINE_PRIMITIVE_ACCESSORS(Float, float, float, FLOAT)
DEFINE_PRIMITIVE_ACCESSORS(Double, double, double, DOUBLE)
DEFINE_PRIMITIVE_ACCESSORS(Bool, bool, bool, BOOL)
#undef DEFINE_PRIMITIVE_ACCESSORS

// -------------------------------------------------------------------

std::string Reflection::GetString(const Message& message,
                                  const FieldDescriptor* field) const {
  USAGE_CHECK_ALL(GetString, SINGULAR, STRING);
  if (field->is_extension()) {
    return GetExtensionSet(message).GetString(field->number(),
                                              field->default_value_string());
  } else {
    if (schema_.InRealOneof(field) && !HasOneofField(message, field)) {
      return field->default_value_string();
    }
    switch (internal::cpp::EffectiveStringCType(field)) {
      case FieldOptions::CORD:
        if (schema_.InRealOneof(field)) {
          return std::string(*GetField<absl::Cord*>(message, field));
        } else {
          return std::string(GetField<absl::Cord>(message, field));
        }
      default:
      case FieldOptions::STRING:
        if (IsInlined(field)) {
          return GetField<InlinedStringField>(message, field).GetNoArena();
        } else {
          const auto& str = GetField<ArenaStringPtr>(message, field);
          return str.IsDefault() ? field->default_value_string() : str.Get();
        }
    }
  }
}

const std::string& Reflection::GetStringReference(const Message& message,
                                                  const FieldDescriptor* field,
                                                  std::string* scratch) const {
  (void)scratch;  // Parameter is used by Google-internal code.
  USAGE_CHECK_ALL(GetStringReference, SINGULAR, STRING);
  if (field->is_extension()) {
    return GetExtensionSet(message).GetString(field->number(),
                                              field->default_value_string());
  } else {
    if (schema_.InRealOneof(field) && !HasOneofField(message, field)) {
      return field->default_value_string();
    }
    switch (internal::cpp::EffectiveStringCType(field)) {
      case FieldOptions::CORD:
        if (schema_.InRealOneof(field)) {
          absl::CopyCordToString(*GetField<absl::Cord*>(message, field),
                                 scratch);
        } else {
          absl::CopyCordToString(GetField<absl::Cord>(message, field), scratch);
        }
        return *scratch;
      default:
      case FieldOptions::STRING:
        if (IsInlined(field)) {
          return GetField<InlinedStringField>(message, field).GetNoArena();
        } else {
          const auto& str = GetField<ArenaStringPtr>(message, field);
          return str.IsDefault() ? field->default_value_string() : str.Get();
        }
    }
  }
}

absl::Cord Reflection::GetCord(const Message& message,
                               const FieldDescriptor* field) const {
  USAGE_CHECK_ALL(GetCord, SINGULAR, STRING);
  if (field->is_extension()) {
    return absl::Cord(GetExtensionSet(message).GetString(
        field->number(), field->default_value_string()));
  } else {
    if (schema_.InRealOneof(field) && !HasOneofField(message, field)) {
      return absl::Cord(field->default_value_string());
    }
    switch (internal::cpp::EffectiveStringCType(field)) {
      case FieldOptions::CORD:
        if (schema_.InRealOneof(field)) {
          return *GetField<absl::Cord*>(message, field);
        } else {
          return GetField<absl::Cord>(message, field);
        }
      default:
      case FieldOptions::STRING:
        if (IsInlined(field)) {
          return absl::Cord(
              GetField<InlinedStringField>(message, field).GetNoArena());
        } else {
          const auto& str = GetField<ArenaStringPtr>(message, field);
          return absl::Cord(str.IsDefault() ? field->default_value_string()
                                            : str.Get());
        }
    }

    ABSL_LOG(FATAL) << "Can't get here.";
    return absl::Cord();  // Make compiler happy.
  }
}


void Reflection::SetString(Message* message, const FieldDescriptor* field,
                           std::string value) const {
  USAGE_MUTABLE_CHECK_ALL(SetString, SINGULAR, STRING);
  if (field->is_extension()) {
    return MutableExtensionSet(message)->SetString(
        field->number(), field->type(), std::move(value), field);
  } else {
    switch (internal::cpp::EffectiveStringCType(field)) {
      case FieldOptions::CORD:
        if (schema_.InRealOneof(field)) {
          if (!HasOneofField(*message, field)) {
            ClearOneof(message, field->containing_oneof());
            *MutableField<absl::Cord*>(message, field) =
                Arena::Create<absl::Cord>(message->GetArena());
          }
          *(*MutableField<absl::Cord*>(message, field)) = value;
          break;
        }
        *MutableField<absl::Cord>(message, field) = value;
        break;
      default:
      case FieldOptions::STRING: {
        if (IsInlined(field)) {
          const uint32_t index = schema_.InlinedStringIndex(field);
          ABSL_DCHECK_GT(index, 0);
          uint32_t* states =
              &MutableInlinedStringDonatedArray(message)[index / 32];
          uint32_t mask = ~(static_cast<uint32_t>(1) << (index % 32));
          MutableField<InlinedStringField>(message, field)
              ->Set(value, message->GetArena(),
                    IsInlinedStringDonated(*message, field), states, mask,
                    message);
          break;
        }

        // Oneof string fields are never set as a default instance.
        // We just need to pass some arbitrary default string to make it work.
        // This allows us to not have the real default accessible from
        // reflection.
        if (schema_.InRealOneof(field) && !HasOneofField(*message, field)) {
          ClearOneof(message, field->containing_oneof());
          MutableField<ArenaStringPtr>(message, field)->InitDefault();
        }
        MutableField<ArenaStringPtr>(message, field)
            ->Set(std::move(value), message->GetArena());
        break;
      }
    }
  }
}

void Reflection::SetString(Message* message, const FieldDescriptor* field,
                           const absl::Cord& value) const {
  USAGE_MUTABLE_CHECK_ALL(SetString, SINGULAR, STRING);
  if (field->is_extension()) {
    return absl::CopyCordToString(value,
                                  MutableExtensionSet(message)->MutableString(
                                      field->number(), field->type(), field));
  } else {
    switch (internal::cpp::EffectiveStringCType(field)) {
      case FieldOptions::CORD:
        if (schema_.InRealOneof(field)) {
          if (!HasOneofField(*message, field)) {
            ClearOneof(message, field->containing_oneof());
            *MutableField<absl::Cord*>(message, field) =
                Arena::Create<absl::Cord>(message->GetArena());
          }
          *(*MutableField<absl::Cord*>(message, field)) = value;
        } else {
          *MutableField<absl::Cord>(message, field) = value;
        }
        break;
      default:
      case FieldOptions::STRING: {
        // Oneof string fields are never set as a default instance.
        // We just need to pass some arbitrary default string to make it work.
        // This allows us to not have the real default accessible from
        // reflection.
        if (schema_.InRealOneof(field) && !HasOneofField(*message, field)) {
          ClearOneof(message, field->containing_oneof());
          MutableField<ArenaStringPtr>(message, field)->InitDefault();
        }
        if (IsInlined(field)) {
          auto* str = MutableField<InlinedStringField>(message, field);
          const uint32_t index = schema_.InlinedStringIndex(field);
          ABSL_DCHECK_GT(index, 0);
          uint32_t* states =
              &MutableInlinedStringDonatedArray(message)[index / 32];
          uint32_t mask = ~(static_cast<uint32_t>(1) << (index % 32));
          str->Set(std::string(value), message->GetArena(),
                   IsInlinedStringDonated(*message, field), states, mask,
                   message);
        } else {
          auto* str = MutableField<ArenaStringPtr>(message, field);
          str->Set(std::string(value), message->GetArena());
        }
        break;
      }
    }
  }
}

std::string Reflection::GetRepeatedString(const Message& message,
                                          const FieldDescriptor* field,
                                          int index) const {
  USAGE_CHECK_ALL(GetRepeatedString, REPEATED, STRING);
  if (field->is_extension()) {
    return GetExtensionSet(message).GetRepeatedString(field->number(), index);
  } else {
    switch (field->options().ctype()) {
      default:  // TODO:  Support other string reps.
      case FieldOptions::STRING:
        return GetRepeatedPtrField<std::string>(message, field, index);
    }
  }
}

const std::string& Reflection::GetRepeatedStringReference(
    const Message& message, const FieldDescriptor* field, int index,
    std::string* scratch) const {
  (void)scratch;  // Parameter is used by Google-internal code.
  USAGE_CHECK_ALL(GetRepeatedStringReference, REPEATED, STRING);
  if (field->is_extension()) {
    return GetExtensionSet(message).GetRepeatedString(field->number(), index);
  } else {
    switch (field->options().ctype()) {
      default:  // TODO:  Support other string reps.
      case FieldOptions::STRING:
        return GetRepeatedPtrField<std::string>(message, field, index);
    }
  }
}


void Reflection::SetRepeatedString(Message* message,
                                   const FieldDescriptor* field, int index,
                                   std::string value) const {
  USAGE_MUTABLE_CHECK_ALL(SetRepeatedString, REPEATED, STRING);
  if (field->is_extension()) {
    MutableExtensionSet(message)->SetRepeatedString(field->number(), index,
                                                    std::move(value));
  } else {
    switch (field->options().ctype()) {
      default:  // TODO:  Support other string reps.
      case FieldOptions::STRING:
        MutableRepeatedField<std::string>(message, field, index)
            ->assign(std::move(value));
        break;
    }
  }
}


void Reflection::AddString(Message* message, const FieldDescriptor* field,
                           std::string value) const {
  USAGE_MUTABLE_CHECK_ALL(AddString, REPEATED, STRING);
  if (field->is_extension()) {
    MutableExtensionSet(message)->AddString(field->number(), field->type(),
                                            std::move(value), field);
  } else {
    switch (field->options().ctype()) {
      default:  // TODO:  Support other string reps.
      case FieldOptions::STRING:
        AddField<std::string>(message, field)->assign(std::move(value));
        break;
    }
  }
}


// -------------------------------------------------------------------

const EnumValueDescriptor* Reflection::GetEnum(
    const Message& message, const FieldDescriptor* field) const {
  // Usage checked by GetEnumValue.
  int value = GetEnumValue(message, field);
  return field->enum_type()->FindValueByNumberCreatingIfUnknown(value);
}

int Reflection::GetEnumValue(const Message& message,
                             const FieldDescriptor* field) const {
  USAGE_CHECK_ALL(GetEnumValue, SINGULAR, ENUM);

  int32_t value;
  if (field->is_extension()) {
    value = GetExtensionSet(message).GetEnum(
        field->number(), field->default_value_enum()->number());
  } else if (schema_.InRealOneof(field) && !HasOneofField(message, field)) {
    value = field->default_value_enum()->number();
  } else {
    value = GetField<int>(message, field);
  }
  return value;
}

void Reflection::SetEnum(Message* message, const FieldDescriptor* field,
                         const EnumValueDescriptor* value) const {
  // Usage checked by SetEnumValue.
  USAGE_CHECK_ENUM_VALUE(SetEnum);
  SetEnumValueInternal(message, field, value->number());
}

void Reflection::SetEnumValue(Message* message, const FieldDescriptor* field,
                              int value) const {
  USAGE_MUTABLE_CHECK_ALL(SetEnumValue, SINGULAR, ENUM);
  if (!CreateUnknownEnumValues(field)) {
    // Check that the value is valid if we don't support direct storage of
    // unknown enum values.
    const EnumValueDescriptor* value_desc =
        field->enum_type()->FindValueByNumber(value);
    if (value_desc == nullptr) {
      MutableUnknownFields(message)->AddVarint(field->number(), value);
      return;
    }
  }
  SetEnumValueInternal(message, field, value);
}

void Reflection::SetEnumValueInternal(Message* message,
                                      const FieldDescriptor* field,
                                      int value) const {
  if (field->is_extension()) {
    MutableExtensionSet(message)->SetEnum(field->number(), field->type(), value,
                                          field);
  } else {
    SetField<int>(message, field, value);
  }
}

const EnumValueDescriptor* Reflection::GetRepeatedEnum(
    const Message& message, const FieldDescriptor* field, int index) const {
  // Usage checked by GetRepeatedEnumValue.
  int value = GetRepeatedEnumValue(message, field, index);
  return field->enum_type()->FindValueByNumberCreatingIfUnknown(value);
}

int Reflection::GetRepeatedEnumValue(const Message& message,
                                     const FieldDescriptor* field,
                                     int index) const {
  USAGE_CHECK_ALL(GetRepeatedEnumValue, REPEATED, ENUM);

  int value;
  if (field->is_extension()) {
    value = GetExtensionSet(message).GetRepeatedEnum(field->number(), index);
  } else {
    value = GetRepeatedField<int>(message, field, index);
  }
  return value;
}

void Reflection::SetRepeatedEnum(Message* message, const FieldDescriptor* field,
                                 int index,
                                 const EnumValueDescriptor* value) const {
  // Usage checked by SetRepeatedEnumValue.
  USAGE_CHECK_ENUM_VALUE(SetRepeatedEnum);
  SetRepeatedEnumValueInternal(message, field, index, value->number());
}

void Reflection::SetRepeatedEnumValue(Message* message,
                                      const FieldDescriptor* field, int index,
                                      int value) const {
  USAGE_MUTABLE_CHECK_ALL(SetRepeatedEnum, REPEATED, ENUM);
  if (!CreateUnknownEnumValues(field)) {
    // Check that the value is valid if we don't support direct storage of
    // unknown enum values.
    const EnumValueDescriptor* value_desc =
        field->enum_type()->FindValueByNumber(value);
    if (value_desc == nullptr) {
      MutableUnknownFields(message)->AddVarint(field->number(), value);
      return;
    }
  }
  SetRepeatedEnumValueInternal(message, field, index, value);
}

void Reflection::SetRepeatedEnumValueInternal(Message* message,
                                              const FieldDescriptor* field,
                                              int index, int value) const {
  if (field->is_extension()) {
    MutableExtensionSet(message)->SetRepeatedEnum(field->number(), index,
                                                  value);
  } else {
    SetRepeatedField<int>(message, field, index, value);
  }
}

void Reflection::AddEnum(Message* message, const FieldDescriptor* field,
                         const EnumValueDescriptor* value) const {
  // Usage checked by AddEnumValue.
  USAGE_CHECK_ENUM_VALUE(AddEnum);
  AddEnumValueInternal(message, field, value->number());
}

void Reflection::AddEnumValue(Message* message, const FieldDescriptor* field,
                              int value) const {
  USAGE_MUTABLE_CHECK_ALL(AddEnum, REPEATED, ENUM);
  if (!CreateUnknownEnumValues(field)) {
    // Check that the value is valid if we don't support direct storage of
    // unknown enum values.
    const EnumValueDescriptor* value_desc =
        field->enum_type()->FindValueByNumber(value);
    if (value_desc == nullptr) {
      MutableUnknownFields(message)->AddVarint(field->number(), value);
      return;
    }
  }
  AddEnumValueInternal(message, field, value);
}

void Reflection::AddEnumValueInternal(Message* message,
                                      const FieldDescriptor* field,
                                      int value) const {
  if (field->is_extension()) {
    MutableExtensionSet(message)->AddEnum(field->number(), field->type(),
                                          field->options().packed(), value,
                                          field);
  } else {
    AddField<int>(message, field, value);
  }
}

// -------------------------------------------------------------------

const Message* Reflection::GetDefaultMessageInstance(
    const FieldDescriptor* field) const {
  // If we are using the generated factory, we cache the prototype in the field
  // descriptor for faster access.
  // The default instances of generated messages are not cross-linked, which
  // means they contain null pointers on their message fields and can't be used
  // to get the default of submessages.
  if (message_factory_ == MessageFactory::generated_factory()) {
    auto& ptr = field->default_generated_instance_;
    auto* res = ptr.load(std::memory_order_acquire);
    if (res == nullptr) {
      // First time asking for this field's default. Load it and cache it.
      res = message_factory_->GetPrototype(field->message_type());
      ptr.store(res, std::memory_order_release);
    }
    return res;
  }

  // For other factories, we try the default's object field.
  // In particular, the DynamicMessageFactory will cross link the default
  // instances to allow for this. But only do this for real fields.
  // This is an optimization to avoid going to GetPrototype() below, as that
  // requires a lock and a map lookup.
  if (!field->is_extension() && !field->options().weak() &&
      !IsLazyField(field) && !schema_.InRealOneof(field)) {
    auto* res = DefaultRaw<const Message*>(field);
    if (res != nullptr) {
      return res;
    }
  }
  // Otherwise, just go to the factory.
  return message_factory_->GetPrototype(field->message_type());
}

const Message& Reflection::GetMessage(const Message& message,
                                      const FieldDescriptor* field,
                                      MessageFactory* factory) const {
  USAGE_CHECK_ALL(GetMessage, SINGULAR, MESSAGE);

  if (factory == nullptr) factory = message_factory_;

  if (field->is_extension()) {
    return static_cast<const Message&>(GetExtensionSet(message).GetMessage(
        field->number(), field->message_type(), factory));
  } else {
    if (schema_.InRealOneof(field) && !HasOneofField(message, field)) {
      return *GetDefaultMessageInstance(field);
    }
    const Message* result = GetRaw<const Message*>(message, field);
    if (result == nullptr) {
      result = GetDefaultMessageInstance(field);
    }
    return *result;
  }
}

Message* Reflection::MutableMessage(Message* message,
                                    const FieldDescriptor* field,
                                    MessageFactory* factory) const {
  USAGE_MUTABLE_CHECK_ALL(MutableMessage, SINGULAR, MESSAGE);

  if (factory == nullptr) factory = message_factory_;

  if (field->is_extension()) {
    return static_cast<Message*>(
        MutableExtensionSet(message)->MutableMessage(field, factory));
  } else {
    Message* result;

    Message** result_holder = MutableRaw<Message*>(message, field);

    if (schema_.InRealOneof(field)) {
      if (!HasOneofField(*message, field)) {
        ClearOneof(message, field->containing_oneof());
        result_holder = MutableField<Message*>(message, field);
        const Message* default_message = GetDefaultMessageInstance(field);
        *result_holder = default_message->New(message->GetArena());
      }
    } else {
      SetBit(message, field);
    }

    if (*result_holder == nullptr) {
      const Message* default_message = GetDefaultMessageInstance(field);
      *result_holder = default_message->New(message->GetArena());
    }
    result = *result_holder;
    return result;
  }
}

void Reflection::UnsafeArenaSetAllocatedMessage(
    Message* message, Message* sub_message,
    const FieldDescriptor* field) const {
  USAGE_MUTABLE_CHECK_ALL(SetAllocatedMessage, SINGULAR, MESSAGE);


  if (field->is_extension()) {
    MutableExtensionSet(message)->UnsafeArenaSetAllocatedMessage(
        field->number(), field->type(), field, sub_message);
  } else {
    if (schema_.InRealOneof(field)) {
      if (sub_message == nullptr) {
        ClearOneof(message, field->containing_oneof());
        return;
      }
        ClearOneof(message, field->containing_oneof());
        *MutableRaw<Message*>(message, field) = sub_message;
      SetOneofCase(message, field);
      return;
    }

    if (sub_message == nullptr) {
      ClearBit(message, field);
    } else {
      SetBit(message, field);
    }
    Message** sub_message_holder = MutableRaw<Message*>(message, field);
    if (message->GetArena() == nullptr) {
      delete *sub_message_holder;
    }
    *sub_message_holder = sub_message;
  }
}

void Reflection::SetAllocatedMessage(Message* message, Message* sub_message,
                                     const FieldDescriptor* field) const {
  ABSL_DCHECK(sub_message == nullptr || sub_message->GetArena() == nullptr ||
              sub_message->GetArena() == message->GetArena());

  // If message and sub-message are in different memory ownership domains
  // (different arenas, or one is on heap and one is not), then we may need to
  // do a copy.
  if (sub_message != nullptr &&
      sub_message->GetArena() != message->GetArena()) {
    if (sub_message->GetArena() == nullptr && message->GetArena() != nullptr) {
      // Case 1: parent is on an arena and child is heap-allocated. We can add
      // the child to the arena's Own() list to free on arena destruction, then
      // set our pointer.
      message->GetArena()->Own(sub_message);
      UnsafeArenaSetAllocatedMessage(message, sub_message, field);
    } else {
      // Case 2: all other cases. We need to make a copy. MutableMessage() will
      // either get the existing message object, or instantiate a new one as
      // appropriate w.r.t. our arena.
      Message* sub_message_copy = MutableMessage(message, field);
      sub_message_copy->CopyFrom(*sub_message);
    }
  } else {
    // Same memory ownership domains.
    UnsafeArenaSetAllocatedMessage(message, sub_message, field);
  }
}

Message* Reflection::UnsafeArenaReleaseMessage(Message* message,
                                               const FieldDescriptor* field,
                                               MessageFactory* factory) const {
  USAGE_MUTABLE_CHECK_ALL(ReleaseMessage, SINGULAR, MESSAGE);

  if (factory == nullptr) factory = message_factory_;

  if (field->is_extension()) {
    return static_cast<Message*>(
        MutableExtensionSet(message)->UnsafeArenaReleaseMessage(field,
                                                                factory));
  } else {
    if (!(field->is_repeated() || schema_.InRealOneof(field))) {
      ClearBit(message, field);
    }
    if (schema_.InRealOneof(field)) {
      if (HasOneofField(*message, field)) {
        *MutableOneofCase(message, field->containing_oneof()) = 0;
      } else {
        return nullptr;
      }
    }
    Message** result = MutableRaw<Message*>(message, field);
    Message* ret = *result;
    *result = nullptr;
    return ret;
  }
}

Message* Reflection::ReleaseMessage(Message* message,
                                    const FieldDescriptor* field,
                                    MessageFactory* factory) const {
  Message* released = UnsafeArenaReleaseMessage(message, field, factory);
#ifdef PROTOBUF_FORCE_COPY_IN_RELEASE
  released = MaybeForceCopy(message->GetArena(), released);
#endif  // PROTOBUF_FORCE_COPY_IN_RELEASE
  if (message->GetArena() != nullptr && released != nullptr) {
    Message* copy_from_arena = released->New();
    copy_from_arena->CopyFrom(*released);
    released = copy_from_arena;
  }
  return released;
}

const Message& Reflection::GetRepeatedMessage(const Message& message,
                                              const FieldDescriptor* field,
                                              int index) const {
  USAGE_CHECK_ALL(GetRepeatedMessage, REPEATED, MESSAGE);

  if (field->is_extension()) {
    return static_cast<const Message&>(
        GetExtensionSet(message).GetRepeatedMessage(field->number(), index));
  } else {
    if (IsMapFieldInApi(field)) {
      return GetRaw<MapFieldBase>(message, field)
          .GetRepeatedField()
          .Get<GenericTypeHandler<Message> >(index);
    } else {
      return GetRaw<RepeatedPtrFieldBase>(message, field)
          .Get<GenericTypeHandler<Message> >(index);
    }
  }
}

Message* Reflection::MutableRepeatedMessage(Message* message,
                                            const FieldDescriptor* field,
                                            int index) const {
  USAGE_MUTABLE_CHECK_ALL(MutableRepeatedMessage, REPEATED, MESSAGE);

  if (field->is_extension()) {
    return static_cast<Message*>(
        MutableExtensionSet(message)->MutableRepeatedMessage(field->number(),
                                                             index));
  } else {
    if (IsMapFieldInApi(field)) {
      return MutableRaw<MapFieldBase>(message, field)
          ->MutableRepeatedField()
          ->Mutable<GenericTypeHandler<Message> >(index);
    } else {
      return MutableRaw<RepeatedPtrFieldBase>(message, field)
          ->Mutable<GenericTypeHandler<Message> >(index);
    }
  }
}

Message* Reflection::AddMessage(Message* message, const FieldDescriptor* field,
                                MessageFactory* factory) const {
  USAGE_MUTABLE_CHECK_ALL(AddMessage, REPEATED, MESSAGE);

  if (factory == nullptr) factory = message_factory_;

  if (field->is_extension()) {
    return static_cast<Message*>(
        MutableExtensionSet(message)->AddMessage(field, factory));
  } else {
    Message* result = nullptr;

    // We can't use AddField<Message>() because RepeatedPtrFieldBase doesn't
    // know how to allocate one.
    RepeatedPtrFieldBase* repeated = nullptr;
    if (IsMapFieldInApi(field)) {
      repeated =
          MutableRaw<MapFieldBase>(message, field)->MutableRepeatedField();
    } else {
      repeated = MutableRaw<RepeatedPtrFieldBase>(message, field);
    }
    result = repeated->AddFromCleared<GenericTypeHandler<Message> >();
    if (result == nullptr) {
      // We must allocate a new object.
      const Message* prototype;
      if (repeated->size() == 0) {
        prototype = factory->GetPrototype(field->message_type());
      } else {
        prototype = &repeated->Get<GenericTypeHandler<Message> >(0);
      }
      result = prototype->New(message->GetArena());
      // We can guarantee here that repeated and result are either both heap
      // allocated or arena owned. So it is safe to call the unsafe version
      // of AddAllocated.
      repeated->UnsafeArenaAddAllocated<GenericTypeHandler<Message> >(result);
    }

    return result;
  }
}

void Reflection::AddAllocatedMessage(Message* message,
                                     const FieldDescriptor* field,
                                     Message* new_entry) const {
  USAGE_MUTABLE_CHECK_ALL(AddAllocatedMessage, REPEATED, MESSAGE);

  if (field->is_extension()) {
    MutableExtensionSet(message)->AddAllocatedMessage(field, new_entry);
  } else {
    RepeatedPtrFieldBase* repeated = nullptr;
    if (IsMapFieldInApi(field)) {
      repeated =
          MutableRaw<MapFieldBase>(message, field)->MutableRepeatedField();
    } else {
      repeated = MutableRaw<RepeatedPtrFieldBase>(message, field);
    }
    repeated->AddAllocated<GenericTypeHandler<Message> >(new_entry);
  }
}

void Reflection::UnsafeArenaAddAllocatedMessage(Message* message,
                                                const FieldDescriptor* field,
                                                Message* new_entry) const {
  USAGE_MUTABLE_CHECK_ALL(UnsafeArenaAddAllocatedMessage, REPEATED, MESSAGE);

  if (field->is_extension()) {
    MutableExtensionSet(message)->UnsafeArenaAddAllocatedMessage(field,
                                                                 new_entry);
  } else {
    RepeatedPtrFieldBase* repeated = nullptr;
    if (IsMapFieldInApi(field)) {
      repeated =
          MutableRaw<MapFieldBase>(message, field)->MutableRepeatedField();
    } else {
      repeated = MutableRaw<RepeatedPtrFieldBase>(message, field);
    }
    repeated->UnsafeArenaAddAllocated<GenericTypeHandler<Message>>(new_entry);
  }
}

void* Reflection::MutableRawRepeatedField(Message* message,
                                          const FieldDescriptor* field,
                                          FieldDescriptor::CppType cpptype,
                                          int ctype,
                                          const Descriptor* desc) const {
  (void)ctype;  // Parameter is used by Google-internal code.
  USAGE_CHECK_REPEATED("MutableRawRepeatedField");

  if (field->cpp_type() != cpptype &&
      (field->cpp_type() != FieldDescriptor::CPPTYPE_ENUM ||
       cpptype != FieldDescriptor::CPPTYPE_INT32))
    ReportReflectionUsageTypeError(descriptor_, field,
                                   "MutableRawRepeatedField", cpptype);
  if (desc != nullptr)
    ABSL_CHECK_EQ(field->message_type(), desc) << "wrong submessage type";
  if (field->is_extension()) {
    return MutableExtensionSet(message)->MutableRawRepeatedField(
        field->number(), field->type(), field->is_packed(), field);
  } else {
    // Trigger transform for MapField
    if (IsMapFieldInApi(field)) {
      return MutableRawNonOneof<MapFieldBase>(message, field)
          ->MutableRepeatedField();
    }
    return MutableRawNonOneof<void>(message, field);
  }
}

const void* Reflection::GetRawRepeatedField(const Message& message,
                                            const FieldDescriptor* field,
                                            FieldDescriptor::CppType cpptype,
                                            int ctype,
                                            const Descriptor* desc) const {
  USAGE_CHECK_REPEATED("GetRawRepeatedField");
  if (field->cpp_type() != cpptype &&
      (field->cpp_type() != FieldDescriptor::CPPTYPE_ENUM ||
       cpptype != FieldDescriptor::CPPTYPE_INT32))
    ReportReflectionUsageTypeError(descriptor_, field, "GetRawRepeatedField",
                                   cpptype);
  if (ctype >= 0)
    ABSL_CHECK_EQ(field->options().ctype(), ctype) << "subtype mismatch";
  if (desc != nullptr)
    ABSL_CHECK_EQ(field->message_type(), desc) << "wrong submessage type";
  if (field->is_extension()) {
    return GetExtensionSet(message).GetRawRepeatedField(
        field->number(), internal::DefaultRawPtr());
  } else {
    // Trigger transform for MapField
    if (IsMapFieldInApi(field)) {
      return &(GetRawNonOneof<MapFieldBase>(message, field).GetRepeatedField());
    }
    return &GetRawNonOneof<char>(message, field);
  }
}

const FieldDescriptor* Reflection::GetOneofFieldDescriptor(
    const Message& message, const OneofDescriptor* oneof_descriptor) const {
  if (OneofDescriptorLegacy(oneof_descriptor).is_synthetic()) {
    const FieldDescriptor* field = oneof_descriptor->field(0);
    return HasField(message, field) ? field : nullptr;
  }
  uint32_t field_number = GetOneofCase(message, oneof_descriptor);
  if (field_number == 0) {
    return nullptr;
  }
  return descriptor_->FindFieldByNumber(field_number);
}

bool Reflection::ContainsMapKey(const Message& message,
                                const FieldDescriptor* field,
                                const MapKey& key) const {
  USAGE_CHECK(IsMapFieldInApi(field), "LookupMapValue",
              "Field is not a map field.");
  return GetRaw<MapFieldBase>(message, field).ContainsMapKey(key);
}

bool Reflection::InsertOrLookupMapValue(Message* message,
                                        const FieldDescriptor* field,
                                        const MapKey& key,
                                        MapValueRef* val) const {
  USAGE_CHECK(IsMapFieldInApi(field), "InsertOrLookupMapValue",
              "Field is not a map field.");
  val->SetType(field->message_type()->map_value()->cpp_type());
  return MutableRaw<MapFieldBase>(message, field)
      ->InsertOrLookupMapValue(key, val);
}

bool Reflection::LookupMapValue(const Message& message,
                                const FieldDescriptor* field, const MapKey& key,
                                MapValueConstRef* val) const {
  USAGE_CHECK(IsMapFieldInApi(field), "LookupMapValue",
              "Field is not a map field.");
  val->SetType(field->message_type()->map_value()->cpp_type());
  return GetRaw<MapFieldBase>(message, field).LookupMapValue(key, val);
}

bool Reflection::DeleteMapValue(Message* message, const FieldDescriptor* field,
                                const MapKey& key) const {
  USAGE_CHECK(IsMapFieldInApi(field), "DeleteMapValue",
              "Field is not a map field.");
  return MutableRaw<MapFieldBase>(message, field)->DeleteMapValue(key);
}

MapIterator Reflection::MapBegin(Message* message,
                                 const FieldDescriptor* field) const {
  USAGE_CHECK(IsMapFieldInApi(field), "MapBegin", "Field is not a map field.");
  MapIterator iter(message, field);
  GetRaw<MapFieldBase>(*message, field).MapBegin(&iter);
  return iter;
}

MapIterator Reflection::MapEnd(Message* message,
                               const FieldDescriptor* field) const {
  USAGE_CHECK(IsMapFieldInApi(field), "MapEnd", "Field is not a map field.");
  MapIterator iter(message, field);
  GetRaw<MapFieldBase>(*message, field).MapEnd(&iter);
  return iter;
}

int Reflection::MapSize(const Message& message,
                        const FieldDescriptor* field) const {
  USAGE_CHECK(IsMapFieldInApi(field), "MapSize", "Field is not a map field.");
  return GetRaw<MapFieldBase>(message, field).size();
}

// -----------------------------------------------------------------------------

const FieldDescriptor* Reflection::FindKnownExtensionByName(
    absl::string_view name) const {
  if (!schema_.HasExtensionSet()) return nullptr;
  return descriptor_pool_->FindExtensionByPrintableName(descriptor_, name);
}

const FieldDescriptor* Reflection::FindKnownExtensionByNumber(
    int number) const {
  if (!schema_.HasExtensionSet()) return nullptr;
  return descriptor_pool_->FindExtensionByNumber(descriptor_, number);
}

bool Reflection::SupportsUnknownEnumValues() const {
  return FileDescriptorLegacy(descriptor_->file()).syntax() ==
         FileDescriptorLegacy::Syntax::SYNTAX_PROTO3;
}

// ===================================================================
// Some private helpers.

// These simple template accessors obtain pointers (or references) to
// the given field.

template <class Type>
const Type& Reflection::GetRawNonOneof(const Message& message,
                                       const FieldDescriptor* field) const {
  const uint32_t field_offset = schema_.GetFieldOffsetNonOneof(field);
  if (!schema_.IsSplit(field)) {
    return GetConstRefAtOffset<Type>(message, field_offset);
  }
  const void* split = GetSplitField(&message);
  if (SplitFieldHasExtraIndirection(field)) {
    return **GetConstPointerAtOffset<Type*>(split, field_offset);
  }
  return *GetConstPointerAtOffset<Type>(split, field_offset);
}

void Reflection::PrepareSplitMessageForWrite(Message* message) const {
  ABSL_DCHECK_NE(message, schema_.default_instance_);
  void** split = MutableSplitField(message);
  const void* default_split = GetSplitField(schema_.default_instance_);
  if (*split == default_split) {
    uint32_t size = schema_.SizeofSplit();
    Arena* arena = message->GetArena();
    *split = (arena == nullptr) ? ::operator new(size)
                                : arena->AllocateAligned(size);
    memcpy(*split, default_split, size);
  }
}

template <class Type>
static Type* AllocIfDefault(const FieldDescriptor* field, Type*& ptr,
                            Arena* arena) {
  if (ptr == internal::DefaultRawPtr()) {
    // Note: we can't rely on Type to distinguish between these cases (Type can
    // be e.g. char).
    if (field->cpp_type() < FieldDescriptor::CPPTYPE_STRING ||
        (field->cpp_type() == FieldDescriptor::CPPTYPE_STRING &&
         internal::cpp::EffectiveStringCType(field) == FieldOptions::CORD)) {
      ptr = reinterpret_cast<Type*>(
          Arena::CreateMessage<RepeatedField<int32_t>>(arena));
    } else {
      ptr = reinterpret_cast<Type*>(
          Arena::CreateMessage<RepeatedPtrFieldBase>(arena));
    }
  }
  return ptr;
}

template <class Type>
Type* Reflection::MutableRawNonOneof(Message* message,
                                     const FieldDescriptor* field) const {
  const uint32_t field_offset = schema_.GetFieldOffsetNonOneof(field);
  if (!schema_.IsSplit(field)) {
    return GetPointerAtOffset<Type>(message, field_offset);
  }
  PrepareSplitMessageForWrite(message);
  void** split = MutableSplitField(message);
  if (SplitFieldHasExtraIndirection(field)) {
    return AllocIfDefault(field,
                          *GetPointerAtOffset<Type*>(*split, field_offset),
                          message->GetArena());
  }
  return GetPointerAtOffset<Type>(*split, field_offset);
}

template <typename Type>
Type* Reflection::MutableRaw(Message* message,
                             const FieldDescriptor* field) const {
  const uint32_t field_offset = schema_.GetFieldOffset(field);
  if (!schema_.IsSplit(field)) {
    return GetPointerAtOffset<Type>(message, field_offset);
  }
  PrepareSplitMessageForWrite(message);
  void** split = MutableSplitField(message);
  if (SplitFieldHasExtraIndirection(field)) {
    return AllocIfDefault(field,
                          *GetPointerAtOffset<Type*>(*split, field_offset),
                          message->GetArena());
  }
  return GetPointerAtOffset<Type>(*split, field_offset);
}

const uint32_t* Reflection::GetHasBits(const Message& message) const {
  ABSL_DCHECK(schema_.HasHasbits());
  return &GetConstRefAtOffset<uint32_t>(message, schema_.HasBitsOffset());
}

uint32_t* Reflection::MutableHasBits(Message* message) const {
  ABSL_DCHECK(schema_.HasHasbits());
  return GetPointerAtOffset<uint32_t>(message, schema_.HasBitsOffset());
}

uint32_t Reflection::GetOneofCase(
    const Message& message, const OneofDescriptor* oneof_descriptor) const {
  ABSL_DCHECK(!OneofDescriptorLegacy(oneof_descriptor).is_synthetic());
  return internal::GetConstRefAtOffset<uint32_t>(
      message, schema_.GetOneofCaseOffset(oneof_descriptor));
}

uint32_t* Reflection::MutableOneofCase(
    Message* message, const OneofDescriptor* oneof_descriptor) const {
  ABSL_DCHECK(!OneofDescriptorLegacy(oneof_descriptor).is_synthetic());
  return GetPointerAtOffset<uint32_t>(
      message, schema_.GetOneofCaseOffset(oneof_descriptor));
}

const ExtensionSet& Reflection::GetExtensionSet(const Message& message) const {
  return GetConstRefAtOffset<ExtensionSet>(message,
                                           schema_.GetExtensionSetOffset());
}

ExtensionSet* Reflection::MutableExtensionSet(Message* message) const {
  return GetPointerAtOffset<ExtensionSet>(message,
                                          schema_.GetExtensionSetOffset());
}

const InternalMetadata& Reflection::GetInternalMetadata(
    const Message& message) const {
  return GetConstRefAtOffset<InternalMetadata>(message,
                                               schema_.GetMetadataOffset());
}

InternalMetadata* Reflection::MutableInternalMetadata(Message* message) const {
  return GetPointerAtOffset<InternalMetadata>(message,
                                              schema_.GetMetadataOffset());
}

const uint32_t* Reflection::GetInlinedStringDonatedArray(
    const Message& message) const {
  ABSL_DCHECK(schema_.HasInlinedString());
  return &GetConstRefAtOffset<uint32_t>(message,
                                        schema_.InlinedStringDonatedOffset());
}

uint32_t* Reflection::MutableInlinedStringDonatedArray(Message* message) const {
  ABSL_DCHECK(schema_.HasInlinedString());
  return GetPointerAtOffset<uint32_t>(message,
                                      schema_.InlinedStringDonatedOffset());
}

// Simple accessors for manipulating _inlined_string_donated_;
bool Reflection::IsInlinedStringDonated(const Message& message,
                                        const FieldDescriptor* field) const {
  uint32_t index = schema_.InlinedStringIndex(field);
  ABSL_DCHECK_GT(index, 0);
  return IsIndexInHasBitSet(GetInlinedStringDonatedArray(message), index);
}

inline void SetInlinedStringDonated(uint32_t index, uint32_t* array) {
  array[index / 32] |= (static_cast<uint32_t>(1) << (index % 32));
}

inline void ClearInlinedStringDonated(uint32_t index, uint32_t* array) {
  array[index / 32] &= ~(static_cast<uint32_t>(1) << (index % 32));
}

void Reflection::SwapInlinedStringDonated(Message* lhs, Message* rhs,
                                          const FieldDescriptor* field) const {
  Arena* lhs_arena = lhs->GetArena();
  Arena* rhs_arena = rhs->GetArena();
  // If arenas differ, inined string fields are swapped by copying values.
  // Donation status should not be swapped.
  if (lhs_arena != rhs_arena) {
    return;
  }
  bool lhs_donated = IsInlinedStringDonated(*lhs, field);
  bool rhs_donated = IsInlinedStringDonated(*rhs, field);
  if (lhs_donated == rhs_donated) {
    return;
  }
  // If one is undonated, both must have already registered ArenaDtor.
  uint32_t* lhs_array = MutableInlinedStringDonatedArray(lhs);
  uint32_t* rhs_array = MutableInlinedStringDonatedArray(rhs);
  ABSL_CHECK_EQ(lhs_array[0] & 0x1u, 0u);
  ABSL_CHECK_EQ(rhs_array[0] & 0x1u, 0u);
  // Swap donation status bit.
  uint32_t index = schema_.InlinedStringIndex(field);
  ABSL_DCHECK_GT(index, 0);
  if (rhs_donated) {
    SetInlinedStringDonated(index, lhs_array);
    ClearInlinedStringDonated(index, rhs_array);
  } else {  // lhs_donated
    ClearInlinedStringDonated(index, lhs_array);
    SetInlinedStringDonated(index, rhs_array);
  }
}

// Simple accessors for manipulating has_bits_.
bool Reflection::HasBit(const Message& message,
                        const FieldDescriptor* field) const {
  ABSL_DCHECK(!field->options().weak());
  if (schema_.HasBitIndex(field) != static_cast<uint32_t>(-1)) {
    return IsIndexInHasBitSet(GetHasBits(message), schema_.HasBitIndex(field));
  }

  // proto3: no has-bits. All fields present except messages, which are
  // present only if their message-field pointer is non-null.
  if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
    return !schema_.IsDefaultInstance(message) &&
           GetRaw<const Message*>(message, field) != nullptr;
  } else {
    // Non-message field (and non-oneof, since that was handled in HasField()
    // before calling us), and singular (again, checked in HasField). So, this
    // field must be a scalar.

    // Scalar primitive (numeric or string/bytes) fields are present if
    // their value is non-zero (numeric) or non-empty (string/bytes). N.B.:
    // we must use this definition here, rather than the "scalar fields
    // always present" in the proto3 docs, because MergeFrom() semantics
    // require presence as "present on wire", and reflection-based merge
    // (which uses HasField()) needs to be consistent with this.
    switch (field->cpp_type()) {
      case FieldDescriptor::CPPTYPE_STRING:
        switch (internal::cpp::EffectiveStringCType(field)) {
          case FieldOptions::CORD:
            return !GetField<const absl::Cord>(message, field).empty();
          default:
          case FieldOptions::STRING: {
            if (IsInlined(field)) {
              return !GetField<InlinedStringField>(message, field)
                          .GetNoArena()
                          .empty();
            }

            return GetField<ArenaStringPtr>(message, field).Get().size() > 0;
          }
        }
        return false;
      case FieldDescriptor::CPPTYPE_BOOL:
        return GetRaw<bool>(message, field) != false;
      case FieldDescriptor::CPPTYPE_INT32:
        return GetRaw<int32_t>(message, field) != 0;
      case FieldDescriptor::CPPTYPE_INT64:
        return GetRaw<int64_t>(message, field) != 0;
      case FieldDescriptor::CPPTYPE_UINT32:
        return GetRaw<uint32_t>(message, field) != 0;
      case FieldDescriptor::CPPTYPE_UINT64:
        return GetRaw<uint64_t>(message, field) != 0;
      case FieldDescriptor::CPPTYPE_FLOAT:
        static_assert(sizeof(uint32_t) == sizeof(float),
                      "Code assumes uint32_t and float are the same size.");
        return GetRaw<uint32_t>(message, field) != 0;
      case FieldDescriptor::CPPTYPE_DOUBLE:
        static_assert(sizeof(uint64_t) == sizeof(double),
                      "Code assumes uint64_t and double are the same size.");
        return GetRaw<uint64_t>(message, field) != 0;
      case FieldDescriptor::CPPTYPE_ENUM:
        return GetRaw<int>(message, field) != 0;
      case FieldDescriptor::CPPTYPE_MESSAGE:
        // handled above; avoid warning
        break;
    }
    ABSL_LOG(FATAL) << "Reached impossible case in HasBit().";
    return false;
  }
}

void Reflection::SetBit(Message* message, const FieldDescriptor* field) const {
  ABSL_DCHECK(!field->options().weak());
  const uint32_t index = schema_.HasBitIndex(field);
  if (index == static_cast<uint32_t>(-1)) return;
  MutableHasBits(message)[index / 32] |=
      (static_cast<uint32_t>(1) << (index % 32));
}

void Reflection::ClearBit(Message* message,
                          const FieldDescriptor* field) const {
  ABSL_DCHECK(!field->options().weak());
  const uint32_t index = schema_.HasBitIndex(field);
  if (index == static_cast<uint32_t>(-1)) return;
  MutableHasBits(message)[index / 32] &=
      ~(static_cast<uint32_t>(1) << (index % 32));
}

void Reflection::SwapBit(Message* message1, Message* message2,
                         const FieldDescriptor* field) const {
  ABSL_DCHECK(!field->options().weak());
  if (!schema_.HasHasbits()) {
    return;
  }
  bool temp_has_bit = HasBit(*message1, field);
  if (HasBit(*message2, field)) {
    SetBit(message1, field);
  } else {
    ClearBit(message1, field);
  }
  if (temp_has_bit) {
    SetBit(message2, field);
  } else {
    ClearBit(message2, field);
  }
}

bool Reflection::HasOneof(const Message& message,
                          const OneofDescriptor* oneof_descriptor) const {
  if (OneofDescriptorLegacy(oneof_descriptor).is_synthetic()) {
    return HasField(message, oneof_descriptor->field(0));
  }
  return (GetOneofCase(message, oneof_descriptor) > 0);
}

void Reflection::SetOneofCase(Message* message,
                              const FieldDescriptor* field) const {
  *MutableOneofCase(message, field->containing_oneof()) = field->number();
}

void Reflection::ClearOneofField(Message* message,
                                 const FieldDescriptor* field) const {
  if (HasOneofField(*message, field)) {
    ClearOneof(message, field->containing_oneof());
  }
}

void Reflection::ClearOneof(Message* message,
                            const OneofDescriptor* oneof_descriptor) const {
  if (OneofDescriptorLegacy(oneof_descriptor).is_synthetic()) {
    ClearField(message, oneof_descriptor->field(0));
    return;
  }
  // TODO: Consider to cache the unused object instead of deleting
  // it. It will be much faster if an application switches a lot from
  // a few oneof fields.  Time/space tradeoff
  uint32_t oneof_case = GetOneofCase(*message, oneof_descriptor);
  if (oneof_case > 0) {
    const FieldDescriptor* field = descriptor_->FindFieldByNumber(oneof_case);
    if (message->GetArena() == nullptr) {
      switch (field->cpp_type()) {
        case FieldDescriptor::CPPTYPE_STRING: {
          switch (internal::cpp::EffectiveStringCType(field)) {
            case FieldOptions::CORD:
              delete *MutableRaw<absl::Cord*>(message, field);
              break;
            default:
            case FieldOptions::STRING: {
              // Oneof string fields are never set as a default instance.
              // We just need to pass some arbitrary default string to make it
              // work. This allows us to not have the real default accessible
              // from reflection.
              MutableField<ArenaStringPtr>(message, field)->Destroy();
              break;
            }
          }
          break;
        }

        case FieldDescriptor::CPPTYPE_MESSAGE:
          delete *MutableRaw<Message*>(message, field);
          break;
        default:
          break;
      }
    } else {
    }

    *MutableOneofCase(message, oneof_descriptor) = 0;
  }
}

#define HANDLE_TYPE(TYPE, CPPTYPE, CTYPE)                                  \
  template <>                                                              \
  const RepeatedField<TYPE>& Reflection::GetRepeatedFieldInternal<TYPE>(   \
      const Message& message, const FieldDescriptor* field) const {        \
    return *static_cast<const RepeatedField<TYPE>*>(                       \
        GetRawRepeatedField(message, field, CPPTYPE, CTYPE, nullptr));     \
  }                                                                        \
                                                                           \
  template <>                                                              \
  RepeatedField<TYPE>* Reflection::MutableRepeatedFieldInternal<TYPE>(     \
      Message * message, const FieldDescriptor* field) const {             \
    return static_cast<RepeatedField<TYPE>*>(                              \
        MutableRawRepeatedField(message, field, CPPTYPE, CTYPE, nullptr)); \
  }

HANDLE_TYPE(int32_t, FieldDescriptor::CPPTYPE_INT32, -1);
HANDLE_TYPE(int64_t, FieldDescriptor::CPPTYPE_INT64, -1);
HANDLE_TYPE(uint32_t, FieldDescriptor::CPPTYPE_UINT32, -1);
HANDLE_TYPE(uint64_t, FieldDescriptor::CPPTYPE_UINT64, -1);
HANDLE_TYPE(float, FieldDescriptor::CPPTYPE_FLOAT, -1);
HANDLE_TYPE(double, FieldDescriptor::CPPTYPE_DOUBLE, -1);
HANDLE_TYPE(bool, FieldDescriptor::CPPTYPE_BOOL, -1);


#undef HANDLE_TYPE

const void* Reflection::GetRawRepeatedString(const Message& message,
                                             const FieldDescriptor* field,
                                             bool is_string) const {
  (void)is_string;  // Parameter is used by Google-internal code.
  return GetRawRepeatedField(message, field, FieldDescriptor::CPPTYPE_STRING,
                             FieldOptions::STRING, nullptr);
}

void* Reflection::MutableRawRepeatedString(Message* message,
                                           const FieldDescriptor* field,
                                           bool is_string) const {
  (void)is_string;  // Parameter is used by Google-internal code.
  return MutableRawRepeatedField(message, field,
                                 FieldDescriptor::CPPTYPE_STRING,
                                 FieldOptions::STRING, nullptr);
}

// Template implementations of basic accessors.  Inline because each
// template instance is only called from one location.  These are
// used for all types except messages.
template <typename Type>
const Type& Reflection::GetField(const Message& message,
                                 const FieldDescriptor* field) const {
  return GetRaw<Type>(message, field);
}

template <typename Type>
void Reflection::SetField(Message* message, const FieldDescriptor* field,
                          const Type& value) const {
  bool real_oneof = schema_.InRealOneof(field);
  if (real_oneof && !HasOneofField(*message, field)) {
    ClearOneof(message, field->containing_oneof());
  }
  *MutableRaw<Type>(message, field) = value;
  real_oneof ? SetOneofCase(message, field) : SetBit(message, field);
}

template <typename Type>
Type* Reflection::MutableField(Message* message,
                               const FieldDescriptor* field) const {
  schema_.InRealOneof(field) ? SetOneofCase(message, field)
                             : SetBit(message, field);
  return MutableRaw<Type>(message, field);
}

template <typename Type>
const Type& Reflection::GetRepeatedField(const Message& message,
                                         const FieldDescriptor* field,
                                         int index) const {
  return GetRaw<RepeatedField<Type> >(message, field).Get(index);
}

template <typename Type>
const Type& Reflection::GetRepeatedPtrField(const Message& message,
                                            const FieldDescriptor* field,
                                            int index) const {
  return GetRaw<RepeatedPtrField<Type> >(message, field).Get(index);
}

template <typename Type>
void Reflection::SetRepeatedField(Message* message,
                                  const FieldDescriptor* field, int index,
                                  Type value) const {
  MutableRaw<RepeatedField<Type> >(message, field)->Set(index, value);
}

template <typename Type>
Type* Reflection::MutableRepeatedField(Message* message,
                                       const FieldDescriptor* field,
                                       int index) const {
  RepeatedPtrField<Type>* repeated =
      MutableRaw<RepeatedPtrField<Type> >(message, field);
  return repeated->Mutable(index);
}

template <typename Type>
void Reflection::AddField(Message* message, const FieldDescriptor* field,
                          const Type& value) const {
  MutableRaw<RepeatedField<Type> >(message, field)->Add(value);
}

template <typename Type>
Type* Reflection::AddField(Message* message,
                           const FieldDescriptor* field) const {
  RepeatedPtrField<Type>* repeated =
      MutableRaw<RepeatedPtrField<Type> >(message, field);
  return repeated->Add();
}

MessageFactory* Reflection::GetMessageFactory() const {
  return message_factory_;
}

const void* Reflection::RepeatedFieldData(
    const Message& message, const FieldDescriptor* field,
    FieldDescriptor::CppType cpp_type, const Descriptor* message_type) const {
  ABSL_CHECK(field->is_repeated());
  ABSL_CHECK(field->cpp_type() == cpp_type ||
             (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM &&
              cpp_type == FieldDescriptor::CPPTYPE_INT32))
      << "The type parameter T in RepeatedFieldRef<T> API doesn't match "
      << "the actual field type (for enums T should be the generated enum "
      << "type or int32_t).";
  if (message_type != nullptr) {
    ABSL_CHECK_EQ(message_type, field->message_type());
  }
  if (field->is_extension()) {
    return GetExtensionSet(message).GetRawRepeatedField(
        field->number(), internal::DefaultRawPtr());
  } else {
    return &GetRawNonOneof<char>(message, field);
  }
}

void* Reflection::RepeatedFieldData(Message* message,
                                    const FieldDescriptor* field,
                                    FieldDescriptor::CppType cpp_type,
                                    const Descriptor* message_type) const {
  ABSL_CHECK(field->is_repeated());
  ABSL_CHECK(field->cpp_type() == cpp_type ||
             (field->cpp_type() == FieldDescriptor::CPPTYPE_ENUM &&
              cpp_type == FieldDescriptor::CPPTYPE_INT32))
      << "The type parameter T in RepeatedFieldRef<T> API doesn't match "
      << "the actual field type (for enums T should be the generated enum "
      << "type or int32_t).";
  if (message_type != nullptr) {
    ABSL_CHECK_EQ(message_type, field->message_type());
  }
  if (field->is_extension()) {
    return MutableExtensionSet(message)->MutableRawRepeatedField(
        field->number(), field->type(), field->is_packed(), field);
  } else {
    return MutableRawNonOneof<char>(message, field);
  }
}

MapFieldBase* Reflection::MutableMapData(Message* message,
                                         const FieldDescriptor* field) const {
  USAGE_CHECK(IsMapFieldInApi(field), "GetMapData",
              "Field is not a map field.");
  return MutableRaw<MapFieldBase>(message, field);
}

const MapFieldBase* Reflection::GetMapData(const Message& message,
                                           const FieldDescriptor* field) const {
  USAGE_CHECK(IsMapFieldInApi(field), "GetMapData",
              "Field is not a map field.");
  return &(GetRaw<MapFieldBase>(message, field));
}

template <typename T>
static uint32_t AlignTo(uint32_t v) {
  return (v + alignof(T) - 1) & ~(alignof(T) - 1);
}

static internal::TailCallParseFunc GetFastParseFunction(
    internal::TcParseFunction func) {
#define PROTOBUF_TC_PARSE_FUNCTION_X(value) internal::TcParser::value,
  static constexpr internal::TailCallParseFunc kFuncs[] = {
      {}, PROTOBUF_TC_PARSE_FUNCTION_LIST};
#undef PROTOBUF_TC_PARSE_FUNCTION_X
  const int index = static_cast<int>(func);
  if (index < 0 || index >= std::end(kFuncs) - std::begin(kFuncs) ||
      kFuncs[index] == nullptr) {
    ABSL_DLOG(FATAL) << "Failed to find function: " << static_cast<int>(func);
    // Let's not crash in opt, just in case.
    // MiniParse is always a valid parser.
    return &internal::TcParser::MiniParse;
  }
  return kFuncs[index];
}

const internal::TcParseTableBase* Reflection::CreateTcParseTableReflectionOnly()
    const {
  // ParseLoop can't parse message set wire format.
  // Create a dummy table that only exists to make TcParser::ParseLoop jump
  // into the reflective parse loop.

  using Table = internal::TcParseTable<0, 0, 0, 0, 1>;
  // We use `operator new` here because the destruction will be done with
  // `operator delete` unconditionally.
  void* p = ::operator new(sizeof(Table));
  auto* full_table = ::new (p)
      Table{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, schema_.default_instance_, nullptr},
            {{{&internal::TcParser::ReflectionParseLoop, {}}}}};
  ABSL_DCHECK_EQ(static_cast<void*>(&full_table->header),
                 static_cast<void*>(full_table));
  return &full_table->header;
}

void Reflection::PopulateTcParseFastEntries(
    const internal::TailCallTableInfo& table_info,
    TcParseTableBase::FastFieldEntry* fast_entries) const {
  for (const auto& fast_field : table_info.fast_path_fields) {
    if (auto* nonfield = fast_field.AsNonField()) {
      // No field, but still a special entry.
      *fast_entries++ = {GetFastParseFunction(nonfield->func),
                         {nonfield->coded_tag, nonfield->nonfield_info}};
    } else if (auto* as_field = fast_field.AsField()) {
      *fast_entries++ = {
          GetFastParseFunction(as_field->func),
          {as_field->coded_tag, as_field->hasbit_idx, as_field->aux_idx,
           static_cast<uint16_t>(schema_.GetFieldOffset(as_field->field))}};
    } else {
      ABSL_DCHECK(fast_field.is_empty());
      // No fast entry here. Use mini parser.
      *fast_entries++ = {internal::TcParser::MiniParse, {}};
    }
  }
}

static void PopulateTcParseLookupTable(
    const internal::TailCallTableInfo& table_info, uint16_t* lookup_table) {
  for (const auto& entry_block : table_info.num_to_entry_table.blocks) {
    *lookup_table++ = entry_block.first_fnum & 0xFFFF;
    *lookup_table++ = entry_block.first_fnum >> 16;
    *lookup_table++ = entry_block.entries.size();
    for (auto se16 : entry_block.entries) {
      *lookup_table++ = se16.skipmap;
      *lookup_table++ = se16.field_entry_offset;
    }
  }
  *lookup_table++ = 0xFFFF;
  *lookup_table++ = 0xFFFF;
}

void Reflection::PopulateTcParseEntries(
    internal::TailCallTableInfo& table_info,
    TcParseTableBase::FieldEntry* entries) const {
  for (const auto& entry : table_info.field_entries) {
    const FieldDescriptor* field = entry.field;
    ABSL_CHECK(!field->options().weak());
    if (field->type() == field->TYPE_ENUM &&
        table_info.aux_entries[entry.aux_idx].type ==
            internal::TailCallTableInfo::kEnumValidator) {
      // Mini parse can't handle it. Fallback to reflection.
      *entries = {};
      table_info.aux_entries[entry.aux_idx] = {};
    } else {
      const OneofDescriptor* oneof = field->real_containing_oneof();
      entries->offset = schema_.GetFieldOffset(field);
      if (oneof != nullptr) {
        entries->has_idx = schema_.oneof_case_offset_ + 4 * oneof->index();
      } else if (schema_.HasHasbits()) {
        entries->has_idx =
            static_cast<int>(8 * schema_.HasBitsOffset() + entry.hasbit_idx);
      } else {
        entries->has_idx = 0;
      }
      entries->aux_idx = entry.aux_idx;
      entries->type_card = entry.type_card;
    }

    ++entries;
  }
}

void Reflection::PopulateTcParseFieldAux(
    const internal::TailCallTableInfo& table_info,
    TcParseTableBase::FieldAux* field_aux) const {
  for (const auto& aux_entry : table_info.aux_entries) {
    switch (aux_entry.type) {
      case internal::TailCallTableInfo::kNothing:
        *field_aux++ = {};
        break;
      case internal::TailCallTableInfo::kInlinedStringDonatedOffset:
        field_aux++->offset =
            static_cast<uint32_t>(schema_.inlined_string_donated_offset_);
        break;
      case internal::TailCallTableInfo::kSplitOffset:
        field_aux++->offset = schema_.SplitOffset();
        break;
      case internal::TailCallTableInfo::kSplitSizeof:
        field_aux++->offset = schema_.SizeofSplit();
        break;
      case internal::TailCallTableInfo::kSubTable:
      case internal::TailCallTableInfo::kSubMessageWeak:
      case internal::TailCallTableInfo::kCreateInArena:
      case internal::TailCallTableInfo::kMessageVerifyFunc:
        ABSL_LOG(FATAL) << "Not supported";
        break;
      case internal::TailCallTableInfo::kMapAuxInfo:
        // Default constructed info, which causes MpMap to call the fallback.
        // DynamicMessage uses DynamicMapField, which uses variant keys and
        // values. TcParser does not support them yet, so mark the field as
        // unsupported to fallback to reflection.
        field_aux++->map_info = internal::MapAuxInfo{};
        break;
      case internal::TailCallTableInfo::kSubMessage:
        field_aux++->message_default_p =
            GetDefaultMessageInstance(aux_entry.field);
        break;
      case internal::TailCallTableInfo::kEnumRange:
        field_aux++->enum_range = {aux_entry.enum_range.start,
                                   aux_entry.enum_range.size};
        break;
      case internal::TailCallTableInfo::kEnumValidator:
        ABSL_LOG(FATAL) << "Not supported.";
        break;
      case internal::TailCallTableInfo::kNumericOffset:
        field_aux++->offset = aux_entry.offset;
        break;
    }
  }
}

const internal::TcParseTableBase* Reflection::CreateTcParseTable() const {
  using TcParseTableBase = internal::TcParseTableBase;

  if (descriptor_->options().message_set_wire_format()) {
    return CreateTcParseTableReflectionOnly();
  }

  for (int i = 0; i < descriptor_->field_count(); ++i) {
    if (descriptor_->field(i)->options().weak()) {
      return CreateTcParseTableReflectionOnly();
    }
  }

  std::vector<const FieldDescriptor*> fields;
  constexpr int kNoHasbit = -1;
  std::vector<int> has_bit_indices(
      static_cast<size_t>(descriptor_->field_count()), kNoHasbit);
  std::vector<int> inlined_string_indices = has_bit_indices;
  for (int i = 0; i < descriptor_->field_count(); ++i) {
    auto* field = descriptor_->field(i);
    fields.push_back(field);
    has_bit_indices[static_cast<size_t>(field->index())] =
        static_cast<int>(schema_.HasBitIndex(field));

    if (IsInlined(field)) {
      inlined_string_indices[static_cast<size_t>(field->index())] =
          schema_.InlinedStringIndex(field);
    }
  }
  std::sort(fields.begin(), fields.end(),
            [](const FieldDescriptor* a, const FieldDescriptor* b) {
              return a->number() < b->number();
            });

  class ReflectionOptionProvider final
      : public internal::TailCallTableInfo::OptionProvider {
   public:
    explicit ReflectionOptionProvider(const Reflection& ref) : ref_(ref) {}
    internal::TailCallTableInfo::PerFieldOptions GetForField(
        const FieldDescriptor* field) const final {
      const auto verify_flag = [&] {
        if (ref_.IsEagerlyVerifiedLazyField(field))
          return internal::field_layout::kTvEager;
        if (ref_.IsLazilyVerifiedLazyField(field))
          return internal::field_layout::kTvLazy;
        return internal::field_layout::TransformValidation{};
      };
      return {
          1.f,                    // All fields are assumed present.
          verify_flag(),          //
          ref_.IsInlined(field),  //

          // Only LITE can be implicitly weak.
          /* is_implicitly_weak */ false,

          // We could change this to use direct table.
          // Might be easier to do when all messages support TDP.
          /* use_direct_tcparser_table */ false,

          ref_.schema_.IsSplit(field),  //
      };
    }

   private:
    const Reflection& ref_;
  };
  internal::TailCallTableInfo table_info(
      descriptor_, fields,
      {
          /* is_lite */ false,
          /* uses_codegen */ false,
      },
      ReflectionOptionProvider(*this), has_bit_indices, inlined_string_indices);

  const size_t fast_entries_count = table_info.fast_path_fields.size();
  ABSL_CHECK_EQ(static_cast<int>(fast_entries_count),
                1 << table_info.table_size_log2);
  const uint16_t lookup_table_offset = AlignTo<uint16_t>(
      sizeof(TcParseTableBase) +
      fast_entries_count * sizeof(TcParseTableBase::FastFieldEntry));
  const uint32_t field_entry_offset = AlignTo<TcParseTableBase::FieldEntry>(
      lookup_table_offset +
      sizeof(uint16_t) * table_info.num_to_entry_table.size16());
  const uint32_t aux_offset = AlignTo<TcParseTableBase::FieldAux>(
      field_entry_offset +
      sizeof(TcParseTableBase::FieldEntry) * fields.size());

  int byte_size =
      aux_offset +
      sizeof(TcParseTableBase::FieldAux) * table_info.aux_entries.size() +
      sizeof(char) * table_info.field_name_data.size();

  void* p = ::operator new(byte_size);
  auto* res = ::new (p) TcParseTableBase{
      static_cast<uint16_t>(schema_.HasHasbits() ? schema_.HasBitsOffset() : 0),
      schema_.HasExtensionSet()
          ? static_cast<uint16_t>(schema_.GetExtensionSetOffset())
          : uint16_t{0},
      static_cast<uint32_t>(fields.empty() ? 0 : fields.back()->number()),
      static_cast<uint8_t>((fast_entries_count - 1) << 3),
      lookup_table_offset,
      table_info.num_to_entry_table.skipmap32,
      field_entry_offset,
      static_cast<uint16_t>(fields.size()),
      static_cast<uint16_t>(table_info.aux_entries.size()),
      aux_offset,
      schema_.default_instance_,
      &internal::TcParser::ReflectionFallback};

  // Now copy the rest of the payloads
  PopulateTcParseFastEntries(table_info, res->fast_entry(0));

  PopulateTcParseLookupTable(table_info, res->field_lookup_begin());

  PopulateTcParseEntries(table_info, res->field_entries_begin());

  PopulateTcParseFieldAux(table_info, res->field_aux(0u));

  // Copy the name data.
  if (!table_info.field_name_data.empty()) {
    memcpy(res->name_data(), table_info.field_name_data.data(),
           table_info.field_name_data.size());
  }
  // Validation to make sure we used all the bytes correctly.
  ABSL_CHECK_EQ(res->name_data() + table_info.field_name_data.size() -
                    reinterpret_cast<char*>(res),
                byte_size);

  return res;
}

namespace {

// Helper function to transform migration schema into reflection schema.
ReflectionSchema MigrationToReflectionSchema(
    const Message* const* default_instance, const uint32_t* offsets,
    MigrationSchema migration_schema) {
  ReflectionSchema result;
  result.default_instance_ = *default_instance;
  // First 9 offsets are offsets to the special fields. The following offsets
  // are the proto fields.
  //
  // TODO: Find a way to not encode sizeof_split_ in offsets.
  result.offsets_ = offsets + migration_schema.offsets_index + 8;
  result.has_bit_indices_ = offsets + migration_schema.has_bit_indices_index;
  result.has_bits_offset_ = offsets[migration_schema.offsets_index + 0];
  result.metadata_offset_ = offsets[migration_schema.offsets_index + 1];
  result.extensions_offset_ = offsets[migration_schema.offsets_index + 2];
  result.oneof_case_offset_ = offsets[migration_schema.offsets_index + 3];
  result.object_size_ = migration_schema.object_size;
  result.weak_field_map_offset_ = offsets[migration_schema.offsets_index + 4];
  result.inlined_string_donated_offset_ =
      offsets[migration_schema.offsets_index + 5];
  result.split_offset_ = offsets[migration_schema.offsets_index + 6];
  result.sizeof_split_ = offsets[migration_schema.offsets_index + 7];
  result.inlined_string_indices_ =
      offsets + migration_schema.inlined_string_indices_index;
  return result;
}

}  // namespace

class AssignDescriptorsHelper {
 public:
  AssignDescriptorsHelper(MessageFactory* factory,
                          Metadata* file_level_metadata,
                          const EnumDescriptor** file_level_enum_descriptors,
                          const MigrationSchema* schemas,
                          const Message* const* default_instance_data,
                          const uint32_t* offsets)
      : factory_(factory),
        file_level_metadata_(file_level_metadata),
        file_level_enum_descriptors_(file_level_enum_descriptors),
        schemas_(schemas),
        default_instance_data_(default_instance_data),
        offsets_(offsets) {}

  void AssignMessageDescriptor(const Descriptor* descriptor) {
    for (int i = 0; i < descriptor->nested_type_count(); i++) {
      AssignMessageDescriptor(descriptor->nested_type(i));
    }

    file_level_metadata_->descriptor = descriptor;

    file_level_metadata_->reflection =
        new Reflection(descriptor,
                       MigrationToReflectionSchema(default_instance_data_,
                                                   offsets_, *schemas_),
                       DescriptorPool::internal_generated_pool(), factory_);
    for (int i = 0; i < descriptor->enum_type_count(); i++) {
      AssignEnumDescriptor(descriptor->enum_type(i));
    }
    schemas_++;
    default_instance_data_++;
    file_level_metadata_++;
  }

  void AssignEnumDescriptor(const EnumDescriptor* descriptor) {
    *file_level_enum_descriptors_ = descriptor;
    file_level_enum_descriptors_++;
  }

  const Metadata* GetCurrentMetadataPtr() const { return file_level_metadata_; }

 private:
  MessageFactory* factory_;
  Metadata* file_level_metadata_;
  const EnumDescriptor** file_level_enum_descriptors_;
  const MigrationSchema* schemas_;
  const Message* const* default_instance_data_;
  const uint32_t* offsets_;
};

namespace {

// We have the routines that assign descriptors and build reflection
// automatically delete the allocated reflection. MetadataOwner owns
// all the allocated reflection instances.
struct MetadataOwner {
  ~MetadataOwner() {
    for (auto range : metadata_arrays_) {
      for (const Metadata* m = range.first; m < range.second; m++) {
        delete m->reflection;
      }
    }
  }

  void AddArray(const Metadata* begin, const Metadata* end) {
    mu_.Lock();
    metadata_arrays_.push_back(std::make_pair(begin, end));
    mu_.Unlock();
  }

  static MetadataOwner* Instance() {
    static MetadataOwner* res = OnShutdownDelete(new MetadataOwner);
    return res;
  }

 private:
  MetadataOwner() = default;  // private because singleton

  absl::Mutex mu_;
  std::vector<std::pair<const Metadata*, const Metadata*> > metadata_arrays_;
};

void AddDescriptors(const DescriptorTable* table);

void AssignDescriptorsImpl(const DescriptorTable* table, bool eager) {
  // Ensure the file descriptor is added to the pool.
  {
    // This only happens once per proto file. So a global mutex to serialize
    // calls to AddDescriptors.
    static absl::Mutex mu{absl::kConstInit};
    mu.Lock();
    AddDescriptors(table);
    mu.Unlock();
  }
  if (eager) {
    // Normally we do not want to eagerly build descriptors of our deps.
    // However if this proto is optimized for code size (ie using reflection)
    // and it has a message extending a custom option of a descriptor with that
    // message being optimized for code size as well. Building the descriptors
    // in this file requires parsing the serialized file descriptor, which now
    // requires parsing the message extension, which potentially requires
    // building the descriptor of the message extending one of the options.
    // However we are already updating descriptor pool under a lock. To prevent
    // this the compiler statically looks for this case and we just make sure we
    // first build the descriptors of all our dependencies, preventing the
    // deadlock.
    int num_deps = table->num_deps;
    for (int i = 0; i < num_deps; i++) {
      // In case of weak fields deps[i] could be null.
      if (table->deps[i]) {
        absl::call_once(*table->deps[i]->once, AssignDescriptorsImpl,
                        table->deps[i],
                        /*eager=*/true);
      }
    }
  }

  // Fill the arrays with pointers to descriptors and reflection classes.
  const FileDescriptor* file =
      DescriptorPool::internal_generated_pool()->FindFileByName(
          table->filename);
  ABSL_CHECK(file != nullptr);

  MessageFactory* factory = MessageFactory::generated_factory();

  AssignDescriptorsHelper helper(
      factory, table->file_level_metadata, table->file_level_enum_descriptors,
      table->schemas, table->default_instances, table->offsets);

  for (int i = 0; i < file->message_type_count(); i++) {
    helper.AssignMessageDescriptor(file->message_type(i));
  }

  for (int i = 0; i < file->enum_type_count(); i++) {
    helper.AssignEnumDescriptor(file->enum_type(i));
  }
  if (file->options().cc_generic_services()) {
    for (int i = 0; i < file->service_count(); i++) {
      table->file_level_service_descriptors[i] = file->service(i);
    }
  }
  MetadataOwner::Instance()->AddArray(table->file_level_metadata,
                                      helper.GetCurrentMetadataPtr());
}

void MaybeInitializeLazyDescriptors(const DescriptorTable* table) {
  if (!IsLazilyInitializedFile(table->filename)) {
    // Ensure the generated pool has been lazily initialized.
    DescriptorPool::generated_pool();
  }
}

void AddDescriptorsImpl(const DescriptorTable* table) {
  // Reflection refers to the default fields so make sure they are initialized.
  internal::InitProtobufDefaults();
  internal::InitializeFileDescriptorDefaultInstances();

  // Ensure all dependent descriptors are registered to the generated descriptor
  // pool and message factory.
  int num_deps = table->num_deps;
  for (int i = 0; i < num_deps; i++) {
    // In case of weak fields deps[i] could be null.
    if (table->deps[i]) AddDescriptors(table->deps[i]);
  }

  // Register the descriptor of this file.
  DescriptorPool::InternalAddGeneratedFile(table->descriptor, table->size);
  MessageFactory::InternalRegisterGeneratedFile(table);
}

void AddDescriptors(const DescriptorTable* table) {
  // AddDescriptors is not thread safe. Callers need to ensure calls are
  // properly serialized. This function is only called pre-main by global
  // descriptors and we can assume single threaded access or it's called
  // by AssignDescriptorImpl which uses a mutex to sequence calls.
  if (table->is_initialized) return;
  table->is_initialized = true;
  AddDescriptorsImpl(table);
}

}  // namespace

// Separate function because it needs to be a friend of
// Reflection
void RegisterAllTypesInternal(const Metadata* file_level_metadata, int size) {
  for (int i = 0; i < size; i++) {
    const Reflection* reflection = file_level_metadata[i].reflection;
    MessageFactory::InternalRegisterGeneratedMessage(
        file_level_metadata[i].descriptor,
        reflection->schema_.default_instance_);
  }
}

namespace internal {

Metadata AssignDescriptors(const DescriptorTable* (*table)(),
                           absl::once_flag* once, const Metadata& metadata) {
  absl::call_once(*once, [=] {
    auto* t = table();
    MaybeInitializeLazyDescriptors(t);
    AssignDescriptorsImpl(t, t->is_eager);
  });

  return metadata;
}

void AssignDescriptors(const DescriptorTable* table) {
  MaybeInitializeLazyDescriptors(table);
  absl::call_once(*table->once, AssignDescriptorsImpl, table, table->is_eager);
}

AddDescriptorsRunner::AddDescriptorsRunner(const DescriptorTable* table) {
  AddDescriptors(table);
}

void RegisterFileLevelMetadata(const DescriptorTable* table) {
  AssignDescriptors(table);
  RegisterAllTypesInternal(table->file_level_metadata, table->num_messages);
}

void UnknownFieldSetSerializer(const uint8_t* base, uint32_t offset,
                               uint32_t /*tag*/, uint32_t /*has_offset*/,
                               io::CodedOutputStream* output) {
  const void* ptr = base + offset;
  const InternalMetadata* metadata = static_cast<const InternalMetadata*>(ptr);
  if (metadata->have_unknown_fields()) {
    metadata->unknown_fields<UnknownFieldSet>(UnknownFieldSet::default_instance)
        .SerializeToCodedStream(output);
  }
}

bool IsDescendant(Message& root, const Message& message) {
  const Reflection* reflection = root.GetReflection();
  std::vector<const FieldDescriptor*> fields;
  reflection->ListFields(root, &fields);

  for (const auto* field : fields) {
    // Skip non-message fields.
    if (field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) continue;

    // Optional messages.
    if (!field->is_repeated()) {
      Message* sub_message = reflection->MutableMessage(&root, field);
      if (sub_message == &message || IsDescendant(*sub_message, message)) {
        return true;
      }
      continue;
    }

    // Repeated messages.
    if (!IsMapFieldInApi(field)) {
      int count = reflection->FieldSize(root, field);
      for (int i = 0; i < count; i++) {
        Message* sub_message =
            reflection->MutableRepeatedMessage(&root, field, i);
        if (sub_message == &message || IsDescendant(*sub_message, message)) {
          return true;
        }
      }
      continue;
    }

    // Map field: if accessed as repeated fields, messages are *copied* and
    // matching pointer won't work. Must directly access map.
    constexpr int kValIdx = 1;
    const FieldDescriptor* val_field = field->message_type()->field(kValIdx);
    // Skip map fields whose value type is not message.
    if (val_field->cpp_type() != FieldDescriptor::CPPTYPE_MESSAGE) continue;

    MapIterator end = reflection->MapEnd(&root, field);
    for (auto iter = reflection->MapBegin(&root, field); iter != end; ++iter) {
      Message* sub_message = iter.MutableValueRef()->MutableMessageValue();
      if (sub_message == &message || IsDescendant(*sub_message, message)) {
        return true;
      }
    }
  }

  return false;
}

bool SplitFieldHasExtraIndirection(const FieldDescriptor* field) {
  return field->is_repeated();
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


