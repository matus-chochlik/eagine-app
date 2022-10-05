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
export class resource_request_result;
export class resource_loader;
//------------------------------------------------------------------------------
/// @brief Resource kind enumeration.
/// @see resource_loader
/// @see resource_loader_signals
export enum class resource_kind {
    /// @brief JSON text.
    json_text,
    /// @brief YAML text.
    yaml_text,
    /// @brief GLSL text.
    glsl_text,
    /// @brief Shape generator.
    shape_generator,
    /// @brief Value tree.
    value_tree,
    /// @brief Value tree stream traversal.
    value_tree_traversal,
    /// @brief GLSL source string collection.
    glsl_source,
    /// @brief GL shader object.
    gl_shader,
    ///@brief GL program object.
    gl_program,
    /// @brief Unknown resource types.
    unknown
};
//------------------------------------------------------------------------------
class pending_resource_info {
    pending_resource_info(
      resource_loader& loader,
      identifier_t req_id,
      url loc,
      resource_kind k) noexcept
      : parent{loader}
      , request_id{req_id}
      , locator{std::move(loc)}
      , kind{k} {}

    resource_loader& parent;
    const identifier_t request_id;
    const url locator;
    resource_kind kind{resource_kind::unknown};

    void add_valtree_stream_input(
      valtree::value_tree_stream_input input) noexcept;

    void add_shape_generator(std::shared_ptr<shapes::generator> gen) noexcept;

    auto update() noexcept -> work_done;

    // continuation handlers
    void handle_source_data(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void handle_source_finished(const pending_resource_info&) noexcept;
    void handle_source_cancelled(const pending_resource_info&) noexcept;

private:
    friend class resource_loader;
    friend class resource_request_result;

    void _handle_json_text(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void _handle_yaml_text(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void _handle_glsl_strings(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    pending_resource_info* _continuation{nullptr};

    // the pending resource state
    struct _pending_valtree_traversal_state {
        valtree::value_tree_stream_input input;
    };

    struct _pending_shape_generator_state {
        std::shared_ptr<shapes::generator> generator;
    };

    std::variant<
      std::monostate,
      _pending_valtree_traversal_state,
      _pending_shape_generator_state>
      state;
};
//------------------------------------------------------------------------------
/// @brief Result of resource request operation.
/// @see resource_kind
export class resource_request_result {
public:
    resource_request_result(pending_resource_info& info) noexcept
      : _info{info} {}

    resource_request_result(
      std::pair<const identifier_t, pending_resource_info>& init) noexcept
      : resource_request_result{std::get<1>(init)} {}

    /// @brief Indicates if the request is valid.
    explicit operator bool() const noexcept {
        return _info.request_id != 0;
    }

    /// @brief Returns the unique id of the request.
    auto request_id() const noexcept -> identifier_t {
        return _info.request_id;
    }

    auto info() noexcept -> pending_resource_info& {
        return _info;
    }

    /// @brief Returns the locator of the requested resource.
    auto locator() const noexcept -> const url& {
        return _info.locator;
    }

    /// @brief Sets the reference to the continuation request of this request.
    auto set_continuation(resource_request_result& cont) const noexcept
      -> const resource_request_result& {
        _info._continuation = &cont._info;
        return *this;
    }

private:
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

    /// @brief Emitted when a value tree is loaded.
    signal<void(identifier_t, const valtree::compound&, const url&) noexcept>
      value_tree_loaded;

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

    void forget_resource(identifier_t request_id) noexcept;

    /// @brief Does some work and updates internal state (should be called periodically).
    auto update() noexcept -> work_done;

    /// @brief Requests a value tree object resource.
    auto request_value_tree(url locator) noexcept -> resource_request_result;

    auto request_value_tree_traversal(
      url locator,
      std::shared_ptr<valtree::value_tree_visitor>,
      span_size_t max_token_size) noexcept -> resource_request_result;

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
    friend class pending_resource_info;

    void _init() noexcept;

    void _handle_stream_data_appended(
      const identifier_t blob_id,
      const span_size_t offset,
      const memory::span<const memory::const_block>,
      const msgbus::blob_info& info) noexcept;

    void _handle_stream_finished(identifier_t blob_id) noexcept;
    void _handle_stream_cancelled(identifier_t blob_id) noexcept;

    auto _cancelled_resource(
      const identifier_t blob_id,
      url& locator,
      const resource_kind) noexcept -> resource_request_result;

    auto _cancelled_resource(url& locator, const resource_kind kind) noexcept
      -> resource_request_result {
        return _cancelled_resource(get_request_id(), locator, kind);
    }

    auto _cancelled_resource(url& locator) noexcept -> resource_request_result {
        return _cancelled_resource(locator, resource_kind::unknown);
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

    auto _new_resource(url locator, resource_kind kind) noexcept
      -> resource_request_result {
        return _new_resource(get_request_id(), std::move(locator), kind);
    }

    std::map<identifier_t, pending_resource_info> _pending;
    std::map<identifier_t, pending_resource_info> _cancelled;
    flat_set<identifier_t> _deleted;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

