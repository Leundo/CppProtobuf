// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google_protobuf_arena.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <typeinfo>
#include <vector>

#include <CppAbseil/absl_base_attributes.hpp>
#include <CppAbseil/absl_container_internal_layout.hpp>
#include <CppAbseil/absl_synchronization_mutex.hpp>
#include "google_protobuf_arena_allocation_policy.hpp"
#include "google_protobuf_arenaz_sampler.hpp"
#include "google_protobuf_port.hpp"
#include "google_protobuf_serial_arena.hpp"
#include "google_protobuf_thread_safe_arena.hpp"


#ifdef ADDRESS_SANITIZER
#include <sanitizer/asan_interface.h>
#endif  // ADDRESS_SANITIZER

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
namespace internal {
namespace {

#if defined(__GNUC__) && __GNUC__ >= 5
// kSentryArenaBlock is used for arenas which can be referenced pre-main. So,
// constexpr is required.
constexpr ArenaBlock kSentryArenaBlock;

ArenaBlock* SentryArenaBlock() {
  // const_cast<> is okay as kSentryArenaBlock will never be mutated.
  return const_cast<ArenaBlock*>(&kSentryArenaBlock);
}
#else
// TODO Remove this once we're not using GCC 4.9 for tests.
// There is a compiler bug in this version that causes the above constexpr to
// fail.  This version is no longer in our support window, but we use it in
// some of our aarch64 docker images.
ArenaBlock* SentryArenaBlock() {
  static const ArenaBlock kSentryArenaBlock;
  // const_cast<> is okay as kSentryArenaBlock will never be mutated.
  return const_cast<ArenaBlock*>(&kSentryArenaBlock);
}
#endif

}  // namespace

static SizedPtr AllocateMemory(const AllocationPolicy* policy_ptr,
                               size_t last_size, size_t min_bytes) {
  AllocationPolicy policy;  // default policy
  if (policy_ptr) policy = *policy_ptr;
  size_t size;
  if (last_size != 0) {
    // Double the current block size, up to a limit.
    auto max_size = policy.max_block_size;
    size = std::min(2 * last_size, max_size);
  } else {
    size = policy.start_block_size;
  }
  // Verify that min_bytes + kBlockHeaderSize won't overflow.
  ABSL_CHECK_LE(min_bytes, std::numeric_limits<size_t>::max() -
                               SerialArena::kBlockHeaderSize);
  size = std::max(size, SerialArena::kBlockHeaderSize + min_bytes);

  if (policy.block_alloc == nullptr) {
    return AllocateAtLeast(size);
  }
  return {policy.block_alloc(size), size};
}

class GetDeallocator {
 public:
  GetDeallocator(const AllocationPolicy* policy, size_t* space_allocated)
      : dealloc_(policy ? policy->block_dealloc : nullptr),
        space_allocated_(space_allocated) {}

  void operator()(SizedPtr mem) const {
#ifdef ADDRESS_SANITIZER
    // This memory was provided by the underlying allocator as unpoisoned,
    // so return it in an unpoisoned state.
    ASAN_UNPOISON_MEMORY_REGION(mem.p, mem.n);
#endif  // ADDRESS_SANITIZER
    if (dealloc_) {
      dealloc_(mem.p, mem.n);
    } else {
      internal::SizedDelete(mem.p, mem.n);
    }
    *space_allocated_ += mem.n;
  }

