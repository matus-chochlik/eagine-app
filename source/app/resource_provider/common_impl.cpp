/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// simple_string_source_blob_io
//------------------------------------------------------------------------------
simple_string_source_blob_io::simple_string_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  std::string content) noexcept
  : main_ctx_object{id, parent}
  , _content{std::move(content)} {}
//------------------------------------------------------------------------------
auto simple_string_source_blob_io::total_size() noexcept -> span_size_t {
    return span_size(_content.size());
}
//------------------------------------------------------------------------------
auto simple_string_source_blob_io::fetch_fragment(
  const span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    return copy(head(skip(view(_content), offs), dst.size()), dst).size();
}
//------------------------------------------------------------------------------
// simple_buffer_source_blob_io
//------------------------------------------------------------------------------
simple_buffer_source_blob_io::simple_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t size) noexcept
  : main_ctx_object{id, parent}
  , _content{*this, size} {
    _content.clear();
}
//------------------------------------------------------------------------------
simple_buffer_source_blob_io::simple_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t size,
  std::function<memory::buffer(memory::buffer)> make_content) noexcept
  : main_ctx_object{id, parent}
  , _content{*this, make_content(main_context().buffers().get(size))} {}
//------------------------------------------------------------------------------
void simple_buffer_source_blob_io::append(const memory::const_block part) {
    memory::append_to(part, _content);
}
//------------------------------------------------------------------------------
void simple_buffer_source_blob_io::append(const string_view part) {
    append(as_bytes(part));
}
//------------------------------------------------------------------------------
void simple_buffer_source_blob_io::append(const byte b) {
    append(view_one(b));
}
//------------------------------------------------------------------------------
auto simple_buffer_source_blob_io::total_size() noexcept -> span_size_t {
    return span_size(_content.size());
}
//------------------------------------------------------------------------------
auto simple_buffer_source_blob_io::fetch_fragment(
  const span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    return copy(head(skip(view(_content), offs), dst.size()), dst).size();
}
//------------------------------------------------------------------------------
// compressed_buffer_source_blob_io
//------------------------------------------------------------------------------
auto compressed_buffer_source_blob_io::_append_compressed(
  const memory::const_block packed) noexcept -> bool {
    this->append(packed);
    return true;
}
//------------------------------------------------------------------------------
auto compressed_buffer_source_blob_io::_compress_handler() noexcept {
    return data_compressor::data_handler{
      this,
      member_function_constant_t<
        &compressed_buffer_source_blob_io::_append_compressed>{}};
}
//------------------------------------------------------------------------------
compressed_buffer_source_blob_io::compressed_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t size) noexcept
  : simple_buffer_source_blob_io{id, parent, size}
  , _compress{
      main_context().compressor(),
      _compress_handler(),
      default_data_compression_method()} {}
//------------------------------------------------------------------------------
compressed_buffer_source_blob_io::compressed_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t size,
  std::function<memory::buffer(memory::buffer)> func) noexcept
  : simple_buffer_source_blob_io{id, parent, size, func}
  , _compress{
      main_context().compressor(),
      _compress_handler(),
      default_data_compression_method()} {}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::compress(
  const memory::const_block blk) noexcept {
    _compress.next(blk, data_compression_level::highest);
}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::compress(const string_view str) noexcept {
    compress(as_bytes(str));
}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::compress(const byte byt) noexcept {
    compress(view_one(byt));
}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::finish() noexcept {
    _compress.finish();
}
//------------------------------------------------------------------------------
// ostream_io
//------------------------------------------------------------------------------
auto ostream_io::ostream() noexcept -> std::ostream& {
    return _content;
}
//------------------------------------------------------------------------------
auto ostream_io::total_size() noexcept -> span_size_t {
    return span_size(_content.view().size());
}
//------------------------------------------------------------------------------
auto ostream_io::fetch_fragment(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    const auto src{skip(memory::view(_content.view()), offs)};
    dst = head(dst, src);
    return copy(head(src, dst), dst).size();
}
//------------------------------------------------------------------------------
// eagitex_provider_base
//------------------------------------------------------------------------------
eagitex_provider_base::eagitex_provider_base(
  identifier id,
  main_ctx_parent parent) noexcept
  : main_ctx_object{id, parent} {}
//------------------------------------------------------------------------------
} // namespace eagine::app

