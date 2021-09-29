/// @example app/013_tiling/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/animated_value.hpp>
#include <eagine/app/camera.hpp>
#include <eagine/app/main.hpp>
#include <eagine/app_config.hpp>
#include <eagine/embed.hpp>
#include <eagine/oglplus/glsl/string_ref.hpp>
#include <eagine/oglplus/math/matrix.hpp>
#include <eagine/oglplus/math/primitives.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/round_cube.hpp>
#include <eagine/timeout.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class example_tiling : public application {
public:
    example_tiling(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{60}};

    std::vector<oglplus::shape_draw_operation> _ops;
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name normals;
    oglplus::owned_buffer_name coords;
    oglplus::owned_buffer_name indices;

    oglplus::owned_texture_name tiling_tex;
    oglplus::owned_texture_name tileset_tex;

    oglplus::owned_program_name prog;

    animated_value<std::tuple<radians_t<float>, radians_t<float>, float>, float>
      geo_coord;
    enum class animation_status { zoom_in, zoom_out, relocate };
    animation_status camera_status{animation_status::zoom_in};

    orbiting_camera camera;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
example_tiling::example_tiling(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // vertex shader
    auto vs_source = embed(EAGINE_ID(VertShader), "vertex.glsl");
    oglplus::owned_shader_name vs;
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source.unpack(ec)));
    gl.compile_shader(vs);

    // fragment shader
    auto fs_source = embed(EAGINE_ID(FragShader), "fragment.glsl");
    oglplus::owned_shader_name fs;
    gl.create_shader(GL.fragment_shader) >> fs;
    auto cleanup_fs = gl.delete_shader.raii(fs);
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_source.unpack(ec)));
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
      shapes::unit_round_cube(
        shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::normal |
          shapes::vertex_attrib_kind::face_coord,
        32));

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

    // coords
    oglplus::vertex_attrib_location coord_loc{2};
    gl.gen_buffers() >> coords;
    shape.attrib_setup(
      glapi,
      vao,
      coords,
      coord_loc,
      eagine::shapes::vertex_attrib_kind::face_coord,
      _ctx.buffer());
    gl.bind_attrib_location(prog, coord_loc, "TexCoord");

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, _ctx.buffer());

    // tiling texture
    const auto tiling_tex_src{embed(EAGINE_ID(TilingTex), "tiles_r4_s256_a6")};
    const auto tiling_img{
      oglplus::texture_image_block(tiling_tex_src.unpack(ec))};

    gl.gen_textures() >> tiling_tex;
    gl.active_texture(GL.texture0 + 0);
    gl.bind_texture(GL.texture_2d_array, tiling_tex);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_min_filter, GL.nearest);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.nearest);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_s, GL.repeat);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_t, GL.repeat);
    glapi.spec_tex_image3d(GL.texture_2d_array, 0, 0, tiling_img);
    oglplus::uniform_location tiling_tex_loc;
    gl.get_uniform_location(prog, "TilingTex") >> tiling_tex_loc;
    glapi.set_uniform(prog, tiling_tex_loc, 0);

    // tile-set texture
    const auto tileset_tex_src{embed(EAGINE_ID(TilesetTex), "tileset_nodes16")};
    const auto tileset_img{
      oglplus::texture_image_block(tileset_tex_src.unpack(ec))};

    gl.gen_textures() >> tileset_tex;
    gl.active_texture(GL.texture0 + 1);
    gl.bind_texture(GL.texture_2d_array, tileset_tex);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_min_filter, GL.linear_mipmap_linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_s, GL.repeat);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_t, GL.repeat);
    glapi.spec_tex_image3d(GL.texture_2d_array, 0, 0, tileset_img);
    gl.generate_mipmap(GL.texture_2d_array);
    oglplus::uniform_location tileset_tex_loc;
    gl.get_uniform_location(prog, "TilesetTex") >> tileset_tex_loc;
    glapi.set_uniform(prog, tileset_tex_loc, 1);

    // uniform
    gl.get_uniform_location(prog, "Camera") >> camera_loc;

    const auto [azimuth, elevation, orbit] =
      geo_coord.update(_ctx.state().frame_duration().value()).get();
    camera.set_near(0.01F)
      .set_far(50.F)
      .set_orbit_min(1.05F)
      .set_orbit_max(2.0F)
      .set_fov(degrees_(60))
      .set_azimuth(azimuth)
      .set_elevation(elevation)
      .set_orbit_factor(orbit);

    gl.clear_color(0.1F, 0.1F, 0.1F, 0.0F);
    gl.enable(GL.depth_test);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_tiling::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport(_video.surface_size());
}
//------------------------------------------------------------------------------
void example_tiling::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        const auto [azimuth, elevation, orbit] =
          geo_coord.update(state.frame_duration().value()).get();
        if(geo_coord.is_done()) {
            switch(camera_status) {
                case animation_status::relocate:
                    geo_coord.set({azimuth, elevation, 0.F}, 2.F);
                    camera_status = animation_status::zoom_in;
                    break;
                case animation_status::zoom_in:
                    geo_coord.set({azimuth, elevation, 1.F}, 2.F);
                    camera_status = animation_status::zoom_out;
                    break;
                case animation_status::zoom_out:
                    geo_coord.set(
                      {turns_(_ctx.random_uniform_01()),
                       right_angles_(_ctx.random_uniform_11()),
                       1.F},
                      3.F);
                    camera_status = animation_status::relocate;
                    break;
            }
        }
        camera.mark_changed()
          .set_azimuth(azimuth)
          .set_elevation(elevation)
          .set_orbit_factor(orbit);
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
void example_tiling::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_textures(std::move(tileset_tex));
    gl.delete_textures(std::move(tiling_tex));
    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(coords));
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
                    return {std::make_unique<example_tiling>(ec, vc)};
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
