/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:loaded_resource;

import std;
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
import eagine.msgbus;
import :context;
import :geometry;
import :old_resource_loader;

namespace eagine::app {
//------------------------------------------------------------------------------
/// @brief Common base for loaded resource template.
/// @see old_resource_loader
export class loaded_resource_base {
public:
    loaded_resource_base(url locator) noexcept
      : _locator_str{locator.release_string()} {}

    loaded_resource_base(const resource_request_params& params) noexcept
      : _locator_str{params.locator.get_string()}
      , _max_time{params.max_time}
      , _priority{params.priority} {}

    loaded_resource_base(loaded_resource_base&&) = delete;
    loaded_resource_base(const loaded_resource_base&) = delete;
    auto operator=(loaded_resource_base&&) = delete;
    auto operator=(const loaded_resource_base&) = delete;
    ~loaded_resource_base() noexcept = default;

    /// @brief Signal emitted when the resource load status changes.
    signal<void(const loaded_resource_base&) noexcept> load_event;

    /// @brief Returns this resource's URL.
    /// @see parameters
    [[nodiscard]] auto locator() const noexcept -> url {
        return {_locator_str};
    }

    /// @brief Returns the resource request parameters.
    /// @see locator
    [[nodiscard]] auto request_parameters() const noexcept
      -> resource_request_params {
        return {
          .locator = locator(), .max_time = _max_time, .priority = _priority};
    }

    /// @brief Indicates if this resource is currently loading.
    /// @see is_loaded
    /// @see is_cancelled
    /// @see has_failed
    [[nodiscard]] auto is_loading() const noexcept -> bool {
        return (_request_id != 0) and
               (_status == resource_load_status::loading);
    }

    /// @brief Indicates if this resource is loaded.
    /// @see is_loading
    /// @see is_cancelled
    /// @see has_failed
    [[nodiscard]] auto is_loaded() const noexcept -> bool {
        return _status == resource_load_status::loaded;
    }

    /// @brief Indicates if this resource is cancelled.
    /// @see is_loading
    /// @see has_failed
    [[nodiscard]] auto is_cancelled() const noexcept -> bool {
        return _status == resource_load_status::cancelled;
    }

    /// @brief Indicates if the loading of this resource has failed
    /// @see is_loading
    /// @see is_loaded
    /// @see is_cancelled
    [[nodiscard]] auto has_failed() const noexcept -> bool {
        return (_status == resource_load_status::cancelled) or
               (_status == resource_load_status::error);
    }

    /// @brief Indicates if this resource is the same as that resource.
    [[nodiscard]] auto is(const loaded_resource_base& that) const noexcept
      -> bool {
        return this->_locator_str == that._locator_str;
    }

    /// @brief Compares resources for equality.
    [[nodiscard]] auto operator==(
      const loaded_resource_base& that) const noexcept -> bool {
        return this->is(that);
    }

    /// @brief Indicates if this resource is the same as that resource and it's loaded.
    [[nodiscard]] auto is_loaded(const loaded_resource_base& that) const noexcept
      -> bool {
        return is_loaded() and this->is(that);
    }

    /// @brief Indicates if this resource is one in the specified collection.
    template <std::derived_from<loaded_resource_base>... R>
    [[nodiscard]] auto is_one_of(const R&... those) const noexcept -> bool {
        return (... or is(those));
    }

protected:
    std::string _locator_str;
    identifier_t _request_id{0};
    std::optional<std::chrono::seconds> _max_time{};
    std::optional<msgbus::message_priority> _priority{};
    resource_load_status _status{resource_load_status::loading};
};
//------------------------------------------------------------------------------
export template <typename Resource>
class loaded_resource;
//------------------------------------------------------------------------------
export template <typename Resource>
struct get_resource_load_params : std::type_identity<std::tuple<>> {};

export template <typename Resource>
struct get_resource_load_params<loaded_resource<Resource>>
  : get_resource_load_params<Resource> {};

export template <typename Resource>
using resource_load_params = typename get_resource_load_params<Resource>::type;
//------------------------------------------------------------------------------
export template <typename Params>
struct get_resource_load_context_params : std::type_identity<Params> {};

export template <typename Params>
using resource_load_context_params =
  typename get_resource_load_context_params<Params>::type;
//------------------------------------------------------------------------------
export template <typename Resource>
class loaded_resource;

export template <typename Resource>
struct resource_load_info {
    using base_load_info = typename old_resource_loader::load_info_t<Resource>;

