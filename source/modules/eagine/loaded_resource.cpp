/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:loaded_resource;

import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.reflection;
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

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const loaded_resource_base&) noexcept> base_loaded;

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

export template <typename Resource>
struct get_resource_load_params : std::type_identity<std::tuple<>> {};

export template <typename Resource>
struct get_resource_load_params<loaded_resource<Resource>>
  : get_resource_load_params<Resource> {};

export template <typename Resource>
using resource_load_params = typename get_resource_load_params<Resource>::type;

export template <typename Params>
struct get_resource_load_context_params : std::type_identity<Params> {};

export template <typename... P>
struct get_resource_load_context_params<std::tuple<video_context&, P...>>
  : std::type_identity<std::tuple<P...>> {};

export template <typename Params>
using resource_load_context_params =
  typename get_resource_load_context_params<Params>::type;
//------------------------------------------------------------------------------
export template <
  typename Resource,
  typename Params = resource_load_params<Resource>>
struct resource_load_utils;

export template <typename Resource, typename... P>
struct resource_load_utils<Resource, std::tuple<P...>> {

    auto request(resource_loader& loader, url locator, P... params)
      -> resource_request_result {
        return loader.request(
          std::type_identity<Resource>{}, std::move(locator), params...);
    }

    auto request(execution_context& ctx, url locator, P... params)
      -> resource_request_result {
        return ctx.loader().request(
          std::type_identity<Resource>{}, std::move(locator), params...);
    }
};

export template <typename Resource, typename... P>
struct resource_load_utils<Resource, std::tuple<video_context&, P...>> {

    auto request(
      resource_loader& loader,
      url locator,
      video_context& video,
      P... params) -> resource_request_result {
        return loader.request(
          std::type_identity<Resource>{}, std::move(locator), video, params...);
    }

    auto request(execution_context& ctx, url locator, P... params)
      -> resource_request_result {
        return ctx.loader().request(
          std::type_identity<Resource>{},
          std::move(locator),
          ctx.main_video(),
          params...);
    }
};
//------------------------------------------------------------------------------
export template <typename Resource>
class loaded_resource
  : public Resource
  , public loaded_resource_base
  , public resource_load_utils<Resource> {
    static_assert(default_mapped_struct<Resource>);

    using utils = resource_load_utils<Resource>;

public:
    using base_load_info =
      typename resource_loader::load_info_t<valtree::compound>;

    /// @brief Returns a reference to the underlying resource.
    auto resource() noexcept -> Resource& {
        return *this;
    }

    /// @brief Returns a const reference to the underlying resource.
    auto resource() const noexcept -> const Resource& {
        return *this;
    }

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(resource_loader& loader) noexcept {
        _connect(loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader)
      : loaded_resource_base{std::move(locator)} {
        init(loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader()} {}

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader) {
        this->resource().clear();
        _disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader());
    }

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return _loaded;
    }

    /// @brief Indicates if this resource is loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(resource_loader& loader) -> work_done {
        if(!is_loaded() && !is_loading()) {
            if(const auto request{utils::request(loader, locator())}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(execution_context& ctx) -> work_done {
        return load_if_needed(ctx.loader());
    }

private:
    void _connect(resource_loader& loader) noexcept {
        _sig_key = connect<&loaded_resource::_handle_loaded>(
          this, loader.value_tree_loaded);
    }

    void _disconnect(resource_loader& loader) noexcept {
        if(_sig_key) {
            loader.value_tree_loaded.disconnect(_sig_key);
            _sig_key = {};
        }
    }

    void _handle_loaded(const base_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            _loaded = info.load(resource());
            base_loaded(*this);
            _request_id = 0;
        }
    }

    signal_binding_key _sig_key{};
    bool _loaded{false};
};
//------------------------------------------------------------------------------
export template <typename Resource>
class loaded_resource;

export template <
  typename Derived,
  typename Params = resource_load_params<Derived>,
  typename CtxParams = resource_load_context_params<Params>>
class loaded_resource_common;

