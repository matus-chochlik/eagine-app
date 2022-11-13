/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app:context;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.container;
import eagine.core.utility;
import eagine.core.main_ctx;
import eagine.oglplus;
import eagine.oalplus;
import eagine.msgbus;
import :interface;
import :options;
import :state;
import :input;
import :resource_loader;
import <map>;

namespace eagine::app {
export class video_context_state;
//------------------------------------------------------------------------------
/// @brief Class holding video rendering-related application support objects.
/// @ingroup application
/// @see video_options
/// @see audio_context
/// @see execution_context
///
/// Instances of this class represent a single (possibly one of several)
/// video rendering contexts in an application.
export class video_context {
public:
    video_context(
      execution_context& parent,
      std::shared_ptr<video_provider> provider) noexcept;

    /// @brief Returns a reference to the parent application execution context.
    auto parent() const noexcept -> execution_context& {
        return _parent;
    }

    /// @brief Returns the current video frame number.
    auto frame_number() const noexcept {
        return _frame_no;
    }

    /// @brief Start working in this video rendering context (make it current).
    /// @see end
    /// @see commit
    void begin();

    /// @brief Stop working with this video rendering context.
    /// @see begin
    /// @see commit
    void end();

    /// @brief Apply the rendering commands done since the last commit (swap buffers).
    /// @see begin
    /// @see end
    void commit();

    /// @brief Tries to intialize the GL rendering API in this video context.
    /// @see has_gl_api
    /// @see gl_api
    auto init_gl_api() noexcept -> bool;

    /// @brief Indicates if the GL rendering API in this video context is initialized.
    /// @see init_gl_api
    /// @see gl_api
    auto has_gl_api() const noexcept {
        return bool(_gl_api);
    }

    /// @brief Returns a reference to the GL rendering API in this context.
    /// @see init_gl_api
    /// @pre has_gl_api()
    auto gl_api() const noexcept -> oglplus::gl_api& {
        assert(has_gl_api());
        return *_gl_api;
    }

    /// @brief Returns the rendering surface's dimensions (in pixels).
    auto surface_size() noexcept -> std::tuple<int, int> {
        if(_provider) [[likely]] {
            return extract(_provider).surface_size();
        }
        return {1, 1};
    }

    /// @brief Returns the rendering surface's aspect ratio.
    auto surface_aspect() noexcept -> float {
        if(_provider) [[likely]] {
            return extract(_provider).surface_aspect();
        }
        return 1.F;
    }

    /// @brief Adds a clean-up callback.
    void add_cleanup_op(callable_ref<void(video_context&) noexcept> op);

    template <typename Obj>
    auto clean_up_later(Obj& obj) -> auto& {
        add_cleanup_op(callable_ref<void(video_context&) noexcept>{
          obj, &_call_clean_up<Obj>});
        return *this;
    }

    /// @brief Cleans up and releases this rendering context and APIs.
    void clean_up() noexcept;

private:
    template <typename Obj>
    static void _call_clean_up(Obj* obj, video_context& vc) noexcept {
        obj->clean_up(vc);
    }

    execution_context& _parent;
    long _frame_no{0};
    std::shared_ptr<video_provider> _provider{};
    std::shared_ptr<oglplus::gl_api> _gl_api{};
    std::shared_ptr<video_context_state> _state{};
};
//------------------------------------------------------------------------------
/// @brief Class holding audio playback and recording-related application support objects.
/// @ingroup application
/// @see audio_options
/// @see video_context
/// @see execution_context
///
/// Instances of this class represent a single (possibly one of several)
/// audio playback and recording contexts in an application.
export class audio_context {
public:
    audio_context(
      execution_context& parent,
      std::shared_ptr<audio_provider> provider) noexcept;

    /// @brief Start working in this audio context (make it current).
    /// @see end
    /// @see commit
    void begin();

    /// @brief Stop working with this audio context.
    /// @see begin
    /// @see commit
    void end();

    /// @brief Apply the rendering commands done since the last commit.
    /// @see begin
    /// @see end
    void commit();

    /// @brief Tries to intialize the AL sound API in this video context.
    /// @see has_al_api
    /// @see al_api
    auto init_al_api() noexcept -> bool;

    /// @brief Indicates if the AL API in this audio context is initialized.
    /// @see init_al_api
    /// @see al_api
    auto has_al_api() const noexcept {
        return bool(_al_api);
    }

    /// @brief Returns a reference to the AL API in this context.
    /// @see init_al_api
    /// @pre has_al_api()
    auto al_api() const noexcept -> auto& {
        assert(has_al_api());
        return *_al_api;
    }