 private:
  void (*dealloc_)(void*, size_t);
  size_t* space_allocated_;
};

// It is guaranteed that this is constructed in `b`. IOW, this is not the first
// arena and `b` cannot be sentry.
SerialArena::SerialArena(ArenaBlock* b, ThreadSafeArena& parent)
    : ptr_{b->Pointer(kBlockHeaderSize + ThreadSafeArena::kSerialArenaSize)},
      limit_{b->Limit()},
      prefetch_ptr_(
          b->Pointer(kBlockHeaderSize + ThreadSafeArena::kSerialArenaSize)),
      prefetch_limit_(b->Limit()),
      head_{b},
      space_allocated_{b->size},
      parent_{parent} {
  ABSL_DCHECK(!b->IsSentry());
}

// It is guaranteed that this is the first SerialArena. Use sentry block.
SerialArena::SerialArena(ThreadSafeArena& parent)
    : head_{SentryArenaBlock()}, parent_{parent} {}

// It is guaranteed that this is the first SerialArena but `b` may be user
// provided or newly allocated to store AllocationPolicy.
SerialArena::SerialArena(FirstSerialArena, ArenaBlock* b,
                         ThreadSafeArena& parent)
    : head_{b}, space_allocated_{b->size}, parent_{parent} {
  if (b->IsSentry()) return;
  set_range(b->Pointer(kBlockHeaderSize), b->Limit());
}

std::vector<void*> SerialArena::PeekCleanupListForTesting() {
  std::vector<void*> res;

  ArenaBlock* b = head();
  if (b->IsSentry()) return res;

  const auto peek_list = [&](const char* pos, const char* end) {
    while (pos != end) {
      pos += cleanup::PeekNode(pos, res);
    }
  };

  peek_list(limit_, b->Limit());
  for (b = b->next; b; b = b->next) {
    peek_list(reinterpret_cast<char*>(b->cleanup_nodes), b->Limit());
  }
  return res;
}

std::vector<void*> ThreadSafeArena::PeekCleanupListForTesting() {
  return GetSerialArena()->PeekCleanupListForTesting();
}

void SerialArena::Init(ArenaBlock* b, size_t offset) {
  set_range(b->Pointer(offset), b->Limit());
  head_.store(b, std::memory_order_relaxed);
  space_used_.store(0, std::memory_order_relaxed);
  space_allocated_.store(b->size, std::memory_order_relaxed);
  cached_block_length_ = 0;
  cached_blocks_ = nullptr;
  string_block_.store(nullptr, std::memory_order_relaxed);
  string_block_unused_.store(0, std::memory_order_relaxed);
}

SerialArena* SerialArena::New(SizedPtr mem, ThreadSafeArena& parent) {
  ABSL_DCHECK_LE(kBlockHeaderSize + ThreadSafeArena::kSerialArenaSize, mem.n);
  ThreadSafeArenaStats::RecordAllocateStats(parent.arena_stats_.MutableStats(),
                                            /*used=*/0, /*allocated=*/mem.n,
                                            /*wasted=*/0);
  auto b = new (mem.p) ArenaBlock{nullptr, mem.n};
  return new (b->Pointer(kBlockHeaderSize)) SerialArena(b, parent);
}

template <typename Deallocator>
SizedPtr SerialArena::Free(Deallocator deallocator) {
  ArenaBlock* b = head();
  SizedPtr mem = {b, b->size};
  while (b->next) {
    b = b->next;  // We must first advance before deleting this block
    deallocator(mem);
    mem = {b, b->size};
  }
  return mem;
}

PROTOBUF_NOINLINE
void* SerialArena::AllocateAlignedFallback(size_t n) {
  AllocateNewBlock(n);
  void* ret;
  bool res = MaybeAllocateAligned(n, &ret);
  ABSL_DCHECK(res);
  return ret;
}

PROTOBUF_NOINLINE
void* SerialArena::AllocateFromStringBlockFallback() {
  ABSL_DCHECK_EQ(string_block_unused_.load(std::memory_order_relaxed), 0U);
  StringBlock* sb = string_block_.load(std::memory_order_relaxed);
  if (sb) {
    AddSpaceUsed(sb->effective_size());
  }

  void* ptr;
  StringBlock* new_sb;
  size_t size = StringBlock::NextSize(sb);
  if (MaybeAllocateAligned(size, &ptr)) {
    // Correct space_used_ to avoid double counting
    AddSpaceUsed(-size);
    new_sb = StringBlock::Emplace(ptr, size, sb);
  } else {
    new_sb = StringBlock::New(sb);
    AddSpaceAllocated(new_sb->allocated_size());
  }
  string_block_.store(new_sb, std::memory_order_release);
  size_t unused = new_sb->effective_size() - sizeof(std::string);
  string_block_unused_.store(unused, std::memory_order_relaxed);
  return new_sb->AtOffset(unused);
}

PROTOBUF_NOINLINE
void* SerialArena::AllocateAlignedWithCleanupFallback(
    size_t n, size_t align, void (*destructor)(void*)) {
  size_t required = AlignUpTo(n, align) + cleanup::Size(destructor);
  AllocateNewBlock(required);
  return AllocateAlignedWithCleanup(n, align, destructor);
}

PROTOBUF_NOINLINE
void SerialArena::AddCleanupFallback(void* elem, void (*destructor)(void*)) {
  size_t required = cleanup::Size(destructor);
  AllocateNewBlock(required);
  AddCleanupFromExisting(elem, destructor);
}

void SerialArena::AllocateNewBlock(size_t n) {
  size_t used = 0;
  size_t wasted = 0;
  ArenaBlock* old_head = head();
  if (!old_head->IsSentry()) {
    // Sync limit to block
    old_head->cleanup_nodes = limit_;

    // Record how much used in this block.
    used = static_cast<size_t>(ptr() - old_head->Pointer(kBlockHeaderSize));
    wasted = old_head->size - used - kBlockHeaderSize;
    AddSpaceUsed(used);
  }

  // TODO: Evaluate if pushing unused space into the cached blocks is a
  // win. In preliminary testing showed increased memory savings as expected,
  // but with a CPU regression. The regression might have been an artifact of
  // the microbenchmark.

  auto mem = AllocateMemory(parent_.AllocPolicy(), old_head->size, n);
  // We don't want to emit an expensive RMW instruction that requires
  // exclusive access to a cacheline. Hence we write it in terms of a
  // regular add.
  AddSpaceAllocated(mem.n);
  ThreadSafeArenaStats::RecordAllocateStats(parent_.arena_stats_.MutableStats(),
                                            /*used=*/used,
                                            /*allocated=*/mem.n, wasted);
  auto* new_head = new (mem.p) ArenaBlock{old_head, mem.n};
  set_range(new_head->Pointer(kBlockHeaderSize), new_head->Limit());
  // Previous writes must take effect before writing new head.
  head_.store(new_head, std::memory_order_release);

#ifdef ADDRESS_SANITIZER
  ASAN_POISON_MEMORY_REGION(ptr(), limit_ - ptr());
#endif  // ADDRESS_SANITIZER
}

uint64_t SerialArena::SpaceUsed() const {
  // Note: the calculation below technically causes a race with
  // AllocateNewBlock when called from another thread (which happens in
  // ThreadSafeArena::SpaceUsed).  However, worst-case space_used_ will have
  // stale data and the calculation will incorrectly assume 100%
  // usage of the *current* block.
  // TODO Consider eliminating this race in exchange for a possible
  // performance hit on ARM (see cl/455186837).

  uint64_t space_used = 0;
  StringBlock* sb = string_block_.load(std::memory_order_acquire);
  if (sb) {
    size_t unused = string_block_unused_.load(std::memory_order_relaxed);
    space_used += sb->effective_size() - unused;
  }
  const ArenaBlock* h = head_.load(std::memory_order_acquire);
  if (h->IsSentry()) return space_used;

  const uint64_t current_block_size = h->size;
  space_used += std::min(
      static_cast<uint64_t>(
          ptr() - const_cast<ArenaBlock*>(h)->Pointer(kBlockHeaderSize)),
      current_block_size);
  return space_used + space_used_.load(std::memory_order_relaxed);
}

size_t SerialArena::FreeStringBlocks(StringBlock* string_block,
                                     size_t unused_bytes) {
  ABSL_DCHECK(string_block != nullptr);
  StringBlock* next = string_block->next();
  std::string* end = string_block->end();
  for (std::string* s = string_block->AtOffset(unused_bytes); s != end; ++s) {
    s->~basic_string();
  }
  size_t deallocated = StringBlock::Delete(string_block);

  while ((string_block = next) != nullptr) {
    next = string_block->next();
    for (std::string& s : *string_block) {
      s.~basic_string();
    }
    deallocated += StringBlock::Delete(string_block);
  }
  return deallocated;
}

void SerialArena::CleanupList() {
  ArenaBlock* b = head();
  if (b->IsSentry()) return;

  b->cleanup_nodes = limit_;
  do {
    char* limit = b->Limit();
    char* it = reinterpret_cast<char*>(b->cleanup_nodes);
    ABSL_DCHECK(!b->IsSentry() || it == limit);
    while (it < limit) {
      it += cleanup::DestroyNode(it);
    }
    b = b->next;
  } while (b);
}

// Stores arrays of void* and SerialArena* instead of linked list of
// SerialArena* to speed up traversing all SerialArena. The cost of walk is non
// trivial when there are many nodes. Separately storing "ids" minimizes cache
// footprints and more efficient when looking for matching arena.
//
// Uses absl::container_internal::Layout to emulate the following:
//
// struct SerialArenaChunk {
//   struct SerialArenaChunkHeader {
//     SerialArenaChunk* next_chunk;
//     uint32_t capacity;
//     std::atomic<uint32_t> size;
//   } header;
//   std::atomic<void*> ids[];
//   std::atomic<SerialArena*> arenas[];
// };
//
// where the size of "ids" and "arenas" is determined at runtime; hence the use
// of Layout.
struct SerialArenaChunkHeader {
  constexpr SerialArenaChunkHeader(uint32_t capacity, uint32_t size)
      : next_chunk(nullptr), capacity(capacity), size(size) {}

