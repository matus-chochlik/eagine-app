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
// gl_shader_resource
//------------------------------------------------------------------------------
auto gl_shader_resource::kind() const noexcept -> identifier {
    return "GLShdrIncl";
}
//------------------------------------------------------------------------------
struct gl_shader_resource::_loader final
  : simple_loader_of<gl_shader_resource> {
    using base = simple_loader_of<gl_shader_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    glsl_string_resource _glsl;
};
//------------------------------------------------------------------------------
auto gl_shader_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_glsl, parameters()), res_loader);
}
//------------------------------------------------------------------------------
static auto shader_type_from(const url& locator, auto& glapi)
  -> optionally_valid<oglplus::shader_type> {
    if(
      locator.has_scheme("glsl_vert") or
      locator.query().arg_has_value("type", "vertex") or
      locator.query().arg_has_value("type", "vertex_shader") or
      (locator.has_scheme("glsl") and locator.has_path_suffix(".vert"))) {
        return {glapi.constants().vertex_shader};
    }
    if(
      locator.has_scheme("glsl_teco") or
      locator.query().arg_has_value("type", "tess_control") or
      locator.query().arg_has_value("type", "tess_control_shader") or
      (locator.has_scheme("glsl") and locator.has_path_suffix(".teco"))) {
        return {glapi.constants().tess_control_shader};
    }
    if(
      locator.has_scheme("glsl_teev") or
      locator.query().arg_has_value("type", "tess_evaluation") or
      locator.query().arg_has_value("type", "tess_evaluation_shader") or
      (locator.has_scheme("glsl") and locator.has_path_suffix(".teev"))) {
        return {glapi.constants().tess_evaluation_shader};
    }
    if(
      locator.has_scheme("glsl_geom") or
      locator.query().arg_has_value("type", "geometry") or
      locator.query().arg_has_value("type", "geometry_shader") or
      (locator.has_scheme("glsl") and locator.has_path_suffix(".geom"))) {
        return {glapi.constants().geometry_shader};
    }
    if(
      locator.has_scheme("glsl_frag") or
      locator.query().arg_has_value("type", "fragment") or
      locator.query().arg_has_value("type", "fragment_shader") or
      (locator.has_scheme("glsl") and locator.has_path_suffix(".frag"))) {
        return {glapi.constants().fragment_shader};
    }
    if(
      locator.has_scheme("glsl_comp") or
      locator.query().arg_has_value("type", "compute") or
      locator.query().arg_has_value("type", "compute_shader") or
      (locator.has_scheme("glsl") and locator.has_path_suffix(".comp"))) {
        return {glapi.constants().compute_shader};
    }
    return {};
}
//------------------------------------------------------------------------------
void gl_shader_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(auto res_ctx{resource_context()}) {
        auto& glapi{res_ctx->gl_api()};
        if(const auto shdr_type{shader_type_from(info.locator, glapi)}) {
            oglplus::owned_shader_name shdr;
            const auto cleanup_if_failed{glapi.delete_shader.raii(shdr)};
            if(glapi.create_shader(*shdr_type).and_then(_1.move_to(shdr))) {
                if(glapi.shader_source(shdr, _glsl.get())) {
                    if(glapi.compile_shader(shdr)) {
                        resource()._private_ref() = std::move(shdr);
                        base::resource_loaded(info);
                        return;
                    }
                }
            }
        }
    }
    base::resource_cancelled(info);
}
//------------------------------------------------------------------------------
auto gl_shader_resource::make_loader(
  const shared_holder<loaded_resource_context>& context,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(context and context->gl_context()) {
        return {
          hold<gl_shader_resource::_loader>, *this, context, std::move(params)};
    }
    return {};
}
//------------------------------------------------------------------------------
void gl_shader_resource::clean_up(loaded_resource_context& context) noexcept {
    if(get()) {
        assert(context.gl_context());
        context.gl_api().clean_up(release_resource());
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::app::exp

