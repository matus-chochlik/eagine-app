/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:resource_manager;

import eagine.core.types;
import eagine.core.utility;
import eagine.core.runtime;
import :loaded_resource;
import :context;
import <tuple>;
import <utility>;

namespace eagine::app {
export template <typename... Resources>
class basic_resource_manager;
//------------------------------------------------------------------------------
export template <typename Resource>
class managed_resource {
public:
    managed_resource(loaded_resource<Resource>& ref) noexcept
      : _ref{ref} {}

    template <typename... Resources, typename... Args>
    managed_resource(
      basic_resource_manager<Resources...>& manager,
      url locator,
      Args&&... args)
      : managed_resource{manager.template do_add<Resource>(
          std::move(locator),
          std::forward<Args>(args)...)} {}

    auto resource() const noexcept -> loaded_resource<Resource>& {
        return _ref;
    }

    explicit operator bool() const noexcept {
        return bool(_ref);
    }

    operator loaded_resource<Resource>&() const noexcept {
        return _ref;
    }

    auto operator->() const noexcept -> loaded_resource<Resource>* {
        return &_ref;
    }

    auto connect(
      callable_ref<void(const loaded_resource_base&) noexcept> handler) noexcept
      -> managed_resource& {
        _ref.base_loaded.connect(std::move(handler));
        return *this;
    }

    auto connect(
      callable_ref<
        void(const typename loaded_resource<Resource>::load_info&) noexcept>
        handler) noexcept -> managed_resource& {
        _ref.loaded.connect(std::move(handler));
        return *this;
    }

    template <auto MemFnPtr, typename C>
    auto connect_to(C* obj) noexcept -> managed_resource& {
        return connect(make_callable_ref<MemFnPtr>(obj));
    }

private:
    loaded_resource<Resource>& _ref;
};
//------------------------------------------------------------------------------
export template <typename... Resources>
class basic_resource_manager {
    template <typename L>
    using _lr_transf =
      std::vector<std::unique_ptr<std::tuple<L, resource_load_params<L>>>>;
    template <typename R>
    using _r_transf = _lr_transf<loaded_resource<R>>;

public:
    basic_resource_manager(execution_context& ctx) noexcept
      : _ctx{ctx} {}

    template <typename Resource, typename... Args>
    auto do_add(url locator, Args&&... args) -> managed_resource<Resource> {
        auto& res_vec{std::get<_r_transf<Resource>>(_resources)};
        res_vec.emplace_back(
          std::make_unique<
            std::tuple<loaded_resource<Resource>, resource_load_params<Resource>>>(
            std::move(locator),
            resource_load_params<Resource>{std::forward<Args>(args)...}));
        auto& res = std::get<0>(*res_vec.back());
        res.init(_ctx);
        return {res};
    }

    auto context() const noexcept -> execution_context& {
        return _ctx;
    }

    auto are_loaded() noexcept -> bool {
        return _are_loaded(_resources, _is());
    }

    auto load() noexcept -> span_size_t {
        return _load(_resources, _is());
    }

    auto clean_up() noexcept -> auto& {
        _clean_up(_resources, _is());
        return *this;
    }

private:
    static constexpr auto _is() noexcept {
        return std::make_index_sequence<sizeof...(Resources)>();
    }

    template <typename Tup, std::size_t... I>
    auto _are_loaded(Tup& t, std::index_sequence<I...>) noexcept -> bool {
        return (... && _are_loaded(std::get<I>(t)));
    }

    template <typename V>
    auto _are_loaded(V& res_vec) noexcept -> span_size_t {
        for(auto& res_info : res_vec) {
            if(!std::get<0>(*res_info).is_loaded()) {
                return false;
            }
        }
        return true;
    }

    template <typename Tup, std::size_t... I>
    auto _load(Tup& t, std::index_sequence<I...>) noexcept -> span_size_t {
        return (... + _do_load(std::get<I>(t)));
    }

    template <typename V>
    auto _do_load(V& res_vec) noexcept -> span_size_t {
        span_size_t result{0};
        for(auto& res_info : res_vec) {
            auto& [res, params] = *res_info;
            result += span_size_t{res.load_if_needed(_ctx, params) ? 1 : 0};
        }
        return result;
    }

    template <typename Tup, std::size_t... I>
    void _clean_up(Tup& t, std::index_sequence<I...>) noexcept {
        (void)(..., _do_clean_up(std::get<I>(t)));
    }

    template <typename V>
    void _do_clean_up(V& res_vec) noexcept {
        for(auto& res_info : res_vec) {
            std::get<0>(*res_info).clean_up(_ctx);
        }
    }

    execution_context& _ctx;
    std::tuple<_lr_transf<Resources>...> _resources;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
