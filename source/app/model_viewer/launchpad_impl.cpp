/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app.model_viewer;

import eagine.core;
import eagine.oglplus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
//  Launchpad
//------------------------------------------------------------------------------
class model_viewer_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final;
    auto check_requirements(video_context& vc) -> bool final;

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final;
};
//------------------------------------------------------------------------------
auto model_viewer_launchpad::setup(main_ctx&, launch_options& opts) -> bool {
    opts.no_audio().require_input().require_video();
    return true;
}
//------------------------------------------------------------------------------
auto model_viewer_launchpad::launch(execution_context& ec, const launch_options&)
  -> unique_holder<application> {
    return launch_with_video<model_viewer>(ec);
}
//------------------------------------------------------------------------------
static auto model_viewer_requirement_checker() noexcept {
    return [](auto& gl, auto& GL) {
        return gl.enable and gl.disable and gl.clear_color and
               gl.clear_depth and gl.create_shader and gl.shader_source and
               gl.compile_shader and gl.create_program and gl.attach_shader and
               gl.link_program and gl.use_program and gl.gen_buffers and
               gl.bind_buffer and gl.buffer_data and gl.gen_vertex_arrays and
               gl.bind_vertex_array and gl.get_attrib_location and
               gl.vertex_attrib_pointer and gl.enable_vertex_attrib_array and
               gl.draw_arrays and gl.draw_elements and GL.compute_shader and
               gl.active_texture and gl.tex_image1d and gl.tex_image2d and
               gl.tex_image3d and GL.vertex_shader and GL.geometry_shader and
               GL.tess_control_shader and GL.tess_evaluation_shader and
               GL.fragment_shader and GL.texture_1d and GL.texture_1d_array and
               GL.texture_2d and GL.texture_2d_array and GL.texture_cube_map and
               GL.texture_3d;
    };
}
//------------------------------------------------------------------------------
auto model_viewer_launchpad::check_requirements(video_context& video) -> bool {
    return video.with_gl(model_viewer_requirement_checker()).or_false();
}
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> unique_holder<launchpad> {
    return {hold<model_viewer_launchpad>};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