  ThreadSafeArena::SerialArenaChunk* next_chunk;
  uint32_t capacity;
  std::atomic<uint32_t> size;
};

class ThreadSafeArena::SerialArenaChunk {
 public:
  SerialArenaChunk(uint32_t capacity, void* me, SerialArena* serial) {
    new (&header()) SerialArenaChunkHeader{capacity, 1};

    new (&id(0)) std::atomic<void*>{me};
    for (uint32_t i = 1; i < capacity; ++i) {
      new (&id(i)) std::atomic<void*>{nullptr};
    }

    new (&arena(0)) std::atomic<SerialArena*>{serial};
    for (uint32_t i = 1; i < capacity; ++i) {
      new (&arena(i)) std::atomic<void*>{nullptr};
    }
  }

  bool IsSentry() const { return capacity() == 0; }

  // next_chunk
  const SerialArenaChunk* next_chunk() const { return header().next_chunk; }
  SerialArenaChunk* next_chunk() { return header().next_chunk; }
  void set_next(SerialArenaChunk* next_chunk) {
    header().next_chunk = next_chunk;
  }

  // capacity
  uint32_t capacity() const { return header().capacity; }
  void set_capacity(uint32_t capacity) { header().capacity = capacity; }

  // ids: returns up to size().
  absl::Span<const std::atomic<void*>> ids() const {
    return Layout(capacity()).Slice<kIds>(ptr()).first(safe_size());
  }
  absl::Span<std::atomic<void*>> ids() {
    return Layout(capacity()).Slice<kIds>(ptr()).first(safe_size());
  }
  std::atomic<void*>& id(uint32_t i) {
    ABSL_DCHECK_LT(i, capacity());
    return Layout(capacity()).Pointer<kIds>(ptr())[i];
  }

