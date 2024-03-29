// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_MAP_TYPE_HANDLER_H__
#define GOOGLE_PROTOBUF_MAP_TYPE_HANDLER_H__

#include <cstdint>
#include <type_traits>

#include <CppProtobuf/google_protobuf_arena.hpp>
#include <CppProtobuf/google_protobuf_arenastring.hpp>
#include <CppProtobuf/google_protobuf_io_coded_stream.hpp>
#include <CppProtobuf/google_protobuf_parse_context.hpp>
#include <CppProtobuf/google_protobuf_wire_format_lite.hpp>

#ifdef SWIG
#error "You cannot SWIG proto headers"
#endif

namespace google {
namespace protobuf {
namespace internal {

// Define constants for given wire field type
template <WireFormatLite::FieldType field_type, typename Type>
class MapWireFieldTypeTraits {};

#define TYPE_TRAITS(FieldType, CType, WireFormatType)                    \
  template <typename Type>                                               \
  class MapWireFieldTypeTraits<WireFormatLite::TYPE_##FieldType, Type> { \
   public:                                                               \
    using TypeOnMemory =                                                 \
        std::conditional_t<WireFormatLite::TYPE_##FieldType ==           \
                               WireFormatLite::TYPE_MESSAGE,             \
                           Type*, CType>;                                \
    using MapEntryAccessorType =                                         \
        std::conditional_t<std::is_enum<Type>::value, int, Type>;        \
    static const WireFormatLite::WireType kWireType =                    \
        WireFormatLite::WIRETYPE_##WireFormatType;                       \
  };

TYPE_TRAITS(MESSAGE, Type, LENGTH_DELIMITED)
TYPE_TRAITS(STRING, ArenaStringPtr, LENGTH_DELIMITED)
TYPE_TRAITS(BYTES, ArenaStringPtr, LENGTH_DELIMITED)
TYPE_TRAITS(INT64, int64_t, VARINT)
TYPE_TRAITS(UINT64, uint64_t, VARINT)
TYPE_TRAITS(INT32, int32_t, VARINT)
TYPE_TRAITS(UINT32, uint32_t, VARINT)
TYPE_TRAITS(SINT64, int64_t, VARINT)
TYPE_TRAITS(SINT32, int32_t, VARINT)
TYPE_TRAITS(ENUM, int, VARINT)
TYPE_TRAITS(DOUBLE, double, FIXED64)
TYPE_TRAITS(FLOAT, float, FIXED32)
TYPE_TRAITS(FIXED64, uint64_t, FIXED64)
TYPE_TRAITS(FIXED32, uint32_t, FIXED32)
TYPE_TRAITS(SFIXED64, int64_t, FIXED64)
TYPE_TRAITS(SFIXED32, int32_t, FIXED32)
TYPE_TRAITS(BOOL, bool, VARINT)

#undef TYPE_TRAITS

template <WireFormatLite::FieldType field_type, typename Type>
class MapTypeHandler;

template <typename Type>
class MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type> {
 public:
  // Enum type cannot be used for MapTypeHandler::Read. Define a type which will
  // replace Enum with int.
  typedef typename MapWireFieldTypeTraits<WireFormatLite::TYPE_MESSAGE,
                                          Type>::MapEntryAccessorType
      MapEntryAccessorType;
  // Internal stored type in MapEntryLite for given wire field type.
  typedef typename MapWireFieldTypeTraits<WireFormatLite::TYPE_MESSAGE,
                                          Type>::TypeOnMemory TypeOnMemory;
  // Corresponding wire type for field type.
  static constexpr WireFormatLite::WireType kWireType =
      MapWireFieldTypeTraits<WireFormatLite::TYPE_MESSAGE, Type>::kWireType;

  // Functions used in parsing and serialization. ===================
  static inline size_t ByteSize(const MapEntryAccessorType& value);
  static inline int GetCachedSize(const MapEntryAccessorType& value);
  static inline bool Read(io::CodedInputStream* input,
                          MapEntryAccessorType* value);
  static inline const char* Read(const char* ptr, ParseContext* ctx,
                                 MapEntryAccessorType* value);

  static inline uint8_t* Write(int field, const MapEntryAccessorType& value,
                               uint8_t* ptr, io::EpsCopyOutputStream* stream);

  // Functions to manipulate data on memory. ========================
  static inline const Type& GetExternalReference(const Type* value);
  static inline void DeleteNoArena(const Type* x);
  static constexpr TypeOnMemory Constinit();

