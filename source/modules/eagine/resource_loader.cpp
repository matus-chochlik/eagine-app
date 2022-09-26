/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:resource_loader;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.container;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.shapes;
import eagine.oglplus;
import eagine.msgbus;
import <map>;
import <utility>;

namespace eagine::app {

export class video_context;
export class audio_context;
//------------------------------------------------------------------------------
export enum class resource_kind {
    shape_generator,
    glsl_strings,
    gl_shader,
    gl_program,
    unknown
};
//------------------------------------------------------------------------------
struct pending_resource_info {
    pending_resource_info(url loc, resource_kind k) noexcept
      : locator{std::move(loc)}
      , kind{k} {}

    const url locator;
    resource_kind kind{resource_kind::unknown};
};
//------------------------------------------------------------------------------
export class resource_request_result {
public:
    resource_request_result(
      identifier_t req_id,
      const pending_resource_info& info) noexcept
      : _request_id{req_id}
      , _info{info} {}

    resource_request_result(
      const std::pair<const identifier_t, pending_resource_info>& init) noexcept
      : resource_request_result{std::get<0>(init), std::get<1>(init)} {}

    explicit operator bool() const noexcept {
        return _request_id != 0;
    }

    auto request_id() const noexcept -> identifier_t {
        return _request_id;
    }

    auto locator() const noexcept -> const url& {
        return _info.locator;
    }

private:
    identifier_t _request_id;
    const pending_resource_info& _info;
};
//------------------------------------------------------------------------------
export struct resource_loader_signals {
    signal<void(identifier_t, resource_kind, const url&) noexcept>
      resource_cancelled;

    signal<void(identifier_t, const std::shared_ptr<shapes::generator>&) noexcept>
      shape_loaded;

    signal<void(identifier_t, const oglplus::glsl_source_ref&) noexcept>
      shader_source_loaded;

    signal<void(identifier_t, oglplus::buffer_name) noexcept> buffer_loaded;

    signal<void(identifier_t, oglplus::texture_name) noexcept> texture_loaded;

    signal<void(identifier_t, oglplus::shader_name) noexcept> shader_loaded;

    signal<void(identifier_t, oglplus::program_name) noexcept> program_loaded;
};
//------------------------------------------------------------------------------
export class resource_loader
  : public msgbus::resource_data_consumer_node
  , public resource_loader_signals {
    using base = msgbus::resource_data_consumer_node;

public:
    resource_loader(main_ctx& ctx)
      : resource_data_consumer_node{ctx} {
        _init();
    }

    auto update() noexcept -> work_done;

    auto request_shape_generator(url locator) noexcept
      -> resource_request_result;

    auto request_gl_shader_source(url locator) noexcept
      -> resource_request_result;

    auto request_gl_buffer(url locator) noexcept -> resource_request_result;

    auto request_gl_texture(url locator) noexcept -> resource_request_result;

    auto request_gl_shader(url locator) noexcept -> resource_request_result;

    auto request_gl_program(url locator) noexcept -> resource_request_result;

private:
    void _init() noexcept;

    void _handle_glsl_src_data(
      const identifier_t blob_id,
      const memory::span<const memory::const_block> data,
      const msgbus::blob_info& info) noexcept;

    void _handle_stream_data_appended(
      const identifier_t blob_id,
      const span_size_t offset,
      const memory::span<const memory::const_block>,
      const msgbus::blob_info& info) noexcept;

    auto _cancelled_resource(
      const identifier_t blob_id,
      url& locator,
      const resource_kind) noexcept
      -> const std::pair<const identifier_t, pending_resource_info>&;

    auto _cancelled_resource(url& locator) noexcept
      -> const std::pair<const identifier_t, pending_resource_info>& {
        return _cancelled_resource(
          get_request_id(), locator, resource_kind::unknown);
    }

    auto _new_resource(
      const identifier_t blob_id,
      const url& locator,
      resource_kind) noexcept
      -> std::pair<const identifier_t, pending_resource_info>&;

    auto _new_resource(
      const std::pair<identifier_t, const url&>& id_and_loc,
      resource_kind kind) noexcept
      -> std::pair<const identifier_t, pending_resource_info>& {
        return _new_resource(
          std::get<0>(id_and_loc), std::get<1>(id_and_loc), kind);
    }

    void _forget_resource(const identifier_t blob_id) noexcept;

    void _handle_stream_finished(identifier_t blob_id) noexcept;
    void _handle_stream_cancelled(identifier_t blob_id) noexcept;

    std::vector<std::pair<const identifier_t, pending_resource_info>> _cancelled;
    std::map<identifier_t, pending_resource_info> _pending;

    std::vector<std::tuple<identifier_t, std::shared_ptr<shapes::generator>>>
      _shape_generators;

    std::vector<const oglplus::gl_types::char_type*> _gl_strs;
    std::vector<oglplus::gl_types::int_type> _gl_ints;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

