/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "common.hpp"
#include <cassert>

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
  span_size_t size,
  std::function<memory::buffer(memory::buffer)> make_content) noexcept
  : main_ctx_object{id, parent}
  , _content{make_content(main_context().buffers().get(size))} {}
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
// eagitex_provider_base
//------------------------------------------------------------------------------
eagitex_provider_base::eagitex_provider_base(
  identifier id,
  main_ctx_parent parent) noexcept
  : main_ctx_object{id, parent} {}
//------------------------------------------------------------------------------
} // namespace eagine::app