  static inline Type* EnsureMutable(Type** value, Arena* arena);
  // Return default instance if value is not initialized when calling const
  // reference accessor.
  static inline const Type& DefaultIfNotInitialized(const Type* value);
  // Check if all required fields have values set.
  static inline bool IsInitialized(Type* value);
};

#define MAP_HANDLER(FieldType)                                                 \
  template <typename Type>                                                     \
  class MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type> {               \
   public:                                                                     \
    typedef typename MapWireFieldTypeTraits<WireFormatLite::TYPE_##FieldType,  \
                                            Type>::MapEntryAccessorType        \
        MapEntryAccessorType;                                                  \
    typedef typename MapWireFieldTypeTraits<WireFormatLite::TYPE_##FieldType,  \
                                            Type>::TypeOnMemory TypeOnMemory;  \
    static const WireFormatLite::WireType kWireType =                          \
        MapWireFieldTypeTraits<WireFormatLite::TYPE_##FieldType,               \
                               Type>::kWireType;                               \
    static inline int ByteSize(const MapEntryAccessorType& value);             \
    static inline int GetCachedSize(const MapEntryAccessorType& value);        \
    static inline bool Read(io::CodedInputStream* input,                       \
                            MapEntryAccessorType* value);                      \
    static inline const char* Read(const char* begin, ParseContext* ctx,       \
                                   MapEntryAccessorType* value);               \
    static inline uint8_t* Write(int field, const MapEntryAccessorType& value, \
                                 uint8_t* ptr,                                 \
                                 io::EpsCopyOutputStream* stream);             \
    static inline const MapEntryAccessorType& GetExternalReference(            \
        const TypeOnMemory& value);                                            \
    static inline void DeleteNoArena(const TypeOnMemory& x);                   \
    static inline const MapEntryAccessorType& DefaultIfNotInitialized(         \
        const TypeOnMemory& value);                                            \
    static inline bool IsInitialized(const TypeOnMemory& value);               \
    static void DeleteNoArena(TypeOnMemory& value);                            \
    static constexpr TypeOnMemory Constinit();                                 \
    static inline MapEntryAccessorType* EnsureMutable(TypeOnMemory* value,     \
                                                      Arena* arena);           \
  };
MAP_HANDLER(STRING)
MAP_HANDLER(BYTES)
MAP_HANDLER(INT64)
MAP_HANDLER(UINT64)
MAP_HANDLER(INT32)
MAP_HANDLER(UINT32)
MAP_HANDLER(SINT64)
MAP_HANDLER(SINT32)
MAP_HANDLER(ENUM)
MAP_HANDLER(DOUBLE)
MAP_HANDLER(FLOAT)
MAP_HANDLER(FIXED64)
MAP_HANDLER(FIXED32)
MAP_HANDLER(SFIXED64)
MAP_HANDLER(SFIXED32)
MAP_HANDLER(BOOL)
#undef MAP_HANDLER

template <typename Type>
inline size_t MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::ByteSize(
    const MapEntryAccessorType& value) {
  return WireFormatLite::MessageSizeNoVirtual(value);
}

#define GOOGLE_PROTOBUF_BYTE_SIZE(FieldType, DeclaredType)                     \
  template <typename Type>                                                     \
  inline int MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::ByteSize( \
      const MapEntryAccessorType& value) {                                     \
    return static_cast<int>(WireFormatLite::DeclaredType##Size(value));        \
  }

GOOGLE_PROTOBUF_BYTE_SIZE(STRING, String)
GOOGLE_PROTOBUF_BYTE_SIZE(BYTES, Bytes)
GOOGLE_PROTOBUF_BYTE_SIZE(INT64, Int64)
GOOGLE_PROTOBUF_BYTE_SIZE(UINT64, UInt64)
GOOGLE_PROTOBUF_BYTE_SIZE(INT32, Int32)
GOOGLE_PROTOBUF_BYTE_SIZE(UINT32, UInt32)
GOOGLE_PROTOBUF_BYTE_SIZE(SINT64, SInt64)
GOOGLE_PROTOBUF_BYTE_SIZE(SINT32, SInt32)
GOOGLE_PROTOBUF_BYTE_SIZE(ENUM, Enum)

#undef GOOGLE_PROTOBUF_BYTE_SIZE

#define FIXED_BYTE_SIZE(FieldType, DeclaredType)                               \
  template <typename Type>                                                     \
  inline int MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::ByteSize( \
      const MapEntryAccessorType& /* value */) {                               \
    return WireFormatLite::k##DeclaredType##Size;                              \
  }

FIXED_BYTE_SIZE(DOUBLE, Double)
FIXED_BYTE_SIZE(FLOAT, Float)
FIXED_BYTE_SIZE(FIXED64, Fixed64)
FIXED_BYTE_SIZE(FIXED32, Fixed32)
FIXED_BYTE_SIZE(SFIXED64, SFixed64)
FIXED_BYTE_SIZE(SFIXED32, SFixed32)
FIXED_BYTE_SIZE(BOOL, Bool)

#undef FIXED_BYTE_SIZE

template <typename Type>
inline int MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::GetCachedSize(
    const MapEntryAccessorType& value) {
  return static_cast<int>(WireFormatLite::LengthDelimitedSize(
      static_cast<size_t>(value.GetCachedSize())));
}

#define GET_CACHED_SIZE(FieldType, DeclaredType)                         \
  template <typename Type>                                               \
  inline int                                                             \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::GetCachedSize( \
      const MapEntryAccessorType& value) {                               \
    return static_cast<int>(WireFormatLite::DeclaredType##Size(value));  \
  }

GET_CACHED_SIZE(STRING, String)
GET_CACHED_SIZE(BYTES, Bytes)
GET_CACHED_SIZE(INT64, Int64)
GET_CACHED_SIZE(UINT64, UInt64)
GET_CACHED_SIZE(INT32, Int32)
GET_CACHED_SIZE(UINT32, UInt32)
GET_CACHED_SIZE(SINT64, SInt64)
GET_CACHED_SIZE(SINT32, SInt32)
GET_CACHED_SIZE(ENUM, Enum)

#undef GET_CACHED_SIZE

#define GET_FIXED_CACHED_SIZE(FieldType, DeclaredType)                   \
  template <typename Type>                                               \
  inline int                                                             \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::GetCachedSize( \
      const MapEntryAccessorType& /* value */) {                         \
    return WireFormatLite::k##DeclaredType##Size;                        \
  }

GET_FIXED_CACHED_SIZE(DOUBLE, Double)
GET_FIXED_CACHED_SIZE(FLOAT, Float)
GET_FIXED_CACHED_SIZE(FIXED64, Fixed64)
GET_FIXED_CACHED_SIZE(FIXED32, Fixed32)
GET_FIXED_CACHED_SIZE(SFIXED64, SFixed64)
GET_FIXED_CACHED_SIZE(SFIXED32, SFixed32)
GET_FIXED_CACHED_SIZE(BOOL, Bool)

#undef GET_FIXED_CACHED_SIZE

template <typename Type>
inline uint8_t* MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::Write(
    int field, const MapEntryAccessorType& value, uint8_t* ptr,
    io::EpsCopyOutputStream* stream) {
  ptr = stream->EnsureSpace(ptr);
  return WireFormatLite::InternalWriteMessage(
      field, value, value.GetCachedSize(), ptr, stream);
}

#define WRITE_METHOD(FieldType, DeclaredType)                     \
  template <typename Type>                                        \
  inline uint8_t*                                                 \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::Write(  \
      int field, const MapEntryAccessorType& value, uint8_t* ptr, \
      io::EpsCopyOutputStream* stream) {                          \
    ptr = stream->EnsureSpace(ptr);                               \
    return stream->Write##DeclaredType(field, value, ptr);        \
  }

WRITE_METHOD(STRING, String)
WRITE_METHOD(BYTES, Bytes)

#undef WRITE_METHOD
#define WRITE_METHOD(FieldType, DeclaredType)                               \
  template <typename Type>                                                  \
  inline uint8_t*                                                           \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::Write(            \
      int field, const MapEntryAccessorType& value, uint8_t* ptr,           \
      io::EpsCopyOutputStream* stream) {                                    \
    ptr = stream->EnsureSpace(ptr);                                         \
    return WireFormatLite::Write##DeclaredType##ToArray(field, value, ptr); \
  }

WRITE_METHOD(INT64, Int64)
WRITE_METHOD(UINT64, UInt64)
WRITE_METHOD(INT32, Int32)
WRITE_METHOD(UINT32, UInt32)
WRITE_METHOD(SINT64, SInt64)
WRITE_METHOD(SINT32, SInt32)
WRITE_METHOD(ENUM, Enum)
WRITE_METHOD(DOUBLE, Double)
WRITE_METHOD(FLOAT, Float)
WRITE_METHOD(FIXED64, Fixed64)
WRITE_METHOD(FIXED32, Fixed32)
WRITE_METHOD(SFIXED64, SFixed64)
WRITE_METHOD(SFIXED32, SFixed32)
WRITE_METHOD(BOOL, Bool)

#undef WRITE_METHOD

template <typename Type>
inline bool MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::Read(
    io::CodedInputStream* input, MapEntryAccessorType* value) {
  return WireFormatLite::ReadMessageNoVirtual(input, value);
}

template <typename Type>
inline bool MapTypeHandler<WireFormatLite::TYPE_STRING, Type>::Read(
    io::CodedInputStream* input, MapEntryAccessorType* value) {
  return WireFormatLite::ReadString(input, value);
}

template <typename Type>
inline bool MapTypeHandler<WireFormatLite::TYPE_BYTES, Type>::Read(
    io::CodedInputStream* input, MapEntryAccessorType* value) {
  return WireFormatLite::ReadBytes(input, value);
}

template <typename Type>
const char* MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::Read(
    const char* ptr, ParseContext* ctx, MapEntryAccessorType* value) {
  return ctx->ParseMessage(value, ptr);
}

template <typename Type>
const char* MapTypeHandler<WireFormatLite::TYPE_STRING, Type>::Read(
    const char* ptr, ParseContext* ctx, MapEntryAccessorType* value) {
  int size = ReadSize(&ptr);
  GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
  return ctx->ReadString(ptr, size, value);
}

template <typename Type>
const char* MapTypeHandler<WireFormatLite::TYPE_BYTES, Type>::Read(
    const char* ptr, ParseContext* ctx, MapEntryAccessorType* value) {
  int size = ReadSize(&ptr);
  GOOGLE_PROTOBUF_PARSER_ASSERT(ptr);
  return ctx->ReadString(ptr, size, value);
}

inline const char* ReadINT64(const char* ptr, int64_t* value) {
  return VarintParse(ptr, reinterpret_cast<uint64_t*>(value));
}
inline const char* ReadUINT64(const char* ptr, uint64_t* value) {
  return VarintParse(ptr, value);
}
inline const char* ReadINT32(const char* ptr, int32_t* value) {
  return VarintParse(ptr, reinterpret_cast<uint32_t*>(value));
}
inline const char* ReadUINT32(const char* ptr, uint32_t* value) {
  return VarintParse(ptr, value);
}
inline const char* ReadSINT64(const char* ptr, int64_t* value) {
  *value = ReadVarintZigZag64(&ptr);
  return ptr;
}
inline const char* ReadSINT32(const char* ptr, int32_t* value) {
  *value = ReadVarintZigZag32(&ptr);
  return ptr;
}
template <typename E>
inline const char* ReadENUM(const char* ptr, E* value) {
  *value = static_cast<E>(ReadVarint32(&ptr));
  return ptr;
}
inline const char* ReadBOOL(const char* ptr, bool* value) {
  *value = static_cast<bool>(ReadVarint64(&ptr));
  return ptr;
}

template <typename F>
inline const char* ReadUnaligned(const char* ptr, F* value) {
  *value = UnalignedLoad<F>(ptr);
  return ptr + sizeof(F);
}
inline const char* ReadFLOAT(const char* ptr, float* value) {
  return ReadUnaligned(ptr, value);
}
inline const char* ReadDOUBLE(const char* ptr, double* value) {
  return ReadUnaligned(ptr, value);
}
inline const char* ReadFIXED64(const char* ptr, uint64_t* value) {
  return ReadUnaligned(ptr, value);
}
inline const char* ReadFIXED32(const char* ptr, uint32_t* value) {
  return ReadUnaligned(ptr, value);
}
inline const char* ReadSFIXED64(const char* ptr, int64_t* value) {
  return ReadUnaligned(ptr, value);
}
inline const char* ReadSFIXED32(const char* ptr, int32_t* value) {
  return ReadUnaligned(ptr, value);
}

#define READ_METHOD(FieldType)                                              \
  template <typename Type>                                                  \
  inline bool MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::Read( \
      io::CodedInputStream* input, MapEntryAccessorType* value) {           \
    return WireFormatLite::ReadPrimitive<TypeOnMemory,                      \
                                         WireFormatLite::TYPE_##FieldType>( \
        input, value);                                                      \
  }                                                                         \
  template <typename Type>                                                  \
  const char* MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::Read( \
      const char* begin, ParseContext* ctx, MapEntryAccessorType* value) {  \
    (void)ctx;                                                              \
    return Read##FieldType(begin, value);                                   \
  }

READ_METHOD(INT64)
READ_METHOD(UINT64)
READ_METHOD(INT32)
READ_METHOD(UINT32)
READ_METHOD(SINT64)
READ_METHOD(SINT32)
READ_METHOD(ENUM)
READ_METHOD(DOUBLE)
READ_METHOD(FLOAT)
READ_METHOD(FIXED64)
READ_METHOD(FIXED32)
READ_METHOD(SFIXED64)
READ_METHOD(SFIXED32)
READ_METHOD(BOOL)

#undef READ_METHOD

// Definition for message handler

template <typename Type>
inline const Type&
MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::GetExternalReference(
    const Type* value) {
  return *value;
}

template <typename Type>
void MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::DeleteNoArena(
    const Type* ptr) {
  delete ptr;
}

template <typename Type>
constexpr auto MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::Constinit()
    -> TypeOnMemory {
  return nullptr;
}

template <typename Type>
inline Type* MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::EnsureMutable(
    Type** value, Arena* arena) {
  if (*value == nullptr) {
    *value = Arena::CreateMessage<Type>(arena);
  }
  return *value;
}

template <typename Type>
inline const Type&
MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::DefaultIfNotInitialized(
    const Type* value) {
  return value != nullptr ? *value : *Type::internal_default_instance();
}

template <typename Type>
inline bool MapTypeHandler<WireFormatLite::TYPE_MESSAGE, Type>::IsInitialized(
    Type* value) {
  return value ? value->IsInitialized() : false;
}

// Definition for string/bytes handler

#define STRING_OR_BYTES_HANDLER_FUNCTIONS(FieldType)                          \
  template <typename Type>                                                    \
  inline const typename MapTypeHandler<WireFormatLite::TYPE_##FieldType,      \
                                       Type>::MapEntryAccessorType&           \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType,                            \
                 Type>::GetExternalReference(const TypeOnMemory& value) {     \
    return value.Get();                                                       \
  }                                                                           \
  template <typename Type>                                                    \
  void MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::DeleteNoArena( \
      TypeOnMemory& value) {                                                  \
    value.Destroy();                                                          \
  }                                                                           \
  template <typename Type>                                                    \
  constexpr auto                                                              \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::Constinit()         \
      ->TypeOnMemory {                                                        \
    return TypeOnMemory(&internal::fixed_address_empty_string,                \
                        ConstantInitialized{});                               \
  }                                                                           \
  template <typename Type>                                                    \
  inline typename MapTypeHandler<WireFormatLite::TYPE_##FieldType,            \
                                 Type>::MapEntryAccessorType*                 \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::EnsureMutable(      \
      TypeOnMemory* value, Arena* arena) {                                    \
    return value->Mutable(arena);                                             \
  }                                                                           \
  template <typename Type>                                                    \
  inline const typename MapTypeHandler<WireFormatLite::TYPE_##FieldType,      \
                                       Type>::MapEntryAccessorType&           \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType,                            \
                 Type>::DefaultIfNotInitialized(const TypeOnMemory& value) {  \
    return value.Get();                                                       \
  }                                                                           \
  template <typename Type>                                                    \
  inline bool                                                                 \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::IsInitialized(      \
      const TypeOnMemory& /* value */) {                                      \
    return true;                                                              \
  }
STRING_OR_BYTES_HANDLER_FUNCTIONS(STRING)
STRING_OR_BYTES_HANDLER_FUNCTIONS(BYTES)
#undef STRING_OR_BYTES_HANDLER_FUNCTIONS

#define PRIMITIVE_HANDLER_FUNCTIONS(FieldType)                               \
  template <typename Type>                                                   \
  inline const typename MapTypeHandler<WireFormatLite::TYPE_##FieldType,     \
                                       Type>::MapEntryAccessorType&          \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType,                           \
                 Type>::GetExternalReference(const TypeOnMemory& value) {    \
    return value;                                                            \
  }                                                                          \
  template <typename Type>                                                   \
  inline void MapTypeHandler<WireFormatLite::TYPE_##FieldType,               \
                             Type>::DeleteNoArena(TypeOnMemory& /* x */) {}  \
  template <typename Type>                                                   \
  constexpr auto                                                             \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::Constinit()        \
      ->TypeOnMemory {                                                       \
    return 0;                                                                \
  }                                                                          \
  template <typename Type>                                                   \
  inline typename MapTypeHandler<WireFormatLite::TYPE_##FieldType,           \
                                 Type>::MapEntryAccessorType*                \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::EnsureMutable(     \
      TypeOnMemory* value, Arena* /* arena */) {                             \
    return value;                                                            \
  }                                                                          \
  template <typename Type>                                                   \
  inline const typename MapTypeHandler<WireFormatLite::TYPE_##FieldType,     \
                                       Type>::MapEntryAccessorType&          \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType,                           \
                 Type>::DefaultIfNotInitialized(const TypeOnMemory& value) { \
    return value;                                                            \
  }                                                                          \
  template <typename Type>                                                   \
  inline bool                                                                \
  MapTypeHandler<WireFormatLite::TYPE_##FieldType, Type>::IsInitialized(     \
      const TypeOnMemory& /* value */) {                                     \
    return true;                                                             \
  }
PRIMITIVE_HANDLER_FUNCTIONS(INT64)
PRIMITIVE_HANDLER_FUNCTIONS(UINT64)
PRIMITIVE_HANDLER_FUNCTIONS(INT32)
PRIMITIVE_HANDLER_FUNCTIONS(UINT32)
PRIMITIVE_HANDLER_FUNCTIONS(SINT64)
PRIMITIVE_HANDLER_FUNCTIONS(SINT32)
PRIMITIVE_HANDLER_FUNCTIONS(ENUM)
PRIMITIVE_HANDLER_FUNCTIONS(DOUBLE)
PRIMITIVE_HANDLER_FUNCTIONS(FLOAT)
PRIMITIVE_HANDLER_FUNCTIONS(FIXED64)
PRIMITIVE_HANDLER_FUNCTIONS(FIXED32)
PRIMITIVE_HANDLER_FUNCTIONS(SFIXED64)
PRIMITIVE_HANDLER_FUNCTIONS(SFIXED32)
PRIMITIVE_HANDLER_FUNCTIONS(BOOL)
#undef PRIMITIVE_HANDLER_FUNCTIONS

// Functions for operating on a map entry using type handlers.
//
// Does not contain any representation (this class is not intended to be
// instantiated).
template <typename Key, typename Value, WireFormatLite::FieldType kKeyFieldType,
          WireFormatLite::FieldType kValueFieldType>
struct MapEntryFuncs {
  typedef MapTypeHandler<kKeyFieldType, Key> KeyTypeHandler;
  typedef MapTypeHandler<kValueFieldType, Value> ValueTypeHandler;
  enum : int {
    kKeyFieldNumber = 1,
    kValueFieldNumber = 2
  };

