/// @example app/011_mandelbrot/main.cpp
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
class example_mandelbrot : public application {
public:
    example_mandelbrot(execution_context& ctx, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

    void dampening(const input&) noexcept;
    void dragging(const input&) noexcept;
    void zoom(const input&) noexcept;
    void pan_x(const input&) noexcept;
    void pan_y(const input&) noexcept;
    void change_gradient(const input&) noexcept;
    void set_gradient_image() noexcept;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds(20)};

    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name coords;

    oglplus::owned_shader_name vs;
    oglplus::owned_shader_name fs;

    oglplus::owned_program_name prog;

    oglplus::uniform_location offset_loc;
    oglplus::uniform_location scale_loc;

    oglplus::owned_texture_name gradient;

    float offset_x{-0.5F};
    float offset_y{0.0F};
    float scale{1.0F};
    float aspect{1.0F};
    trigger_released reset_triggered;
    bool dampen_motion{false};
    bool is_dragging{false};

    auto motion_adjust() const noexcept {
        return dampen_motion ? 0.2 : 1.0;
    }

    static constexpr const float min_scale{0.00001F};
    static constexpr const float max_scale{10.0F};
};
//------------------------------------------------------------------------------
example_mandelbrot::example_mandelbrot(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& [gl, GL] = _video.gl_api();

    gl.clear_color(0.4F, 0.4F, 0.4F, 0.0F);

    // vertex shader
    auto vs_source = embed<"VertShader">("vertex.glsl");
    gl.create_shader(GL.vertex_shader) >> vs;
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
    gl.compile_shader(vs);

    // fragment shader
    auto fs_source = embed<"FragShader">("fragment.glsl");
    gl.create_shader(GL.fragment_shader) >> fs;
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_source));
    gl.compile_shader(fs);

    // program
    gl.create_program() >> prog;
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    gl.get_uniform_location(prog, "Offset") >> offset_loc;
    gl.get_uniform_location(prog, "Scale") >> scale_loc;
    gl.uniform2f(offset_loc, offset_x, offset_y);
    gl.uniform2f(scale_loc, aspect * scale, scale);

    // vao
    gl.gen_vertex_arrays() >> vao;
    gl.bind_vertex_array(vao);

    // positions
    const auto position_data =
      GL.float_.array(-1.0F, -1.0F, -1.0F, 1.0F, 1.0F, -1.0F, 1.0F, 1.0F);

    gl.gen_buffers() >> positions;
    gl.bind_buffer(GL.array_buffer, positions);
    gl.buffer_data(GL.array_buffer, view(position_data), GL.static_draw);
    oglplus::vertex_attrib_location position_loc;
    gl.get_attrib_location(prog, "Position") >> position_loc;

    gl.vertex_attrib_pointer(position_loc, 2, GL.float_, GL.false_);
    gl.enable_vertex_attrib_array(position_loc);

    const auto coord_data =
      GL.float_.array(-1.0F, -1.0F, -1.0F, 1.0F, 1.0F, -1.0F, 1.0F, 1.0F);

    gl.gen_buffers() >> coords;
    gl.bind_buffer(GL.array_buffer, coords);
    gl.buffer_data(GL.array_buffer, view(coord_data), GL.static_draw);
    oglplus::vertex_attrib_location coord_loc;
    gl.get_attrib_location(prog, "Coord") >> coord_loc;

    gl.vertex_attrib_pointer(coord_loc, 2, GL.float_, GL.false_);
    gl.enable_vertex_attrib_array(coord_loc);

    // gradient texture
    gl.gen_textures() >> gradient;
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_1d, gradient);
    gl.tex_parameter_i(GL.texture_1d, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_1d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_1d, GL.texture_wrap_s, GL.repeat);
    set_gradient_image();

    gl.disable(GL.depth_test);

    ec.connect_inputs()
      .add_ui_button({"Button", "ChngGrad"}, "Change gradient")
      .connect_input(
        {"Motion", "Dampening"},
        make_callable_ref<&example_mandelbrot::dampening>(this))
      .connect_input(
        {"Cursor", "Dragging"},
        make_callable_ref<&example_mandelbrot::dragging>(this))
      .connect_input(
        {"View", "Zoom"}, make_callable_ref<&example_mandelbrot::zoom>(this))
      .connect_input(
        {"View", "PanX"}, make_callable_ref<&example_mandelbrot::pan_x>(this))
      .connect_input(
        {"View", "PanY"}, make_callable_ref<&example_mandelbrot::pan_y>(this))
      .connect_input(
        {"Example", "ChngGrad"},
        make_callable_ref<&example_mandelbrot::change_gradient>(this))
      .map_inputs()
      .map_key({"Motion", "Dampening"}, {"LeftCtrl"})
      .map_key({"Cursor", "Dragging"}, {"Button0"})
      .map_wheel_scroll_y({"View", "Zoom"})
      .map_key(
        {"View", "Zoom"}, {"KpPlus"}, input_setup().trigger().multiply(0.25))
      .map_key(
        {"View", "Zoom"},
        {"KpMinus"},
        input_setup().trigger().multiply(0.25).invert())
      .map_key(
        {"View", "PanX"}, {"Left"}, input_setup().trigger().multiply(0.25))
      .map_key(
        {"View", "PanX"},
        {"Right"},
        input_setup().trigger().multiply(0.25).invert())
      .map_key(
        {"View", "PanY"}, {"Down"}, input_setup().trigger().multiply(0.25))
      .map_key(
        {"View", "PanY"},
        {"Up"},
        input_setup().trigger().multiply(0.25).invert())
      .map_cursor_motion_x(
        {"View", "PanX"},
        input_setup().relative().multiply(2).only_if(is_dragging))
      .map_cursor_motion_y(
        {"View", "PanY"},
        input_setup().relative().multiply(2).only_if(is_dragging))
      .map_key({"Example", "ChngGrad"}, {"G"})
      .map_ui_input(
        {"Example", "ChngGrad"},
        {"Button", "ChngGrad"},
        input_setup().trigger())
      .switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_mandelbrot::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();

    aspect = _video.surface_aspect();
    gl.viewport[_video.surface_size()];
    gl.uniform2f(scale_loc, scale * aspect, scale);
}
//------------------------------------------------------------------------------
void example_mandelbrot::dampening(const input& i) noexcept {
    dampen_motion = bool(i);
}
//------------------------------------------------------------------------------
void example_mandelbrot::dragging(const input& i) noexcept {
    is_dragging = bool(i);
}
//------------------------------------------------------------------------------
void example_mandelbrot::zoom(const input& i) noexcept {
    scale *= float(std::pow(2, -i.get() * motion_adjust()));
    if(scale < min_scale) {
        scale = min_scale;
    }
    if(scale > max_scale) {
        scale = max_scale;
    }

    const auto& gl = _video.gl_api();
    gl.uniform2f(scale_loc, scale * aspect, scale);
}
//------------------------------------------------------------------------------
void example_mandelbrot::pan_x(const input& i) noexcept {
    offset_x -= float(i.get() * scale * motion_adjust());

    const auto& gl = _video.gl_api();
    gl.uniform2f(offset_loc, offset_x, offset_y);
}
//------------------------------------------------------------------------------
void example_mandelbrot::pan_y(const input& i) noexcept {
    offset_y -= float(i.get() * scale * motion_adjust());

    const auto& gl = _video.gl_api();
    gl.uniform2f(offset_loc, offset_x, offset_y);
}
//------------------------------------------------------------------------------
void example_mandelbrot::change_gradient(const input& i) noexcept {
    if(reset_triggered(i)) {
        set_gradient_image();
    }
}
//------------------------------------------------------------------------------
void example_mandelbrot::set_gradient_image() noexcept {
    const auto& [gl, GL] = _video.gl_api();
    auto gradient_data = GL.float_.array(size_constant<8 * 3>{});
    _ctx.random_uniform_01(cover(gradient_data));

    gl.tex_image1d(
      GL.texture_1d,
      0,
      GL.rgb,
      8,
      0,
      GL.rgb,
      GL.float_,
      as_bytes(view(gradient_data)));
}
//------------------------------------------------------------------------------
void example_mandelbrot::update() noexcept {
    auto& state = _ctx.state();
    const auto& [gl, GL] = _video.gl_api();

    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_time() > std::chrono::seconds(1)) {
        const float s = value(state.frame_duration()) * 60;
        const float dest_offset_x = -0.525929F;
        const float dest_offset_y = -0.668547F;
        const float c = 0.02F * s;

        offset_x = c * dest_offset_x + (1 - c) * offset_x;
        offset_y = c * dest_offset_y + (1 - c) * offset_y;

        scale *= (1 - 0.01F * s);
        if(scale < min_scale) {
            scale = min_scale;
        }

        gl.uniform2f(offset_loc, offset_x, offset_y);
        gl.uniform2f(scale_loc, scale * aspect, scale);
    }

    gl.clear(GL.color_buffer_bit);
    gl.draw_arrays(GL.triangle_strip, 0, 4);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_mandelbrot::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_shader(std::move(vs));
    gl.delete_shader(std::move(fs));
    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(positions));
    gl.delete_buffers(std::move(coords));
    gl.delete_vertex_arrays(std::move(vao));
    gl.delete_textures(std::move(gradient));

    _video.end();
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
        return launch_with_video<example_mandelbrot>(ec);
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
