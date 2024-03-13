// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google_protobuf_json_internal_parser.hpp"

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <utility>

#include "google_protobuf_type.pb.hpp"
#include <CppAbseil/absl_base_attributes.hpp>
#include <CppAbseil/absl_container_flat_hash_set.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_status_status.hpp>
#include <CppAbseil/absl_status_statusor.hpp>
#include <CppAbseil/absl_strings_ascii.hpp>
#include <CppAbseil/absl_strings_escaping.hpp>
#include <CppAbseil/absl_strings_match.hpp>
#include <CppAbseil/absl_strings_numbers.hpp>
#include <CppAbseil/absl_strings_str_format.hpp>
#include <CppAbseil/absl_strings_str_split.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include <CppAbseil/absl_types_optional.hpp>
#include <CppAbseil/absl_types_span.hpp>
#include "google_protobuf_descriptor.hpp"
#include "google_protobuf_dynamic_message.hpp"
#include "google_protobuf_io_zero_copy_sink.hpp"
#include "google_protobuf_io_zero_copy_stream.hpp"
#include "google_protobuf_io_zero_copy_stream_impl_lite.hpp"
#include "google_protobuf_json_internal_descriptor_traits.hpp"
#include "google_protobuf_json_internal_lexer.hpp"
#include "google_protobuf_json_internal_parser_traits.hpp"
#include "google_protobuf_message.hpp"
#include "google_protobuf_util_type_resolver.hpp"
#include "google_protobuf_stubs_status_macros.hpp"

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
namespace json_internal {
namespace {
// This file contains code that drives a JsonLexer to visit a JSON document and
// convert it into some form of proto.
//
// This semantic layer is duplicated: proto2-ish code can deserialize directly
// into a message, whereas proto3-ish code deserializes into a byte stream,
// using TypeResolvers instead of Descriptors.
//
// The parsing code is templated over which of these two reflection + output
// combinations is used. The traits types that collect the per-instantiation
// functionality can be found in json_util2_parser_traits-inl.h.

// This table maps an unsigned `char` value, interpreted as an ASCII character,
// to a corresponding value in the base64 alphabet (both traditional and
// "web-safe" characters are included).
//
// If a character is not valid base64, it maps to -1; this is used by the bit
// operations that assemble a base64-encoded word to determine if an error
// occurred, by checking the sign bit.
constexpr signed char kBase64Table[256] = {
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       62 /*+*/, -1,       62 /*-*/, -1,       63 /*/ */, 52 /*0*/,
    53 /*1*/, 54 /*2*/, 55 /*3*/, 56 /*4*/, 57 /*5*/, 58 /*6*/,  59 /*7*/,
    60 /*8*/, 61 /*9*/, -1,       -1,       -1,       -1,        -1,
    -1,       -1,       0 /*A*/,  1 /*B*/,  2 /*C*/,  3 /*D*/,   4 /*E*/,
    5 /*F*/,  6 /*G*/,  07 /*H*/, 8 /*I*/,  9 /*J*/,  10 /*K*/,  11 /*L*/,
    12 /*M*/, 13 /*N*/, 14 /*O*/, 15 /*P*/, 16 /*Q*/, 17 /*R*/,  18 /*S*/,
    19 /*T*/, 20 /*U*/, 21 /*V*/, 22 /*W*/, 23 /*X*/, 24 /*Y*/,  25 /*Z*/,
    -1,       -1,       -1,       -1,       63 /*_*/, -1,        26 /*a*/,
    27 /*b*/, 28 /*c*/, 29 /*d*/, 30 /*e*/, 31 /*f*/, 32 /*g*/,  33 /*h*/,
    34 /*i*/, 35 /*j*/, 36 /*k*/, 37 /*l*/, 38 /*m*/, 39 /*n*/,  40 /*o*/,
    41 /*p*/, 42 /*q*/, 43 /*r*/, 44 /*s*/, 45 /*t*/, 46 /*u*/,  47 /*v*/,
    48 /*w*/, 49 /*x*/, 50 /*y*/, 51 /*z*/, -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1,       -1,       -1,        -1,
    -1,       -1,       -1,       -1};

uint32_t Base64Lookup(char c) {
  // Sign-extend return value so high bit will be set on any unexpected char.
  return static_cast<uint32_t>(kBase64Table[static_cast<uint8_t>(c)]);
}

// Decodes `base64` in-place, shrinking the length as appropriate.
absl::StatusOr<absl::Span<char>> DecodeBase64InPlace(absl::Span<char> base64) {
  // We decode in place. This is safe because this is a new buffer (not
  // aliasing the input) and because base64 decoding shrinks 4 bytes into 3.
  char* out = base64.data();
  const char* ptr = base64.data();
  const char* end = ptr + base64.size();
  const char* end4 = ptr + (base64.size() & ~3u);

  for (; ptr < end4; ptr += 4, out += 3) {
    auto val = Base64Lookup(ptr[0]) << 18 | Base64Lookup(ptr[1]) << 12 |
               Base64Lookup(ptr[2]) << 6 | Base64Lookup(ptr[3]) << 0;

    if (static_cast<int32_t>(val) < 0) {
      // Junk chars or padding. Remove trailing padding, if any.
      if (end - ptr == 4 && ptr[3] == '=') {
        if (ptr[2] == '=') {
          end -= 2;
        } else {
          end -= 1;
        }
      }
      break;
    }

    out[0] = val >> 16;
    out[1] = (val >> 8) & 0xff;
    out[2] = val & 0xff;
  }

  if (ptr < end) {
    uint32_t val = ~0u;
    switch (end - ptr) {
      case 2:
        val = Base64Lookup(ptr[0]) << 18 | Base64Lookup(ptr[1]) << 12;
        out[0] = val >> 16;
        out += 1;
        break;
      case 3:
        val = Base64Lookup(ptr[0]) << 18 | Base64Lookup(ptr[1]) << 12 |
              Base64Lookup(ptr[2]) << 6;
        out[0] = val >> 16;
        out[1] = (val >> 8) & 0xff;
        out += 2;
        break;
    }

    if (static_cast<int32_t>(val) < 0) {
      return absl::InvalidArgumentError("corrupt base64");
    }
  }

  return absl::Span<char>(base64.data(),
                          static_cast<size_t>(out - base64.data()));
}

template <typename T>
absl::StatusOr<LocationWith<T>> ParseIntInner(JsonLexer& lex, double lo,
                                              double hi) {
  absl::StatusOr<JsonLexer::Kind> kind = lex.PeekKind();
  RETURN_IF_ERROR(kind.status());

  LocationWith<T> n;
  switch (*kind) {
    case JsonLexer::kNum: {
      absl::StatusOr<LocationWith<MaybeOwnedString>> x = lex.ParseRawNumber();
      RETURN_IF_ERROR(x.status());
      n.loc = x->loc;
      if (absl::SimpleAtoi(x->value.AsView(), &n.value)) {
        break;
      }

      double d;
      if (!absl::SimpleAtod(x->value.AsView(), &d) || !std::isfinite(d)) {
        return x->loc.Invalid(
            absl::StrFormat("invalid number: '%s'", x->value.AsView()));
      }

      // Conversion overflow here would be UB.
      if (lo > d || d > hi) {
        return lex.Invalid("JSON number out of range for int");
      }
      n.value = static_cast<T>(d);
      if (d - static_cast<double>(n.value) != 0) {
        return lex.Invalid(
            "expected integer, but JSON number had fractional part");
      }
      break;
    }
    case JsonLexer::kStr: {
      absl::StatusOr<LocationWith<MaybeOwnedString>> str = lex.ParseUtf8();
      RETURN_IF_ERROR(str.status());
      // SimpleAtoi will ignore leading and trailing whitespace, so we need
      // to check for it ourselves.
      for (char c : str->value.AsView()) {
        if (absl::ascii_isspace(c)) {
          return lex.Invalid("non-number characters in quoted number");
        }
      }
      if (!absl::SimpleAtoi(str->value.AsView(), &n.value)) {
        return str->loc.Invalid("non-number characters in quoted number");
      }
      n.loc = str->loc;
      break;
    }
    default:
      return lex.Invalid("expected number or string");
  }

  return n;
}

template <typename Traits>
absl::StatusOr<int64_t> ParseInt(JsonLexer& lex, Field<Traits> field) {
  absl::StatusOr<LocationWith<int64_t>> n =
      ParseIntInner<int64_t>(lex, -9007199254740992.0, 9007199254740992.0);
  RETURN_IF_ERROR(n.status());

  if (Traits::Is32Bit(field)) {
    if (std::numeric_limits<int32_t>::min() > n->value ||
        n->value > std::numeric_limits<int32_t>::max()) {
      return n->loc.Invalid("integer out of range");
    }
  }

  return n->value;
}

template <typename Traits>
absl::StatusOr<uint64_t> ParseUInt(JsonLexer& lex, Field<Traits> field) {
  absl::StatusOr<LocationWith<uint64_t>> n =
      ParseIntInner<uint64_t>(lex, 0, 18014398509481984.0);
  RETURN_IF_ERROR(n.status());

  if (Traits::Is32Bit(field)) {
    if (n->value > std::numeric_limits<uint32_t>::max()) {
      return n->loc.Invalid("integer out of range");
    }
  }

  return n->value;
}

template <typename Traits>
absl::StatusOr<double> ParseFp(JsonLexer& lex, Field<Traits> field) {
  absl::StatusOr<JsonLexer::Kind> kind = lex.PeekKind();
  RETURN_IF_ERROR(kind.status());

  double n;
  switch (*kind) {
    case JsonLexer::kNum: {
      absl::StatusOr<LocationWith<double>> d = lex.ParseNumber();
      RETURN_IF_ERROR(d.status());
      n = d->value;
      break;
    }
    case JsonLexer::kStr: {
      absl::StatusOr<LocationWith<MaybeOwnedString>> str = lex.ParseUtf8();
      RETURN_IF_ERROR(str.status());

      if (str->value == "NaN") {
        n = NAN;
      } else if (str->value == "Infinity") {
        n = INFINITY;
      } else if (str->value == "-Infinity") {
        n = -INFINITY;
      } else if (!absl::SimpleAtod(str->value.AsView(), &n)) {
        return str->loc.Invalid("non-number characters in quoted number");
      }
      break;
    }
    default:
      return lex.Invalid("expected number or string");
  }

  if (Traits::Is32Bit(field)) {
    // Detect out-of-range 32-bit floats by seeing whether the conversion result
    // is still finite. Finite extreme values may have textual representations
    // that parse to 64-bit values outside the 32-bit range, but which are
    // closer to the 32-bit extreme than to the "next value with the same
    // precision".
    if (std::isfinite(n) && !std::isfinite(static_cast<float>(n))) {
      return lex.Invalid("float out of range");
    }
  }

  return n;
}

template <typename Traits>
absl::StatusOr<std::string> ParseStrOrBytes(JsonLexer& lex,
                                            Field<Traits> field) {
  absl::StatusOr<LocationWith<MaybeOwnedString>> str = lex.ParseUtf8();
  RETURN_IF_ERROR(str.status());

  if (Traits::FieldType(field) == FieldDescriptor::TYPE_BYTES) {
    std::string& b64 = str->value.ToString();
    absl::StatusOr<absl::Span<char>> decoded =
        DecodeBase64InPlace(absl::MakeSpan(&b64[0], b64.size()));
    if (!decoded.ok()) {
      return str->loc.Invalid(decoded.status().message());
    }
    b64.resize(decoded->size());
  }

  return std::move(str->value.ToString());
}

template <typename Traits>
absl::StatusOr<std::optional<int32_t>> ParseEnumFromStr(JsonLexer& lex,
                                                         MaybeOwnedString& str,
                                                         Field<Traits> field) {
  absl::StatusOr<int32_t> value = Traits::EnumNumberByName(
      field, str.AsView(), lex.options().case_insensitive_enum_parsing);
  if (value.ok()) {
    return std::optional<int32_t>(*value);
  }

  int32_t i;
  if (absl::SimpleAtoi(str.AsView(), &i)) {
    return std::optional<int32_t>(i);
  } else if (lex.options().ignore_unknown_fields) {
    return {absl::nullopt};
  }

  return value.status();
}

// Parses an enum; can return nullopt if a quoted enumerator that we don't
// know about is received and `ignore_unknown_fields` is set.
template <typename Traits>
absl::StatusOr<std::optional<int32_t>> ParseEnum(JsonLexer& lex,
                                                  Field<Traits> field) {
  absl::StatusOr<JsonLexer::Kind> kind = lex.PeekKind();
  RETURN_IF_ERROR(kind.status());

  int32_t n = 0;
  switch (*kind) {
    case JsonLexer::kStr: {
      absl::StatusOr<LocationWith<MaybeOwnedString>> str = lex.ParseUtf8();
      RETURN_IF_ERROR(str.status());

      auto e = ParseEnumFromStr<Traits>(lex, str->value, field);
      RETURN_IF_ERROR(e.status());
      if (!e->has_value()) {
        return {absl::nullopt};
      }
      n = **e;
      break;
    }
    case JsonLexer::kNum:
      return ParseInt<Traits>(lex, field);
    default:
      return lex.Invalid("expected number or string");
  }

  return n;
}

// Mutually recursive with functions that follow.
template <typename Traits>
absl::Status ParseMessage(JsonLexer& lex, const Desc<Traits>& desc,
                          Msg<Traits>& msg, bool any_reparse);
template <typename Traits>
absl::Status ParseField(JsonLexer& lex, const Desc<Traits>& desc,
                        absl::string_view name, Msg<Traits>& msg);

template <typename Traits>
absl::Status ParseSingular(JsonLexer& lex, Field<Traits> field,
                           Msg<Traits>& msg) {
  auto field_type = Traits::FieldType(field);
  if (lex.Peek(JsonLexer::kNull)) {
    auto message_type = ClassifyMessage(Traits::FieldTypeName(field));
    switch (field_type) {
      case FieldDescriptor::TYPE_ENUM:
        if (message_type == MessageType::kNull) {
          Traits::SetEnum(field, msg, 0);
        }
        break;
      case FieldDescriptor::TYPE_MESSAGE: {
        if (message_type == MessageType::kValue) {
          return Traits::NewMsg(
              field, msg,
              [&](const Desc<Traits>& type, Msg<Traits>& msg) -> absl::Status {
                auto field = Traits::FieldByNumber(type, 1);
                ABSL_DCHECK(field.has_value());
                RETURN_IF_ERROR(lex.Expect("null"));
                Traits::SetEnum(Traits::MustHaveField(type, 1), msg, 0);
                return absl::OkStatus();
              });
        }
        break;
      }
      default:
        break;
    }
    return lex.Expect("null");
  }

  switch (field_type) {
    case FieldDescriptor::TYPE_FLOAT: {
      auto x = ParseFp<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());
      Traits::SetFloat(field, msg, *x);
      break;
    }
    case FieldDescriptor::TYPE_DOUBLE: {
      auto x = ParseFp<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());
      Traits::SetDouble(field, msg, *x);
      break;
    }

    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_INT64: {
      auto x = ParseInt<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());
      Traits::SetInt64(field, msg, *x);
      break;
    }
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64: {
      auto x = ParseUInt<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());
      Traits::SetUInt64(field, msg, *x);
      break;
    }

    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_INT32: {
      auto x = ParseInt<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());
      Traits::SetInt32(field, msg, static_cast<int32_t>(*x));
      break;
    }
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32: {
      auto x = ParseUInt<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());
      Traits::SetUInt32(field, msg, static_cast<uint32_t>(*x));
      break;
    }
    case FieldDescriptor::TYPE_BOOL: {
      absl::StatusOr<JsonLexer::Kind> kind = lex.PeekKind();
      RETURN_IF_ERROR(kind.status());

      switch (*kind) {
        case JsonLexer::kTrue:
          RETURN_IF_ERROR(lex.Expect("true"));
          Traits::SetBool(field, msg, true);
          break;
        case JsonLexer::kFalse:
          RETURN_IF_ERROR(lex.Expect("false"));
          Traits::SetBool(field, msg, false);
          break;
        case JsonLexer::kStr: {
          if (!lex.options().allow_legacy_syntax) {
            goto bad;
          }

          auto x = lex.ParseUtf8();
          RETURN_IF_ERROR(x.status());

          bool flag;
          if (!absl::SimpleAtob(x->value, &flag)) {
            // Is this error a lie? Do we accept things otyher than "true" and
            // "false" because SimpleAtob does? Absolutely!
            return x->loc.Invalid("expected 'true' or 'false'");
          }
          Traits::SetBool(field, msg, flag);

          break;
        }
        bad:
        default:
          return lex.Invalid("expected 'true' or 'false'");
      }
      break;
    }
    case FieldDescriptor::TYPE_STRING:
    case FieldDescriptor::TYPE_BYTES: {
      auto x = ParseStrOrBytes<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());
      Traits::SetString(field, msg, *x);
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      absl::StatusOr<std::optional<int32_t>> x = ParseEnum<Traits>(lex, field);
      RETURN_IF_ERROR(x.status());

      if (x->has_value() || Traits::IsImplicitPresence(field)) {
        Traits::SetEnum(field, msg, x->value_or(0));
      }
      break;
    }
    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP: {
      return Traits::NewMsg(
          field, msg,
          [&](const Desc<Traits>& type, Msg<Traits>& msg) -> absl::Status {
            return ParseMessage<Traits>(lex, type, msg,
                                        /*any_reparse=*/false);
          });
    }
    default:
      return lex.Invalid(
          absl::StrCat("unsupported field type: ", Traits::FieldType(field)));
  }

  return absl::OkStatus();
}

