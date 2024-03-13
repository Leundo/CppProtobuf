// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google_protobuf_json_internal_unparser.hpp"

#include <cfloat>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_status_status.hpp>
#include <CppAbseil/absl_strings_ascii.hpp>
#include <CppAbseil/absl_strings_escaping.hpp>
#include <CppAbseil/absl_strings_str_cat.hpp>
#include <CppAbseil/absl_strings_str_format.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include <CppAbseil/absl_types_optional.hpp>
#include "google_protobuf_descriptor.hpp"
#include "google_protobuf_dynamic_message.hpp"
#include "google_protobuf_io_coded_stream.hpp"
#include "google_protobuf_io_zero_copy_stream.hpp"
#include "google_protobuf_json_internal_descriptor_traits.hpp"
#include "google_protobuf_json_internal_unparser_traits.hpp"
#include "google_protobuf_json_internal_writer.hpp"
#include "google_protobuf_message.hpp"
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
template <typename Traits>
bool IsEmpty(const Msg<Traits>& msg, const Desc<Traits>& desc) {
  size_t count = Traits::FieldCount(desc);
  for (size_t i = 0; i < count; ++i) {
    if (Traits::GetSize(Traits::FieldByIndex(desc, i), msg) > 0) {
      return false;
    }
  }
  return true;
}

enum class IntegerEnumStyle {
  kQuoted,
  kUnquoted,
};

template <typename Traits>
void WriteEnum(JsonWriter& writer, Field<Traits> field, int32_t value,
               IntegerEnumStyle int_style = IntegerEnumStyle::kUnquoted) {
  if (ClassifyMessage(Traits::FieldTypeName(field)) == MessageType::kNull) {
    writer.Write("null");
    return;
  }

  if (!writer.options().always_print_enums_as_ints) {
    auto name = Traits::EnumNameByNumber(field, value);
    if (name.ok()) {
      writer.Write("\"", *name, "\"");
      return;
    }
  }

  if (int_style == IntegerEnumStyle::kQuoted) {
    writer.Write("\"", value, "\"");
  } else {
    writer.Write(value);
  }
}

// Returns true if x round-trips through being cast to a double, i.e., if
// x is represenable exactly as a double. This is a slightly weaker condition
// than x < 2^52.
template <typename Int>
bool RoundTripsThroughDouble(Int x) {
  auto d = static_cast<double>(x);
  // d has guaranteed to be finite with no fractional part, because it came from
  // an integer, so we only need to check that it is not outside of the
  // representable range of `int`. The way to do this is somewhat not obvious:
  // UINT64_MAX isn't representable, and what it gets rounded to when we go
  // int->double is unspecified!
  //
  // Thus, we have to go through ldexp.
  double min = 0;
  double max_plus_one = std::ldexp(1.0, sizeof(Int) * 8);
  if (std::is_signed<Int>::value) {
    max_plus_one /= 2;
    min = -max_plus_one;
  }

  if (d < min || d >= max_plus_one) {
    return false;
  }

  return static_cast<Int>(d) == x;
}

// Mutually recursive with functions that follow.
template <typename Traits>
absl::Status WriteMessage(JsonWriter& writer, const Msg<Traits>& msg,
                          const Desc<Traits>& desc, bool is_top_level = false);

