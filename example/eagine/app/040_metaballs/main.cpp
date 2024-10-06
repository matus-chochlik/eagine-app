/// @example app/040_metaballs/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#include "main.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
example::example(execution_context& ec, video_context& vc)
  : timeouting_application{ec, std::chrono::seconds{60}}
  , _load_progress{ec.progress(), "Loading resources"}
  , _video{vc}
  , _bg{_video, {0.5F, 0.5F, 0.5F, 1.F}, {0.25F, 0.25F, 0.25F, 0.0F}, 1.F}
  , _mball_prog{*this}
  , _field_prog{*this}
  , _srfce_prog{*this}
  , _other{context()} {
    _other.add(ec.old_loader().request_camera_parameters(
      {.locator = url{"json:///Camera"}}, _camera));

    _mball_prog.loaded.connect(_load_handler());
    _field_prog.loaded.connect(_load_handler());
    _srfce_prog.loaded.connect(_load_handler());

    _volume.init(*this);

    _camera.connect_inputs(ec).basic_input_mapping(ec);
    ec.setup_inputs().switch_input_mapping();
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
    _load_progress.update_progress(
      _mball_prog and _field_prog and _srfce_prog and _other);
}
//------------------------------------------------------------------------------
void example::update() noexcept {
    auto& state = context().state();
    if(state.is_active()) {
        reset_timeout();
    }
    if(state.user_idle_too_long()) {
        _camera.idle_update(state, 7.F);
    }

    if(_load_progress.done()) {
        const auto& glapi = _video.gl_api();
        const auto& [gl, GL] = glapi;

        _mball_prog.use(context());
        gl.dispatch_compute(1, 1, 1);

        _field_prog.use(context());
        _volume.compute(*this);

        _bg.clear(_video, _camera);

        gl.enable(GL.depth_test);
        gl.enable(GL.cull_face);

        _srfce_prog.use(context());
        _srfce_prog.prepare_frame(*this);
        _volume.draw(*this);
    } else {
        _mball_prog.load_if_needed(context());
        _field_prog.load_if_needed(context());
        _srfce_prog.load_if_needed(context());
    }

    _video.commit(*this);
}
//------------------------------------------------------------------------------
void example::clean_up() noexcept {
    _bg.clean_up(_video);
    _mball_prog.clean_up(context());
    _field_prog.clean_up(context());
    _srfce_prog.clean_up(context());
    _video.end();
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
               GL.geometry_shader and GL.fragment_shader and GL.compute_shader;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return launch_with_video<example>(ec);
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
