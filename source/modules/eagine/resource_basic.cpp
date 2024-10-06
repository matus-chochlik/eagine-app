/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app:resource_basic;

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.identifier;
import eagine.core.value_tree;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.progress;
import eagine.core.main_ctx;
import eagine.msgbus;
import :resource_loader;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// visited valtree resource
//------------------------------------------------------------------------------
export class visited_valtree_resource final : public resource_interface {
public:
    visited_valtree_resource(
      shared_holder<valtree::value_tree_visitor> visitor,
      span_size_t max_token_size) noexcept
      : _visitor{std::move(visitor)}
      , _max_token_size{max_token_size} {}

    auto kind() const noexcept -> identifier final;

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    using resource_type = shared_holder<valtree::value_tree_visitor>;

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_visitor);
    }

    template <std::derived_from<valtree::object_builder> Builder>
    auto builder_as() noexcept -> shared_holder<Builder> {
        if(_visitor) {
            return _visitor->template get_builder_as<Builder>();
        }
        return {};
    }

    struct _loader;

private:
    friend struct _loader;

    shared_holder<valtree::value_tree_visitor> _visitor;
    span_size_t _max_token_size;
    resource_status _status{resource_status::created};
};
//------------------------------------------------------------------------------
// plain_text_resource
//------------------------------------------------------------------------------
export class plain_text_resource final : public simple_resource<std::string> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// string_list_resource
//------------------------------------------------------------------------------
export class string_list_resource final
  : public simple_resource<std::vector<std::string>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// url_list_resource
//------------------------------------------------------------------------------
export class url_list_resource final
  : public simple_resource<std::vector<url>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// float_list_resource
//------------------------------------------------------------------------------
export class float_list_resource final
  : public simple_resource<std::vector<float>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// vec3_list_resource
//------------------------------------------------------------------------------
export class vec3_list_resource final
  : public simple_resource<std::vector<math::vector<float, 3>>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// mat4_list_resource
//------------------------------------------------------------------------------
export class mat4_list_resource final
  : public simple_resource<std::vector<math::matrix<float, 4, 4, true>>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