template <typename Traits>
absl::Status EmitNull(JsonLexer& lex, Field<Traits> field, Msg<Traits>& msg) {
  switch (Traits::FieldType(field)) {
    case FieldDescriptor::TYPE_FLOAT:
      Traits::SetFloat(field, msg, 0);
      break;
    case FieldDescriptor::TYPE_DOUBLE:
      Traits::SetDouble(field, msg, 0);
      break;
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_INT64:
      Traits::SetInt64(field, msg, 0);
      break;
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64:
      Traits::SetUInt64(field, msg, 0);
      break;
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_INT32:
      Traits::SetInt32(field, msg, 0);
      break;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32:
      Traits::SetUInt32(field, msg, 0);
      break;
    case FieldDescriptor::TYPE_BOOL:
      Traits::SetBool(field, msg, false);
      break;
    case FieldDescriptor::TYPE_STRING:
    case FieldDescriptor::TYPE_BYTES:
      Traits::SetString(field, msg, "");
      break;
    case FieldDescriptor::TYPE_ENUM:
      Traits::SetEnum(field, msg, 0);
      break;
    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP:
      return Traits::NewMsg(field, msg,
                            [](const auto&, const auto&) -> absl::Status {
                              return absl::OkStatus();
                            });
    default:
      return lex.Invalid(
          absl::StrCat("unsupported field type: ", Traits::FieldType(field)));
  }
  return absl::OkStatus();
}

