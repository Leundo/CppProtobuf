// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <type_traits>
#include <utility>

#include <CppAbseil/absl_base_optimization.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_numeric_bits.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include "google_protobuf_generated_message_tctable_decl.hpp"
#include "google_protobuf_generated_message_tctable_impl.hpp"
#include "google_protobuf_inlined_string_field.hpp"
#include "google_protobuf_io_zero_copy_stream_impl_lite.hpp"
#include "google_protobuf_map.hpp"
#include "google_protobuf_message_lite.hpp"
#include "google_protobuf_parse_context.hpp"
#include "google_protobuf_repeated_field.hpp"
#include "google_protobuf_repeated_ptr_field.hpp"
#include "google_protobuf_varint_shuffle.hpp"
#include "google_protobuf_wire_format_lite.hpp"
#include "utf8range_utf8_validity.hpp"


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

namespace google {
namespace protobuf {
namespace internal {

using FieldEntry = TcParseTableBase::FieldEntry;

//////////////////////////////////////////////////////////////////////////////
// Template instantiations:
//////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
void AlignFail(std::integral_constant<size_t, 4>, std::uintptr_t address) {
  ABSL_LOG(FATAL) << "Unaligned (4) access at " << address;

  // Explicit abort to let compilers know this function does not return
  abort();
}
void AlignFail(std::integral_constant<size_t, 8>, std::uintptr_t address) {
  ABSL_LOG(FATAL) << "Unaligned (8) access at " << address;

  // Explicit abort to let compilers know this function does not return
  abort();
}
#endif

const char* TcParser::GenericFallbackLite(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return GenericFallbackImpl<MessageLite, std::string>(
      PROTOBUF_TC_PARAM_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// Core fast parsing implementation:
//////////////////////////////////////////////////////////////////////////////

PROTOBUF_NOINLINE const char* TcParser::ParseLoop(
    MessageLite* msg, const char* ptr, ParseContext* ctx,
    const TcParseTableBase* table) {
  // Note: TagDispatch uses a dispatch table at "&table->fast_entries".
  // For fast dispatch, we'd like to have a pointer to that, but if we use
  // that expression, there's no easy way to get back to "table", which we also
  // need during dispatch.  It turns out that "table + 1" points exactly to
  // fast_entries, so we just increment table by 1 here, to get the register
  // holding the value we want.
  table += 1;
  while (!ctx->Done(&ptr)) {
#if defined(__GNUC__)
    // Note: this asm prevents the compiler (clang, specifically) from
    // believing (thanks to CSE) that it needs to dedicate a registeer both
    // to "table" and "&table->fast_entries".
    // TODO: remove this asm
    asm("" : "+r"(table));
#endif
    ptr = TagDispatch(msg, ptr, ctx, TcFieldData::DefaultInit(), table - 1, 0);
    if (ptr == nullptr) break;
    if (ctx->LastTag() != 1) break;  // Ended on terminating tag
  }
  return ptr;
}

// On the fast path, a (matching) 1-byte tag already has the decoded value.
static uint32_t FastDecodeTag(uint8_t coded_tag) {
  return coded_tag;
}

// On the fast path, a (matching) 2-byte tag always needs to be decoded.
static uint32_t FastDecodeTag(uint16_t coded_tag) {
  uint32_t result = coded_tag;
  result += static_cast<int8_t>(coded_tag);
  return result >> 1;
}

//////////////////////////////////////////////////////////////////////////////
// Core mini parsing implementation:
//////////////////////////////////////////////////////////////////////////////

// Field lookup table layout:
//
// Because it consists of a series of variable-length segments, the lookuup
// table is organized within an array of uint16_t, and each element is either
// a uint16_t or a uint32_t stored little-endian as a pair of uint16_t.
//
// Its fundamental building block maps 16 contiguously ascending field numbers
// to their locations within the field entry table:

struct SkipEntry16 {
  uint16_t skipmap;
  uint16_t field_entry_offset;
};

// The skipmap is a bitfield of which of those field numbers do NOT have a
// field entry.  The lowest bit of the skipmap corresponds to the lowest of
// the 16 field numbers, so if a proto had only fields 1, 2, 3, and 7, the
// skipmap would contain 0b11111111'10111000.
//
// The field lookup table begins with a single 32-bit skipmap that maps the
// field numbers 1 through 32.  This is because the majority of proto
// messages only contain fields numbered 1 to 32.
//
// The rest of the lookup table is a repeated series of
// { 32-bit field #,  #SkipEntry16s,  {SkipEntry16...} }
// That is, the next thing is a pair of uint16_t that form the next
// lowest field number that the lookup table handles.  If this number is -1,
// that is the end of the table.  Then there is a uint16_t that is
// the number of contiguous SkipEntry16 entries that follow, and then of
// course the SkipEntry16s themselves.

// Originally developed and tested at https://godbolt.org/z/vbc7enYcf

// Returns the address of the field for `tag` in the table's field entries.
// Returns nullptr if the field was not found.
const TcParseTableBase::FieldEntry* TcParser::FindFieldEntry(
    const TcParseTableBase* table, uint32_t field_num) {
  const FieldEntry* const field_entries = table->field_entries_begin();

  uint32_t fstart = 1;
  uint32_t adj_fnum = field_num - fstart;

  if (PROTOBUF_PREDICT_TRUE(adj_fnum < 32)) {
    uint32_t skipmap = table->skipmap32;
    uint32_t skipbit = 1 << adj_fnum;
    if (PROTOBUF_PREDICT_FALSE(skipmap & skipbit)) return nullptr;
    skipmap &= skipbit - 1;
    adj_fnum -= absl::popcount(skipmap);
    auto* entry = field_entries + adj_fnum;
    PROTOBUF_ASSUME(entry != nullptr);
    return entry;
  }
  const uint16_t* lookup_table = table->field_lookup_begin();
  for (;;) {
#ifdef PROTOBUF_LITTLE_ENDIAN
    memcpy(&fstart, lookup_table, sizeof(fstart));
#else
    fstart = lookup_table[0] | (lookup_table[1] << 16);
#endif
    lookup_table += sizeof(fstart) / sizeof(*lookup_table);
    uint32_t num_skip_entries = *lookup_table++;
    if (field_num < fstart) return nullptr;
    adj_fnum = field_num - fstart;
    uint32_t skip_num = adj_fnum / 16;
    if (PROTOBUF_PREDICT_TRUE(skip_num < num_skip_entries)) {
      // for each group of 16 fields we have:
      // a bitmap of 16 bits
      // a 16-bit field-entry offset for the first of them.
      auto* skip_data = lookup_table + (adj_fnum / 16) * (sizeof(SkipEntry16) /
                                                          sizeof(uint16_t));
      SkipEntry16 se = {skip_data[0], skip_data[1]};
      adj_fnum &= 15;
      uint32_t skipmap = se.skipmap;
      uint16_t skipbit = 1 << adj_fnum;
      if (PROTOBUF_PREDICT_FALSE(skipmap & skipbit)) return nullptr;
      skipmap &= skipbit - 1;
      adj_fnum += se.field_entry_offset;
      adj_fnum -= absl::popcount(skipmap);
      auto* entry = field_entries + adj_fnum;
      PROTOBUF_ASSUME(entry != nullptr);
      return entry;
    }
    lookup_table +=
        num_skip_entries * (sizeof(SkipEntry16) / sizeof(*lookup_table));
  }
}

// Field names are stored in a format of:
//
// 1) A table of name sizes, one byte each, from 1 to 255 per name.
//    `entries` is the size of this first table.
// 1a) padding bytes, so the table of name sizes is a multiple of
//     eight bytes in length. They are zero.
//
// 2) All the names, concatenated, with neither separation nor termination.
//
// This is designed to be compact but not particularly fast to retrieve.
// In particular, it takes O(n) to retrieve the name of the n'th field,
// which is usually fine because most protos have fewer than 10 fields.
static absl::string_view FindName(const char* name_data, size_t entries,
                                  size_t index) {
  // The compiler unrolls these... if this isn't fast enough,
  // there's an AVX version at https://godbolt.org/z/eojrjqzfr
  // ARM-compatible version at https://godbolt.org/z/n5YT5Ee85

  // The field name sizes are padded up to a multiple of 8, so we
  // must pad them here.
  size_t num_sizes = (entries + 7) & -8;
  auto* uint8s = reinterpret_cast<const uint8_t*>(name_data);
  size_t pos = std::accumulate(uint8s, uint8s + index, num_sizes);
  size_t size = name_data[index];
  auto* start = &name_data[pos];
  return {start, size};
}

absl::string_view TcParser::MessageName(const TcParseTableBase* table) {
  return FindName(table->name_data(), table->num_field_entries + 1, 0);
}

absl::string_view TcParser::FieldName(const TcParseTableBase* table,
                                      const FieldEntry* field_entry) {
  const FieldEntry* const field_entries = table->field_entries_begin();
  auto field_index = static_cast<size_t>(field_entry - field_entries);
  return FindName(table->name_data(), table->num_field_entries + 1,
                  field_index + 1);
}

PROTOBUF_NOINLINE const char* TcParser::Error(PROTOBUF_TC_PARAM_NO_DATA_DECL) {
  (void)ctx;
  (void)ptr;
  SyncHasbits(msg, hasbits, table);
  return nullptr;
}

template <bool export_called_function>
inline PROTOBUF_ALWAYS_INLINE const char* TcParser::MiniParse(
    PROTOBUF_TC_PARAM_DECL) {
  TestMiniParseResult* test_out;
  if (export_called_function) {
    test_out = reinterpret_cast<TestMiniParseResult*>(
        static_cast<uintptr_t>(data.data));
  }

  uint32_t tag;
  ptr = ReadTagInlined(ptr, &tag);
  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    if (export_called_function) *test_out = {Error};
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }

  auto* entry = FindFieldEntry(table, tag >> 3);
  if (entry == nullptr) {
    if (export_called_function) *test_out = {table->fallback, tag};
    data.data = tag;
    PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
  }

  // The handler may need the tag and the entry to resolve fallback logic. Both
  // of these are 32 bits, so pack them into (the 64-bit) `data`. Since we can't
  // pack the entry pointer itself, just pack its offset from `table`.
  uint64_t entry_offset = reinterpret_cast<const char*>(entry) -
                          reinterpret_cast<const char*>(table);
  data.data = entry_offset << 32 | tag;

  using field_layout::FieldKind;
  auto field_type =
      entry->type_card & (+field_layout::kSplitMask | FieldKind::kFkMask);

  static constexpr TailCallParseFunc kMiniParseTable[] = {
      &MpFallback,             // FieldKind::kFkNone
      &MpVarint<false>,        // FieldKind::kFkVarint
      &MpPackedVarint<false>,  // FieldKind::kFkPackedVarint
      &MpFixed<false>,         // FieldKind::kFkFixed
      &MpPackedFixed<false>,   // FieldKind::kFkPackedFixed
      &MpString<false>,        // FieldKind::kFkString
      &MpMessage<false>,       // FieldKind::kFkMessage
      &MpMap<false>,           // FieldKind::kFkMap
      &Error,                  // kSplitMask | FieldKind::kFkNone
      &MpVarint<true>,         // kSplitMask | FieldKind::kFkVarint
      &MpPackedVarint<true>,   // kSplitMask | FieldKind::kFkPackedVarint
      &MpFixed<true>,          // kSplitMask | FieldKind::kFkFixed
      &MpPackedFixed<true>,    // kSplitMask | FieldKind::kFkPackedFixed
      &MpString<true>,         // kSplitMask | FieldKind::kFkString
      &MpMessage<true>,        // kSplitMask | FieldKind::kFkMessage
      &MpMap<true>,            // kSplitMask | FieldKind::kFkMap
  };
  // Just to be sure we got the order right, above.
  static_assert(0 == FieldKind::kFkNone, "Invalid table order");
  static_assert(1 == FieldKind::kFkVarint, "Invalid table order");
  static_assert(2 == FieldKind::kFkPackedVarint, "Invalid table order");
  static_assert(3 == FieldKind::kFkFixed, "Invalid table order");
  static_assert(4 == FieldKind::kFkPackedFixed, "Invalid table order");
  static_assert(5 == FieldKind::kFkString, "Invalid table order");
  static_assert(6 == FieldKind::kFkMessage, "Invalid table order");
  static_assert(7 == FieldKind::kFkMap, "Invalid table order");

  static_assert(8 == (+field_layout::kSplitMask | FieldKind::kFkNone),
    "Invalid table order");
  static_assert(9 == (+field_layout::kSplitMask | FieldKind::kFkVarint),
    "Invalid table order");
  static_assert(10 == (+field_layout::kSplitMask | FieldKind::kFkPackedVarint),
    "Invalid table order");
  static_assert(11 == (+field_layout::kSplitMask | FieldKind::kFkFixed),
    "Invalid table order");
  static_assert(12 == (+field_layout::kSplitMask | FieldKind::kFkPackedFixed),
    "Invalid table order");
  static_assert(13 == (+field_layout::kSplitMask | FieldKind::kFkString),
    "Invalid table order");
  static_assert(14 == (+field_layout::kSplitMask | FieldKind::kFkMessage),
    "Invalid table order");
  static_assert(15 == (+field_layout::kSplitMask | FieldKind::kFkMap),
    "Invalid table order");

  TailCallParseFunc parse_fn = kMiniParseTable[field_type];
  if (export_called_function) *test_out = {parse_fn, tag, entry};

  PROTOBUF_MUSTTAIL return parse_fn(PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::MiniParse(
    PROTOBUF_TC_PARAM_NO_DATA_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse<false>(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
PROTOBUF_NOINLINE TcParser::TestMiniParseResult TcParser::TestMiniParse(
    PROTOBUF_TC_PARAM_DECL) {
  TestMiniParseResult result = {};
  data.data = reinterpret_cast<uintptr_t>(&result);
  result.ptr = MiniParse<true>(PROTOBUF_TC_PARAM_PASS);
  return result;
}

PROTOBUF_NOINLINE const char* TcParser::MpFallback(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
}

template <typename TagType>
const char* TcParser::FastEndGroupImpl(PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  ctx->SetLastTag(data.decoded_tag());
  ptr += sizeof(TagType);
  PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEndG1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return FastEndGroupImpl<uint8_t>(PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEndG2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return FastEndGroupImpl<uint16_t>(PROTOBUF_TC_PARAM_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// Message fields
//////////////////////////////////////////////////////////////////////////////

template <typename TagType, bool group_coding, bool aux_is_table>
inline PROTOBUF_ALWAYS_INLINE const char* TcParser::SingularParseMessageAuxImpl(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  auto saved_tag = UnalignedLoad<TagType>(ptr);
  ptr += sizeof(TagType);
  hasbits |= (uint64_t{1} << data.hasbit_idx());
  SyncHasbits(msg, hasbits, table);
  auto& field = RefAt<MessageLite*>(msg, data.offset());

  if (aux_is_table) {
    const auto* inner_table = table->field_aux(data.aux_idx())->table;
    if (field == nullptr) {
      field = inner_table->default_instance->New(msg->GetArena());
    }
    if (group_coding) {
      return ctx->ParseGroup<TcParser>(field, ptr, FastDecodeTag(saved_tag),
                                       inner_table);
    }
    return ctx->ParseMessage<TcParser>(field, ptr, inner_table);
  } else {
    if (field == nullptr) {
      const MessageLite* default_instance =
          table->field_aux(data.aux_idx())->message_default();
      field = default_instance->New(msg->GetArena());
    }
    if (group_coding) {
      return ctx->ParseGroup(field, ptr, FastDecodeTag(saved_tag));
    }
    return ctx->ParseMessage(field, ptr);
  }
}

PROTOBUF_NOINLINE const char* TcParser::FastMdS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint8_t, false, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMdS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint16_t, false, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGdS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint8_t, true, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGdS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint16_t, true, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMtS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint8_t, false, true>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMtS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint16_t, false, true>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGtS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint8_t, true, true>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGtS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularParseMessageAuxImpl<uint16_t, true, true>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename TagType>
const char* TcParser::LazyMessage(PROTOBUF_TC_PARAM_DECL) {
  ABSL_LOG(FATAL) << "Unimplemented";
  return nullptr;
}

PROTOBUF_NOINLINE const char* TcParser::FastMlS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return LazyMessage<uint8_t>(PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMlS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return LazyMessage<uint16_t>(PROTOBUF_TC_PARAM_PASS);
}

template <typename TagType, bool group_coding, bool aux_is_table>
inline PROTOBUF_ALWAYS_INLINE const char* TcParser::RepeatedParseMessageAuxImpl(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  const auto expected_tag = UnalignedLoad<TagType>(ptr);
  const auto aux = *table->field_aux(data.aux_idx());
  auto& field = RefAt<RepeatedPtrFieldBase>(msg, data.offset());
  const MessageLite* const default_instance =
      aux_is_table ? aux.table->default_instance : aux.message_default();
  do {
    ptr += sizeof(TagType);
    MessageLite* submsg =
        field.Add<GenericTypeHandler<MessageLite>>(default_instance);
    if (aux_is_table) {
      if (group_coding) {
        ptr = ctx->ParseGroup<TcParser>(submsg, ptr,
                                        FastDecodeTag(expected_tag), aux.table);
      } else {
        ptr = ctx->ParseMessage<TcParser>(submsg, ptr, aux.table);
      }
    } else {
      if (group_coding) {
        ptr = ctx->ParseGroup(submsg, ptr, FastDecodeTag(expected_tag));
      } else {
        ptr = ctx->ParseMessage(submsg, ptr);
      }
    }
    if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
      PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
    if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) {
      PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
  } while (UnalignedLoad<TagType>(ptr) == expected_tag);

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMdR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint8_t, false, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMdR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint16_t, false, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGdR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint8_t, true, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGdR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint16_t, true, false>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMtR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint8_t, false, true>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastMtR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint16_t, false, true>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGtR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint8_t, true, true>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastGtR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedParseMessageAuxImpl<uint16_t, true, true>(
      PROTOBUF_TC_PARAM_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// Fixed fields
//////////////////////////////////////////////////////////////////////////////

template <typename LayoutType, typename TagType>
PROTOBUF_ALWAYS_INLINE const char* TcParser::SingularFixed(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  ptr += sizeof(TagType);  // Consume tag
  hasbits |= (uint64_t{1} << data.hasbit_idx());
  RefAt<LayoutType>(msg, data.offset()) = UnalignedLoad<LayoutType>(ptr);
  ptr += sizeof(LayoutType);
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastF32S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularFixed<uint32_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF32S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularFixed<uint32_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF64S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularFixed<uint64_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF64S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularFixed<uint64_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename LayoutType, typename TagType>
PROTOBUF_ALWAYS_INLINE const char* TcParser::RepeatedFixed(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  auto& field = RefAt<RepeatedField<LayoutType>>(msg, data.offset());
  const auto tag = UnalignedLoad<TagType>(ptr);
  do {
    field.Add(UnalignedLoad<LayoutType>(ptr + sizeof(TagType)));
    ptr += sizeof(TagType) + sizeof(LayoutType);
    if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) {
      PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
  } while (UnalignedLoad<TagType>(ptr) == tag);
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastF32R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedFixed<uint32_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF32R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedFixed<uint32_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF64R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedFixed<uint64_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF64R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedFixed<uint64_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename LayoutType, typename TagType>
PROTOBUF_ALWAYS_INLINE const char* TcParser::PackedFixed(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  ptr += sizeof(TagType);
  // Since ctx->ReadPackedFixed does not use TailCall<> or Return<>, sync any
  // pending hasbits now:
  SyncHasbits(msg, hasbits, table);
  auto& field = RefAt<RepeatedField<LayoutType>>(msg, data.offset());
  int size = ReadSize(&ptr);
  // TODO: add a tailcalling variant of ReadPackedFixed.
  return ctx->ReadPackedFixed(ptr, size,
                              static_cast<RepeatedField<LayoutType>*>(&field));
}

PROTOBUF_NOINLINE const char* TcParser::FastF32P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedFixed<uint32_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF32P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedFixed<uint32_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF64P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedFixed<uint64_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastF64P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedFixed<uint64_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// Varint fields
//////////////////////////////////////////////////////////////////////////////

namespace {

template <typename Type>
inline PROTOBUF_ALWAYS_INLINE const char* ParseVarint(const char* p,
                                                      Type* value) {
  static_assert(sizeof(Type) == 4 || sizeof(Type) == 8,
                "Only [u]int32_t and [u]int64_t please");
#ifdef __aarch64__
  // The VarintParse parser has a faster implementation on ARM.
  absl::conditional_t<sizeof(Type) == 4, uint32_t, uint64_t> tmp;
  p = VarintParse(p, &tmp);
  if (p != nullptr) {
    *value = tmp;
  }
  return p;
#endif
  int64_t res;
  p = ShiftMixParseVarint<Type>(p, res);
  *value = res;
  return p;
}

// This overload is specifically for handling bool, because bools have very
// different requirements and performance opportunities than ints.
inline PROTOBUF_ALWAYS_INLINE const char* ParseVarint(const char* p,
                                                      bool* value) {
  unsigned char byte = static_cast<unsigned char>(*p++);
  if (PROTOBUF_PREDICT_TRUE(byte == 0 || byte == 1)) {
    // This is the code path almost always taken,
    // so we take care to make it very efficient.
    if (sizeof(byte) == sizeof(*value)) {
      memcpy(value, &byte, 1);
    } else {
      // The C++ standard does not specify that a `bool` takes only one byte
      *value = byte;
    }
    return p;
  }
  // This part, we just care about code size.
  // Although it's almost never used, we have to support it because we guarantee
  // compatibility for users who change a field from an int32 or int64 to a bool
  if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
    byte = (byte - 0x80) | *p++;
    if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
      byte = (byte - 0x80) | *p++;
      if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
        byte = (byte - 0x80) | *p++;
        if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
          byte = (byte - 0x80) | *p++;
          if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
            byte = (byte - 0x80) | *p++;
            if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
              byte = (byte - 0x80) | *p++;
              if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
                byte = (byte - 0x80) | *p++;
                if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
                  byte = (byte - 0x80) | *p++;
                  if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
                    // We only care about the continuation bit and the first bit
                    // of the 10th byte.
                    byte = (byte - 0x80) | (*p++ & 0x81);
                    if (PROTOBUF_PREDICT_FALSE(byte & 0x80)) {
                      return nullptr;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  *value = byte;
  return p;
}

template <typename FieldType, bool zigzag = false>
inline FieldType ZigZagDecodeHelper(FieldType value) {
  return static_cast<FieldType>(value);
}

template <>
inline int32_t ZigZagDecodeHelper<int32_t, true>(int32_t value) {
  return WireFormatLite::ZigZagDecode32(value);
}

template <>
inline int64_t ZigZagDecodeHelper<int64_t, true>(int64_t value) {
  return WireFormatLite::ZigZagDecode64(value);
}

// Prefetch the enum data, if necessary.
// We can issue the prefetch before we start parsing the ints.
PROTOBUF_ALWAYS_INLINE void PrefetchEnumData(uint16_t xform_val,
                                             TcParseTableBase::FieldAux aux) {
}

// When `xform_val` is a constant, we want to inline `ValidateEnum` because it
// is either dropped when not a kTvEnum, or useful when it is.
//
// When it is not a constant, we do not inline `ValidateEnum` because it bloats
// the code around it and pessimizes the non-enum and kTvRange cases which are
// way more common than the kTvEnum cases. It is also called from places that
// already have out-of-line functions (like MpVarint) so an extra out-of-line
// call to `ValidateEnum` does not affect much.
PROTOBUF_ALWAYS_INLINE bool EnumIsValidAux(int32_t val, uint16_t xform_val,
                                           TcParseTableBase::FieldAux aux) {
  if (xform_val == field_layout::kTvRange) {
    auto lo = aux.enum_range.start;
    return lo <= val && val < (lo + aux.enum_range.length);
  }
  if (PROTOBUF_BUILTIN_CONSTANT_P(xform_val)) {
    return internal::ValidateEnumInlined(val, aux.enum_data);
  } else {
    return internal::ValidateEnum(val, aux.enum_data);
  }
}

}  // namespace

template <typename FieldType, typename TagType, bool zigzag>
PROTOBUF_ALWAYS_INLINE const char* TcParser::SingularVarint(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  ptr += sizeof(TagType);  // Consume tag
  hasbits |= (uint64_t{1} << data.hasbit_idx());

  // clang isn't smart enough to be able to only conditionally save
  // registers to the stack, so we turn the integer-greater-than-128
  // case into a separate routine.
  if (PROTOBUF_PREDICT_FALSE(static_cast<int8_t>(*ptr) < 0)) {
    PROTOBUF_MUSTTAIL return SingularVarBigint<FieldType, TagType, zigzag>(
        PROTOBUF_TC_PARAM_PASS);
  }

  RefAt<FieldType>(msg, data.offset()) =
      ZigZagDecodeHelper<FieldType, zigzag>(static_cast<uint8_t>(*ptr++));
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <typename FieldType, typename TagType, bool zigzag>
PROTOBUF_NOINLINE const char* TcParser::SingularVarBigint(
    PROTOBUF_TC_PARAM_DECL) {
  // For some reason clang wants to save 5 registers to the stack here,
  // but we only need four for this code, so save the data we don't need
  // to the stack.  Happily, saving them this way uses regular store
  // instructions rather than PUSH/POP, which saves time at the cost of greater
  // code size, but for this heavily-used piece of code, that's fine.
  struct Spill {
    uint64_t field_data;
    ::google::protobuf::MessageLite* msg;
    const ::google::protobuf::internal::TcParseTableBase* table;
    uint64_t hasbits;
  };
  Spill spill = {data.data, msg, table, hasbits};
#if defined(__GNUC__)
  // This empty asm block convinces the compiler that the contents of spill may
  // have changed, and thus can't be cached in registers.  It's similar to, but
  // more optimal than, the effect of declaring it "volatile".
  asm("" : "+m"(spill));
#endif

  uint64_t tmp;
  PROTOBUF_ASSUME(static_cast<int8_t>(*ptr) < 0);
  ptr = ParseVarint(ptr, &tmp);

  data.data = spill.field_data;
  msg = spill.msg;
  table = spill.table;
  hasbits = spill.hasbits;

  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  RefAt<FieldType>(msg, data.offset()) =
      ZigZagDecodeHelper<FieldType, zigzag>(tmp);
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <typename FieldType>
PROTOBUF_ALWAYS_INLINE const char* TcParser::FastVarintS1(
    PROTOBUF_TC_PARAM_DECL) {
  using TagType = uint8_t;
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  int64_t res;
  ptr = ShiftMixParseVarint<FieldType>(ptr + sizeof(TagType), res);
  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  hasbits |= (uint64_t{1} << data.hasbit_idx());
  RefAt<FieldType>(msg, data.offset()) = res;
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastV8S1(PROTOBUF_TC_PARAM_DECL) {
  using TagType = uint8_t;

  // Special case for a varint bool field with a tag of 1 byte:
  // The coded_tag() field will actually contain the value too and we can check
  // both at the same time.
  auto coded_tag = data.coded_tag<uint16_t>();
  if (PROTOBUF_PREDICT_TRUE(coded_tag == 0x0000 || coded_tag == 0x0100)) {
    auto& field = RefAt<bool>(msg, data.offset());
    // Note: we use `data.data` because Clang generates suboptimal code when
    // using coded_tag.
    // In x86_64 this uses the CH register to read the second byte out of
    // `data`.
    uint8_t value = data.data >> 8;
    // The assume allows using a mov instead of test+setne.
    PROTOBUF_ASSUME(value <= 1);
    field = static_cast<bool>(value);

    ptr += sizeof(TagType) + 1;  // Consume the tag and the value.
    hasbits |= (uint64_t{1} << data.hasbit_idx());

    PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }

  // If it didn't match above either the tag is wrong, or the value is encoded
  // non-canonically.
  // Jump to MiniParse as wrong tag is the most probable reason.
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastV8S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularVarint<bool, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV32S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return FastVarintS1<uint32_t>(PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV32S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularVarint<uint32_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV64S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return FastVarintS1<uint64_t>(PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV64S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularVarint<uint64_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastZ32S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularVarint<int32_t, uint8_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ32S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularVarint<int32_t, uint16_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ64S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularVarint<int64_t, uint8_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ64S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularVarint<int64_t, uint16_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename FieldType, typename TagType, bool zigzag>
PROTOBUF_ALWAYS_INLINE const char* TcParser::RepeatedVarint(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  auto& field = RefAt<RepeatedField<FieldType>>(msg, data.offset());
  const auto expected_tag = UnalignedLoad<TagType>(ptr);
  do {
    ptr += sizeof(TagType);
    FieldType tmp;
    ptr = ParseVarint(ptr, &tmp);
    if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
      PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
    field.Add(ZigZagDecodeHelper<FieldType, zigzag>(tmp));
    if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) {
      PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
  } while (UnalignedLoad<TagType>(ptr) == expected_tag);
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastV8R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<bool, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV8R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<bool, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV32R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<uint32_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV32R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<uint32_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV64R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<uint64_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV64R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<uint64_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastZ32R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<int32_t, uint8_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ32R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<int32_t, uint16_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ64R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<int64_t, uint8_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ64R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedVarint<int64_t, uint16_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename FieldType, typename TagType, bool zigzag>
PROTOBUF_ALWAYS_INLINE const char* TcParser::PackedVarint(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  ptr += sizeof(TagType);
  // Since ctx->ReadPackedVarint does not use TailCall or Return, sync any
  // pending hasbits now:
  SyncHasbits(msg, hasbits, table);
  auto* field = &RefAt<RepeatedField<FieldType>>(msg, data.offset());
  return ctx->ReadPackedVarint(ptr, [field](uint64_t varint) {
    FieldType val;
    if (zigzag) {
      if (sizeof(FieldType) == 8) {
        val = WireFormatLite::ZigZagDecode64(varint);
      } else {
        val = WireFormatLite::ZigZagDecode32(varint);
      }
    } else {
      val = varint;
    }
    field->Add(val);
  });
}

PROTOBUF_NOINLINE const char* TcParser::FastV8P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<bool, uint8_t>(PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV8P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<bool, uint16_t>(PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV32P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<uint32_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV32P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<uint32_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV64P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<uint64_t, uint8_t>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastV64P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<uint64_t, uint16_t>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastZ32P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<int32_t, uint8_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ32P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<int32_t, uint16_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ64P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<int64_t, uint8_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastZ64P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedVarint<int64_t, uint16_t, true>(
      PROTOBUF_TC_PARAM_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// Enum fields
//////////////////////////////////////////////////////////////////////////////

PROTOBUF_NOINLINE const char* TcParser::FastUnknownEnumFallback(
    PROTOBUF_TC_PARAM_DECL) {
  // Skip MiniParse/fallback and insert the element directly into the unknown
  // field set. We also normalize the value into an int32 as we do for known
  // enum values.
  uint32_t tag;
  ptr = ReadTag(ptr, &tag);
  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  uint64_t tmp;
  ptr = ParseVarint(ptr, &tmp);
  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  AddUnknownEnum(msg, table, tag, static_cast<int32_t>(tmp));
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::MpUnknownEnumFallback(
    PROTOBUF_TC_PARAM_DECL) {
  // Like FastUnknownEnumFallback, but with the Mp ABI.
  uint32_t tag = data.tag();
  uint64_t tmp;
  ptr = ParseVarint(ptr, &tmp);
  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  AddUnknownEnum(msg, table, tag, static_cast<int32_t>(tmp));
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <typename TagType, uint16_t xform_val>
PROTOBUF_ALWAYS_INLINE const char* TcParser::SingularEnum(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  const TcParseTableBase::FieldAux aux = *table->field_aux(data.aux_idx());
  PrefetchEnumData(xform_val, aux);
  const char* ptr2 = ptr;  // Save for unknown enum case
  ptr += sizeof(TagType);  // Consume tag
  uint64_t tmp;
  ptr = ParseVarint(ptr, &tmp);
  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  if (PROTOBUF_PREDICT_FALSE(
          !EnumIsValidAux(static_cast<int32_t>(tmp), xform_val, aux))) {
    ptr = ptr2;
    PROTOBUF_MUSTTAIL return FastUnknownEnumFallback(PROTOBUF_TC_PARAM_PASS);
  }
  hasbits |= (uint64_t{1} << data.hasbit_idx());
  RefAt<int32_t>(msg, data.offset()) = tmp;
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastErS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnum<uint8_t, field_layout::kTvRange>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastErS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnum<uint16_t, field_layout::kTvRange>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEvS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnum<uint8_t, field_layout::kTvEnum>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEvS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnum<uint16_t, field_layout::kTvEnum>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename TagType, uint16_t xform_val>
PROTOBUF_ALWAYS_INLINE const char* TcParser::RepeatedEnum(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  auto& field = RefAt<RepeatedField<int32_t>>(msg, data.offset());
  const auto expected_tag = UnalignedLoad<TagType>(ptr);
  const TcParseTableBase::FieldAux aux = *table->field_aux(data.aux_idx());
  PrefetchEnumData(xform_val, aux);
  do {
    const char* ptr2 = ptr;  // save for unknown enum case
    ptr += sizeof(TagType);
    uint64_t tmp;
    ptr = ParseVarint(ptr, &tmp);
    if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
      PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
    if (PROTOBUF_PREDICT_FALSE(
            !EnumIsValidAux(static_cast<int32_t>(tmp), xform_val, aux))) {
      // We can avoid duplicate work in MiniParse by directly calling
      // table->fallback.
      ptr = ptr2;
      PROTOBUF_MUSTTAIL return FastUnknownEnumFallback(PROTOBUF_TC_PARAM_PASS);
    }
    field.Add(static_cast<int32_t>(tmp));
    if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) {
      PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
  } while (UnalignedLoad<TagType>(ptr) == expected_tag);

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

const TcParser::UnknownFieldOps& TcParser::GetUnknownFieldOps(
    const TcParseTableBase* table) {
  // Call the fallback function in a special mode to only act as a
  // way to return the ops.
  // Hiding the unknown fields vtable behind the fallback function avoids adding
  // more pointers in TcParseTableBase, and the extra runtime jumps are not
  // relevant because unknown fields are rare.
  const char* ptr = table->fallback(nullptr, nullptr, nullptr, {}, nullptr, 0);
  return *reinterpret_cast<const UnknownFieldOps*>(ptr);
}

PROTOBUF_NOINLINE void TcParser::AddUnknownEnum(MessageLite* msg,
                                                const TcParseTableBase* table,
                                                uint32_t tag,
                                                int32_t enum_value) {
  GetUnknownFieldOps(table).write_varint(msg, tag >> 3, enum_value);
}

template <typename TagType, uint16_t xform_val>
PROTOBUF_ALWAYS_INLINE const char* TcParser::PackedEnum(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  const auto saved_tag = UnalignedLoad<TagType>(ptr);
  ptr += sizeof(TagType);
  // Since ctx->ReadPackedVarint does not use TailCall or Return, sync any
  // pending hasbits now:
  SyncHasbits(msg, hasbits, table);
  auto* field = &RefAt<RepeatedField<int32_t>>(msg, data.offset());
  const TcParseTableBase::FieldAux aux = *table->field_aux(data.aux_idx());
  PrefetchEnumData(xform_val, aux);
  return ctx->ReadPackedVarint(ptr, [=](int32_t value) {
    if (!EnumIsValidAux(value, xform_val, aux)) {
      AddUnknownEnum(msg, table, FastDecodeTag(saved_tag), value);
    } else {
      field->Add(value);
    }
  });
}

PROTOBUF_NOINLINE const char* TcParser::FastErR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnum<uint8_t, field_layout::kTvRange>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastErR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnum<uint16_t, field_layout::kTvRange>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEvR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnum<uint8_t, field_layout::kTvEnum>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEvR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnum<uint16_t, field_layout::kTvEnum>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastErP1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnum<uint8_t, field_layout::kTvRange>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastErP2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnum<uint16_t, field_layout::kTvRange>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEvP1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnum<uint8_t, field_layout::kTvEnum>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEvP2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnum<uint16_t, field_layout::kTvEnum>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename TagType, uint8_t min>
PROTOBUF_ALWAYS_INLINE const char* TcParser::SingularEnumSmallRange(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }

  uint8_t v = ptr[sizeof(TagType)];
  if (PROTOBUF_PREDICT_FALSE(min > v || v > data.aux_idx())) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }

  RefAt<int32_t>(msg, data.offset()) = v;
  ptr += sizeof(TagType) + 1;
  hasbits |= (uint64_t{1} << data.hasbit_idx());
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEr0S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnumSmallRange<uint8_t, 0>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEr0S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnumSmallRange<uint16_t, 0>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEr1S1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnumSmallRange<uint8_t, 1>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEr1S2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularEnumSmallRange<uint16_t, 1>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename TagType, uint8_t min>
PROTOBUF_ALWAYS_INLINE const char* TcParser::RepeatedEnumSmallRange(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  auto& field = RefAt<RepeatedField<int32_t>>(msg, data.offset());
  auto expected_tag = UnalignedLoad<TagType>(ptr);
  const uint8_t max = data.aux_idx();
  do {
    uint8_t v = ptr[sizeof(TagType)];
    if (PROTOBUF_PREDICT_FALSE(min > v || v > max)) {
      PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
    field.Add(static_cast<int32_t>(v));
    ptr += sizeof(TagType) + 1;
    if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) {
      PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }
  } while (UnalignedLoad<TagType>(ptr) == expected_tag);

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEr0R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnumSmallRange<uint8_t, 0>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEr0R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnumSmallRange<uint16_t, 0>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEr1R1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnumSmallRange<uint8_t, 1>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEr1R2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedEnumSmallRange<uint16_t, 1>(
      PROTOBUF_TC_PARAM_PASS);
}

template <typename TagType, uint8_t min>
PROTOBUF_ALWAYS_INLINE const char* TcParser::PackedEnumSmallRange(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }

  // Since ctx->ReadPackedVarint does not use TailCall or Return, sync any
  // pending hasbits now:
  SyncHasbits(msg, hasbits, table);

  const auto saved_tag = UnalignedLoad<TagType>(ptr);
  ptr += sizeof(TagType);
  auto* field = &RefAt<RepeatedField<int32_t>>(msg, data.offset());
  const uint8_t max = data.aux_idx();

  return ctx->ReadPackedVarint(
      ptr,
      [=](int32_t v) {
        if (PROTOBUF_PREDICT_FALSE(min > v || v > max)) {
          AddUnknownEnum(msg, table, FastDecodeTag(saved_tag), v);
        } else {
          field->Add(v);
        }
      },
      /*size_callback=*/
      [=](int32_t size_bytes) {
        // For enums that fit in one varint byte, optimistically assume that all
        // the values are one byte long (i.e. no large unknown values).  If so,
        // we know exactly how many values we're going to get.
        //
        // But! size_bytes might be much larger than the total size of the
        // serialized proto (e.g. input corruption, or parsing msg1 as msg2).
        // We don't want a small serialized proto to lead to giant memory
        // allocations.
        //
        // Ideally we'd restrict size_bytes to the total size of the input, but
        // we don't know that value.  The best we can do is to restrict it to
        // the remaining bytes in the chunk, plus a "benefit of the doubt"
        // factor if we're very close to the end of the chunk.
        //
        // Do these calculations in int64 because it's possible we overflow
        // int32 (imgaine that field->size() and size_bytes are both large).
        int64_t new_size =
            int64_t{field->size()} +
            std::min(size_bytes, std::max(1024, ctx->MaximumReadSize(ptr)));
        field->Reserve(static_cast<int32_t>(
            std::min(new_size, int64_t{std::numeric_limits<int32_t>::max()})));
      });
}

PROTOBUF_NOINLINE const char* TcParser::FastEr0P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnumSmallRange<uint8_t, 0>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEr0P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnumSmallRange<uint16_t, 0>(
      PROTOBUF_TC_PARAM_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastEr1P1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnumSmallRange<uint8_t, 1>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastEr1P2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return PackedEnumSmallRange<uint16_t, 1>(
      PROTOBUF_TC_PARAM_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// String/bytes fields
//////////////////////////////////////////////////////////////////////////////

// Defined in wire_format_lite.cc
void PrintUTF8ErrorLog(absl::string_view message_name,
                       absl::string_view field_name, const char* operation_str,
                       bool emit_stacktrace);

void TcParser::ReportFastUtf8Error(uint32_t decoded_tag,
                                   const TcParseTableBase* table) {
  uint32_t field_num = decoded_tag >> 3;
  const auto* entry = FindFieldEntry(table, field_num);
  PrintUTF8ErrorLog(MessageName(table), FieldName(table, entry), "parsing",
                    false);
}

namespace {

// Here are overloads of ReadStringIntoArena, ReadStringNoArena and IsValidUTF8
// for every string class for which we provide fast-table parser support.

PROTOBUF_ALWAYS_INLINE inline const char* ReadStringIntoArena(
    MessageLite* /*msg*/, const char* ptr, ParseContext* ctx,
    uint32_t /*aux_idx*/, const TcParseTableBase* /*table*/,
    ArenaStringPtr& field, Arena* arena) {
  return ctx->ReadArenaString(ptr, &field, arena);
}

PROTOBUF_NOINLINE
const char* ReadStringNoArena(MessageLite* /*msg*/, const char* ptr,
                              ParseContext* ctx, uint32_t /*aux_idx*/,
                              const TcParseTableBase* /*table*/,
                              ArenaStringPtr& field) {
  int size = ReadSize(&ptr);
  if (!ptr) return nullptr;
  return ctx->ReadString(ptr, size, field.MutableNoCopy(nullptr));
}

PROTOBUF_ALWAYS_INLINE inline bool IsValidUTF8(ArenaStringPtr& field) {
  return utf8_range::IsStructurallyValid(field.Get());
}


}  // namespace

template <typename TagType, typename FieldType, TcParser::Utf8Type utf8>
PROTOBUF_ALWAYS_INLINE const char* TcParser::SingularString(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  auto saved_tag = UnalignedLoad<TagType>(ptr);
  ptr += sizeof(TagType);
  hasbits |= (uint64_t{1} << data.hasbit_idx());
  auto& field = RefAt<FieldType>(msg, data.offset());
  auto arena = msg->GetArena();
  if (arena) {
    ptr =
        ReadStringIntoArena(msg, ptr, ctx, data.aux_idx(), table, field, arena);
  } else {
    ptr = ReadStringNoArena(msg, ptr, ctx, data.aux_idx(), table, field);
  }
  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  switch (utf8) {
    case kNoUtf8:
#ifdef NDEBUG
    case kUtf8ValidateOnly:
#endif
      break;
    default:
      if (PROTOBUF_PREDICT_TRUE(IsValidUTF8(field))) {
        break;
      }
      ReportFastUtf8Error(FastDecodeTag(saved_tag), table);
      if (utf8 == kUtf8) {
        PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
      }
      break;
  }

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastBS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularString<uint8_t, ArenaStringPtr, kNoUtf8>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastBS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularString<uint16_t, ArenaStringPtr, kNoUtf8>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastSS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularString<uint8_t, ArenaStringPtr,
                                          kUtf8ValidateOnly>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastSS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularString<uint16_t, ArenaStringPtr,
                                          kUtf8ValidateOnly>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastUS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularString<uint8_t, ArenaStringPtr, kUtf8>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastUS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return SingularString<uint16_t, ArenaStringPtr, kUtf8>(
      PROTOBUF_TC_PARAM_PASS);
}

// Inlined string variants:

const char* TcParser::FastBiS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastBiS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastSiS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastSiS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastUiS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastUiS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

// Corded string variants:
const char* TcParser::FastBcS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastBcS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastScS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastScS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastUcS1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}
const char* TcParser::FastUcS2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <typename TagType, typename FieldType, TcParser::Utf8Type utf8>
PROTOBUF_ALWAYS_INLINE const char* TcParser::RepeatedString(
    PROTOBUF_TC_PARAM_DECL) {
  if (PROTOBUF_PREDICT_FALSE(data.coded_tag<TagType>() != 0)) {
    PROTOBUF_MUSTTAIL return MiniParse(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  const auto expected_tag = UnalignedLoad<TagType>(ptr);
  auto& field = RefAt<FieldType>(msg, data.offset());

  const auto validate_last_string = [expected_tag, table, &field] {
    switch (utf8) {
      case kNoUtf8:
#ifdef NDEBUG
      case kUtf8ValidateOnly:
#endif
        return true;
      default:
        if (PROTOBUF_PREDICT_TRUE(
                utf8_range::IsStructurallyValid(field[field.size() - 1]))) {
          return true;
        }
        ReportFastUtf8Error(FastDecodeTag(expected_tag), table);
        if (utf8 == kUtf8) return false;
        return true;
    }
  };

  auto* arena = field.GetArena();
  SerialArena* serial_arena;
  if (PROTOBUF_PREDICT_TRUE(arena != nullptr &&
                            arena->impl_.GetSerialArenaFast(&serial_arena) &&
                            field.PrepareForParse())) {
    do {
      ptr += sizeof(TagType);
      ptr = ParseRepeatedStringOnce(ptr, serial_arena, ctx, field);

      if (PROTOBUF_PREDICT_FALSE(ptr == nullptr || !validate_last_string())) {
        PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
      }
      if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
    } while (UnalignedLoad<TagType>(ptr) == expected_tag);
  } else {
    do {
      ptr += sizeof(TagType);
      std::string* str = field.Add();
      ptr = InlineGreedyStringParser(str, ptr, ctx);
      if (PROTOBUF_PREDICT_FALSE(ptr == nullptr || !validate_last_string())) {
        PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
      }
      if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
    } while (UnalignedLoad<TagType>(ptr) == expected_tag);
  }
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
parse_loop:
  PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_NOINLINE const char* TcParser::FastBR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedString<
      uint8_t, RepeatedPtrField<std::string>, kNoUtf8>(PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastBR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedString<
      uint16_t, RepeatedPtrField<std::string>, kNoUtf8>(PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastSR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedString<
      uint8_t, RepeatedPtrField<std::string>, kUtf8ValidateOnly>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastSR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedString<
      uint16_t, RepeatedPtrField<std::string>, kUtf8ValidateOnly>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastUR1(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedString<uint8_t,
                                          RepeatedPtrField<std::string>, kUtf8>(
      PROTOBUF_TC_PARAM_PASS);
}
PROTOBUF_NOINLINE const char* TcParser::FastUR2(PROTOBUF_TC_PARAM_DECL) {
  PROTOBUF_MUSTTAIL return RepeatedString<uint16_t,
                                          RepeatedPtrField<std::string>, kUtf8>(
      PROTOBUF_TC_PARAM_PASS);
}

//////////////////////////////////////////////////////////////////////////////
// Mini parsing
//////////////////////////////////////////////////////////////////////////////

namespace {
inline void SetHas(const FieldEntry& entry, MessageLite* msg) {
  auto has_idx = static_cast<uint32_t>(entry.has_idx);
#if defined(__x86_64__) && defined(__GNUC__)
  asm("bts %1, %0\n" : "+m"(*msg) : "r"(has_idx));
#else
  auto& hasblock = TcParser::RefAt<uint32_t>(msg, has_idx / 32 * 4);
  hasblock |= uint32_t{1} << (has_idx % 32);
#endif
}
}  // namespace

// Destroys any existing oneof union member (if necessary). Returns true if the
// caller is responsible for initializing the object, or false if the field
// already has the desired case.
bool TcParser::ChangeOneof(const TcParseTableBase* table,
                           const TcParseTableBase::FieldEntry& entry,
                           uint32_t field_num, ParseContext* ctx,
                           MessageLite* msg) {
  // The _oneof_case_ value offset is stored in the has-bit index.
  uint32_t* oneof_case = &TcParser::RefAt<uint32_t>(msg, entry.has_idx);
  uint32_t current_case = *oneof_case;
  *oneof_case = field_num;

  if (current_case == 0) {
    // If the member is empty, we don't have anything to clear. Caller is
    // responsible for creating a new member object.
    return true;
  }
  if (current_case == field_num) {
    // If the member is already active, then it should be merged. We're done.
    return false;
  }
  // Look up the value that is already stored, and dispose of it if necessary.
  const FieldEntry* current_entry = FindFieldEntry(table, current_case);
  uint16_t current_kind = current_entry->type_card & field_layout::kFkMask;
  uint16_t current_rep = current_entry->type_card & field_layout::kRepMask;
  if (current_kind == field_layout::kFkString) {
    switch (current_rep) {
      case field_layout::kRepAString: {
        auto& field = RefAt<ArenaStringPtr>(msg, current_entry->offset);
        field.Destroy();
        break;
      }
      case field_layout::kRepSString:
      case field_layout::kRepIString:
      default:
        ABSL_DLOG(FATAL) << "string rep not handled: "
                         << (current_rep >> field_layout::kRepShift);
        return true;
    }
  } else if (current_kind == field_layout::kFkMessage) {
    switch (current_rep) {
      case field_layout::kRepMessage:
      case field_layout::kRepGroup: {
        auto& field = RefAt<MessageLite*>(msg, current_entry->offset);
        if (!msg->GetArena()) {
          delete field;
        }
        break;
      }
      default:
        ABSL_DLOG(FATAL) << "message rep not handled: "
                         << (current_rep >> field_layout::kRepShift);
        break;
    }
  }
  return true;
}

namespace {
uint32_t GetSplitOffset(const TcParseTableBase* table) {
  return table->field_aux(kSplitOffsetAuxIdx)->offset;
}

uint32_t GetSizeofSplit(const TcParseTableBase* table) {
  return table->field_aux(kSplitSizeAuxIdx)->offset;
}
}  // namespace

void* TcParser::MaybeGetSplitBase(MessageLite* msg, const bool is_split,
                                  const TcParseTableBase* table) {
  void* out = msg;
  if (is_split) {
    const uint32_t split_offset = GetSplitOffset(table);
    void* default_split =
        TcParser::RefAt<void*>(table->default_instance, split_offset);
    void*& split = TcParser::RefAt<void*>(msg, split_offset);
    if (split == default_split) {
      // Allocate split instance when needed.
      uint32_t size = GetSizeofSplit(table);
      Arena* arena = msg->GetArena();
      split = (arena == nullptr) ? ::operator new(size)
                                 : arena->AllocateAligned(size);
      memcpy(split, default_split, size);
    }
    out = split;
  }
  return out;
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpFixed(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint16_t type_card = entry.type_card;
  const uint16_t card = type_card & field_layout::kFcMask;

  // Check for repeated parsing (wiretype fallback is handled there):
  if (card == field_layout::kFcRepeated) {
    PROTOBUF_MUSTTAIL return MpRepeatedFixed<is_split>(PROTOBUF_TC_PARAM_PASS);
  }
  // Check for mismatched wiretype:
  const uint16_t rep = type_card & field_layout::kRepMask;
  const uint32_t decoded_wiretype = data.tag() & 7;
  if (rep == field_layout::kRep64Bits) {
    if (decoded_wiretype != WireFormatLite::WIRETYPE_FIXED64) {
      PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
  } else {
    ABSL_DCHECK_EQ(rep, static_cast<uint16_t>(field_layout::kRep32Bits));
    if (decoded_wiretype != WireFormatLite::WIRETYPE_FIXED32) {
      PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
  }
  // Set the field present:
  if (card == field_layout::kFcOptional) {
    SetHas(entry, msg);
  } else if (card == field_layout::kFcOneof) {
    ChangeOneof(table, entry, data.tag() >> 3, ctx, msg);
  }
  void* const base = MaybeGetSplitBase(msg, is_split, table);
  // Copy the value:
  if (rep == field_layout::kRep64Bits) {
    RefAt<uint64_t>(base, entry.offset) = UnalignedLoad<uint64_t>(ptr);
    ptr += sizeof(uint64_t);
  } else {
    RefAt<uint32_t>(base, entry.offset) = UnalignedLoad<uint32_t>(ptr);
    ptr += sizeof(uint32_t);
  }
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpRepeatedFixed(
    PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint32_t decoded_tag = data.tag();
  const uint32_t decoded_wiretype = decoded_tag & 7;

  // Check for packed repeated fallback:
  if (decoded_wiretype == WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    PROTOBUF_MUSTTAIL return MpPackedFixed<is_split>(PROTOBUF_TC_PARAM_PASS);
  }

  void* const base = MaybeGetSplitBase(msg, is_split, table);
  const uint16_t type_card = entry.type_card;
  const uint16_t rep = type_card & field_layout::kRepMask;
  if (rep == field_layout::kRep64Bits) {
    if (decoded_wiretype != WireFormatLite::WIRETYPE_FIXED64) {
      PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
    auto& field = MaybeCreateRepeatedFieldRefAt<uint64_t, is_split>(
        base, entry.offset, msg);
    constexpr auto size = sizeof(uint64_t);
    const char* ptr2 = ptr;
    uint32_t next_tag;
    do {
      ptr = ptr2;
      *field.Add() = UnalignedLoad<uint64_t>(ptr);
      ptr += size;
      if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
      ptr2 = ReadTag(ptr, &next_tag);
      if (PROTOBUF_PREDICT_FALSE(ptr2 == nullptr)) goto error;
    } while (next_tag == decoded_tag);
  } else {
    ABSL_DCHECK_EQ(rep, static_cast<uint16_t>(field_layout::kRep32Bits));
    if (decoded_wiretype != WireFormatLite::WIRETYPE_FIXED32) {
      PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
    auto& field = MaybeCreateRepeatedFieldRefAt<uint32_t, is_split>(
        base, entry.offset, msg);
    constexpr auto size = sizeof(uint32_t);
    const char* ptr2 = ptr;
    uint32_t next_tag;
    do {
      ptr = ptr2;
      *field.Add() = UnalignedLoad<uint32_t>(ptr);
      ptr += size;
      if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
      ptr2 = ReadTag(ptr, &next_tag);
      if (PROTOBUF_PREDICT_FALSE(ptr2 == nullptr)) goto error;
    } while (next_tag == decoded_tag);
  }

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
parse_loop:
  PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
error:
  PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpPackedFixed(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint16_t type_card = entry.type_card;
  const uint32_t decoded_wiretype = data.tag() & 7;

  // Check for non-packed repeated fallback:
  if (decoded_wiretype != WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    PROTOBUF_MUSTTAIL return MpRepeatedFixed<is_split>(PROTOBUF_TC_PARAM_PASS);
  }

  void* const base = MaybeGetSplitBase(msg, is_split, table);
  int size = ReadSize(&ptr);
  uint16_t rep = type_card & field_layout::kRepMask;
  if (rep == field_layout::kRep64Bits) {
    auto& field = MaybeCreateRepeatedFieldRefAt<uint64_t, is_split>(
        base, entry.offset, msg);
    ptr = ctx->ReadPackedFixed(ptr, size, &field);
  } else {
    ABSL_DCHECK_EQ(rep, static_cast<uint16_t>(field_layout::kRep32Bits));
    auto& field = MaybeCreateRepeatedFieldRefAt<uint32_t, is_split>(
        base, entry.offset, msg);
    ptr = ctx->ReadPackedFixed(ptr, size, &field);
  }

  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpVarint(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint16_t type_card = entry.type_card;
  const uint16_t card = type_card & field_layout::kFcMask;

  // Check for repeated parsing:
  if (card == field_layout::kFcRepeated) {
    PROTOBUF_MUSTTAIL return MpRepeatedVarint<is_split>(PROTOBUF_TC_PARAM_PASS);
  }
  // Check for wire type mismatch:
  if ((data.tag() & 7) != WireFormatLite::WIRETYPE_VARINT) {
    PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
  }
  const uint16_t xform_val = type_card & field_layout::kTvMask;
  const bool is_zigzag = xform_val == field_layout::kTvZigZag;
  const bool is_validated_enum = xform_val & field_layout::kTvEnum;

  // Parse the value:
  const char* ptr2 = ptr;  // save for unknown enum case
  uint64_t tmp;
  ptr = ParseVarint(ptr, &tmp);
  if (ptr == nullptr) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }

  // Transform and/or validate the value
  uint16_t rep = type_card & field_layout::kRepMask;
  if (rep == field_layout::kRep64Bits) {
    if (is_zigzag) {
      tmp = WireFormatLite::ZigZagDecode64(tmp);
    }
  } else if (rep == field_layout::kRep32Bits) {
    if (is_validated_enum) {
      if (!EnumIsValidAux(tmp, xform_val, *table->field_aux(&entry))) {
        ptr = ptr2;
        PROTOBUF_MUSTTAIL return MpUnknownEnumFallback(PROTOBUF_TC_PARAM_PASS);
      }
    } else if (is_zigzag) {
      tmp = WireFormatLite::ZigZagDecode32(static_cast<uint32_t>(tmp));
    }
  }

  // Mark the field as present:
  const bool is_oneof = card == field_layout::kFcOneof;
  if (card == field_layout::kFcOptional) {
    SetHas(entry, msg);
  } else if (is_oneof) {
    ChangeOneof(table, entry, data.tag() >> 3, ctx, msg);
  }

  void* const base = MaybeGetSplitBase(msg, is_split, table);
  if (rep == field_layout::kRep64Bits) {
    RefAt<uint64_t>(base, entry.offset) = tmp;
  } else if (rep == field_layout::kRep32Bits) {
    RefAt<uint32_t>(base, entry.offset) = static_cast<uint32_t>(tmp);
  } else {
    ABSL_DCHECK_EQ(rep, static_cast<uint16_t>(field_layout::kRep8Bits));
    RefAt<bool>(base, entry.offset) = static_cast<bool>(tmp);
  }

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <bool is_split, typename FieldType, uint16_t xform_val_in>
const char* TcParser::MpRepeatedVarintT(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint32_t decoded_tag = data.tag();
  // For is_split we ignore the incoming xform_val and read it from entry to
  // reduce duplication for the uncommon paths.
  const uint16_t xform_val =
      is_split ? (entry.type_card & field_layout::kTvMask) : xform_val_in;
  const bool is_zigzag = xform_val == field_layout::kTvZigZag;
  const bool is_validated_enum = xform_val & field_layout::kTvEnum;

  const char* ptr2 = ptr;
  uint32_t next_tag;
  void* const base = MaybeGetSplitBase(msg, is_split, table);
  auto& field = MaybeCreateRepeatedFieldRefAt<FieldType, is_split>(
      base, entry.offset, msg);

  TcParseTableBase::FieldAux aux;
  if (is_validated_enum) {
    aux = *table->field_aux(&entry);
    PrefetchEnumData(xform_val, aux);
  }

  do {
    uint64_t tmp;
    ptr = ParseVarint(ptr2, &tmp);
    if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) goto error;
    if (is_validated_enum) {
      if (!EnumIsValidAux(static_cast<int32_t>(tmp), xform_val, aux)) {
        ptr = ptr2;
        PROTOBUF_MUSTTAIL return MpUnknownEnumFallback(PROTOBUF_TC_PARAM_PASS);
      }
    } else if (is_zigzag) {
      tmp = sizeof(FieldType) == 8 ? WireFormatLite::ZigZagDecode64(tmp)
                                   : WireFormatLite::ZigZagDecode32(tmp);
    }
    field.Add(static_cast<FieldType>(tmp));
    if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
    ptr2 = ReadTag(ptr, &next_tag);
    if (PROTOBUF_PREDICT_FALSE(ptr2 == nullptr)) goto error;
  } while (next_tag == decoded_tag);

parse_loop:
  PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
error:
  PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpRepeatedVarint(
    PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const auto type_card = entry.type_card;
  const uint32_t decoded_tag = data.tag();
  const auto decoded_wiretype = decoded_tag & 7;

  // Check for packed repeated fallback:
  if (decoded_wiretype == WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    PROTOBUF_MUSTTAIL return MpPackedVarint<is_split>(PROTOBUF_TC_PARAM_PASS);
  }
  // Check for wire type mismatch:
  if (decoded_wiretype != WireFormatLite::WIRETYPE_VARINT) {
    PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
  }
  // For split we avoid the duplicate code and have the impl reload the value.
  // Less code bloat for uncommon paths.
  const uint16_t xform_val = (type_card & field_layout::kTvMask);
  const uint16_t rep = type_card & field_layout::kRepMask;
  switch (rep >> field_layout::kRepShift) {
    case field_layout::kRep64Bits >> field_layout::kRepShift:
      if (xform_val == 0) {
        PROTOBUF_MUSTTAIL return MpRepeatedVarintT<is_split, uint64_t, 0>(
            PROTOBUF_TC_PARAM_PASS);
      } else {
        ABSL_DCHECK_EQ(xform_val, +field_layout::kTvZigZag);
        PROTOBUF_MUSTTAIL return MpRepeatedVarintT<
            is_split, uint64_t, (is_split ? 0 : field_layout::kTvZigZag)>(
            PROTOBUF_TC_PARAM_PASS);
      }
    case field_layout::kRep32Bits >> field_layout::kRepShift:
      switch (xform_val >> field_layout::kTvShift) {
        case 0:
          PROTOBUF_MUSTTAIL return MpRepeatedVarintT<is_split, uint32_t, 0>(
              PROTOBUF_TC_PARAM_PASS);
        case field_layout::kTvZigZag >> field_layout::kTvShift:
          PROTOBUF_MUSTTAIL return MpRepeatedVarintT<
              is_split, uint32_t, (is_split ? 0 : field_layout::kTvZigZag)>(
              PROTOBUF_TC_PARAM_PASS);
        case field_layout::kTvEnum >> field_layout::kTvShift:
          PROTOBUF_MUSTTAIL return MpRepeatedVarintT<
              is_split, uint32_t, (is_split ? 0 : field_layout::kTvEnum)>(
              PROTOBUF_TC_PARAM_PASS);
        case field_layout::kTvRange >> field_layout::kTvShift:
          PROTOBUF_MUSTTAIL return MpRepeatedVarintT<
              is_split, uint32_t, (is_split ? 0 : field_layout::kTvRange)>(
              PROTOBUF_TC_PARAM_PASS);
        default:
          PROTOBUF_ASSUME(false);
      }
    case field_layout::kRep8Bits >> field_layout::kRepShift:
      PROTOBUF_MUSTTAIL return MpRepeatedVarintT<is_split, bool, 0>(
          PROTOBUF_TC_PARAM_PASS);

    default:
      PROTOBUF_ASSUME(false);
      return nullptr;  // To silence -Werror=return-type in some toolchains
  }
}

template <bool is_split, typename FieldType, uint16_t xform_val_in>
const char* TcParser::MpPackedVarintT(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  // For is_split we ignore the incoming xform_val and read it from entry to
  // reduce duplication for the uncommon paths.
  const uint16_t xform_val =
      is_split ? (entry.type_card & field_layout::kTvMask) : xform_val_in;
  const bool is_zigzag = xform_val == field_layout::kTvZigZag;
  const bool is_validated_enum = xform_val & field_layout::kTvEnum;

  void* const base = MaybeGetSplitBase(msg, is_split, table);
  auto* field = &MaybeCreateRepeatedFieldRefAt<FieldType, is_split>(
      base, entry.offset, msg);

  if (is_validated_enum) {
    const TcParseTableBase::FieldAux aux = *table->field_aux(entry.aux_idx);
    PrefetchEnumData(xform_val, aux);
    return ctx->ReadPackedVarint(ptr, [=](int32_t value) {
      if (!EnumIsValidAux(value, xform_val, aux)) {
        AddUnknownEnum(msg, table, data.tag(), value);
      } else {
        field->Add(value);
      }
    });
  } else {
    return ctx->ReadPackedVarint(ptr, [=](uint64_t value) {
      field->Add(is_zigzag ? (sizeof(FieldType) == 8
                                  ? WireFormatLite::ZigZagDecode64(value)
                                  : WireFormatLite::ZigZagDecode32(
                                        static_cast<uint32_t>(value)))
                           : value);
    });
  }
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpPackedVarint(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const auto type_card = entry.type_card;
  const auto decoded_wiretype = data.tag() & 7;

  // Check for non-packed repeated fallback:
  if (decoded_wiretype != WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    PROTOBUF_MUSTTAIL return MpRepeatedVarint<is_split>(PROTOBUF_TC_PARAM_PASS);
  }

  // For split we avoid the duplicate code and have the impl reload the value.
  // Less code bloat for uncommon paths.
  const uint16_t xform_val = (type_card & field_layout::kTvMask);

  // Since ctx->ReadPackedFixed does not use TailCall<> or Return<>, sync any
  // pending hasbits now:
  SyncHasbits(msg, hasbits, table);

  const uint16_t rep = type_card & field_layout::kRepMask;

  switch (rep >> field_layout::kRepShift) {
    case field_layout::kRep64Bits >> field_layout::kRepShift:
      if (xform_val == 0) {
        PROTOBUF_MUSTTAIL return MpPackedVarintT<is_split, uint64_t, 0>(
            PROTOBUF_TC_PARAM_PASS);
      } else {
        ABSL_DCHECK_EQ(xform_val, +field_layout::kTvZigZag);
        PROTOBUF_MUSTTAIL return MpPackedVarintT<
            is_split, uint64_t, (is_split ? 0 : field_layout::kTvZigZag)>(
            PROTOBUF_TC_PARAM_PASS);
      }
    case field_layout::kRep32Bits >> field_layout::kRepShift:
      switch (xform_val >> field_layout::kTvShift) {
        case 0:
          PROTOBUF_MUSTTAIL return MpPackedVarintT<is_split, uint32_t, 0>(
              PROTOBUF_TC_PARAM_PASS);
        case field_layout::kTvZigZag >> field_layout::kTvShift:
          PROTOBUF_MUSTTAIL return MpPackedVarintT<
              is_split, uint32_t, (is_split ? 0 : field_layout::kTvZigZag)>(
              PROTOBUF_TC_PARAM_PASS);
        case field_layout::kTvEnum >> field_layout::kTvShift:
          PROTOBUF_MUSTTAIL return MpPackedVarintT<
              is_split, uint32_t, (is_split ? 0 : field_layout::kTvEnum)>(
              PROTOBUF_TC_PARAM_PASS);
        case field_layout::kTvRange >> field_layout::kTvShift:
          PROTOBUF_MUSTTAIL return MpPackedVarintT<
              is_split, uint32_t, (is_split ? 0 : field_layout::kTvRange)>(
              PROTOBUF_TC_PARAM_PASS);
        default:
          PROTOBUF_ASSUME(false);
      }
    case field_layout::kRep8Bits >> field_layout::kRepShift:
      PROTOBUF_MUSTTAIL return MpPackedVarintT<is_split, bool, 0>(
          PROTOBUF_TC_PARAM_PASS);

    default:
      PROTOBUF_ASSUME(false);
      return nullptr;  // To silence -Werror=return-type in some toolchains
  }
}

bool TcParser::MpVerifyUtf8(absl::string_view wire_bytes,
                            const TcParseTableBase* table,
                            const FieldEntry& entry, uint16_t xform_val) {
  if (xform_val == field_layout::kTvUtf8) {
    if (!utf8_range::IsStructurallyValid(wire_bytes)) {
      PrintUTF8ErrorLog(MessageName(table), FieldName(table, &entry), "parsing",
                        false);
      return false;
    }
    return true;
  }
#ifndef NDEBUG
  if (xform_val == field_layout::kTvUtf8Debug) {
    if (!utf8_range::IsStructurallyValid(wire_bytes)) {
      PrintUTF8ErrorLog(MessageName(table), FieldName(table, &entry), "parsing",
                        false);
    }
  }
#endif  // NDEBUG
  return true;
}
bool TcParser::MpVerifyUtf8(const absl::Cord& wire_bytes,
                            const TcParseTableBase* table,
                            const FieldEntry& entry, uint16_t xform_val) {
  switch (xform_val) {
    default:
      ABSL_DCHECK_EQ(xform_val, 0);
      return true;
  }
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpString(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint16_t type_card = entry.type_card;
  const uint16_t card = type_card & field_layout::kFcMask;
  const uint32_t decoded_wiretype = data.tag() & 7;

  if (decoded_wiretype != WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
  }
  if (card == field_layout::kFcRepeated) {
    PROTOBUF_MUSTTAIL return MpRepeatedString<is_split>(PROTOBUF_TC_PARAM_PASS);
  }
  const uint16_t xform_val = type_card & field_layout::kTvMask;
  const uint16_t rep = type_card & field_layout::kRepMask;

  // Mark the field as present:
  const bool is_oneof = card == field_layout::kFcOneof;
  bool need_init = false;
  if (card == field_layout::kFcOptional) {
    SetHas(entry, msg);
  } else if (is_oneof) {
    need_init = ChangeOneof(table, entry, data.tag() >> 3, ctx, msg);
  }

  bool is_valid = false;
  void* const base = MaybeGetSplitBase(msg, is_split, table);
  switch (rep) {
    case field_layout::kRepAString: {
      auto& field = RefAt<ArenaStringPtr>(base, entry.offset);
      if (need_init) field.InitDefault();
      Arena* arena = msg->GetArena();
      if (arena) {
        ptr = ctx->ReadArenaString(ptr, &field, arena);
      } else {
        std::string* str = field.MutableNoCopy(nullptr);
        ptr = InlineGreedyStringParser(str, ptr, ctx);
      }
      if (!ptr) break;
      is_valid = MpVerifyUtf8(field.Get(), table, entry, xform_val);
      break;
    }


    case field_layout::kRepCord: {
      absl::Cord* field;
      if (is_oneof) {
        if (need_init) {
          field = Arena::Create<absl::Cord>(msg->GetArena());
          RefAt<absl::Cord*>(msg, entry.offset) = field;
        } else {
          field = RefAt<absl::Cord*>(msg, entry.offset);
        }
      } else {
        field = &RefAt<absl::Cord>(base, entry.offset);
      }
      ptr = InlineCordParser(field, ptr, ctx);
      if (!ptr) break;
      is_valid = MpVerifyUtf8(*field, table, entry, xform_val);
      break;
    }

    default:
      PROTOBUF_ASSUME(false);
  }

  if (PROTOBUF_PREDICT_FALSE(ptr == nullptr || !is_valid)) {
    PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
  }
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

PROTOBUF_ALWAYS_INLINE const char* TcParser::ParseRepeatedStringOnce(
    const char* ptr, SerialArena* serial_arena, ParseContext* ctx,
    RepeatedPtrField<std::string>& field) {
  int size = ReadSize(&ptr);
  if (PROTOBUF_PREDICT_FALSE(!ptr)) return {};
  auto* str = new (serial_arena->AllocateFromStringBlock()) std::string();
  field.AddAllocatedForParse(str);
  ptr = ctx->ReadString(ptr, size, str);
  if (PROTOBUF_PREDICT_FALSE(!ptr)) return {};
  PROTOBUF_ASSUME(ptr != nullptr);
  return ptr;
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpRepeatedString(
    PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint16_t type_card = entry.type_card;
  const uint32_t decoded_tag = data.tag();
  const uint32_t decoded_wiretype = decoded_tag & 7;

  if (decoded_wiretype != WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
    PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
  }

  const uint16_t rep = type_card & field_layout::kRepMask;
  const uint16_t xform_val = type_card & field_layout::kTvMask;
  void* const base = MaybeGetSplitBase(msg, is_split, table);
  switch (rep) {
    case field_layout::kRepSString: {
      auto& field = MaybeCreateRepeatedPtrFieldRefAt<std::string, is_split>(
          base, entry.offset, msg);
      const char* ptr2 = ptr;
      uint32_t next_tag;

      auto* arena = field.GetArena();
      SerialArena* serial_arena;
      if (PROTOBUF_PREDICT_TRUE(
              arena != nullptr &&
              arena->impl_.GetSerialArenaFast(&serial_arena) &&
              field.PrepareForParse())) {
        do {
          ptr = ptr2;
          ptr = ParseRepeatedStringOnce(ptr, serial_arena, ctx, field);
          if (PROTOBUF_PREDICT_FALSE(ptr == nullptr ||
                                     !MpVerifyUtf8(field[field.size() - 1],
                                                   table, entry, xform_val))) {
            PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
          }
          if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
          ptr2 = ReadTag(ptr, &next_tag);
        } while (next_tag == decoded_tag);
      } else {
        do {
          ptr = ptr2;
          std::string* str = field.Add();
          ptr = InlineGreedyStringParser(str, ptr, ctx);
          if (PROTOBUF_PREDICT_FALSE(
                  ptr == nullptr ||
                  !MpVerifyUtf8(*str, table, entry, xform_val))) {
            PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
          }
          if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
          ptr2 = ReadTag(ptr, &next_tag);
        } while (next_tag == decoded_tag);
      }

      break;
    }

#ifndef NDEBUG
    default:
      ABSL_LOG(FATAL) << "Unsupported repeated string rep: " << rep;
      break;
#endif
  }

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
parse_loop:
  PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}


template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpMessage(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint16_t type_card = entry.type_card;
  const uint16_t card = type_card & field_layout::kFcMask;

  // Check for repeated parsing:
  if (card == field_layout::kFcRepeated) {
    const uint16_t rep = type_card & field_layout::kRepMask;
    switch (rep) {
      case field_layout::kRepMessage:
        PROTOBUF_MUSTTAIL return MpRepeatedMessageOrGroup<is_split, false>(
            PROTOBUF_TC_PARAM_PASS);
      case field_layout::kRepGroup:
        PROTOBUF_MUSTTAIL return MpRepeatedMessageOrGroup<is_split, true>(
            PROTOBUF_TC_PARAM_PASS);
      default:
        PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
  }

  const uint32_t decoded_tag = data.tag();
  const uint32_t decoded_wiretype = decoded_tag & 7;
  const uint16_t rep = type_card & field_layout::kRepMask;
  const bool is_group = rep == field_layout::kRepGroup;

  // Validate wiretype:
  switch (rep) {
    case field_layout::kRepMessage:
      if (decoded_wiretype != WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
        goto fallback;
      }
      break;
    case field_layout::kRepGroup:
      if (decoded_wiretype != WireFormatLite::WIRETYPE_START_GROUP) {
        goto fallback;
      }
      break;
    default: {
    fallback:
      PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
  }

  const bool is_oneof = card == field_layout::kFcOneof;
  bool need_init = false;
  if (card == field_layout::kFcOptional) {
    SetHas(entry, msg);
  } else if (is_oneof) {
    need_init = ChangeOneof(table, entry, data.tag() >> 3, ctx, msg);
  }

  void* const base = MaybeGetSplitBase(msg, is_split, table);
  SyncHasbits(msg, hasbits, table);
  MessageLite*& field = RefAt<MessageLite*>(base, entry.offset);
  if ((type_card & field_layout::kTvMask) == field_layout::kTvTable) {
    auto* inner_table = table->field_aux(&entry)->table;
    if (need_init || field == nullptr) {
      field = inner_table->default_instance->New(msg->GetArena());
    }
    if (is_group) {
      return ctx->ParseGroup<TcParser>(field, ptr, decoded_tag, inner_table);
    }
    return ctx->ParseMessage<TcParser>(field, ptr, inner_table);
  } else {
    if (need_init || field == nullptr) {
      const MessageLite* def;
      if ((type_card & field_layout::kTvMask) == field_layout::kTvDefault) {
        def = table->field_aux(&entry)->message_default();
      } else {
        ABSL_DCHECK_EQ(type_card & field_layout::kTvMask,
                       +field_layout::kTvWeakPtr);
        def = table->field_aux(&entry)->message_default_weak();
      }
      field = def->New(msg->GetArena());
    }
    if (is_group) {
      return ctx->ParseGroup(field, ptr, decoded_tag);
    }
    return ctx->ParseMessage(field, ptr);
  }
}

template <bool is_split, bool is_group>
const char* TcParser::MpRepeatedMessageOrGroup(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  const uint16_t type_card = entry.type_card;
  ABSL_DCHECK_EQ(type_card & field_layout::kFcMask,
                 static_cast<uint16_t>(field_layout::kFcRepeated));
  const uint32_t decoded_tag = data.tag();
  const uint32_t decoded_wiretype = decoded_tag & 7;

  // Validate wiretype:
  if (!is_group) {
    ABSL_DCHECK_EQ(type_card & field_layout::kRepMask,
                   static_cast<uint16_t>(field_layout::kRepMessage));
    if (decoded_wiretype != WireFormatLite::WIRETYPE_LENGTH_DELIMITED) {
      PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
  } else {
    ABSL_DCHECK_EQ(type_card & field_layout::kRepMask,
                   static_cast<uint16_t>(field_layout::kRepGroup));
    if (decoded_wiretype != WireFormatLite::WIRETYPE_START_GROUP) {
      PROTOBUF_MUSTTAIL return table->fallback(PROTOBUF_TC_PARAM_PASS);
    }
  }

  void* const base = MaybeGetSplitBase(msg, is_split, table);
  RepeatedPtrFieldBase& field =
      MaybeCreateRepeatedRefAt<RepeatedPtrFieldBase, is_split>(
          base, entry.offset, msg);
  const auto aux = *table->field_aux(&entry);
  if ((type_card & field_layout::kTvMask) == field_layout::kTvTable) {
    auto* inner_table = aux.table;
    const MessageLite* default_instance = inner_table->default_instance;
    const char* ptr2 = ptr;
    uint32_t next_tag;
    do {
      MessageLite* value =
          field.template Add<GenericTypeHandler<MessageLite>>(default_instance);
      ptr = is_group ? ctx->ParseGroup<TcParser>(value, ptr2, decoded_tag,
                                                 inner_table)
                     : ctx->ParseMessage<TcParser>(value, ptr2, inner_table);
      if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) goto error;
      if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
      ptr2 = ReadTag(ptr, &next_tag);
      if (PROTOBUF_PREDICT_FALSE(ptr2 == nullptr)) goto error;
    } while (next_tag == decoded_tag);
  } else {
    const MessageLite* default_instance;
    if ((type_card & field_layout::kTvMask) == field_layout::kTvDefault) {
      default_instance = aux.message_default();
    } else {
      ABSL_DCHECK_EQ(type_card & field_layout::kTvMask,
                     +field_layout::kTvWeakPtr);
      default_instance = aux.message_default_weak();
    }
    const char* ptr2 = ptr;
    uint32_t next_tag;
    do {
      MessageLite* value =
          field.template Add<GenericTypeHandler<MessageLite>>(default_instance);
      ptr = is_group ? ctx->ParseGroup(value, ptr2, decoded_tag)
                     : ctx->ParseMessage(value, ptr2);
      if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) goto error;
      if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) goto parse_loop;
      ptr2 = ReadTag(ptr, &next_tag);
      if (PROTOBUF_PREDICT_FALSE(ptr2 == nullptr)) goto error;
    } while (next_tag == decoded_tag);
  }
  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
parse_loop:
  PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
error:
  PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

static void SerializeMapKey(const NodeBase* node, MapTypeCard type_card,
                            io::CodedOutputStream& coded_output) {
  switch (type_card.wiretype()) {
    case WireFormatLite::WIRETYPE_VARINT:
      switch (type_card.cpp_type()) {
        case MapTypeCard::kBool:
          WireFormatLite::WriteBool(
              1, static_cast<const KeyNode<bool>*>(node)->key(), &coded_output);
          break;
        case MapTypeCard::k32:
          if (type_card.is_zigzag()) {
            WireFormatLite::WriteSInt32(
                1, static_cast<const KeyNode<uint32_t>*>(node)->key(),
                &coded_output);
          } else if (type_card.is_signed()) {
            WireFormatLite::WriteInt32(
                1, static_cast<const KeyNode<uint32_t>*>(node)->key(),
                &coded_output);
          } else {
            WireFormatLite::WriteUInt32(
                1, static_cast<const KeyNode<uint32_t>*>(node)->key(),
                &coded_output);
          }
          break;
        case MapTypeCard::k64:
          if (type_card.is_zigzag()) {
            WireFormatLite::WriteSInt64(
                1, static_cast<const KeyNode<uint64_t>*>(node)->key(),
                &coded_output);
          } else if (type_card.is_signed()) {
            WireFormatLite::WriteInt64(
                1, static_cast<const KeyNode<uint64_t>*>(node)->key(),
                &coded_output);
          } else {
            WireFormatLite::WriteUInt64(
                1, static_cast<const KeyNode<uint64_t>*>(node)->key(),
                &coded_output);
          }
          break;
        default:
          PROTOBUF_ASSUME(false);
      }
      break;
    case WireFormatLite::WIRETYPE_FIXED32:
      WireFormatLite::WriteFixed32(
          1, static_cast<const KeyNode<uint32_t>*>(node)->key(), &coded_output);
      break;
    case WireFormatLite::WIRETYPE_FIXED64:
      WireFormatLite::WriteFixed64(
          1, static_cast<const KeyNode<uint64_t>*>(node)->key(), &coded_output);
      break;
    case WireFormatLite::WIRETYPE_LENGTH_DELIMITED:
      // We should never have a message here. They can only be values maps.
      ABSL_DCHECK_EQ(+type_card.cpp_type(), +MapTypeCard::kString);
      WireFormatLite::WriteString(
          1, static_cast<const KeyNode<std::string>*>(node)->key(),
          &coded_output);
      break;
    default:
      PROTOBUF_ASSUME(false);
  }
}

void TcParser::WriteMapEntryAsUnknown(MessageLite* msg,
                                      const TcParseTableBase* table,
                                      uint32_t tag, NodeBase* node,
                                      MapAuxInfo map_info) {
  std::string serialized;
  {
    io::StringOutputStream string_output(&serialized);
    io::CodedOutputStream coded_output(&string_output);
    SerializeMapKey(node, map_info.key_type_card, coded_output);
    // The mapped_type is always an enum here.
    ABSL_DCHECK(map_info.value_is_validated_enum);
    WireFormatLite::WriteInt32(2,
                               *reinterpret_cast<int32_t*>(
                                   node->GetVoidValue(map_info.node_size_info)),
                               &coded_output);
  }
  GetUnknownFieldOps(table).write_length_delimited(msg, tag >> 3, serialized);
}

PROTOBUF_ALWAYS_INLINE inline void TcParser::InitializeMapNodeEntry(
    void* obj, MapTypeCard type_card, UntypedMapBase& map,
    const TcParseTableBase::FieldAux* aux, bool is_key) {
  (void)is_key;
  switch (type_card.cpp_type()) {
    case MapTypeCard::kBool:
      memset(obj, 0, sizeof(bool));
      break;
    case MapTypeCard::k32:
      memset(obj, 0, sizeof(uint32_t));
      break;
    case MapTypeCard::k64:
      memset(obj, 0, sizeof(uint64_t));
      break;
    case MapTypeCard::kString:
      Arena::CreateInArenaStorage(reinterpret_cast<std::string*>(obj),
                                  map.arena());
      break;
    case MapTypeCard::kMessage:
      aux[1].create_in_arena(map.arena(), reinterpret_cast<MessageLite*>(obj));
      break;
    default:
      PROTOBUF_ASSUME(false);
  }
}

PROTOBUF_NOINLINE void TcParser::DestroyMapNode(NodeBase* node,
                                                MapAuxInfo map_info,
                                                UntypedMapBase& map) {
  if (map_info.key_type_card.cpp_type() == MapTypeCard::kString) {
    static_cast<std::string*>(node->GetVoidKey())->~basic_string();
  }
  if (map_info.value_type_card.cpp_type() == MapTypeCard::kString) {
    static_cast<std::string*>(node->GetVoidValue(map_info.node_size_info))
        ->~basic_string();
  } else if (map_info.value_type_card.cpp_type() == MapTypeCard::kMessage) {
    static_cast<MessageLite*>(node->GetVoidValue(map_info.node_size_info))
        ->~MessageLite();
  }
  map.DeallocNode(node, map_info.node_size_info);
}

template <typename T>
const char* ReadFixed(void* obj, const char* ptr) {
  auto v = UnalignedLoad<T>(ptr);
  ptr += sizeof(v);
  memcpy(obj, &v, sizeof(v));
  return ptr;
}

const char* TcParser::ParseOneMapEntry(
    NodeBase* node, const char* ptr, ParseContext* ctx,
    const TcParseTableBase::FieldAux* aux, const TcParseTableBase* table,
    const TcParseTableBase::FieldEntry& entry, Arena* arena) {
  using WFL = WireFormatLite;

  const auto map_info = aux[0].map_info;
  const uint8_t key_tag = WFL::MakeTag(1, map_info.key_type_card.wiretype());
  const uint8_t value_tag =
      WFL::MakeTag(2, map_info.value_type_card.wiretype());

  while (!ctx->Done(&ptr)) {
    uint32_t inner_tag = ptr[0];

    if (PROTOBUF_PREDICT_FALSE(inner_tag != key_tag &&
                               inner_tag != value_tag)) {
      // Do a full parse and check again in case the tag has non-canonical
      // encoding.
      ptr = ReadTag(ptr, &inner_tag);
      if (PROTOBUF_PREDICT_FALSE(inner_tag != key_tag &&
                                 inner_tag != value_tag)) {
        if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;

        if (inner_tag == 0 || (inner_tag & 7) == WFL::WIRETYPE_END_GROUP) {
          ctx->SetLastTag(inner_tag);
          break;
        }

        ptr = UnknownFieldParse(inner_tag, nullptr, ptr, ctx);
        if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
        continue;
      }
    } else {
      ++ptr;
    }

    MapTypeCard type_card;
    void* obj;
    if (inner_tag == key_tag) {
      type_card = map_info.key_type_card;
      obj = node->GetVoidKey();
    } else {
      type_card = map_info.value_type_card;
      obj = node->GetVoidValue(map_info.node_size_info);
    }

    switch (type_card.wiretype()) {
      case WFL::WIRETYPE_VARINT:
        uint64_t tmp;
        ptr = ParseVarint(ptr, &tmp);
        if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
        switch (type_card.cpp_type()) {
          case MapTypeCard::kBool:
            *reinterpret_cast<bool*>(obj) = static_cast<bool>(tmp);
            continue;
          case MapTypeCard::k32: {
            uint32_t v = static_cast<uint32_t>(tmp);
            if (type_card.is_zigzag()) v = WFL::ZigZagDecode32(v);
            memcpy(obj, &v, sizeof(v));
            continue;
          }
          case MapTypeCard::k64:
            if (type_card.is_zigzag()) tmp = WFL::ZigZagDecode64(tmp);
            memcpy(obj, &tmp, sizeof(tmp));
            continue;
          default:
            PROTOBUF_ASSUME(false);
        }
      case WFL::WIRETYPE_FIXED32:
        ptr = ReadFixed<uint32_t>(obj, ptr);
        continue;
      case WFL::WIRETYPE_FIXED64:
        ptr = ReadFixed<uint64_t>(obj, ptr);
        continue;
      case WFL::WIRETYPE_LENGTH_DELIMITED:
        if (type_card.cpp_type() == MapTypeCard::kString) {
          const int size = ReadSize(&ptr);
          if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
          std::string* str = reinterpret_cast<std::string*>(obj);
          ptr = ctx->ReadString(ptr, size, str);
          if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
          bool do_utf8_check = map_info.fail_on_utf8_failure;
#ifndef NDEBUG
          do_utf8_check |= map_info.log_debug_utf8_failure;
#endif
          if (type_card.is_utf8() && do_utf8_check &&
              !utf8_range::IsStructurallyValid(*str)) {
            PrintUTF8ErrorLog(MessageName(table), FieldName(table, &entry),
                              "parsing", false);
            if (map_info.fail_on_utf8_failure) {
              return nullptr;
            }
          }
          continue;
        } else {
          ABSL_DCHECK_EQ(+type_card.cpp_type(), +MapTypeCard::kMessage);
          ABSL_DCHECK_EQ(inner_tag, value_tag);
          ptr = ctx->ParseMessage(reinterpret_cast<MessageLite*>(obj), ptr);
          if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) return nullptr;
          continue;
        }
      default:
        PROTOBUF_ASSUME(false);
    }
  }
  return ptr;
}

template <bool is_split>
PROTOBUF_NOINLINE const char* TcParser::MpMap(PROTOBUF_TC_PARAM_DECL) {
  const auto& entry = RefAt<FieldEntry>(table, data.entry_offset());
  // `aux[0]` points into a MapAuxInfo.
  // If we have a message mapped_type aux[1] points into a `create_in_arena`.
  // If we have a validated enum mapped_type aux[1] point into a
  // `enum_data`.
  const auto* aux = table->field_aux(&entry);
  const auto map_info = aux[0].map_info;

  if (PROTOBUF_PREDICT_FALSE(!map_info.is_supported ||
                             (data.tag() & 7) !=
                                 WireFormatLite::WIRETYPE_LENGTH_DELIMITED)) {
    PROTOBUF_MUSTTAIL return MpFallback(PROTOBUF_TC_PARAM_PASS);
  }

  // When using LITE, the offset points directly into the Map<> object.
  // Otherwise, it points into a MapField and we must synchronize with
  // reflection. It is done by calling the MutableMap() virtual function on the
  // field's base class.
  void* const base = MaybeGetSplitBase(msg, is_split, table);
  UntypedMapBase& map =
      map_info.use_lite
          ? RefAt<UntypedMapBase>(base, entry.offset)
          : *RefAt<MapFieldBaseForParse>(base, entry.offset).MutableMap();

  const uint32_t saved_tag = data.tag();

  while (true) {
    NodeBase* node = map.AllocNode(map_info.node_size_info);

    InitializeMapNodeEntry(node->GetVoidKey(), map_info.key_type_card, map, aux,
                           true);
    InitializeMapNodeEntry(node->GetVoidValue(map_info.node_size_info),
                           map_info.value_type_card, map, aux, false);

    ptr = ctx->ParseLengthDelimitedInlined(ptr, [&](const char* ptr) {
      return ParseOneMapEntry(node, ptr, ctx, aux, table, entry, map.arena());
    });

    if (PROTOBUF_PREDICT_TRUE(ptr != nullptr)) {
      if (PROTOBUF_PREDICT_FALSE(map_info.value_is_validated_enum &&
                                 !internal::ValidateEnumInlined(
                                     *static_cast<int32_t*>(node->GetVoidValue(
                                         map_info.node_size_info)),
                                     aux[1].enum_data))) {
        WriteMapEntryAsUnknown(msg, table, saved_tag, node, map_info);
      } else {
        // Done parsing the node, try to insert it.
        // If it overwrites something we get old node back to destroy it.
        switch (map_info.key_type_card.cpp_type()) {
          case MapTypeCard::kBool:
            node = static_cast<KeyMapBase<bool>&>(map).InsertOrReplaceNode(
                static_cast<KeyMapBase<bool>::KeyNode*>(node));
            break;
          case MapTypeCard::k32:
            node = static_cast<KeyMapBase<uint32_t>&>(map).InsertOrReplaceNode(
                static_cast<KeyMapBase<uint32_t>::KeyNode*>(node));
            break;
          case MapTypeCard::k64:
            node = static_cast<KeyMapBase<uint64_t>&>(map).InsertOrReplaceNode(
                static_cast<KeyMapBase<uint64_t>::KeyNode*>(node));
            break;
          case MapTypeCard::kString:
            node =
                static_cast<KeyMapBase<std::string>&>(map).InsertOrReplaceNode(
                    static_cast<KeyMapBase<std::string>::KeyNode*>(node));
            break;
          default:
            PROTOBUF_ASSUME(false);
        }
      }
    }

    // Destroy the node if we have it.
    // It could be because we failed to parse, or because insertion returned
    // an overwritten node.
    if (PROTOBUF_PREDICT_FALSE(node != nullptr && map.arena() == nullptr)) {
      DestroyMapNode(node, map_info, map);
    }

    if (PROTOBUF_PREDICT_FALSE(ptr == nullptr)) {
      PROTOBUF_MUSTTAIL return Error(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }

    if (PROTOBUF_PREDICT_FALSE(!ctx->DataAvailable(ptr))) {
      PROTOBUF_MUSTTAIL return ToParseLoop(PROTOBUF_TC_PARAM_NO_DATA_PASS);
    }

    uint32_t next_tag;
    const char* ptr2 = ReadTagInlined(ptr, &next_tag);
    if (next_tag != saved_tag) break;
    ptr = ptr2;
  }

  PROTOBUF_MUSTTAIL return ToTagDispatch(PROTOBUF_TC_PARAM_NO_DATA_PASS);
}

}  // namespace internal
}  // namespace protobuf
}  // namespace google

