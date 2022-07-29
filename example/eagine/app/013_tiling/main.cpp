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
#include <eagine/oglplus/shapes/geometry.hpp>
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
    void change_tileset(const input&) noexcept;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{60}};

    oglplus::geometry cube;

    oglplus::owned_texture_name tiling_tex;
    oglplus::texture_name_array<4> tileset_texs;
    std::array<int, 4> tileset_tex_units{1, 2, 3, 4};
    std::size_t tileset_tex_idx{0U};

    oglplus::owned_program_name prog;

    animated_value<std::tuple<radians_t<float>, radians_t<float>, float>, float>
      geo_coord;
    enum class animation_status { zoom_in, zoom_out, relocate };
    animation_status camera_status{animation_status::zoom_in};

    orbiting_camera camera;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location tileset_tex_loc;
};
//------------------------------------------------------------------------------
example_tiling::example_tiling(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // geometry
    memory::buffer temp;
    oglplus::shape_generator shape(
      glapi,
      shapes::unit_round_cube(
        shapes::vertex_attrib_kind::position |
          shapes::vertex_attrib_kind::normal |
          shapes::vertex_attrib_kind::face_coord,
        32));
    oglplus::vertex_attrib_bindings bindings{shape};
    cube = oglplus::geometry{glapi, shape, bindings, _ctx.buffer()};
    cube.use(glapi);

    // vertex shader
    auto vs_source = embed("VertShader", "vertex.glsl");
    oglplus::owned_shader_name vs;
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source.unpack(ec)));
    gl.compile_shader(vs);

    // fragment shader
    auto fs_source = embed("FragShader", "fragment.glsl");
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

    gl.bind_attrib_location(
      prog,
      bindings.location(shapes::vertex_attrib_kind::position),
      "Position");
    gl.bind_attrib_location(
      prog, bindings.location(shapes::vertex_attrib_kind::normal), "Normal");
    gl.bind_attrib_location(
      prog,
      bindings.location(shapes::vertex_attrib_kind::face_coord),
      "TexCoord");

    // tiling texture
    const auto tiling_tex_src{embed("TilingTex", "tiles_r4_s1024_a6")};
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

    // tile-set textures
    const std::array<embedded_resource, 4> tileset_srcs{
      embed("Nodes512", "tileset_nodes16"),
      embed("Blocks512", "tileset_blocks16"),
      embed("Conncts512", "tileset_connections16"),
      embed("Thicket512", "tileset_thicket16")};

    gl.gen_textures(tileset_texs.raw_handles());
    for(const auto idx : integer_range(tileset_srcs.size())) {
        oglplus::texture_image_block tileset_img{tileset_srcs[idx].unpack(ec)};
        gl.active_texture(GL.texture0 + tileset_tex_units[idx]);
        gl.bind_texture(GL.texture_2d_array, tileset_texs[span_size(idx)]);
        gl.tex_parameter_i(
          GL.texture_2d_array, GL.texture_min_filter, GL.linear_mipmap_linear);
        gl.tex_parameter_i(
          GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
        gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_s, GL.repeat);
        gl.tex_parameter_i(GL.texture_2d_array, GL.texture_wrap_t, GL.repeat);
        glapi.spec_tex_image3d(GL.texture_2d_array, 0, 0, tileset_img);
        gl.generate_mipmap(GL.texture_2d_array);
    }
    gl.get_uniform_location(prog, "TilesetTex") >> tileset_tex_loc;

    // uniform
    gl.get_uniform_location(prog, "Camera") >> camera_loc;

    const auto [azimuth, elevation, orbit] =
      geo_coord.update(_ctx.state().frame_duration().value()).get();
    camera.set_near(0.01F)
      .set_far(50.F)
      .set_orbit_min(1.02F)
      .set_orbit_max(2.2F)
      .set_fov(degrees_(55))
      .set_azimuth(azimuth)
      .set_elevation(elevation)
      .set_orbit_factor(orbit);

    gl.clear_color(0.1F, 0.1F, 0.1F, 0.0F);
    gl.enable(GL.depth_test);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.connect_inputs()
      .add_ui_button("Change tile-set", EAGINE_MSG_ID(GUI, ChngTilset))
      .connect_input(
        EAGINE_MSG_ID(Example, ChngTilset),
        EAGINE_THIS_MEM_FUNC_REF(change_tileset))
      .map_inputs()
      .map_input(
        EAGINE_MSG_ID(Example, ChngTilset),
        EAGINE_MSG_ID(Keyboard, T),
        input_setup().trigger())
      .map_input(
        EAGINE_MSG_ID(Example, ChngTilset),
        EAGINE_MSG_ID(GUI, ChngTilset),
        input_setup().trigger())
      .switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_tiling::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_tiling::change_tileset(const input& i) noexcept {
    if(!i) {
        tileset_tex_idx = (tileset_tex_idx + 1) % tileset_tex_units.size();
    }
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
    glapi.set_uniform(
      prog, tileset_tex_loc, tileset_tex_units[tileset_tex_idx]);
    glapi.set_uniform(prog, camera_loc, camera.matrix(_video));

    cube.draw(glapi);

    _video.commit();
}
//------------------------------------------------------------------------------
void example_tiling::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.clean_up(tileset_texs);
    gl.clean_up(std::move(tiling_tex));
    gl.clean_up(std::move(prog));
    cube.clean_up(gl);

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
