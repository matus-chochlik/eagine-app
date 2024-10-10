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
import eagine.core.valid_if;
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
    struct load_info {
        const url& locator;
        identifier_t request_id{0};
        resource_kind kind{};
    };

    struct loader : abstract<loader> {
        loader(
          resource_interface& resource,
          resource_request_params params) noexcept
          : _resource{resource}
          , _params{std::move(params)} {}

        auto request_id() const noexcept -> identifier_t {
            return _request_id;
        }

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
          resource_request_params&& params) noexcept
          -> valid_if_not_zero<identifier_t> = 0;

        virtual void stream_data_appended(
          const msgbus::blob_stream_chunk&) noexcept;

        virtual void stream_finished() noexcept;

        virtual void stream_cancelled() noexcept;

        virtual void resource_loaded(const load_info&) noexcept;

        virtual void resource_cancelled(const load_info&) noexcept;

    protected:
        auto _set_request_id(identifier_t req_id) noexcept -> identifier_t;

        void _notify_loaded(resource_loader&) noexcept;
        void _notify_cancelled(resource_loader&) noexcept;

    private:
        identifier_t _request_id{0};
        std::reference_wrapper<resource_interface> _resource;
        const resource_request_params _params;
    };

    virtual auto kind() const noexcept -> resource_kind = 0;

    virtual auto load_status() const noexcept -> resource_status = 0;

    auto is_loaded() const noexcept -> bool {
        return load_status() == resource_status::loaded;
    }

    auto is_loading() const noexcept -> bool {
        return load_status() == resource_status::loading;
    }

    auto can_be_loaded() const noexcept -> bool {
        return load_status() == resource_status::created;
    }

    auto should_be_loaded() const noexcept -> bool {
        return not is_loaded() and not is_loaded() and can_be_loaded();
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

    template <typename Resource>
    class simple_loader_of
      : public std::enable_shared_from_this<typename Resource::_loader>
      , public loader_of<Resource> {
    public:
        using loader_of<Resource>::loader_of;

        using loader_of<Resource>::resource;

        void stream_finished() noexcept override {
            derived().set_status(resource_status::loaded);
            if(_res_loader) [[likely]] {
                this->_notify_loaded(*_res_loader);
            }
        }

        void stream_cancelled() noexcept override {
            derived().set_status(resource_status::cancelled);
            if(_res_loader) [[likely]] {
                this->_notify_cancelled(*_res_loader);
            }
        }

    protected:
        auto derived() noexcept -> typename Resource::_loader& {
            return *static_cast<typename Resource::_loader*>(this);
        }

        auto _add_single_dependency(
          identifier_t req_id,
          shared_holder<resource_loader> res_loader) noexcept -> identifier_t;

        shared_holder<resource_loader> _res_loader;

    private:
        identifier_t _dep_req_id{0};
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

    signal<void(const resource_interface::load_info&) noexcept> resource_loaded;

    signal<void(const resource_interface::load_info&) noexcept>
      resource_cancelled;

    template <std::derived_from<resource_interface> Resource>
    auto load(Resource& resource, resource_request_params params) noexcept
      -> identifier_t {
        if(auto loader{resource.make_loader(std::move(params))}) {
            if(auto req_id{loader->request_dependencies(
                 shared_from_this(), std::move(params))}) {
                return req_id.value_anyway();
            }
        }
        return {};
    }

    template <std::derived_from<resource_interface> Resource, typename Getter>
    auto load_if_needed(Resource& resource, const Getter& param_getter) noexcept
      -> identifier_t {
        if(resource.should_be_loaded()) {
            return load(resource, param_getter());
        }
        return 0;
    }

    auto has_pending_resources() const noexcept -> bool;

    auto add_consumer(
      identifier_t request_id,
      const std::shared_ptr<resource_interface::loader>& l) noexcept
      -> resource_loader&;

    /// @brief Does some work and updates internal state (should be called periodically).
    auto update_and_process_all() noexcept -> work_done final;

private:
    friend class resource_interface::loader;

    void _handle_preparation_progressed(identifier_t blob_id, float) noexcept;
    void _handle_stream_data_appended(const msgbus::blob_stream_chunk&) noexcept;
    void _handle_stream_finished(identifier_t blob_id) noexcept;
    void _handle_stream_cancelled(identifier_t blob_id) noexcept;

    void _handle_resource_loaded(
      identifier_t,
      const url&,
      resource_interface&) noexcept;

    void _handle_resource_cancelled(
      identifier_t,
      const url&,
      const resource_interface&) noexcept;

    flat_map<identifier_t, shared_holder<resource_interface::loader>> _pending;
    flat_map<identifier_t, shared_holder<resource_interface::loader>> _consumer;
};
//------------------------------------------------------------------------------
template <typename Resource>
auto resource_interface::simple_loader_of<Resource>::_add_single_dependency(
  identifier_t req_id,
  shared_holder<resource_loader> res_loader) noexcept -> identifier_t {
    if(req_id > 0) {
        _res_loader = std::move(res_loader);
        _res_loader->add_consumer(req_id, this->shared_from_this());
        _dep_req_id = req_id;
        derived().set_status(resource_status::loading);
        return this->_set_request_id(res_loader->get_request_id());
    }
    derived().set_status(resource_status::error);
    return 0;
}
//------------------------------------------------------------------------------
} // namespace exp
export using exp::resource_interface;
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
