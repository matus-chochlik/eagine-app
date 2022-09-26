/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.reflection;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.shapes;
import eagine.oglplus;
import eagine.msgbus;

namespace eagine::app {
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_data_appended(
  identifier_t blob_id,
  const span_size_t offset,
  const memory::span<const memory::const_block>,
  const msgbus::blob_info& info) noexcept {
    (void)blob_id;
    (void)offset;
    (void)info;
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_finished(identifier_t blob_id) noexcept {
    (void)blob_id;
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(identifier_t blob_id) noexcept {
    (void)blob_id;
}
//------------------------------------------------------------------------------
resource_loader::resource_loader(main_ctx& ctx)
  : resource_data_consumer_node{ctx} {
    connect<&resource_loader::_handle_stream_data_appended>(
      this, blob_stream_data_appended);
    connect<&resource_loader::_handle_stream_finished>(
      this, blob_stream_finished);
    connect<&resource_loader::_handle_stream_cancelled>(
      this, blob_stream_cancelled);
}
//------------------------------------------------------------------------------
auto resource_loader::request_shape_generator(url locator) noexcept
  -> resource_request_result {
    if(auto gen{shapes::shape_from(locator, main_context())}) {
        const auto request_id{get_request_id()};
        _shape_generators.emplace_back(request_id, std::move(gen));
        return {request_id, std::move(locator)};
    }
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader_source(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_program(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::update() noexcept -> work_done {
    some_true something_done{base::update()};

    for(auto& [request_id, shape_gen] : _shape_generators) {
        shape_loaded(request_id, shape_gen);
    }
    _shape_generators.clear();

    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
