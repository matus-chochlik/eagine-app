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
void resource_loader::_handle_glsl_src_data(
  const identifier_t blob_id,
  const pending_resource_info& rinfo,
  const memory::span<const memory::const_block> data,
  const msgbus::blob_info&) noexcept {
    _gl_strs.clear();
    _gl_strs.reserve(std_size(data.size()));
    _gl_ints.clear();
    _gl_ints.reserve(std_size(data.size()));
    for(const auto& blk : data) {
        _gl_strs.emplace_back(
          reinterpret_cast<const oglplus::gl_types::char_type*>(blk.data()));
        _gl_ints.emplace_back(
          static_cast<const oglplus::gl_types::int_type>(blk.size()));
    }
    const oglplus::glsl_source_ref src{
      data.size(), _gl_strs.data(), _gl_ints.data()};
    shader_source_loaded(blob_id, src, rinfo.locator);
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_data_appended(
  identifier_t blob_id,
  const span_size_t offset,
  const memory::span<const memory::const_block> data,
  const msgbus::blob_info& binfo) noexcept {
    (void)offset;
    if(const auto pos{_pending.find(blob_id)}; pos != _pending.end()) {
        const auto& rinfo{std::get<1>(*pos)};
        switch(rinfo.kind) {
            case resource_kind::glsl_strings:
                assert(offset == 0);
                _handle_glsl_src_data(blob_id, rinfo, data, binfo);
                break;
            default:
                break;
        }
    }
}
//------------------------------------------------------------------------------
auto resource_loader::_cancelled_resource(
  const identifier_t blob_id,
  url& locator,
  const resource_kind kind) noexcept
  -> std::pair<const identifier_t, pending_resource_info>& {
    _cancelled.push_back({blob_id, {std::move(locator), kind}});
    return _cancelled.back();
}
//------------------------------------------------------------------------------
auto resource_loader::_new_resource(
  const identifier_t blob_id,
  url locator,
  resource_kind kind) noexcept
  -> std::pair<const identifier_t, pending_resource_info>& {
    auto result{_pending.insert({blob_id, {std::move(locator), kind}})};
    return *std::get<0>(result);
}
//------------------------------------------------------------------------------
void resource_loader::_forget_resource(const identifier_t blob_id) noexcept {
    _pending.erase(blob_id);
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_finished(identifier_t blob_id) noexcept {
    _forget_resource(blob_id);
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(identifier_t blob_id) noexcept {
    if(const auto pos{_pending.find(blob_id)}; pos != _pending.end()) {
        const auto& rinfo{std::get<1>(*pos)};
        resource_cancelled(blob_id, rinfo.kind, rinfo.locator);
        _pending.erase(pos);
    }
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
auto resource_loader::request_shape_generator(url locator) noexcept
  -> resource_request_result {
    const auto request_id{get_request_id()};
    if(auto gen{shapes::shape_from(locator, main_context())}) {
        auto new_res{_new_resource(
          request_id, std::move(locator), resource_kind::shape_generator)};
        std::get<1>(new_res).state = std::move(gen);
        return {std::move(new_res)};
    }
    return _cancelled_resource(
      request_id, locator, resource_kind::shape_generator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader_source(url locator) noexcept
  -> resource_request_result {
    if(locator.has_scheme("glsl")) {
        return {_new_resource(
          fetch_resource_chunks(
            std::move(locator),
            2048,
            msgbus::message_priority::normal,
            std::chrono::seconds{15}),
          resource_kind::glsl_strings)};
    }

    return {_cancelled_resource(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(url locator, video_context&) noexcept
  -> resource_request_result {
    return {_cancelled_resource(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(url locator, video_context&) noexcept
  -> resource_request_result {
    return {_cancelled_resource(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(url locator, video_context&) noexcept
  -> resource_request_result {
    return {_cancelled_resource(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_program(url locator, video_context&) noexcept
  -> resource_request_result {
    return {_cancelled_resource(locator)};
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
        using ssg_t = std::shared_ptr<shapes::generator>;
        if(std::holds_alternative<ssg_t>(info.state)) {
            shape_loaded(request_id, std::get<ssg_t>(info.state), info.locator);
            pos = _pending.erase(pos);
            something_done();
        } else {
            ++pos;
        }
    }

    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