template <typename Traits>
absl::Status ParseArray(JsonLexer& lex, Field<Traits> field, Msg<Traits>& msg) {
  if (lex.Peek(JsonLexer::kNull)) {
    return lex.Expect("null");
  }

  return lex.VisitArray([&]() -> absl::Status {
    lex.path().NextRepeated();
    MessageType type = ClassifyMessage(Traits::FieldTypeName(field));

    if (lex.Peek(JsonLexer::kNull)) {
      if (type == MessageType::kValue) {
        return ParseSingular<Traits>(lex, field, msg);
      }
      if (type == MessageType::kNull) {
        return ParseSingular<Traits>(lex, field, msg);
      }

      if (lex.options().allow_legacy_syntax) {
        RETURN_IF_ERROR(lex.Expect("null"));
        return EmitNull<Traits>(lex, field, msg);
      }
      return lex.Invalid("null cannot occur inside of repeated fields");
    }

    // Note that this is sufficient to catch when we are inside of a ListValue,
    // because a ListValue's sole field is of type Value. Thus, we only need to
    // classify cases in which we are inside of an array and parsing messages
    // that like looking like arrays.
    //
    // This will also correctly handle e.g. writing out a ListValue with the
    // legacy syntax of `{"values": [[0], [1], [2]]}`, which does not go through
    // the custom parser handler.
    bool can_flatten =
        type != MessageType::kValue && type != MessageType::kList;
    if (can_flatten && lex.options().allow_legacy_syntax &&
        lex.Peek(JsonLexer::kArr)) {
      // You read that right. In legacy mode, if we encounter an array within
      // an array, we just flatten it as part of the current array!
      //
      // This DOES NOT apply when parsing a google.protobuf.Value or a
      // google.protobuf.ListValue!
      return ParseArray<Traits>(lex, field, msg);
    }
    return ParseSingular<Traits>(lex, field, msg);
  });
}

