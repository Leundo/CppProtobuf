// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_PARSE_CONTEXT_H__
#define GOOGLE_PROTOBUF_PARSE_CONTEXT_H__

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

#include <CppAbseil/absl_base_config.hpp>
#include <CppAbseil/absl_log_absl_check.hpp>
#include <CppAbseil/absl_log_absl_log.hpp>
#include <CppAbseil/absl_strings_cord.hpp>
#include <CppAbseil/absl_strings_internal_resize_uninitialized.hpp>
#include <CppAbseil/absl_strings_string_view.hpp>
#include <CppAbseil/absl_types_optional.hpp>
#include <CppProtobuf/google_protobuf_arena.hpp>
#include <CppProtobuf/google_protobuf_arenastring.hpp>
#include <CppProtobuf/google_protobuf_endian.hpp>
#include <CppProtobuf/google_protobuf_implicit_weak_message.hpp>
#include <CppProtobuf/google_protobuf_inlined_string_field.hpp>
#include <CppProtobuf/google_protobuf_io_coded_stream.hpp>
#include <CppProtobuf/google_protobuf_io_zero_copy_stream.hpp>
#include <CppProtobuf/google_protobuf_metadata_lite.hpp>
#include <CppProtobuf/google_protobuf_port.hpp>
#include <CppProtobuf/google_protobuf_repeated_field.hpp>
#include <CppProtobuf/google_protobuf_wire_format_lite.hpp>


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

class UnknownFieldSet;
class DescriptorPool;
class MessageFactory;

namespace internal {

// Template code below needs to know about the existence of these functions.
PROTOBUF_EXPORT void WriteVarint(uint32_t num, uint64_t val, std::string* s);
PROTOBUF_EXPORT void WriteLengthDelimited(uint32_t num, absl::string_view val,
                                          std::string* s);
// Inline because it is just forwarding to s->WriteVarint
inline void WriteVarint(uint32_t num, uint64_t val, UnknownFieldSet* s);
inline void WriteLengthDelimited(uint32_t num, absl::string_view val,
                                 UnknownFieldSet* s);


// The basic abstraction the parser is designed for is a slight modification
// of the ZeroCopyInputStream (ZCIS) abstraction. A ZCIS presents a serialized
// stream as a series of buffers that concatenate to the full stream.
// Pictorially a ZCIS presents a stream in chunks like so
// [---------------------------------------------------------------]
// [---------------------] chunk 1
//                      [----------------------------] chunk 2
//                                          chunk 3 [--------------]
//
// Where the '-' represent the bytes which are vertically lined up with the
// bytes of the stream. The proto parser requires its input to be presented
// similarly with the extra
// property that each chunk has kSlopBytes past its end that overlaps with the
// first kSlopBytes of the next chunk, or if there is no next chunk at least its
// still valid to read those bytes. Again, pictorially, we now have
//
// [---------------------------------------------------------------]
// [-------------------....] chunk 1
//                    [------------------------....] chunk 2
//                                    chunk 3 [------------------..**]
//                                                      chunk 4 [--****]
// Here '-' mean the bytes of the stream or chunk and '.' means bytes past the
// chunk that match up with the start of the next chunk. Above each chunk has
// 4 '.' after the chunk. In the case these 'overflow' bytes represents bytes
// past the stream, indicated by '*' above, their values are unspecified. It is
// still legal to read them (ie. should not segfault). Reading past the
// end should be detected by the user and indicated as an error.
//
// The reason for this, admittedly, unconventional invariant is to ruthlessly
// optimize the protobuf parser. Having an overlap helps in two important ways.
// Firstly it alleviates having to performing bounds checks if a piece of code
// is guaranteed to not read more than kSlopBytes. Secondly, and more
// importantly, the protobuf wireformat is such that reading a key/value pair is
// always less than 16 bytes. This removes the need to change to next buffer in
// the middle of reading primitive values. Hence there is no need to store and
// load the current position.

class PROTOBUF_EXPORT EpsCopyInputStream {
 public:
  enum { kMaxCordBytesToCopy = 512 };
  explicit EpsCopyInputStream(bool enable_aliasing)
      : aliasing_(enable_aliasing ? kOnPatch : kNoAliasing) {}

  void BackUp(const char* ptr) {
    ABSL_DCHECK(ptr <= buffer_end_ + kSlopBytes);
    int count;
    if (next_chunk_ == patch_buffer_) {
      count = static_cast<int>(buffer_end_ + kSlopBytes - ptr);
    } else {
      count = size_ + static_cast<int>(buffer_end_ - ptr);
    }
    if (count > 0) StreamBackUp(count);
  }

  // In sanitizer mode we use memory poisoning to guarantee that:
  //  - We do not read an uninitialized token.
  //  - We would like to verify that this token was consumed, but unforuntately
  //    __asan_address_is_poisoned is allowed to have false negatives.
  class LimitToken {
   public:
    LimitToken() { PROTOBUF_POISON_MEMORY_REGION(&token_, sizeof(token_)); }

    explicit LimitToken(int token) : token_(token) {
      PROTOBUF_UNPOISON_MEMORY_REGION(&token_, sizeof(token_));
    }

    LimitToken(const LimitToken&) = delete;
    LimitToken& operator=(const LimitToken&) = delete;

    LimitToken(LimitToken&& other) { *this = std::move(other); }

    LimitToken& operator=(LimitToken&& other) {
      PROTOBUF_UNPOISON_MEMORY_REGION(&token_, sizeof(token_));
      token_ = other.token_;
      PROTOBUF_POISON_MEMORY_REGION(&other.token_, sizeof(token_));
      return *this;
    }

    ~LimitToken() { PROTOBUF_UNPOISON_MEMORY_REGION(&token_, sizeof(token_)); }

    int token() && {
      int t = token_;
      PROTOBUF_POISON_MEMORY_REGION(&token_, sizeof(token_));
      return t;
    }

   private:
    int token_;
  };

  // If return value is negative it's an error
  PROTOBUF_NODISCARD LimitToken PushLimit(const char* ptr, int limit) {
    ABSL_DCHECK(limit >= 0 && limit <= INT_MAX - kSlopBytes);
    // This add is safe due to the invariant above, because
    // ptr - buffer_end_ <= kSlopBytes.
    limit += static_cast<int>(ptr - buffer_end_);
    limit_end_ = buffer_end_ + (std::min)(0, limit);
    auto old_limit = limit_;
    limit_ = limit;
    return LimitToken(old_limit - limit);
  }

  PROTOBUF_NODISCARD bool PopLimit(LimitToken delta) {
    // We must update the limit first before the early return. Otherwise, we can
    // end up with an invalid limit and it can lead to integer overflows.
    limit_ = limit_ + std::move(delta).token();
    if (PROTOBUF_PREDICT_FALSE(!EndedAtLimit())) return false;
    // TODO We could remove this line and hoist the code to
    // DoneFallback. Study the perf/bin-size effects.
    limit_end_ = buffer_end_ + (std::min)(0, limit_);
    return true;
  }