  // arenas: returns up to size().
  absl::Span<const std::atomic<SerialArena*>> arenas() const {
    return Layout(capacity()).Slice<kArenas>(ptr()).first(safe_size());
  }
  absl::Span<std::atomic<SerialArena*>> arenas() {
    return Layout(capacity()).Slice<kArenas>(ptr()).first(safe_size());
  }
  const std::atomic<SerialArena*>& arena(uint32_t i) const {
    ABSL_DCHECK_LT(i, capacity());
    return Layout(capacity()).Pointer<kArenas>(ptr())[i];
  }
  std::atomic<SerialArena*>& arena(uint32_t i) {
    ABSL_DCHECK_LT(i, capacity());
    return Layout(capacity()).Pointer<kArenas>(ptr())[i];
  }

  // Tries to insert {id, serial} to head chunk. Returns false if the head is
  // already full.
  //
  // Note that the updating "size", "id", "arena" is individually atomic but
  // those are not protected by a mutex. This is acceptable because concurrent
  // lookups from SpaceUsed or SpaceAllocated accept inaccuracy due to race. On
  // other paths, either race is not possible (GetSerialArenaFallback) or must
  // be prevented by users (CleanupList, Free).
  bool insert(void* me, SerialArena* serial) {
    uint32_t idx = size().fetch_add(1, std::memory_order_relaxed);
    // Bail out if this chunk is full.
    if (idx >= capacity()) {
      // Write old value back to avoid potential overflow.
      size().store(capacity(), std::memory_order_relaxed);
      return false;
    }

    id(idx).store(me, std::memory_order_relaxed);
    arena(idx).store(serial, std::memory_order_release);
    return true;
  }

  constexpr static size_t AllocSize(size_t n) { return Layout(n).AllocSize(); }

 private:
  constexpr static int kHeader = 0;
  constexpr static int kIds = 1;
  constexpr static int kArenas = 2;

  using layout_type = absl::container_internal::Layout<
      SerialArenaChunkHeader, std::atomic<void*>, std::atomic<SerialArena*>>;

  const char* ptr() const { return reinterpret_cast<const char*>(this); }
  char* ptr() { return reinterpret_cast<char*>(this); }

  SerialArenaChunkHeader& header() {
    return *layout_type::Partial().Pointer<kHeader>(ptr());
  }
  const SerialArenaChunkHeader& header() const {
    return *layout_type::Partial().Pointer<kHeader>(ptr());
  }

  std::atomic<uint32_t>& size() { return header().size; }
  const std::atomic<uint32_t>& size() const { return header().size; }

  // Returns the size capped by the capacity as fetch_add may result in a size
  // greater than capacity.
  uint32_t safe_size() const {
    return std::min(capacity(), size().load(std::memory_order_relaxed));
  }