template <typename Traits>
absl::Status ParseMap(JsonLexer& lex, Field<Traits> field, Msg<Traits>& msg) {
  if (lex.Peek(JsonLexer::kNull)) {
    return lex.Expect("null");
  }

  absl::flat_hash_set<std::string> keys_seen;
  return lex.VisitObject(
      [&](LocationWith<MaybeOwnedString>& key) -> absl::Status {
        lex.path().NextRepeated();
        auto insert_result = keys_seen.emplace(key.value.AsView());
        if (!insert_result.second) {
          return key.loc.Invalid(absl::StrFormat(
              "got unexpectedly-repeated repeated map key: '%s'",
              key.value.AsView()));
        }
        return Traits::NewMsg(
            field, msg,
            [&](const Desc<Traits>& type, Msg<Traits>& entry) -> absl::Status {
              auto key_field = Traits::KeyField(type);
              switch (Traits::FieldType(key_field)) {
                case FieldDescriptor::TYPE_INT64:
                case FieldDescriptor::TYPE_SINT64:
                case FieldDescriptor::TYPE_SFIXED64: {
                  int64_t n;
                  if (!absl::SimpleAtoi(key.value.AsView(), &n)) {
                    return key.loc.Invalid(
                        "non-number characters in quoted number");
                  }
                  Traits::SetInt64(key_field, entry, n);
                  break;
                }
                case FieldDescriptor::TYPE_UINT64:
                case FieldDescriptor::TYPE_FIXED64: {
                  uint64_t n;
                  if (!absl::SimpleAtoi(key.value.AsView(), &n)) {
                    return key.loc.Invalid(
                        "non-number characters in quoted number");
                  }
                  Traits::SetUInt64(key_field, entry, n);
                  break;
                }
                case FieldDescriptor::TYPE_INT32:
                case FieldDescriptor::TYPE_SINT32:
                case FieldDescriptor::TYPE_SFIXED32: {
                  int32_t n;
                  if (!absl::SimpleAtoi(key.value.AsView(), &n)) {
                    return key.loc.Invalid(
                        "non-number characters in quoted number");
                  }
                  Traits::SetInt32(key_field, entry, n);
                  break;
                }
                case FieldDescriptor::TYPE_UINT32:
                case FieldDescriptor::TYPE_FIXED32: {
                  uint32_t n;
                  if (!absl::SimpleAtoi(key.value.AsView(), &n)) {
                    return key.loc.Invalid(
                        "non-number characters in quoted number");
                  }
                  Traits::SetUInt32(key_field, entry, n);
                  break;
                }
                case FieldDescriptor::TYPE_BOOL: {
                  if (key.value == "true") {
                    Traits::SetBool(key_field, entry, true);
                  } else if (key.value == "false") {
                    Traits::SetBool(key_field, entry, false);
                  } else {
                    return key.loc.Invalid(absl::StrFormat(
                        "expected bool string, got '%s'", key.value.AsView()));
                  }
                  break;
                }
                case FieldDescriptor::TYPE_ENUM: {
                  MaybeOwnedString key_str = key.value;
                  auto e = ParseEnumFromStr<Traits>(lex, key_str, field);
                  RETURN_IF_ERROR(e.status());
                  Traits::SetEnum(key_field, entry, e->value_or(0));
                  break;
                }
                case FieldDescriptor::TYPE_STRING: {
                  Traits::SetString(key_field, entry,
                                    std::move(key.value.ToString()));
                  break;
                }
                default:
                  return lex.Invalid("unsupported map key type");
              }

              return ParseSingular<Traits>(lex, Traits::ValueField(type),
                                           entry);
            });
      });
}