  PROTOBUF_NODISCARD const char* Skip(const char* ptr, int size) {
    if (size <= buffer_end_ + kSlopBytes - ptr) {
      return ptr + size;
    }
    return SkipFallback(ptr, size);
  }
  PROTOBUF_NODISCARD const char* ReadString(const char* ptr, int size,
                                            std::string* s) {
    if (size <= buffer_end_ + kSlopBytes - ptr) {
      // Fundamentally we just want to do assign to the string.
      // However micro-benchmarks regress on string reading cases. So we copy
      // the same logic from the old CodedInputStream ReadString. Note: as of
      // Apr 2021, this is still a significant win over `assign()`.
      absl::strings_internal::STLStringResizeUninitialized(s, size);
      char* z = &(*s)[0];
      memcpy(z, ptr, size);
      return ptr + size;
    }
    return ReadStringFallback(ptr, size, s);
  }
  PROTOBUF_NODISCARD const char* AppendString(const char* ptr, int size,
                                              std::string* s) {
    if (size <= buffer_end_ + kSlopBytes - ptr) {
      s->append(ptr, size);
      return ptr + size;
    }
    return AppendStringFallback(ptr, size, s);
  }
  // Implemented in arenastring.cc
  PROTOBUF_NODISCARD const char* ReadArenaString(const char* ptr,
                                                 ArenaStringPtr* s,
                                                 Arena* arena);

  PROTOBUF_NODISCARD const char* ReadCord(const char* ptr, int size,
                                          ::absl::Cord* cord) {
    if (size <= std::min<int>(static_cast<int>(buffer_end_ + kSlopBytes - ptr),
                              kMaxCordBytesToCopy)) {
      *cord = absl::string_view(ptr, size);
      return ptr + size;
    }
    return ReadCordFallback(ptr, size, cord);
  }


  template <typename Tag, typename T>
  PROTOBUF_NODISCARD const char* ReadRepeatedFixed(const char* ptr,
                                                   Tag expected_tag,
                                                   RepeatedField<T>* out);

  template <typename T>
  PROTOBUF_NODISCARD const char* ReadPackedFixed(const char* ptr, int size,
                                                 RepeatedField<T>* out);
  template <typename Add>
  PROTOBUF_NODISCARD const char* ReadPackedVarint(const char* ptr, Add add) {
    return ReadPackedVarint(ptr, add, [](int) {});
  }
  template <typename Add, typename SizeCb>
  PROTOBUF_NODISCARD const char* ReadPackedVarint(const char* ptr, Add add,
                                                  SizeCb size_callback);

  uint32_t LastTag() const { return last_tag_minus_1_ + 1; }
  bool ConsumeEndGroup(uint32_t start_tag) {
    bool res = last_tag_minus_1_ == start_tag;
    last_tag_minus_1_ = 0;
    return res;
  }
  bool EndedAtLimit() const { return last_tag_minus_1_ == 0; }
  bool EndedAtEndOfStream() const { return last_tag_minus_1_ == 1; }
  void SetLastTag(uint32_t tag) { last_tag_minus_1_ = tag - 1; }
  void SetEndOfStream() { last_tag_minus_1_ = 1; }
  bool IsExceedingLimit(const char* ptr) {
    return ptr > limit_end_ &&
           (next_chunk_ == nullptr || ptr - buffer_end_ > limit_);
  }
  bool AliasingEnabled() const { return aliasing_ != kNoAliasing; }
  int BytesUntilLimit(const char* ptr) const {
    return limit_ + static_cast<int>(buffer_end_ - ptr);
  }
  // Maximum number of sequential bytes that can be read starting from `ptr`.
  int MaximumReadSize(const char* ptr) const {
    return static_cast<int>(limit_end_ - ptr) + kSlopBytes;
  }
  // Returns true if more data is available, if false is returned one has to
  // call Done for further checks.
  bool DataAvailable(const char* ptr) { return ptr < limit_end_; }

 protected:
  // Returns true is limit (either an explicit limit or end of stream) is
  // reached. It aligns *ptr across buffer seams.
  // If limit is exceeded it returns true and ptr is set to null.
  bool DoneWithCheck(const char** ptr, int d) {
    ABSL_DCHECK(*ptr);
    if (PROTOBUF_PREDICT_TRUE(*ptr < limit_end_)) return false;
    int overrun = static_cast<int>(*ptr - buffer_end_);
    ABSL_DCHECK_LE(overrun, kSlopBytes);  // Guaranteed by parse loop.
    if (overrun ==
        limit_) {  //  No need to flip buffers if we ended on a limit.
      // If we actually overrun the buffer and next_chunk_ is null. It means
      // the stream ended and we passed the stream end.
      if (overrun > 0 && next_chunk_ == nullptr) *ptr = nullptr;
      return true;
    }
    auto res = DoneFallback(overrun, d);
    *ptr = res.first;
    return res.second;
  }

  const char* InitFrom(absl::string_view flat) {
    overall_limit_ = 0;
    if (flat.size() > kSlopBytes) {
      limit_ = kSlopBytes;
      limit_end_ = buffer_end_ = flat.data() + flat.size() - kSlopBytes;
      next_chunk_ = patch_buffer_;
      if (aliasing_ == kOnPatch) aliasing_ = kNoDelta;
      return flat.data();
    } else {
      if (!flat.empty()) {
        std::memcpy(patch_buffer_, flat.data(), flat.size());
      }
      limit_ = 0;
      limit_end_ = buffer_end_ = patch_buffer_ + flat.size();
      next_chunk_ = nullptr;
      if (aliasing_ == kOnPatch) {
        aliasing_ = reinterpret_cast<std::uintptr_t>(flat.data()) -
                    reinterpret_cast<std::uintptr_t>(patch_buffer_);
      }
      return patch_buffer_;
    }
  }

  const char* InitFrom(io::ZeroCopyInputStream* zcis);

  const char* InitFrom(io::ZeroCopyInputStream* zcis, int limit) {
    if (limit == -1) return InitFrom(zcis);
    overall_limit_ = limit;
    auto res = InitFrom(zcis);
    limit_ = limit - static_cast<int>(buffer_end_ - res);
    limit_end_ = buffer_end_ + (std::min)(0, limit_);
    return res;
  }

 private:
  enum { kSlopBytes = 16, kPatchBufferSize = 32 };
  static_assert(kPatchBufferSize >= kSlopBytes * 2,
                "Patch buffer needs to be at least large enough to hold all "
                "the slop bytes from the previous buffer, plus the first "
                "kSlopBytes from the next buffer.");

  const char* limit_end_;  // buffer_end_ + min(limit_, 0)
  const char* buffer_end_;
  const char* next_chunk_;
  int size_;
  int limit_;  // relative to buffer_end_;
  io::ZeroCopyInputStream* zcis_ = nullptr;
  char patch_buffer_[kPatchBufferSize] = {};
  enum { kNoAliasing = 0, kOnPatch = 1, kNoDelta = 2 };
  std::uintptr_t aliasing_ = kNoAliasing;
  // This variable is used to communicate how the parse ended, in order to
  // completely verify the parsed data. A wire-format parse can end because of
  // one of the following conditions:
  // 1) A parse can end on a pushed limit.
  // 2) A parse can end on End Of Stream (EOS).
  // 3) A parse can end on 0 tag (only valid for toplevel message).
  // 4) A parse can end on an end-group tag.
  // This variable should always be set to 0, which indicates case 1. If the
  // parse terminated due to EOS (case 2), it's set to 1. In case the parse
  // ended due to a terminating tag (case 3 and 4) it's set to (tag - 1).
  // This var doesn't really belong in EpsCopyInputStream and should be part of
  // the ParseContext, but case 2 is most easily and optimally implemented in
  // DoneFallback.
  uint32_t last_tag_minus_1_ = 0;
  int overall_limit_ = INT_MAX;  // Overall limit independent of pushed limits.
  // Pretty random large number that seems like a safe allocation on most
  // systems. TODO do we need to set this as build flag?
  enum { kSafeStringSize = 50000000 };

