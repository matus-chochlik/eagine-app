/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app:context;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.container;
import eagine.core.utility;
import eagine.core.main_ctx;
import eagine.eglplus;
import eagine.oglplus;
import eagine.oalplus;
import eagine.msgbus;
import :types;
import :interface;
import :options;
import :state;
import :input;
import :resource_loader;

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
      shared_holder<video_provider> provider) noexcept;

    /// @brief Returns a reference to the parent application execution context.
    [[nodiscard]] auto parent() const noexcept -> execution_context& {
        return _parent;
    }

    /// @brief Returns the current video frame number.
    [[nodiscard]] auto frame_number() const noexcept {
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
    void commit(application&);

    /// @brief Tries to intialize the GL rendering API in this video context.
    /// @see has_gl_api
    /// @see gl_api
    auto init_gl_api(execution_context&) noexcept -> bool;

    /// @brief Returns the shared GL API context wrapper.
    [[nodiscard]] auto gl_context() const noexcept
      -> oglplus::shared_gl_api_context {
        return _gl_api_context;
    }

    /// @brief Indicates if the GL rendering API in this video context is initialized.
    /// @see init_gl_api
    /// @see gl_api
    [[nodiscard]] auto has_gl_api() const noexcept {
        return bool(_gl_api_context);
    }

    /// @brief Returns a smart reference to the GL rendering API in this context.
    /// @see init_gl_api
    /// @see with_gl
    [[nodiscard]] auto gl_ref() const noexcept -> oglplus::gl_api_reference {
        return _gl_api_context.gl_ref();
    }

    /// @brief Returns a reference to the GL rendering API in this context.
    /// @see init_gl_api
    /// @see gl_ref
    /// @see with_gl
    /// @pre has_gl_api()
    [[nodiscard]] auto gl_api() const noexcept -> const oglplus::gl_api& {
        assert(has_gl_api());
        return _gl_api_context.gl_api();
    }

    /// @brief Calls the specified function if the EGL API is available.
    /// @see init_gl_api
    /// @see gl_ref
    template <typename Function>
    constexpr auto with_gl(Function&& function) const noexcept {
        return gl_ref().and_then(std::forward<Function>(function));
    }

    /// @brief Returns a smart reference to the EGL API in this context.
    /// @see init_egl_api
    /// @see egl_display
    /// @see with_egl
    [[nodiscard]] auto egl_ref() const noexcept -> eglplus::egl_api_reference {
        if(_provider) [[likely]] {
            return _provider->egl_ref();
        }
        return {};
    }

    /// @brief Returns a handle to this context's EGL display (if any).
    /// @see egl_ref
    [[nodiscard]] auto egl_display() noexcept -> eglplus::display_handle {
        if(_provider) [[likely]] {
            return _provider->egl_display();
        }
        return {};
    }

    /// @brief Calls the specified function if the EGL API is available.
    /// @see init_egl_api
    /// @see egl_ref
    /// @see egl_display
    template <typename Function>
    constexpr auto with_egl(Function&& function) const noexcept {
        return egl_ref().and_then(std::forward<Function>(function));
    }

    /// @brief Returns the rendering surface's dimensions (in pixels).
    [[nodiscard]] auto surface_size() noexcept -> std::tuple<int, int> {
        if(_provider) [[likely]] {
            return _provider->surface_size();
        }
        return {1, 1};
    }

    /// @brief Returns the rendering surface's dimensions (in pixels) as vec2.
    [[nodiscard]] auto surface_size_vec2() noexcept -> vec2 {
        const auto [x, y] = surface_size();
        return vec2{float(x), float(y)};
    }

    /// @brief Returns the rendering surface's aspect ratio.
    [[nodiscard]] auto surface_aspect() noexcept -> float {
        if(_provider) [[likely]] {
            return _provider->surface_aspect();
        }
        return 1.F;
    }

    /// @brief Adds a clean-up callback.
    void add_cleanup_op(callable_ref<void(video_context&) noexcept> op);

    template <typename Obj>
    auto clean_up_later(Obj& obj) -> auto& {
        add_cleanup_op(make_callable_ref({obj}, &_call_clean_up<Obj>));
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
    shared_holder<video_provider> _provider{};
    oglplus::shared_gl_api_context _gl_api_context{};
    shared_holder<video_context_state> _state{};
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
      shared_holder<audio_provider> provider) noexcept;

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
    void commit(application&);

    /// @brief Tries to intialize the AL sound API in this video context.
    /// @see al_ref
    auto init_al_api(execution_context&) noexcept -> bool;

    /// @brief Returns the shared AL API context wrapper.
    [[nodiscard]] auto al_context() const noexcept
      -> oalplus::shared_al_api_context {
        return _al_api_context;
    }

    /// @brief Indicates if the AL rendering API in this video context is initialized.
    /// @see init_al_api
    /// @see al_api
    [[nodiscard]] auto has_al_api() const noexcept {
        return bool(_al_api_context);
    }

    /// @brief Returns a smart reference to the AL rendering API in this context.
    /// @see init_al_api
    /// @see with_al
    [[nodiscard]] auto al_ref() const noexcept -> oalplus::al_api_reference {
        return _al_api_context.al_ref();
    }

    /// @brief Returns a reference to the AL rendering API in this context.
    /// @see init_al_api
    /// @see al_ref
    /// @see with_al
    /// @pre has_al_api()
    [[nodiscard]] auto al_api() const noexcept -> const oalplus::al_api& {
        assert(has_al_api());
        return _al_api_context.al_api();
    }

    template <typename Function>
    constexpr auto with_al(Function&& function) const noexcept {
        return al_ref().and_then(std::forward<Function>(function));
    }

    /// @brief Returns a reference to the ALUT sound API in this context.
    /// @see with_alut
    [[nodiscard]] auto alut_ref() const noexcept
      -> oalplus::alut_api_reference {
        return _alut_api_context.alut_ref();
    }

    template <typename Function>
    constexpr auto with_alut(Function&& function) const noexcept {
        return alut_ref().and_then(std::forward<Function>(function));
    }

    /// @brief Cleans up and releases this audio context and APIs.
    void clean_up() noexcept;

private:
    execution_context& _parent;
    shared_holder<audio_provider> _provider{};
    oalplus::shared_al_api_context _al_api_context{};
    oalplus::shared_alut_api_context _alut_api_context{};
};
//------------------------------------------------------------------------------
/// @brief Class providing various contexts to which a loaded_resource belongs
/// @see loaded_resource
/// @see resource_loader
export class loaded_resource_context {
public:
    loaded_resource_context(resource_loader& loader) noexcept
      : _loader{loader} {}