std::optional<uint32_t> TakeTimeDigitsWithSuffixAndAdvance(
    absl::string_view& data, int max_digits, absl::string_view end) {
  ABSL_DCHECK_LE(max_digits, 9);

  uint32_t val = 0;
  int limit = max_digits;
  while (!data.empty()) {
    if (limit-- < 0) {
      return absl::nullopt;
    }
    uint32_t digit = data[0] - '0';
    if (digit >= 10) {
      break;
    }

    val *= 10;
    val += digit;
    data = data.substr(1);
  }
  if (!absl::StartsWith(data, end)) {
    return absl::nullopt;
  }

  data = data.substr(end.size());
  return val;
}

std::optional<int32_t> TakeNanosAndAdvance(absl::string_view& data) {
  int32_t frac_secs = 0;
  size_t frac_digits = 0;
  if (absl::StartsWith(data, ".")) {
    for (char c : data.substr(1)) {
      if (!absl::ascii_isdigit(c)) {
        break;
      }
      ++frac_digits;
    }
    auto digits = data.substr(1, frac_digits);
    if (frac_digits == 0 || frac_digits > 9 ||
        !absl::SimpleAtoi(digits, &frac_secs)) {
      return absl::nullopt;
    }
    data = data.substr(frac_digits + 1);
  }
  for (int i = 0; i < 9 - frac_digits; ++i) {
    frac_secs *= 10;
  }
  return frac_secs;
}

template <typename Traits>
absl::Status ParseTimestamp(JsonLexer& lex, const Desc<Traits>& desc,
                            Msg<Traits>& msg) {
  if (lex.Peek(JsonLexer::kNull)) {
    return lex.Expect("null");
  }

  absl::StatusOr<LocationWith<MaybeOwnedString>> str = lex.ParseUtf8();
  RETURN_IF_ERROR(str.status());

  absl::string_view data = str->value.AsView();
  if (data.size() < 20) {
    return str->loc.Invalid("timestamp string too short");
  }

  int64_t secs;
  {
    /* 1972-01-01T01:00:00 */
    auto year = TakeTimeDigitsWithSuffixAndAdvance(data, 4, "-");
    if (!year.has_value() || *year == 0) {
      return str->loc.Invalid("bad year in timestamp");
    }
    auto mon = TakeTimeDigitsWithSuffixAndAdvance(data, 2, "-");
    if (!mon.has_value() || *mon == 0) {
      return str->loc.Invalid("bad month in timestamp");
    }
    auto day = TakeTimeDigitsWithSuffixAndAdvance(data, 2, "T");
    if (!day.has_value() || *day == 0) {
      return str->loc.Invalid("bad day in timestamp");
    }
    auto hour = TakeTimeDigitsWithSuffixAndAdvance(data, 2, ":");
    if (!hour.has_value()) {
      return str->loc.Invalid("bad hours in timestamp");
    }
    auto min = TakeTimeDigitsWithSuffixAndAdvance(data, 2, ":");
    if (!min.has_value()) {
      return str->loc.Invalid("bad minutes in timestamp");
    }
    auto sec = TakeTimeDigitsWithSuffixAndAdvance(data, 2, "");
    if (!sec.has_value()) {
      return str->loc.Invalid("bad seconds in timestamp");
    }

    uint32_t m_adj = *mon - 3;  // March-based month.
    uint32_t carry = m_adj > *mon ? 1 : 0;

    uint32_t year_base = 4800;  // Before min year, multiple of 400.
    uint32_t y_adj = *year + year_base - carry;

    uint32_t month_days = ((m_adj + carry * 12) * 62719 + 769) / 2048;
    uint32_t leap_days = y_adj / 4 - y_adj / 100 + y_adj / 400;
    int32_t epoch_days =
        y_adj * 365 + leap_days + month_days + (*day - 1) - 2472632;

    secs = int64_t{epoch_days} * 86400 + *hour * 3600 + *min * 60 + *sec;
  }

  auto nanos = TakeNanosAndAdvance(data);
  if (!nanos.has_value()) {
    return str->loc.Invalid("timestamp had bad nanoseconds");
  }

  if (data.empty()) {
    return str->loc.Invalid("timestamp missing timezone offset");
  }

  {
    // [+-]hh:mm or Z
    bool neg = false;
    switch (data[0]) {
      case '-':
        neg = true;
        ABSL_FALLTHROUGH_INTENDED;
      case '+': {
        if (data.size() != 6) {
          return str->loc.Invalid("timestamp offset of wrong size.");
        }

        data = data.substr(1);
        auto hour = TakeTimeDigitsWithSuffixAndAdvance(data, 2, ":");
        auto mins = TakeTimeDigitsWithSuffixAndAdvance(data, 2, "");
        if (!hour.has_value() || !mins.has_value()) {
          return str->loc.Invalid("timestamp offset has bad hours and minutes");
        }

        int64_t offset = (*hour * 60 + *mins) * 60;
        secs += (neg ? offset : -offset);
        break;
      }
      // Lowercase z is not accepted, per the spec.
      case 'Z':
        if (data.size() == 1) {
          break;
        }
        ABSL_FALLTHROUGH_INTENDED;
      default:
        return str->loc.Invalid("bad timezone offset");
    }
  }

  Traits::SetInt64(Traits::MustHaveField(desc, 1), msg, secs);
  Traits::SetInt32(Traits::MustHaveField(desc, 2), msg, *nanos);

  return absl::OkStatus();
}