  // Advances to next buffer chunk returns a pointer to the same logical place
  // in the stream as set by overrun. Overrun indicates the position in the slop
  // region the parse was left (0 <= overrun <= kSlopBytes). Returns true if at
  // limit, at which point the returned pointer maybe null if there was an
  // error. The invariant of this function is that it's guaranteed that
  // kSlopBytes bytes can be accessed from the returned ptr. This function might
  // advance more buffers than one in the underlying ZeroCopyInputStream.
  std::pair<const char*, bool> DoneFallback(int overrun, int depth);
  // Advances to the next buffer, at most one call to Next() on the underlying
  // ZeroCopyInputStream is made. This function DOES NOT match the returned
  // pointer to where in the slop region the parse ends, hence no overrun
  // parameter. This is useful for string operations where you always copy
  // to the end of the buffer (including the slop region).
  const char* Next();
  // overrun is the location in the slop region the stream currently is
  // (0 <= overrun <= kSlopBytes). To prevent flipping to the next buffer of
  // the ZeroCopyInputStream in the case the parse will end in the last
  // kSlopBytes of the current buffer. depth is the current depth of nested
  // groups (or negative if the use case does not need careful tracking).
  inline const char* NextBuffer(int overrun, int depth);
  const char* SkipFallback(const char* ptr, int size);
  const char* AppendStringFallback(const char* ptr, int size, std::string* str);
  const char* ReadStringFallback(const char* ptr, int size, std::string* str);
  const char* ReadCordFallback(const char* ptr, int size, absl::Cord* cord);
  static bool ParseEndsInSlopRegion(const char* begin, int overrun, int depth);
  bool StreamNext(const void** data) {
    bool res = zcis_->Next(data, &size_);
    if (res) overall_limit_ -= size_;
    return res;
  }
  void StreamBackUp(int count) {
    zcis_->BackUp(count);
    overall_limit_ += count;
  }

  template <typename A>
  const char* AppendSize(const char* ptr, int size, const A& append) {
    int chunk_size = static_cast<int>(buffer_end_ + kSlopBytes - ptr);
    do {
      ABSL_DCHECK(size > chunk_size);
      if (next_chunk_ == nullptr) return nullptr;
      append(ptr, chunk_size);
      ptr += chunk_size;
      size -= chunk_size;
      // TODO Next calls NextBuffer which generates buffers with
      // overlap and thus incurs cost of copying the slop regions. This is not
      // necessary for reading strings. We should just call Next buffers.
      if (limit_ <= kSlopBytes) return nullptr;
      ptr = Next();
      if (ptr == nullptr) return nullptr;  // passed the limit
      ptr += kSlopBytes;
      chunk_size = static_cast<int>(buffer_end_ + kSlopBytes - ptr);
    } while (size > chunk_size);
    append(ptr, size);
    return ptr + size;
  }

  // AppendUntilEnd appends data until a limit (either a PushLimit or end of
  // stream. Normal payloads are from length delimited fields which have an
  // explicit size. Reading until limit only comes when the string takes
  // the place of a protobuf, ie RawMessage, lazy fields and implicit weak
  // messages. We keep these methods private and friend them.
  template <typename A>
  const char* AppendUntilEnd(const char* ptr, const A& append) {
    if (ptr - buffer_end_ > limit_) return nullptr;
    while (limit_ > kSlopBytes) {
      size_t chunk_size = buffer_end_ + kSlopBytes - ptr;
      append(ptr, chunk_size);
      ptr = Next();
      if (ptr == nullptr) return limit_end_;
      ptr += kSlopBytes;
    }
    auto end = buffer_end_ + limit_;
    ABSL_DCHECK(end >= ptr);
    append(ptr, end - ptr);
    return end;
  }

  PROTOBUF_NODISCARD const char* AppendString(const char* ptr,
                                              std::string* str) {
    return AppendUntilEnd(
        ptr, [str](const char* p, ptrdiff_t s) { str->append(p, s); });
  }
  friend class ImplicitWeakMessage;

  // Needs access to kSlopBytes.
  friend PROTOBUF_EXPORT std::pair<const char*, int32_t> ReadSizeFallback(
      const char* p, uint32_t res);
};

using LazyEagerVerifyFnType = const char* (*)(const char* ptr,
                                              ParseContext* ctx);
using LazyEagerVerifyFnRef = std::remove_pointer<LazyEagerVerifyFnType>::type&;

// ParseContext holds all data that is global to the entire parse. Most
// importantly it contains the input stream, but also recursion depth and also
// stores the end group tag, in case a parser ended on a endgroup, to verify
// matching start/end group tags.
class PROTOBUF_EXPORT ParseContext : public EpsCopyInputStream {
 public:
  struct Data {
    const DescriptorPool* pool = nullptr;
    MessageFactory* factory = nullptr;
  };

  template <typename... T>
  ParseContext(int depth, bool aliasing, const char** start, T&&... args)
      : EpsCopyInputStream(aliasing), depth_(depth) {
    *start = InitFrom(std::forward<T>(args)...);
  }

  struct Spawn {};
  static constexpr Spawn kSpawn = {};

  // Creates a new context from a given "ctx" to inherit a few attributes to
  // emulate continued parsing. For example, recursion depth or descriptor pools
  // must be passed down to a new "spawned" context to maintain the same parse
  // context. Note that the spawned context always disables aliasing (different
  // input).
  template <typename... T>
  ParseContext(Spawn, const ParseContext& ctx, const char** start, T&&... args)
      : EpsCopyInputStream(false),
        depth_(ctx.depth_),
        data_(ctx.data_)
  {
    *start = InitFrom(std::forward<T>(args)...);
  }

  // Move constructor and assignment operator are not supported because "ptr"
  // for parsing may have pointed to an inlined buffer (patch_buffer_) which can
  // be invalid afterwards.
  ParseContext(ParseContext&&) = delete;
  ParseContext& operator=(ParseContext&&) = delete;
  ParseContext& operator=(const ParseContext&) = delete;

  void TrackCorrectEnding() { group_depth_ = 0; }

  // Done should only be called when the parsing pointer is pointing to the
  // beginning of field data - that is, at a tag.  Or if it is NULL.
  bool Done(const char** ptr) { return DoneWithCheck(ptr, group_depth_); }

  int depth() const { return depth_; }

  Data& data() { return data_; }
  const Data& data() const { return data_; }

  const char* ParseMessage(MessageLite* msg, const char* ptr);

  // This overload supports those few cases where ParseMessage is called
  // on a class that is not actually a proto message.
  // TODO: Eliminate this use case.
  template <typename T,
            typename std::enable_if<!std::is_base_of<MessageLite, T>::value,
                                    bool>::type = true>
  PROTOBUF_NODISCARD const char* ParseMessage(T* msg, const char* ptr);

  // Read the length prefix, push the new limit, call the func(ptr), and then
  // pop the limit. Useful for situations that don't value an actual message,
  // like map entries.
  template <typename Func>
  PROTOBUF_NODISCARD const char* ParseLengthDelimitedInlined(const char*,
                                                             const Func& func);

