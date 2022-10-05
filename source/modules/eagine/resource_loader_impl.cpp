/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

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
// pending_resource_info
//------------------------------------------------------------------------------
void pending_resource_info::add_valtree_stream_input(
  valtree::value_tree_stream_input input) noexcept {
    state = _pending_valtree_traversal_state{.input = std::move(input)};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_shape_generator(
  std::shared_ptr<shapes::generator> gen) noexcept {
    state = _pending_shape_generator_state{.generator = std::move(gen)};
}
//------------------------------------------------------------------------------
auto pending_resource_info::update() noexcept -> work_done {
    some_true something_done;
    if(std::holds_alternative<_pending_shape_generator_state>(state)) {
        if(const auto shape_gen{
             std::get<_pending_shape_generator_state>(state).generator}) {
            parent.shape_loaded(request_id, shape_gen, locator);
            something_done();
        }
        parent.forget_resource(request_id);
    }
    return something_done;
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_json_text(
  const msgbus::blob_info&,
  const pending_resource_info&,
  const span_size_t,
  const memory::span<const memory::const_block>) noexcept {
    if(kind == resource_kind::value_tree_traversal) {
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_yaml_text(
  const msgbus::blob_info&,
  const pending_resource_info&,
  const span_size_t,
  const memory::span<const memory::const_block>) noexcept {
    if(kind == resource_kind::value_tree) {
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_glsl_strings(
  const msgbus::blob_info&,
  const pending_resource_info& source,
  [[maybe_unused]] const span_size_t offset,
  const memory::span<const memory::const_block> data) noexcept {
    if(kind == resource_kind::glsl_source) {
        std::vector<const oglplus::gl_types::char_type*> gl_strs;
        std::vector<oglplus::gl_types::int_type> gl_ints;
        gl_strs.reserve(std_size(data.size()));
        gl_ints.reserve(std_size(data.size()));

        for(const auto& blk : data) {
            gl_strs.emplace_back(
              reinterpret_cast<const oglplus::gl_types::char_type*>(
                blk.data()));
            gl_ints.emplace_back(
              static_cast<const oglplus::gl_types::int_type>(blk.size()));
        }
        const oglplus::glsl_source_ref glsl_src{
          data.size(), gl_strs.data(), gl_ints.data()};
        parent.shader_source_loaded(request_id, glsl_src, locator);
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_source_data(
  const msgbus::blob_info& binfo,
  const pending_resource_info& rinfo,
  const span_size_t offset,
  const memory::span<const memory::const_block> data) noexcept {
    switch(rinfo.kind) {
        case resource_kind::json_text:
            _handle_json_text(binfo, rinfo, offset, data);
            break;
        case resource_kind::yaml_text:
            _handle_json_text(binfo, rinfo, offset, data);
            break;
        case resource_kind::glsl_text:
            _handle_glsl_strings(binfo, rinfo, offset, data);
            break;
        default:
            break;
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_source_finished(
  const pending_resource_info&) noexcept {
    parent.forget_resource(request_id);
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_source_cancelled(
  const pending_resource_info&) noexcept {
    parent.resource_cancelled(request_id, kind, locator);
    parent.forget_resource(request_id);
}
//------------------------------------------------------------------------------
// resource_loader
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_data_appended(
  identifier_t request_id,
  const span_size_t offset,
  const memory::span<const memory::const_block> data,
  const msgbus::blob_info& binfo) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        const auto& rinfo = std::get<1>(*pos);
        if(rinfo._continuation) {
            extract(rinfo._continuation)
              .handle_source_data(binfo, rinfo, offset, data);
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_finished(identifier_t request_id) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        const auto& rinfo = std::get<1>(*pos);
        if(rinfo._continuation) {
            extract(rinfo._continuation).handle_source_finished(rinfo);
        }
        _pending.erase(pos);
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(
  identifier_t request_id) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        const auto& [request_id, rinfo] = *pos;
        if(rinfo._continuation) {
            extract(rinfo._continuation).handle_source_cancelled(rinfo);
        }
        resource_cancelled(request_id, rinfo.kind, rinfo.locator);
        _pending.erase(pos);
    }
}
//------------------------------------------------------------------------------
auto resource_loader::_cancelled_resource(
  const identifier_t request_id,
  url& locator,
  const resource_kind kind) noexcept -> resource_request_result {
    auto result{_cancelled.emplace(
      request_id,
      pending_resource_info{*this, request_id, std::move(locator), kind})};
    return {*std::get<0>(result)};
}
//------------------------------------------------------------------------------
auto resource_loader::_new_resource(
  const identifier_t request_id,
  url locator,
  resource_kind kind) noexcept -> resource_request_result {
    auto result{_pending.emplace(
      request_id,
      pending_resource_info{*this, request_id, std::move(locator), kind})};
    return {*std::get<0>(result)};
}
//------------------------------------------------------------------------------
void resource_loader::_init() noexcept {
    connect<&resource_loader::_handle_stream_data_appended>(
      this, blob_stream_data_appended);
    connect<&resource_loader::_handle_stream_finished>(
      this, blob_stream_finished);
    connect<&resource_loader::_handle_stream_cancelled>(
      this, blob_stream_cancelled);
}
//------------------------------------------------------------------------------
auto resource_loader::request_value_tree(url locator) noexcept
  -> resource_request_result {
    if(locator.has_path_suffix(".json") || locator.has_scheme("json")) {
        return _new_resource(
          fetch_resource_chunks(
            std::move(locator),
            2048,
            msgbus::message_priority::normal,
            std::chrono::seconds{15}),
          resource_kind::json_text);
        // TODO:
    }

    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_value_tree_traversal(
  url locator,
  std::shared_ptr<valtree::value_tree_visitor> visitor,
  span_size_t max_token_size) noexcept -> resource_request_result {
    if(locator.has_path_suffix(".json") || locator.has_scheme("json")) {
        if(const auto src_request{_new_resource(
             stream_resource(
               std::move(locator),
               msgbus::message_priority::normal,
               std::chrono::hours{1}),
             resource_kind::json_text)}) {

            auto new_request{_new_resource(
              std::move(locator), resource_kind::value_tree_traversal)};
            new_request.info().add_valtree_stream_input(traverse_json_stream(
              std::move(visitor),
              max_token_size,
              buffers(),
              main_context().log()));
            src_request.set_continuation(new_request);
            return new_request;
        }
    }
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_shape_generator(url locator) noexcept
  -> resource_request_result {
    if(auto gen{shapes::shape_from(locator, main_context())}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::shape_generator)};
        new_request.info().add_shape_generator(std::move(gen));
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::shape_generator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader_source(url locator) noexcept
  -> resource_request_result {
    if(locator.has_path_suffix(".glsl") || locator.has_scheme("glsl")) {
        if(const auto src_request{_new_resource(
             fetch_resource_chunks(
               locator,
               2048,
               msgbus::message_priority::normal,
               std::chrono::seconds{15}),
             resource_kind::glsl_text)}) {
            auto new_request{
              _new_resource(std::move(locator), resource_kind::glsl_source)};
            src_request.set_continuation(new_request);
            return new_request;
        }
    }

    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(url locator, video_context&) noexcept
  -> resource_request_result {
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(url locator, video_context&) noexcept
  -> resource_request_result {
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(url locator, video_context&) noexcept
  -> resource_request_result {
    if(const auto src_request{request_gl_shader_source(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::gl_shader)};
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_shader);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_program(url locator, video_context&) noexcept
  -> resource_request_result {
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
void resource_loader::forget_resource(identifier_t request_id) noexcept {
    _deleted.insert(request_id);
}
//------------------------------------------------------------------------------
auto resource_loader::update() noexcept -> work_done {
    some_true something_done{base::update()};

    for(auto& [request_id, rinfo] : _cancelled) {
        resource_cancelled(request_id, rinfo.kind, rinfo.locator);
        something_done();
    }
    _cancelled.clear();

    for(auto pos{_pending.begin()}; pos != _pending.end();) {
        auto& [request_id, info] = *pos;
        if(_deleted.contains(request_id)) {
            pos = _pending.erase(pos);
            something_done();
        } else {
            something_done(info.update());
            ++pos;
        }
    }
    _deleted.clear();

    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
