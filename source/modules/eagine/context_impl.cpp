/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.reflection;
import eagine.core.container;
import eagine.core.valid_if;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.oglplus;
import eagine.oalplus;

namespace eagine::app {
//------------------------------------------------------------------------------
// video_context
//------------------------------------------------------------------------------
class video_context_state {
public:
    video_context_state(execution_context&, const video_options&) noexcept;

    auto init_framebuffer(execution_context&, const oglplus::gl_api&) noexcept
      -> bool;

    auto should_dump_frame(application&) const noexcept -> bool;

    auto framedump_number(const long frame_no) noexcept
      -> valid_if_nonnegative<long>;

    auto commit(
      long frame_number,
      application&,
      video_provider&,
      const oglplus::gl_api&) -> bool;

    void add_cleanup_op(callable_ref<void(video_context&) noexcept> op);

    void clean_up(video_context&) noexcept;

private:
    auto _dump_frame(
      const long frame_number,
      video_provider& provider,
      const oglplus::gl_api& api) -> bool;
    void _clean_up(auto&) noexcept;

    const video_options& _options;
    oglplus::owned_renderbuffer_name _color_rbo;
    oglplus::owned_renderbuffer_name _depth_rbo;
    oglplus::owned_renderbuffer_name _stencil_rbo;
    oglplus::owned_framebuffer_name _offscreen_fbo;
    shared_holder<framedump> _framedump_color{};
    shared_holder<framedump> _framedump_depth{};
    shared_holder<framedump> _framedump_stencil{};
    std::vector<callable_ref<void(video_context&) noexcept>> _cleanup_ops;
    long _dump_frame_no{0};
};
//------------------------------------------------------------------------------
inline auto video_context_state::should_dump_frame(
  application& app) const noexcept -> bool {
    return _options.doing_framedump() and app.should_dump_frame();
}
//------------------------------------------------------------------------------
inline video_context_state::video_context_state(
  execution_context& ctx,
  const video_options& opts) noexcept
  : _options{opts} {
    if(_options.doing_framedump()) {

        auto raw_framedump = make_raw_framedump(ctx);
        if(raw_framedump->initialize(ctx, opts)) {
            if(_options.framedump_color() != framedump_data_type::none) {
                _framedump_color = raw_framedump;
            }
            if(_options.framedump_depth() != framedump_data_type::none) {
                _framedump_depth = raw_framedump;
            }
            if(_options.framedump_stencil() != framedump_data_type::none) {
                _framedump_stencil = raw_framedump;
            }
        }
    }
}
//------------------------------------------------------------------------------
inline auto video_context_state::init_framebuffer(
  execution_context&,
  const oglplus::gl_api&) noexcept -> bool {
    if(_options.needs_offscreen_framebuffer()) {
        // TODO: check options and make RBOs and FBO
    }
    return true;
}
//------------------------------------------------------------------------------
auto video_context_state::framedump_number(const long) noexcept
  -> valid_if_nonnegative<long> {
    return _options.framedump_number(_dump_frame_no++);
}
//------------------------------------------------------------------------------
auto video_context_state::_dump_frame(
  const long frame_number,
  video_provider& provider,
  const oglplus::gl_api& api) -> bool {
    bool result = true;
    const auto& [gl, GL]{api};

    if(gl.read_pixels) [[likely]] {
        const auto do_dump_frame{[&, this](
                                   framedump& target,
                                   const auto gl_format,
                                   const auto gl_type,
                                   const framedump_pixel_format format,
                                   const framedump_data_type type,
                                   const int elements,
                                   const span_size_t element_size) {
            const auto [width, height] = provider.surface_size();

            if(const auto framedump_no{framedump_number(frame_number)}) {
                const auto size{
                  span_size(width * height * elements * element_size)};
                auto buffer{target.get_buffer(size)};

                api.operations().read_pixels(
                  0,
                  0,
                  oglplus::gl_types::sizei_type(width),
                  oglplus::gl_types::sizei_type(height),
                  gl_format,
                  gl_type,
                  buffer);

                if(not target.dump_frame(
                     *framedump_no,
                     width,
                     height,
                     elements,
                     element_size,
                     format,
                     type,
                     buffer)) {
                    result = false;
                }
            }
        }};

        if(_framedump_color) {
            switch(_options.framedump_color()) {
                case framedump_data_type::none:
                    break;
                case framedump_data_type::float_type:
                    do_dump_frame(
                      *_framedump_color,
                      GL.rgba,
                      GL.float_,
                      framedump_pixel_format::rgba,
                      framedump_data_type::float_type,
                      4,
                      span_size_of<oglplus::gl_types::float_type>());
                    break;
                case framedump_data_type::byte_type:
                    do_dump_frame(
                      *_framedump_color,
                      GL.rgba,
                      GL.unsigned_byte_,
                      framedump_pixel_format::rgba,
                      framedump_data_type::byte_type,
                      4,
                      span_size_of<oglplus::gl_types::ubyte_type>());
                    break;
            }
        }

        if(_framedump_depth) {
            switch(_options.framedump_depth()) {
                case framedump_data_type::none:
                case framedump_data_type::byte_type:
                    break;
                case framedump_data_type::float_type:
                    do_dump_frame(
                      *_framedump_depth,
                      GL.depth_component,
                      GL.float_,
                      framedump_pixel_format::depth,
                      framedump_data_type::float_type,
                      1,
                      span_size_of<oglplus::gl_types::float_type>());
                    break;
            }
        }

        if(_framedump_stencil) {
            switch(_options.framedump_stencil()) {
                case framedump_data_type::none:
                case framedump_data_type::float_type:
                    break;
                case framedump_data_type::byte_type:
                    do_dump_frame(
                      *_framedump_stencil,
                      GL.stencil_index,
                      GL.unsigned_byte_,
                      framedump_pixel_format::stencil,
                      framedump_data_type::byte_type,
                      1,
                      span_size_of<oglplus::gl_types::ubyte_type>());
                    break;
            }
        }
    }
    return result;
}
//------------------------------------------------------------------------------
inline auto video_context_state::commit(
  const long frame_number,
  application& app,
  video_provider& provider,
  const oglplus::gl_api& api) -> bool {
    bool result = true;
    if(should_dump_frame(app)) [[unlikely]] {
        result = _dump_frame(frame_number, provider, api) and result;
    }
    return result;
}
//------------------------------------------------------------------------------
void video_context_state::add_cleanup_op(
  callable_ref<void(video_context&) noexcept> op) {
    _cleanup_ops.push_back(op);
}
//------------------------------------------------------------------------------
void video_context_state::_clean_up(auto& gl) noexcept {
    if(_offscreen_fbo) {
        gl.delete_framebuffers(std::move(_offscreen_fbo));
    }
    if(_stencil_rbo) {
        gl.delete_renderbuffers(std::move(_stencil_rbo));
    }
    if(_depth_rbo) {
        gl.delete_renderbuffers(std::move(_depth_rbo));
    }
    if(_color_rbo) {
        gl.delete_renderbuffers(std::move(_color_rbo));
    }
}
//------------------------------------------------------------------------------
inline void video_context_state::clean_up(video_context& vc) noexcept {
    for(auto& clean_up : _cleanup_ops) {
        clean_up(vc);
    }
    _cleanup_ops.clear();
    vc.with_gl([this](auto& gl) { _clean_up(gl); });
}
//------------------------------------------------------------------------------
static void video_context_debug_callback(
  [[maybe_unused]] oglplus::gl_types::enum_type source,
  [[maybe_unused]] oglplus::gl_types::enum_type type,
  [[maybe_unused]] oglplus::gl_types::uint_type id,
  [[maybe_unused]] oglplus::gl_types::enum_type severity,
  [[maybe_unused]] oglplus::gl_types::sizei_type length,
  [[maybe_unused]] const oglplus::gl_types::char_type* message,
  const void* raw_pvc) {
    assert(raw_pvc);
    const auto& vc = *static_cast<const video_context*>(raw_pvc);
    const auto msg = length >= 0 ? string_view(message, span_size(length))
                                 : string_view(message);
    vc.parent()
      .log_debug(msg)
      .tag("glDbgOutpt")
      .arg("severity", "DbgOutSvrt", severity)
      .arg("source", "DbgOutSrce", source)
      .arg("type", "DbgOutType", type)
      .arg("id", id);
}
//------------------------------------------------------------------------------
video_context::video_context(
  execution_context& parent,
  shared_holder<video_provider> provider) noexcept
  : _parent{parent}
  , _provider{std::move(provider)} {
    assert(_provider);
    _provider->parent_context_changed(*this);
}
//------------------------------------------------------------------------------
auto video_context::init_gl_api(execution_context& ec) noexcept -> bool {
    try {
        _gl_api_context.ensure(_parent.as_parent());
        ec.gl_initialized(*this);
        const auto& [gl, GL] = gl_api();

        const auto found{eagine::find(
          _parent.options().video_requirements(), _provider->instance_id())};
        assert(found);
        const auto& opts = *found;

        if(opts.gl_debug_context()) {
            if(gl.ARB_debug_output) {
                _parent.log_info("enabling GL debug output");

                gl.debug_message_callback(
                  &video_context_debug_callback,
                  static_cast<const void*>(this));

                gl.debug_message_control(
                  GL.dont_care, GL.dont_care, GL.dont_care, GL.true_);

                gl.debug_message_insert(
                  GL.debug_source_application,
                  GL.debug_type_other,
                  0U, // ID
                  GL.debug_severity_medium,
                  "successfully enabled GL debug output");
            } else {
                _parent.log_warning(
                  "requested GL debug, but GL context does not support it");
            }
        }

        _state.emplace(_parent, opts);

        if(not _provider->has_framebuffer()) {
            if(not _state->init_framebuffer(_parent, gl_api())) {
                _parent.log_error("failed to create offscreen framebuffer");
                return false;
            }
        }
    } catch(...) {
        return false;
    }
    return bool(_gl_api_context);
}
//------------------------------------------------------------------------------
void video_context::begin() {
    _provider->video_begin(_parent);
}
//------------------------------------------------------------------------------
void video_context::end() {
    _provider->video_end(_parent);
}
//------------------------------------------------------------------------------
void video_context::commit(application& app) {
    if(_gl_api_context) [[likely]] {
        if(not _state->commit(_frame_no, app, *_provider, gl_api()))
          [[unlikely]] {
            _parent.stop_running();
        }
    }
    _provider->video_commit(_parent);

    if(_parent.enough_frames(++_frame_no)) [[unlikely]] {
        _parent.stop_running();
    }
    if(_parent.enough_run_time()) [[unlikely]] {
        _parent.stop_running();
    }
}
//------------------------------------------------------------------------------
void video_context::add_cleanup_op(
  callable_ref<void(video_context&) noexcept> op) {
    if(_state) {
        _state->add_cleanup_op(op);
    }
}
//------------------------------------------------------------------------------
void video_context::clean_up() noexcept {
    try {
        if(_state) {
            _state->clean_up(*this);
            _state.reset();
        }
    } catch(...) {
    }
}
//------------------------------------------------------------------------------
// audio_context
//------------------------------------------------------------------------------
audio_context::audio_context(
  execution_context& parent,
  shared_holder<audio_provider> provider) noexcept
  : _parent{parent}
  , _provider{std::move(provider)} {
    assert(_provider);
    _provider->parent_context_changed(*this);
}
//------------------------------------------------------------------------------
void audio_context::begin() {
    _provider->audio_begin(_parent);
}
//------------------------------------------------------------------------------
void audio_context::end() {
    _provider->audio_end(_parent);
}
//------------------------------------------------------------------------------
void audio_context::commit(application&) {
    _provider->audio_commit(_parent);
}
//------------------------------------------------------------------------------
auto audio_context::init_al_api(execution_context& ec) noexcept -> bool {
    try {
        _al_api_context.ensure(_parent.as_parent());
        _alut_api_context.ensure(_parent.as_parent());
        ec.al_initialized(*this);
    } catch(...) {
    }
    return bool(_al_api_context);
}
//------------------------------------------------------------------------------
void audio_context::clean_up() noexcept {}
//------------------------------------------------------------------------------
// providers
//------------------------------------------------------------------------------
inline auto make_all_hmi_providers(main_ctx_parent parent)
  -> std::array<shared_holder<hmi_provider>, 3> {
    return {
      {make_glfw3_opengl_provider(parent),
       make_eglplus_opengl_provider(parent),
       make_oalplus_openal_provider(parent)}};
}
//------------------------------------------------------------------------------
// execution_context
//------------------------------------------------------------------------------
execution_context::execution_context(main_ctx_parent parent) noexcept
  : main_ctx_object("AppExecCtx", parent)
  , _resource_manager{
      default_selector,
      shared_holder<loaded_resource_context>{
        default_selector,
        _registry.emplace<old_resource_loader>("ORsrLoadr"),
        _registry.emplace<resource_loader>("RsrsLoadr")}} {}
//------------------------------------------------------------------------------
inline auto execution_context::_setup_providers() noexcept -> bool {
    const auto try_init{[&](auto provider) -> bool {
        if(provider->is_initialized()) {
            return true;
        }
        if(provider->should_initialize(*this)) {
            if(provider->initialize(*this)) {
                return true;
            } else {
                log_error("failed to initialize HMI provider ${name}")
                  .arg("name", provider->implementation_name());
            }
        } else {
            log_debug("skipping initialization of HMI provider ${name}")
              .arg("name", provider->implementation_name());
            return true;
        }
        return false;
    }};

    for(auto& video_opts : _options._video_opts) {
        // TODO: proper provider selection
        if(not video_opts.second.has_provider()) {
            video_opts.second.set_provider("glfw3");
        }
    }

    for(auto& audio_opts : _options._audio_opts) {
        // TODO: proper provider selection
        if(not audio_opts.second.has_provider()) {
            audio_opts.second.set_provider("oalplus");
        }
    }

    for(auto& provider : _hmi_providers) {
        if(try_init(provider)) {
            const auto add_input{[&](shared_holder<input_provider> input) {
                input->input_connect(*this);
                _input_providers.emplace_back(std::move(input));
            }};
            provider->input_enumerate({construct_from, add_input});

            const auto add_video{[&](shared_holder<video_provider> video) {
                _video_contexts.emplace_back(
                  default_selector, *this, std::move(video));
            }};
            provider->video_enumerate({construct_from, add_video});

            const auto add_audio{[&](shared_holder<audio_provider> audio) {
                _audio_contexts.emplace_back(
                  default_selector, *this, std::move(audio));
            }};
            provider->audio_enumerate({construct_from, add_audio});
        }
    }

    return true;
}
//------------------------------------------------------------------------------
auto execution_context::resources() noexcept -> resource_manager& {
    assert(_resource_manager);
    return *_resource_manager;
}
//------------------------------------------------------------------------------
auto execution_context::shared_resource_context() noexcept
  -> const shared_holder<loaded_resource_context>& {
    return resources().resource_context();
}
//------------------------------------------------------------------------------
auto execution_context::resource_context() noexcept
  -> loaded_resource_context& {
    return *(resources().resource_context());
}
//------------------------------------------------------------------------------
auto execution_context::old_loader() noexcept -> old_resource_loader& {
    return resource_context().old_loader();
}
//------------------------------------------------------------------------------
auto execution_context::loader() noexcept -> resource_loader& {
    return resource_context().loader();
}
//------------------------------------------------------------------------------
auto execution_context::buffer() const noexcept -> memory::buffer& {
    return main_ctx_object::main_context().scratch_space();
}
//------------------------------------------------------------------------------
auto execution_context::state() const noexcept -> const context_state_view& {
    assert(_state);
    return *_state;
}
//------------------------------------------------------------------------------
auto execution_context::enough_run_time() const noexcept -> bool {
    return options().enough_run_time(state().run_time());
}
//------------------------------------------------------------------------------
auto execution_context::enough_frames(const span_size_t frame_no) const noexcept
  -> bool {
    return options().enough_frames(frame_no);
}
//------------------------------------------------------------------------------
auto execution_context::video_ctx_count() const noexcept -> span_size_t {
    return span_size(_video_contexts.size());
}
//------------------------------------------------------------------------------
auto execution_context::video_ctx(const span_size_t index) const noexcept
  -> optional_reference<video_context> {
    if((index >= 0) and (index < video_ctx_count())) {
        return _video_contexts[integer(index)].get();
    }
    return {};
}
//------------------------------------------------------------------------------
auto execution_context::main_video() const noexcept -> video_context& {
    assert(not _video_contexts.empty());
    assert(_video_contexts.front());
    return *_video_contexts.front();
}
//------------------------------------------------------------------------------
auto execution_context::gl_initialized(video_context& video) noexcept
  -> execution_context& {
    if(not resource_context().gl_context()) {
        resource_context().set(video.gl_context());
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::audio_ctx_count() const noexcept -> span_size_t {
    return span_size(_audio_contexts.size());
}
//------------------------------------------------------------------------------
auto execution_context::audio_ctx(const span_size_t index) const noexcept
  -> optional_reference<audio_context> {
    if((index >= 0) and (index < audio_ctx_count())) {
        return _audio_contexts[integer(index)].get();
    }
    return {};
}
//------------------------------------------------------------------------------
auto execution_context::main_audio() const noexcept -> audio_context& {
    assert(not _audio_contexts.empty());
    assert(_audio_contexts.front());
    return *_audio_contexts.front();
}
//------------------------------------------------------------------------------
auto execution_context::al_initialized(audio_context& audio) noexcept
  -> execution_context& {
    if(not resource_context().al_context()) {
        resource_context().set(audio.al_context());
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::prepare(unique_holder<launchpad> pad)
  -> execution_context& {
    if(pad) {
        if(pad->setup(main_context(), _options)) {

            for(auto& provider : make_all_hmi_providers(*this)) {
                if(provider->is_implemented()) {
                    log_debug("using ${name} HMI provider")
                      .arg("name", provider->implementation_name());
                    _hmi_providers.emplace_back(std::move(provider));
                } else {
                    log_debug("${name} HMI provider is not implemented")
                      .arg("name", provider->implementation_name());
                }
            }

            if(_hmi_providers.empty()) {
                log_error("there are no available HMI providers");
                _exec_result = 5;
            } else {
                if(_setup_providers()) {
                    _state.emplace(*this);
                    assert(_state);
                    if((_app = pad->launch(*this, _options))) {
                        _app->on_video_resize();
                    } else {
                        log_error("failed to launch application");
                        _exec_result = 3;
                    }
                } else {
                    log_error("failed to setup providers");
                    _exec_result = 4;
                }
            }
        } else {
            log_error("failed to setup application launchpad");
            _exec_result = 2;
        }
    } else {
        log_error("received invalid application launchpad");
        _exec_result = 1;
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::is_running() noexcept -> bool {
    if(_keep_running) [[likely]] {
        if(_app) [[likely]] {
            return not _app->is_done();
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void execution_context::stop_running() noexcept {
    _keep_running = false;
}
//------------------------------------------------------------------------------
void execution_context::clean_up() noexcept {
    if(_app) {
        _app->clean_up();
    }
    for(auto& input : _input_providers) {
        input->input_disconnect();
    }
    for(auto& audio : _audio_contexts) {
        audio->clean_up();
    }
    for(auto& video : _video_contexts) {
        video->clean_up();
    }
    for(auto& provider : _hmi_providers) {
        provider->clean_up(*this);
    }
}
//------------------------------------------------------------------------------
auto execution_context::run() noexcept -> execution_context& {
    declare_state("running", "runStart", "runFinish");
    active_state("running");
    log_info("application main loop started").tag("runStart");
    while(is_running()) {
        update();
    }
    log_info("application main loop finishing").tag("runFinish");
    clean_up();
    return *this;
}
//------------------------------------------------------------------------------
void execution_context::update() noexcept {
    static const auto exec_time_id{register_time_interval("appUpdate")};
    const auto exec_time{measure_time_interval(exec_time_id)};
    _registry.update_and_process();
    _resource_manager->update();
    _state->update_activity();
    for(auto& provider : _hmi_providers) {
        provider->update(*this, *_app);
    }
    _state->advance_time();
    _app->update();
}
//------------------------------------------------------------------------------
auto execution_context::app_gui_device_id() const noexcept -> identifier {
    return {"AppGUI"};
}
//------------------------------------------------------------------------------
auto execution_context::keyboard_device_id() const noexcept -> identifier {
    return {"Keyboard"};
}
//------------------------------------------------------------------------------
auto execution_context::mouse_device_id() const noexcept -> identifier {
    return {"Mouse"};
}
//------------------------------------------------------------------------------
auto execution_context::connect_input(
  const message_id input_id,
  const input_handler handler) -> execution_context& {
    _connected_inputs.emplace(input_id, handler);
    log_info("added input slot ${input}")
      .tag("connInput")
      .arg("input", input_id);
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::connect_inputs() -> execution_context& {
    connect_input(stop_running_input());
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::add_ui_feedback(
  const identifier mapping_id,
  const identifier device_id,
  const message_id signal_id,
  const message_id input_id,
  input_feedback_trigger trigger,
  input_feedback_action action,
  std::variant<std::monostate, bool, float> threshold,
  std::variant<std::monostate, bool, float> constant) noexcept
  -> execution_context& {
    for(auto& input : _input_providers) {
        input->add_ui_feedback(
          mapping_id,
          device_id,
          signal_id,
          input_id,
          trigger,
          action,
          threshold,
          constant);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::add_ui_button(
  const message_id input_id,
  const string_view label) -> execution_context& {
    for(auto& input : _input_providers) {
        input->add_ui_button(input_id, label);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::add_ui_toggle(
  const message_id input_id,
  const string_view label,
  bool initial) -> execution_context& {
    for(auto& input : _input_providers) {
        input->add_ui_toggle(input_id, label, initial);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::set_ui_toggle(
  const message_id input_id,
  bool value) noexcept -> execution_context& {
    for(auto& input : _input_providers) {
        input->set_ui_toggle(input_id, value);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::add_ui_slider(
  const message_id input_id,
  const string_view label,
  float min,
  float max,
  float initial) -> execution_context& {
    if((min <= initial) and (initial <= max)) {
        const input_value_kind kind = ((min >= 0.F) and (max <= 1.F))
                                        ? input_value_kind::absolute_norm
                                        : input_value_kind::absolute_free;
        for(auto& input : _input_providers) {
            input->add_ui_slider(input_id, label, min, max, initial, kind);
        }
    } else {
        log_error("inconsistent min, max and initial values")
          .arg("min", min)
          .arg("max", max)
          .arg("initial", initial);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::set_ui_slider(
  const message_id input_id,
  float value) noexcept -> execution_context& {
    for(auto& input : _input_providers) {
        input->set_ui_slider(input_id, value);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::map_input(
  const identifier mapping_id,
  const message_id input_id,
  const identifier device_id,
  const message_id signal_id,
  const input_setup setup) -> execution_context& {
    _input_mappings[mapping_id].emplace(
      std::make_tuple(device_id, signal_id), input_id, setup);
    log_info("mapped input signal ${signal} to input slot ${input}")
      .tag("mapInput")
      .arg("signal", signal_id)
      .arg("device", device_id)
      .arg("input", input_id)
      .arg("mapping", mapping_id);
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::map_cursor_motion_x(
  const identifier mapping_id,
  const message_id input_id,
  const input_setup setup) -> execution_context& {
    return map_input(
      mapping_id, input_id, {"Mouse"}, {"Cursor", "MotionX"}, setup);
}
//------------------------------------------------------------------------------
auto execution_context::map_cursor_motion_y(
  const identifier mapping_id,
  const message_id input_id,
  const input_setup setup) -> execution_context& {
    return map_input(
      mapping_id, input_id, {"Mouse"}, {"Cursor", "MotionY"}, setup);
}
//------------------------------------------------------------------------------
auto execution_context::map_wheel_scroll_y(
  const identifier mapping_id,
  const message_id input_id,
  const input_setup setup) -> execution_context& {
    return map_input(
      mapping_id, input_id, {"Mouse"}, {"Wheel", "ScrollY"}, setup);
}
//------------------------------------------------------------------------------
auto execution_context::map_left_mouse_button(
  const identifier mapping_id,
  const message_id input_id,
  const input_setup setup) -> execution_context& {
    return map_input(
      mapping_id, input_id, {"Mouse"}, {"Cursor", "Pressure"}, setup);
}
//------------------------------------------------------------------------------
auto execution_context::map_inputs(const identifier mapping_id)
  -> execution_context& {
    map_key(mapping_id, {"App", "Stop"}, {"Escape"});
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::switch_input_mapping(const identifier mapping_id)
  -> execution_context& {
    if(_input_mapping != mapping_id) {
        _mapped_inputs.clear();
        const auto& mapping = _input_mappings[mapping_id];
        for(auto& [dev_sig_id, slot] : mapping) {
            if(const auto found{
                 eagine::find(_connected_inputs, std::get<0>(slot))}) {
                _mapped_inputs.emplace(dev_sig_id, std::get<1>(slot), *found);
            }
        }

        for(auto& input : _input_providers) {
            input->mapping_begin(mapping_id);
            for(auto& slot : _mapped_inputs) {
                input->mapping_enable(
                  std::get<0>(slot.first), std::get<1>(slot.first));
            }
            input->mapping_commit(*this, mapping_id);
        }

        _input_mapping = mapping_id;
    }
    return *this;
}
//------------------------------------------------------------------------------
auto execution_context::stop_running_input() noexcept -> input_slot {
    return {
      message_id{"App", "Stop"},
      make_callable_ref<&execution_context::_handle_stop_running>(this)};
}
//------------------------------------------------------------------------------
void execution_context::_handle_stop_running(const input& engaged) noexcept {
    if(engaged) {
        stop_running();
    }
}
//------------------------------------------------------------------------------
template <typename T>
void execution_context::_forward_input(
  const input_info& info,
  const input_value<T>& value) noexcept {
    if(const auto found{eagine::find(
         _mapped_inputs, std::make_tuple(info.device_id, info.signal_id))}) {
        const auto& [setup, handler] = *found;
        if(setup.is_applicable() and setup.has(info.value_kind)) {
            handler(input(value, info, setup));
        }
    }
    _state->notice_user_active();
}
//------------------------------------------------------------------------------
void execution_context::consume(
  const input_info& info,
  const input_value<bool>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
void execution_context::consume(
  const input_info& info,
  const input_value<int>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
void execution_context::consume(
  const input_info& info,
  const input_value<float>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
void execution_context::consume(
  const input_info& info,
  const input_value<double>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
void execution_context::random_uniform(span<byte> dest) {
    _state->random_uniform(dest);
}
//------------------------------------------------------------------------------
void execution_context::random_uniform_01(span<float> dest) {
    _state->random_uniform_01(dest);
}
//------------------------------------------------------------------------------
auto execution_context::random_uniform_01() -> float {
    return _state->random_uniform_01();
}
//------------------------------------------------------------------------------
auto execution_context::random_uniform_11() -> float {
    return _state->random_uniform_11();
}
//------------------------------------------------------------------------------
void execution_context::random_normal(span<float> dest) {
    _state->random_normal(dest);
}
//------------------------------------------------------------------------------
auto video_ctx(execution_context& ec, span_size_t index) noexcept
  -> optional_reference<video_context> {
    return ec.video_ctx(index);
}
//------------------------------------------------------------------------------
auto audio_ctx(execution_context& ec, span_size_t index) noexcept
  -> optional_reference<audio_context> {
    return ec.audio_ctx(index);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