    /// @brief The base info from the loader signal.
    const base_load_info& base;
    /// @brief The loaded resource.
    const loaded_resource<Resource>& resource;

    resource_load_info(
      const base_load_info& info,
      loaded_resource<Resource>& parent) noexcept
      : base{info}
      , resource{parent} {}
};
//------------------------------------------------------------------------------
export template <typename Resource>
class loaded_resource
  : public Resource
  , public loaded_resource_base {
    static_assert(mapped_struct<Resource>);

    using Derived = loaded_resource<Resource>;

    auto derived() noexcept -> Derived& {
        return *static_cast<Derived*>(this);
    }

    auto derived() const noexcept -> const Derived& {
        return *static_cast<const Derived*>(this);
    }

public:
    /// @brief Type of the load_event signal parameter.
    using base_load_info = typename old_resource_loader::load_info_t<Resource>;

    /// @brief Type of the loaded signal parameter.
    using load_info = resource_load_info<Resource>;

    /// @brief Signal emitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Indicates if this resource is loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Returns a reference to the underlying resource.
    [[nodiscard]] auto resource() noexcept -> Resource& {
        return *this;
    }

    /// @brief Returns a const reference to the underlying resource.
    [[nodiscard]] auto resource() const noexcept -> const Resource& {
        return *this;
    }

    /// @brief Constructor specifying the resource locator.
    loaded_resource(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Constructor specifying the resource resource parameters.
    loaded_resource(const resource_request_params& params) noexcept
      : loaded_resource_base{params} {}

    /// @brief Delay-initializes the resource.
    void init(old_resource_loader& loader) noexcept {
        _connect(loader);
    }

    /// @brief Delay-initializes the resource.
    void init(loaded_resource_context& ctx) noexcept {
        _connect(ctx.old_loader());
    }

    /// @brief Delay-initializes the resource.
    void init(execution_context& ctx) noexcept {
        _connect(ctx.old_loader());
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, old_resource_loader& loader)
      : loaded_resource_base{std::move(locator)} {
        init(loader);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, loaded_resource_context& ctx)
      : loaded_resource_base{std::move(locator)} {
        init(ctx);
    }

    /// @brief Constructor specifying the locator and initializing the resource.
    loaded_resource(url locator, execution_context& ctx)
      : loaded_resource_base{std::move(locator)} {
        init(ctx);
    }

    /// @brief Constructor specifying the parameters and initializing the resource.
    loaded_resource(
      const resource_request_params& params,
      old_resource_loader& loader)
      : loaded_resource_base{params} {
        init(loader);
    }

    /// @brief Constructor specifying the parameters and initializing the resource.
    loaded_resource(
      const resource_request_params& params,
      loaded_resource_context& ctx)
      : loaded_resource_base{params} {
        init(ctx);
    }

    /// @brief Constructor specifying the parameters and initializing the resource.
    loaded_resource(
      const resource_request_params& params,
      execution_context& ctx)
      : loaded_resource_base{params} {
        init(ctx);
    }

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader) {
        _disconnect(loader);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        derived().clean_up(ctx.old_loader());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        derived().clean_up(ctx.old_loader());
    }

    /// @brief Indicates if the load info originated from this loaded resource.
    [[nodiscard]] auto originated(const base_load_info& info) const noexcept
      -> bool {
        return info.request_id == _request_id;
    }

    /// @brief Indicates if the load info originated from this loaded resource.
    [[nodiscard]] auto originated(const load_info& info) const noexcept
      -> bool {
        return originated(info.base);
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(loaded_resource_context& ctx) -> work_done {
        if(not is_loaded() and not is_loading()) {
            if(const auto request{ctx.old_loader().request(
                 std::type_identity<Resource>{}, request_parameters(), ctx)}) {
                _request_id = request.request_id();
            }
        }
        return is_loading();
    }

    auto load_if_needed(loaded_resource_context& ctx, std::tuple<>)
      -> work_done {
        return load_if_needed(ctx);
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(execution_context& ctx) -> work_done {
        return load_if_needed(ctx.resource_context());
    }

    auto load_if_needed(execution_context& ctx, std::tuple<>) -> work_done {
        return load_if_needed(ctx.old_loader());
    }

private:
    void _connect(old_resource_loader& loader) noexcept {
        _sig_key = connect<&loaded_resource::_handle_loaded>(
          this, loader.mapped_struct_loaded);
    }

    void _disconnect(old_resource_loader& loader) noexcept {
        if(_sig_key) {
            loader.mapped_struct_loaded.disconnect(_sig_key);
            _sig_key = {};
        }
    }

    void _handle_loaded(const base_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            if(info.move_to(resource())) {
                _status = resource_load_status::loaded;
                loaded(load_info(info, *this));
            } else {
                _status = resource_load_status::error;
            }
            load_event(*this);
            _request_id = 0;
        }
    }

    signal_binding_key _sig_key{};
};
//------------------------------------------------------------------------------
export template <
  typename Derived,
  typename LoadParams = resource_load_params<Derived>,
  typename LoadCtxParams = resource_load_context_params<LoadParams>>
class loaded_resource_common;
//------------------------------------------------------------------------------
export template <typename Resource, typename... LoadP, typename... CtxLoadP>
class loaded_resource_common<
  loaded_resource<Resource>,
  std::tuple<LoadP...>,
  std::tuple<CtxLoadP...>>
  : public Resource
  , public loaded_resource_base {
    using base = loaded_resource_base;

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

    /// @brief Type of the load_event signal parameter.
    using base_load_info = typename old_resource_loader::load_info_t<Resource>;

    /// @brief Type of the loaded signal parameter.
    using load_info = resource_load_info<Resource>;

    /// @brief Signal emitted when the resource is successfully loaded.
    signal<void(const load_info&) noexcept> loaded;

    /// @brief Constructor specifying the resource locator.
    loaded_resource_common(url locator) noexcept
      : loaded_resource_base{std::move(locator)} {}

    /// @brief Constructor specifying the resource locator.
    loaded_resource_common(url locator, old_resource_loader& loader) noexcept
      : loaded_resource_base{std::move(locator)} {
        derived().init(loader);
    }

    /// @brief Constructor specifying the resource locator.
    loaded_resource_common(url locator, loaded_resource_context& ctx) noexcept
      : loaded_resource_base{std::move(locator)} {
        derived().init(ctx);
    }

    /// @brief Constructor specifying the resource locator.
    loaded_resource_common(url locator, execution_context& ctx) noexcept
      : loaded_resource_base{std::move(locator)} {
        derived().init(ctx);
    }

    /// @brief Constructor specifying the resource request parameters.
    loaded_resource_common(
      const resource_request_params& params,
      old_resource_loader& loader) noexcept
      : loaded_resource_base{params} {
        derived().init(loader);
    }

    /// @brief Constructor specifying the resource request parameters.
    loaded_resource_common(
      const resource_request_params& params,
      loaded_resource_context& ctx) noexcept
      : loaded_resource_base{params} {
        derived().init(ctx);
    }

    /// @brief Constructor specifying the resource request parameters.
    loaded_resource_common(
      const resource_request_params& params,
      execution_context& ctx) noexcept
      : loaded_resource_base{params} {
        derived().init(ctx);
    }

    /// @brief Delay-initializes the resource.
    void init(old_resource_loader& loader) noexcept {
        _connect(loader);
    }

    /// @brief Delay-initializes the resource.
    void init(loaded_resource_context& ctx) noexcept {
        _connect(ctx.old_loader());
    }

    /// @brief Delay-initializes the resource.
    void init(execution_context& ctx) noexcept {
        _connect(ctx.old_loader());
    }

    /// @brief Indicates if this resource is loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Returns a reference to the underlying resource.
    [[nodiscard]] auto resource() noexcept -> Resource& {
        return *this;
    }

    /// @brief Returns a const reference to the underlying resource.
    [[nodiscard]] auto resource() const noexcept -> const Resource& {
        return *this;
    }

    /// @brief Indicates if the load info originated from this loaded resource.
    [[nodiscard]] auto originated(const base_load_info& info) const noexcept
      -> bool {
        return info.request_id == _request_id;
    }

    /// @brief Indicates if the load info originated from this loaded resource.
    [[nodiscard]] auto originated(const load_info& info) const noexcept
      -> bool {
        return originated(info.base);
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(loaded_resource_context& ctx, LoadP... params)
      -> work_done {
        if(not is_loaded() and not is_loading()) {
            if(const auto request{ctx.old_loader().request(
                 std::type_identity<Resource>{},
                 request_parameters(),
                 ctx,
                 params...)}) {
                _request_id = request.request_id();
            }
        }
        return is_loading();
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(
      loaded_resource_context& ctx,
      const std::tuple<LoadP...>& params) -> work_done {
        return load_if_needed(ctx, std::get<LoadP>(params)...);
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(execution_context& ctx, CtxLoadP... params)
      -> work_done {
        return load_if_needed(ctx.resource_context(), std::move(params)...);
    }

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(
      execution_context& ctx,
      const std::tuple<LoadP...>& params) -> work_done {
        return load_if_needed(ctx, std::get<LoadP>(params)...);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        derived().clean_up(ctx.old_loader());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        derived().clean_up(ctx.old_loader());
    }

protected:
    void _connect(old_resource_loader& loader) noexcept {
        _sig_key = connect<&loaded_resource_common::_handle_loaded>(
          this, loader.load_signal(resource_tid()));
    }

    void _disconnect(old_resource_loader& loader) noexcept {
        if(_sig_key) {
            loader.load_signal(resource_tid()).disconnect(_sig_key);
            _sig_key = {};
        }
    }

    template <typename R>
    auto _assign(R&& initial) noexcept -> bool {
        resource() = std::forward<R>(initial);
        return true;
    }

private:
    void _handle_loaded(const base_load_info& info) noexcept {
        if(info.request_id == _request_id) {
            if(derived().assign(info)) {
                _status = resource_load_status::loaded;
                loaded(load_info(info, derived()));
            } else {
                _status = resource_load_status::error;
            }
            load_event(*this);
            _request_id = 0;
        }
    }

    signal_binding_key _sig_key{};
};
//------------------------------------------------------------------------------
export template <>
class loaded_resource<std::string>
  : public loaded_resource_common<loaded_resource<std::string>> {
    using common = loaded_resource_common<loaded_resource<std::string>>;

public:
    using common::clean_up;
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader) {
        this->resource().clear();
        common::_disconnect(loader);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.text);
    }
};
export using plain_text_resource = loaded_resource<std::string>;
//------------------------------------------------------------------------------
export template <typename T>
class loaded_resource<std::vector<T>>
  : public loaded_resource_common<loaded_resource<std::vector<T>>> {

    using common = loaded_resource_common<loaded_resource<std::vector<T>>>;

public:
    using common::clean_up;
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader) {
        this->resource().clear();
        common::_disconnect(loader);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.values);
    }
};
//------------------------------------------------------------------------------
export template <>
class loaded_resource<std::vector<std::string>>
  : public loaded_resource_common<loaded_resource<std::vector<std::string>>> {

    using common =
      loaded_resource_common<loaded_resource<std::vector<std::string>>>;

public:
    using common::clean_up;
    using common::common;

    /// @brief Removes the blank lines
    auto erase_blank_lines() {
        std::erase_if(this->resource(), [](auto& entry) {
            return entry.empty() or
                   std::all_of(entry.begin(), entry.end(), [](auto chr) {
                       return std::isspace(chr);
                   });
        });
    }

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader) {
        this->resource().clear();
        common::_disconnect(loader);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.strings));
    }
};
//------------------------------------------------------------------------------
export using string_list_resource = loaded_resource<std::vector<std::string>>;
export using url_list_resource = loaded_resource<std::vector<url>>;
export using float_vector_resource = loaded_resource<std::vector<float>>;
export using vec3_vector_resource =
  loaded_resource<std::vector<math::vector<float, 3>>>;
