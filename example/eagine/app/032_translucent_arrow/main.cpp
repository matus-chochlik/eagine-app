/// @example app/032_translucent_arrow/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#if EAGINE_APP_MODULE
import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;
#else
#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/camera.hpp>
#include <eagine/app/main.hpp>
#include <eagine/cleanup_group.hpp>
#include <eagine/embed.hpp>
#include <eagine/oglplus/math/matrix.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/torus.hpp>
#include <eagine/shapes/twisted_torus.hpp>
#include <eagine/shapes/value_tree.hpp>
#include <eagine/timeout.hpp>
#include <eagine/value_tree/json.hpp>
#endif

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_arrow : public application {
public:
    example_arrow(
      execution_context&,
      video_context&,
      const std::shared_ptr<shapes::generator>&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;

private:
    execution_context& _ctx;
    video_context& _video;
    timeout _is_done{std::chrono::seconds{30}};

    orbiting_camera camera;

    depth_program depth_prog;
    draw_program draw_prog;
    shape_geometry shape;
    depth_texture depth_tex;
};
//------------------------------------------------------------------------------
example_arrow::example_arrow(
  execution_context& ec,
  video_context& vc,
  const std::shared_ptr<shapes::generator>& gen)
  : _ctx{ec}
  , _video{vc} {
    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    shape.init(gen, vc);
    vc.clean_up_later(shape);
    depth_tex.init(ec, vc);
    vc.clean_up_later(depth_tex);

    depth_prog.init(vc);
    vc.clean_up_later(depth_prog);
    depth_prog.bind_position_location(vc, shape.position_loc());

    draw_prog.init(vc);
    vc.clean_up_later(draw_prog);
    draw_prog.bind_position_location(vc, shape.position_loc());
    draw_prog.bind_normal_location(vc, shape.normal_loc());
    draw_prog.set_depth_texture(vc, depth_tex.texture_unit());

    // camera
    const auto sr = shape.bounding_sphere().radius();
    camera.set_near(sr * 0.1F)
      .set_far(sr * 3.0F)
      .set_orbit_min(sr * 1.2F)
      .set_orbit_max(sr * 1.7F)
      .set_fov(degrees_(80.F));
    depth_prog.set_camera(vc, camera);
    draw_prog.set_camera(vc, camera);

    gl.clear_color(0.45F, 0.45F, 0.45F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);

    camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example_arrow::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example_arrow::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        camera.idle_update(state, 5);
    }

    const auto& glapi = _video.gl_api();
    auto& [gl, GL] = glapi;

    gl.clear_depth(0);
    gl.clear(GL.depth_buffer_bit);

    gl.enable(GL.depth_test);
    gl.depth_func(GL.greater);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.front);
    depth_prog.use(_video);
    depth_prog.set_camera(_video, camera);
    shape.use(_video);
    shape.draw(_video);

    depth_tex.copy_from_fb(_video);

    gl.clear_depth(1);
    gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
    gl.depth_func(GL.less);
    gl.cull_face(GL.back);
    draw_prog.update(_ctx, _video);
    draw_prog.set_camera(_video, camera);
    shape.draw(_video);

    _video.commit();
}
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx& ctx, launch_options& opts) -> bool final {
        opts.no_audio().require_input().require_video();

        if(ctx.args().find("--monkey")) {
            const auto json_text =
              as_chars(embed<"MonkeyJson">("monkey.json").unpack(ctx));
            _gen = shapes::from_value_tree(
              valtree::from_json_text(json_text, ctx), ctx);
        } else if(ctx.args().find("--twisted-torus")) {
            _gen = shapes::unit_twisted_torus(
              shapes::vertex_attrib_kind::position |
                shapes::vertex_attrib_kind::normal,
              6,
              48,
              4,
              0.5F);
        } else if(ctx.args().find("--torus")) {
            _gen = shapes::unit_torus(
              shapes::vertex_attrib_kind::position |
              shapes::vertex_attrib_kind::normal);
        }

        if(!_gen) {
            auto load_shape_data = [&]() {
                return valtree::from_json_text(
                  as_chars(embed<"ArrowJson">("arrow.json").unpack(ctx)), ctx);
            };
            _gen = shapes::from_value_tree(load_shape_data(), ctx);
        }
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
                    return {std::make_unique<example_arrow>(ec, vc, _gen)};
                }
            }
        }
        return {};
    }

private:
    std::shared_ptr<shapes::generator> _gen;
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

#if EAGINE_APP_MODULE
auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::app::example_main);
}
#endif