    loaded_resource_context(
      resource_loader& loader,
      const oglplus::shared_gl_api_context& gl_context) noexcept
      : _loader{loader}
      , _gl_context{gl_context} {}

    /// @brief Reference to a resource's parent loader.
    [[nodiscard]] auto loader() const noexcept -> resource_loader& {
        return _loader.get();
    }

    auto set(const oglplus::shared_gl_api_context& gl_context) noexcept
      -> loaded_resource_context& {
        _gl_context = gl_context;
        return *this;
    }

    auto set(const oalplus::shared_al_api_context& al_context) noexcept
      -> loaded_resource_context& {
        _al_context = al_context;
        return *this;
    }

    /// @brief Reference to a resource's parent GL context.
    [[nodiscard]] auto gl_context() const noexcept
      -> const oglplus::shared_gl_api_context& {
        return _gl_context;
    }

    /// @brief Reference to a resource's parent GL API.
    [[nodiscard]] auto gl_api() const noexcept -> const oglplus::gl_api& {
        return _gl_context.gl_api();
    }

    /// @brief Reference to a resource's parent AL context.
    [[nodiscard]] auto al_context() const noexcept
      -> const oalplus::shared_al_api_context& {
        return _al_context;
    }

    /// @brief Reference to a resource's parent AL API.
    [[nodiscard]] auto al_api() const noexcept -> const oalplus::al_api& {
        return _al_context.al_api();
    }

private:
    std::reference_wrapper<resource_loader> _loader;
    oglplus::shared_gl_api_context _gl_context;
    oalplus::shared_al_api_context _al_context;
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
    execution_context(main_ctx_parent parent) noexcept;

