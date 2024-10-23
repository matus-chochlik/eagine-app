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
import eagine.core.value_tree;
import eagine.core.main_ctx;
import eagine.msgbus;
import eagine.shapes;
import eagine.oglplus;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// gl_shader_include_resource
//------------------------------------------------------------------------------
struct gl_shader_include_resource::_loader final
  : simple_loader_of<gl_shader_include_resource> {
    using base = simple_loader_of<gl_shader_include_resource>;
    using base::base;

    auto request_dependencies() noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    glsl_string_resource _glsl;
};
//------------------------------------------------------------------------------
auto gl_shader_include_resource::_loader::request_dependencies() noexcept
  -> valid_if_not_zero<identifier_t> {
    return add_single_loader_dependency(
      parent_loader().load(_glsl, resource_context(), parameters()));
}
//------------------------------------------------------------------------------
void gl_shader_include_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(auto path{info.locator.query().decoded_arg_value("path")}) {
        if(auto res_ctx{resource_context()}) {
            auto& glapi{res_ctx->gl_api()};
            if(glapi.add_shader_include(*path, _glsl->storage())) {
                resource()._private_ref() = {std::move(*path)};
                mark_loaded();
                return;
            }
        }
    }
    mark_error();
}
//------------------------------------------------------------------------------
auto gl_shader_include_resource::kind() const noexcept -> identifier {
    return "GLShdrIncl";
}
//------------------------------------------------------------------------------
auto gl_shader_include_resource::make_loader(
  main_ctx_parent parent,
  const shared_holder<loaded_resource_context>& context,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(context and context->gl_context()) {
        return {
          hold<gl_shader_include_resource::_loader>,
          parent,
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
// gl_shader_includes_resource
//------------------------------------------------------------------------------
struct gl_shader_includes_resource::_loader final
  : simple_loader_of<gl_shader_includes_resource> {
    using base = simple_loader_of<gl_shader_includes_resource>;
    using base::base;

    auto request_dependencies() noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    url_list_resource _urls;
    identifier_t _urls_req_id{0};

    std::vector<
      std::tuple<identifier_t, unique_keeper<gl_shader_include_resource>>>
      _includes;
};
//------------------------------------------------------------------------------
auto gl_shader_includes_resource::_loader::request_dependencies() noexcept
  -> valid_if_not_zero<identifier_t> {
    return add_single_loader_dependency(
      parent_loader().load(_urls, resource_context(), parameters()),
      _urls_req_id);
}
//------------------------------------------------------------------------------
void gl_shader_includes_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(info.has_request_id(_urls_req_id)) {
        auto urls{_urls.release_resource()};
        for(auto& locator : urls) {
            _includes.emplace_back();
            auto& [request_id, shdr_incl]{_includes.back()};
            if(auto new_req_id{parent_loader().load(
                 *shdr_incl,
                 resource_context(),
                 {.locator = std::move(locator)})}) {
                request_id = new_req_id.value_anyway();
                add_as_loader_consumer_of(request_id);
            } else {
                _includes.pop_back();
                // TODO: log error
            }
        }
        return;
    } else {
        for(auto pos{_includes.begin()}; pos != _includes.end(); ++pos) {
            auto& [request_id, shdr_incl]{*pos};
            if(info.has_request_id(request_id)) {
                resource()._private_ref().emplace_back(
                  std::move(shdr_incl->release_resource()));
                _includes.erase(pos);
                if(_includes.empty()) {
                    mark_loaded();
                }
                return;
            }
        }
    }
    mark_error();
}
//------------------------------------------------------------------------------
auto gl_shader_includes_resource::kind() const noexcept -> identifier {
    return "GLShdrIncs";
}
//------------------------------------------------------------------------------
auto gl_shader_includes_resource::make_loader(
  main_ctx_parent parent,
  const shared_holder<loaded_resource_context>& context,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(context and context->gl_context()) {
        return {
          hold<gl_shader_includes_resource::_loader>,
          parent,
          *this,
          context,
          std::move(params)};
    }
    return {};
}
//------------------------------------------------------------------------------
void gl_shader_includes_resource::clean_up(
  loaded_resource_context& context) noexcept {
    assert(context.gl_context());
    auto incls{release_resource()};
    for(auto& shdr_incl : incls) {
        context.gl_api().clean_up(std::move(shdr_incl));
    }
}
//------------------------------------------------------------------------------
static auto is_glsl_shader_locator(const url& locator) noexcept {
    return locator.has_scheme("glsl") or locator.has_path_suffix(".glsl") or
           locator.has_scheme("glsl_vert") or locator.has_scheme("glsl_teco") or
           locator.has_scheme("glsl_teev") or locator.has_scheme("glsl_geom") or
           locator.has_scheme("glsl_frag") or locator.has_scheme("glsl_comp");
}
//------------------------------------------------------------------------------
// shader_type_from
//------------------------------------------------------------------------------
static auto shader_type_from(const url& locator, auto& glapi) noexcept
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
// gl_shader_resource::_loader_glsl
//------------------------------------------------------------------------------
struct gl_shader_resource::_loader_glsl final
  : simple_loader_of<gl_shader_resource, gl_shader_resource::_loader_glsl> {
    using base =
      simple_loader_of<gl_shader_resource, gl_shader_resource::_loader_glsl>;
    using base::base;

    auto request_dependencies() noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    glsl_string_resource _glsl;
};
//------------------------------------------------------------------------------
auto gl_shader_resource::_loader_glsl::request_dependencies() noexcept
  -> valid_if_not_zero<identifier_t> {
    return add_single_loader_dependency(
      parent_loader().load(_glsl, resource_context(), parameters()));
}
//------------------------------------------------------------------------------
void gl_shader_resource::_loader_glsl::resource_loaded(
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
                        mark_loaded();
                        return;
                    }
                }
            }
        }
    }
    mark_error();
}
//------------------------------------------------------------------------------
// gl_shader_resource::_loader_eagishdr
//------------------------------------------------------------------------------
struct gl_shader_resource::_loader_eagishdr final
  : simple_loader_of<gl_shader_resource, gl_shader_resource::_loader_eagishdr> {
    using base =
      simple_loader_of<gl_shader_resource, gl_shader_resource::_loader_eagishdr>;
    using base::base;

    auto request_dependencies() noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    managed_resource<gl_shader_parameters_resource> _param;
    managed_resource<glsl_string_resource> _source;
    managed_resources<gl_shader_include_resource> _includes;
    managed_resources<gl_shader_includes_resource> _libraries;
};
//------------------------------------------------------------------------------
auto gl_shader_resource::_loader_eagishdr::request_dependencies() noexcept
  -> valid_if_not_zero<identifier_t> {
    set_status(resource_status::loading);
    auto& manager{parent_manager()};
    add_as_manager_consumer_of(_param.setup(manager, parameters()));
    return {acquire_request_id()};
}
//------------------------------------------------------------------------------
void gl_shader_resource::_loader_eagishdr::resource_loaded(
  const load_info& info) noexcept {
    if(_param) {
        auto& manager{parent_manager()};
        if(not _source.is_setup()) {
            add_as_manager_consumer_of(
              _source.setup(manager, _param->source_url));
        }
        if(not _includes.is_setup()) {
            for(auto& locator : _param->include_urls) {
                add_as_manager_consumer_of(
                  _includes.setup(manager, std::move(locator)));
            }
        }
        if(not _libraries.is_setup()) {
            for(auto& locator : _param->library_urls) {
                add_as_manager_consumer_of(
                  _libraries.setup(manager, std::move(locator)));
            }
        }
    }

    if(_source and _includes and _libraries) {
        if(auto res_ctx{resource_context()}) {
            auto& glapi{res_ctx->gl_api()};
            if(const auto shdr_type{
                 shader_type_from(_param->source_url, glapi)}) {
                oglplus::owned_shader_name shdr;
                const auto cleanup_if_failed{glapi.delete_shader.raii(shdr)};
                if(glapi.create_shader(*shdr_type).and_then(_1.move_to(shdr))) {
                    if(glapi.shader_source(shdr, _source.get())) {
                        if(glapi.compile_shader(shdr)) {
                            resource()._private_ref() = std::move(shdr);
                            mark_loaded();
                            return;
                        }
                    }
                }
            }
        }
        mark_error();
    }
}
//------------------------------------------------------------------------------
// gl_shader_resource
//------------------------------------------------------------------------------
auto gl_shader_resource::kind() const noexcept -> identifier {
    return "GLShader";
}
//------------------------------------------------------------------------------
auto gl_shader_resource::make_loader(
  main_ctx_parent parent,
  const shared_holder<loaded_resource_context>& context,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(context and context->gl_context()) {
        if(is_glsl_shader_locator(params.locator)) {
            return {
              hold<gl_shader_resource::_loader_glsl>,
              parent,
              *this,
              context,
              std::move(params)};
        }
        return {
          hold<gl_shader_resource::_loader_eagishdr>,
          parent,
          *this,
          context,
          std::move(params)};
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

