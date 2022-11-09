/// @example app/034_shape_tessellation/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
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

    video_context& _video;
    background_icosahedron _bg;

    tess_program _prog;
    tess_geometry _geom;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
auto example_tess::_shape_id() -> identifier {
    if(const auto arg{context().main_context().args().find("--shape").next()}) {
        if(const identifier res_id{arg.get()}) {
            if(
              (res_id == identifier{"icosphere"}) ||
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
    return context().loader().embedded_resource_locator("json", id);
}
//------------------------------------------------------------------------------
example_tess::example_tess(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{60}}
  , _video{vc}
  , _bg{_video, {0.0F, 0.0F, 0.0F, 1.F}, {0.25F, 0.25F, 0.25F, 0.0F}, 1.F}
  , _prog{ec}
  , _geom{_shape_url(_shape_id()), ec} {
    _prog.base_loaded.connect(_load_handler());
    _geom.base_loaded.connect(_load_handler());

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();

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
          .set_fov(degrees_(45.F));
    }
    if(_prog && _geom) {
        _prog.apply_input_bindings(_video, _geom);
    }
    reset_timeout();
}
//------------------------------------------------------------------------------
void example_tess::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 17.F);
    }

    _bg.clear(_video, _camera);

    if(_prog && _geom) {
        const auto t = state.frame_time().value();
        _prog.use(_video);
        _prog.set_projection(_video, _camera);
        _prog.set_factor(_video, math::sine_wave01(t / 7.F));
        _geom.use(_video);
        _geom.draw(_video);
    } else {
        _prog.load_if_needed(context());
        _geom.load_if_needed(context());
    }

    _video.commit();
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

    auto check_requirements(video_context& vc) -> bool {
        const auto& [gl, GL] = vc.gl_api();

        return gl.disable && gl.clear_color && gl.create_shader &&
               gl.shader_source && gl.compile_shader && gl.create_program &&
               gl.attach_shader && gl.link_program && gl.use_program &&
               gl.gen_buffers && gl.bind_buffer && gl.buffer_data &&
               gl.gen_vertex_arrays && gl.bind_vertex_array &&
               gl.get_attrib_location && gl.vertex_attrib_pointer &&
               gl.enable_vertex_attrib_array && gl.draw_arrays &&
               GL.vertex_shader && GL.tess_control_shader &&
               GL.tess_evaluation_shader && GL.vertex_shader &&
               GL.geometry_shader && GL.fragment_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example_tess>(ec, vc)};
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
