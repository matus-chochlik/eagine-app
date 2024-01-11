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
  , _content{main_context().buffers().get(size)} {
    _content.clear();
}
//------------------------------------------------------------------------------
simple_buffer_source_blob_io::simple_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t size,
  std::function<memory::buffer(memory::buffer)> make_content) noexcept
  : main_ctx_object{id, parent}
  , _content{make_content(main_context().buffers().get(size))} {}
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
simple_buffer_source_blob_io::~simple_buffer_source_blob_io() noexcept {
    main_context().buffers().eat(std::move(_content));
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

