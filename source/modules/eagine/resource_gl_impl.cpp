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
import eagine.core.identifier;
import eagine.core.valid_if;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.msgbus;
import eagine.shapes;
import eagine.oglplus;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// gl_shader_include_resource
//------------------------------------------------------------------------------
auto gl_shader_include_resource::kind() const noexcept -> identifier {
    return "GLShdrIncl";
}
//------------------------------------------------------------------------------
struct gl_shader_include_resource::_loader final
  : simple_loader_of<gl_shader_include_resource> {
    using base = simple_loader_of<gl_shader_include_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    glsl_string_resource _glsl;
};
//------------------------------------------------------------------------------
auto gl_shader_include_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_glsl, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void gl_shader_include_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(auto path{info.locator.query().decoded_arg_value("path")}) {
        if(auto res_ctx{resource_context()}) {
            auto& glapi{res_ctx->gl_api()};
            if(glapi.add_shader_include(*path, _glsl->storage())) {
                resource()._private_ref() = {std::move(*path)};
                base::resource_loaded(info);
                return;
            }
        }
    }
    base::resource_cancelled(info);
}
//------------------------------------------------------------------------------
auto gl_shader_include_resource::make_loader(
  const shared_holder<loaded_resource_context>& context,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(context and context->gl_context()) {
        return {
          hold<gl_shader_include_resource::_loader>,
          *this,
          context,
          std::move(params)};
    }
    return {};
}
//------------------------------------------------------------------------------
void gl_shader_include_resource::clean_up(
  loaded_resource_context& context) noexcept {
    if(get()) {
        assert(context.gl_context());
        context.gl_api().clean_up(release_resource());
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::app::exp

