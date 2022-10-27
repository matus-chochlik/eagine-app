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
      : _locator_str{locator.release_string()} {}

    /// @brief Returns this resource's URL.
    auto locator() const noexcept -> url {
        return {_locator_str};
    }

    /// @brief Indicates if this resource is currently loading.
    auto is_loading() const noexcept -> bool {
        return _request_id != 0;
    }

    /// @brief Compares resources for equality.
    auto operator==(const loaded_resource_base& that) const noexcept -> bool {
        return this->_locator_str == that._locator_str;
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
    std::string _locator_str;
    identifier_t _request_id{0};
};
//------------------------------------------------------------------------------
export template <typename Resource>
class loaded_resource;
//------------------------------------------------------------------------------
export template <>
class loaded_resource<gl_geometry_and_bindings>
  : public gl_geometry_and_bindings
  , public loaded_resource_base {
    using _res_t = gl_geometry_and_bindings;

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
        const loaded_resource<gl_geometry_and_bindings>& resource;
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

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.main_video(), ctx.loader());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, video_context& video, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(video, loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.main_video(), ctx.loader()} {}

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
                 locator(), video, draw_var_idx)}) {
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
                 locator(), video, bindings, draw_var_idx)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(execution_context& ctx, span_size_t draw_var_idx = 0)
      -> work_done {
        return update(ctx.main_video(), ctx.loader(), draw_var_idx);
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(
      execution_context& ctx,
      const oglplus::vertex_attrib_bindings& bindings,
      span_size_t draw_var_idx = 0) -> work_done {
        return update(ctx.main_video(), ctx.loader(), bindings, draw_var_idx);
    }

private:
    void _handle_gl_geometry_and_bindings_loaded(
      const resource_loader::gl_geometry_and_bindings_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->loaded({.base = info, .resource = *this});
                this->base_loaded(*this);
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using gl_geometry_and_bindings_resource =
  loaded_resource<gl_geometry_and_bindings>;
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

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader()} {}

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
            if(const auto request{loader.request_value_tree(locator())}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(execution_context& ctx) -> work_done {
        return update(ctx.loader());
    }

private:
    void _handle_value_tree_loaded(
      const resource_loader::value_tree_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.tree);
            if(is_loaded()) {
                this->loaded({.base = info, .resource = *this});
                this->base_loaded(*this);
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

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.main_video(), ctx.loader());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, video_context& video, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(video, loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.main_video(), ctx.loader()} {}

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
                 loader.request_gl_shader(locator(), type, video)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(video_context& video, resource_loader& loader) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{loader.request_gl_shader(locator(), video)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(execution_context& ctx, oglplus::shader_type type)
      -> work_done {
        return update(ctx.main_video(), ctx.loader(), type);
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(execution_context& ctx) -> work_done {
        return update(ctx.main_video(), ctx.loader());
    }

private:
    void _handle_gl_shader_loaded(
      const resource_loader::gl_shader_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->loaded({.base = info, .resource = *this});
                this->base_loaded(*this);
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

        /// @brief Sets the value of a uniform at the specified location.
        template <typename T>
        auto set_uniform(oglplus::uniform_location loc, const T& value)
          const noexcept {
            return base.set_uniform(loc, value);
        }

        /// @brief Sets the value of a uniform having the specified name.
        template <typename T>
        auto set_uniform(const string_view var_name, const T& value)
          const noexcept {
            return base.set_uniform(var_name, value);
        }

        /// @brief Returns the location of a storge block with the specified name.
        auto get_shader_storage_block_index(
          const string_view var_name) const noexcept {
            return base.get_shader_storage_block_index(var_name);
        }

        /// @brief Changes the buffer binding of a shader storage block.
        auto shader_storage_block_binding(
          oglplus::shader_storage_block_index index,
          oglplus::gl_types::uint_type binding) const noexcept {
            return base.shader_storage_block_binding(index, binding);
        }

        /// @brief Changes the buffer binding of a shader storage block.
        auto shader_storage_block_binding(
          const string_view var_name,
          oglplus::gl_types::uint_type binding) const noexcept {
            return base.shader_storage_block_binding(var_name, binding);
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

    /// @brief Makes the current program active within the given execution context.
    auto use(execution_context& ctx) noexcept -> loaded_resource& {
        return use(ctx.main_video());
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

    /// @brief Binds the location of a input attribute variable.
    auto bind(
      const oglplus::gl_api& glapi,
      oglplus::vertex_attrib_location loc,
      const string_view var_name) -> loaded_resource& {
        glapi.bind_attrib_location(*this, loc, var_name);
        return *this;
    }

    /// @brief Binds the location of a input attribute variable.
    auto bind(
      video_context& video,
      oglplus::vertex_attrib_location loc,
      const string_view var_name) -> loaded_resource& {
        return bind(video.gl_api(), loc, var_name);
    }

    /// @brief Applies the vertex attrib bindings to the loaded program.
    auto apply_input_bindings(
      video_context& video,
      const oglplus::vertex_attrib_bindings& attrib_bindings) const noexcept
      -> bool {
        return _inputs.apply(video.gl_api(), *this, attrib_bindings);
    }

    /// @brief Clean's up this resource.
    void clean_up(video_context& video, resource_loader& loader) {
        video.gl_api().clean_up(std::move(_res()));
        if(_sig_key) {
            loader.gl_program_loaded.disconnect(_sig_key);
            _sig_key = {};
        }
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        return clean_up(ctx.main_video(), ctx.loader());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, video_context& video, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(video, loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.main_video(), ctx.loader()} {}

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
            if(const auto request{
                 loader.request_gl_program(locator(), video)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(execution_context& ctx) -> work_done {
        return update(ctx.main_video(), ctx.loader());
    }

private:
    void _handle_gl_program_loaded(
      const resource_loader::gl_program_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            _inputs = info.input_bindings;
            if(is_loaded()) {
                this->loaded({.base = info, .resource = *this});
                this->base_loaded(*this);
                _request_id = 0;
            }
        }
    }

    oglplus::program_input_bindings _inputs;
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

        /// @brief Sets the specified floating-point texture parameter value.
        template <typename Param, typename Value>
        auto parameter_f(Param param, Value value) const noexcept {
            return base.parameter_f(param, value);
        }

        /// @brief Sets the specified integral texture parameter value.
        template <typename Param, typename Value>
        auto parameter_i(Param param, Value value) const noexcept {
            return base.parameter_i(param, value);
        }

        /// @brief Generates texture mipmap.
        auto generate_mipmap() const noexcept {
            return base.generate_mipmap();
        }
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

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.main_video(), ctx.loader());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, video_context& video, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(video, loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.main_video(), ctx.loader()} {}

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
                 loader.request_gl_texture(locator(), video, target, unit)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto update(
      execution_context& ctx,
      oglplus::texture_target target,
      oglplus::texture_unit unit) -> work_done {
        return update(ctx.main_video(), ctx.loader(), target, unit);
    }

private:
    void _handle_gl_texture_loaded(
      const resource_loader::gl_texture_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->loaded({.base = info, .resource = *this});
                this->base_loaded(*this);
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using gl_texture_resource = loaded_resource<oglplus::owned_texture_name>;
//------------------------------------------------------------------------------
export template <>
class loaded_resource<oglplus::owned_buffer_name>
  : public oglplus::owned_buffer_name
  , public loaded_resource_base {
    using _res_t = oglplus::owned_buffer_name;

    auto _res() noexcept -> _res_t& {
        return *this;
    }

public:
    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const loaded_resource_base&) noexcept> base_loaded;

    /// @brief Type of the loaded signal parameter.
    struct load_info {
        /// @brief The base info from the loader signal.
        const resource_loader::gl_buffer_load_info& base;
        /// @brief The loaded buffer resource.
        const loaded_resource<oglplus::owned_buffer_name>& resource;
    };

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(video_context&, resource_loader& loader) {
        _sig_key = connect<&loaded_resource::_handle_gl_buffer_loaded>(
          this, loader.gl_buffer_loaded);
    }

    /// @brief Clean's up this resource.
    void clean_up(video_context& video, resource_loader& loader) {
        video.gl_api().clean_up(std::move(_res()));
        if(_sig_key) {
            loader.gl_buffer_loaded.disconnect(_sig_key);
            _sig_key = {};
        }
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, video_context& video, resource_loader& loader)
      : loaded_resource{std::move(locator)} {
        init(video, loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.main_video(), ctx.loader()} {}

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
      oglplus::buffer_target target) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{
                 loader.request_gl_buffer(locator(), video, target)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

private:
    void _handle_gl_buffer_loaded(
      const resource_loader::gl_buffer_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _res() = std::move(info.ref);
            if(is_loaded()) {
                this->loaded({.base = info, .resource = *this});
                this->base_loaded(*this);
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
export using gl_buffer_resource = loaded_resource<oglplus::owned_buffer_name>;
//------------------------------------------------------------------------------
} // namespace eagine::app
