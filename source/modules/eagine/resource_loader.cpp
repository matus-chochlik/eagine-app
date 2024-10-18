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
export class loaded_resource_context;
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
export class resource_loader;
//------------------------------------------------------------------------------
/// @brief Structure containing parameters for a resource request.
/// @see resource_loader
export using msgbus::resource_request_params;
//------------------------------------------------------------------------------
/// @brief Interface for resources loadable by the resource loader.
/// @see resource_loader
export class resource_interface : public interface<resource_interface> {
public:
    /// @brief Structure containing basic information about resource load status.
    struct load_info {
        /// @brief The locator of the requested resource.
        const url& locator;
        /// @brief The resource_loader request identifier.
        identifier_t request_id{0};
        /// @brief The resource kind identifier.
        identifier kind{};
        /// @brief The resource load status.
        resource_status status{resource_status::created};
    };

    /// @brief Base class for resource-specific temporary loader objects.
    /// @see make_loader
    /// @note Do not construct and use directly, use resource_loader instead.
    struct loader
      : abstract<loader>
      , main_ctx_object {

        loader(
          main_ctx_parent parent,
          resource_interface& resource,
          const shared_holder<loaded_resource_context>& context,
          resource_request_params params) noexcept;

        /// @brief Returns the resource_loader identifier for the associated request.
        auto request_id() const noexcept -> identifier_t {
            return _request_id;
        }

        /// @brief Returns a reference to the resource context (if any).
        auto resource_context() const noexcept
          -> const shared_holder<loaded_resource_context>& {
            return _context;
        }

        /// @brief Returns the associated resource request parameters.
        auto parameters() const noexcept -> const resource_request_params& {
            return _params;
        }

        /// @brief Returns the URL of the associated request.
        auto locator() const noexcept -> const url& {
            return parameters().locator;
        }

        /// @brief Returns a downcast reference to the loaded resource.
        template <std::derived_from<resource_interface> Resource>
        auto resource_as() const noexcept -> Resource& {
            assert(dynamic_cast<Resource*>(&_resource.get()));
            return static_cast<Resource&>(_resource.get());
        }

        /// @brief Returns the resource loading status.
        virtual auto status() const noexcept -> resource_status = 0;

        /// @brief Request resource-specific dependencies for associated loaded resource.
        virtual auto request_dependencies(resource_loader&) noexcept
          -> valid_if_not_zero<identifier_t> = 0;

        virtual void stream_data_appended(
          const msgbus::blob_stream_chunk&) noexcept;

        virtual void stream_finished(identifier_t) noexcept;

        virtual void stream_cancelled(identifier_t) noexcept;

        virtual void resource_loaded(const load_info&) noexcept;

        virtual void resource_cancelled(const load_info&) noexcept;

        virtual void resource_error(const load_info&) noexcept;

    protected:
        auto _set_request_id(identifier_t req_id) noexcept -> identifier_t;

        void _notify_loaded(resource_loader&) noexcept;
        void _notify_cancelled(resource_loader&) noexcept;
        void _notify_error(resource_loader&, resource_status status) noexcept;

    private:
        identifier_t _request_id{0};
        std::reference_wrapper<resource_interface> _resource;
        shared_holder<loaded_resource_context> _context;
        const resource_request_params _params;
    };

    /// @brief Returns the identifier of the kind of this resource.
    virtual auto kind() const noexcept -> identifier = 0;

    /// @brief Returns the resource life-cycle / loading status of this resource.
    virtual auto load_status() const noexcept -> resource_status = 0;

    /// @brief Indicates if this resource has been loaded and is ready for use.
    /// @see load_status
    /// @see is_loading
    /// @see can_be_loaded
    auto is_loaded() const noexcept -> bool {
        return load_status() == resource_status::loaded;
    }

    /// @brief Indicates if this resource is currently being loaded.
    /// @see load_status
    /// @see is_loaded
    /// @see can_be_loaded
    auto is_loading() const noexcept -> bool {
        return load_status() == resource_status::loading;
    }

    /// @brief Indicates if this resource is in a state when it can be loaded.
    /// @see load_status
    /// @see is_loaded
    /// @see is_loading
    auto can_be_loaded() const noexcept -> bool {
        return load_status() == resource_status::created;
    }

    auto should_be_loaded() const noexcept -> bool {
        return can_be_loaded();
    }

