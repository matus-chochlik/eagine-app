/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#define OGLPLUS_GL_STATIC_FUNC(NAME) nullptr

#include <eagine/oalplus/al.hpp>
#include <eagine/oglplus/gl.hpp>
//
#include <eagine/oalplus/al_api.hpp>
#include <eagine/oglplus/gl_api.hpp>
//
#include <eagine/app/framedump_raw.hpp>
#include <eagine/app/input.hpp>
#include <eagine/app/opengl_eglplus.hpp>
#include <eagine/app/opengl_glfw3.hpp>
#include <eagine/app/state.hpp>
#include <eagine/branch_predict.hpp>
#include <eagine/memory/buffer.hpp>
#include <eagine/span.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// video_context
//------------------------------------------------------------------------------
class video_context_state {
public:
    video_context_state(execution_context&, const video_options&) noexcept;

    auto init_framebuffer(execution_context&, oglp::gl_api&) noexcept -> bool;

    auto doing_framedump() const noexcept;

    auto commit(long frame_number, video_provider&, oglp::gl_api&) -> bool;

    void clean_up(oglp::gl_api& api) noexcept;

private:
    const video_options& _options;
    oglp::owned_renderbuffer_name _color_rbo;
    oglp::owned_renderbuffer_name _depth_rbo;
    oglp::owned_renderbuffer_name _stencil_rbo;
    oglp::owned_framebuffer_name _offscreen_fbo;
    std::shared_ptr<framedump> _framedump_color{};
    std::shared_ptr<framedump> _framedump_depth{};
    std::shared_ptr<framedump> _framedump_stencil{};
};
//------------------------------------------------------------------------------
inline video_context_state::video_context_state(
  execution_context& ctx,
  const video_options& opts) noexcept
  : _options{opts} {
    if(_options.doing_framedump()) {

        auto raw_framedump = make_raw_framedump(ctx);
        if(extract(raw_framedump).initialize(ctx, opts)) {
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
EAGINE_LIB_FUNC
inline auto video_context_state::init_framebuffer(
  execution_context&,
  oglp::gl_api&) noexcept -> bool {
    if(_options.needs_offscreen_framebuffer()) {
        // TODO: check options and make RBOs and FBO
    }
    return true;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
inline auto video_context_state::doing_framedump() const noexcept {
    return _options.doing_framedump();
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
inline auto video_context_state::commit(
  long frame_number,
  video_provider& provider,
  oglp::gl_api& api) -> bool {
    bool result = true;
    if(EAGINE_UNLIKELY(doing_framedump())) {
        auto& [gl, GL] = api;

        if(EAGINE_LIKELY(gl.read_pixels)) {

            auto dump_frame = [&](
                                framedump& target,
                                auto gl_format,
                                auto gl_type,
                                framedump_pixel_format format,
                                framedump_data_type type,
                                int elements,
                                span_size_t element_size) {
                const auto [width, height] = provider.surface_size();
                const auto size =
                  span_size(width * height * elements * element_size);
                auto buffer{target.get_buffer(size)};

                api.operations().read_pixels(
                  0,
                  0,
                  oglp::gl_types::sizei_type(width),
                  oglp::gl_types::sizei_type(height),
                  gl_format,
                  gl_type,
                  buffer);

                if(!target.dump_frame(
                     frame_number,
                     width,
                     height,
                     elements,
                     element_size,
                     format,
                     type,
                     buffer)) {
                    result = false;
                }
            };

            if(_framedump_color) {
                switch(_options.framedump_color()) {
                    case framedump_data_type::none:
                        break;
                    case framedump_data_type::float_type:
                        dump_frame(
                          extract(_framedump_color),
                          GL.rgba,
                          GL.float_,
                          framedump_pixel_format::rgba,
                          framedump_data_type::float_type,
                          4,
                          span_size_of<oglp::gl_types::float_type>());
                        break;
                    case framedump_data_type::byte_type:
                        dump_frame(
                          extract(_framedump_color),
                          GL.rgba,
                          GL.unsigned_byte_,
                          framedump_pixel_format::rgba,
                          framedump_data_type::byte_type,
                          4,
                          span_size_of<oglp::gl_types::ubyte_type>());
                        break;
                }
            }

            if(_framedump_depth) {
                switch(_options.framedump_depth()) {
                    case framedump_data_type::none:
                    case framedump_data_type::byte_type:
                        break;
                    case framedump_data_type::float_type:
                        dump_frame(
                          extract(_framedump_depth),
                          GL.depth_component,
                          GL.float_,
                          framedump_pixel_format::depth,
                          framedump_data_type::float_type,
                          1,
                          span_size_of<oglp::gl_types::float_type>());
                        break;
                }
            }

            if(_framedump_stencil) {
                switch(_options.framedump_stencil()) {
                    case framedump_data_type::none:
                    case framedump_data_type::float_type:
                        break;
                    case framedump_data_type::byte_type:
                        dump_frame(
                          extract(_framedump_stencil),
                          GL.stencil_index,
                          GL.unsigned_byte_,
                          framedump_pixel_format::stencil,
                          framedump_data_type::byte_type,
                          1,
                          span_size_of<oglp::gl_types::ubyte_type>());
                        break;
                }
            }
        }
    }
    return result;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
inline void video_context_state::clean_up(oglp::gl_api& api) noexcept {
    if(_offscreen_fbo) {
        api.delete_framebuffers(std::move(_offscreen_fbo));
    }
    if(_stencil_rbo) {
        api.delete_renderbuffers(std::move(_stencil_rbo));
    }
    if(_depth_rbo) {
        api.delete_renderbuffers(std::move(_depth_rbo));
    }
    if(_color_rbo) {
        api.delete_renderbuffers(std::move(_color_rbo));
    }
}
//------------------------------------------------------------------------------
static void video_context_debug_callback(
  oglp::gl_types::enum_type source,
  oglp::gl_types::enum_type type,
  oglp::gl_types::uint_type id,
  oglp::gl_types::enum_type severity,
  oglp::gl_types::sizei_type length,
  const oglp::gl_types::char_type* message,
  const void* raw_pvc) {
    EAGINE_ASSERT(raw_pvc);
    const auto& vc = *static_cast<const video_context*>(raw_pvc);
    const auto msg = length >= 0 ? string_view(message, span_size(length))
                                 : string_view(message);
    vc.parent()
      .log_debug(msg)
      .arg(EAGINE_ID(severity), EAGINE_ID(DbgOutSvrt), severity)
      .arg(EAGINE_ID(source), EAGINE_ID(DbgOutSrce), source)
      .arg(EAGINE_ID(type), EAGINE_ID(DbgOutType), type)
      .arg(EAGINE_ID(id), id);

    EAGINE_MAYBE_UNUSED(source);
    EAGINE_MAYBE_UNUSED(type);
    EAGINE_MAYBE_UNUSED(id);
    EAGINE_MAYBE_UNUSED(severity);
    EAGINE_MAYBE_UNUSED(length);
    EAGINE_MAYBE_UNUSED(message);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto video_context::init_gl_api() noexcept -> bool {
    try {
        _gl_api = std::make_shared<oglp::gl_api>();
        auto& [gl, GL] = extract(_gl_api);

        const auto pos = _parent.options().video_requirements().find(
          extract(_provider).instance_id());
        EAGINE_ASSERT(pos != _parent.options().video_requirements().end());
        const auto& opts = pos->second;

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
                  GL.debug_severity_medium,
                  0U,
                  "successfully enabled GL debug output");
            } else {
                _parent.log_warning(
                  "requested GL debug, but GL context does not support it");
            }
        }

        _state = std::make_shared<video_context_state>(_parent, opts);

        if(!extract(_provider).has_framebuffer()) {
            if(!extract(_state).init_framebuffer(_parent, extract(_gl_api))) {
                _parent.log_error("failed to create offscreen framebuffer");
                return false;
            }
        }
    } catch(...) {
        return false;
    }
    return bool(_gl_api);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void video_context::begin() {
    extract(_provider).video_begin(_parent);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void video_context::end() {
    extract(_provider).video_end(_parent);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void video_context::commit() {
    if(EAGINE_LIKELY(_gl_api)) {
        if(EAGINE_UNLIKELY(!extract(_state).commit(
             _frame_no, extract(_provider), extract(_gl_api)))) {
            _parent.stop_running();
        }
    }
    extract(_provider).video_commit(_parent);

    if(EAGINE_UNLIKELY(_parent.enough_frames(++_frame_no))) {
        _parent.stop_running();
    }
    if(EAGINE_UNLIKELY(_parent.enough_run_time())) {
        _parent.stop_running();
    }
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void video_context::clean_up() noexcept {
    try {
        if(_state) {
            extract(_state).clean_up(extract(_gl_api));
            _state.reset();
        }
    } catch(...) {
    }
}
//------------------------------------------------------------------------------
// audio_context
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto audio_context::init_al_api() noexcept -> bool {
    try {
        _al_api = std::make_shared<oalp::al_api>();
    } catch(...) {
    }
    return bool(_al_api);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void audio_context::clean_up() noexcept {}
//------------------------------------------------------------------------------
// providers
//------------------------------------------------------------------------------
inline auto make_all_hmi_providers(main_ctx_parent parent)
  -> std::array<std::shared_ptr<hmi_provider>, 2> {
    return {
      {make_glfw3_opengl_provider(parent),
       make_eglplus_opengl_provider(parent)}};
}
//------------------------------------------------------------------------------
// execution_context
//------------------------------------------------------------------------------
inline auto execution_context::_setup_providers() -> bool {
    auto try_init = [&](auto provider) -> bool {
        if(extract(provider).is_initialized()) {
            return true;
        }
        if(extract(provider).should_initialize(*this)) {
            if(extract(provider).initialize(*this)) {
                return true;
            } else {
                log_error("failed to initialize HMI provider ${name}")
                  .arg(
                    EAGINE_ID(name), extract(provider).implementation_name());
            }
        } else {
            log_debug("skipping initialization of HMI provider ${name}")
              .arg(EAGINE_ID(name), extract(provider).implementation_name());
            return true;
        }
        return false;
    };

    for(auto& video_opts : _options._video_opts) {
        // TODO: proper provider selection
        if(!video_opts.second.has_provider()) {
            video_opts.second.set_provider("glfw3");
        }
    }

    for(auto& provider : _hmi_providers) {
        if(try_init(provider)) {
            auto add_input = [&](std::shared_ptr<input_provider> input) {
                extract(input).input_connect(*this);
                _input_providers.emplace_back(std::move(input));
            };
            extract(provider).input_enumerate({construct_from, add_input});

            auto add_video = [&](std::shared_ptr<video_provider> video) {
                _video_contexts.emplace_back(
                  std::make_unique<video_context>(*this, std::move(video)));
            };
            extract(provider).video_enumerate({construct_from, add_video});

            auto add_audio = [&](std::shared_ptr<audio_provider> audio) {
                _audio_contexts.emplace_back(
                  std::make_unique<audio_context>(*this, std::move(audio)));
            };
            extract(provider).audio_enumerate({construct_from, add_audio});
        }
    }

    return true;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::buffer() const noexcept -> memory::buffer& {
    return main_ctx_object::main_context().scratch_space();
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::state() const noexcept -> const context_state_view& {
    return extract(_state);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::enough_run_time() const noexcept -> bool {
    return options().enough_run_time(state().run_time());
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::enough_frames(span_size_t frame_no) const noexcept
  -> bool {
    return options().enough_frames(frame_no);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::prepare(std::unique_ptr<launchpad> pad)
  -> execution_context& {
    if(pad) {
        if(extract(pad).setup(main_context(), _options)) {

            for(auto& provider : make_all_hmi_providers(*this)) {
                if(extract(provider).is_implemented()) {
                    log_debug("using ${name} HMI provider")
                      .arg(
                        EAGINE_ID(name),
                        extract(provider).implementation_name());
                    _hmi_providers.emplace_back(std::move(provider));
                } else {
                    log_debug("${name} HMI provider is not implemented")
                      .arg(
                        EAGINE_ID(name),
                        extract(provider).implementation_name());
                }
            }

            if(_hmi_providers.empty()) {
                log_error("there are no available HMI providers");
                _exec_result = 5;
            } else {
                if(_setup_providers()) {
                    _state = std::make_shared<context_state>(*this);
                    EAGINE_ASSERT(_state);
                    if((_app = extract(pad).launch(*this, _options))) {
                        extract(_app).on_video_resize();
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
EAGINE_LIB_FUNC
auto execution_context::is_running() noexcept -> bool {
    if(EAGINE_LIKELY(_keep_running)) {
        if(EAGINE_LIKELY(_app)) {
            return !extract(_app).is_done();
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void execution_context::stop_running() noexcept {
    _keep_running = false;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::clean_up() noexcept {
    if(_app) {
        extract(_app).clean_up();
    }
    for(auto& input : _input_providers) {
        extract(input).input_disconnect();
    }
    for(auto& audio : _audio_contexts) {
        extract(audio).clean_up();
    }
    for(auto& video : _video_contexts) {
        extract(video).clean_up();
    }
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::update() noexcept {
    extract(_state).update_activity();
    for(auto& provider : _hmi_providers) {
        extract(provider).update(*this);
    }
    extract(_state).advance_time();
    extract(_app).update();
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::connect_input(message_id input_id, input_handler handler)
  -> execution_context& {
    _connected_inputs.emplace(input_id, std::move(handler));
    return *this;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::connect_inputs() -> execution_context& {
    connect_input(stop_running_input());
    return *this;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::map_input(
  message_id input_id,
  identifier mapping_id,
  message_id signal_id,
  input_setup setup) -> execution_context& {
    _input_mappings[mapping_id].emplace(signal_id, input_id, setup);
    return *this;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::map_inputs(identifier mapping_id)
  -> execution_context& {
    map_input(
      EAGINE_MSG_ID(App, Stop),
      mapping_id,
      EAGINE_MSG_ID(Keyboard, Escape),
      input_setup().trigger());
    return *this;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto execution_context::switch_input_mapping(identifier mapping_id)
  -> execution_context& {
    if(_input_mapping != mapping_id) {
        _mapped_inputs.clear();
        const auto& mapping = _input_mappings[mapping_id];
        for(auto& [signal_id, slot] : mapping) {
            const auto pos = _connected_inputs.find(std::get<0>(slot));
            if(pos != _connected_inputs.end()) {
                _mapped_inputs.emplace(
                  signal_id, std::get<1>(slot), std::get<1>(*pos));
            }
        }

        for(auto& input : _input_providers) {
            extract(input).mapping_begin(mapping_id);
            for(auto& slot : _mapped_inputs) {
                extract(input).mapping_enable(slot.first);
            }
            extract(input).mapping_commit(mapping_id);
        }

        _input_mapping = mapping_id;
    }
    return *this;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::random_uniform(span<byte> dest) {
    extract(_state).random_uniform(dest);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::random_uniform_01(span<float> dest) {
    extract(_state).random_uniform_01(dest);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::random_normal(span<float> dest) {
    extract(_state).random_normal(dest);
}
//------------------------------------------------------------------------------
template <typename T>
inline void execution_context::_forward_input(
  const input_info& info,
  const input_value<T>& value) noexcept {
    const auto slot_pos = _mapped_inputs.find(info.signal_id);
    if(slot_pos != _mapped_inputs.end()) {
        const auto& [setup, handler] = slot_pos->second;
        if(setup.is_applicable() && setup.has(info.value_kind)) {
            handler(input(value, info, setup));
        }
    }
    extract(_state).notice_user_active();
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::consume(
  const input_info& info,
  const input_value<bool>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::consume(
  const input_info& info,
  const input_value<int>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::consume(
  const input_info& info,
  const input_value<float>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void execution_context::consume(
  const input_info& info,
  const input_value<double>& value) noexcept {
    _forward_input(info, value);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