    /// @brief Returns the application execution result.
    [[nodiscard]] auto result() const noexcept -> int {
        return _exec_result;
    }

    /// @brief Returns a reference to the launch options.
    [[nodiscard]] auto options() const noexcept -> const launch_options& {
        return _options;
    }

    /// @brief Returns a reference to the resource loading context
    [[nodiscard]] auto resource_context() noexcept -> loaded_resource_context&;

    /// @brief Returns a reference to the resource loader.
    [[nodiscard]] auto loader() noexcept -> resource_loader&;

    /// @brief Returns a references to a multi-purpose memory buffer.
    [[nodiscard]] auto buffer() const noexcept -> memory::buffer&;

    /// @brief Returns a reference to the context state view.
    [[nodiscard]] auto state() const noexcept -> const context_state_view&;

    /// @brief Prepares the application launch pad object.
    [[nodiscard]] auto prepare(unique_holder<launchpad> pad)
      -> execution_context&;

    /// @brief Indicates if the application is running its main loop.
    /// @see stop_running
    [[nodiscard]] auto is_running() noexcept -> bool;

    /// @brief Stops the main application loop.
    /// @see is_running
    void stop_running() noexcept;

    /// @brief Updates this execution context (once per a single main loop iteration).
    void update() noexcept;

    /// @brief Cleans up this execution context and managed objects.
    void clean_up() noexcept;

    /// @brief Starts the main application loop (will block until stopped).
    [[nodiscard]] auto run() noexcept -> execution_context&;

    /// @brief Indicates if the application ran long enough.
    [[nodiscard]] auto enough_run_time() const noexcept -> bool;

    /// @brief Indicates if the application rendered enough frames.
    [[nodiscard]] auto enough_frames(const span_size_t frame_no) const noexcept
      -> bool;

    /// @brief Returns the count of created video contexts.
    [[nodiscard]] auto video_ctx_count() const noexcept -> span_size_t;

    /// @brief Returns the video context at the specified index.
    [[nodiscard]] auto video_ctx(const span_size_t index = 0) const noexcept
      -> optional_reference<video_context>;

    /// @brief Returns the main video context.
    [[nodiscard]] auto main_video() const noexcept -> video_context&;

    auto gl_initialized(video_context&) noexcept -> execution_context&;

    /// @brief Returns the count of created audio contexts.
    [[nodiscard]] auto audio_ctx_count() const noexcept -> span_size_t;

    /// @brief Returns the audio context at the specified index.
    [[nodiscard]] auto audio_ctx(const span_size_t index = 0) const noexcept
      -> optional_reference<audio_context>;

    /// @brief Returns the main audio context.
    [[nodiscard]] auto main_audio() const noexcept -> audio_context&;

    auto al_initialized(audio_context&) noexcept -> execution_context&;

    /// @brief Returns the canonical device id for application gui inputs.
    [[nodiscard]] constexpr auto app_gui_device_id() const noexcept
      -> identifier {
        return {"AppGUI"};
    }

    /// @brief Returns the canonical device id for keyboard.
    [[nodiscard]] constexpr auto keyboard_device_id() const noexcept
      -> identifier {
        return {"Keyboard"};
    }

    /// @brief Returns the canonical device id for mouse.
    [[nodiscard]] constexpr auto mouse_device_id() const noexcept
      -> identifier {
        return {"Mouse"};
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

    /// @brief Add a mapping of one input signal to an action on another UI input.
    auto add_ui_feedback(
      const identifier mapping_id,
      const identifier device_id,
      const message_id signal_id,
      const message_id input_id,
      input_feedback_trigger,
      input_feedback_action,
      std::variant<std::monostate, bool, float> threshold,
      std::variant<std::monostate, bool, float> constant) noexcept
      -> execution_context&;

    /// @brief Add a UI button with the specified label and id
    auto add_ui_button(const message_id id, const string_view label)
      -> execution_context&;

    /// @brief Add a UI toggle / checkbox with the specified label and id
    auto add_ui_toggle(const message_id, const string_view label, bool initial)
      -> execution_context&;
    auto set_ui_toggle(const message_id, bool value) noexcept
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
      const identifier mapping_id,
      const message_id input_id,
      const identifier device_id,
      const message_id signal_id,
      const input_setup setup) -> execution_context&;

