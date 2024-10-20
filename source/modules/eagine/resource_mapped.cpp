/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app:resource_mapped;

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.serialization;
import eagine.core.identifier;
import eagine.core.value_tree;
import eagine.core.reflection;
import eagine.core.valid_if;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.progress;
import eagine.core.main_ctx;
import eagine.msgbus;
import :resource_loader;
import :resource_valtree;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// valtree_mapped_struct_builder
//------------------------------------------------------------------------------
template <mapped_struct O>
class valtree_mapped_struct_builder final
  : public valtree::object_builder_impl<valtree_mapped_struct_builder<O>> {
    using base = valtree::object_builder_impl<valtree_mapped_struct_builder<O>>;

public:
    auto max_token_size() noexcept -> span_size_t final {
        return max_identifier_length(_object);
    }

    void do_add(const basic_string_path&, const auto&) noexcept {}

    template <typename T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        _forwarder.forward_data(path, data, _object);
    }

    auto finish() noexcept -> bool final;

    using resource_type = O;

    auto release_resource() noexcept -> O&& {
        return std::move(_object);
    }

private:
    O _object{};
    valtree::object_builder_data_forwarder _forwarder;
};
//------------------------------------------------------------------------------
template <mapped_struct O>
auto valtree_mapped_struct_builder<O>::finish() noexcept -> bool {
    // TODO: notify continuation
    return true;
}
//------------------------------------------------------------------------------
export template <mapped_struct O>
auto make_mapped_struct_builder(std::type_identity<O> = {}) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_mapped_struct_builder<O>>};
}
//------------------------------------------------------------------------------
export template <mapped_struct Struct>
class mapped_struct_resource final : public resource_interface {
public:
    auto kind() const noexcept -> identifier final {
        return "MppdStruct";
    }

    auto load_status() const noexcept -> resource_status final {
        return _status;
    }

    auto make_loader(
      main_ctx_parent parent,
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final {
        return {
          hold<mapped_struct_resource::_loader>,
          parent,
          *this,
          std::move(params),
          make_mapped_struct_builder(std::type_identity<Struct>{})};
    }

private:
    struct _loader final : loader_of<mapped_struct_resource> {

        _loader(
          resource_interface& resource,
          resource_request_params params,
          shared_holder<valtree::object_builder> builder) noexcept
          : loader_of<mapped_struct_resource>{resource, std::move(params)}
          , _visit{std::move(builder)} {}

        auto request_dependencies() noexcept
          -> valid_if_not_zero<identifier_t> final {
            return this->add_single_dependency(
              this->parent_loader().load(_visit, this->parameters()),
              _visit_req_id);
        }

        visited_valtree_resource _visit;
        identifier_t _visit_req_id{0};
    };
    friend struct _loader;

    Struct _object{};
    resource_status _status{resource_status::created};
};
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
