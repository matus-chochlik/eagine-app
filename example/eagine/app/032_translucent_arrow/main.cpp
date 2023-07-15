/// @example app/032_translucent_arrow/main.cpp
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

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_arrow : public timeouting_application {
public:
    example_arrow(
      execution_context&,
      video_context&,
      const shared_holder<shapes::generator>&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example_arrow::_on_resource_loaded>(this);
    }

    video_context& _video;

    depth_program _depth_prog;
    draw_program _draw_prog;
    shape_geometry _shape;
    depth_texture _depth_tex;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
example_arrow::example_arrow(
  execution_context& ec,
  video_context& vc,
  const shared_holder<shapes::generator>& gen)
  : timeouting_application{ec, std::chrono::seconds{30}}
  , _video{vc}
  , _depth_prog{ec}
  , _draw_prog{ec} {
    _depth_prog.base_loaded.connect(_load_handler());
    _draw_prog.base_loaded.connect(_load_handler());

    _shape.init(gen, vc);
    _depth_tex.init(ec, vc);

    const auto sr = _shape.bounding_sphere().radius();
    _camera.set_near(sr * 0.1F)
      .set_far(sr * 5.0F)
      .set_orbit_min(sr * 2.0F)
      .set_orbit_max(sr * 3.0F)
      .set_fov(degrees_(80.F));
    _depth_prog.set_camera(vc, _camera);
    _draw_prog.set_camera(vc, _camera);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.clear_color(0.45F, 0.45F, 0.45F, 0.0F);
    gl.enable(GL.depth_test);
    gl.enable(GL.cull_face);
    gl.cull_face(GL.back);
}
//------------------------------------------------------------------------------
void example_arrow::_on_resource_loaded(
  const loaded_resource_base& loaded) noexcept {
    if(loaded.is(_depth_prog)) {
        _depth_prog.apply_input_bindings(_video, _shape);
    }
    if(loaded.is(_draw_prog)) {
        _draw_prog.apply_input_bindings(_video, _shape);
    }
}
//------------------------------------------------------------------------------
void example_arrow::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 5);
    }

    if(_depth_prog and _draw_prog) {
        const auto& glapi = _video.gl_api();
        auto& [gl, GL] = glapi;

        gl.clear_depth(0);
        gl.clear(GL.depth_buffer_bit);

        gl.enable(GL.depth_test);
        gl.depth_func(GL.greater);
        gl.enable(GL.cull_face);
        gl.cull_face(GL.front);
        _depth_prog.use(_video);
        _depth_prog.set_camera(_video, _camera);
        _shape.use(_video);
        _shape.draw(_video);

        _depth_tex.copy_from_fb(_video);

        gl.clear_depth(1);
        gl.clear(GL.color_buffer_bit | GL.depth_buffer_bit);
        gl.depth_func(GL.less);
        gl.cull_face(GL.back);
        _draw_prog.update(context(), _video);
        _draw_prog.set_camera(_video, _camera);
        _shape.draw(_video);
    } else {
        _depth_prog.load_if_needed(context());
        _draw_prog.load_if_needed(context());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example_arrow::clean_up() noexcept {
    _shape.clean_up(_video);
    _depth_tex.clean_up(_video);
    _depth_prog.clean_up(context());
    _draw_prog.clean_up(context());
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

        if(not _gen) {
            auto load_shape_data = [&]() {
                return valtree::from_json_text(
                  as_chars(embed<"ArrowJson">("arrow.json").unpack(ctx)), ctx);
            };
            _gen = shapes::from_value_tree(load_shape_data(), ctx);
        }
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
               GL.vertex_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return ec.video_ctx().and_then(
          [this, &ec](auto& vc) -> unique_holder<application> {
              vc.begin();
              if(vc.init_gl_api()) {
                  if(check_requirements(vc)) {
                      return {hold<example_arrow>, ec, vc, _gen};
                  }
              }
              return {};
          });
    }

private:
    shared_holder<shapes::generator> _gen;
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