  template <typename TcParser, typename Table>
  PROTOBUF_NODISCARD PROTOBUF_ALWAYS_INLINE const char* ParseMessage(
      MessageLite* msg, const char* ptr, const Table* table) {
    LimitToken old;
    ptr = ReadSizeAndPushLimitAndDepthInlined(ptr, &old);
    if (ptr == nullptr) return ptr;
    auto old_depth = depth_;
    ptr = TcParser::ParseLoop(msg, ptr, this, table);
    if (ptr != nullptr) ABSL_DCHECK_EQ(old_depth, depth_);
    depth_++;
    if (!PopLimit(std::move(old))) return nullptr;
    return ptr;
  }

  template <typename T>
  PROTOBUF_NODISCARD PROTOBUF_NDEBUG_INLINE const char* ParseGroup(
      T* msg, const char* ptr, uint32_t tag) {
    if (--depth_ < 0) return nullptr;
    group_depth_++;
    auto old_depth = depth_;
    auto old_group_depth = group_depth_;
    ptr = msg->_InternalParse(ptr, this);
    if (ptr != nullptr) {
      ABSL_DCHECK_EQ(old_depth, depth_);
      ABSL_DCHECK_EQ(old_group_depth, group_depth_);
    }
    group_depth_--;
    depth_++;
    if (PROTOBUF_PREDICT_FALSE(!ConsumeEndGroup(tag))) return nullptr;
    return ptr;
  }

  template <typename TcParser, typename Table>
  PROTOBUF_NODISCARD PROTOBUF_ALWAYS_INLINE const char* ParseGroup(
      MessageLite* msg, const char* ptr, uint32_t tag, const Table* table) {
    if (--depth_ < 0) return nullptr;
    group_depth_++;
    auto old_depth = depth_;
    auto old_group_depth = group_depth_;
    ptr = TcParser::ParseLoop(msg, ptr, this, table);
    if (ptr != nullptr) {
      ABSL_DCHECK_EQ(old_depth, depth_);
      ABSL_DCHECK_EQ(old_group_depth, group_depth_);
    }
    group_depth_--;
    depth_++;
    if (PROTOBUF_PREDICT_FALSE(!ConsumeEndGroup(tag))) return nullptr;
    return ptr;
  }

 private:
  // Out-of-line routine to save space in ParseContext::ParseMessage<T>
  //   LimitToken old;
  //   ptr = ReadSizeAndPushLimitAndDepth(ptr, &old)
  // is equivalent to:
  //   int size = ReadSize(&ptr);
  //   if (!ptr) return nullptr;
  //   LimitToken old = PushLimit(ptr, size);
  //   if (--depth_ < 0) return nullptr;
  PROTOBUF_NODISCARD const char* ReadSizeAndPushLimitAndDepth(
      const char* ptr, LimitToken* old_limit);

  // As above, but fully inlined for the cases where we care about performance
  // more than size. eg TcParser.
  PROTOBUF_NODISCARD PROTOBUF_ALWAYS_INLINE const char*
  ReadSizeAndPushLimitAndDepthInlined(const char* ptr, LimitToken* old_limit);