export template <typename Resource>
struct resource_load_info {
    using base_load_info = typename resource_loader::load_info_t<Resource>;

    /// @brief The base info from the loader signal.
    const base_load_info& base;
    /// @brief The loaded geometry resource.
    const loaded_resource<Resource>& resource;

    resource_load_info(
      const base_load_info& info,
      loaded_resource<Resource>& parent) noexcept
      : base{info}
      , resource{parent} {}
};
//------------------------------------------------------------------------------
export template <typename Resource, typename... P, typename... Cp>
class loaded_resource_common<
  loaded_resource<Resource>,
  std::tuple<P...>,
  std::tuple<Cp...>>
  : public Resource
  , public loaded_resource_base
  , public resource_load_utils<Resource> {
    using base = loaded_resource_base;
    using utils = resource_load_utils<Resource>;

    using Derived = loaded_resource<Resource>;

    auto derived() noexcept -> Derived& {
        return *static_cast<Derived*>(this);
    }

    auto derived() const noexcept -> const Derived& {
        return *static_cast<const Derived*>(this);
    }

public:
    static constexpr auto resource_tid() noexcept
      -> std::type_identity<Resource> {
        return {};
    }

    /// @brief Type of the base_loaded signal parameter.
    using base_load_info = typename resource_loader::load_info_t<Resource>;

    /// @brief Type of the loaded signal parameter.
    using load_info = resource_load_info<Resource>;

    /// @brief Signal emmitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Indicates if this resource is loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return derived().is_loaded();
    }

    /// @brief Delay-initializes the resource.
    template <typename... Args>
    void init(resource_loader& loader, const Args&...) noexcept {
        _connect(loader);
    }

    /// @brief Returns a reference to the underlying resource.
    auto resource() noexcept -> Resource& {
        return *this;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(resource_loader& loader, P... params) -> work_done {
        if(!derived().is_loaded() && !is_loading()) {
            if(const auto request{
                 utils::request(loader, locator(), params...)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(execution_context& ctx, Cp... params) -> work_done {
        if(!derived().is_loaded() && !is_loading()) {
            if(const auto request{utils::request(ctx, locator(), params...)}) {
                _request_id = request.request_id();
                return true;
            }
        }
        return false;
    }

protected:
    /// @brief Constructor specifying the resource locator.
    loaded_resource_common(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    void _connect(resource_loader& loader) noexcept {
        _sig_key = connect<&loaded_resource_common::_handle_loaded>(
          this, loader.load_signal(resource_tid()));
    }

    void _disconnect(resource_loader& loader) noexcept {
        if(_sig_key) {
            loader.load_signal(resource_tid()).disconnect(_sig_key);
            _sig_key = {};
        }
    }

    template <typename R>
    auto _assign(R&& initial) noexcept -> bool {
        resource() = std::forward<R>(initial);
        return derived().is_loaded();
    }

private:
    void _handle_loaded(const base_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            if(derived().assign(info)) {
                loaded(load_info(info, derived()));
                base_loaded(*this);
                _request_id = 0;
            }
        }
    }

    signal_binding_key _sig_key{};
};
//------------------------------------------------------------------------------
export template <typename T>
class loaded_resource<std::vector<T>>
  : public loaded_resource_common<loaded_resource<std::vector<T>>> {

    using common = loaded_resource_common<loaded_resource<std::vector<T>>>;

public:
    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader)
      : common{std::move(locator)} {
        this->init(loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader()} {}

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader) {
        this->resource().clear();
        common::_disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader());
    }

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return !this->empty();
    }

    using common::load_if_needed;

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.values);
    }
};
//------------------------------------------------------------------------------
export using float_vector_resource = loaded_resource<std::vector<float>>;
export using vec3_vector_resource =
  loaded_resource<std::vector<math::vector<float, 3, true>>>;