// This is templatized so that defaults, singular, and repeated fields can both
// use the same enormous switch-case.
template <typename Traits, typename... Args>
absl::Status WriteSingular(JsonWriter& writer, Field<Traits> field,
                           Args&&... args) {
  // When the pack `args` is empty, the caller has requested printing the
  // default value.
  bool is_default = sizeof...(Args) == 0;
  switch (Traits::FieldType(field)) {
    case FieldDescriptor::TYPE_FLOAT: {
      auto x = Traits::GetFloat(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      if (writer.options().allow_legacy_syntax && is_default &&
          !std::isfinite(*x)) {
        *x = 0;
      }
      writer.Write(*x);
      break;
    }
    case FieldDescriptor::TYPE_DOUBLE: {
      auto x = Traits::GetDouble(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      if (writer.options().allow_legacy_syntax && is_default &&
          !std::isfinite(*x)) {
        *x = 0;
      }
      writer.Write(*x);
      break;
    }
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_INT64: {
      auto x = Traits::GetInt64(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      if (writer.options().unquote_int64_if_possible &&
          RoundTripsThroughDouble(*x)) {
        writer.Write(*x);
      } else {
        writer.Write(MakeQuoted(*x));
      }
      break;
    }
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64: {
      auto x = Traits::GetUInt64(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      if (writer.options().unquote_int64_if_possible &&
          RoundTripsThroughDouble(*x)) {
        writer.Write(*x);
      } else {
        writer.Write(MakeQuoted(*x));
      }
      break;
    }
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_INT32: {
      auto x = Traits::GetInt32(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      writer.Write(*x);
      break;
    }
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32: {
      auto x = Traits::GetUInt32(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      writer.Write(*x);
      break;
    }
    case FieldDescriptor::TYPE_BOOL: {
      auto x = Traits::GetBool(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      writer.Write(*x ? "true" : "false");
      break;
    }
    case FieldDescriptor::TYPE_STRING: {
      auto x = Traits::GetString(field, writer.ScratchBuf(),
                                 std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      writer.Write(MakeQuoted(*x));
      break;
    }
    case FieldDescriptor::TYPE_BYTES: {
      auto x = Traits::GetString(field, writer.ScratchBuf(),
                                 std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      if (writer.options().allow_legacy_syntax && is_default) {
        // Although difficult to verify, it appears that the original ESF parser
        // fails to unescape the contents of a
        // google.protobuf.Field.default_value, which may potentially be
        // escaped if it is for a `bytes` field (note that default_value is a
        // `string` regardless of what type the field is).
        //
        // However, our parser's type.proto guts actually know to do this
        // correctly, so this bug must be manually re-introduced.
        writer.WriteBase64(absl::CEscape(*x));
      } else {
        writer.WriteBase64(*x);
      }
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      auto x = Traits::GetEnumValue(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      WriteEnum<Traits>(writer, field, *x);
      break;
    }
    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP: {
      auto x = Traits::GetMessage(field, std::forward<Args>(args)...);
      RETURN_IF_ERROR(x.status());
      return WriteMessage<Traits>(writer, **x, Traits::GetDesc(**x));
    }
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("unsupported field type: ", Traits::FieldType(field)));
  }

  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteRepeated(JsonWriter& writer, const Msg<Traits>& msg,
                           Field<Traits> field) {
  writer.Write("[");
  writer.Push();

  size_t count = Traits::GetSize(field, msg);
  bool first = true;
  for (size_t i = 0; i < count; ++i) {
    if (ClassifyMessage(Traits::FieldTypeName(field)) == MessageType::kValue) {
      bool empty = false;
      RETURN_IF_ERROR(Traits::WithFieldType(
          field, [&](const Desc<Traits>& desc) -> absl::Status {
            auto inner = Traits::GetMessage(field, msg, i);
            RETURN_IF_ERROR(inner.status());
            empty = IsEmpty<Traits>(**inner, desc);
            return absl::OkStatus();
          }));

      // Empty google.protobuf.Values are silently discarded.
      if (empty) {
        continue;
      }
    }
    writer.WriteComma(first);
    writer.NewLine();
    RETURN_IF_ERROR(WriteSingular<Traits>(writer, field, msg, i));
  }

  writer.Pop();
  if (!first) {
    writer.NewLine();
  }
  writer.Write("]");
  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteMapKey(JsonWriter& writer, const Msg<Traits>& entry,
                         Field<Traits> field) {
  switch (Traits::FieldType(field)) {
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_SINT64:
    case FieldDescriptor::TYPE_INT64: {
      auto x = Traits::GetInt64(field, entry);
      RETURN_IF_ERROR(x.status());
      writer.Write(MakeQuoted(*x));
      break;
    }
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64: {
      auto x = Traits::GetUInt64(field, entry);
      RETURN_IF_ERROR(x.status());
      writer.Write(MakeQuoted(*x));
      break;
    }
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_SINT32:
    case FieldDescriptor::TYPE_INT32: {
      auto x = Traits::GetInt32(field, entry);
      RETURN_IF_ERROR(x.status());
      writer.Write(MakeQuoted(*x));
      break;
    }
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32: {
      auto x = Traits::GetUInt32(field, entry);
      RETURN_IF_ERROR(x.status());
      writer.Write(MakeQuoted(*x));
      break;
    }
    case FieldDescriptor::TYPE_BOOL: {
      auto x = Traits::GetBool(field, entry);
      RETURN_IF_ERROR(x.status());
      writer.Write(MakeQuoted(*x ? "true" : "false"));
      break;
    }
    case FieldDescriptor::TYPE_STRING: {
      auto x = Traits::GetString(field, writer.ScratchBuf(), entry);
      RETURN_IF_ERROR(x.status());
      writer.Write(MakeQuoted(*x));
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      auto x = Traits::GetEnumValue(field, entry);
      RETURN_IF_ERROR(x.status());
      WriteEnum<Traits>(writer, field, *x, IntegerEnumStyle::kQuoted);
      break;
    }
    default:
      return absl::InvalidArgumentError(
          absl::StrCat("unsupported map key type: ", Traits::FieldType(field)));
  }
  return absl::OkStatus();
}

template <typename Traits>
absl::StatusOr<bool> IsEmptyValue(const Msg<Traits>& msg, Field<Traits> field) {
  if (ClassifyMessage(Traits::FieldTypeName(field)) != MessageType::kValue) {
    return false;
  }
  bool empty = false;
  RETURN_IF_ERROR(Traits::WithFieldType(
      field, [&](const Desc<Traits>& desc) -> absl::Status {
        auto inner = Traits::GetMessage(field, msg);
        RETURN_IF_ERROR(inner.status());
        empty = IsEmpty<Traits>(**inner, desc);
        return absl::OkStatus();
      }));
  return empty;
}

template <typename Traits>
absl::Status WriteMap(JsonWriter& writer, const Msg<Traits>& msg,
                      Field<Traits> field) {
  writer.Write("{");
  writer.Push();

  size_t count = Traits::GetSize(field, msg);
  bool first = true;
  for (size_t i = 0; i < count; ++i) {
    absl::StatusOr<const Msg<Traits>*> entry =
        Traits::GetMessage(field, msg, i);
    RETURN_IF_ERROR(entry.status());
    const Desc<Traits>& type = Traits::GetDesc(**entry);

    auto is_empty = IsEmptyValue<Traits>(**entry, Traits::ValueField(type));
    RETURN_IF_ERROR(is_empty.status());
    if (*is_empty) {
      // Empty google.protobuf.Values are silently discarded.
      continue;
    }

    writer.WriteComma(first);
    writer.NewLine();
    RETURN_IF_ERROR(
        WriteMapKey<Traits>(writer, **entry, Traits::KeyField(type)));
    writer.Write(":");
    writer.Whitespace(" ");
    RETURN_IF_ERROR(
        WriteSingular<Traits>(writer, Traits::ValueField(type), **entry));
  }

  writer.Pop();
  if (!first) {
    writer.NewLine();
  }
  writer.Write("}");
  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteField(JsonWriter& writer, const Msg<Traits>& msg,
                        Field<Traits> field, bool& first) {
  if (!Traits::IsRepeated(field)) {  // Repeated case is handled in
                                     // WriteRepeated.
    auto is_empty = IsEmptyValue<Traits>(msg, field);
    RETURN_IF_ERROR(is_empty.status());
    if (*is_empty) {
      // Empty google.protobuf.Values are silently discarded.
      return absl::OkStatus();
    }
  }

  writer.WriteComma(first);
  writer.NewLine();

  if (Traits::IsExtension(field)) {
    writer.Write(MakeQuoted("[", Traits::FieldFullName(field), "]"), ":");
  } else if (writer.options().preserve_proto_field_names) {
    writer.Write(MakeQuoted(Traits::FieldName(field)), ":");
  } else {
    // The generator for type.proto and the internals of descriptor.cc disagree
    // on what the json name of a PascalCase field is supposed to be; type.proto
    // seems to (incorrectly?) capitalize the first letter, which is the
    // behavior ESF defaults to. To fix this, if the original field name starts
    // with an uppercase letter, and the Json name does not, we uppercase it.
    absl::string_view original_name = Traits::FieldName(field);
    absl::string_view json_name = Traits::FieldJsonName(field);
    if (writer.options().allow_legacy_syntax &&
        absl::ascii_isupper(original_name[0]) &&
        !absl::ascii_isupper(json_name[0])) {
      writer.Write(MakeQuoted(absl::ascii_toupper(original_name[0]),
                              original_name.substr(1)),
                   ":");
    } else {
      writer.Write(MakeQuoted(json_name), ":");
    }
  }
  writer.Whitespace(" ");

  if (Traits::IsMap(field)) {
    return WriteMap<Traits>(writer, msg, field);
  } else if (Traits::IsRepeated(field)) {
    return WriteRepeated<Traits>(writer, msg, field);
  } else if (Traits::GetSize(field, msg) == 0) {
    // We can only get here if always_print_primitive_fields is true.
    ABSL_DCHECK(writer.options().always_print_primitive_fields);

    if (Traits::FieldType(field) == FieldDescriptor::TYPE_GROUP) {
      // We do not yet have full group support, but this is required so that we
      // pass the same tests as the ESF parser.
      writer.Write("null");
      return absl::OkStatus();
    }
    return WriteSingular<Traits>(writer, field);
  }

  return WriteSingular<Traits>(writer, field, msg);
}

template <typename Traits>
absl::Status WriteFields(JsonWriter& writer, const Msg<Traits>& msg,
                         const Desc<Traits>& desc, bool& first) {
  std::vector<Field<Traits>> fields;
  size_t total = Traits::FieldCount(desc);
  fields.reserve(total);
  for (size_t i = 0; i < total; ++i) {
    Field<Traits> field = Traits::FieldByIndex(desc, i);

    bool has = Traits::GetSize(field, msg) > 0;
    if (writer.options().always_print_primitive_fields) {
      bool is_singular_message =
          !Traits::IsRepeated(field) &&
          Traits::FieldType(field) == FieldDescriptor::TYPE_MESSAGE;
      has |= !is_singular_message && !Traits::IsOneof(field);
    }

    if (has) {
      fields.push_back(field);
    }
  }

  // Add extensions *before* sorting.
  Traits::FindAndAppendExtensions(msg, fields);

  // Fields are guaranteed to be serialized in field number order.
  absl::c_sort(fields, [](const auto& a, const auto& b) {
    return Traits::FieldNumber(a) < Traits::FieldNumber(b);
  });

  for (auto field : fields) {
    RETURN_IF_ERROR(WriteField<Traits>(writer, msg, field, first));
  }

  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteStructValue(JsonWriter& writer, const Msg<Traits>& msg,
                              const Desc<Traits>& desc);
template <typename Traits>
absl::Status WriteListValue(JsonWriter& writer, const Msg<Traits>& msg,
                            const Desc<Traits>& desc);

template <typename Traits>
absl::Status WriteValue(JsonWriter& writer, const Msg<Traits>& msg,
                        const Desc<Traits>& desc, bool is_top_level) {
  // NOTE: The field numbers 1 through 6 are the numbers of the oneof fields in
  // google.protobuf.Value. Conformance tests verify the correctness of these
  // numbers.
  if (Traits::GetSize(Traits::MustHaveField(desc, 1), msg) > 0) {
    writer.Write("null");
    return absl::OkStatus();
  }

  auto number_field = Traits::MustHaveField(desc, 2);
  if (Traits::GetSize(number_field, msg) > 0) {
    auto x = Traits::GetDouble(number_field, msg);
    RETURN_IF_ERROR(x.status());
    if (std::isnan(*x)) {
      return absl::InvalidArgumentError(
          "google.protobuf.Value cannot encode double values for nan, "
          "because it would be parsed as a string");
    }
    if (*x == std::numeric_limits<double>::infinity() ||
        *x == -std::numeric_limits<double>::infinity()) {
      return absl::InvalidArgumentError(
          "google.protobuf.Value cannot encode double values for "
          "infinity, because it would be parsed as a string");
    }
    writer.Write(*x);
    return absl::OkStatus();
  }

  auto string_field = Traits::MustHaveField(desc, 3);
  if (Traits::GetSize(string_field, msg) > 0) {
    auto x = Traits::GetString(string_field, writer.ScratchBuf(), msg);
    RETURN_IF_ERROR(x.status());
    writer.Write(MakeQuoted(*x));
    return absl::OkStatus();
  }

  auto bool_field = Traits::MustHaveField(desc, 4);
  if (Traits::GetSize(bool_field, msg) > 0) {
    auto x = Traits::GetBool(bool_field, msg);
    RETURN_IF_ERROR(x.status());
    writer.Write(*x ? "true" : "false");
    return absl::OkStatus();
  }

  auto struct_field = Traits::MustHaveField(desc, 5);
  if (Traits::GetSize(struct_field, msg) > 0) {
    auto x = Traits::GetMessage(struct_field, msg);
    RETURN_IF_ERROR(x.status());
    return Traits::WithFieldType(struct_field, [&](const Desc<Traits>& type) {
      return WriteStructValue<Traits>(writer, **x, type);
    });
  }

  auto list_field = Traits::MustHaveField(desc, 6);
  if (Traits::GetSize(list_field, msg) > 0) {
    auto x = Traits::GetMessage(list_field, msg);
    RETURN_IF_ERROR(x.status());
    return Traits::WithFieldType(list_field, [&](const Desc<Traits>& type) {
      return WriteListValue<Traits>(writer, **x, type);
    });
  }

  ABSL_CHECK(is_top_level)
      << "empty, non-top-level Value must be handled one layer "
         "up, since it prints an empty string; reaching this "
         "statement is always a bug";
  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteStructValue(JsonWriter& writer, const Msg<Traits>& msg,
                              const Desc<Traits>& desc) {
  return WriteMap<Traits>(writer, msg, Traits::MustHaveField(desc, 1));
}

template <typename Traits>
absl::Status WriteListValue(JsonWriter& writer, const Msg<Traits>& msg,
                            const Desc<Traits>& desc) {
  return WriteRepeated<Traits>(writer, msg, Traits::MustHaveField(desc, 1));
}

template <typename Traits>
absl::Status WriteTimestamp(JsonWriter& writer, const Msg<Traits>& msg,
                            const Desc<Traits>& desc) {
  auto secs_field = Traits::MustHaveField(desc, 1);
  auto secs = Traits::GetSize(secs_field, msg) > 0
                  ? Traits::GetInt64(secs_field, msg)
                  : 0;
  RETURN_IF_ERROR(secs.status());

  if (*secs < -62135596800) {
    return absl::InvalidArgumentError(
        "minimum acceptable time value is 0001-01-01T00:00:00Z");
  } else if (*secs > 253402300799) {
    return absl::InvalidArgumentError(
        "maximum acceptable time value is 9999-12-31T23:59:59Z");
  }

  // Ensure seconds is positive.
  *secs += 62135596800;

  auto nanos_field = Traits::MustHaveField(desc, 2);
  auto nanos = Traits::GetSize(nanos_field, msg) > 0
                   ? Traits::GetInt32(nanos_field, msg)
                   : 0;
  RETURN_IF_ERROR(nanos.status());

  // Julian Day -> Y/M/D, Algorithm from:
  // Fliegel, H. F., and Van Flandern, T. C., "A Machine Algorithm for
  //   Processing Calendar Dates," Communications of the Association of
  //   Computing Machines, vol. 11 (1968), p. 657.
  int32_t L, N, I, J, K;
  L = static_cast<int32_t>(*secs / 86400) - 719162 + 68569 + 2440588;
  N = 4 * L / 146097;
  L = L - (146097 * N + 3) / 4;
  I = 4000 * (L + 1) / 1461001;
  L = L - 1461 * I / 4 + 31;
  J = 80 * L / 2447;
  K = L - 2447 * J / 80;
  L = J / 11;
  J = J + 2 - 12 * L;
  I = 100 * (N - 49) + I + L;

  int32_t sec = *secs % 60;
  int32_t min = (*secs / 60) % 60;
  int32_t hour = (*secs / 3600) % 24;

  if (*nanos == 0) {
    writer.Write(absl::StrFormat(R"("%04d-%02d-%02dT%02d:%02d:%02dZ")", I, J, K,
                                 hour, min, sec));
    return absl::OkStatus();
  }

  size_t digits = 9;
  uint32_t frac_seconds = std::abs(*nanos);
  while (frac_seconds % 1000 == 0) {
    frac_seconds /= 1000;
    digits -= 3;
  }

  writer.Write(absl::StrFormat(R"("%04d-%02d-%02dT%02d:%02d:%02d.%.*dZ")", I, J,
                               K, hour, min, sec, digits, frac_seconds));
  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteDuration(JsonWriter& writer, const Msg<Traits>& msg,
                           const Desc<Traits>& desc) {
  constexpr int64_t kMaxSeconds = int64_t{3652500} * 86400;
  constexpr int64_t kMaxNanos = 999999999;

  auto secs_field = Traits::MustHaveField(desc, 1);
  auto secs = Traits::GetSize(secs_field, msg) > 0
                  ? Traits::GetInt64(secs_field, msg)
                  : 0;
  RETURN_IF_ERROR(secs.status());

  if (*secs > kMaxSeconds || *secs < -kMaxSeconds) {
    return absl::InvalidArgumentError("duration out of range");
  }

  auto nanos_field = Traits::MustHaveField(desc, 2);
  auto nanos = Traits::GetSize(nanos_field, msg) > 0
                   ? Traits::GetInt32(nanos_field, msg)
                   : 0;
  RETURN_IF_ERROR(nanos.status());

  if (*nanos > kMaxNanos || *nanos < -kMaxNanos) {
    return absl::InvalidArgumentError("duration out of range");
  }
  if ((*secs != 0) && (*nanos != 0) && ((*secs < 0) != (*nanos < 0))) {
    return absl::InvalidArgumentError("nanos and seconds signs do not match");
  }

  if (*nanos == 0) {
    writer.Write(absl::StrFormat(R"("%ds")", *secs));
    return absl::OkStatus();
  }

  size_t digits = 9;
  uint32_t frac_seconds = std::abs(*nanos);
  while (frac_seconds % 1000 == 0) {
    frac_seconds /= 1000;
    digits -= 3;
  }

  absl::string_view sign = ((*secs < 0) || (*nanos < 0)) ? "-" : "";
  writer.Write(absl::StrFormat(R"("%s%d.%.*ds")", sign, std::abs(*secs), digits,
                               frac_seconds));
  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteFieldMask(JsonWriter& writer, const Msg<Traits>& msg,
                            const Desc<Traits>& desc) {
  // google.protobuf.FieldMask has a single field with number 1.
  auto paths_field = Traits::MustHaveField(desc, 1);
  size_t paths = Traits::GetSize(paths_field, msg);
  writer.Write('"');

  bool first = true;
  for (size_t i = 0; i < paths; ++i) {
    writer.WriteComma(first);
    auto path = Traits::GetString(paths_field, writer.ScratchBuf(), msg, i);
    RETURN_IF_ERROR(path.status());
    bool saw_under = false;
    for (char c : *path) {
      if (absl::ascii_islower(c) && saw_under) {
        writer.Write(absl::ascii_toupper(c));
      } else if (absl::ascii_isdigit(c) || absl::ascii_islower(c) || c == '.') {
        writer.Write(c);
      } else if (c == '_' &&
                 (!saw_under || writer.options().allow_legacy_syntax)) {
        saw_under = true;
        continue;
      } else if (!writer.options().allow_legacy_syntax) {
        return absl::InvalidArgumentError("unexpected character in FieldMask");
      } else {
        if (saw_under) {
          writer.Write('_');
        }
        writer.Write(c);
      }
      saw_under = false;
    }
  }
  writer.Write('"');

  return absl::OkStatus();
}

template <typename Traits>
absl::Status WriteAny(JsonWriter& writer, const Msg<Traits>& msg,
                      const Desc<Traits>& desc) {
  auto type_url_field = Traits::MustHaveField(desc, 1);
  auto value_field = Traits::MustHaveField(desc, 2);

  bool has_type_url = Traits::GetSize(type_url_field, msg) > 0;
  bool has_value = Traits::GetSize(value_field, msg) > 0;
  if (!has_type_url && !has_value) {
    writer.Write("{}");
    return absl::OkStatus();
  } else if (!has_type_url) {
    return absl::InvalidArgumentError("broken Any: missing type URL");
  } else if (!has_value && !writer.options().allow_legacy_syntax) {
    return absl::InvalidArgumentError("broken Any: missing value");
  }

  writer.Write("{");
  writer.Push();

  auto type_url = Traits::GetString(type_url_field, writer.ScratchBuf(), msg);
  RETURN_IF_ERROR(type_url.status());
  writer.NewLine();
  writer.Write("\"@type\":");
  writer.Whitespace(" ");
  writer.Write(MakeQuoted(*type_url));

  return Traits::WithDynamicType(
      desc, std::string(*type_url),
      [&](const Desc<Traits>& any_desc) -> absl::Status {
        absl::string_view any_bytes;
        if (has_value) {
          absl::StatusOr<absl::string_view> bytes =
              Traits::GetString(value_field, writer.ScratchBuf(), msg);
          RETURN_IF_ERROR(bytes.status());
          any_bytes = *bytes;
        }

        return Traits::WithDecodedMessage(
            any_desc, any_bytes,
            [&](const Msg<Traits>& unerased) -> absl::Status {
              bool first = false;
              if (ClassifyMessage(Traits::TypeName(any_desc)) !=
                  MessageType::kNotWellKnown) {
                writer.WriteComma(first);
                writer.NewLine();
                writer.Write("\"value\":");
                writer.Whitespace(" ");
                RETURN_IF_ERROR(
                    WriteMessage<Traits>(writer, unerased, any_desc));
              } else {
                RETURN_IF_ERROR(
                    WriteFields<Traits>(writer, unerased, any_desc, first));
              }
              writer.Pop();
              if (!first) {
                writer.NewLine();
              }
              writer.Write("}");
              return absl::OkStatus();
            });
      });
}

template <typename Traits>
absl::Status WriteMessage(JsonWriter& writer, const Msg<Traits>& msg,
                          const Desc<Traits>& desc, bool is_top_level) {
  switch (ClassifyMessage(Traits::TypeName(desc))) {
    case MessageType::kAny:
      return WriteAny<Traits>(writer, msg, desc);
    case MessageType::kWrapper: {
      auto field = Traits::MustHaveField(desc, 1);
      if (Traits::GetSize(field, msg) == 0) {
        return WriteSingular<Traits>(writer, field);
      }
      return WriteSingular<Traits>(writer, field, msg);
    }
    case MessageType::kValue:
      return WriteValue<Traits>(writer, msg, desc, is_top_level);
    case MessageType::kStruct:
      return WriteStructValue<Traits>(writer, msg, desc);
    case MessageType::kList:
      return WriteListValue<Traits>(writer, msg, desc);
    case MessageType::kTimestamp:
      return WriteTimestamp<Traits>(writer, msg, desc);
    case MessageType::kDuration:
      return WriteDuration<Traits>(writer, msg, desc);
    case MessageType::kFieldMask:
      return WriteFieldMask<Traits>(writer, msg, desc);
    default: {
      writer.Write("{");
      writer.Push();
      bool first = true;
      RETURN_IF_ERROR(WriteFields<Traits>(writer, msg, desc, first));
      writer.Pop();
      if (!first) {
        writer.NewLine();
      }
      writer.Write("}");
      return absl::OkStatus();
    }
  }
}
}  // namespace

absl::Status MessageToJsonString(const Message& message, std::string* output,
                                 json_internal::WriterOptions options) {
  if (PROTOBUF_DEBUG) {
    ABSL_DLOG(INFO) << "json2/input: " << message.DebugString();
  }
  io::StringOutputStream out(output);
  JsonWriter writer(&out, options);
  absl::Status s = WriteMessage<UnparseProto2Descriptor>(
      writer, message, *message.GetDescriptor(), /*is_top_level=*/true);
  if (PROTOBUF_DEBUG) ABSL_DLOG(INFO) << "json2/status: " << s;
  RETURN_IF_ERROR(s);

  writer.NewLine();
  if (PROTOBUF_DEBUG) {
    ABSL_DLOG(INFO) << "json2/output: " << absl::CHexEscape(*output);
  }
  return absl::OkStatus();
}

absl::Status BinaryToJsonStream(google::protobuf::util::TypeResolver* resolver,
                                const std::string& type_url,
                                io::ZeroCopyInputStream* binary_input,
                                io::ZeroCopyOutputStream* json_output,
                                json_internal::WriterOptions options) {
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
    while (binary_input->Next(&data, &len)) {
      copy.resize(copy.size() + len);
      std::memcpy(&copy[copy.size() - len], data, len);
    }
    tee_input.emplace(copy.data(), copy.size());
    tee_output.emplace(&out);
    ABSL_DLOG(INFO) << "json2/input: " << absl::BytesToHexString(copy);
  }

  ResolverPool pool(resolver);
  auto desc = pool.FindMessage(type_url);
  RETURN_IF_ERROR(desc.status());

  io::CodedInputStream stream(tee_input.has_value() ? &*tee_input
                                                    : binary_input);
  auto msg = UntypedMessage::ParseFromStream(*desc, stream);
  RETURN_IF_ERROR(msg.status());

  JsonWriter writer(tee_output.has_value() ? &*tee_output : json_output,
                    options);
  absl::Status s = WriteMessage<UnparseProto3Type>(
      writer, *msg, UnparseProto3Type::GetDesc(*msg),
      /*is_top_level=*/true);
  if (PROTOBUF_DEBUG) ABSL_DLOG(INFO) << "json2/status: " << s;
  RETURN_IF_ERROR(s);

  if (PROTOBUF_DEBUG) {
    tee_output.reset();  // Flush the output stream.
    io::zc_sink_internal::ZeroCopyStreamByteSink(json_output)
        .Append(out.data(), out.size());
    ABSL_DLOG(INFO) << "json2/output: " << absl::CHexEscape(out);
  }

  writer.NewLine();
  return absl::OkStatus();
}
}  // namespace json_internal
}  // namespace protobuf
}  // namespace google