template <typename Traits>
absl::Status ParseDuration(JsonLexer& lex, const Desc<Traits>& desc,
                           Msg<Traits>& msg) {
  if (lex.Peek(JsonLexer::kNull)) {
    return lex.Expect("null");
  }

  constexpr int64_t kMaxSeconds = int64_t{3652500} * 86400;

  absl::StatusOr<LocationWith<MaybeOwnedString>> str = lex.ParseUtf8();
  RETURN_IF_ERROR(str.status());

  size_t int_part_end = 0;
  for (char c : str->value.AsView()) {
    if (!absl::ascii_isdigit(c) && c != '-') {
      break;
    }
    ++int_part_end;
  }
  if (int_part_end == 0) {
    return str->loc.Invalid("duration must start with an integer");
  }

  absl::string_view sec_digits = str->value.AsView().substr(0, int_part_end);
  int64_t secs;
  if (!absl::SimpleAtoi(sec_digits, &secs)) {
    return str->loc.Invalid("duration had bad seconds");
  }

  if (secs > kMaxSeconds || secs < -kMaxSeconds) {
    return str->loc.Invalid("duration out of range");
  }

  absl::string_view rest = str->value.AsView().substr(int_part_end);
  auto nanos = TakeNanosAndAdvance(rest);
  if (!nanos.has_value()) {
    return str->loc.Invalid("duration had bad nanoseconds");
  }

  bool isNegative = (secs < 0) || absl::StartsWith(sec_digits, "-");
  if (isNegative) {
    *nanos *= -1;
  }

  if (rest != "s") {
    return str->loc.Invalid("duration must end with a single 's'");
  }

  Traits::SetInt64(Traits::MustHaveField(desc, 1), msg, secs);
  Traits::SetInt32(Traits::MustHaveField(desc, 2), msg, *nanos);

  return absl::OkStatus();
}

template <typename Traits>
absl::Status ParseFieldMask(JsonLexer& lex, const Desc<Traits>& desc,
                            Msg<Traits>& msg) {
  absl::StatusOr<LocationWith<MaybeOwnedString>> str = lex.ParseUtf8();
  RETURN_IF_ERROR(str.status());
  auto paths = str->value.AsView();

  // The special case of the empty string is not handled correctly below,
  // because StrSplit("", ',') is [""], not [].
  if (paths.empty()) {
    return absl::OkStatus();
  }

  // google.protobuf.FieldMask has a single field with number 1.
  auto paths_field = Traits::MustHaveField(desc, 1);
  for (absl::string_view path : absl::StrSplit(paths, ',')) {
    std::string snake_path;
    // Assume approximately six-letter words, so add one extra space for an
    // underscore for every six bytes.
    snake_path.reserve(path.size() * 7 / 6);
    for (char c : path) {
      if (absl::ascii_isdigit(c) || absl::ascii_islower(c) || c == '.') {
        snake_path.push_back(c);
      } else if (absl::ascii_isupper(c)) {
        snake_path.push_back('_');
        snake_path.push_back(absl::ascii_tolower(c));
      } else if (lex.options().allow_legacy_syntax) {
        snake_path.push_back(c);
      } else {
        return str->loc.Invalid("unexpected character in FieldMask");
      }
    }
    Traits::SetString(paths_field, msg, snake_path);
  }

  return absl::OkStatus();
}

template <typename Traits>
absl::Status ParseAny(JsonLexer& lex, const Desc<Traits>& desc,
                      Msg<Traits>& msg) {
  // Buffer an entire object. Because @type can occur anywhere, we're forced
  // to do this.
  RETURN_IF_ERROR(lex.SkipToToken());
  auto mark = lex.BeginMark();

  // Search for @type, buffering the entire object along the way so we can
  // reparse it.
  std::optional<MaybeOwnedString> type_url;
  RETURN_IF_ERROR(lex.VisitObject(
      [&](const LocationWith<MaybeOwnedString>& key) -> absl::Status {
        if (key.value == "@type") {
          if (type_url.has_value()) {
            return key.loc.Invalid("repeated @type in Any");
          }

          absl::StatusOr<LocationWith<MaybeOwnedString>> maybe_url =
              lex.ParseUtf8();
          RETURN_IF_ERROR(maybe_url.status());
          type_url = std::move(maybe_url)->value;
          return absl::OkStatus();
        }
        return lex.SkipValue();
      }));

  // Build a new lexer over the skipped object.
  absl::string_view any_text = mark.value.UpToUnread();
  io::ArrayInputStream in(any_text.data(), any_text.size());
  // Copying lex.options() is important; it inherits the recursion
  // limit.
  JsonLexer any_lex(&in, lex.options(), &lex.path(), mark.loc);

  if (!type_url.has_value() && !lex.options().allow_legacy_syntax) {
    return mark.loc.Invalid("missing @type in Any");
  }

  if (type_url.has_value()) {
    Traits::SetString(Traits::MustHaveField(desc, 1), msg, type_url->AsView());
    return Traits::NewDynamic(
        Traits::MustHaveField(desc, 2), type_url->ToString(), msg,
        [&](const Desc<Traits>& desc, Msg<Traits>& msg) {
          auto pop = any_lex.path().Push("<any>", FieldDescriptor::TYPE_MESSAGE,
                                         Traits::TypeName(desc));
          return ParseMessage<Traits>(any_lex, desc, msg,
                                      /*any_reparse=*/true);
        });
  } else {
    // Empty {} is accepted in legacy mode.
    ABSL_DCHECK(lex.options().allow_legacy_syntax);
    RETURN_IF_ERROR(any_lex.VisitObject([&](auto&) {
      return mark.loc.Invalid(
          "in legacy mode, missing @type in Any is only allowed for an empty "
          "object");
    }));
    return absl::OkStatus();
  }
}

// These are mutually recursive with ParseValue.
template <typename Traits>
absl::Status ParseStructValue(JsonLexer& lex, const Desc<Traits>& desc,
                              Msg<Traits>& msg);
template <typename Traits>
absl::Status ParseListValue(JsonLexer& lex, const Desc<Traits>& desc,
                            Msg<Traits>& msg);

