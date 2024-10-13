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
import eagine.core.memory;
import eagine.core.string;
import eagine.core.math;
import eagine.core.identifier;
import eagine.core.value_tree;
import eagine.core.valid_if;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.msgbus;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// valtree_resource
//------------------------------------------------------------------------------
auto valtree_resource::kind() const noexcept -> identifier {
    return "ValueTree";
}
//------------------------------------------------------------------------------
struct valtree_resource::_loader final : simple_loader_of<valtree_resource> {
    using base = simple_loader_of<valtree_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    plain_text_resource _text;
};
//------------------------------------------------------------------------------
auto valtree_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_text, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void valtree_resource::_loader::resource_loaded(const load_info& info) noexcept {
    if(info.locator.has_path_suffix(".json") or info.locator.has_scheme("json")) {
        if(auto tree{valtree::from_json_text(_text.get(), main_ctx::get())}) {
            resource()._private_ref() = std::move(tree);
            base::resource_loaded(info);
            return;
        }
    }
    if(info.locator.has_path_suffix(".yaml") or info.locator.has_scheme("yaml")) {
        if(auto tree{valtree::from_yaml_text(_text.get(), main_ctx::get())}) {
            resource()._private_ref() = std::move(tree);
            base::resource_loaded(info);
            return;
        }
    }
    base::resource_cancelled(info);
}
//------------------------------------------------------------------------------
auto valtree_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(
      params.locator.has_path_suffix(".json") or
      params.locator.has_scheme("json") or
      params.locator.has_path_suffix(".yaml") or
      params.locator.has_scheme("yaml")) {
        return {hold<valtree_resource::_loader>, *this, ctx, std::move(params)};
    }
    return {};
}
//------------------------------------------------------------------------------
// visited_valtree_resource
//------------------------------------------------------------------------------
auto visited_valtree_resource::kind() const noexcept -> identifier {
    return "ValtrVisit";
}
//------------------------------------------------------------------------------
struct visited_valtree_resource::_loader final
  : simple_loader_of<visited_valtree_resource> {
    using base = simple_loader_of<visited_valtree_resource>;
    using base::base;

    _loader(
      resource_interface& resource,
      const shared_holder<loaded_resource_context>& context,
      resource_request_params params,
      valtree::value_tree_stream_input input) noexcept
      : base{resource, context, std::move(params)}
      , _input{std::move(input)} {}

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;

    void stream_finished(identifier_t) noexcept final;

    valtree::value_tree_stream_input _input;
    bool _finished{false};
};
//------------------------------------------------------------------------------
auto visited_valtree_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.fetch_resource_chunks(parameters(), 1024).first, res_loader);
}
//------------------------------------------------------------------------------
void visited_valtree_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    for(const auto chunk : chunks.data) {
        if(not _finished) {
            if(not _input.consume_data(chunk)) {
                _input.finish();
                _finished = true;
            }
        }
    }
}
//------------------------------------------------------------------------------
void visited_valtree_resource::_loader::stream_finished(
  identifier_t request_id) noexcept {
    if(not _finished) {
        _input.finish();
        _finished = true;
    }
    base::stream_finished(request_id);
}
//------------------------------------------------------------------------------
auto visited_valtree_resource::make_loader(
  const shared_holder<loaded_resource_context>& context,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(
      params.locator.has_path_suffix(".json") or
      params.locator.has_scheme("json")) {
        return {
          hold<visited_valtree_resource::_loader>,
          *this,
          context,
          std::move(params),
          traverse_json_stream(
            _visitor,
            _max_token_size,
            main_ctx::get().buffers(),
            main_ctx::get().log())};
    }
    if(
      params.locator.has_path_suffix(".yaml") or
      params.locator.has_scheme("yaml")) {
        return {
          hold<visited_valtree_resource::_loader>,
          *this,
          context,
          std::move(params),
          traverse_yaml_stream(
            _visitor,
            _max_token_size,
            main_ctx::get().buffers(),
            main_ctx::get().log())};
    }
    return {};
}
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
