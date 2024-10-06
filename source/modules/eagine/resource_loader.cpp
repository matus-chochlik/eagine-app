/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app:resource_loader;

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.container;
import eagine.core.identifier;
import eagine.core.reflection;
import eagine.core.value_tree;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.logging;
import eagine.core.progress;
import eagine.core.main_ctx;
import eagine.msgbus;

namespace eagine {
namespace app {
namespace exp {
//------------------------------------------------------------------------------
/// @brief Resource loading status.
/// @see resource_loader
/// @see loaded resource
/// @see resource_loader_signals
export enum class resource_status : std::uint8_t {
    created,
    loading,
    loaded,
    cancelled,
    not_found,
    error
};
//------------------------------------------------------------------------------
/// @brief Resource kind enumeration.
/// @see resource_loader
/// @see resource_loader_signals
export enum class resource_kind {
    /// @brief Unknown resource type.
    unknown,
    /// @brief Plain text.
    plain_text,
    /// @brief Visited valtree
    visited_valtree,
    /// @brief Vector of string values.
    string_list,
    /// @brief Vector of URL values.
    url_list,
    /// @brief Vector of floating-point values.
    float_list,
    /// @brief Vector of vec3 values.
    vec3_list,
    /// @brief Vector of mat4 values.
    mat4_list,
    /// @brief Smooth bezier curve of vec3 points.
    smooth_vec3_curve,
    /// @brief Camera parameters.
    camera_parameters,
    /// @brief User input setup on the execution context.
    input_setup,
    /// @brief GLSL text.
    glsl_text,
    /// @brief Shape generator.
    shape_generator,
    /// @brief OGLplus shape wrapper.
    gl_shape,
    /// @brief Shape geometry buffers, vao instructions and attribute bindings.
    gl_geometry_and_bindings,
    /// @brief Value tree.
    value_tree,
    /// @brief Value tree stream traversal.
    value_tree_traversal,
    /// @brief GLSL source string collection.
    glsl_source,
    /// @brief GL shader source include string.
    gl_shader_include,
    /// @brief GL shader object.
    gl_shader,
    /// @brief GL program object.
    gl_program,
    /// @brief GL texture object.
    gl_texture,
    /// @brief GL texture image parameters and data.
    gl_texture_image,
    /// @brief GL texture image update.
    gl_texture_update,
    /// @brief GL buffer object.
    gl_buffer,
    /// @brief GL buffer content update.
    gl_buffer_update,
    /// @brief Arbitrary structure with defined attribute mapping.
    mapped_struct,
    /// @brief Marks that resource request is finished.
    finished
};
//------------------------------------------------------------------------------
export class resource_loader;
//------------------------------------------------------------------------------
/// @brief Structure containing parameters for a resource request.
/// @see resource_loader
/// @see resource_request_result
export using msgbus::resource_request_params;
//------------------------------------------------------------------------------
export class resource_interface : public interface<resource_interface> {
public:
    struct loader : abstract<loader> {
        loader(
          resource_interface& resource,
          resource_request_params params) noexcept
          : _resource{resource}
          , _params{std::move(params)} {}

        auto locator() const noexcept -> const url& {
            return _params.locator;
        }

        template <std::derived_from<resource_interface> Resource>
        auto resource_as() const noexcept -> Resource& {
            assert(dynamic_cast<Resource*>(&_resource.get()));
            return static_cast<Resource&>(_resource.get());
        }

        virtual auto status() const noexcept -> resource_status = 0;

        virtual auto request_dependencies(
          const std::shared_ptr<resource_loader>&,
          resource_request_params&& params) noexcept -> bool = 0;

        virtual void stream_data_appended(
          const msgbus::blob_stream_chunk&) noexcept {}

        virtual void stream_finished() noexcept {}

        virtual void stream_cancelled() noexcept {}

    private:
        std::reference_wrapper<resource_interface> _resource;
        const resource_request_params _params;
    };

    virtual auto kind() const noexcept -> resource_kind = 0;

    virtual auto load_status() const noexcept -> resource_status = 0;

    auto is_loaded() const noexcept -> bool {
        return load_status() == resource_status::loaded;
    }

    auto can_be_loaded() const noexcept -> bool {
        return load_status() == resource_status::created;
    }

    explicit operator bool() const noexcept {
        return is_loaded();
    }

    virtual auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> = 0;

protected:
    template <typename Resource>
    class loader_of : public loader {
    public:
        using loader::loader;

        auto resource() const noexcept -> Resource& {
            return this->template resource_as<Resource>();
        }

        auto status() const noexcept -> resource_status override {
            return resource().load_status();
        }
    };
};
//------------------------------------------------------------------------------
/// @brief Loader of resources of various types.
/// @see pending_resource_requests
export class resource_loader
  : public std::enable_shared_from_this<resource_loader>
  , public msgbus::resource_data_consumer_node {
    using base = msgbus::resource_data_consumer_node;

public:
    /// @brief Initializing constructor
    resource_loader(msgbus::endpoint& bus);

    template <std::derived_from<resource_interface> Resource>
    auto load(Resource& resource, resource_request_params params) noexcept
      -> identifier_t {
        if(auto loader{resource.make_loader(std::move(params))}) {
            if(loader->request_dependencies(
                 shared_from_this(), std::move(params))) {
                return get_request_id();
            }
        }
        return {};
    }

    auto has_pending_resources() const noexcept -> bool;

    auto add_consumer(
      identifier_t request_id,
      const std::shared_ptr<resource_interface::loader>& l) noexcept
      -> resource_loader&;

    /// @brief Does some work and updates internal state (should be called periodically).
    auto update_and_process_all() noexcept -> work_done final;

private:
    void _handle_preparation_progressed(identifier_t blob_id, float) noexcept;
    void _handle_stream_data_appended(const msgbus::blob_stream_chunk&) noexcept;
    void _handle_stream_finished(identifier_t blob_id) noexcept;
    void _handle_stream_cancelled(identifier_t blob_id) noexcept;

    flat_map<identifier_t, shared_holder<resource_interface::loader>> _pending;
    flat_map<identifier_t, shared_holder<resource_interface::loader>> _consumer;
};
//------------------------------------------------------------------------------
} // namespace exp
export using exp::resource_loader;
} // namespace app
//------------------------------------------------------------------------------
export template <>
struct enumerator_traits<app::exp::resource_status> {
    static constexpr auto mapping() noexcept {
        using app::exp::resource_status;
        return enumerator_map_type<resource_status, 6>{
          {{"created", resource_status::created},
           {"loading", resource_status::loading},
           {"loaded", resource_status::loaded},
           {"cancelled", resource_status::cancelled},
           {"not_found", resource_status::not_found},
           {"error", resource_status::error}}};
    }
};
//------------------------------------------------------------------------------
} // namespace eagine