  // The context keeps an internal stack to keep track of the recursive
  // part of the parse state.
  // Current depth of the active parser, depth counts down.
  // This is used to limit recursion depth (to prevent overflow on malicious
  // data), but is also used to index in stack_ to store the current state.
  int depth_;
  // Unfortunately necessary for the fringe case of ending on 0 or end-group tag
  // in the last kSlopBytes of a ZeroCopyInputStream chunk.
  int group_depth_ = INT_MIN;
  Data data_;
};

template <int>
struct EndianHelper;

template <>
struct EndianHelper<1> {
  static uint8_t Load(const void* p) { return *static_cast<const uint8_t*>(p); }
};

template <>
struct EndianHelper<2> {
  static uint16_t Load(const void* p) {
    uint16_t tmp;
    std::memcpy(&tmp, p, 2);
    return little_endian::ToHost(tmp);
  }
};

template <>
struct EndianHelper<4> {
  static uint32_t Load(const void* p) {
    uint32_t tmp;
    std::memcpy(&tmp, p, 4);
    return little_endian::ToHost(tmp);
  }
};

template <>
struct EndianHelper<8> {
  static uint64_t Load(const void* p) {
    uint64_t tmp;
    std::memcpy(&tmp, p, 8);
    return little_endian::ToHost(tmp);
  }
};

template <typename T>
T UnalignedLoad(const char* p) {
  auto tmp = EndianHelper<sizeof(T)>::Load(p);
  T res;
  memcpy(&res, &tmp, sizeof(T));
  return res;
}
template <typename T, typename Void,
          typename = std::enable_if_t<std::is_same<Void, void>::value>>
T UnalignedLoad(const Void* p) {
  return UnalignedLoad<T>(reinterpret_cast<const char*>(p));
}

PROTOBUF_EXPORT
std::pair<const char*, uint32_t> VarintParseSlow32(const char* p, uint32_t res);
PROTOBUF_EXPORT
std::pair<const char*, uint64_t> VarintParseSlow64(const char* p, uint32_t res);

inline const char* VarintParseSlow(const char* p, uint32_t res, uint32_t* out) {
  auto tmp = VarintParseSlow32(p, res);
  *out = tmp.second;
  return tmp.first;
}

inline const char* VarintParseSlow(const char* p, uint32_t res, uint64_t* out) {
  auto tmp = VarintParseSlow64(p, res);
  *out = tmp.second;
  return tmp.first;
}

#ifdef __aarch64__
// Generally, speaking, the ARM-optimized Varint decode algorithm is to extract
// and concatenate all potentially valid data bits, compute the actual length
// of the Varint, and mask off the data bits which are not actually part of the
// result.  More detail on the two main parts is shown below.
//
// 1) Extract and concatenate all potentially valid data bits.
//    Two ARM-specific features help significantly:
//    a) Efficient and non-destructive bit extraction (UBFX)
//    b) A single instruction can perform both an OR with a shifted
//       second operand in one cycle.  E.g., the following two lines do the same
//       thing
//       ```result = operand_1 | (operand2 << 7);```
//       ```ORR %[result], %[operand_1], %[operand_2], LSL #7```
//    The figure below shows the implementation for handling four chunks.
//
// Bits   32    31-24    23   22-16    15    14-8      7     6-0
//      +----+---------+----+---------+----+---------+----+---------+
//      |CB 3| Chunk 3 |CB 2| Chunk 2 |CB 1| Chunk 1 |CB 0| Chunk 0 |
//      +----+---------+----+---------+----+---------+----+---------+
//                |              |              |              |
//               UBFX           UBFX           UBFX           UBFX    -- cycle 1
//                |              |              |              |
//                V              V              V              V
//               Combined LSL #7 and ORR     Combined LSL #7 and ORR  -- cycle 2
//                                 |             |
//                                 V             V
//                            Combined LSL #14 and ORR                -- cycle 3
//                                       |
//                                       V
//                                Parsed bits 0-27
//
//
// 2) Calculate the index of the cleared continuation bit in order to determine
//    where the encoded Varint ends and the size of the decoded value.  The
//    easiest way to do this is mask off all data bits, leaving just the
//    continuation bits.  We actually need to do the masking on an inverted
//    copy of the data, which leaves a 1 in all continuation bits which were
//    originally clear.  The number of trailing zeroes in this value indicates
//    the size of the Varint.
//
//  AND  0x80    0x80    0x80    0x80    0x80    0x80    0x80    0x80
//
// Bits   63      55      47      39      31      23      15       7
//      +----+--+----+--+----+--+----+--+----+--+----+--+----+--+----+--+
// ~    |CB 7|  |CB 6|  |CB 5|  |CB 4|  |CB 3|  |CB 2|  |CB 1|  |CB 0|  |
//      +----+--+----+--+----+--+----+--+----+--+----+--+----+--+----+--+
//         |       |       |       |       |       |       |       |
//         V       V       V       V       V       V       V       V
// Bits   63      55      47      39      31      23      15       7
//      +----+--+----+--+----+--+----+--+----+--+----+--+----+--+----+--+
//      |~CB 7|0|~CB 6|0|~CB 5|0|~CB 4|0|~CB 3|0|~CB 2|0|~CB 1|0|~CB 0|0|
//      +----+--+----+--+----+--+----+--+----+--+----+--+----+--+----+--+
//                                      |
//                                     CTZ
//                                      V
//                     Index of first cleared continuation bit
//
//
// While this is implemented in C++ significant care has been taken to ensure
// the compiler emits the best instruction sequence.  In some cases we use the
// following two functions to manipulate the compiler's scheduling decisions.
//
// Controls compiler scheduling by telling it that the first value is modified
// by the second value the callsite.  This is useful if non-critical path
// instructions are too aggressively scheduled, resulting in a slowdown of the
// actual critical path due to opportunity costs.  An example usage is shown
// where a false dependence of num_bits on result is added to prevent checking
// for a very unlikely error until all critical path instructions have been
// fetched.
//
// ```
// num_bits = <multiple operations to calculate new num_bits value>
// result = <multiple operations to calculate result>
// num_bits = ValueBarrier(num_bits, result);
// if (num_bits == 63) {
//   ABSL_LOG(FATAL) << "Invalid num_bits value";
// }
// ```
// Falsely indicate that the specific value is modified at this location.  This
// prevents code which depends on this value from being scheduled earlier.
template <typename V1Type>
PROTOBUF_ALWAYS_INLINE inline V1Type ValueBarrier(V1Type value1) {
  asm("" : "+r"(value1));
  return value1;
}

template <typename V1Type, typename V2Type>
PROTOBUF_ALWAYS_INLINE inline V1Type ValueBarrier(V1Type value1,
                                                  V2Type value2) {
  asm("" : "+r"(value1) : "r"(value2));
  return value1;
}

// Performs a 7 bit UBFX (Unsigned Bit Extract) starting at the indicated bit.
static PROTOBUF_ALWAYS_INLINE inline uint64_t Ubfx7(uint64_t data,
                                                    uint64_t start) {
  return ValueBarrier((data >> start) & 0x7f);
}

PROTOBUF_ALWAYS_INLINE inline uint64_t ExtractAndMergeTwoChunks(
    uint64_t data, uint64_t first_byte) {
  ABSL_DCHECK_LE(first_byte, 6U);
  uint64_t first = Ubfx7(data, first_byte * 8);
  uint64_t second = Ubfx7(data, (first_byte + 1) * 8);
  return ValueBarrier(first | (second << 7));
}

struct SlowPathEncodedInfo {
  const char* p;
  uint64_t last8;
  uint64_t valid_bits;
  uint64_t valid_chunk_bits;
  uint64_t masked_cont_bits;
};

// Performs multiple actions which are identical between 32 and 64 bit Varints
// in order to compute the length of the encoded Varint and compute the new
// of p.
PROTOBUF_ALWAYS_INLINE inline SlowPathEncodedInfo ComputeLengthAndUpdateP(
    const char* p) {
  SlowPathEncodedInfo result;
  // Load the last two bytes of the encoded Varint.
  std::memcpy(&result.last8, p + 2, sizeof(result.last8));
  uint64_t mask = ValueBarrier(0x8080808080808080);
  // Only set continuation bits remain
  result.masked_cont_bits = ValueBarrier(mask & ~result.last8);
  // The first cleared continuation bit is the most significant 1 in the
  // reversed value.  Result is undefined for an input of 0 and we handle that
  // case below.
  result.valid_bits = absl::countr_zero(result.masked_cont_bits);
  // Calculates the number of chunks in the encoded Varint.  This value is low
  // by three as neither the cleared continuation chunk nor the first two chunks
  // are counted.
  uint64_t set_continuation_bits = result.valid_bits >> 3;
  // Update p to point past the encoded Varint.
  result.p = p + set_continuation_bits + 3;
  // Calculate number of valid data bits in the decoded value so invalid bits
  // can be masked off.  Value is too low by 14 but we account for that when
  // calculating the mask.
  result.valid_chunk_bits = result.valid_bits - set_continuation_bits;
  return result;
}

inline PROTOBUF_ALWAYS_INLINE std::pair<const char*, uint64_t>
VarintParseSlowArm64(const char* p, uint64_t first8) {
  constexpr uint64_t kResultMaskUnshifted = 0xffffffffffffc000ULL;
  constexpr uint64_t kFirstResultBitChunk2 = 2 * 7;
  constexpr uint64_t kFirstResultBitChunk4 = 4 * 7;
  constexpr uint64_t kFirstResultBitChunk6 = 6 * 7;
  constexpr uint64_t kFirstResultBitChunk8 = 8 * 7;

  SlowPathEncodedInfo info = ComputeLengthAndUpdateP(p);
  // Extract data bits from the low six chunks.  This includes chunks zero and
  // one which we already know are valid.
  uint64_t merged_01 = ExtractAndMergeTwoChunks(first8, /*first_chunk=*/0);
  uint64_t merged_23 = ExtractAndMergeTwoChunks(first8, /*first_chunk=*/2);
  uint64_t merged_45 = ExtractAndMergeTwoChunks(first8, /*first_chunk=*/4);
  // Low 42 bits of decoded value.
  uint64_t result = merged_01 | (merged_23 << kFirstResultBitChunk2) |
                    (merged_45 << kFirstResultBitChunk4);
  // This immediate ends in 14 zeroes since valid_chunk_bits is too low by 14.
  uint64_t result_mask = kResultMaskUnshifted << info.valid_chunk_bits;
  //  iff the Varint i invalid.
  if (PROTOBUF_PREDICT_FALSE(info.masked_cont_bits == 0)) {
    return {nullptr, 0};
  }
  // Test for early exit if Varint does not exceed 6 chunks.  Branching on one
  // bit is faster on ARM than via a compare and branch.
  if (PROTOBUF_PREDICT_FALSE((info.valid_bits & 0x20) != 0)) {
    // Extract data bits from high four chunks.
    uint64_t merged_67 = ExtractAndMergeTwoChunks(first8, /*first_chunk=*/6);
    // Last two chunks come from last two bytes of info.last8.
    uint64_t merged_89 =
        ExtractAndMergeTwoChunks(info.last8, /*first_chunk=*/6);
    result |= merged_67 << kFirstResultBitChunk6;
    result |= merged_89 << kFirstResultBitChunk8;
    // Handle an invalid Varint with all 10 continuation bits set.
  }
  // Mask off invalid data bytes.
  result &= ~result_mask;
  return {info.p, result};
}

// See comments in VarintParseSlowArm64 for a description of the algorithm.
// Differences in the 32 bit version are noted below.
inline PROTOBUF_ALWAYS_INLINE std::pair<const char*, uint32_t>
VarintParseSlowArm32(const char* p, uint64_t first8) {
  constexpr uint64_t kResultMaskUnshifted = 0xffffffffffffc000ULL;
  constexpr uint64_t kFirstResultBitChunk1 = 1 * 7;
  constexpr uint64_t kFirstResultBitChunk3 = 3 * 7;

  // This also skips the slop bytes.
  SlowPathEncodedInfo info = ComputeLengthAndUpdateP(p);
  // Extract data bits from chunks 1-4.  Chunk zero is merged in below.
  uint64_t merged_12 = ExtractAndMergeTwoChunks(first8, /*first_chunk=*/1);
  uint64_t merged_34 = ExtractAndMergeTwoChunks(first8, /*first_chunk=*/3);
  first8 = ValueBarrier(first8, p);
  uint64_t result = Ubfx7(first8, /*start=*/0);
  result = ValueBarrier(result | merged_12 << kFirstResultBitChunk1);
  result = ValueBarrier(result | merged_34 << kFirstResultBitChunk3);
  uint64_t result_mask = kResultMaskUnshifted << info.valid_chunk_bits;
  result &= ~result_mask;
  // It is extremely unlikely that a Varint is invalid so checking that
  // condition isn't on the critical path. Here we make sure that we don't do so
  // until result has been computed.
  info.masked_cont_bits = ValueBarrier(info.masked_cont_bits, result);
  if (PROTOBUF_PREDICT_FALSE(info.masked_cont_bits == 0)) {
    return {nullptr, 0};
  }
  return {info.p, result};
}

static const char* VarintParseSlowArm(const char* p, uint32_t* out,
                                      uint64_t first8) {
  auto tmp = VarintParseSlowArm32(p, first8);
  *out = tmp.second;
  return tmp.first;
}

static const char* VarintParseSlowArm(const char* p, uint64_t* out,
                                      uint64_t first8) {
  auto tmp = VarintParseSlowArm64(p, first8);
  *out = tmp.second;
  return tmp.first;
}
#endif

template <typename T>
PROTOBUF_NODISCARD const char* VarintParse(const char* p, T* out) {
#if defined(__aarch64__) && defined(PROTOBUF_LITTLE_ENDIAN)
  // This optimization is not supported in big endian mode
  uint64_t first8;
  std::memcpy(&first8, p, sizeof(first8));
  if (PROTOBUF_PREDICT_TRUE((first8 & 0x80) == 0)) {
    *out = static_cast<uint8_t>(first8);
    return p + 1;
  }
  if (PROTOBUF_PREDICT_TRUE((first8 & 0x8000) == 0)) {
    uint64_t chunk1;
    uint64_t chunk2;
    // Extracting the two chunks this way gives a speedup for this path.
    chunk1 = Ubfx7(first8, 0);
    chunk2 = Ubfx7(first8, 8);
    *out = chunk1 | (chunk2 << 7);
    return p + 2;
  }
  return VarintParseSlowArm(p, out, first8);
#else   // __aarch64__
  auto ptr = reinterpret_cast<const uint8_t*>(p);
  uint32_t res = ptr[0];
  if ((res & 0x80) == 0) {
    *out = res;
    return p + 1;
  }
  return VarintParseSlow(p, res, out);
#endif  // __aarch64__
}

// Used for tags, could read up to 5 bytes which must be available.
// Caller must ensure it's safe to call.

PROTOBUF_EXPORT
std::pair<const char*, uint32_t> ReadTagFallback(const char* p, uint32_t res);

// Same as ParseVarint but only accept 5 bytes at most.
inline const char* ReadTag(const char* p, uint32_t* out,
                           uint32_t /*max_tag*/ = 0) {
  uint32_t res = static_cast<uint8_t>(p[0]);
  if (res < 128) {
    *out = res;
    return p + 1;
  }
  uint32_t second = static_cast<uint8_t>(p[1]);
  res += (second - 1) << 7;
  if (second < 128) {
    *out = res;
    return p + 2;
  }
  auto tmp = ReadTagFallback(p, res);
  *out = tmp.second;
  return tmp.first;
}

// As above, but optimized to consume very few registers while still being fast,
// ReadTagInlined is useful for callers that don't mind the extra code but would
// like to avoid an extern function call causing spills into the stack.
//
// Two support routines for ReadTagInlined come first...
template <class T>
PROTOBUF_NODISCARD PROTOBUF_ALWAYS_INLINE constexpr T RotateLeft(
    T x, int s) noexcept {
  return static_cast<T>(x << (s & (std::numeric_limits<T>::digits - 1))) |
         static_cast<T>(x >> ((-s) & (std::numeric_limits<T>::digits - 1)));
}

PROTOBUF_NODISCARD inline PROTOBUF_ALWAYS_INLINE uint64_t
RotRight7AndReplaceLowByte(uint64_t res, const char& byte) {
  // TODO: remove the inline assembly
#if defined(__x86_64__) && defined(__GNUC__)
  // This will only use one register for `res`.
  // `byte` comes as a reference to allow the compiler to generate code like:
  //
  //   rorq    $7, %rcx
  //   movb    1(%rax), %cl
  //
  // which avoids loading the incoming bytes into a separate register first.
  asm("ror $7,%0\n\t"
      "movb %1,%b0"
      : "+r"(res)
      : "m"(byte));
#else
  res = RotateLeft(res, -7);
  res = res & ~0xFF;
  res |= 0xFF & byte;
#endif
  return res;
}

inline PROTOBUF_ALWAYS_INLINE const char* ReadTagInlined(const char* ptr,
                                                         uint32_t* out) {
  uint64_t res = 0xFF & ptr[0];
  if (PROTOBUF_PREDICT_FALSE(res >= 128)) {
    res = RotRight7AndReplaceLowByte(res, ptr[1]);
    if (PROTOBUF_PREDICT_FALSE(res & 0x80)) {
      res = RotRight7AndReplaceLowByte(res, ptr[2]);
      if (PROTOBUF_PREDICT_FALSE(res & 0x80)) {
        res = RotRight7AndReplaceLowByte(res, ptr[3]);
        if (PROTOBUF_PREDICT_FALSE(res & 0x80)) {
          // Note: this wouldn't work if res were 32-bit,
          // because then replacing the low byte would overwrite
          // the bottom 4 bits of the result.
          res = RotRight7AndReplaceLowByte(res, ptr[4]);
          if (PROTOBUF_PREDICT_FALSE(res & 0x80)) {
            // The proto format does not permit longer than 5-byte encodings for
            // tags.
            *out = 0;
            return nullptr;
          }
          *out = static_cast<uint32_t>(RotateLeft(res, 28));
#if defined(__GNUC__)
          // Note: this asm statement prevents the compiler from
          // trying to share the "return ptr + constant" among all
          // branches.
          asm("" : "+r"(ptr));
#endif
          return ptr + 5;
        }
        *out = static_cast<uint32_t>(RotateLeft(res, 21));
        return ptr + 4;
      }
      *out = static_cast<uint32_t>(RotateLeft(res, 14));
      return ptr + 3;
    }
    *out = static_cast<uint32_t>(RotateLeft(res, 7));
    return ptr + 2;
  }
  *out = static_cast<uint32_t>(res);
  return ptr + 1;
}

// Decode 2 consecutive bytes of a varint and returns the value, shifted left
// by 1. It simultaneous updates *ptr to *ptr + 1 or *ptr + 2 depending if the
// first byte's continuation bit is set.
// If bit 15 of return value is set (equivalent to the continuation bits of both
// bytes being set) the varint continues, otherwise the parse is done. On x86
// movsx eax, dil
// and edi, eax
// add eax, edi
// adc [rsi], 1
inline uint32_t DecodeTwoBytes(const char** ptr) {
  uint32_t value = UnalignedLoad<uint16_t>(*ptr);
  // Sign extend the low byte continuation bit
  uint32_t x = static_cast<int8_t>(value);
  value &= x;  // Mask out the high byte iff no continuation
  // This add is an amazing operation, it cancels the low byte continuation bit
  // from y transferring it to the carry. Simultaneously it also shifts the 7
  // LSB left by one tightly against high byte varint bits. Hence value now
  // contains the unpacked value shifted left by 1.
  value += x;
  // Use the carry to update the ptr appropriately.
  *ptr += value < x ? 2 : 1;
  return value;
}

// More efficient varint parsing for big varints
inline const char* ParseBigVarint(const char* p, uint64_t* out) {
  auto pnew = p;
  auto tmp = DecodeTwoBytes(&pnew);
  uint64_t res = tmp >> 1;
  if (PROTOBUF_PREDICT_TRUE(static_cast<std::int16_t>(tmp) >= 0)) {
    *out = res;
    return pnew;
  }
  for (std::uint32_t i = 1; i < 5; i++) {
    pnew = p + 2 * i;
    tmp = DecodeTwoBytes(&pnew);
    res += (static_cast<std::uint64_t>(tmp) - 2) << (14 * i - 1);
    if (PROTOBUF_PREDICT_TRUE(static_cast<std::int16_t>(tmp) >= 0)) {
      *out = res;
      return pnew;
    }
  }
  return nullptr;
}

PROTOBUF_EXPORT
std::pair<const char*, int32_t> ReadSizeFallback(const char* p, uint32_t first);
// Used for tags, could read up to 5 bytes which must be available. Additionally
// it makes sure the unsigned value fits a int32_t, otherwise returns nullptr.
// Caller must ensure its safe to call.
inline uint32_t ReadSize(const char** pp) {
  auto p = *pp;
  uint32_t res = static_cast<uint8_t>(p[0]);
  if (res < 128) {
    *pp = p + 1;
    return res;
  }
  auto x = ReadSizeFallback(p, res);
  *pp = x.first;
  return x.second;
}

// Some convenience functions to simplify the generated parse loop code.
// Returning the value and updating the buffer pointer allows for nicer
// function composition. We rely on the compiler to inline this.
// Also in debug compiles having local scoped variables tend to generated
// stack frames that scale as O(num fields).
inline uint64_t ReadVarint64(const char** p) {
  uint64_t tmp;
  *p = VarintParse(*p, &tmp);
  return tmp;
}

inline uint32_t ReadVarint32(const char** p) {
  uint32_t tmp;
  *p = VarintParse(*p, &tmp);
  return tmp;
}

inline int64_t ReadVarintZigZag64(const char** p) {
  uint64_t tmp;
  *p = VarintParse(*p, &tmp);
  return WireFormatLite::ZigZagDecode64(tmp);
}

inline int32_t ReadVarintZigZag32(const char** p) {
  uint64_t tmp;
  *p = VarintParse(*p, &tmp);
  return WireFormatLite::ZigZagDecode32(static_cast<uint32_t>(tmp));
}

template <typename T, typename std::enable_if<
                          !std::is_base_of<MessageLite, T>::value, bool>::type>
PROTOBUF_NODISCARD const char* ParseContext::ParseMessage(T* msg,
                                                          const char* ptr) {
  LimitToken old;
  ptr = ReadSizeAndPushLimitAndDepth(ptr, &old);
  if (ptr == nullptr) return ptr;
  auto old_depth = depth_;
  ptr = msg->_InternalParse(ptr, this);
  if (ptr != nullptr) ABSL_DCHECK_EQ(old_depth, depth_);
  depth_++;
  if (!PopLimit(std::move(old))) return nullptr;
  return ptr;
}

template <typename Func>
PROTOBUF_NODISCARD PROTOBUF_ALWAYS_INLINE const char*
ParseContext::ParseLengthDelimitedInlined(const char* ptr, const Func& func) {
  LimitToken old;
  ptr = ReadSizeAndPushLimitAndDepthInlined(ptr, &old);
  if (ptr == nullptr) return ptr;
  PROTOBUF_ALWAYS_INLINE_CALL ptr = func(ptr);
  depth_++;
  if (!PopLimit(std::move(old))) return nullptr;
  return ptr;
}

inline const char* ParseContext::ReadSizeAndPushLimitAndDepthInlined(
    const char* ptr, LimitToken* old_limit) {
  int size = ReadSize(&ptr);
  if (PROTOBUF_PREDICT_FALSE(!ptr) || depth_ <= 0) {
    return nullptr;
  }
  *old_limit = PushLimit(ptr, size);
  --depth_;
  return ptr;
}

template <typename Tag, typename T>
const char* EpsCopyInputStream::ReadRepeatedFixed(const char* ptr,
                                                  Tag expected_tag,
                                                  RepeatedField<T>* out) {
  do {
    out->Add(UnalignedLoad<T>(ptr));
    ptr += sizeof(T);
    if (PROTOBUF_PREDICT_FALSE(ptr >= limit_end_)) return ptr;
  } while (UnalignedLoad<Tag>(ptr) == expected_tag && (ptr += sizeof(Tag)));
  return ptr;
}

// Add any of the following lines to debug which parse function is failing.

#define GOOGLE_PROTOBUF_ASSERT_RETURN(predicate, ret) \
  if (!(predicate)) {                                  \
    /*  ::raise(SIGINT);  */                           \
    /*  ABSL_LOG(ERROR) << "Parse failure";  */        \
    return ret;                                        \
  }

#define GOOGLE_PROTOBUF_PARSER_ASSERT(predicate) \
  GOOGLE_PROTOBUF_ASSERT_RETURN(predicate, nullptr)

template <typename T>
const char* EpsCopyInputStream::ReadPackedFixed(const char* ptr, int size,
                                                RepeatedField<T>* out) {
  GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
  int nbytes = static_cast<int>(buffer_end_ + kSlopBytes - ptr);
  while (size > nbytes) {
    int num = nbytes / sizeof(T);
    int old_entries = out->size();
    out->Reserve(old_entries + num);
    int block_size = num * sizeof(T);
    auto dst = out->AddNAlreadyReserved(num);
#ifdef PROTOBUF_LITTLE_ENDIAN
    std::memcpy(dst, ptr, block_size);
#else
    for (int i = 0; i < num; i++)
      dst[i] = UnalignedLoad<T>(ptr + i * sizeof(T));
#endif
    size -= block_size;
    if (limit_ <= kSlopBytes) return nullptr;
    ptr = Next();
    if (ptr == nullptr) return nullptr;
    ptr += kSlopBytes - (nbytes - block_size);
    nbytes = static_cast<int>(buffer_end_ + kSlopBytes - ptr);
  }
  int num = size / sizeof(T);
  int block_size = num * sizeof(T);
  if (num == 0) return size == block_size ? ptr : nullptr;
  int old_entries = out->size();
  out->Reserve(old_entries + num);
  auto dst = out->AddNAlreadyReserved(num);
#ifdef PROTOBUF_LITTLE_ENDIAN
  ABSL_CHECK(dst != nullptr) << out << "," << num;
  std::memcpy(dst, ptr, block_size);
#else
  for (int i = 0; i < num; i++) dst[i] = UnalignedLoad<T>(ptr + i * sizeof(T));
#endif
  ptr += block_size;
  if (size != block_size) return nullptr;
  return ptr;
}

template <typename Add>
const char* ReadPackedVarintArray(const char* ptr, const char* end, Add add) {
  while (ptr < end) {
    uint64_t varint;
    ptr = VarintParse(ptr, &varint);
    if (ptr == nullptr) return nullptr;
    add(varint);
  }
  return ptr;
}

template <typename Add, typename SizeCb>
const char* EpsCopyInputStream::ReadPackedVarint(const char* ptr, Add add,
                                                 SizeCb size_callback) {
  int size = ReadSize(&ptr);
  size_callback(size);

  GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
  int chunk_size = static_cast<int>(buffer_end_ - ptr);
  while (size > chunk_size) {
    ptr = ReadPackedVarintArray(ptr, buffer_end_, add);
    if (ptr == nullptr) return nullptr;
    int overrun = static_cast<int>(ptr - buffer_end_);
    ABSL_DCHECK(overrun >= 0 && overrun <= kSlopBytes);
    if (size - chunk_size <= kSlopBytes) {
      // The current buffer contains all the information needed, we don't need
      // to flip buffers. However we must parse from a buffer with enough space
      // so we are not prone to a buffer overflow.
      char buf[kSlopBytes + 10] = {};
      std::memcpy(buf, buffer_end_, kSlopBytes);
      ABSL_CHECK_LE(size - chunk_size, kSlopBytes);
      auto end = buf + (size - chunk_size);
      auto res = ReadPackedVarintArray(buf + overrun, end, add);
      if (res == nullptr || res != end) return nullptr;
      return buffer_end_ + (res - buf);
    }
    size -= overrun + chunk_size;
    ABSL_DCHECK_GT(size, 0);
    // We must flip buffers
    if (limit_ <= kSlopBytes) return nullptr;
    ptr = Next();
    if (ptr == nullptr) return nullptr;
    ptr += overrun;
    chunk_size = static_cast<int>(buffer_end_ - ptr);
  }
  auto end = ptr + size;
  ptr = ReadPackedVarintArray(ptr, end, add);
  return end == ptr ? ptr : nullptr;
}

// Helper for verification of utf8
PROTOBUF_EXPORT
bool VerifyUTF8(absl::string_view s, const char* field_name);

inline bool VerifyUTF8(const std::string* s, const char* field_name) {
  return VerifyUTF8(*s, field_name);
}

// All the string parsers with or without UTF checking and for all CTypes.
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* InlineGreedyStringParser(
    std::string* s, const char* ptr, ParseContext* ctx);

PROTOBUF_NODISCARD inline const char* InlineCordParser(::absl::Cord* cord,
                                                       const char* ptr,
                                                       ParseContext* ctx) {
  int size = ReadSize(&ptr);
  if (!ptr) return nullptr;
  return ctx->ReadCord(ptr, size, cord);
}


template <typename T>
PROTOBUF_NODISCARD const char* FieldParser(uint64_t tag, T& field_parser,
                                           const char* ptr, ParseContext* ctx) {
  uint32_t number = tag >> 3;
  GOOGLE_PROTOBUF_PARSER_ASSERT(number != 0);
  using WireType = internal::WireFormatLite::WireType;
  switch (tag & 7) {
    case WireType::WIRETYPE_VARINT: {
      uint64_t value;
      ptr = VarintParse(ptr, &value);
      GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
      field_parser.AddVarint(number, value);
      break;
    }
    case WireType::WIRETYPE_FIXED64: {
      uint64_t value = UnalignedLoad<uint64_t>(ptr);
      ptr += 8;
      field_parser.AddFixed64(number, value);
      break;
    }
    case WireType::WIRETYPE_LENGTH_DELIMITED: {
      ptr = field_parser.ParseLengthDelimited(number, ptr, ctx);
      GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
      break;
    }
    case WireType::WIRETYPE_START_GROUP: {
      ptr = field_parser.ParseGroup(number, ptr, ctx);
      GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
      break;
    }
    case WireType::WIRETYPE_END_GROUP: {
      ABSL_LOG(FATAL) << "Can't happen";
      break;
    }
    case WireType::WIRETYPE_FIXED32: {
      uint32_t value = UnalignedLoad<uint32_t>(ptr);
      ptr += 4;
      field_parser.AddFixed32(number, value);
      break;
    }
    default:
      return nullptr;
  }
  return ptr;
}

template <typename T>
PROTOBUF_NODISCARD const char* WireFormatParser(T& field_parser,
                                                const char* ptr,
                                                ParseContext* ctx) {
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ReadTag(ptr, &tag);
    GOOGLE_PROTOBUF_PARSER_ASSERT(ptr != nullptr);
    if (tag == 0 || (tag & 7) == 4) {
      ctx->SetLastTag(tag);
      return ptr;
    }
    ptr = FieldParser(tag, field_parser, ptr, ctx);
    GOOGLE_PROTOBUF_PARSER_ASSERT(ptr != nullptr);
  }
  return ptr;
}