export using mat4_vector_resource =
  loaded_resource<std::vector<math::matrix<float, 4, 4, true>>>;
//------------------------------------------------------------------------------
export template <typename T, typename P, span_size_t O>
class loaded_resource<math::bezier_curves<T, P, O>>
  : public loaded_resource_common<loaded_resource<math::bezier_curves<T, P, O>>> {

    using common =
      loaded_resource_common<loaded_resource<math::bezier_curves<T, P, O>>>;

public:
    using common::clean_up;
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader) {
        this->clear();
        common::_disconnect(loader);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.curve);
    }
};
export using smooth_vec3_curve_resource =
  loaded_resource<math::bezier_curves<math::vector<float, 3>, float, 3>>;
//------------------------------------------------------------------------------
export template <>
class loaded_resource<shared_holder<shapes::generator>>
  : public loaded_resource_common<
      loaded_resource<shared_holder<shapes::generator>>> {

    using common =
      loaded_resource_common<loaded_resource<shared_holder<shapes::generator>>>;

public:
    using common::clean_up;
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader) {
        resource().reset();
        common::_disconnect(loader);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.generator) and info.generator;
    }
};
export using shape_generator_resource =
  loaded_resource<shared_holder<shapes::generator>>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<gl_geometry_and_bindings>
  : std::type_identity<std::tuple<oglplus::vertex_attrib_bindings, span_size_t>> {
};