  static uint8_t* InternalSerialize(int field_number, const Key& key,
                                    const Value& value, uint8_t* ptr,
                                    io::EpsCopyOutputStream* stream) {
    ptr = stream->EnsureSpace(ptr);
    ptr = WireFormatLite::WriteTagToArray(
        field_number, WireFormatLite::WIRETYPE_LENGTH_DELIMITED, ptr);
    ptr = io::CodedOutputStream::WriteVarint32ToArray(GetCachedSize(key, value),
                                                      ptr);

    ptr = KeyTypeHandler::Write(kKeyFieldNumber, key, ptr, stream);
    return ValueTypeHandler::Write(kValueFieldNumber, value, ptr, stream);
  }

  static size_t ByteSizeLong(const Key& key, const Value& value) {
    // Tags for key and value will both be one byte (field numbers 1 and 2).
    size_t inner_length =
        2 + KeyTypeHandler::ByteSize(key) + ValueTypeHandler::ByteSize(value);
    return inner_length + io::CodedOutputStream::VarintSize32(
                              static_cast<uint32_t>(inner_length));
  }

  static int GetCachedSize(const Key& key, const Value& value) {
    // Tags for key and value will both be one byte (field numbers 1 and 2).
    return 2 + KeyTypeHandler::GetCachedSize(key) +
           ValueTypeHandler::GetCachedSize(value);
  }
};

}  // namespace internal
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_MAP_TYPE_HANDLER_H__

