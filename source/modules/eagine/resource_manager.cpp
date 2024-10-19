/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app:resource_manager;

import std;
import eagine.core.types;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.valid_if;
import eagine.core.identifier;
import eagine.core.container;
import eagine.core.main_ctx;
import :resource_loader;
import :resource_mapped;

namespace eagine::app {
namespace exp {
//------------------------------------------------------------------------------
export using resource_identifier = identifier;
//------------------------------------------------------------------------------
struct managed_resource_info {
    unique_holder<resource_interface> resource;
    resource_request_params params;

    template <std::derived_from<resource_interface> Resource>
    auto ensure(std::type_identity<Resource> tid) -> bool {
        if(resource) {
            return resource.is(tid);
        } else {
            return resource.ensure_derived(hold<Resource>).has_value();
        }
    }

    template <std::derived_from<resource_interface> Resource>
    auto is(std::type_identity<Resource> tid) -> bool {
        return resource.is(tid);
    }

    template <std::derived_from<resource_interface> Resource>
    auto as_ref(std::type_identity<Resource> tid)
      -> optional_reference<std::add_const_t<typename Resource::resource_type>> {
        if(auto res{resource.as_ref(tid)}) {
            return {res->get()};
        }
        return {};
    }

    auto has_parameters() const noexcept -> bool;

    auto should_be_loaded() const noexcept -> bool;

    auto load_if_needed(
      resource_loader&,
      const shared_holder<loaded_resource_context>&) const noexcept
      -> valid_if_not_zero<identifier_t>;
};
//------------------------------------------------------------------------------
class managed_resource_base;

export class resource_manager {
public:
    resource_manager(shared_holder<loaded_resource_context>) noexcept;

    [[nodiscard]] auto resource_context() const noexcept
      -> const shared_holder<loaded_resource_context>&;

    [[nodiscard]] auto loader() noexcept -> resource_loader&;

    auto add_parameters(
      resource_identifier res_id,
      resource_request_params) noexcept -> resource_manager&;

    auto update() noexcept -> work_done;

private:
    friend class managed_resource_base;

    static auto _res_id_from(const url&) noexcept -> resource_identifier;

    auto _ensure_info(resource_identifier res_id) noexcept
      -> const shared_holder<managed_resource_info>&;

    auto _ensure_parameters(
      resource_identifier res_id,
      resource_request_params) noexcept
      -> const shared_holder<managed_resource_info>&;

    auto _ensure_parameters(resource_request_params) noexcept
      -> const shared_holder<managed_resource_info>&;

    shared_holder<loaded_resource_context> _context;
    chunk_map<resource_identifier, shared_holder<managed_resource_info>, 4096>
      _resources;
};
//------------------------------------------------------------------------------
class managed_resource_base {
public:
    [[nodiscard]] auto has_storage() const noexcept -> bool;

    [[nodiscard]] auto is_loaded() const noexcept -> bool;

    explicit operator bool() const noexcept {
        return is_loaded();
    }

    [[nodiscard]] auto kind() const noexcept -> identifier;

    [[nodiscard]] auto has_parameters() const noexcept -> bool;

    auto load_if_needed(resource_manager&) const noexcept
      -> valid_if_not_zero<identifier_t>;

protected:
    managed_resource_base() noexcept = default;

    void _init(resource_manager&, resource_identifier) noexcept;
    void _init(resource_manager&, resource_request_params) noexcept;
    void _init(
      resource_manager&,
      resource_identifier,
      resource_request_params) noexcept;

    void _add_parameters(resource_request_params) noexcept;

    managed_resource_base(resource_manager&, resource_identifier);
    managed_resource_base(resource_manager&, resource_request_params);

    managed_resource_base(
      resource_manager&,
      resource_identifier,
      resource_request_params);

    shared_holder<managed_resource_info> _info;
};
//------------------------------------------------------------------------------
export template <typename Resource>
class managed_resource : public managed_resource_base {
    static constexpr auto _rid() noexcept -> std::type_identity<Resource> {
        return {};
    }

    void _ensure_storage() noexcept {
        this->_info->ensure(_rid());
    }

public:
    managed_resource() noexcept = default;

    managed_resource(
      resource_manager& manager,
      resource_identifier res_id) noexcept
      : managed_resource_base{manager, res_id} {
        _ensure_storage();
    }

    managed_resource(
      resource_manager& manager,
      resource_request_params params) noexcept
      : managed_resource_base{manager, std::move(params)} {
        _ensure_storage();
    }

    managed_resource(resource_manager& manager, url locator) noexcept
      : managed_resource_base{
          manager,
          resource_request_params{.locator = std::move(locator)}} {
        _ensure_storage();
    }

    managed_resource(
      resource_manager& manager,
      resource_identifier res_id,
      resource_request_params params) noexcept
      : managed_resource_base{manager, res_id, std::move(params)} {
        _ensure_storage();
    }

    auto init(resource_manager& manager, resource_identifier res_id) noexcept
      -> managed_resource& {
        this->_init(manager, res_id);
        _ensure_storage();
        return *this;
    }

    auto init(resource_manager& manager, resource_request_params params) noexcept
      -> managed_resource& {
        this->_init(manager, std::move(params));
        _ensure_storage();
        return *this;
    }

    auto init(
      resource_manager& manager,
      resource_identifier res_id,
      resource_request_params params) noexcept -> managed_resource& {
        this->_init(manager, res_id, std::move(params));
        _ensure_storage();
        return *this;
    }

    auto add_parameters(resource_request_params params) noexcept
      -> managed_resource& {
        this->_add_parameters(std::move(params));
        return *this;
    }

    auto ref() const noexcept
      -> optional_reference<std::add_const_t<typename Resource::resource_type>> {
        if(this->_info) {
            return this->_info->as_ref(_rid());
        }
        return {};
    }

    auto get() const noexcept
      -> std::add_const_t<typename Resource::resource_type>& {
        auto res_ref{ref()};
        assert(res_ref);
        return *res_ref;
    }

    auto operator->() const noexcept
      -> std::add_const_t<typename Resource::resource_type>* {
        auto res_ref{ref()};
        assert(res_ref);
        return res_ref.operator->();
    }
};
//------------------------------------------------------------------------------
} // namespace exp
export using exp::managed_resource;
export using exp::resource_manager;
} // namespace eagine::app