export template <>
class loaded_resource<gl_geometry_and_bindings>
  : public loaded_resource_common<loaded_resource<gl_geometry_and_bindings>> {

    using common =
      loaded_resource_common<loaded_resource<gl_geometry_and_bindings>>;

public:
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader, const oglplus::gl_api& glapi) {
        resource().clean_up(glapi);
        common::_disconnect(loader);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        clean_up(ctx.old_loader(), ctx.gl_api());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.resource_context());
    }

    using common::load_if_needed;

    /// @brief Updates the resource, possibly doing resource load request.
    auto load_if_needed(execution_context& ctx) -> work_done {
        return common::load_if_needed(
          ctx, oglplus::vertex_attrib_bindings{}, 0);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref)) and this->is_initialized();
    }
};
export using gl_geometry_and_bindings_resource =
  loaded_resource<gl_geometry_and_bindings>;
//------------------------------------------------------------------------------
export template <>
struct resource_load_info<valtree::compound> {
    using Resource = valtree::compound;
    using base_load_info = typename old_resource_loader::load_info_t<Resource>;

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
    auto load(mapped_struct auto& object) const noexcept -> bool {
        return base.load(object);
    }

    /// @brief Loads the data from the value tree resource into an object.
    auto load(mapped_struct auto& object, const basic_string_path& path)
      const noexcept -> bool {
        return base.load(object, path);
    }
};

