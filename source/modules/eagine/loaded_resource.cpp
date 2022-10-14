/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:loaded_resource;

import eagine.core.types;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.shapes;
import eagine.oglplus;
import :context;
import :geometry;
import :resource_loader;

namespace eagine::app {
//------------------------------------------------------------------------------
/// @brief Common base for loaded resource template.
/// @see resource_loader
export class loaded_resource_base {
public:
    loaded_resource_base(url locator) noexcept
      : _locator{std::move(locator)} {}

    /// @brief Returns this resource's URL.
    auto locator() const noexcept -> const url& {
        return _locator;
    }

    /// @brief Indicates if this resource is currently loading.
    auto is_loading() const noexcept -> bool {
        return _request_id != 0;
    }

protected:
    url _locator;
    identifier_t _request_id{0};
};
//------------------------------------------------------------------------------
export template <typename Resource>
class loaded_resource;
//------------------------------------------------------------------------------
export template <>
class loaded_resource<oglplus::owned_program_name>
  : public oglplus::owned_program_name
  , public loaded_resource_base {
    using _res_t = oglplus::owned_program_name;

    auto _res() noexcept -> _res_t& {
        return *this;
    }

public:
    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(
      oglplus::program_name,
      const oglplus::program_input_bindings&) noexcept>
      loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(video_context&, resource_loader& loader) {
        _sig_key = connect<&loaded_resource::_handle_gl_program_loaded>(
          this, loader.gl_program_loaded);
    }

    /// @brief Clean's up this resource.
    void clean_up(video_context& video, resource_loader& loader) {
        video.gl_api().clean_up(std::move(_res()));
        if(_sig_key) {
            loader.gl_program_loaded.disconnect(_sig_key);
            _sig_key = {};
        }
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, video_context& video, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(video, loader);
    }

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return this->is_valid();
    }

    /// @brief Indicates if this resource is loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(video_context& video, resource_loader& loader) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{loader.request_gl_program(_locator, video)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

private:
    void _handle_gl_program_loaded(
      const identifier_t request_id,
      oglplus::program_name prog,
      oglplus::owned_program_name& ref,
      const oglplus::program_input_bindings& input_bindings,
      const url&) noexcept {
        if(request_id == _request_id) {
            _res() = std::move(ref);
            if(is_loaded()) {
                _request_id = 0;
                this->loaded(prog, input_bindings);
            }
        }
    }

    signal_binding_key _sig_key{};
};
//------------------------------------------------------------------------------
export template <>
class loaded_resource<geometry_and_bindings>
  : public geometry_and_bindings
  , public loaded_resource_base {
    using _res_t = geometry_and_bindings;

    auto _res() noexcept -> _res_t& {
        return *this;
    }

public:
    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const geometry_and_bindings& geom) noexcept> loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(video_context&, resource_loader& loader) {
        _sig_key =
          connect<&loaded_resource::_handle_gl_geometry_and_bindings_loaded>(
            this, loader.gl_geometry_and_bindings_loaded);
    }

    /// @brief Clean's up this resource.
    void clean_up(video_context&, resource_loader& loader) {
        if(_sig_key) {
            loader.gl_geometry_and_bindings_loaded.disconnect(_sig_key);
            _sig_key = {};
        }
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, video_context& video, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(video, loader);
    }

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return this->is_initialized();
    }

    /// @brief Indicates if this resource is loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(
      video_context& video,
      resource_loader& loader,
      const oglplus::vertex_attrib_bindings& bindings,
      span_size_t draw_var_idx = 0) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{loader.request_gl_geometry_and_bindings(
                 _locator, video, bindings, draw_var_idx)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

private:
    void _handle_gl_geometry_and_bindings_loaded(
      const identifier_t request_id,
      geometry_and_bindings& ref,
      const url&) noexcept {
        if(request_id == _request_id) {
            _res() = std::move(ref);
            if(is_loaded()) {
                _request_id = 0;
                this->loaded(_res());
            }
        }
    }

    signal_binding_key _sig_key{};
};
//------------------------------------------------------------------------------
} // namespace eagine::app
