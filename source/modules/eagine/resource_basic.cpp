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
import eagine.core.value_tree;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.progress;
import eagine.core.main_ctx;
import eagine.msgbus;
import :resource_loader;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// plain_text_resource
//------------------------------------------------------------------------------
export class plain_text_resource final : public resource_interface {
public:
    auto kind() const noexcept -> resource_kind final {
        return resource_kind::plain_text;
    }

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    using resource_type = std::string;

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_text);
    }

    auto operator->() const noexcept -> const resource_type* {
        return &_text;
    }

    struct _loader;

private:
    friend struct _loader;

    std::string _text;
    resource_status _status{resource_status::created};
};
//------------------------------------------------------------------------------
// string_list_resource
//------------------------------------------------------------------------------
export class string_list_resource final : public resource_interface {
public:
    auto kind() const noexcept -> resource_kind final {
        return resource_kind::string_list;
    }

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    using resource_type = std::vector<std::string>;

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_strings);
    }

    auto operator->() const noexcept -> const resource_type* {
        return &_strings;
    }

    struct _loader;

private:
    friend struct _loader;

    std::vector<std::string> _strings;
    resource_status _status{resource_status::created};
};
//------------------------------------------------------------------------------
// url_list_resource
//------------------------------------------------------------------------------
export class url_list_resource final : public resource_interface {
public:
    auto kind() const noexcept -> resource_kind final {
        return resource_kind::url_list;
    }

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    using resource_type = std::vector<url>;

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_urls);
    }

    auto operator->() const noexcept -> const resource_type* {
        return &_urls;
    }

private:
    struct _loader;
    friend struct _loader;

    std::vector<url> _urls;
    resource_status _status{resource_status::created};
};
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

    auto kind() const noexcept -> resource_kind final {
        return resource_kind::visited_valtree;
    }

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    using resource_type = shared_holder<valtree::value_tree_visitor>;

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_visitor);
    }

private:
    struct _loader;
    friend struct _loader;

    shared_holder<valtree::value_tree_visitor> _visitor;
    span_size_t _max_token_size;
    resource_status _status{resource_status::created};
};
//------------------------------------------------------------------------------
// float_list_resource
//------------------------------------------------------------------------------
export class float_list_resource final : public resource_interface {
public:
    auto kind() const noexcept -> resource_kind final {
        return resource_kind::float_list;
    }

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    using resource_type = std::vector<float>;

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_values);
    }

    auto operator->() const noexcept -> const resource_type* {
        return &_values;
    }

private:
    struct _loader;
    friend struct _loader;

    std::vector<float> _values;
    resource_status _status{resource_status::created};
};
//------------------------------------------------------------------------------
// vec3_list_resource
//------------------------------------------------------------------------------
export class vec3_list_resource final : public resource_interface {
public:
    auto kind() const noexcept -> resource_kind final {
        return resource_kind::vec3_list;
    }

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(resource_request_params params) noexcept
      -> shared_holder<loader> final;

    using resource_type = std::vector<math::vector<float, 3>>;

    auto release_resource() noexcept -> resource_type&& {
        return std::move(_values);
    }

    auto operator->() const noexcept -> const resource_type* {
        return &_values;
    }

private:
    struct _loader;
    friend struct _loader;

    std::vector<math::vector<float, 3>> _values;
    resource_status _status{resource_status::created};
};
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
