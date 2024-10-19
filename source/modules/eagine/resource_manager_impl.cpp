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
auto managed_resource_info::should_be_loaded() const noexcept -> bool {
    return resource and resource->should_be_loaded();
}
//------------------------------------------------------------------------------
auto managed_resource_info::load_if_needed(
  resource_loader& loader,
  const shared_holder<loaded_resource_context>& context) const noexcept
  -> valid_if_not_zero<identifier_t> {
    if(should_be_loaded()) {
        return loader.load_any(*resource, context, params);
    }
    return {0};
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
    auto& info{_resources[res_id]};
    info.ensure();
    return info;
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
auto resource_manager::update() noexcept -> work_done {
    some_true something_done;
    auto& res_loader{loader()};
    for(const auto& entry : _resources) {
        if(const auto& info{std::get<1>(entry)}) {
            something_done(
              info->load_if_needed(res_loader, _context).has_value());
        }
    }
    return something_done;
}
//------------------------------------------------------------------------------
// managed_resource_base
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
auto managed_resource_base::is_loaded() const noexcept -> bool {
    return _info and _info->resource and _info->resource->is_loaded();
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
        return _info->load_if_needed(
          manager.loader(), manager.resource_context());
    }
    return {0};
}
//------------------------------------------------------------------------------
} // namespace exp
} // namespace eagine::app