  constexpr static layout_type Layout(size_t n) {
    return layout_type(
        /*header*/ 1,
        /*ids*/ n,
        /*arenas*/ n);
  }
};

constexpr SerialArenaChunkHeader kSentryArenaChunk = {0, 0};

ThreadSafeArena::SerialArenaChunk* ThreadSafeArena::SentrySerialArenaChunk() {
  // const_cast is okay because the sentry chunk is never mutated. Also,
  // reinterpret_cast is acceptable here as it should be identical to
  // SerialArenaChunk with zero payload. This is a necessary trick to
  // constexpr initialize kSentryArenaChunk.
  return reinterpret_cast<SerialArenaChunk*>(
      const_cast<SerialArenaChunkHeader*>(&kSentryArenaChunk));
}


alignas(kCacheAlignment) ABSL_CONST_INIT
    std::atomic<ThreadSafeArena::LifecycleId> ThreadSafeArena::lifecycle_id_{0};
#if defined(PROTOBUF_NO_THREADLOCAL)
ThreadSafeArena::ThreadCache& ThreadSafeArena::thread_cache() {
  static internal::ThreadLocalStorage<ThreadCache>* thread_cache_ =
      new internal::ThreadLocalStorage<ThreadCache>();
  return *thread_cache_->Get();
}
#elif defined(PROTOBUF_USE_DLLS) && defined(_WIN32)
ThreadSafeArena::ThreadCache& ThreadSafeArena::thread_cache() {
  static PROTOBUF_THREAD_LOCAL ThreadCache thread_cache;
  return thread_cache;
}
#else
PROTOBUF_CONSTINIT PROTOBUF_THREAD_LOCAL
    ThreadSafeArena::ThreadCache ThreadSafeArena::thread_cache_;
#endif

ThreadSafeArena::ThreadSafeArena() : first_arena_(*this) { Init(); }

ThreadSafeArena::ThreadSafeArena(char* mem, size_t size)
    : first_arena_(FirstSerialArena{}, FirstBlock(mem, size), *this) {
  Init();
}

ThreadSafeArena::ThreadSafeArena(void* mem, size_t size,
                                 const AllocationPolicy& policy)
    : first_arena_(FirstSerialArena{}, FirstBlock(mem, size, policy), *this) {
  InitializeWithPolicy(policy);
}

ArenaBlock* ThreadSafeArena::FirstBlock(void* buf, size_t size) {
  ABSL_DCHECK_EQ(reinterpret_cast<uintptr_t>(buf) & 7, 0u);
  if (buf == nullptr || size <= kBlockHeaderSize) {
    return SentryArenaBlock();
  }
  // Record user-owned block.
  alloc_policy_.set_is_user_owned_initial_block(true);
  return new (buf) ArenaBlock{nullptr, size};
}

ArenaBlock* ThreadSafeArena::FirstBlock(void* buf, size_t size,
                                        const AllocationPolicy& policy) {
  if (policy.IsDefault()) return FirstBlock(buf, size);

  ABSL_DCHECK_EQ(reinterpret_cast<uintptr_t>(buf) & 7, 0u);

  SizedPtr mem;
  if (buf == nullptr || size < kBlockHeaderSize + kAllocPolicySize) {
    mem = AllocateMemory(&policy, 0, kAllocPolicySize);
  } else {
    mem = {buf, size};
    // Record user-owned block.
    alloc_policy_.set_is_user_owned_initial_block(true);
  }

  return new (mem.p) ArenaBlock{nullptr, mem.n};
}

void ThreadSafeArena::InitializeWithPolicy(const AllocationPolicy& policy) {
  Init();

  if (policy.IsDefault()) return;

#ifndef NDEBUG
  const uint64_t old_alloc_policy = alloc_policy_.get_raw();
  // If there was a policy (e.g., in Reset()), make sure flags were preserved.
#define ABSL_DCHECK_POLICY_FLAGS_() \
  if (old_alloc_policy > 3)         \
  ABSL_CHECK_EQ(old_alloc_policy & 3, alloc_policy_.get_raw() & 3)
#else
#define ABSL_DCHECK_POLICY_FLAGS_()
#endif  // NDEBUG

  // We ensured enough space so this cannot fail.
  void* p;
  if (!first_arena_.MaybeAllocateAligned(kAllocPolicySize, &p)) {
    ABSL_LOG(FATAL) << "MaybeAllocateAligned cannot fail here.";
    return;
  }
  new (p) AllocationPolicy{policy};
  // Low bits store flags, so they mustn't be overwritten.
  ABSL_DCHECK_EQ(0u, reinterpret_cast<uintptr_t>(p) & 3);
  alloc_policy_.set_policy(reinterpret_cast<AllocationPolicy*>(p));
  ABSL_DCHECK_POLICY_FLAGS_();

#undef ABSL_DCHECK_POLICY_FLAGS_
}

uint64_t ThreadSafeArena::GetNextLifeCycleId() {
  ThreadCache& tc = thread_cache();
  uint64_t id = tc.next_lifecycle_id;
  constexpr uint64_t kInc = ThreadCache::kPerThreadIds;
  if (PROTOBUF_PREDICT_FALSE((id & (kInc - 1)) == 0)) {
    // On platforms that don't support uint64_t atomics we can certainly not
    // afford to increment by large intervals and expect uniqueness due to
    // wrapping, hence we only add by 1.
    id = lifecycle_id_.fetch_add(1, std::memory_order_relaxed) * kInc;
  }
  tc.next_lifecycle_id = id + 1;
  return id;
}

// We assume that #threads / arena is bimodal; i.e. majority small ones are
// single threaded but some big ones are highly concurrent. To balance between
// memory overhead and minimum pointer chasing, we start with few entries and
// exponentially (4x) grow with a limit (255 entries). Note that parameters are
// picked for x64 architectures as hint and the actual size is calculated by
// Layout.
ThreadSafeArena::SerialArenaChunk* ThreadSafeArena::NewSerialArenaChunk(
    uint32_t prev_capacity, void* id, SerialArena* serial) {
  constexpr size_t kMaxBytes = 4096;  // Can hold up to 255 entries.
  constexpr size_t kGrowthFactor = 4;
  constexpr size_t kHeaderSize = SerialArenaChunk::AllocSize(0);
  constexpr size_t kEntrySize = SerialArenaChunk::AllocSize(1) - kHeaderSize;

  // On x64 arch: {4, 16, 64, 256, 256, ...} * 16.
  size_t prev_bytes = SerialArenaChunk::AllocSize(prev_capacity);
  size_t next_bytes = std::min(kMaxBytes, prev_bytes * kGrowthFactor);
  uint32_t next_capacity =
      static_cast<uint32_t>(next_bytes - kHeaderSize) / kEntrySize;
  // Growth based on bytes needs to be adjusted by AllocSize.
  next_bytes = SerialArenaChunk::AllocSize(next_capacity);

  // If we allocate bigger memory than requested, we should expand
  // size to use that extra space, and add extra entries permitted
  // by the extra space.
  SizedPtr mem = AllocateAtLeast(next_bytes);
  next_capacity = static_cast<uint32_t>(mem.n - kHeaderSize) / kEntrySize;
  ABSL_DCHECK_LE(SerialArenaChunk::AllocSize(next_capacity), mem.n);
  return new (mem.p) SerialArenaChunk{next_capacity, id, serial};
}

// Tries to reserve an entry by atomic fetch_add. If the head chunk is already
// full (size >= capacity), acquires the mutex and adds a new head.
void ThreadSafeArena::AddSerialArena(void* id, SerialArena* serial) {
  SerialArenaChunk* head = head_.load(std::memory_order_acquire);
  // Fast path without acquiring mutex.
  if (!head->IsSentry() && head->insert(id, serial)) {
    return;
  }

  // Slow path with acquiring mutex.
  absl::MutexLock lock(&mutex_);

  // Refetch and if someone else installed a new head, try allocating on that!
  SerialArenaChunk* new_head = head_.load(std::memory_order_acquire);
  if (new_head != head) {
    if (new_head->insert(id, serial)) return;
    // Update head to link to the latest one.
    head = new_head;
  }

  new_head = NewSerialArenaChunk(head->capacity(), id, serial);
  new_head->set_next(head);

  // Use "std::memory_order_release" to make sure prior stores are visible after
  // this one.
  head_.store(new_head, std::memory_order_release);
}

void ThreadSafeArena::Init() {
  tag_and_id_ = GetNextLifeCycleId();
  arena_stats_ = Sample();
  head_.store(SentrySerialArenaChunk(), std::memory_order_relaxed);
  first_owner_ = &thread_cache();

  // Record allocation for the first block that was either user-provided or
  // newly allocated.
  ThreadSafeArenaStats::RecordAllocateStats(
      arena_stats_.MutableStats(),
      /*used=*/0,
      /*allocated=*/first_arena_.SpaceAllocated(),
      /*wasted=*/0);

  CacheSerialArena(&first_arena_);
}

ThreadSafeArena::~ThreadSafeArena() {
  // Have to do this in a first pass, because some of the destructors might
  // refer to memory in other blocks.
  CleanupList();

  size_t space_allocated = 0;
  auto mem = Free(&space_allocated);
  if (alloc_policy_.is_user_owned_initial_block()) {
#ifdef ADDRESS_SANITIZER
    // Unpoison the initial block, now that it's going back to the user.
    ASAN_UNPOISON_MEMORY_REGION(mem.p, mem.n);
#endif  // ADDRESS_SANITIZER
    space_allocated += mem.n;
  } else if (mem.n > 0) {
    GetDeallocator(alloc_policy_.get(), &space_allocated)(mem);
  }
}

SizedPtr ThreadSafeArena::Free(size_t* space_allocated) {
  auto deallocator = GetDeallocator(alloc_policy_.get(), space_allocated);

  WalkSerialArenaChunk([&](SerialArenaChunk* chunk) {
    absl::Span<std::atomic<SerialArena*>> span = chunk->arenas();
    // Walks arenas backward to handle the first serial arena the last. Freeing
    // in reverse-order to the order in which objects were created may not be
    // necessary to Free and we should revisit this. (b/247560530)
    for (auto it = span.rbegin(); it != span.rend(); ++it) {
      SerialArena* serial = it->load(std::memory_order_relaxed);
      ABSL_DCHECK_NE(serial, nullptr);
      // Free string blocks
      *space_allocated += serial->FreeStringBlocks();
      // Always frees the first block of "serial" as it cannot be user-provided.
      SizedPtr mem = serial->Free(deallocator);
      ABSL_DCHECK_NE(mem.p, nullptr);
      deallocator(mem);
    }

    // Delete the chunk as we're done with it.
    internal::SizedDelete(chunk,
                          SerialArenaChunk::AllocSize(chunk->capacity()));
  });

  // The first block of the first arena is special and let the caller handle it.
  *space_allocated += first_arena_.FreeStringBlocks();
  return first_arena_.Free(deallocator);
}

uint64_t ThreadSafeArena::Reset() {
  // Have to do this in a first pass, because some of the destructors might
  // refer to memory in other blocks.
  CleanupList();

  // Discard all blocks except the first one. Whether it is user-provided or
  // allocated, always reuse the first block for the first arena.
  size_t space_allocated = 0;
  auto mem = Free(&space_allocated);
  space_allocated += mem.n;

  // Reset the first arena with the first block. This avoids redundant
  // free / allocation and re-allocating for AllocationPolicy. Adjust offset if
  // we need to preserve alloc_policy_.
  if (alloc_policy_.is_user_owned_initial_block() ||
      alloc_policy_.get() != nullptr) {
    size_t offset = alloc_policy_.get() == nullptr
                        ? kBlockHeaderSize
                        : kBlockHeaderSize + kAllocPolicySize;
    first_arena_.Init(new (mem.p) ArenaBlock{nullptr, mem.n}, offset);
  } else {
    first_arena_.Init(SentryArenaBlock(), 0);
  }

  // Since the first block and potential alloc_policy on the first block is
  // preserved, this can be initialized by Init().
  Init();

  return space_allocated;
}

void* ThreadSafeArena::AllocateAlignedWithCleanup(size_t n, size_t align,
                                                  void (*destructor)(void*)) {
  SerialArena* arena;
  if (PROTOBUF_PREDICT_TRUE(GetSerialArenaFast(&arena))) {
    return arena->AllocateAlignedWithCleanup(n, align, destructor);
  } else {
    return AllocateAlignedWithCleanupFallback(n, align, destructor);
  }
}

void ThreadSafeArena::AddCleanup(void* elem, void (*cleanup)(void*)) {
  GetSerialArena()->AddCleanup(elem, cleanup);
}

SerialArena* ThreadSafeArena::GetSerialArena() {
  SerialArena* arena;
  if (PROTOBUF_PREDICT_FALSE(!GetSerialArenaFast(&arena))) {
    arena = GetSerialArenaFallback(kMaxCleanupNodeSize);
  }
  return arena;
}

PROTOBUF_NOINLINE
void* ThreadSafeArena::AllocateAlignedWithCleanupFallback(
    size_t n, size_t align, void (*destructor)(void*)) {
  return GetSerialArenaFallback(n + kMaxCleanupNodeSize)
      ->AllocateAlignedWithCleanup(n, align, destructor);
}

PROTOBUF_NOINLINE
void* ThreadSafeArena::AllocateFromStringBlock() {
  return GetSerialArena()->AllocateFromStringBlock();
}

template <typename Functor>
void ThreadSafeArena::WalkConstSerialArenaChunk(Functor fn) const {
  const SerialArenaChunk* chunk = head_.load(std::memory_order_acquire);

  for (; !chunk->IsSentry(); chunk = chunk->next_chunk()) {
    fn(chunk);
  }
}

template <typename Functor>
void ThreadSafeArena::WalkSerialArenaChunk(Functor fn) {
  // By omitting an Acquire barrier we help the sanitizer that any user code
  // that doesn't properly synchronize Reset() or the destructor will throw a
  // TSAN warning.
  SerialArenaChunk* chunk = head_.load(std::memory_order_relaxed);

  while (!chunk->IsSentry()) {
    // Cache next chunk in case this chunk is destroyed.
    SerialArenaChunk* next_chunk = chunk->next_chunk();
    fn(chunk);
    chunk = next_chunk;
  }
}

template <typename Functor>
void ThreadSafeArena::PerConstSerialArenaInChunk(Functor fn) const {
  WalkConstSerialArenaChunk([&fn](const SerialArenaChunk* chunk) {
    for (const auto& each : chunk->arenas()) {
      const SerialArena* serial = each.load(std::memory_order_acquire);
      // It is possible that newly added SerialArena is not updated although
      // size was. This is acceptable for SpaceAllocated and SpaceUsed.
      if (serial == nullptr) continue;
      fn(serial);
    }
  });
}

uint64_t ThreadSafeArena::SpaceAllocated() const {
  uint64_t space_allocated = first_arena_.SpaceAllocated();
  PerConstSerialArenaInChunk([&space_allocated](const SerialArena* serial) {
    space_allocated += serial->SpaceAllocated();
  });
  return space_allocated;
}

uint64_t ThreadSafeArena::SpaceUsed() const {
  // First arena is inlined to ThreadSafeArena and the first block's overhead is
  // smaller than others that contain SerialArena.
  uint64_t space_used = first_arena_.SpaceUsed();
  PerConstSerialArenaInChunk([&space_used](const SerialArena* serial) {
    // SerialArena on chunks directly allocated from the block and needs to be
    // subtracted from SpaceUsed.
    space_used += serial->SpaceUsed() - kSerialArenaSize;
  });
  return space_used - (alloc_policy_.get() ? sizeof(AllocationPolicy) : 0);
}

template <AllocationClient alloc_client>
PROTOBUF_NOINLINE void* ThreadSafeArena::AllocateAlignedFallback(size_t n) {
  return GetSerialArenaFallback(n)->AllocateAligned<alloc_client>(n);
}

template void* ThreadSafeArena::AllocateAlignedFallback<
    AllocationClient::kDefault>(size_t);
template void*
    ThreadSafeArena::AllocateAlignedFallback<AllocationClient::kArray>(size_t);

void ThreadSafeArena::CleanupList() {
  WalkSerialArenaChunk([](SerialArenaChunk* chunk) {
    absl::Span<std::atomic<SerialArena*>> span = chunk->arenas();
    // Walks arenas backward to handle the first serial arena the last.
    // Destroying in reverse-order to the construction is often assumed by users
    // and required not to break inter-object dependencies. (b/247560530)
    for (auto it = span.rbegin(); it != span.rend(); ++it) {
      SerialArena* serial = it->load(std::memory_order_relaxed);
      ABSL_DCHECK_NE(serial, nullptr);
      serial->CleanupList();
    }
  });
  // First arena must be cleaned up last. (b/247560530)
  first_arena_.CleanupList();
}

PROTOBUF_NOINLINE
SerialArena* ThreadSafeArena::GetSerialArenaFallback(size_t n) {
  void* const id = &thread_cache();
  if (id == first_owner_) {
    CacheSerialArena(&first_arena_);
    return &first_arena_;
  }

  // Search matching SerialArena.
  SerialArena* serial = nullptr;
  WalkConstSerialArenaChunk([&serial, id](const SerialArenaChunk* chunk) {
    absl::Span<const std::atomic<void*>> ids = chunk->ids();
    for (uint32_t i = 0; i < ids.size(); ++i) {
      if (ids[i].load(std::memory_order_relaxed) == id) {
        serial = chunk->arena(i).load(std::memory_order_relaxed);
        ABSL_DCHECK_NE(serial, nullptr);
        break;
      }
    }
  });

  if (!serial) {
    // This thread doesn't have any SerialArena, which also means it doesn't
    // have any blocks yet.  So we'll allocate its first block now. It must be
    // big enough to host SerialArena and the pending request.
    serial = SerialArena::New(
        AllocateMemory(alloc_policy_.get(), 0, n + kSerialArenaSize), *this);

    AddSerialArena(id, serial);
  }

  CacheSerialArena(serial);
  return serial;
}

}  // namespace internal

void* Arena::Allocate(size_t n) { return impl_.AllocateAligned(n); }

void* Arena::AllocateForArray(size_t n) {
  return impl_.AllocateAligned<internal::AllocationClient::kArray>(n);
}

void* Arena::AllocateAlignedWithCleanup(size_t n, size_t align,
                                        void (*destructor)(void*)) {
  return impl_.AllocateAlignedWithCleanup(n, align, destructor);
}

std::vector<void*> Arena::PeekCleanupListForTesting() {
  return impl_.PeekCleanupListForTesting();
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


