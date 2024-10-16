/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:resource_gl;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.identifier;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.progress;
import eagine.core.main_ctx;
import eagine.msgbus;
import eagine.shapes;
import eagine.oglplus;
import :resource_loader;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// gl_shader_include_resource
//------------------------------------------------------------------------------
export class gl_shader_include_resource final
  : public simple_resource<oglplus::shader_include> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      main_ctx_parent,
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    void clean_up(loaded_resource_context&) noexcept final;

    struct _loader;
};
//------------------------------------------------------------------------------
// gl_shader_includes_resource
//------------------------------------------------------------------------------
export class gl_shader_includes_resource final
  : public simple_resource<std::vector<oglplus::shader_include>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      main_ctx_parent,
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    void clean_up(loaded_resource_context&) noexcept final;

    struct _loader;
};
//------------------------------------------------------------------------------
// gl_shader_resource
//------------------------------------------------------------------------------
export class gl_shader_resource final
  : public simple_resource<oglplus::owned_shader_name> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      main_ctx_parent,
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    void clean_up(loaded_resource_context&) noexcept final;

    struct _loader;
};
//------------------------------------------------------------------------------
} // namespace eagine::app::exp

