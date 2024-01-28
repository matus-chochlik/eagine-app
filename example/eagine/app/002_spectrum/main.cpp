/// @example app/002_spectrum/main.cpp
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
class example_spectrum : public application {
public:
    example_spectrum(execution_context& ec, video_context& vc)
      : _ec{ec}
      , _video{vc} {
        ec.connect_inputs().map_inputs().switch_input_mapping();
    }

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final {
        const auto [width, height] = _video.surface_size();
        const auto& [gl, GL] = _video.gl_api();

        gl.viewport(width, height);

        const auto h = GL.double_(2);
        const auto w = h * GL.double_(_video.surface_aspect());

        gl.matrix_mode(GL.projection);
        gl.load_identity();
        gl.ortho(-w, +w, -h, +h, 0, 1);
    }

    void update() noexcept final {

        _bg.clear(_video);
        const auto& [gl, GL] = _video.gl_api();

        gl.matrix_mode(GL.modelview);
        gl.load_identity();
        gl.rotate_f(degrees_(_ec.state().frame_time().value() * 90), 0, 0, 1);

        gl.begin(GL.triangle_fan);

        gl.color3f(0.5F, 0.5F, 0.5F);
        gl.vertex2f(0.000F, 0.000F);

        gl.color3f(1.0F, 0.0F, 0.0F);
        gl.vertex2f(0.000F, 1.000F);

        gl.color3f(1.0F, 0.7F, 0.0F);
        gl.vertex2f(0.707F, 0.707F);

        gl.color3f(0.7F, 1.0F, 0.0F);
        gl.vertex2f(1.000F, 0.000F);

        gl.color3f(0.0F, 1.0F, 0.0F);
        gl.vertex2f(0.707F, -0.707F);

        gl.color3f(0.0F, 1.0F, 0.7F);
        gl.vertex2f(0.000F, -1.000F);

        gl.color3f(0.0F, 0.7F, 1.0F);
        gl.vertex2f(-0.707F, -0.707F);

        gl.color3f(0.0F, 0.0F, 1.0F);
        gl.vertex2f(-1.000F, 0.000F);

        gl.color3f(0.7F, 0.0F, 0.7F);
        gl.vertex2f(-0.707F, 0.707F);

        gl.color3f(1.0F, 0.0F, 0.0F);
        gl.vertex2f(0.000F, 1.000F);

        gl.end();

        gl.begin(GL.line_loop);

        gl.color3f(0, 0, 0);

        gl.vertex2f(0.000F, 1.000F);
        gl.vertex2f(0.707F, 0.707F);
        gl.vertex2f(1.000F, 0.000F);
        gl.vertex2f(0.707F, -0.707F);
        gl.vertex2f(0.000F, -1.000F);
        gl.vertex2f(-0.707F, -0.707F);
        gl.vertex2f(-1.000F, 0.000F);
        gl.vertex2f(-0.707F, 0.707F);
        gl.vertex2f(0.000F, 1.000F);

        gl.end();

        _video.commit();
    }

    void clean_up() noexcept final {
        _video.end();
    }

private:
    execution_context& _ec;
    video_context& _video;
    background_color _bg;
    timeout _is_done{std::chrono::seconds{8}};
};
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.no_audio().require_input().require_video();
        return true;
    }

    auto check_requirements(video_context& vc) -> bool final {
        const auto& [gl, GL] = vc.gl_api();

        return gl.viewport and gl.clear_color and gl.clear and
               GL.color_buffer_bit and gl.load_identity and gl.ortho and
               gl.rotate_f and gl.begin and gl.end and gl.vertex2f and
               gl.color3f and GL.modelview and GL.projection and
               GL.triangle_fan and GL.line_loop;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_spectrum>(ec);
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