export template <>
class loaded_resource<valtree::compound>
  : public loaded_resource_common<loaded_resource<valtree::compound>> {

    using common = loaded_resource_common<loaded_resource<valtree::compound>>;

public:
    using common::clean_up;
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader) {
        common::_disconnect(loader);
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(info.tree) and
               static_cast<const valtree::compound&>(*this);
    }
};
export using value_tree_resource = loaded_resource<valtree::compound>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::shader_include>
  : std::type_identity<std::tuple<std::string>> {};

export template <>
class loaded_resource<oglplus::shader_include>
  : public loaded_resource_common<loaded_resource<oglplus::shader_include>> {

    using common =
      loaded_resource_common<loaded_resource<oglplus::shader_include>>;

public:
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader, const oglplus::gl_api& glapi) {
        glapi.clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        clean_up(ctx.old_loader(), ctx.gl_api());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.resource_context());
    }
};

export using gl_shader_include_resource =
  loaded_resource<oglplus::shader_include>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::owned_shader_name>
  : std::type_identity<std::tuple<oglplus::shader_type>> {};

export template <>
class loaded_resource<oglplus::owned_shader_name>
  : public loaded_resource_common<loaded_resource<oglplus::owned_shader_name>> {

    using common =
      loaded_resource_common<loaded_resource<oglplus::owned_shader_name>>;

public:
    using common::common;

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader, const oglplus::gl_api& glapi) {
        glapi.clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        clean_up(ctx.old_loader(), ctx.gl_api());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.resource_context());
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref)) and this->has_value();
    }
};
export using gl_shader_resource = loaded_resource<oglplus::owned_shader_name>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::owned_program_name>
  : std::type_identity<std::tuple<>> {};

