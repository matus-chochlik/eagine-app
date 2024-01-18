/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

export module eagine.app.tiling_viewer:program;

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;
import :resource;

namespace eagine::app {
//------------------------------------------------------------------------------
struct tiling_viewer_program_intf : tiling_viewer_resource_intf {
    virtual void apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings&) = 0;
    virtual void set_camera(video_context&, const mat4&) = 0;
    virtual void set_tiling_unit(
      video_context&,
      oglplus::texture_unit::value_type) = 0;
    virtual void set_transition_unit(
      video_context&,
      oglplus::texture_unit::value_type) = 0;
    virtual void set_tileset_unit(
      video_context&,
      oglplus::texture_unit::value_type) = 0;
};
using tiling_viewer_program_holder = unique_holder<tiling_viewer_program_intf>;
//------------------------------------------------------------------------------
class tiling_viewer_program
  : public tiling_viewer_resource_wrapper<tiling_viewer_program_intf> {
    using base = tiling_viewer_resource_wrapper<tiling_viewer_program_intf>;

public:
    using base::base;

    auto apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings& attrib_bindings)
      -> tiling_viewer_program&;

    auto set_camera(video_context&, orbiting_camera& camera)
      -> tiling_viewer_program&;

    auto set_tiling_unit(video_context&, oglplus::texture_unit::value_type)
      -> tiling_viewer_program&;

    auto set_transition_unit(video_context&, oglplus::texture_unit::value_type)
      -> tiling_viewer_program&;

    auto set_tileset_unit(video_context&, oglplus::texture_unit::value_type)
      -> tiling_viewer_program&;
};
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<tiling_viewer_program>,
  url,
  execution_context&,
  video_context&) -> tiling_viewer_program_holder;
//------------------------------------------------------------------------------
} // namespace eagine::app
