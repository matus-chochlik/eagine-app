/// @example app/015_worley/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#if EAGINE_APP_MODULE
import eagine.core;
import eagine.oglplus;
import eagine.app;
import <cmath>;
#else
#include <eagine/app/main.hpp>
#include <eagine/app_config.hpp>
#include <eagine/embed.hpp>
#include <eagine/timeout.hpp>

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/oglplus/glsl/string_ref.hpp>
#include <eagine/oglplus/math/primitives.hpp>
#include <eagine/oglplus/math/vector.hpp>
#endif

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
        {"Cursor", "Dragging"},
        make_callable_ref<&example_worley::dragging>(this))
      .connect_input(
        {"View", "Zoom"}, make_callable_ref<&example_worley::zoom>(this))
      .connect_input(
        {"View", "PanX"}, make_callable_ref<&example_worley::pan_x>(this))
      .connect_input(
        {"View", "PanY"}, make_callable_ref<&example_worley::pan_y>(this))
      .map_inputs()
      .map_input(
        {"Motion", "Dampening"},
        {"Keyboard", "LeftCtrl"},
        input_setup().trigger())
      .map_input(
        {"Cursor", "Dragging"}, {"Cursor", "Button0"}, input_setup().trigger())
      .map_input(
        {"View", "Zoom"}, {"Wheel", "ScrollY"}, input_setup().relative())
      .map_input(
        {"View", "Zoom"},
        {"Keyboard", "KpPlus"},
        input_setup().trigger().multiply(0.25))
      .map_input(
        {"View", "Zoom"},
        {"Keyboard", "KpMinus"},
        input_setup().trigger().multiply(0.25).invert())
      .map_input(
        {"View", "PanX"},
        {"Keyboard", "Left"},
        input_setup().trigger().multiply(0.25))
      .map_input(
        {"View", "PanX"},
        {"Keyboard", "Right"},
        input_setup().trigger().multiply(0.25).invert())
      .map_input(
        {"View", "PanY"},
        {"Keyboard", "Down"},
        input_setup().trigger().multiply(0.25))
      .map_input(
        {"View", "PanY"},
        {"Keyboard", "Up"},
        input_setup().trigger().multiply(0.25).invert())
      .map_input(
        {"View", "PanX"},
        {"Cursor", "MotionX"},
        input_setup().relative().multiply(2).only_if(is_dragging))
      .map_input(
        {"View", "PanY"},
        {"Cursor", "MotionY"},
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

    auto check_requirements(video_context& vc) -> bool {
        const auto& [gl, GL] = vc.gl_api();

        return gl.disable && gl.clear_color && gl.create_shader &&
               gl.shader_source && gl.compile_shader && gl.create_program &&
               gl.attach_shader && gl.link_program && gl.use_program &&
               gl.gen_buffers && gl.bind_buffer && gl.buffer_data &&
               gl.gen_vertex_arrays && gl.bind_vertex_array &&
               gl.get_attrib_location && gl.vertex_attrib_pointer &&
               gl.enable_vertex_attrib_array && gl.draw_arrays &&
               GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_worley>(ec, vc)};
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

#if EAGINE_APP_MODULE
auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
#endif