export template <>
struct resource_load_info<oglplus::owned_program_name> {
    using Resource = oglplus::owned_program_name;
    using base_load_info = typename old_resource_loader::load_info_t<Resource>;

    /// @brief The base info from the loader signal.
    const base_load_info& base;
    /// @brief The loaded program resource.
    loaded_resource<Resource>& resource;

    resource_load_info(
      const base_load_info& info,
      loaded_resource<Resource>& parent) noexcept
      : base{info}
      , resource{parent} {}

    /// @brief Applies the vertex attribute bindings to the loaded program.
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
    [[nodiscard]] auto get_uniform_location(
      const string_view var_name) const noexcept {
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
    [[nodiscard]] auto get_shader_storage_block_index(
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
    using common::common;

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
    [[nodiscard]] auto get_uniform_location(
      const oglplus::gl_api& glapi,
      const string_view var_name) const noexcept {
        return glapi.get_uniform_location(*this, var_name);
    }

    /// @brief Returns the location of a uniform with the specified name.
    [[nodiscard]] auto get_uniform_location(
      video_context& video,
      const string_view var_name) const noexcept {
        return get_uniform_location(video.gl_api(), var_name);
    }

    /// @brief Sets the value of a uniform variable with the specified name.
    template <typename T>
    auto set_uniform(
      const oglplus::gl_api& glapi,
      const string_view var_name,
      T&& value) -> loaded_resource& {
        oglplus::uniform_location loc;
        glapi.get_uniform_location(*this, var_name) >> loc;
        glapi.set_uniform(*this, loc, std::forward<T>(value));
        return *this;
    }

    /// @brief Sets the value of a uniform variable with the specified name.
    template <typename T>
    auto set_uniform(video_context& video, const string_view var_name, T&& value)
      -> loaded_resource& {
        return set_uniform(video.gl_api(), var_name, std::forward<T>(value));
    }

    /// @brief Sets the value of a uniform variable with the specified location.
    template <typename T>
    auto set(
      const oglplus::gl_api& glapi,
      oglplus::uniform_location loc,
      T&& value) -> loaded_resource& {
        glapi.set_uniform(*this, loc, std::forward<T>(value));
        return *this;
    }

    /// @brief Sets the value of a uniform variable with the specified location.
    template <typename T>
    auto set(video_context& video, oglplus::uniform_location loc, T&& value)
      -> loaded_resource& {
        return set(video.gl_api(), loc, std::forward<T>(value));
    }

    /// @brief Binds the location of a input attribute variable.
    auto bind(
      const oglplus::gl_api& glapi,
      const oglplus::vertex_attrib_location loc,
      const string_view& var_name) -> loaded_resource& {
        glapi.BindAttribLocation(
          this->value(), loc.location(), c_str(var_name));
        return *this;
    }

    /// @brief Binds the location of a input attribute variable.
    auto bind(
      video_context& video,
      const oglplus::vertex_attrib_location loc,
      const string_view& var_name) -> loaded_resource& {
        return bind(video.gl_api(), loc, var_name);
    }

    /// @brief Applies the vertex attribute bindings to the loaded program.
    auto apply_input_bindings(
      video_context& video,
      const oglplus::vertex_attrib_bindings& attrib_bindings) const noexcept
      -> bool {
        return _inputs.apply(video.gl_api(), *this, attrib_bindings);
    }

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader, const oglplus::gl_api& glapi) {
        glapi.clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        clean_up(ctx.old_loader(), ctx.gl_api());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.resource_context());
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        if(this->_assign(std::move(info.ref))) {
            _inputs = info.input_bindings;
            return this->has_value();
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
      std::tuple<oglplus::texture_target, oglplus::texture_unit>> {};

export template <>
struct resource_load_info<oglplus::owned_texture_name> {
    using Resource = oglplus::owned_texture_name;
    using base_load_info = typename old_resource_loader::load_info_t<Resource>;

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
    using common::common;

    /// @brief Request the update of the texture image data.
    auto request_update(
      url new_locator,
      old_resource_loader& loader,
      loaded_resource_context& ctx,
      oglplus::texture_target tgt,
      oglplus::texture_unit tu) -> auto& {
        _locator_str = new_locator.release_string();
        if(const auto request{loader.request_gl_texture_update(
             request_parameters(), ctx.gl_context(), tgt, tu, *this)}) {
            _request_id = request.request_id();
            _status = resource_load_status::loading;
        }
        return *this;
    }

    auto request_update(
      url new_locator,
      old_resource_loader& loader,
      execution_context& ctx,
      oglplus::texture_target tgt,
      oglplus::texture_unit tu) -> auto& {
        return request_update(
          std::move(new_locator), loader, ctx.resource_context(), tgt, tu);
    }

    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader, const oglplus::gl_api& glapi) {
        glapi.clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        clean_up(ctx.old_loader(), ctx.gl_api());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.resource_context());
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref)) and this->has_value();
    }
};
export using gl_texture_resource = loaded_resource<oglplus::owned_texture_name>;
//------------------------------------------------------------------------------
template <>
struct get_resource_load_params<oglplus::owned_buffer_name>
  : std::type_identity<std::tuple<oglplus::buffer_target>> {};

export template <>
class loaded_resource<oglplus::owned_buffer_name>
  : public loaded_resource_common<loaded_resource<oglplus::owned_buffer_name>> {

    using common =
      loaded_resource_common<loaded_resource<oglplus::owned_buffer_name>>;

public:
    /// @brief Cleans up this resource.
    void clean_up(old_resource_loader& loader, const oglplus::gl_api& glapi) {
        glapi.clean_up(std::move(resource()));
        common::_disconnect(loader);
    }

    /// @brief Cleans up this resource.
    void clean_up(loaded_resource_context& ctx) {
        clean_up(ctx.old_loader(), ctx.gl_api());
    }

    /// @brief Cleans up this resource.
    void clean_up(execution_context& ctx) {
        clean_up(ctx.resource_context());
    }

    auto assign(const typename common::base_load_info& info) noexcept -> bool {
        return this->_assign(std::move(info.ref)) and this->has_value();
    }
};
export using gl_buffer_resource = loaded_resource<oglplus::owned_buffer_name>;
//------------------------------------------------------------------------------
} // namespace eagine::app
