/// @example app/018_uv_map_barrel/main.cpp
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
#include <eagine/shapes/value_tree.hpp>
#include <eagine/timeout.hpp>
#include <eagine/value_tree/json.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class example_uv_map : public application {
public:
    example_uv_map(execution_context&, video_context&);

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
    oglplus::owned_buffer_name wrap_coords;
    oglplus::owned_buffer_name indices;

    oglplus::owned_texture_name color_tex{};
    oglplus::owned_texture_name aoccl_tex{};
    oglplus::owned_texture_name rough_tex{};

    oglplus::owned_program_name prog;

    orbiting_camera camera;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
example_uv_map::example_uv_map(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // program
    gl.create_program() >> prog;
    glapi.add_shader(
      prog,
      GL.vertex_shader,
      oglplus::glsl_string_ref(embed<"VertShader">("vertex.glsl").unpack(ec)));
    glapi.add_shader(
      prog,
      GL.fragment_shader,
      oglplus::glsl_string_ref(
        embed<"FragShader">("fragment.glsl").unpack(ec)));
    gl.link_program(prog);
    gl.use_program(prog);

    // geometry
    auto json_text = as_chars(embed<"ShapeJson">("barrel_1.json").unpack(ec));
    oglplus::shape_generator shape(
      glapi,
      shapes::from_value_tree(
        valtree::from_json_text(json_text, ec.as_parent()), ec.as_parent()));

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

    // wrap_coords
    oglplus::vertex_attrib_location wrap_coord_loc{2};
    gl.gen_buffers() >> wrap_coords;
    shape.attrib_setup(
      glapi,
      vao,
      wrap_coords,
      wrap_coord_loc,
      eagine::shapes::vertex_attrib_kind::wrap_coord,
      _ctx.buffer());
    gl.bind_attrib_location(prog, wrap_coord_loc, "WrapCoord");

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, _ctx.buffer());

    // color texture
    const auto color_tex_src{embed<"ColorTex">("barrel_1_color")};

    gl.gen_textures() >> color_tex;
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_2d, color_tex);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.clamp_to_border);
    glapi.spec_tex_image2d(
      GL.texture_2d,
      0,
      0,
      oglplus::texture_image_block(color_tex_src.unpack(ec)));
    oglplus::uniform_location color_tex_loc;
    gl.get_uniform_location(prog, "ColorTex") >> color_tex_loc;
    glapi.set_uniform(prog, color_tex_loc, 0);

    // light texture
    const auto light_tex_src{embed<"LightTex">("barrel_1_aoccl")};

    gl.gen_textures() >> aoccl_tex;
    gl.active_texture(GL.texture0 + 1);
    gl.bind_texture(GL.texture_2d, aoccl_tex);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.clamp_to_border);
    glapi.spec_tex_image2d(
      GL.texture_2d,
      0,
      0,
      oglplus::texture_image_block(light_tex_src.unpack(ec)));
    oglplus::uniform_location light_tex_loc;
    gl.get_uniform_location(prog, "LightTex") >> light_tex_loc;
    glapi.set_uniform(prog, light_tex_loc, 1);

    // roughness texture
    const auto rough_tex_src{embed<"RoughTex">("barrel_1_rough")};

    gl.gen_textures() >> rough_tex;
    gl.active_texture(GL.texture0 + 2);
    gl.bind_texture(GL.texture_2d, rough_tex);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.clamp_to_border);
    glapi.spec_tex_image2d(
      GL.texture_2d,
      0,
      0,
      oglplus::texture_image_block(rough_tex_src.unpack(ec)));
    oglplus::uniform_location rough_tex_loc;
    gl.get_uniform_location(prog, "RoughTex") >> rough_tex_loc;
    glapi.set_uniform(prog, rough_tex_loc, 2);

    // camera
    const auto bs = shape.bounding_sphere();
    const auto sr = bs.radius();
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    camera.set_target(bs.center())
      .set_near(sr * 0.1F)
      .set_far(sr * 5.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 2.4F)
      .set_fov(degrees_(70));

    gl.clear_color(0.35F, 0.35F, 0.35F, 1.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_uv_map::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_uv_map::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 5.F);
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
void example_uv_map::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_textures(std::move(rough_tex));
    gl.delete_textures(std::move(aoccl_tex));
    gl.delete_textures(std::move(color_tex));
    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(wrap_coords));
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
               gl.tex_image2d && GL.vertex_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_uv_map>(ec, vc)};
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