    /// @brief Indicates if this resource has been loaded and is ready for use.
    /// @see load_status
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Constructs a temporary resource-specific loader object.
    /// @note Do not use directly, use resource_loader instead.
    virtual auto make_loader(
      main_ctx_parent,
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> = 0;

    /// @brief Cleans-up the resource within the specified context.
    virtual void clean_up(loaded_resource_context&) noexcept;

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

    template <typename Resource, typename Loader = typename Resource::_loader>
    class simple_loader_of
      : public std::enable_shared_from_this<Loader>
      , public loader_of<Resource> {
    public:
        using loader_of<Resource>::loader_of;

        using loader_of<Resource>::resource;

        void stream_finished(identifier_t) noexcept override {
            mark_loaded();
        }

        void stream_cancelled(identifier_t) noexcept override {
            mark_cancelled();
        }

        void resource_loaded(const load_info&) noexcept override {
            mark_loaded();
        }

        void resource_cancelled(const load_info&) noexcept override {
            mark_cancelled();
        }

        void resource_error(const load_info& info) noexcept override {
            mark_error(info.status);
        }

        void set_status(resource_status status) noexcept {
            resource()._private_set_status(status);
        }

    protected:
        auto derived() noexcept -> Loader& {
            return *static_cast<Loader*>(this);
        }

        void mark_loaded() noexcept {
            derived().set_status(resource_status::loaded);
            if(_res_loader) [[likely]] {
                this->_notify_loaded(*_res_loader);
            }
        }

        void mark_cancelled() noexcept {
            derived().set_status(resource_status::cancelled);
            if(_res_loader) [[likely]] {
                this->_notify_cancelled(*_res_loader);
            }
        }

        void mark_error(
          resource_status status = resource_status::error) noexcept {
            derived().set_status(status);
            if(_res_loader) [[likely]] {
                this->_notify_error(*_res_loader, status);
            }
        }

        auto _add_single_dependency(
          valid_if_not_zero<identifier_t> req_id,
          resource_loader& res_loader) noexcept
          -> valid_if_not_zero<identifier_t>;

        optional_reference<resource_loader> _res_loader;
    };
};
//------------------------------------------------------------------------------
/// @brief Loader of resources of various types.
/// @see pending_resource_requests
export class resource_loader : public msgbus::resource_data_consumer_node {
    using base = msgbus::resource_data_consumer_node;

public:
    /// @brief Initializing constructor
    resource_loader(msgbus::endpoint& bus);

    /// @brief Signal emitted when a resource is successfully loaded.
    /// @see resource_cancelled
    /// @see resource_error
    signal<void(const resource_interface::load_info&) noexcept> resource_loaded;

    /// @brief Signal emitted when resource loading has been cancelled.
    /// @see resource_loaded
    /// @see resource_error
    signal<void(const resource_interface::load_info&) noexcept>
      resource_cancelled;

    /// @brief Signal emitted when resource loading encountered an error.
    /// @see resource_loaded
    /// @see resource_cancelled
    signal<void(const resource_interface::load_info&) noexcept> resource_error;

    auto load_any(
      resource_interface& resource,
      const shared_holder<loaded_resource_context>& context,
      resource_request_params params) noexcept
      -> valid_if_not_zero<identifier_t>;

    /// @brief Loads the specified resource in context with the specified parameters.
    /// @see load_if_needed
    /// @note The resource may not get destroyed while it is being loaded.
    template <std::derived_from<resource_interface> Resource>
    auto load(
      Resource& resource,
      const shared_holder<loaded_resource_context>& context,
      resource_request_params params) noexcept
      -> valid_if_not_zero<identifier_t> {
        return load_any(resource, context, std::move(params));
    }

    /// @brief Loads resource if necessary, using param_getter to get the load parameters.
    /// @see load
    /// @note The resource may not get destroyed while it is being loaded.
    template <std::derived_from<resource_interface> Resource, typename Getter>
    auto load_if_needed(
      Resource& resource,
      const shared_holder<loaded_resource_context>& context,
      const Getter& param_getter) noexcept -> valid_if_not_zero<identifier_t> {
        if(resource.should_be_loaded()) {
            return load(resource, context, param_getter());
        }
        return {0};
    }

    /// @brief Indicates if the loader is currently loading any resources.
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
      const identifier_t,
      const url&,
      resource_interface&) noexcept;

    void _handle_resource_cancelled(
      const identifier_t,
      const url&,
      const resource_interface&) noexcept;

    void _handle_resource_error(
      const identifier_t,
      const url&,
      const resource_interface&,
      const resource_status status) noexcept;

    flat_map<identifier_t, shared_holder<resource_interface::loader>> _pending;
    flat_map<identifier_t, shared_holder<resource_interface::loader>> _consumer;
};
//------------------------------------------------------------------------------
template <typename Resource, typename Loader>
auto resource_interface::simple_loader_of<Resource, Loader>::
  _add_single_dependency(
    valid_if_not_zero<identifier_t> req_id,
    resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    if(req_id) {
        res_loader.add_consumer(
          req_id.value_anyway(), this->shared_from_this());
        _res_loader = res_loader;
        derived().set_status(resource_status::loading);
        return {this->_set_request_id(res_loader.get_request_id())};
    }
    derived().set_status(resource_status::error);
    return {0};
}
//------------------------------------------------------------------------------
template <typename T>
class simple_resource : public resource_interface {
public:
    using resource_type = T;

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_resource);
    }

    auto get() const noexcept -> const resource_type& {
        return _resource;
    }

    auto operator->() const noexcept -> const resource_type* {
        return &_resource;
    }

    auto _private_ref() noexcept -> resource_type& {
        return _resource;
    }

    void _private_set_status(resource_status s) noexcept {
        _status = s;
    }

private:
    T _resource;
    resource_status _status{resource_status::created};
};
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
