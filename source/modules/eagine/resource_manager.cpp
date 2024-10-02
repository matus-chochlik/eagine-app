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
    struct resource_and_params {
        loaded_resource_context context; // TODO: optimize as flyweight
        Resource resource;
        resource_load_params<Resource> params;

        template <typename... Args>
        resource_and_params(
          loaded_resource_context& ctx,
          url locator,
          Args&&... args) noexcept
          : context{ctx}
          , resource{std::move(locator), ctx}
          , params{std::forward<Args>(args)...} {}

        auto load_if_needed() noexcept {
            return resource.load_if_needed(context, params);
        }

        auto clean_up() noexcept {
            return resource.clean_up(context);
        }
    };

public:
    template <typename Resource>
    using managed_resource_context =
      resource_and_params<adjust_res_t<Resource>>;
};
//------------------------------------------------------------------------------
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
      identifier resource_id,
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
      identifier resource_id,
      url locator,
      Args&&... args)
      : managed_resource{
          manager,
          ec.resource_context(),
          resource_id,
          std::move(locator),
          std::forward<Args>(args)...} {}

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

    auto resource() const noexcept -> loaded_resource<Resource>& {
        return _ref->resource;
    }

    auto is_loaded() const noexcept -> bool {
        return resource().is_loaded();
    }

    explicit operator bool() const noexcept {
        return is_loaded();
    }

    operator loaded_resource<Resource>&() const noexcept {
        return resource();
    }

    auto operator->() const noexcept -> loaded_resource<Resource>* {
        return &(resource());
    }

    auto connect(
      callable_ref<void(const loaded_resource_base&) noexcept> handler) noexcept
      -> managed_resource& {
        resource().load_event.connect(std::move(handler));
        return *this;
    }

    auto connect(
      callable_ref<
        void(const typename loaded_resource<Resource>::load_info&) noexcept>
        handler) noexcept -> managed_resource& {
        resource().loaded.connect(std::move(handler));
        return *this;
    }

    template <auto MemFnPtr, typename C>
    auto connect_to(C* obj) noexcept -> managed_resource& {
        return connect(make_callable_ref<MemFnPtr>(obj));
    }

private:
    shared_holder<_context_t> _ref;
};
//------------------------------------------------------------------------------
export template <typename... Resources>
class basic_resource_manager {

    template <typename X>
    using resource_storage = std::vector<
      shared_holder<managed_resource_utils::managed_resource_context<X>>>;

public:
    template <typename Resource, typename... Args>
    [[nodiscard]] auto do_add(
      std::type_identity<Resource> rtid,
      identifier, // TODO (use as map key)
      url locator,
      loaded_resource_context& context,
      Args&&... args)
      -> shared_holder<
        managed_resource_utils::managed_resource_context<Resource>> {
        auto& res_vec{_get(rtid)};
        res_vec.emplace_back(
          std::make_shared<
            managed_resource_utils::managed_resource_context<Resource>>(
            context, std::move(locator), std::forward<Args>(args)...));
        return res_vec.back();
    }

    [[nodiscard]] auto are_loaded() noexcept -> bool {
        return _are_loaded(_resources, _is());
    }

    auto load() noexcept -> span_size_t {
        return _load(_resources, _is());
    }

    auto update() noexcept -> basic_resource_manager& {
        if(not are_loaded()) {
            load();
        }
        return *this;
    }

    auto clean_up() noexcept -> basic_resource_manager& {
        _clean_up(_resources, _is());
        return *this;
    }

private:
    template <typename Resource>
    auto _get(std::type_identity<Resource>) noexcept
      -> resource_storage<Resource>& {
        return std::get<resource_storage<Resource>>(_resources);
    }

    static constexpr auto _is() noexcept {
        return std::make_index_sequence<sizeof...(Resources)>();
    }

    template <typename V>
    auto _are_loaded(V& res_vec) const noexcept -> span_size_t {
        for(const auto& res_info : res_vec) {
            if(not res_info->resource.is_loaded()) {
                return false;
            }
        }
        return true;
    }

    template <typename Tup, std::size_t... I>
    auto _are_loaded(Tup& t, std::index_sequence<I...>) const noexcept -> bool {
        return (... and _are_loaded(std::get<I>(t)));
    }

    template <typename V>
    auto _do_load(V& res_vec) noexcept -> span_size_t {
        span_size_t result{0};
        for(auto& res_info : res_vec) {
            if(res_info->load_if_needed()) {
                ++result;
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
    void _do_clean_up(V& res_vec) noexcept {
        for(auto& res_info : res_vec) {
            res_info->clean_up();
        }
    }

    std::tuple<resource_storage<Resources>...> _resources;
};
//------------------------------------------------------------------------------
export using managed_plain_text = managed_resource<std::string>;
export using managed_string_list = managed_resource<std::vector<std::string>>;
export using managed_url_list = managed_resource<std::vector<url>>;
export using managed_float_vector = managed_resource<std::vector<float>>;
export using managed_vec3_vector =
  managed_resource<std::vector<math::vector<float, 3>>>;
export using managed_smooth_vec3_curve =
  managed_resource<math::bezier_curves<math::vector<float, 3>, float, 3>>;
export using managed_mat4_vector =
  managed_resource<std::vector<math::matrix<float, 4, 4, true>>>;
export using managed_value_tree = managed_resource<valtree::compound>;
export using managed_gl_geometry_and_bindings =
  managed_resource<gl_geometry_and_bindings>;
export using managed_gl_shader = managed_resource<oglplus::owned_shader_name>;
export using managed_gl_program = managed_resource<oglplus::owned_program_name>;
export using managed_gl_texture = managed_resource<oglplus::owned_texture_name>;
export using managed_gl_buffer = managed_resource<oglplus::owned_buffer_name>;
//------------------------------------------------------------------------------
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
export using resource_manager = default_resource_manager<>;
//------------------------------------------------------------------------------
} // namespace eagine::app
