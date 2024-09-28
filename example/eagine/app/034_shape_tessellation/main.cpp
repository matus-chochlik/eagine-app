/// @example app/034_shape_tessellation/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

import eagine.core;
import eagine.oglplus;
import eagine.app;

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example_tess : public timeouting_application {
public:
    example_tess(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

private:
    auto _shape_id() -> identifier;
    auto _shape_url(identifier) -> url;

    void _on_loaded(const loaded_resource_base&) noexcept;
    auto _load_handler() noexcept {
        return make_callable_ref<&example_tess::_on_loaded>(this);
    }

    void _change_factor(const input&) noexcept;

    video_context& _video;
    background_icosahedron _bg;

    tess_program _prog;
    tess_geometry _geom;
    pending_resource_requests _other;

    orbiting_camera _camera;
    float _tess_factor{0.5F};
};
//------------------------------------------------------------------------------
auto example_tess::_shape_id() -> identifier {
    if(const auto arg{context().main_context().args().find("--shape").next()}) {
        if(const identifier res_id{arg.get()}) {
            if(
              (res_id == identifier{"icosphere"}) or
              (res_id == identifier{"torus"}) or
              context().loader().has_embedded_resource(res_id)) {
                return res_id;
            }
        }
    }
    return {"octasphere"};
}
//------------------------------------------------------------------------------
auto example_tess::_shape_url(identifier id) -> url {
    if(id == identifier{"icosphere"}) {
        return url{
          "shape:///unit_icosahedron"
          "?to_patches=true"
          "+pivot_pivot=true"
          "+vertex_pivot=true"
          "+position=true"
          "+opposite_length=true"
          "+face_area=true"};
    }
    if(id == identifier{"torus"}) {
        return url{
          "shape:///unit_torus"
          "?to_patches=true"
          "+pivot_pivot=true"
          "+vertex_pivot=true"
          "+position=true"
          "+opposite_length=true"
          "+face_area=true"};
    }
    return context().loader().embedded_resource_locator("json", id);
}
//------------------------------------------------------------------------------
example_tess::example_tess(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{60}}
  , _video{vc}
  , _bg{_video, {0.0F, 0.0F, 0.0F, 1.F}, {0.25F, 0.25F, 0.25F, 0.0F}, 1.F}
  , _prog{ec}
  , _geom{_shape_url(_shape_id()), ec}
  , _other{ec} {
    _prog.load_event.connect(_load_handler());
    _geom.load_event.connect(_load_handler());
    _other.add(
      ec.loader().request_input_setup({.locator = url{"json:///Inputs"}}, ec));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().connect_input(
      {"Example", "SetFactor"},
      make_callable_ref<&example_tess::_change_factor>(this));

    const auto& glapi = _video.gl_api();
    const auto& [gl, GL] = glapi;

    gl.enable(GL.depth_test);
}
//------------------------------------------------------------------------------
void example_tess::_on_loaded(const loaded_resource_base& loaded) noexcept {
    if(loaded.is(_geom)) {
        const auto bs{_geom.bounding_sphere()};
        const auto sr{bs.radius()};
        _camera.set_target(bs.center())
          .set_near(0.01F * sr)
          .set_far(100.F * sr)
          .set_orbit_min(1.1F * sr)
          .set_orbit_max(3.0F * sr)
          .set_fov(degrees_(30.F));
    }
    if(_prog and _geom) {
        _prog.apply_input_bindings(_video, _geom);
    }
    reset_timeout();
}
//------------------------------------------------------------------------------
void example_tess::_change_factor(const input& i) noexcept {
    _tess_factor = i.get();
}
//------------------------------------------------------------------------------
void example_tess::update() noexcept {
    auto& state = context().state();
    const auto t = state.frame_time().value();

    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 17.F);
        _tess_factor = math::sine_wave01(t / 7.F);
    }

    _bg.clear(_video, _camera);

    if(_prog and _geom and _other) {
        _prog.use(_video);
        _prog.set_projection(_video, _camera);
        _prog.set_factor(_video, _tess_factor);
        _geom.use(_video);
        _geom.draw(_video);
    } else {
        _prog.load_if_needed(context());
        _geom.load_if_needed(
          context(),
          oglplus::vertex_attrib_bindings{
            shapes::vertex_attrib_kind::pivot_pivot,
            shapes::vertex_attrib_kind::vertex_pivot,
            shapes::vertex_attrib_kind::position,
            shapes::vertex_attrib_kind::opposite_length,
            shapes::vertex_attrib_kind::face_area},
          0);
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example_tess::clean_up() noexcept {
    _geom.clean_up(context());
    _prog.clean_up(context());
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
               GL.vertex_shader and GL.tess_control_shader and
               GL.tess_evaluation_shader and GL.vertex_shader and
               GL.geometry_shader and GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example_tess>(ec);
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