//------------------------------------------------------------------------------
export template <typename T, typename P, span_size_t O>
class loaded_resource<math::bezier_curves<T, P, O>>
  : public loaded_resource_common<loaded_resource<math::bezier_curves<T, P, O>>> {

    using common =
      loaded_resource_common<loaded_resource<math::bezier_curves<T, P, O>>>;

public:
    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader)
      : common{std::move(locator)} {
        this->init(loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader()} {}

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader) {
        this->resource().clear();
        common::_disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader());
    }

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return !this->control_points().empty();
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.curve);
    }
};
export using smooth_vec3_curve =
  loaded_resource<math::bezier_curves<math::vector<float, 3, true>, float, 3>>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<gl_geometry_and_bindings>
  : std::type_identity<std::tuple<video_context&, span_size_t>> {};

export template <>
class loaded_resource<gl_geometry_and_bindings>
  : public loaded_resource_common<loaded_resource<gl_geometry_and_bindings>> {

    using common =
      loaded_resource_common<loaded_resource<gl_geometry_and_bindings>>;

public:
    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : common{std::move(locator)} {}

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader, video_context& video) {
        resource().clean_up(video);
        common::_disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader(), ctx.main_video());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader, video_context& video)
      : loaded_resource{std::move(locator)} {
        this->init(loader, video);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader(), ctx.main_video()} {}

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return this->is_initialized();
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(execution_context& ctx) -> work_done {
        return common::load_if_needed(ctx, 0);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref));
    }
};
export using gl_geometry_and_bindings_resource =
  loaded_resource<gl_geometry_and_bindings>;
//------------------------------------------------------------------------------
export template <>
struct resource_load_info<valtree::compound> {
    using Resource = valtree::compound;
    using base_load_info = typename resource_loader::load_info_t<Resource>;

    /// @brief The base info from the loader signal.
    const base_load_info& base;
    /// @brief The loaded program resource.
    loaded_resource<Resource>& resource;

    resource_load_info(
      const base_load_info& info,
      loaded_resource<Resource>& parent) noexcept
      : base{info}
      , resource{parent} {}

    /// @brief Loads the data from the value tree resource into an object.
    auto load(default_mapped_struct auto& object) const noexcept -> bool {
        return base.load(object);
    }

    /// @brief Loads the data from the value tree resource into an object.
    auto load(default_mapped_struct auto& object, const basic_string_path& path)
      const noexcept -> bool {
        return base.load(object, path);
    }
};

export template <>
class loaded_resource<valtree::compound>
  : public loaded_resource_common<loaded_resource<valtree::compound>> {

    using common = loaded_resource_common<loaded_resource<valtree::compound>>;

public:
    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : common{std::move(locator)} {}

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader) {
        common::_disconnect(loader);
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

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.tree);
    }
};
export using value_tree_resource = loaded_resource<valtree::compound>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::owned_shader_name>
  : std::type_identity<std::tuple<video_context&, oglplus::shader_type>> {};

export template <>
class loaded_resource<oglplus::owned_shader_name>
  : public loaded_resource_common<loaded_resource<oglplus::owned_shader_name>> {

    using common =
      loaded_resource_common<loaded_resource<oglplus::owned_shader_name>>;

public:
    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : common{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(resource_loader& loader, video_context&) {
        common::_connect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader, video_context& video) {
        video.gl_api().clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader(), ctx.main_video());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader, video_context& video)
      : loaded_resource{std::move(locator)} {
        init(loader, video);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader(), ctx.main_video()} {}

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return this->is_valid();
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref));
    }
};
export using gl_shader_resource = loaded_resource<oglplus::owned_shader_name>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::owned_program_name>
  : std::type_identity<std::tuple<video_context&>> {};

export template <>
struct resource_load_info<oglplus::owned_program_name> {
    using Resource = oglplus::owned_program_name;
    using base_load_info = typename resource_loader::load_info_t<Resource>;

    /// @brief The base info from the loader signal.
    const base_load_info& base;
    /// @brief The loaded program resource.
    loaded_resource<Resource>& resource;

