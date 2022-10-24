/// @example app/040_metaballs/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "main.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
example::example(execution_context& ec, video_context& vc)
  : _ctx{ec}
  , _video{vc}
  , _bg{_video, {0.5F, 0.5F, 0.5F, 1.F}, {0.25F, 0.25F, 0.25F, 0.0F}, 1.F}
  , _mball_prog{*this}
  , _field_prog{*this}
  , _srfce_prog{*this} {
    _mball_prog.loaded.connect(_load_handler());
    _field_prog.loaded.connect(_load_handler());
    _srfce_prog.loaded.connect(_load_handler());

    _volume.init(*this);

    _camera.set_near(0.1F)
      .set_far(50.F)
      .set_orbit_min(7.0F)
      .set_orbit_max(8.0F)
      .set_fov(degrees_(75.F));

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
}
//------------------------------------------------------------------------------
void example::on_video_resize() noexcept {
    const auto& gl = _video.gl_api();
    gl.viewport[_video.surface_size()];
}
//------------------------------------------------------------------------------
void example::_on_loaded(const gl_program_resource::load_info& loaded) noexcept {
    if(loaded.resource.is(_mball_prog)) {
        loaded.shader_storage_block_binding(
          "MetaballBlock", _volume.metaballs_binding());
    } else if(loaded.resource.is(_field_prog)) {
        loaded.set_uniform("PlaneCount", _volume.plane_count());
        loaded.shader_storage_block_binding(
          "FieldBlock", _volume.field_binding());
        loaded.shader_storage_block_binding(
          "MetaballBlock", _volume.metaballs_binding());
    } else if(loaded.resource.is(_srfce_prog)) {
        loaded.set_uniform("PlaneCount", _volume.plane_count());
        loaded.set_uniform("DivCount", _volume.div_count());
        loaded.shader_storage_block_binding(
          "FieldBlock", _volume.field_binding());
        loaded.shader_storage_block_binding(
          "ConfigsBlock", _volume.configs_binding());
        _srfce_prog.bind_corner_location(*this, _volume.corner_loc());
    }
}
//------------------------------------------------------------------------------
void example::update() noexcept {
    auto& state = _ctx.state();
    if(state.is_active()) {
        _is_done.reset();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 7.F);
    }

    if(_mball_prog && _field_prog && _srfce_prog) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        _mball_prog.use(ctx());
        gl.dispatch_compute(1, 1, 1);

        _field_prog.use(ctx());
        _volume.compute(*this);

        _bg.clear(_video, _camera);

        gl.enable(GL.depth_test);
        gl.enable(GL.cull_face);

        _srfce_prog.use(ctx());
        _srfce_prog.prepare_frame(*this);
        _volume.draw(*this);
    } else {
        _mball_prog.update(ctx());
        _field_prog.update(ctx());
        _srfce_prog.update(ctx());
    }

    _video.commit();
}
//------------------------------------------------------------------------------
void example::clean_up() noexcept {
    _bg.clean_up(_video);
    _cleanup.clear();
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
               GL.vertex_shader && GL.tess_control_shader &&
               GL.tess_evaluation_shader && GL.vertex_shader &&
               GL.geometry_shader && GL.fragment_shader && GL.compute_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        if(auto opt_vc{ec.video_ctx()}) {
            auto& vc = extract(opt_vc);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    return {std::make_unique<example>(ec, vc)};
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