    /// @brief Map a specified logical input to a physical input signal.
    auto map_input(
      const message_id input_id,
      const identifier device_id,
      const message_id signal_id,
      const input_setup setup) -> execution_context& {
        return map_input({}, input_id, device_id, signal_id, setup);
    }

    /// @brief Map a specified logical input to a keyboard key signal.
    auto map_key(
      const identifier mapping_id,
      const message_id input_id,
      const identifier key_id,
      const input_setup setup) -> execution_context& {
        return map_input(
          mapping_id, input_id, keyboard_device_id(), {{"Key"}, key_id}, setup);
    }

    /// @brief Map a specified logical input to a keyboard key signal.
    auto map_key(
      const identifier mapping_id,
      const message_id input_id,
      const identifier key_id) -> execution_context& {
        return map_key(mapping_id, input_id, key_id, input_setup().trigger());
    }

    /// @brief Map a specified logical input to a keyboard key signal.
    auto map_key(const message_id input_id, const identifier key_id)
      -> execution_context& {
        return map_key({}, input_id, key_id);
    }

    /// @brief Map a specified logical input to a keyboard key signal.
    auto map_key(
      const message_id input_id,
      const identifier key_id,
      const input_setup setup) -> execution_context& {
        return map_key({}, input_id, key_id, setup);
    }

    /// @brief Map a specified logical input to a mouse x-axis motion signal.
    auto map_cursor_motion_x(
      const identifier mapping_id,
      const message_id input_id,
      const input_setup setup) -> execution_context&;

    /// @brief Map a specified logical input to a mouse x-axis motion signal.
    auto map_cursor_motion_x(const message_id input_id, const input_setup setup)
      -> execution_context& {
        return map_cursor_motion_x({}, input_id, setup);
    }

    /// @brief Map a specified logical input to a mouse y-axis motion signal.
    auto map_cursor_motion_y(
      const identifier mapping_id,
      const message_id input_id,
      const input_setup setup) -> execution_context&;

    /// @brief Map a specified logical input to a mouse y-axis motion signal.
    auto map_cursor_motion_y(const message_id input_id, const input_setup setup)
      -> execution_context& {
        return map_cursor_motion_y({}, input_id, setup);
    }

    /// @brief Map a specified logical input to a y-axis scroll signal.
    auto map_wheel_scroll_y(
      const identifier mapping_id,
      const message_id input_id,
      const input_setup setup) -> execution_context&;

    /// @brief Map a specified logical input to a y-axis scroll signal.
    auto map_wheel_scroll_y(const message_id input_id, const input_setup setup)
      -> execution_context& {
        return map_wheel_scroll_y({}, input_id, setup);
    }

    /// @brief Map a specified logical input to a y-axis scroll signal.
    auto map_wheel_scroll_y(const message_id input_id) -> execution_context& {
        return map_wheel_scroll_y({}, input_id, input_setup().relative());
    }

    /// @brief Map a specified logical input to a left mouse button signal.
    auto map_left_mouse_button(
      const identifier mapping_id,
      const message_id input_id,
      const input_setup setup) -> execution_context&;

    /// @brief Map a specified logical input to a left mouse button signal.
    auto map_left_mouse_button(
      const message_id input_id,
      const input_setup setup) -> execution_context& {
        return map_left_mouse_button({}, input_id, setup);
    }

    /// @brief Map a specified logical input to a left mouse button signal.
    auto map_left_mouse_button(const message_id input_id)
      -> execution_context& {
        return map_left_mouse_button(input_id, input_setup().trigger());
    }

