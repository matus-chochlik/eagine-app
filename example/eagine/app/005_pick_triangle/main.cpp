/// @example application/005_pick_triangle/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
class example_picking : public timeouting_application {
public:
    example_picking(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

    void motion_x(const input&) noexcept;
    void motion_y(const input&) noexcept;

private:
    video_context& _video;

    oglplus::triangle tri{
      {-0.2F, 0.5F, 0.0F},
      {-0.7F, -0.6F, 0.0F},
      {0.6F, 0.2F, 0.0F}};

    float x_pos{0.F};
    float y_pos{0.F};
    float hl_value{0.F};
    bool is_inside{false};
    bool has_moved{false};

    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;

    oglplus::owned_program_name prog;

    oglplus::uniform_location highlight_loc;
};
//------------------------------------------------------------------------------
example_picking::example_picking(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{15}}
  , _video{vc} {

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.4F, 0.4F, 0.4F, 0.0F);

    // vertex shader
    auto vs_source = embed<"VertShader">("vertex.glsl");
    oglplus::owned_shader_name vs;
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
    gl.compile_shader(vs);

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
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    // vao
    gl.gen_vertex_arrays() >> vao;
    gl.bind_vertex_array(vao);

    // positions
    const auto position_data = GL.float_.array(
      tri.a().x(),
      tri.a().y(),
      tri.b().x(),
      tri.b().y(),
      tri.c().x(),
      tri.c().y());

    gl.gen_buffers() >> positions;
    gl.bind_buffer(GL.array_buffer, positions);
    gl.buffer_data(GL.array_buffer, view(position_data), GL.static_draw);
    oglplus::vertex_attrib_location position_loc;
    gl.get_attrib_location(prog, "Position") >> position_loc;

    gl.vertex_attrib_pointer(position_loc, 2, GL.float_, GL.false_);
    gl.enable_vertex_attrib_array(position_loc);

    // uniform
    gl.get_uniform_location(prog, "Highlight") >> highlight_loc;
    glapi.set_uniform(prog, highlight_loc, hl_value);

    gl.disable(GL.depth_test);

    ec.connect_inputs()
      .map_inputs()
      .connect_input(
        {"Example", "MotionX"},
        make_callable_ref<&example_picking::motion_x>(this))
      .map_cursor_motion_x(
        {"Example", "MotionX"}, input_setup().multiply(2).absolute_norm())
      .connect_input(
        {"Example", "MotionY"},
        make_callable_ref<&example_picking::motion_y>(this))
      .map_cursor_motion_y(
        {"Example", "MotionY"}, input_setup().multiply(2).absolute_norm())
      .switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_picking::update() noexcept {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    if(has_moved) {
        oglplus::line ray(
          oglplus::point3(x_pos, y_pos, 1.F), oglplus::vec3(0.F, 0.F, -1.F));

        is_inside = bool(math::line_triangle_intersection(ray, tri));
        has_moved = false;
        reset_timeout();
    }

    const auto dt = context().state().frame_duration().value();
    if(is_inside) {
        hl_value = math::minimum(hl_value + dt * 3.0F, 1.F);
    } else {
        hl_value = math::maximum(hl_value - dt * 1.0F, 0.F);
    }
    glapi.set_uniform(prog, highlight_loc, hl_value);

    gl.clear(GL.color_buffer_bit);
    gl.draw_arrays(GL.triangles, 0, 3);

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_picking::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(positions));
    gl.delete_vertex_arrays(std::move(vao));

    _video.end();
}
//------------------------------------------------------------------------------
void example_picking::motion_x(const input& i) noexcept {
    x_pos = float(i.get());
    has_moved = true;
}
//------------------------------------------------------------------------------
void example_picking::motion_y(const input& i) noexcept {
    y_pos = float(i.get());
    has_moved = true;
}
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.no_audio().require_input().require_video();
        return true;
    }

    auto check_requirements(video_context& vc) -> bool final {
        const auto& [gl, GL] = vc.gl_api();

        return gl.disable and gl.clear_color and gl.create_shader and
               gl.shader_source and gl.compile_shader and gl.create_program and
               gl.attach_shader and gl.link_program and gl.use_program and
               gl.gen_buffers and gl.bind_buffer and gl.buffer_data and
               gl.gen_vertex_arrays and gl.bind_vertex_array and
               gl.get_attrib_location and gl.vertex_attrib_pointer and
               gl.enable_vertex_attrib_array and gl.draw_arrays and
               GL.vertex_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_picking>(ec);
    }
};
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> unique_holder<launchpad> {
    return {hold<example_launchpad>};
}
//------------------------------------------------------------------------------
auto example_main(main_ctx& ctx) -> int {
    return default_main(ctx, establish(ctx));
}
} // namespace eagine::app

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
