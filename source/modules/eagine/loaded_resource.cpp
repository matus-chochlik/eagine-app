/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:loaded_resource;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.value_tree;
import eagine.shapes;
import eagine.oglplus;
import :context;
import :geometry;
import :resource_loader;
import <concepts>;

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

    /// @brief Compares resources for equality.
    auto operator==(const loaded_resource_base& that) const noexcept -> bool {
        return this->_locator == that._locator;
    }

    /// @brief Indicates if this resource is the same as that resource.
    auto is(const loaded_resource_base& that) const noexcept -> bool {
        return (*this) == that;
    }

    /// @brief Indicates if this resource is one in the specified collection.
    template <std::derived_from<loaded_resource_base>... R>
    auto is_one_of(const R&... those) const noexcept -> bool {
        return (... || is(those));
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
class loaded_resource<geometry_and_bindings>
  : public geometry_and_bindings
  , public loaded_resource_base {
    using _res_t = geometry_and_bindings;

    auto _res() noexcept -> _res_t& {
        return *this;
    }

public:
    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const loaded_resource_base&) noexcept> base_loaded;

    /// @brief Type of the loaded signal parameter.
    struct load_info {
        /// @brief The base info from the loader signal.
        const resource_loader::gl_geometry_and_bindings_load_info& base;
        /// @brief The loaded geometry resource.
        const loaded_resource<geometry_and_bindings>& resource;
    };

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

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
    void clean_up(video_context& video, resource_loader& loader) {
        _res().clean_up(video);
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
      span_size_t draw_var_idx = 0) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{loader.request_gl_geometry_and_bindings(
                 _locator, video, draw_var_idx)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
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
      const resource_loader::gl_geometry_and_bindings_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->base_loaded(*this);
                this->loaded({.base = info, .resource = *this});
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using geometry_and_bindings_resource =
  loaded_resource<geometry_and_bindings>;
//------------------------------------------------------------------------------
export template <>
class loaded_resource<valtree::compound>
  : public valtree::compound
  , public loaded_resource_base {
    using _res_t = valtree::compound;

    auto _res() noexcept -> _res_t& {
        return *this;
    }

public:
    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const loaded_resource_base&) noexcept> base_loaded;

    /// @brief Type of the loaded signal parameter.
    struct load_info {
        /// @brief The base info from the loader signal.
        const resource_loader::value_tree_load_info& base;
        /// @brief The loaded geometry resource.
        const loaded_resource<valtree::compound>& resource;
    };

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(resource_loader& loader) {
        _sig_key = connect<&loaded_resource::_handle_value_tree_loaded>(
          this, loader.value_tree_loaded);
    }

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader) {
        if(_sig_key) {
            loader.value_tree_loaded.disconnect(_sig_key);
            _sig_key = {};
        }
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(loader);
    }

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return bool(static_cast<const valtree::compound&>(*this));
    }

    /// @brief Indicates if this resource is loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(resource_loader& loader) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{loader.request_value_tree(_locator)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

