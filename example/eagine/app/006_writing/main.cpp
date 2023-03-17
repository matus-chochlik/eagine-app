/// @example application/006_writing/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
class example_writing : public timeouting_application {
public:
    example_writing(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    video_context& _video;

    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::gl_types::sizei_type point_count{0};

    oglplus::owned_program_name prog;
};
//------------------------------------------------------------------------------
example_writing::example_writing(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // vertex shader
    auto vs_source = embed<"VertShader">("vertex.glsl");
    oglplus::owned_shader_name vs;
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
    gl.compile_shader(vs);

    // geometry shader
    auto gs_source = embed<"GeomShader">("geometry.glsl");
    oglplus::owned_shader_name gs;
    gl.create_shader(GL.geometry_shader) >> gs;
    auto cleanup_gs = gl.delete_shader.raii(gs);
    gl.shader_source(gs, oglplus::glsl_string_ref(gs_source));
    gl.compile_shader(gs);

    // fragment shader
    auto fs_source = embed<"FragShader">("fragment.glsl");
    oglplus::owned_shader_name fs;
    gl.create_shader(GL.fragment_shader) >> fs;
    auto cleanup_fs = gl.delete_shader.raii(fs);
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_source));
    gl.compile_shader(fs);

    // program
    gl.create_program() >> prog;
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, gs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    // geometry
    const std::array<oglplus::vec2, 34> control_points{
      {{-0.33F, +0.50F}, {-0.45F, +0.70F}, {-0.66F, +0.70F}, {-0.66F, +0.30F},
       {-0.66F, -0.20F}, {-0.35F, -0.15F}, {-0.30F, +0.05F}, {-0.20F, +0.50F},
       {-0.30F, +0.50F}, {-0.33F, +0.50F}, {-0.50F, +0.45F}, {-0.10F, +0.40F},
       {+0.10F, +0.55F}, {-0.20F, +0.40F}, {-0.30F, -0.10F}, {+0.00F, -0.10F},
       {+0.10F, -0.10F}, {+0.20F, -0.10F}, {+0.10F, +0.55F}, {+0.20F, +0.00F},
       {+0.30F, -0.70F}, {+0.00F, -0.75F}, {-0.40F, -0.75F}, {+0.00F, +0.00F},
       {+0.40F, +0.10F}, {+0.60F, +0.10F}, {+0.70F, +0.90F}, {+0.55F, +0.90F},
       {+0.35F, +0.90F}, {+0.10F, -0.10F}, {+0.55F, +0.00F}, {+0.90F, +0.10F},
       {+0.70F, +0.10F}, {+0.90F, +0.20F}}};

    math::bezier_curves<oglplus::vec2, float, 3> curve(view(control_points));
    std::vector<oglplus::vec2> curve_points;
    curve.approximate(curve_points, 20);
    std::vector<float> position_data;
    memory::flatten(view(curve_points), position_data);
    point_count = limit_cast<decltype(point_count)>(curve_points.size());

    // vao
    gl.gen_vertex_arrays() >> vao;
    gl.bind_vertex_array(vao);

    // positions
    gl.gen_buffers() >> positions;
    gl.bind_buffer(GL.array_buffer, positions);
    gl.buffer_data(GL.array_buffer, view(position_data), GL.static_draw);

    oglplus::vertex_attrib_location position_loc;
    gl.get_attrib_location(prog, "Position") >> position_loc;
    gl.vertex_attrib_pointer(position_loc, 2, GL.float_, GL.false_);
    gl.enable_vertex_attrib_array(position_loc);

    //
    gl.clear_color(0.4F, 0.4F, 0.4F, 0.0F);
    gl.disable(GL.depth_test);
    gl.disable(GL.cull_face);

    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_writing::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit);
    gl.draw_arrays(GL.line_strip, 0, point_count);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_writing::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(positions));
    gl.delete_vertex_arrays(std::move(vao));

    _video.end();
}
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.no_audio().require_input().require_video();
        return true;
    }

    auto check_requirements(video_context& vc) -> bool {
        const auto& [gl, GL] = vc.gl_api();

        return gl.disable and gl.clear_color and gl.create_shader and
               gl.shader_source and gl.compile_shader and gl.create_program and
               gl.attach_shader and gl.link_program and gl.use_program and
               gl.gen_buffers and gl.bind_buffer and gl.buffer_data and
               gl.gen_vertex_arrays and gl.bind_vertex_array and
               gl.get_attrib_location and gl.vertex_attrib_pointer and
               gl.enable_vertex_attrib_array and gl.draw_arrays and
               GL.vertex_shader and GL.geometry_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_writing>(ec, vc)};
                }
            }
        }
        return {};
    }
};
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> std::unique_ptr<launchpad> {
    return {std::make_unique<example_launchpad>()};
}
//------------------------------------------------------------------------------
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
} // namespace eagine::app

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