    resource_load_info(
      const base_load_info& info,
      loaded_resource<Resource>& parent) noexcept
      : base{info}
      , resource{parent} {}

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
    auto set_uniform(const string_view var_name, const T& value) const noexcept {
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

export template <>
class loaded_resource<oglplus::owned_program_name>
  : public loaded_resource_common<loaded_resource<oglplus::owned_program_name>> {

    using common =
      loaded_resource_common<loaded_resource<oglplus::owned_program_name>>;

public:
    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : common{std::move(locator)} {}

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

    /// @brief Returns the location of a uniform with the specified name.
    auto get_uniform_location(
      const oglplus::gl_api& glapi,
      const string_view var_name) const noexcept {
        return glapi.get_uniform_location(*this, var_name);
    }

    /// @brief Returns the location of a uniform with the specified name.
    auto get_uniform_location(video_context& video, const string_view var_name)
      const noexcept {
        return get_uniform_location(video.gl_api(), var_name);
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
    void clean_up(resource_loader& loader, video_context& video) {
        video.gl_api().clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        return clean_up(ctx.loader(), ctx.main_video());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader, video_context& video)
      : loaded_resource{std::move(locator)} {
        init(loader, video);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader(), ctx.main_video()} {}

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return this->is_valid();
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        if(this->_assign(std::move(info.ref))) {
            _inputs = info.input_bindings;
            return true;
        }
        return false;
    }

private:
    oglplus::program_input_bindings _inputs;
};
export using gl_program_resource = loaded_resource<oglplus::owned_program_name>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::owned_texture_name>
  : std::type_identity<
      std::tuple<video_context&, oglplus::texture_target, oglplus::texture_unit>> {
};

export template <>
struct resource_load_info<oglplus::owned_texture_name> {
    using Resource = oglplus::owned_texture_name;
    using base_load_info = typename resource_loader::load_info_t<Resource>;

    /// @brief The base info from the loader signal.
    const base_load_info& base;
    /// @brief The loaded texture resource.
    loaded_resource<Resource>& resource;

    resource_load_info(
      const base_load_info& info,
      loaded_resource<Resource>& parent) noexcept
      : base{info}
      , resource{parent} {}

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

export template <>
class loaded_resource<oglplus::owned_texture_name>
  : public loaded_resource_common<loaded_resource<oglplus::owned_texture_name>> {

    using common =
      loaded_resource_common<loaded_resource<oglplus::owned_texture_name>>;

public:
    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : common{std::move(locator)} {}

    /// @brief Delay-initializes the resource.
    void init(resource_loader& loader, video_context&) {
        common::_connect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader, video_context& video) {
        video.gl_api().clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader(), ctx.main_video());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader, video_context& video)
      : loaded_resource{std::move(locator)} {
        init(loader, video);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader(), ctx.main_video()} {}

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return this->is_valid();
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref));
    }
};
export using gl_texture_resource = loaded_resource<oglplus::owned_texture_name>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::owned_buffer_name>
  : std::type_identity<std::tuple<video_context&>> {};

export template <>
class loaded_resource<oglplus::owned_buffer_name>
  : public loaded_resource_common<loaded_resource<oglplus::owned_buffer_name>> {

    using common =
      loaded_resource_common<loaded_resource<oglplus::owned_buffer_name>>;

public:
    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : common{std::move(locator)} {}

    /// @brief Clean's up this resource.
    void clean_up(resource_loader& loader, video_context& video) {
        video.gl_api().clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Clean's up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.loader(), ctx.main_video());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, resource_loader& loader, video_context& video)
      : loaded_resource{std::move(locator)} {
        init(loader, video);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource{std::move(locator), ctx.loader(), ctx.main_video()} {}

    /// @brief Indicates if this resource is loaded.
    auto is_loaded() const noexcept -> bool {
        return this->is_valid();
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref));
    }
};
export using gl_buffer_resource = loaded_resource<oglplus::owned_buffer_name>;
//------------------------------------------------------------------------------
} // namespace eagine::app
