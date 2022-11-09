/// @example app/009_json_shape/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
class example_shape : public timeouting_application {
public:
    example_shape(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    video_context& _video;

    std::vector<oglplus::shape_draw_operation> _ops;
    oglplus::owned_vertex_array_name vao;

    oglplus::owned_buffer_name positions;
    oglplus::owned_buffer_name colors;
    oglplus::owned_buffer_name occlusion;
    oglplus::owned_buffer_name indices;

    oglplus::owned_program_name prog;

    orbiting_camera camera;
    oglplus::uniform_location camera_loc;
};
//------------------------------------------------------------------------------
example_shape::example_shape(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    // vertex shader
    auto vs_source = embed<"VertShader">("vertex.glsl");
    oglplus::owned_shader_name vs;
    gl.create_shader(GL.vertex_shader) >> vs;
    auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.shader_source(
      vs, oglplus::glsl_string_ref(vs_source.unpack(vc.parent())));
    gl.compile_shader(vs);

    // fragment shader
    auto fs_source = embed<"FragShader">("fragment.glsl");
    oglplus::owned_shader_name fs;
    gl.create_shader(GL.fragment_shader) >> fs;
    auto cleanup_fs = gl.delete_shader.raii(fs);
    gl.shader_source(
      fs, oglplus::glsl_string_ref(fs_source.unpack(vc.parent())));
    gl.compile_shader(fs);

    // program
    gl.create_program() >> prog;
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    // geometry
    auto get_json_source = [&]() -> embedded_resource {
        const auto& args{ec.main_context().args()};
        if(args.find("--stool")) {
            return embed<"Stool">("stool.json");
        }
        if(args.find("--crate")) {
            return embed<"Crate">("crate.json");
        }
        if(args.find("--guitar")) {
            return embed<"Guitar">("guitar.json");
        }
        return embed<"ShapeJson">("shape.json");
    };
    oglplus::shape_generator shape(
      glapi,
      shapes::from_value_tree(
        valtree::from_json_text(
          as_chars(get_json_source().unpack(vc.parent())), ec.main_context()),
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
      context().buffer());
    gl.bind_attrib_location(prog, position_loc, "Position");

    // colors
    oglplus::vertex_attrib_location color_loc{1};
    gl.gen_buffers() >> colors;
    shape.attrib_setup(
      glapi,
      vao,
      colors,
      color_loc,
      eagine::shapes::vertex_attrib_kind::color,
      context().buffer());
    gl.bind_attrib_location(prog, color_loc, "Color");

    // occlusion
    oglplus::vertex_attrib_location occl_loc{2};
    gl.gen_buffers() >> occlusion;
    shape.attrib_setup(
      glapi,
      vao,
      occlusion,
      occl_loc,
      eagine::shapes::vertex_attrib_kind::occlusion,
      context().buffer());
    gl.bind_attrib_location(prog, occl_loc, "Occl");

    // indices
    gl.gen_buffers() >> indices;
    shape.index_setup(glapi, indices, context().buffer());

    // uniform
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    // camera
    const auto bs = shape.bounding_sphere();
    const auto sr = bs.radius();
    gl.get_uniform_location(prog, "Camera") >> camera_loc;
    camera.set_target(bs.center())
      .set_near(sr * 0.1F)
      .set_far(sr * 10.0F)
      .set_orbit_min(sr * 2.0F)
      .set_orbit_max(sr * 4.0F)
      .set_fov(right_angle_());

    gl.clear_color(0.45F, 0.45F, 0.45F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_shape::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
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
void example_shape::clean_up() noexcept {
    auto& gl = _video.gl_api();

    gl.delete_program(std::move(prog));
    gl.delete_buffers(std::move(indices));
    gl.delete_buffers(std::move(colors));
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
                    return {std::make_unique<example_shape>(ec, vc)};
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

auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
