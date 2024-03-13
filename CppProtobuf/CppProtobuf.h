//
//  CppProtobuf.h
//  CppProtobuf
//
//  Created by Undo Hatsune on 2024/01/16.
//

#import <Foundation/Foundation.h>

//! Project version number for CppProtobuf.
FOUNDATION_EXPORT double CppProtobufVersionNumber;

//! Project version string for CppProtobuf.
FOUNDATION_EXPORT const unsigned char CppProtobufVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <CppProtobuf/PublicHeader.h>


//#import <CppProtobuf/utf8range_utf8_range.hpp>
//#import <CppProtobuf/utf8range_utf8_validity.hpp>


#import <CppProtobuf/google_protobuf_any.hpp>
#import <CppProtobuf/google_protobuf_any.pb.hpp>
#import <CppProtobuf/google_protobuf_arena.hpp>
#import <CppProtobuf/google_protobuf_arena_align.hpp>
#import <CppProtobuf/google_protobuf_arena_allocation_policy.hpp>
#import <CppProtobuf/google_protobuf_arena_cleanup.hpp>
#import <CppProtobuf/google_protobuf_arenastring.hpp>
#import <CppProtobuf/google_protobuf_arenaz_sampler.hpp>
#import <CppProtobuf/google_protobuf_cpp_edition_defaults.hpp>
#import <CppProtobuf/google_protobuf_cpp_features.pb.hpp>
#import <CppProtobuf/google_protobuf_descriptor.hpp>
#import <CppProtobuf/google_protobuf_descriptor.pb.hpp>
#import <CppProtobuf/google_protobuf_descriptor_database.hpp>
#import <CppProtobuf/google_protobuf_descriptor_legacy.hpp>
#import <CppProtobuf/google_protobuf_descriptor_visitor.hpp>
#import <CppProtobuf/google_protobuf_duration.pb.hpp>
#import <CppProtobuf/google_protobuf_dynamic_message.hpp>
#import <CppProtobuf/google_protobuf_endian.hpp>
#import <CppProtobuf/google_protobuf_explicitly_constructed.hpp>
#import <CppProtobuf/google_protobuf_extension_set.hpp>
#import <CppProtobuf/google_protobuf_extension_set_inl.hpp>
#import <CppProtobuf/google_protobuf_feature_resolver.hpp>
#import <CppProtobuf/google_protobuf_field_mask.pb.hpp>
#import <CppProtobuf/google_protobuf_generated_enum_reflection.hpp>
#import <CppProtobuf/google_protobuf_generated_enum_util.hpp>
#import <CppProtobuf/google_protobuf_generated_message_reflection.hpp>
#import <CppProtobuf/google_protobuf_generated_message_tctable_decl.hpp>
#import <CppProtobuf/google_protobuf_generated_message_tctable_gen.hpp>
#import <CppProtobuf/google_protobuf_generated_message_tctable_impl.hpp>
#import <CppProtobuf/google_protobuf_generated_message_util.hpp>
#import <CppProtobuf/google_protobuf_has_bits.hpp>
#import <CppProtobuf/google_protobuf_implicit_weak_message.hpp>
#import <CppProtobuf/google_protobuf_inlined_string_field.hpp>
#import <CppProtobuf/google_protobuf_internal_visibility.hpp>
#import <CppProtobuf/google_protobuf_io_coded_stream.hpp>
#import <CppProtobuf/google_protobuf_io_io_win32.hpp>
#import <CppProtobuf/google_protobuf_io_printer.hpp>
#import <CppProtobuf/google_protobuf_io_strtod.hpp>
#import <CppProtobuf/google_protobuf_io_tokenizer.hpp>
#import <CppProtobuf/google_protobuf_io_zero_copy_sink.hpp>
#import <CppProtobuf/google_protobuf_io_zero_copy_stream.hpp>
#import <CppProtobuf/google_protobuf_io_zero_copy_stream_impl.hpp>
#import <CppProtobuf/google_protobuf_io_zero_copy_stream_impl_lite.hpp>
#import <CppProtobuf/google_protobuf_json_internal_descriptor_traits.hpp>
#import <CppProtobuf/google_protobuf_json_internal_lexer.hpp>
#import <CppProtobuf/google_protobuf_json_internal_message_path.hpp>
#import <CppProtobuf/google_protobuf_json_internal_parser.hpp>
#import <CppProtobuf/google_protobuf_json_internal_parser_traits.hpp>
#import <CppProtobuf/google_protobuf_json_internal_unparser.hpp>
#import <CppProtobuf/google_protobuf_json_internal_unparser_traits.hpp>
#import <CppProtobuf/google_protobuf_json_internal_untyped_message.hpp>
#import <CppProtobuf/google_protobuf_json_internal_writer.hpp>
#import <CppProtobuf/google_protobuf_json_internal_zero_copy_buffered_stream.hpp>
#import <CppProtobuf/google_protobuf_json_json.hpp>
#import <CppProtobuf/google_protobuf_map.hpp>
#import <CppProtobuf/google_protobuf_map_entry.hpp>
#import <CppProtobuf/google_protobuf_map_field.hpp>
#import <CppProtobuf/google_protobuf_map_field_inl.hpp>
#import <CppProtobuf/google_protobuf_map_field_lite.hpp>
#import <CppProtobuf/google_protobuf_map_type_handler.hpp>
#import <CppProtobuf/google_protobuf_message.hpp>
#import <CppProtobuf/google_protobuf_message_lite.hpp>
#import <CppProtobuf/google_protobuf_metadata_lite.hpp>
#import <CppProtobuf/google_protobuf_parse_context.hpp>
#import <CppProtobuf/google_protobuf_port.hpp>
#import <CppProtobuf/google_protobuf_raw_ptr.hpp>
#import <CppProtobuf/google_protobuf_reflection.hpp>
#import <CppProtobuf/google_protobuf_reflection_internal.hpp>
#import <CppProtobuf/google_protobuf_reflection_mode.hpp>
#import <CppProtobuf/google_protobuf_reflection_ops.hpp>
#import <CppProtobuf/google_protobuf_repeated_field.hpp>
#import <CppProtobuf/google_protobuf_repeated_ptr_field.hpp>
#import <CppProtobuf/google_protobuf_serial_arena.hpp>
#import <CppProtobuf/google_protobuf_source_context.pb.hpp>
#import <CppProtobuf/google_protobuf_string_block.hpp>
#import <CppProtobuf/google_protobuf_stubs_callback.hpp>
#import <CppProtobuf/google_protobuf_stubs_common.hpp>
#import <CppProtobuf/google_protobuf_stubs_platform_macros.hpp>
#import <CppProtobuf/google_protobuf_stubs_port.hpp>
#import <CppProtobuf/google_protobuf_stubs_status_macros.hpp>
#import <CppProtobuf/google_protobuf_text_format.hpp>
#import <CppProtobuf/google_protobuf_thread_safe_arena.hpp>
#import <CppProtobuf/google_protobuf_timestamp.pb.hpp>
#import <CppProtobuf/google_protobuf_type.pb.hpp>
#import <CppProtobuf/google_protobuf_unknown_field_set.hpp>
#import <CppProtobuf/google_protobuf_util_delimited_message_util.hpp>
#import <CppProtobuf/google_protobuf_util_field_comparator.hpp>
#import <CppProtobuf/google_protobuf_util_field_mask_util.hpp>
#import <CppProtobuf/google_protobuf_util_json_util.hpp>
#import <CppProtobuf/google_protobuf_util_message_differencer.hpp>
#import <CppProtobuf/google_protobuf_util_time_util.hpp>
#import <CppProtobuf/google_protobuf_util_type_resolver.hpp>
#import <CppProtobuf/google_protobuf_util_type_resolver_util.hpp>
#import <CppProtobuf/google_protobuf_varint_shuffle.hpp>
#import <CppProtobuf/google_protobuf_wire_format.hpp>
#import <CppProtobuf/google_protobuf_wire_format_lite.hpp>
#import <CppProtobuf/google_protobuf_wrappers.pb.hpp>
#import <CppProtobuf/google_protobuf_generated_message_bases.hpp>


