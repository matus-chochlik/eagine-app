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
import <variant>;

namespace eagine::app {

export class video_context;
export class audio_context;
//------------------------------------------------------------------------------
/// @brief Resource kind enumeration.
/// @see resource_loader
/// @see resource_loader_signals
export enum class resource_kind {
    /// @brief Shape generator.
    shape_generator,
    /// @brief GLSL string collection.
    glsl_strings,
    /// @brief GL shader object.
    gl_shader,
    ///@brief GL program object.
    gl_program,
    /// @brief Unknown resource types.
    unknown
};
//------------------------------------------------------------------------------
struct pending_resource_info {
    pending_resource_info(url loc, resource_kind k) noexcept
      : locator{std::move(loc)}
      , kind{k} {}

    const url locator;
    resource_kind kind{resource_kind::unknown};

    struct _pending_shape_generator_state {
        std::shared_ptr<shapes::generator> generator;
    };

    void add_shape_generator(std::shared_ptr<shapes::generator> gen) noexcept {
        state = _pending_shape_generator_state{.generator = std::move(gen)};
    }

    auto get_shape_generator() const noexcept
      -> std::shared_ptr<shapes::generator> {
        if(std::holds_alternative<_pending_shape_generator_state>(state)) {
            return std::get<_pending_shape_generator_state>(state).generator;
        }
        return {};
    }

    struct _pending_gl_shader_state {
        identifier_t glsl_source_request_id{0};
    };

    void add_glsl_source_request_id(identifier_t request_id) noexcept {
        state = _pending_gl_shader_state{.glsl_source_request_id = request_id};
    }

    auto has_glsl_source_request_id(identifier_t request_id) const noexcept
      -> bool {
        if(std::holds_alternative<_pending_gl_shader_state>(state)) {
            return std::get<_pending_gl_shader_state>(state)
                     .glsl_source_request_id == request_id;
        }
        return false;
    }

    std::variant<
      std::monostate,
      _pending_shape_generator_state,
      _pending_gl_shader_state>
      state;
};
//------------------------------------------------------------------------------
/// @brief Result of resource request operation.
/// @see resource_kind
export class resource_request_result {
public:
    resource_request_result(
      identifier_t req_id,
      pending_resource_info& info) noexcept
      : _request_id{req_id}
      , _info{info} {}

    resource_request_result(
      std::pair<const identifier_t, pending_resource_info>& init) noexcept
      : resource_request_result{std::get<0>(init), std::get<1>(init)} {}

    /// @brief Indicates if the request is valid.
    explicit operator bool() const noexcept {
        return _request_id != 0;
    }

    /// @brief Returns the unique id of the request.
    auto request_id() const noexcept -> identifier_t {
        return _request_id;
    }

    auto info() noexcept -> pending_resource_info& {
        return _info;
    }

    /// @brief Returns the locator of the requested resource.
    auto locator() const noexcept -> const url& {
        return _info.locator;
    }

private:
    identifier_t _request_id;
    pending_resource_info& _info;
};
//------------------------------------------------------------------------------
/// @brief Collection of signals emitted by the resource loader.
/// @see resource_loader
export struct resource_loader_signals {
    /// @brief Emitted then the loading of a resource is cancelled.
    signal<void(identifier_t, resource_kind, const url&) noexcept>
      resource_cancelled;

    /// @brief Emitted when a shape generator is loaded.
    signal<void(
      identifier_t,
      const std::shared_ptr<shapes::generator>&,
      const url&) noexcept>
      shape_loaded;

    /// @brief Emitted when a GLSL source code is loaded.
    signal<
      void(identifier_t, const oglplus::glsl_source_ref&, const url&) noexcept>
      shader_source_loaded;

    signal<void(identifier_t, oglplus::buffer_name, const url&) noexcept>
      buffer_loaded;

    signal<void(identifier_t, oglplus::texture_name, const url&) noexcept>
      texture_loaded;

    signal<void(identifier_t, oglplus::shader_name, const url&) noexcept>
      shader_loaded;

    signal<void(identifier_t, oglplus::program_name, const url&) noexcept>
      program_loaded;
};
//------------------------------------------------------------------------------
/// @brief Loader of resources of various types.
/// @see resource_request_result
export class resource_loader
  : public msgbus::resource_data_consumer_node
  , public resource_loader_signals {
    using base = msgbus::resource_data_consumer_node;

public:
    /// @brief Initializing constructor
    resource_loader(main_ctx& ctx)
      : resource_data_consumer_node{ctx} {
        _init();
    }

    /// @brief Does some work and updates internal state (should be called periodically).
    auto update() noexcept -> work_done;

    /// @brief Requests a shape geometry generator / loader resource.
    auto request_shape_generator(url locator) noexcept
      -> resource_request_result;

    /// @brief Requests GLSL shader source code resource.
    auto request_gl_shader_source(url locator) noexcept
      -> resource_request_result;

    auto request_gl_buffer(url locator, video_context&) noexcept
      -> resource_request_result;

    auto request_gl_texture(url locator, video_context&) noexcept
      -> resource_request_result;

    auto request_gl_shader(url locator, video_context&) noexcept
      -> resource_request_result;

    auto request_gl_program(url locator, video_context&) noexcept
      -> resource_request_result;

private:
    void _init() noexcept;

    void _handle_glsl_src_data(
      const identifier_t blob_id,
      const pending_resource_info&,
      const memory::span<const memory::const_block> data,
      const msgbus::blob_info&) noexcept;

    void _handle_stream_data_appended(
      const identifier_t blob_id,
      const span_size_t offset,
      const memory::span<const memory::const_block>,
      const msgbus::blob_info& info) noexcept;

    auto _cancelled_resource(
      const identifier_t blob_id,
      url& locator,
      const resource_kind) noexcept -> resource_request_result;

    auto _cancelled_resource(url& locator) noexcept -> resource_request_result {
        return _cancelled_resource(
          get_request_id(), locator, resource_kind::unknown);
    }

    auto _new_resource(
      const identifier_t blob_id,
      url locator,
      resource_kind) noexcept -> resource_request_result;

    auto _new_resource(
      const std::pair<identifier_t, const url&>& id_and_loc,
      resource_kind kind) noexcept -> resource_request_result {
        return _new_resource(
          std::get<0>(id_and_loc), std::get<1>(id_and_loc), kind);
    }

    void _forget_resource(const identifier_t blob_id) noexcept;

    void _handle_stream_finished(identifier_t blob_id) noexcept;
    void _handle_stream_cancelled(identifier_t blob_id) noexcept;

    std::vector<std::pair<const identifier_t, pending_resource_info>> _cancelled;
    std::map<identifier_t, pending_resource_info> _pending;

    std::vector<const oglplus::gl_types::char_type*> _gl_strs;
    std::vector<oglplus::gl_types::int_type> _gl_ints;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

