/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:resource_manager;

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.identifier;
import eagine.core.value_tree;
import eagine.oglplus;
import :loaded_resource;
import :geometry;
import :context;

namespace eagine::app {
export template <typename Resource>
class managed_resource;

export template <typename... Resources>
class basic_resource_manager;
//------------------------------------------------------------------------------
class managed_resource_utils {
private:
    template <typename X>
    struct _adjust_res : std::type_identity<loaded_resource<X>> {};

    template <typename R>
    struct _adjust_res<loaded_resource<R>>
      : std::type_identity<loaded_resource<R>> {};

    template <typename R>
    struct _adjust_res<managed_resource<R>>
      : std::type_identity<loaded_resource<R>> {};

    template <typename X>
    using adjust_res_t = typename _adjust_res<X>::type;

    template <typename Resource>
    class resource_and_params {
        loaded_resource_context _context; // TODO: optimize as flyweight
        Resource _resource;
        resource_load_params<Resource> _params;

    public:
        template <typename... Args>
        resource_and_params(
          loaded_resource_context& ctx,
          url locator,
          Args&&... args) noexcept
          : _context{ctx}
          , _resource{std::move(locator), ctx}
          , _params{std::forward<Args>(args)...} {}

        auto resource() noexcept -> Resource& {
            return _resource;
        }

        auto load_if_needed() noexcept {
            return _resource.load_if_needed(_context, _params);
        }

        auto clean_up() noexcept {
            return _resource.clean_up(_context);
        }
    };

public:
    template <typename Resource>
    using managed_resource_context =
      resource_and_params<adjust_res_t<Resource>>;
};
//------------------------------------------------------------------------------
/// @brief Template for resources managed by basic_resource_manager.
/// @see resource_loader
export template <typename Resource>
class managed_resource {
    using _context_t =
      managed_resource_utils::managed_resource_context<Resource>;

public:
    managed_resource(shared_holder<_context_t> ref) noexcept
      : _ref{std::move(ref)} {}

    template <typename... Resources, typename... Args>
    managed_resource(
      basic_resource_manager<Resources...>& manager,
      loaded_resource_context& context,
      identifier_t resource_id,
      url locator,
      Args&&... args)
      : managed_resource{manager.do_add(
          std::type_identity<Resource>{},
          resource_id,
          std::move(locator),
          context,
          std::forward<Args>(args)...)} {}

    template <typename... Resources, typename... Args>
    managed_resource(
      basic_resource_manager<Resources...>& manager,
      execution_context& ec,
      identifier_t resource_id,
      url locator,
      Args&&... args)
      : managed_resource{
          manager,
          ec.resource_context(),
          resource_id,
          std::move(locator),
          std::forward<Args>(args)...} {}

    /// @brief Add or lookup a new managed resource in the specified manager.
    template <typename... Resources, typename... Args>
    managed_resource(
      basic_resource_manager<Resources...>& manager,
      loaded_resource_context& context,
      url locator,
      Args&&... args)
      : managed_resource{
          manager,
          context,
          locator.hash_id(),
          locator,
          std::forward<Args>(args)...} {}

    /// @brief Add or lookup a new managed resource in the specified manager.
    template <typename... Resources, typename... Args>
    managed_resource(
      basic_resource_manager<Resources...>& manager,
      execution_context& ec,
      url locator,
      Args&&... args)
      : managed_resource{
          manager,
          ec.resource_context(),
          std::move(locator),
          std::forward<Args>(args)...} {}

    /// @brier Returns a reference to the underlying loaded_resource.
    auto resource() const noexcept -> loaded_resource<Resource>& {
        return _ref->resource();
    }

    /// @brief Indicates if this resource is already loaded.
    auto is_loaded() const noexcept -> bool {
        return resource().is_loaded();
    }

    /// @brief Indicates if this resource is already loaded.
    /// @see is_loaded
    auto has_value() const noexcept -> bool {
        return is_loaded();
    }

    /// @brief Indicates if this resource is already loaded.
    /// @see is_loaded
    explicit operator bool() const noexcept {
        return is_loaded();
    }

    /// @brief Conversion to reference to the underlying loaded_resource.
    /// @see resource
    operator loaded_resource<Resource>&() const noexcept {
        return resource();
    }