    /// @brief Map a specified logical input to a pointing device pressure signal.
    auto map_cursor_pressure(
      const identifier mapping_id,
      const message_id input_id,
      const input_setup setup) -> execution_context& {
        return map_left_mouse_button(mapping_id, input_id, setup);
    }

    /// @brief Map a specified logical input to a pointing device pressure signal.
    auto map_cursor_pressure(
      const identifier mapping_id,
      const message_id input_id) -> execution_context& {
        return map_cursor_pressure(
          mapping_id, input_id, input_setup().trigger());
    }

    /// @brief Map a specified logical input to a pointing device pressure signal.
    auto map_cursor_pressure(const message_id input_id) -> execution_context& {
        return map_cursor_pressure({}, input_id);
    }

    /// @brief Map a specified logical input to a GUI input signal.
    auto map_ui_input(
      const identifier mapping_id,
      const message_id input_id,
      const message_id signal_id,
      const input_setup setup) -> execution_context& {
        return map_input(
          mapping_id, input_id, app_gui_device_id(), signal_id, setup);
    }

    /// @brief Map a specified logical input to a GUI input signal.
    auto map_ui_input(
      const message_id input_id,
      const message_id signal_id,
      const input_setup setup) -> execution_context& {
        return map_input({}, input_id, app_gui_device_id(), signal_id, setup);
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

    [[nodiscard]] auto stop_running_input() noexcept -> input_slot;

    /// @brief Generates random uniformly-distributed bytes into @p dest.
    void random_uniform(span<byte> dest);

    /// @brief Generates random uniformly-distributed floats in <0, 1> into @p dest.
    void random_uniform_01(span<float> dest);

    /// @brief Generates random uniformly-distributed float in <0, 1>.
    [[nodiscard]] auto random_uniform_01() -> float;

    /// @brief Generates random uniformly-distributed float in <-1, 1>.
    [[nodiscard]] auto random_uniform_11() -> float;

    /// @brief Generates random normally-distributed floats into @p dest.
    void random_normal(span<float> dest);

private:
    int _exec_result{0};
    launch_options _options{*this};
    msgbus::registry _registry{*this};
    shared_holder<context_state> _state;
    unique_holder<application> _app;

    loaded_resource_context _resource_context;

    std::vector<shared_holder<hmi_provider>> _hmi_providers;
    std::vector<shared_holder<input_provider>> _input_providers;
    std::vector<unique_holder<video_context>> _video_contexts;
    std::vector<unique_holder<audio_context>> _audio_contexts;

    bool _keep_running{true};

    auto _setup_providers() noexcept -> bool;

    identifier _input_mapping{"initial"};

    // input id -> handler function reference
    flat_map<message_id, input_handler> _connected_inputs;

    // mapping id -> (device id, signal id) -> (input id, setup)
    flat_map<
      identifier,
      flat_map<
        std::tuple<identifier, message_id>,
        std::tuple<message_id, input_setup>>>
      _input_mappings;

    // (device id, signal id) -> (setup, handler)
    flat_map<
      std::tuple<identifier, message_id>,
      std::tuple<input_setup, input_handler>>
      _mapped_inputs;

    void _handle_stop_running(const input& engaged) noexcept;

    template <typename T>
    void _forward_input(
      const input_info& info,
      const input_value<T>& value) noexcept;

    void consume(const input_info&, const input_value<bool>&) noexcept final;
    void consume(const input_info&, const input_value<int>&) noexcept final;
    void consume(const input_info&, const input_value<float>&) noexcept final;
    void consume(const input_info&, const input_value<double>&) noexcept final;
};
//------------------------------------------------------------------------------
/// @brief Common implementation of the application interface.
export class common_application : public application {
public:
    /// @brief Construction specifying the application execution context.
    common_application(execution_context& ec) noexcept
      : _ctx{ec} {}

    /// @brief Returns the associated execution context.
    [[nodiscard]] auto context() const noexcept -> execution_context& {
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