    /// @brief Cleans up and releases this audio context and APIs.
    void clean_up() noexcept;

private:
    execution_context& _parent;
    std::shared_ptr<audio_provider> _provider{};
    std::shared_ptr<oalplus::al_api> _al_api{};
};
//------------------------------------------------------------------------------
/// @brief Class holding shared video/audio rendering application support objects.
/// @ingroup application
/// @see video_context
/// @see audio_context
/// @see launch_options
export class execution_context
  : public main_ctx_object
  , private input_sink {
public:
    execution_context(main_ctx_parent parent) noexcept
      : main_ctx_object("AppExecCtx", parent)
      , _options{*this}
      , _registry{*this}
      , _loader{_registry.emplace<resource_loader>("RsrsLoadr")} {}

    /// @brief Returns the application execution result.
    auto result() const noexcept -> int {
        return _exec_result;
    }

    /// @brief Returns a reference to the launch options.
    auto options() const noexcept -> const launch_options& {
        return _options;
    }

    /// @brief Returns a reference to the resource loader.
    auto loader() const noexcept -> resource_loader& {
        return _loader;
    }

    /// @brief Returns a references to a multi-purpose memory buffer.
    auto buffer() const noexcept -> memory::buffer&;

    /// @brief Returns a reference to the context state view.
    auto state() const noexcept -> const context_state_view&;

    /// @brief Prepares the application launch pad object.
    auto prepare(std::unique_ptr<launchpad> pad) -> execution_context&;

    /// @brief Indicates if the application is running its main loop.
    /// @see stop_running
    auto is_running() noexcept -> bool;

    /// @brief Stops the main application loop.
    /// @see is_running
    void stop_running() noexcept;

    /// @brief Updates this execution context (once per a single main loop iteration).
    void update() noexcept;

    /// @brief Cleans up this execution context and managed objects.
    void clean_up() noexcept;

    /// @brief Starts the main application loop (will block until stopped).
    auto run() noexcept -> execution_context& {
        while(is_running()) {
            update();
        }
        clean_up();
        return *this;
    }

    /// @brief Indicates if the application ran long enough.
    auto enough_run_time() const noexcept -> bool;

    /// @brief Indicates if the application rendered enough frames.
    auto enough_frames(const span_size_t frame_no) const noexcept -> bool;

    /// @brief Returns the count of created video contexts.
    auto video_ctx_count() const noexcept {
        return span_size(_video_contexts.size());
    }

    /// @brief Returns the video context at the specified index.
    auto video_ctx(const span_size_t index = 0) const noexcept
      -> video_context* {
        if((index >= 0) && (index < video_ctx_count())) {
            return _video_contexts[integer(index)].get();
        }
        return nullptr;
    }

    /// @brief Returns the main video context.
    auto main_video() const noexcept -> video_context& {
        assert(!_video_contexts.empty());
        assert(_video_contexts.front());
        return *_video_contexts.front();
    }

    /// @brief Returns the count of created audio contexts.
    auto audio_ctx_count() const noexcept {
        return span_size(_audio_contexts.size());
    }

    /// @brief Returns the audio context at the specified index.
    auto audio_ctx(const span_size_t index = 0) const noexcept
      -> audio_context* {
        if((index >= 0) && (index < audio_ctx_count())) {
            return _audio_contexts[integer(index)].get();
        }
        return nullptr;
    }

    /// @brief Returns the main audio context.
    auto main_audio() const noexcept -> audio_context& {
        assert(!_audio_contexts.empty());
        assert(_audio_contexts.front());
        return *_audio_contexts.front();
    }

    /// @brief Connect the specified logical input to a callable handler reference.
    auto connect_input(const message_id input_id, const input_handler handler)
      -> execution_context&;

    /// @brief Connect the specified input slot.
    /// @see input_slot
    auto connect_input(const input_slot& input) -> auto& {
        return connect_input(input.id(), input.handler());
    }

    /// @brief Connect generic, reusable application logical input slots.
    auto connect_inputs() -> execution_context&;

    /// @brief Add a UI button with the specified label and id
    auto add_ui_button(const message_id id, const string_view label)
      -> execution_context&;

    /// @brief Add a UI slider with the specified label and id
    auto add_ui_slider(
      const message_id id,
      const string_view label,
      float min,
      float max,
      float initial) -> execution_context&;

    /// @brief Sets the slider position with the specified id to a value.
    auto set_ui_slider(const message_id, float value) noexcept
      -> execution_context&;

    /// @brief Map a specified logical input to a physical input signal.
    auto map_input(
      const message_id input_id,
      const identifier mapping_id,
      const message_id signal_id,
      const input_setup setup) -> execution_context&;

    /// @brief Map a specified logical input to a physical input signal.
    auto map_input(
      const message_id input_id,
      const message_id signal_id,
      const input_setup setup) -> execution_context& {
        return map_input(input_id, {}, signal_id, setup);
    }

    /// @brief Binds generic application inputs to default physical input signals.
    auto map_inputs(const identifier mapping_id) -> execution_context&;

    /// @brief Binds generic application inputs to default physical input signals.
    auto map_inputs() -> execution_context& {
        return map_inputs({});
    }

    /// @brief Sets-up (connects handlers and binds to signals) default inputs.
    auto setup_inputs(const identifier mapping_id) -> execution_context& {
        return connect_inputs().map_inputs(mapping_id);
    }

    /// @brief Sets-up (connects handlers and binds to signals) default inputs.
    auto setup_inputs() -> execution_context& {
        return setup_inputs({});
    }

    /// @brief Switches to the input mapping with the specified id.
    auto switch_input_mapping(const identifier mapping_id)
      -> execution_context&;

    /// @brief Switches to the default input mapping.
    auto switch_input_mapping() -> auto& {
        return switch_input_mapping({});
    }

    auto stop_running_input() noexcept -> input_slot {
        return {
          message_id{"App", "Stop"},
          make_callable_ref<&execution_context::_handle_stop_running>(this)};
    }

    /// @brief Generates random uniformly-distributed bytes into @p dest.
    void random_uniform(span<byte> dest);

    /// @brief Generates random uniformly-distributed floats in <0, 1> into @p dest.
    void random_uniform_01(span<float> dest);

    /// @brief Generates random uniformly-distributed float in <0, 1>.
    auto random_uniform_01() -> float;

    /// @brief Generates random uniformly-distributed float in <-1, 1>.
    auto random_uniform_11() -> float;

    /// @brief Generates random normally-distributed floats into @p dest.
    void random_normal(span<float> dest);

private:
    int _exec_result{0};
    launch_options _options;
    msgbus::registry _registry;
    std::shared_ptr<context_state> _state;
    std::unique_ptr<application> _app;

    resource_loader& _loader;
    std::vector<std::shared_ptr<hmi_provider>> _hmi_providers;
    std::vector<std::shared_ptr<input_provider>> _input_providers;
    std::vector<std::unique_ptr<video_context>> _video_contexts;
    std::vector<std::unique_ptr<audio_context>> _audio_contexts;

    bool _keep_running{true};

    auto _setup_providers() noexcept -> bool;

    identifier _input_mapping{"initial"};

    // input id -> handler function reference
    flat_map<message_id, input_handler> _connected_inputs;

    // mapping id -> signal id -> (input id, setup)
    flat_map<
      identifier,
      flat_map<message_id, std::tuple<message_id, input_setup>>>
      _input_mappings;

    // signal id -> (setup, handler)
    flat_map<message_id, std::tuple<input_setup, input_handler>> _mapped_inputs;

    void _handle_stop_running(const input& engaged) noexcept {
        if(engaged) {
            stop_running();
        }
    }

    template <typename T>
    void _forward_input(
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

    void consume(
      const input_info& info,
      const input_value<bool>& value) noexcept final {
        _forward_input(info, value);
    }
    void consume(const input_info& info, const input_value<int>& value) noexcept
      final {
        _forward_input(info, value);
    }
    void consume(
      const input_info& info,
      const input_value<float>& value) noexcept final {
        _forward_input(info, value);
    }
    void consume(
      const input_info& info,
      const input_value<double>& value) noexcept final {
        _forward_input(info, value);
    }
};
//------------------------------------------------------------------------------
/// @brief Common implementation of the application interface.
export class common_application : public application {
public:
    /// @brief Construction specifying the application execution context.
    common_application(execution_context& ec) noexcept
      : _ctx{ec} {}

    /// @brief Returns the associated execution context.
    auto context() const noexcept -> execution_context& {
        return _ctx;
    }

    /// @brief Default implementation of the rendering surface resize handler.
    void on_video_resize() noexcept override {
        auto video = _ctx.main_video();
        video.gl_api().viewport[video.surface_size()];
    }

private:
    execution_context& _ctx;
};
//------------------------------------------------------------------------------
/// @brief Implementation of the application interface with execution timeout.
export class timeouting_application : public common_application {
public:
    /// @brief Construction specifying the execution context and timeout interval.
    timeouting_application(
      execution_context& ec,
      const timeout::duration_type is_done_time) noexcept
      : common_application{ec}
      , _is_done{is_done_time} {}

    /// @brief Resets the is-done timeout.
    auto reset_timeout() noexcept -> auto& {
        _is_done.reset();
        return *this;
    }

    /// @brief Returns a callable reference to reset_timeout on this object.
    auto reset_timeout_handler() noexcept {
        return make_callable_ref<&timeouting_application::_do_reset_timeout>(
          this);
    }

    /// @brief Indicates if the timeout specified in the constructor expired.
    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

private:
    void _do_reset_timeout(const input&) noexcept {
        _is_done.reset();
    }

    timeout _is_done{std::chrono::seconds{30}};
};
//------------------------------------------------------------------------------

} // namespace eagine::app