    /// @brief Returns a pointer to the underlying loaded_resource.
    /// @see resource
    auto operator->() const noexcept -> loaded_resource<Resource>* {
        return &(resource());
    }

    /// @brief Connects the specified callable to the load event of the loaded_resource.
    auto connect(
      callable_ref<void(const loaded_resource_base&) noexcept> handler) noexcept
      -> managed_resource& {
        resource().load_event.connect(std::move(handler));
        return *this;
    }

    /// @brief Connects the specified callable to the load signal of the loaded_resource.
    auto connect(
      callable_ref<
        void(const typename loaded_resource<Resource>::load_info&) noexcept>
        handler) noexcept -> managed_resource& {
        resource().loaded.connect(std::move(handler));
        return *this;
    }

    /// @brief Connects the specified member function to a signal of the loaded_resource.
    template <auto MemFnPtr, typename C>
    auto connect_to(C* obj) noexcept -> managed_resource& {
        return connect(make_callable_ref<MemFnPtr>(obj));
    }

private:
    shared_holder<_context_t> _ref;
};
//------------------------------------------------------------------------------
/// @brief Class managing a collection of managed_resources.
/// @see resource_loader
export template <typename... Resources>
class basic_resource_manager {

    template <typename X>
    using resource_storage = std::map<
      identifier_t,
      shared_holder<managed_resource_utils::managed_resource_context<X>>>;

    static constexpr auto _is() noexcept {
        return std::make_index_sequence<sizeof...(Resources)>();
    }

public:
    template <typename Resource, typename... Args>
    [[nodiscard]] auto do_add(
      std::type_identity<Resource> rtid,
      identifier_t res_id,
      url locator,
      loaded_resource_context& context,
      Args&&... args)
      -> shared_holder<
        managed_resource_utils::managed_resource_context<Resource>> {
        // try find in already loaded resource storage
        auto& loaded{_get_loaded(rtid)};
        const auto pos{loaded.find(res_id)};
        if(pos != loaded.end()) {
            return std::get<1>(*pos);
        }

        // find or emplace to pending resource storage
        auto& pending{_get_pending(rtid)};
        auto& info{pending[res_id]};
        if(not info.has_value()) {
            info = std::make_shared<
              managed_resource_utils::managed_resource_context<Resource>>(
              context, std::move(locator), std::forward<Args>(args)...);
        }
        return info;
    }

    /// @brief Indicates if all currently managed resources are loaded.
    [[nodiscard]] auto are_loaded() noexcept -> bool {
        return _are_loaded(_resources, _is());
    }

    /// @brief Load all pending resources.
    auto load() noexcept -> span_size_t {
        return _load(_resources, _is());
    }

    /// @brief Updates the internal state of this manager, loads resources if needed.
    auto update() noexcept -> basic_resource_manager& {
        if(not are_loaded()) {
            load();
        }
        return *this;
    }

    /// @brief Clean-up and remove all currently managed resources.
    auto clean_up() noexcept -> basic_resource_manager& {
        _clean_up(_resources, _is());
        return *this;
    }

private:
    template <typename Resource>
    auto _get_both(std::type_identity<Resource>) noexcept
      -> std::array<resource_storage<Resource>, 2Z>& {
        return std::get<std::array<resource_storage<Resource>, 2Z>>(_resources);
    }

    template <typename Resource>
    auto _get_pending(std::type_identity<Resource> tid) noexcept
      -> resource_storage<Resource>& {
        return _get_both(tid)[0];
    }

    template <typename Resource>
    auto _get_loaded(std::type_identity<Resource> tid) noexcept
      -> resource_storage<Resource>& {
        return _get_both(tid)[1];
    }

    template <typename V>
    auto _are_loaded(V& res_stores) const noexcept -> span_size_t {
        for(const auto& res_entry : res_stores[0]) {
            if(auto& res_info{std::get<1>(res_entry)}) {
                if(not res_info->resource().is_loaded()) {
                    return false;
                }
            }
        }
        return true;
    }

    template <typename Tup, std::size_t... I>
    auto _are_loaded(Tup& t, std::index_sequence<I...>) const noexcept -> bool {
        return (... and _are_loaded(std::get<I>(t)));
    }

