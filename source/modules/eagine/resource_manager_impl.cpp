/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app;

import std;
import eagine.core.types;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.valid_if;
import eagine.core.identifier;
import eagine.core.container;
import eagine.core.main_ctx;
import :resource_loader;
import :resource_valtree;
import :resource_basic;
import :resource_mapped;

namespace eagine::app {
namespace exp {
//------------------------------------------------------------------------------
// managed_resource_info
//------------------------------------------------------------------------------
auto managed_resource_info::has_parameters() const noexcept -> bool {
    return bool(params.locator);
}
//------------------------------------------------------------------------------
auto managed_resource_info::is_loaded() const noexcept -> bool {
    return resource and resource->is_loaded();
}
//------------------------------------------------------------------------------
auto managed_resource_info::load(
  resource_loader& loader,
  const shared_holder<loaded_resource_context>& context) const noexcept
  -> valid_if_not_zero<identifier_t> {
    if(resource and resource->can_be_loaded()) {
        return loader.load_any(*resource, context, params);
    }
    return {0};
}
//------------------------------------------------------------------------------
void managed_resource_info::on_loaded(
  const shared_holder<resource_interface::loader>& loader) noexcept {
    if(loader and resource) [[likely]] {
        const resource_interface::load_info info{
          params.locator, 0, resource->kind(), resource_status::loaded};
        loader->resource_loaded(info);
    }
}
//------------------------------------------------------------------------------
void managed_resource_info::on_loaded(
  const std::vector<shared_holder<resource_interface::loader>>&
    loaders) noexcept {
    if(resource) [[likely]] {
        const resource_interface::load_info info{
          params.locator, 0, resource->kind(), resource_status::loaded};
        for(auto& loader : loaders) {
            loader->resource_loaded(info);
        }
    }
}
//------------------------------------------------------------------------------
// resource_manager
//------------------------------------------------------------------------------
resource_manager::resource_manager(
  shared_holder<loaded_resource_context> context) noexcept
  : _context{std::move(context)} {
    assert(_context);
    _context->set(*this);
}
//------------------------------------------------------------------------------
auto resource_manager::resource_context() const noexcept
  -> const shared_holder<loaded_resource_context>& {
    return _context;
}
//------------------------------------------------------------------------------
auto resource_manager::loader() noexcept -> resource_loader& {
    return _context->loader();
}
//------------------------------------------------------------------------------
auto resource_manager::_res_id_from(const url& locator) noexcept
  -> resource_identifier {
    if(auto id{locator.query().arg_identifier("resource_id")}) {
        return id;
    }
    return locator.path_identifier();
}
//------------------------------------------------------------------------------
auto resource_manager::_ensure_info(resource_identifier res_id) noexcept
  -> const shared_holder<managed_resource_info>& {
    if(auto found{find(_loaded, res_id)}) {
        return found->ensure();
    }
    auto& info{_pending[res_id]};
    return info.ensure();
}
//------------------------------------------------------------------------------
auto resource_manager::_ensure_parameters(
  resource_identifier res_id,
  resource_request_params params) noexcept
  -> const shared_holder<managed_resource_info>& {
    auto& info{_ensure_info(res_id)};
    info->params = std::move(params);
    return info;
}
//------------------------------------------------------------------------------
auto resource_manager::_ensure_parameters(
  resource_request_params params) noexcept
  -> const shared_holder<managed_resource_info>& {
    auto& info{_ensure_info(_res_id_from(params.locator))};
    info->params = std::move(params);
    return info;
}
//------------------------------------------------------------------------------
auto resource_manager::add_parameters(
  resource_identifier res_id,
  resource_request_params params) noexcept -> resource_manager& {
    _ensure_parameters(res_id, std::move(params));
    return *this;
}
//------------------------------------------------------------------------------
auto resource_manager::add_consumer(
  resource_identifier res_id,
  const std::shared_ptr<resource_interface::loader>& l) noexcept
  -> resource_manager& {
    if(auto found{find(_loaded, res_id)}) {
        auto& info{*found};
        assert(info);
        info->on_loaded(l);
    } else {
        _consumers[res_id].emplace_back(l);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto resource_manager::update() noexcept -> work_done {
    some_true something_done;
    auto& res_loader{loader()};
    auto pos{_pending.begin()};
    while(pos != _pending.end()) {
        const auto& [res_id, info]{*pos};
        assert(info);
        if(info->is_loaded()) {
            if(const auto found{find(_consumers, res_id)}) {
                info->on_loaded(*found);
            }
            _loaded[res_id] = std::move(info);
            pos = _pending.erase(pos);
            something_done();
        } else if(auto req_id{info->load(res_loader, _context)}) {
            something_done();
            ++pos;
        } else {
            ++pos;
        }
    }
    return something_done;
}
//------------------------------------------------------------------------------
// managed_resource_base
//------------------------------------------------------------------------------
void managed_resource_base::_init(
  resource_manager& manager,
  resource_identifier res_id) noexcept {
    _info = {manager._ensure_info(res_id)};
}
//------------------------------------------------------------------------------
void managed_resource_base::_init(
  resource_manager& manager,
  resource_request_params params) noexcept {
    _info = {manager._ensure_parameters(std::move(params))};
}
//------------------------------------------------------------------------------
void managed_resource_base::_init(
  resource_manager& manager,
  resource_identifier res_id,
  resource_request_params params) noexcept {
    _info = {manager._ensure_parameters(res_id, std::move(params))};
}
//------------------------------------------------------------------------------
void managed_resource_base::_add_parameters(
  resource_request_params params) noexcept {
    assert(_info);
    _info->params = std::move(params);
}
//------------------------------------------------------------------------------
managed_resource_base::managed_resource_base(
  resource_manager& manager,
  resource_identifier res_id)
  : _info{manager._ensure_info(res_id)} {}
//------------------------------------------------------------------------------
managed_resource_base::managed_resource_base(
  resource_manager& manager,
  resource_request_params params)
  : _info{manager._ensure_parameters(std::move(params))} {}
//------------------------------------------------------------------------------
managed_resource_base::managed_resource_base(
  resource_manager& manager,
  resource_identifier res_id,
  resource_request_params params)
  : _info{manager._ensure_parameters(res_id, std::move(params))} {}
//------------------------------------------------------------------------------
auto managed_resource_base::is_setup() const noexcept -> bool {
    return _info and _info->resource;
}
//------------------------------------------------------------------------------
auto managed_resource_base::is_loaded() const noexcept -> bool {
    return is_setup() and _info->resource->is_loaded();
}
//------------------------------------------------------------------------------
auto managed_resource_base::kind() const noexcept -> identifier {
    if(_info and _info->resource) {
        return _info->resource->kind();
    }
    return {};
}
//------------------------------------------------------------------------------
auto managed_resource_base::has_parameters() const noexcept -> bool {
    return _info and _info->has_parameters();
}
//------------------------------------------------------------------------------
auto managed_resource_base::load_if_needed(
  resource_manager& manager) const noexcept -> valid_if_not_zero<identifier_t> {
    if(_info) {
        return _info->load(manager.loader(), manager.resource_context());
    }
    return {0};
}
//------------------------------------------------------------------------------
} // namespace exp
} // namespace eagine::app
