/// @example app/007_cube/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/camera.hpp>
#include <eagine/app/main.hpp>
#include <eagine/app_config.hpp>
#include <eagine/embed.hpp>
#include <eagine/oglplus/glsl/string_ref.hpp>
#include <eagine/oglplus/math/matrix.hpp>
#include <eagine/oglplus/math/primitives.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/cube.hpp>
#include <eagine/timeout.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class example_cube : public application {
public:
    example_cube(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{30}};

    std::vector<oglplus::shape_draw_operation> _ops;
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name normals;
    oglplus::owned_buffer_name indices;

    oglplus::owned_program_name prog;

    orbiting_camera camera;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
example_cube::example_cube(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // vertex shader
    auto vs_source = embed(EAGINE_ID(VertShader), "vertex.glsl");
    oglplus::owned_shader_name vs;
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
    gl.compile_shader(vs);

    // fragment shader
    auto fs_source = embed(EAGINE_ID(FragShader), "fragment.glsl");
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

    // geometry
    oglplus::shape_generator shape(
      glapi,
      shapes::unit_cube(
        shapes::vertex_attrib_kind::position |
        shapes::vertex_attrib_kind::normal));

    _ops.resize(std_size(shape.operation_count()));
    shape.instructions(glapi, cover(_ops));

    // vao
    gl.gen_vertex_arrays() >> vao;
    gl.bind_vertex_array(vao);

    // positions
    oglplus::vertex_attrib_location position_loc{0};
    gl.gen_buffers() >> positions;
    shape.attrib_setup(
      glapi,
      vao,
      positions,
      position_loc,
      eagine::shapes::vertex_attrib_kind::position,
      _ctx.buffer());
    gl.bind_attrib_location(prog, position_loc, "Position");

    // normals
    oglplus::vertex_attrib_location normal_loc{1};
    gl.gen_buffers() >> normals;
    shape.attrib_setup(
      glapi,
      vao,
      normals,
      normal_loc,
      eagine::shapes::vertex_attrib_kind::normal,
      _ctx.buffer());
    gl.bind_attrib_location(prog, normal_loc, "Normal");

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, _ctx.buffer());

    // uniform
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(1.1F)
      .set_orbit_max(3.5F)
      .set_fov(right_angle_());

    gl.clear_color(0.4F, 0.4F, 0.4F, 0.0F);
    gl.enable(GL.depth_test);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_cube::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport(_video.surface_size());
}
//------------------------------------------------------------------------------
void example_cube::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state);
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
    if(camera.has_changed()) {
        glapi.set_uniform(prog, camera_loc, camera.matrix(_video));
    }
    oglplus::draw_using_instructions(glapi, view(_ops));

    _video.commit();
}
//------------------------------------------------------------------------------
void example_cube::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(normals));
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
                    return {std::make_unique<example_cube>(ec, vc)};
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
} // namespace eagine::app