    template <typename V>
    auto _do_load(V& res_stores) noexcept -> span_size_t {
        span_size_t result{0};
        auto& pending{res_stores[0]};
        auto& loaded{res_stores[1]};
        for(auto pos{pending.begin()}; pos != pending.end();) {
            auto& res_info{std::get<1>(*pos)};
            if(res_info.has_value()) {
                if(res_info->load_if_needed()) {
                    loaded[std::get<0>(*pos)] = std::move(res_info);
                    pos = pending.erase(pos);
                    result++;
                } else {
                    ++pos;
                }
            } else {
                pos = pending.erase(pos);
            }
        }
        return result;
    }

    template <typename Tup, std::size_t... I>
    auto _load(Tup& t, std::index_sequence<I...>) noexcept -> span_size_t {
        return (... + _do_load(std::get<I>(t)));
    }

    template <typename Tup, std::size_t... I>
    void _clean_up(Tup& t, std::index_sequence<I...>) noexcept {
        (void)(..., _do_clean_up(std::get<I>(t)));
    }

    template <typename V>
    void _do_clean_up(V& res_stores) noexcept {
        for(auto& res_store : res_stores) {
            for(auto& res_entry : res_store) {
                if(auto& res_info{std::get<1>(res_entry)}) {
                    res_info->clean_up();
                }
            }
        }
    }

    // the first element is the storage of pending (not yet loaded resources)
    // the second element is the storage of already loaded resources
    std::tuple<std::array<resource_storage<Resources>, 2Z>...> _resources;
};
//------------------------------------------------------------------------------
/// @brief Alias for managed text string resource.
export using managed_plain_text = managed_resource<std::string>;
/// @brief Alias for managed text string list resource.
export using managed_string_list = managed_resource<std::vector<std::string>>;
/// @brief Alias for managed URL list resource.
export using managed_url_list = managed_resource<std::vector<url>>;
/// @brief Alias for managed floating-point number resource.
export using managed_float_vector = managed_resource<std::vector<float>>;
/// @brief Alias for managed floating-point number 3d vector resource.
export using managed_vec3_vector =
  managed_resource<std::vector<math::vector<float, 3>>>;
/// @brief Alias for managed curve of 3d vectors resource.
export using managed_smooth_vec3_curve =
  managed_resource<math::bezier_curves<math::vector<float, 3>, float, 3>>;
/// @brief Alias for managed curve of 4x4 matrices resource.
export using managed_mat4_vector =
  managed_resource<std::vector<math::matrix<float, 4, 4, true>>>;
/// @brief Alias for managed value tree resource.
export using managed_value_tree = managed_resource<valtree::compound>;
/// @brief Alias for managed GL geometry and attribute bindings resource.
export using managed_gl_geometry_and_bindings =
  managed_resource<gl_geometry_and_bindings>;
/// @brief Alias for managed GL shader resource.
export using managed_gl_shader = managed_resource<oglplus::owned_shader_name>;
/// @brief Alias for managed GL GPU program resource.
export using managed_gl_program = managed_resource<oglplus::owned_program_name>;
/// @brief Alias for managed GL texture resource.
export using managed_gl_texture = managed_resource<oglplus::owned_texture_name>;
/// @brief Alias for managed GL buffer resource.
export using managed_gl_buffer = managed_resource<oglplus::owned_buffer_name>;
//------------------------------------------------------------------------------
/// @brief Template for the default resource manager instantiations.
/// @see resource_manager
/// @see managed_resource
export template <typename... Resources>
using default_resource_manager = basic_resource_manager<
  plain_text_resource,
  string_list_resource,
  url_list_resource,
  float_vector_resource,
  vec3_vector_resource,
  smooth_vec3_curve_resource,
  mat4_vector_resource,
  value_tree_resource,
  gl_geometry_and_bindings_resource,
  gl_shader_resource,
  gl_program_resource,
  gl_texture_resource,
  gl_buffer_resource,
  Resources...>;
//------------------------------------------------------------------------------
/// @brief Template for the default resource manager.
/// @see managed_resource
export using resource_manager = default_resource_manager<>;
//------------------------------------------------------------------------------
} // namespace eagine::app
