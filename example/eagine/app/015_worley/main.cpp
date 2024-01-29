/// @example app/015_worley/main.cpp
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

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_worley : public application {
public:
    example_worley(execution_context& ctx, video_context&);

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

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds(30)};

    screen_geometry screen;
    worley_program voi_prog;
    random_texture rand_tex;

    float ofs_x_dir{1.F};
    float ofs_y_dir{1.F};
    float offset_x{-0.5F};
    float offset_y{0.0F};
    float scale_dir{1.F};
    float scale{1.0F};
    float aspect{1.0F};
    bool dampen_motion{false};
    bool is_dragging{false};

    auto motion_adjust() const noexcept {
        return dampen_motion ? 0.2 : 1.0;
    }

    static constexpr const float min_scale{1.F};
    static constexpr const float max_scale{100.0F};
};
//------------------------------------------------------------------------------
example_worley::example_worley(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {

    screen.init(ec, vc);
    voi_prog.init(ec, vc);
    rand_tex.init(ec, vc);

    const auto& [gl, GL] = _video.gl_api();

    gl.clear_color(0.4F, 0.4F, 0.4F, 0.0F);
    gl.disable(GL.depth_test);

    ec.connect_inputs()
      .connect_input(
        {"Motion", "Dampening"},
        make_callable_ref<&example_worley::dampening>(this))
      .connect_input(
        {"Pointer", "Dragging"},
        make_callable_ref<&example_worley::dragging>(this))
      .connect_input(
        {"View", "Zoom"}, make_callable_ref<&example_worley::zoom>(this))
      .connect_input(
        {"View", "PanX"}, make_callable_ref<&example_worley::pan_x>(this))
      .connect_input(
        {"View", "PanY"}, make_callable_ref<&example_worley::pan_y>(this))
      .map_inputs()
      .map_key({"Motion", "Dampening"}, {"LeftCtrl"})
      .map_left_mouse_button({"Pointer", "Dragging"})
      .map_wheel_scroll_y({"View", "Zoom"}, input_setup().relative())
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
      .switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_worley::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();

    aspect = _video.surface_aspect();
    gl.viewport[_video.surface_size()];
    gl.uniform2f(voi_prog.scale_loc, scale * aspect, scale);
}
//------------------------------------------------------------------------------
void example_worley::dampening(const input& i) noexcept {
    dampen_motion = bool(i);
}
//------------------------------------------------------------------------------
void example_worley::dragging(const input& i) noexcept {
    is_dragging = bool(i);
}
//------------------------------------------------------------------------------
void example_worley::zoom(const input& i) noexcept {
    scale *= float(std::pow(2, -i.get() * motion_adjust()));
    if(scale < min_scale) {
        scale = min_scale;
    }
    if(scale > max_scale) {
        scale = max_scale;
    }

    const auto& gl = _video.gl_api();
    gl.uniform2f(voi_prog.scale_loc, scale * aspect, scale);
}
//------------------------------------------------------------------------------
void example_worley::pan_x(const input& i) noexcept {
    offset_x -= float(i.get() * scale * motion_adjust());

    const auto& gl = _video.gl_api();
    gl.uniform2f(voi_prog.offset_loc, offset_x, offset_y);
}
//------------------------------------------------------------------------------
void example_worley::pan_y(const input& i) noexcept {
    offset_y -= float(i.get() * scale * motion_adjust());

    const auto& gl = _video.gl_api();
    gl.uniform2f(voi_prog.offset_loc, offset_x, offset_y);
}
//------------------------------------------------------------------------------
void example_worley::update() noexcept {
    auto& state = _ctx.state();
    const auto& [gl, GL] = _video.gl_api();

    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_time() > std::chrono::seconds(1)) {
        const float s = value(state.frame_duration()) * 60;

        scale *= std::pow(1.F + 0.05F * s, scale_dir);
        if(scale < min_scale) {
            scale_dir *= -1.F;
            ofs_x_dir *= -1.F;
            ofs_y_dir *= ofs_x_dir;
            scale = min_scale;
        }
        if(scale > max_scale) {
            scale_dir *= -1.F;
            ofs_y_dir *= -1.F;
            ofs_x_dir *= ofs_y_dir;
            scale = max_scale;
        }

        offset_x += ofs_x_dir * s * scale / 30;
        offset_y += ofs_y_dir * s * scale / 30;

        gl.uniform2f(voi_prog.offset_loc, offset_x, offset_y);
        gl.uniform2f(voi_prog.scale_loc, scale * aspect, scale);
    }

    gl.clear(GL.color_buffer_bit);
    gl.draw_arrays(GL.triangle_strip, 0, 4);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_worley::clean_up() noexcept {
    screen.init(_ctx, _video);
    voi_prog.init(_ctx, _video);
    rand_tex.init(_ctx, _video);
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
        return launch_with_video<example_worley>(ec);
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