private:
    void _handle_value_tree_loaded(
      const resource_loader::value_tree_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.tree);
            if(is_loaded()) {
                this->base_loaded(*this);
                this->loaded({.base = info, .resource = *this});
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using value_tree_resource = loaded_resource<valtree::compound>;
//------------------------------------------------------------------------------
export template <>
class loaded_resource<oglplus::owned_shader_name>
  : public oglplus::owned_shader_name
  , public loaded_resource_base {
    using _res_t = oglplus::owned_shader_name;

    auto _res() noexcept -> _res_t& {
        return *this;
    }

public:
    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const loaded_resource_base&) noexcept> base_loaded;

    /// @brief Type of the loaded signal parameter.
    struct load_info {
        /// @brief The base info from the loader signal.
        const resource_loader::gl_shader_load_info& base;
        /// @brief The loaded shader resource.
        const loaded_resource<oglplus::owned_shader_name>& resource;
    };

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(video_context&, resource_loader& loader) {
        _sig_key = connect<&loaded_resource::_handle_gl_shader_loaded>(
          this, loader.gl_shader_loaded);
    }

    /// @brief Clean's up this resource.
    void clean_up(video_context& video, resource_loader& loader) {
        video.gl_api().clean_up(std::move(_res()));
        if(_sig_key) {
            loader.gl_shader_loaded.disconnect(_sig_key);
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
    auto update(
      video_context& video,
      resource_loader& loader,
      oglplus::shader_type type) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{
                 loader.request_gl_shader(_locator, type, video)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(video_context& video, resource_loader& loader) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{loader.request_gl_shader(_locator, video)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

private:
    void _handle_gl_shader_loaded(
      const resource_loader::gl_shader_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->base_loaded(*this);
                this->loaded({.base = info, .resource = *this});
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using gl_shader_resource = loaded_resource<oglplus::owned_shader_name>;
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
    signal<void(const loaded_resource_base&) noexcept> base_loaded;

    /// @brief Type of the loaded signal parameter.
    struct load_info {
        /// @brief The base info from the loader signal.
        const resource_loader::gl_program_load_info& base;
        /// @brief The loaded program resource.
        const loaded_resource<oglplus::owned_program_name>& resource;

        /// @brief Applies the vertex attrib bindings to the loaded program.
        auto apply_input_bindings(
          const oglplus::vertex_attrib_bindings& attrib_bindings) const noexcept
          -> bool {
            return base.apply_input_bindings(attrib_bindings);
        }

        /// @brief Uses the loaded program in the relevant GL context.
        auto use_program() const noexcept {
            return base.use_program();
        }

        /// @brief Returns the location of a uniform with the specified name.
        auto get_uniform_location(const string_view var_name) const noexcept {
            return base.get_uniform_location(var_name);
        }
    };

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(video_context&, resource_loader& loader) {
        _sig_key = connect<&loaded_resource::_handle_gl_program_loaded>(
          this, loader.gl_program_loaded);
    }

    /// @brief Makes the current program active within the given API object.
    auto use(const oglplus::gl_api& glapi) noexcept -> loaded_resource& {
        glapi.use_program(*this);
        return *this;
    }

    /// @brief Makes the current program active within the given video context.
    auto use(video_context& video) noexcept -> loaded_resource& {
        return use(video.gl_api());
    }

    /// @brief Sets the value of a uniform variable
    template <typename T>
    auto set(
      const oglplus::gl_api& glapi,
      oglplus::uniform_location loc,
      T&& value) -> loaded_resource& {
        glapi.set_uniform(*this, loc, std::forward<T>(value));
        return *this;
    }

    /// @brief Sets the value of a uniform variable
    template <typename T>
    auto set(video_context& video, oglplus::uniform_location loc, T&& value)
      -> loaded_resource& {
        return set(video.gl_api(), loc, std::forward<T>(value));
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
      const resource_loader::gl_program_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->base_loaded(*this);
                this->loaded({.base = info, .resource = *this});
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using gl_program_resource = loaded_resource<oglplus::owned_program_name>;
//------------------------------------------------------------------------------
export template <>
class loaded_resource<oglplus::owned_texture_name>
  : public oglplus::owned_texture_name
  , public loaded_resource_base {
    using _res_t = oglplus::owned_texture_name;

    auto _res() noexcept -> _res_t& {
        return *this;
    }

public:
    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const loaded_resource_base&) noexcept> base_loaded;

    /// @brief Type of the loaded signal parameter.
    struct load_info {
        /// @brief The base info from the loader signal.
        const resource_loader::gl_texture_load_info& base;
        /// @brief The loaded texture resource.
        const loaded_resource<oglplus::owned_texture_name>& resource;
    };

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(video_context&, resource_loader& loader) {
        _sig_key = connect<&loaded_resource::_handle_gl_texture_loaded>(
          this, loader.gl_texture_loaded);
    }

    /// @brief Clean's up this resource.
    void clean_up(video_context& video, resource_loader& loader) {
        video.gl_api().clean_up(std::move(_res()));
        if(_sig_key) {
            loader.gl_texture_loaded.disconnect(_sig_key);
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
    auto update(
      video_context& video,
      resource_loader& loader,
      oglplus::texture_target target,
      oglplus::texture_unit unit) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{
                 loader.request_gl_texture(_locator, video, target, unit)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

private:
    void _handle_gl_texture_loaded(
      const resource_loader::gl_texture_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->base_loaded(*this);
                this->loaded({.base = info, .resource = *this});
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using gl_texture_resource = loaded_resource<oglplus::owned_texture_name>;
//------------------------------------------------------------------------------
} // namespace eagine::app
