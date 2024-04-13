/// @example app/003_checker/main.cpp
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
class example_checker : public application {
public:
    example_checker(execution_context& ec, video_context& vc);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ec;
    video_context& _video;
    timeout _is_done{std::chrono::seconds(8)};

    oglplus::owned_program_name prog;
};
//------------------------------------------------------------------------------
example_checker::example_checker(execution_context& ec, video_context& vc)
  : _ec{ec}
  , _video{vc} {
    const auto& [gl, GL] = _video.gl_api();
    gl.clear_color(0.4F, 0.4F, 0.4F, 0.0F);

    // fragment shader
    auto fs_source = embed<"FragShader">("fragment.glsl");
    oglplus::owned_shader_name fs;
    gl.create_shader(GL.fragment_shader) >> fs;
    const auto cleanup_fs = gl.delete_shader.raii(fs);
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_source));
    gl.compile_shader(fs);

    // program
    gl.create_program() >> prog;
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    ec.connect_inputs().map_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_checker::on_video_resize() noexcept {
    const auto& [gl, GL] = _video.gl_api();

    gl.viewport[_video.surface_size()];

    const auto h = GL.double_(2);
    const auto w = h * GL.double_(_video.surface_aspect());

    gl.matrix_mode(GL.projection);
    gl.load_identity();
    gl.ortho(-w, +w, -h, +h, 0, 1);
}
//------------------------------------------------------------------------------
void example_checker::update() noexcept {
    const auto& [gl, GL] = _video.gl_api();

    gl.clear(GL.color_buffer_bit);

    gl.matrix_mode(GL.modelview);
    gl.load_identity();
    gl.rotate_f(degrees_(_ec.state().frame_time().value() * 90), 0, 0, 1);

    gl.begin(GL.quads);

    gl.tex_coord2i(0, 0);
    gl.color3f(1.0F, 1.0F, 1.0F);
    gl.vertex2i(-1, -1);

    gl.tex_coord2i(1, 0);
    gl.color3f(1.0F, 0.2F, 0.2F);
    gl.vertex2i(+1, -1);

    gl.tex_coord2i(1, 1);
    gl.color3f(0.2F, 1.0F, 0.2F);
    gl.vertex2i(+1, +1);

    gl.tex_coord2i(0, 1);
    gl.color3f(0.2F, 0.2F, 1.0F);
    gl.vertex2i(-1, +1);

    gl.end();

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_checker::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_program(std::move(prog));
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

        return gl.viewport and gl.clear_color and gl.clear and
               GL.color_buffer_bit and gl.load_identity and gl.ortho and
               gl.rotate_f and gl.begin and gl.end and gl.vertex2i and
               gl.color3f and gl.tex_coord2i and GL.modelview and
               GL.projection and GL.triangle_fan and GL.line_loop and
               gl.create_shader and gl.shader_source and gl.compile_shader and
               GL.fragment_shader and gl.create_program and gl.attach_shader and
               gl.link_program;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_checker>(ec);
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