template <typename Traits>
absl::Status ParseValue(JsonLexer& lex, const Desc<Traits>& desc,
                        Msg<Traits>& msg) {
  auto kind = lex.PeekKind();
  RETURN_IF_ERROR(kind.status());
  // NOTE: The field numbers 1 through 6 are the numbers of the oneof fields
  // in google.protobuf.Value. Conformance tests verify the correctness of
  // these numbers.
  switch (*kind) {
    case JsonLexer::kNull: {
      auto field = Traits::MustHaveField(desc, 1);
      auto pop =
          lex.path().Push(Traits::FieldName(field), Traits::FieldType(field),
                          Traits::FieldTypeName(field));

      RETURN_IF_ERROR(lex.Expect("null"));
      Traits::SetEnum(field, msg, 0);
      break;
    }
    case JsonLexer::kNum: {
      auto field = Traits::MustHaveField(desc, 2);
      auto pop =
          lex.path().Push(Traits::FieldName(field), Traits::FieldType(field),
                          Traits::FieldTypeName(field));

      auto number = lex.ParseNumber();
      RETURN_IF_ERROR(number.status());
      Traits::SetDouble(field, msg, number->value);
      break;
    }
    case JsonLexer::kStr: {
      auto field = Traits::MustHaveField(desc, 3);
      auto pop =
          lex.path().Push(Traits::FieldName(field), Traits::FieldType(field),
                          Traits::FieldTypeName(field));

      auto str = lex.ParseUtf8();
      RETURN_IF_ERROR(str.status());
      Traits::SetString(field, msg, std::move(str->value.ToString()));
      break;
    }
    case JsonLexer::kFalse:
    case JsonLexer::kTrue: {
      auto field = Traits::MustHaveField(desc, 4);
      auto pop =
          lex.path().Push(Traits::FieldName(field), Traits::FieldType(field),
                          Traits::FieldTypeName(field));

      // "Quoted" bools, including non-standard Abseil Atob bools, are not
      // supported, because all strings are treated as genuine JSON strings.
      if (*kind == JsonLexer::kTrue) {
        RETURN_IF_ERROR(lex.Expect("true"));
        Traits::SetBool(field, msg, true);
      } else {
        RETURN_IF_ERROR(lex.Expect("false"));
        Traits::SetBool(field, msg, false);
      }
      break;
    }
    case JsonLexer::kObj: {
      auto field = Traits::MustHaveField(desc, 5);
      auto pop =
          lex.path().Push(Traits::FieldName(field), Traits::FieldType(field),
                          Traits::FieldTypeName(field));

      return Traits::NewMsg(field, msg, [&](auto& desc, auto& msg) {
        return ParseStructValue<Traits>(lex, desc, msg);
      });
    }
    case JsonLexer::kArr: {
      auto field = Traits::MustHaveField(desc, 6);
      auto pop =
          lex.path().Push(Traits::FieldName(field), Traits::FieldType(field),
                          Traits::FieldTypeName(field));

      return Traits::NewMsg(field, msg, [&](auto& desc, auto& msg) {
        return ParseListValue<Traits>(lex, desc, msg);
      });
    }
  }

  return absl::OkStatus();
}

template <typename Traits>
absl::Status ParseStructValue(JsonLexer& lex, const Desc<Traits>& desc,
                              Msg<Traits>& msg) {
  auto entry_field = Traits::MustHaveField(desc, 1);
  auto pop = lex.path().Push("<struct>", FieldDescriptor::TYPE_MESSAGE,
                             Traits::FieldTypeName(entry_field));

  // Structs are always cleared even if set to {}.
  Traits::RecordAsSeen(entry_field, msg);

  // Parsing a map does the right thing: Struct has a single map<string,
  // Value> field; keys are correctly parsed as strings, and the values
  // recurse into ParseMessage, which will be routed into ParseValue. This
  // results in some extra overhead, but performance is not what we're going
  // for here.
  return ParseMap<Traits>(lex, entry_field, msg);
}

template <typename Traits>
absl::Status ParseListValue(JsonLexer& lex, const Desc<Traits>& desc,
                            Msg<Traits>& msg) {
  auto entry_field = Traits::MustHaveField(desc, 1);
  auto pop = lex.path().Push("<list>", FieldDescriptor::TYPE_MESSAGE,
                             Traits::FieldTypeName(entry_field));

  // ListValues are always cleared even if set to [].
  Traits::RecordAsSeen(entry_field, msg);
  // Parsing an array does the right thing: see the analogous comment in
  // ParseStructValue.
  return ParseArray<Traits>(lex, entry_field, msg);
}

template <typename Traits>
absl::Status ParseField(JsonLexer& lex, const Desc<Traits>& desc,
                        absl::string_view name, Msg<Traits>& msg) {
  std::optional<Field<Traits>> field;
  if (absl::StartsWith(name, "[") && absl::EndsWith(name, "]")) {
    absl::string_view extn_name = name.substr(1, name.size() - 2);
    field = Traits::ExtensionByName(desc, extn_name);

    if (field.has_value()) {
      // The check for whether this is an invalid field occurs below, since it
      // is combined for both extension and non-extension fields.
      auto correct_type_name = Traits::TypeName(desc);
      if (Traits::TypeName(Traits::ContainingType(*field)) !=
          correct_type_name) {
        return lex.Invalid(absl::StrFormat(
            "'%s' is a known extension name, but is not an extension "
            "of '%s' as expected",
            extn_name, correct_type_name));
      }
    }
  } else {
    field = Traits::FieldByName(desc, name);
  }

  if (!field.has_value()) {
    if (!lex.options().ignore_unknown_fields) {
      return lex.Invalid(absl::StrFormat("no such field: '%s'", name));
    }
    return lex.SkipValue();
  }

  auto pop = lex.path().Push(name, Traits::FieldType(*field),
                             Traits::FieldTypeName(*field));

  if (Traits::HasParsed(
          *field, msg,
          /*allow_repeated_non_oneof=*/lex.options().allow_legacy_syntax) &&
      !lex.Peek(JsonLexer::kNull)) {
    return lex.Invalid(absl::StrFormat(
        "'%s' has already been set (either directly or as part of a oneof)",
        name));
  }

  if (Traits::IsMap(*field)) {
    return ParseMap<Traits>(lex, *field, msg);
  }

  if (Traits::IsRepeated(*field)) {
    if (lex.options().allow_legacy_syntax && !lex.Peek(JsonLexer::kArr)) {
      // The original ESF parser permits a single element in place of an array
      // thereof.
      return ParseSingular<Traits>(lex, *field, msg);
    }
    return ParseArray<Traits>(lex, *field, msg);
  }

  return ParseSingular<Traits>(lex, *field, msg);
}

