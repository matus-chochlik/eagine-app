/// @example app/018_uv_map_hydrant/main.cpp
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
    oglplus::owned_buffer_name tangents;
    oglplus::owned_buffer_name bitangents;
    oglplus::owned_buffer_name wrap_coords;
    oglplus::owned_buffer_name indices;

    oglplus::owned_texture_name tex{};

    oglplus::owned_program_name prog;

    orbiting_camera camera;
    oglplus::uniform_location camera_loc;
    oglplus::uniform_location light_dir_loc;
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
      oglplus::glsl_string_ref(embed("VertShader", "vertex.glsl").unpack(ec)));
    glapi.add_shader(
      prog,
      GL.fragment_shader,
      oglplus::glsl_string_ref(
        embed("FragShader", "fragment.glsl").unpack(ec)));
    gl.link_program(prog);
    gl.use_program(prog);

    // geometry
    auto json_text = as_chars(embed("ShapeJson", "hydrant_1.json").unpack(ec));
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

    // tangents
    oglplus::vertex_attrib_location tangent_loc{2};
    gl.gen_buffers() >> tangents;
    shape.attrib_setup(
      glapi,
      vao,
      tangents,
      tangent_loc,
      eagine::shapes::vertex_attrib_kind::tangent,
      _ctx.buffer());
    gl.bind_attrib_location(prog, tangent_loc, "Tangent");

    // bitangents
    oglplus::vertex_attrib_location bitangent_loc{3};
    gl.gen_buffers() >> bitangents;
    shape.attrib_setup(
      glapi,
      vao,
      bitangents,
      bitangent_loc,
      eagine::shapes::vertex_attrib_kind::bitangent,
      _ctx.buffer());
    gl.bind_attrib_location(prog, bitangent_loc, "Bitangent");

    // wrap_coords
    oglplus::vertex_attrib_location wrap_coord_loc{4};
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

    // textures
    const auto tex_src{embed("HydrantTex", "hydrant")};

    gl.gen_textures() >> tex;
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_2d_array, tex);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d_array, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_s, GL.clamp_to_border);
    gl.tex_parameter_i(
      GL.texture_2d_array, GL.texture_wrap_t, GL.clamp_to_border);
    glapi.spec_tex_image3d(
      GL.texture_2d_array,
      0,
      0,
      oglplus::texture_image_block(tex_src.unpack(ec)));
    oglplus::uniform_location tex_loc;
    gl.get_uniform_location(prog, "Tex") >> tex_loc;
    glapi.set_uniform(prog, tex_loc, 0);

    // camera
    const auto bs = shape.bounding_sphere();
    const auto sr = bs.radius();
    camera.set_pitch_max(degrees_(89.F))
      .set_pitch_min(degrees_(-1.F))
      .set_target(bs.center())
      .set_near(sr * 0.01F)
      .set_far(sr * 5.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 2.9F)
      .set_fov(degrees_(70));

    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    gl.get_uniform_location(prog, "LightDir") >> light_dir_loc;

    gl.clear_color(0.35F, 0.35F, 0.35F, 1.0F);
    gl.enable(GL.depth_test);

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

    const auto rad = radians_(state.frame_time().value());
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
    if(camera.has_changed()) {
        glapi.set_uniform(prog, camera_loc, camera.matrix(_video));
    }
    glapi.set_uniform(
      prog,
      light_dir_loc,
      math::normalized(
        oglplus::vec3(cos(rad) * 6, sin(rad) * 7, sin(rad * 0.618F) * 8)));

    oglplus::draw_using_instructions(glapi, view(_ops));

    _video.commit();
}
//------------------------------------------------------------------------------
void example_uv_map::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.clean_up(std::move(tex));
    gl.clean_up(std::move(prog));
    gl.clean_up(std::move(indices));
    gl.clean_up(std::move(wrap_coords));
    gl.clean_up(std::move(bitangents));
    gl.clean_up(std::move(tangents));
    gl.clean_up(std::move(normals));
    gl.clean_up(std::move(positions));
    gl.clean_up(std::move(vao));

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
