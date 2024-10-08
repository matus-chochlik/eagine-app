/// @example app/012_atomic_counters/main.cpp
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
class example_atomics : public timeouting_application {
public:
    example_atomics(execution_context& ctx, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    video_context& _video;

    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name counters;

    oglplus::owned_program_name prog;
};
//------------------------------------------------------------------------------
example_atomics::example_atomics(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{20}}
  , _video{vc} {
    const auto& [gl, GL] = _video.gl_api();

    // vertex shader
    oglplus::owned_shader_name vs;
    auto vs_cleanup = gl.delete_shader.raii(vs);
    auto vs_source = embed<"VertShader">("vertex.glsl");
    gl.create_shader(GL.vertex_shader) >> vs;
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source.unpack(ec)));
    gl.compile_shader(vs);

    // geometry shader
    oglplus::owned_shader_name gs;
    auto gs_cleanup = gl.delete_shader.raii(gs);
    auto gs_source = embed<"GeomShader">("geometry.glsl");
    gl.create_shader(GL.geometry_shader) >> gs;
    gl.shader_source(gs, oglplus::glsl_string_ref(gs_source.unpack(ec)));
    gl.compile_shader(gs);

    // fragment shader
    oglplus::owned_shader_name fs;
    auto fs_cleanup = gl.delete_shader.raii(fs);
    auto fs_source = embed<"FragShader">("fragment.glsl");
    gl.create_shader(GL.fragment_shader) >> fs;
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_source.unpack(ec)));
    gl.compile_shader(fs);

    // program
    gl.create_program() >> prog;
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, gs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

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

    // counters
    const auto counter_values = GL.unsigned_int_.array(0U, 0U, 0U);

    gl.gen_buffers() >> counters;
    gl.bind_buffer(GL.atomic_counter_buffer, counters);
    gl.bind_buffer_base(GL.atomic_counter_buffer, 0, counters);
    gl.buffer_data(
      GL.atomic_counter_buffer, view(counter_values), GL.dynamic_draw);

    gl.disable(GL.depth_test);

    ec.connect_inputs().map_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_atomics::update() noexcept {
    auto& state = context().state();
    const auto& [gl, GL] = _video.gl_api();

    if(state.is_active()) {
        reset_timeout();
    }

    gl.clear(GL.color_buffer_bit);
    gl.draw_arrays(GL.triangle_strip, 0, 4);

    const auto counter_values = GL.unsigned_int_.array(0U, 0U, 0U);
    gl.buffer_data(
      GL.atomic_counter_buffer, view(counter_values), GL.dynamic_draw);

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_atomics::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(counters));
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

    auto check_requirements(video_context& vc) -> bool final {
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
      -> unique_holder<application> final {
        return launch_with_video<example_atomics>(ec);
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
