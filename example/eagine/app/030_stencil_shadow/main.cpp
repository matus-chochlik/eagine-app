/// @example app/030_stencil_shadow/main.cpp
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
#include <eagine/shapes/adjacency.hpp>
#include <eagine/shapes/value_tree.hpp>
#include <eagine/timeout.hpp>
#include <eagine/value_tree/json.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class example_stencil_shadow : public application {
public:
    example_stencil_shadow(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{90}};

    std::vector<oglplus::shape_draw_operation> _ops;
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name normals;
    oglplus::owned_buffer_name wrap_coords;
    oglplus::owned_buffer_name indices;

    oglplus::owned_texture_name color_tex{};
    oglplus::owned_texture_name light_tex{};

    oglplus::owned_program_name draw_prog;
    oglplus::owned_program_name mask_prog;

    orbiting_camera camera;
    oglplus::uniform_location camera_draw_loc;
    oglplus::uniform_location camera_mask_loc;
    oglplus::uniform_location light_dir_draw_loc;
    oglplus::uniform_location light_dir_mask_loc;
    oglplus::uniform_location light_mult_loc;
};
//------------------------------------------------------------------------------
example_stencil_shadow::example_stencil_shadow(
  execution_context& ec,
  video_context& vc)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // programs
    const auto draw_prog_src{
      embed(EAGINE_ID(DrawProg), "stencil_shadow_draw.oglpprog")};
    gl.create_program() >> draw_prog;
    glapi.build_program(draw_prog, draw_prog_src.unpack(ec));

    const auto mask_prog_src{
      embed(EAGINE_ID(MaskProg), "stencil_shadow_mask.oglpprog")};
    gl.create_program() >> mask_prog;
    glapi.build_program(mask_prog, mask_prog_src.unpack(ec));

    // geometry
    const auto json_text =
      as_chars(embed(EAGINE_ID(ShapeJson), "wheelcart_1.json").unpack(ec));
    oglplus::shape_generator shape(
      glapi,
      shapes::add_triangle_adjacency(
        shapes::from_value_tree(
          valtree::from_json_text(json_text, ec.as_parent()), ec.as_parent()),
        ec.as_parent()));

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

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, _ctx.buffer());

    // color texture
    const auto color_tex_src{embed(EAGINE_ID(ColorTex), "wheelcart_1_color")};

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

    // light texture
    const auto light_tex_src{embed(EAGINE_ID(LightTex), "wheelcart_1_aoccl")};

    gl.gen_textures() >> light_tex;
    gl.active_texture(GL.texture0 + 1);
    gl.bind_texture(GL.texture_2d, light_tex);
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

    // uniforms
    gl.use_program(draw_prog);
    gl.get_uniform_location(draw_prog, "ColorTex") >> color_tex_loc;
    glapi.set_uniform(draw_prog, color_tex_loc, 0);
    gl.get_uniform_location(draw_prog, "LightTex") >> light_tex_loc;
    glapi.set_uniform(draw_prog, light_tex_loc, 1);
    gl.get_uniform_location(draw_prog, "LightMult") >> light_mult_loc;
    gl.get_uniform_location(draw_prog, "LightDir") >> light_dir_draw_loc;
    gl.get_uniform_location(draw_prog, "Camera") >> camera_draw_loc;

    gl.use_program(mask_prog);
    gl.get_uniform_location(mask_prog, "LightDir") >> light_dir_mask_loc;
    gl.get_uniform_location(mask_prog, "Camera") >> camera_mask_loc;

    // camera
    camera.set_pitch_max(degrees_(89.F))
      .set_pitch_min(degrees_(10.F))
      .set_orbit_max(6.5F)
      .set_orbit_min(3.5F)
      .set_target(0.F)
      .set_fov(degrees_(70))
      .set_near(0.01F)
      .set_far(100.0F);
    camera.idle_update(_ctx.state(), 11.F);

    gl.clear_color(0.35F, 0.35F, 0.35F, 1.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_stencil_shadow::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport(_video.surface_size());
}
//------------------------------------------------------------------------------
void example_stencil_shadow::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 11.F);
    }

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    const auto ft = state.frame_time().value();
    const auto light_angle{turns_(ft * 0.1F)};
    const oglplus::vec3 light_dir{
      10.F * cos(light_angle), 10.F, 10.F * sin(light_angle)};

    // first color pass (shadow)
    gl.color_mask(GL.true_, GL.true_, GL.true_, GL.true_);
    gl.depth_mask(GL.true_);
    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit | GL.stencil_buffer_bit);
    gl.disable(GL.stencil_test);

    gl.use_program(draw_prog);
    glapi.set_uniform(draw_prog, light_mult_loc, 0.7F);
    glapi.set_uniform(draw_prog, light_dir_draw_loc, light_dir);
    glapi.set_uniform(draw_prog, camera_draw_loc, camera.matrix(_video));
    gl.cull_face(GL.back);
    oglplus::draw_using_instructions(glapi, view(_ops));

    // stencil update pass
    gl.color_mask(GL.false_, GL.false_, GL.false_, GL.false_);
    gl.depth_mask(GL.false_);
    gl.clear(GL.depth_buffer_bit);
    gl.enable(GL.stencil_test);
    gl.stencil_func(GL.always, 0);
    gl.stencil_op_separate(GL.front, GL.keep, GL.keep, GL.incr);
    gl.stencil_op_separate(GL.back, GL.keep, GL.keep, GL.decr);

    gl.use_program(mask_prog);
    glapi.set_uniform(mask_prog, light_dir_mask_loc, light_dir);
    glapi.set_uniform(mask_prog, camera_mask_loc, camera.matrix(_video));
    gl.cull_face(GL.back);
    oglplus::draw_using_instructions(glapi, view(_ops));
    gl.cull_face(GL.front);
    oglplus::draw_using_instructions(glapi, view(_ops));

    // second color pass (light)
    gl.color_mask(GL.true_, GL.true_, GL.true_, GL.true_);
    gl.depth_mask(GL.true_);
    gl.clear(GL.depth_buffer_bit);
    gl.stencil_func(GL.equal, 0);
    gl.stencil_op(GL.keep, GL.keep, GL.keep);

    gl.use_program(draw_prog);
    glapi.set_uniform(draw_prog, light_mult_loc, 1.0F);
    glapi.set_uniform(draw_prog, light_dir_draw_loc, light_dir);
    glapi.set_uniform(draw_prog, camera_draw_loc, camera.matrix(_video));
    gl.cull_face(GL.back);
    oglplus::draw_using_instructions(glapi, view(_ops));

    _video.commit();
}
//------------------------------------------------------------------------------
void example_stencil_shadow::clean_up() noexcept {
    const auto& gl = _video.gl_api();

    gl.delete_textures(std::move(light_tex));
    gl.delete_textures(std::move(color_tex));
    gl.delete_program(std::move(mask_prog));
    gl.delete_program(std::move(draw_prog));
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
        opts.no_audio().require_input();
        opts.require_video().with_stencil();
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
                    return {std::make_unique<example_stencil_shadow>(ec, vc)};
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