template <typename Traits>
absl::Status ParseMessage(JsonLexer& lex, const Desc<Traits>& desc,
                          Msg<Traits>& msg, bool any_reparse) {
  MessageType type = ClassifyMessage(Traits::TypeName(desc));
  if (!any_reparse) {
    switch (type) {
      case MessageType::kAny:
        return ParseAny<Traits>(lex, desc, msg);
      case MessageType::kValue:
        return ParseValue<Traits>(lex, desc, msg);
      case MessageType::kStruct:
        return ParseStructValue<Traits>(lex, desc, msg);
      default:
        break;
    }
    // For some types, the ESF parser permits parsing the "non-special" version.
    // It is not clear if this counts as out-of-spec, but we're treating it as
    // such.
    bool is_upcoming_object = lex.Peek(JsonLexer::kObj);
    if (!(is_upcoming_object && lex.options().allow_legacy_syntax)) {
      switch (type) {
        case MessageType::kList:
          return ParseListValue<Traits>(lex, desc, msg);
        case MessageType::kWrapper: {
          return ParseSingular<Traits>(lex, Traits::MustHaveField(desc, 1),
                                       msg);
        }
        case MessageType::kTimestamp:
          return ParseTimestamp<Traits>(lex, desc, msg);
        case MessageType::kDuration:
          return ParseDuration<Traits>(lex, desc, msg);
        case MessageType::kFieldMask:
          return ParseFieldMask<Traits>(lex, desc, msg);
        default:
          break;
      }
    }
  }

  return lex.VisitObject(
      [&](LocationWith<MaybeOwnedString>& name) -> absl::Status {
        // If this is a well-known type, we expect its contents to be inside
        // of a JSON field named "value".
        if (any_reparse) {
          if (name.value == "@type") {
            RETURN_IF_ERROR(lex.SkipValue());
            return absl::OkStatus();
          }
          if (type != MessageType::kNotWellKnown) {
            if (name.value != "value") {
              return lex.Invalid(
                  "fields in a well-known-typed Any must be @type or value");
            }
            // Parse the upcoming value as the message itself. This is *not*
            // an Any reparse because we do not expect to see @type in the
            // upcoming value.
            return ParseMessage<Traits>(lex, desc, msg,
                                        /*any_reparse=*/false);
          }
        }

        return ParseField<Traits>(lex, desc, name.value.ToString(), msg);
      });
}
}  // namespace

absl::Status JsonStringToMessage(absl::string_view input, Message* message,
                                 json_internal::ParseOptions options) {
  MessagePath path(message->GetDescriptor()->full_name());
  if (PROTOBUF_DEBUG) {
    ABSL_DLOG(INFO) << "json2/input: " << absl::CHexEscape(input);
  }
  io::ArrayInputStream in(input.data(), input.size());
  JsonLexer lex(&in, options, &path);

  ParseProto2Descriptor::Msg msg(message);
  absl::Status s =
      ParseMessage<ParseProto2Descriptor>(lex, *message->GetDescriptor(), msg,
                                          /*any_reparse=*/false);
  if (s.ok() && !lex.AtEof()) {
    s = absl::InvalidArgumentError(
        "extraneous characters after end of JSON object");
  }

  if (PROTOBUF_DEBUG) {
    ABSL_DLOG(INFO) << "json2/status: " << s;
    ABSL_DLOG(INFO) << "json2/output: " << message->DebugString();
  }
  return s;
}

absl::Status JsonToBinaryStream(google::protobuf::util::TypeResolver* resolver,
                                const std::string& type_url,
                                io::ZeroCopyInputStream* json_input,
                                io::ZeroCopyOutputStream* binary_output,
                                json_internal::ParseOptions options) {
  // NOTE: Most of the contortions in this function are to allow for capture of
  // input and output of the parser in ABSL_DLOG mode. Destruction order is very
  // critical in this function, because io::ZeroCopy*Stream types usually only
  // flush on destruction.

  // For ABSL_DLOG, we would like to print out the input and output, which
  // requires buffering both instead of doing "zero copy". This block, and the
  // one at the end of the function, set up and tear down interception of the
  // input and output streams.
  std::string copy;
  std::string out;
  std::optional<io::ArrayInputStream> tee_input;
  std::optional<io::StringOutputStream> tee_output;
  if (PROTOBUF_DEBUG) {
    const void* data;
    int len;
    while (json_input->Next(&data, &len)) {
      copy.resize(copy.size() + len);
      std::memcpy(&copy[copy.size() - len], data, len);
    }
    tee_input.emplace(copy.data(), copy.size());
    tee_output.emplace(&out);
    ABSL_DLOG(INFO) << "json2/input: " << absl::CHexEscape(copy);
  }

  // This scope forces the CodedOutputStream inside of `msg` to flush before we
  // possibly handle logging the binary protobuf output.
  absl::Status s;
  {
    MessagePath path(type_url);
    JsonLexer lex(tee_input.has_value() ? &*tee_input : json_input, options,
                  &path);
    Msg<ParseProto3Type> msg(tee_output.has_value() ? &*tee_output
                                                    : binary_output);

    ResolverPool pool(resolver);
    auto desc = pool.FindMessage(type_url);
    RETURN_IF_ERROR(desc.status());

    s = ParseMessage<ParseProto3Type>(lex, **desc, msg, /*any_reparse=*/false);
    if (s.ok() && !lex.AtEof()) {
      s = absl::InvalidArgumentError(
          "extraneous characters after end of JSON object");
    }
  }

  if (PROTOBUF_DEBUG) {
    tee_output.reset();  // Flush the output stream.
    io::zc_sink_internal::ZeroCopyStreamByteSink(binary_output)
        .Append(out.data(), out.size());
    ABSL_DLOG(INFO) << "json2/status: " << s;
    ABSL_DLOG(INFO) << "json2/output: " << absl::BytesToHexString(out);
  }

  return s;
}
}  // namespace json_internal
}  // namespace protobuf
}  // namespace google