// The packed parsers parse repeated numeric primitives directly into  the
// corresponding field

// These are packed varints
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedInt32Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedUInt32Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedInt64Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedUInt64Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedSInt32Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedSInt64Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedEnumParser(
    void* object, const char* ptr, ParseContext* ctx);

template <typename T>
PROTOBUF_NODISCARD const char* PackedEnumParser(void* object, const char* ptr,
                                                ParseContext* ctx,
                                                bool (*is_valid)(int),
                                                InternalMetadata* metadata,
                                                int field_num) {
  return ctx->ReadPackedVarint(
      ptr, [object, is_valid, metadata, field_num](int32_t val) {
        if (is_valid(val)) {
          static_cast<RepeatedField<int>*>(object)->Add(val);
        } else {
          WriteVarint(field_num, val, metadata->mutable_unknown_fields<T>());
        }
      });
}

template <typename T>
PROTOBUF_NODISCARD const char* PackedEnumParserArg(
    void* object, const char* ptr, ParseContext* ctx,
    bool (*is_valid)(const void*, int), const void* data,
    InternalMetadata* metadata, int field_num) {
  return ctx->ReadPackedVarint(
      ptr, [object, is_valid, data, metadata, field_num](int32_t val) {
        if (is_valid(data, val)) {
          static_cast<RepeatedField<int>*>(object)->Add(val);
        } else {
          WriteVarint(field_num, val, metadata->mutable_unknown_fields<T>());
        }
      });
}

PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedBoolParser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedFixed32Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedSFixed32Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedFixed64Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedSFixed64Parser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedFloatParser(
    void* object, const char* ptr, ParseContext* ctx);
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* PackedDoubleParser(
    void* object, const char* ptr, ParseContext* ctx);

// This is the only recursive parser.
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* UnknownGroupLiteParse(
    std::string* unknown, const char* ptr, ParseContext* ctx);
// This is a helper to for the UnknownGroupLiteParse but is actually also
// useful in the generated code. It uses overload on std::string* vs
// UnknownFieldSet* to make the generated code isomorphic between full and lite.
PROTOBUF_NODISCARD PROTOBUF_EXPORT const char* UnknownFieldParse(
    uint32_t tag, std::string* unknown, const char* ptr, ParseContext* ctx);

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


#endif  // GOOGLE_PROTOBUF_PARSE_CONTEXT_H__

