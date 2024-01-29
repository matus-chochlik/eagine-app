/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

#if __has_include(<GLFW/glfw3.h>)
#include <GLFW/glfw3.h>
#define EAGINE_APP_HAS_GLFW3 1
#else
#define EAGINE_APP_HAS_GLFW3 0
#endif

module eagine.app;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.identifier;
import eagine.core.container;
import eagine.core.logging;
import eagine.core.progress;
import eagine.core.utility;
import eagine.core.valid_if;
import eagine.core.c_api;
import eagine.core.main_ctx;
import eagine.eglplus;
import eagine.guiplus;

namespace eagine::app {
//------------------------------------------------------------------------------
#if EAGINE_APP_HAS_GLFW3
struct glfw3_activity_progress_info {
    activity_progress_id_t activity_id{0};
    activity_progress_id_t parent_id{0};
    span_size_t current_steps{0};
    span_size_t total_steps{0};
    std::string title;
};
//------------------------------------------------------------------------------
// glfw3_window_ui_input_feedback
//------------------------------------------------------------------------------
class glfw3_opengl_window;
struct glfw3_window_ui_input_state;
struct glfw3_window_ui_input_feedback {
    message_id input_id;
    std::variant<std::monostate, bool, float> threshold;
    std::variant<std::monostate, bool, float> constant;
    input_feedback_trigger trigger{input_feedback_trigger::change};
    input_feedback_action action{input_feedback_action::copy};

    auto is_under_threshold(const input_variable<bool>& inp) const noexcept
      -> bool;
    auto is_over_threshold(const input_variable<bool>& inp) const noexcept
      -> bool;
    auto is_under_threshold(const input_variable<float>& inp) const noexcept
      -> bool;
    auto is_over_threshold(const input_variable<float>& inp) const noexcept
      -> bool;

    template <typename T>
    auto is_triggered(const input_variable<T>& inp) const noexcept -> bool;

    void key_press_changed(
      const execution_context& ctx,
      glfw3_opengl_window& parent,
      const input_variable<bool>& inp) const noexcept;

    auto multiply(bool value) const noexcept -> bool;
    auto multiply(float value) const noexcept -> float;

    void copy_to(bool& dst, const input_value<bool>& inp) const noexcept {
        dst = inp.get();
    }
    void copy_to(bool& dst, const input_value<float>& inp) const noexcept {
        dst = std::abs(inp.get()) > 0.F;
    }
    void copy_to(float& dst, const input_value<bool>& inp) const noexcept {
        dst = (inp.get() ? 1.F : 0.F);
    }
    void copy_to(float& dst, const input_value<float>& inp) const noexcept {
        dst = inp.get();
    }

    void set_zero(bool& dst) const noexcept {
        dst = false;
    }
    void set_zero(float& dst) const noexcept {
        dst = 0.F;
    }
    void set_one(bool& dst) const noexcept {
        dst = true;
    }
    void set_one(float& dst) const noexcept {
        dst = 1.F;
    }
    void flip(bool& dst) const noexcept {
        dst = not dst;
    }
    void flip(float&) const noexcept {
        // TODO: what does this mean?
    }

    void add_to(bool& dst) const noexcept {
        dst = dst or multiply(true);
    }
    void add_to(float& dst) const noexcept {
        dst = dst + multiply(1.F);
    }

    void multiply_add_to(bool& dst, const input_value<bool>& inp)
      const noexcept {
        dst = dst or multiply(inp.get());
    }
    void multiply_add_to(bool& dst, const input_value<float>& inp)
      const noexcept {
        dst = dst or (std::abs(multiply(inp.get())) > 0.F);
    }
    void multiply_add_to(float& dst, const input_value<bool>& inp)
      const noexcept {
        dst = dst + (multiply(inp.get()) ? 1.F : 0.F);
    }
    void multiply_add_to(float& dst, const input_value<float>& inp)
      const noexcept {
        dst = dst + multiply(inp.get());
    }

    template <typename T, typename S>
    void apply_to(T& dst, const input_value<S>& src) const noexcept;
};
//------------------------------------------------------------------------------
// glfw3_window_ui_button_state
//------------------------------------------------------------------------------
struct glfw3_window_ui_button_state {
    input_variable<bool> pressed{false};
    std::string label;

    template <typename T>
    void apply_feedback(
      const execution_context& ctx,
      glfw3_opengl_window&,
      const glfw3_window_ui_input_state&,
      const glfw3_window_ui_input_feedback&,
      const input_value<T>&);
};
//------------------------------------------------------------------------------
template <typename T>
void glfw3_window_ui_button_state::apply_feedback(
  const execution_context&,
  glfw3_opengl_window& parent,
  const glfw3_window_ui_input_state&,
  const glfw3_window_ui_input_feedback&,
  const input_value<T>&) {}
//------------------------------------------------------------------------------
// glfw3_window_ui_toggle_state
//------------------------------------------------------------------------------
struct glfw3_window_ui_toggle_state {
    input_variable<bool> toggled_on{false};
    std::string label;
    bool value{false};

    template <typename T>
    void apply_feedback(
      const execution_context& ctx,
      glfw3_opengl_window&,
      const glfw3_window_ui_input_state&,
      const glfw3_window_ui_input_feedback&,
      const input_value<T>&);
};
//------------------------------------------------------------------------------
// glfw3_window_ui_slider_state
//------------------------------------------------------------------------------
struct glfw3_window_ui_slider_state {
    input_variable<float> position{0.5F};
    std::string label;
    float min{0.F};
    float max{1.F};
    float value{0.5F};
    input_value_kind kind{input_value_kind::absolute_norm};

    template <typename T>
    void apply_feedback(
      const execution_context& ctx,
      glfw3_opengl_window&,
      const glfw3_window_ui_input_state&,
      const glfw3_window_ui_input_feedback&,
      const input_value<T>&);
};
//------------------------------------------------------------------------------
// glfw3_window_ui_input_state
//------------------------------------------------------------------------------
using glfw3_window_ui_state_variant = std::variant<
  glfw3_window_ui_button_state,
  glfw3_window_ui_toggle_state,
  glfw3_window_ui_slider_state>;

struct glfw3_window_ui_input_state {
    message_id input_id;
    glfw3_window_ui_state_variant state;

    auto kind() const noexcept -> input_value_kind;

    auto get_toggle() noexcept
      -> optional_reference<glfw3_window_ui_toggle_state>;
    auto get_slider() noexcept
      -> optional_reference<glfw3_window_ui_slider_state>;

    auto apply(const auto& func) noexcept {
        std::visit(func, state);
    }
};
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_state::kind() const noexcept -> input_value_kind {
    return get_if<glfw3_window_ui_slider_state>(state)
      .member(&glfw3_window_ui_slider_state::kind)
      .value_or(input_value_kind::absolute_norm);
}
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_state::get_toggle() noexcept
  -> optional_reference<glfw3_window_ui_toggle_state> {
    return get_if<glfw3_window_ui_toggle_state>(state);
}
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_state::get_slider() noexcept
  -> optional_reference<glfw3_window_ui_slider_state> {
    return get_if<glfw3_window_ui_slider_state>(state);
}
//------------------------------------------------------------------------------
// glfw3_opengl_window
//------------------------------------------------------------------------------
class glfw3_opengl_provider;
class glfw3_opengl_window
  : public main_ctx_object
  , public video_provider
  , public input_provider {
public:
    glfw3_opengl_window(
      application_config&,
      const identifier instance_id,
      glfw3_opengl_provider& parent);

    glfw3_opengl_window(
      application_config&,
      const identifier instance_id,
      const string_view instance,
      glfw3_opengl_provider& parent);

    auto initialize(
      const launch_options&,
      const video_options&,
      const span<GLFWmonitor* const>) -> bool;

    void update_gui(execution_context&, application&);
    void update_glfw(execution_context&, application&);
    void update(execution_context&, application&);

    void clean_up();

    auto video_kind() const noexcept -> video_context_kind final;
    auto instance_id() const noexcept -> identifier final;

    auto is_offscreen() noexcept -> tribool final;
    auto has_framebuffer() noexcept -> tribool final;
    auto surface_size() noexcept -> std::tuple<int, int> final;
    auto surface_aspect() noexcept -> float final;
    auto egl_ref() noexcept -> eglplus::egl_api_reference final;
    auto egl_display() noexcept -> eglplus::display_handle final;
    auto imgui_ref() noexcept -> guiplus::imgui_api_reference final;

    void parent_context_changed(const video_context&) final;
    void video_begin(execution_context&) final;
    void video_end(execution_context&) final;
    void video_commit(execution_context&) final;

    void input_enumerate(
      execution_context&,
      const callable_ref<void(
        const identifier,
        const message_id,
        const input_value_kinds) noexcept>) noexcept final;

    void input_connect(input_sink&) final;
    void input_disconnect() final;

    void mapping_begin(const identifier setup_id) final;
    void mapping_enable(const identifier device_id, const message_id signal_id)
      final;
    void mapping_commit(execution_context&, const identifier setup_id) final;

    auto add_ui_feedback(
      const identifier mapping_id,
      const identifier device_id,
      const message_id signal_id,
      const message_id input_id,
      input_feedback_trigger,
      input_feedback_action,
      std::variant<std::monostate, bool, float> threshold,
      std::variant<std::monostate, bool, float> constant) noexcept
      -> bool final;

    auto add_ui_button(const message_id, const string_view label) -> bool final;

    auto add_ui_toggle(const message_id, const string_view label, bool initial)
      -> bool final;
    auto set_ui_toggle(const message_id, bool value) noexcept -> bool final;

    auto add_ui_slider(
      const message_id,
      const string_view label,
      float min,
      float max,
      float initial,
      input_value_kind kind) -> bool final;
    auto set_ui_slider(const message_id, float value) noexcept -> bool final;

    void on_scroll(const float x, const float y) {
        _wheel_change_x += x;
        _wheel_change_y += y;
    }

    auto handle_progress() noexcept -> bool;

private:
    glfw3_opengl_provider& _provider;
    identifier _instance_id;
    guiplus::gui_utils _gui{*this};
    guiplus::imgui_context _imgui_context;
    std::string _format_buffer;

    GLFWwindow* _window{nullptr};
    optional_reference<input_sink> _input_sink{nullptr};
    const video_context* _parent_context{nullptr};
    int _window_width{1};
    int _window_height{1};

    struct key_state {
        identifier key_id;
        int key_code;
        input_variable<bool> pressed{false};
        bool enabled{false};

        constexpr key_state(const identifier id, const int code) noexcept
          : key_id{id}
          , key_code{code} {}
    };

    flat_set<std::tuple<identifier, message_id>> _enabled_signals;

    std::vector<key_state> _key_states;
    std::vector<key_state> _mouse_states;

    friend struct glfw3_window_ui_input_feedback;
    friend struct glfw3_window_ui_toggle_state;
    friend struct glfw3_window_ui_slider_state;

    auto _setup_ui_input(
      message_id input_id,
      glfw3_window_ui_state_variant state) -> bool;
    auto _find_ui_input(message_id input_id) noexcept
      -> optional_reference<glfw3_window_ui_input_state>;

    std::vector<glfw3_window_ui_input_state> _ui_input_states;

    struct ui_feedback_targets {
        std::vector<glfw3_window_ui_input_feedback> targets;

        auto key_press_changed(
          const execution_context& ctx,
          glfw3_opengl_window& parent,
          const input_variable<bool>&) const noexcept -> bool;
    };

    template <typename T>
    void _forward_feedback(
      const execution_context& ctx,
      const glfw3_window_ui_input_state&,
      const input_value<T>& value);

    void _ui_input_feedback(
      const execution_context& ctx,
      const glfw3_window_ui_input_feedback&,
      const input_variable<bool>&) noexcept;

    void _feedback_key_press_change(
      const execution_context& ctx,
      const identifier device_id,
      const message_id key_id,
      const input_variable<bool>&) noexcept;

    flat_map<std::tuple<identifier, message_id>, ui_feedback_targets>
      _ui_feedbacks;

    input_variable<float> _mouse_x_pix{0};
    input_variable<float> _mouse_y_pix{0};
    input_variable<float> _mouse_x_ndc{0};
    input_variable<float> _mouse_y_ndc{0};
    input_variable<float> _mouse_x_delta{0};
    input_variable<float> _mouse_y_delta{0};
    input_variable<float> _wheel_scroll_x{0};
    input_variable<float> _wheel_scroll_y{0};
    float _norm_x_ndc{1};
    float _norm_y_ndc{1};
    float _aspect{1};
    float _wheel_change_x{0};
    float _wheel_change_y{0};
    bool _imgui_visible{false};
    bool _imgui_updated{false};
    bool _mouse_enabled{false};
    bool _backtick_was_pressed{false};
};
//------------------------------------------------------------------------------
class glfw3_opengl_provider final
  : public main_ctx_object
  , public hmi_provider
  , public progress_observer {
public:
    glfw3_opengl_provider(main_ctx_parent parent);
    glfw3_opengl_provider(glfw3_opengl_provider&&) = delete;
    glfw3_opengl_provider(const glfw3_opengl_provider&) = delete;
    auto operator=(glfw3_opengl_provider&&) = delete;
    auto operator=(const glfw3_opengl_provider&) = delete;
    ~glfw3_opengl_provider() noexcept final;

    auto is_implemented() const noexcept -> bool final;
    auto implementation_name() const noexcept -> string_view final;

    auto is_initialized() -> bool final;
    auto should_initialize(execution_context&) -> bool final;
    auto initialize(execution_context&) -> bool final;
    void update(execution_context&, application&) final;
    void clean_up(execution_context&) final;

    void input_enumerate(
      callable_ref<void(shared_holder<input_provider>)>) final;
    void video_enumerate(
      callable_ref<void(shared_holder<video_provider>)>) final;
    void audio_enumerate(
      callable_ref<void(shared_holder<audio_provider>)>) final;

    auto activities() const noexcept
      -> span<const glfw3_activity_progress_info> {
        return view(_activities);
    }

    void activity_begun(
      const activity_progress_id_t parent_id,
      const activity_progress_id_t activity_id,
      const string_view title,
      const span_size_t total_steps) noexcept final;

    void activity_finished(
      const activity_progress_id_t parent_id,
      const activity_progress_id_t activity_id,
      const string_view title,
      span_size_t total_steps) noexcept final;

    void activity_updated(
      const activity_progress_id_t parent_id,
      const activity_progress_id_t activity_id,
      const span_size_t current,
      const span_size_t total) noexcept final;

private:
#if EAGINE_APP_HAS_GLFW3
    std::map<identifier, shared_holder<glfw3_opengl_window>> _windows;
    std::vector<glfw3_activity_progress_info> _activities;
#endif
    auto _get_progress_callback() noexcept -> callable_ref<bool() noexcept>;
    auto _handle_progress() noexcept -> bool;
};
//------------------------------------------------------------------------------
// glfw3_opengl_window_scroll_callback
//------------------------------------------------------------------------------
void glfw3_opengl_window_scroll_callback(GLFWwindow* window, double x, double y) {
    if(auto raw_that{glfwGetWindowUserPointer(window)}) {
        auto that = reinterpret_cast<glfw3_opengl_window*>(raw_that);
        that->on_scroll(float(x), float(y));
    }
}
//------------------------------------------------------------------------------
glfw3_opengl_window::glfw3_opengl_window(
  application_config& c,
  const identifier instance_id,
  const string_view instance,
  glfw3_opengl_provider& parent)
  : main_ctx_object{"GLFW3Wndow", parent}
  , _provider{parent}
  , _instance_id{instance_id} {

    // keyboard keys/buttons
    _key_states.emplace_back("Spacebar", GLFW_KEY_SPACE);
    _key_states.emplace_back("Backspace", GLFW_KEY_BACKSPACE);
    _key_states.emplace_back("Escape", GLFW_KEY_ESCAPE);
    _key_states.emplace_back("Enter", GLFW_KEY_ENTER);
    _key_states.emplace_back("Tab", GLFW_KEY_TAB);
    _key_states.emplace_back("Left", GLFW_KEY_LEFT);
    _key_states.emplace_back("Right", GLFW_KEY_RIGHT);
    _key_states.emplace_back("Up", GLFW_KEY_UP);
    _key_states.emplace_back("Down", GLFW_KEY_DOWN);
    _key_states.emplace_back("PageUp", GLFW_KEY_PAGE_UP);
    _key_states.emplace_back("PageDown", GLFW_KEY_PAGE_DOWN);
    _key_states.emplace_back("Home", GLFW_KEY_HOME);
    _key_states.emplace_back("End", GLFW_KEY_END);
    _key_states.emplace_back("Insert", GLFW_KEY_INSERT);
    _key_states.emplace_back("Delete", GLFW_KEY_DELETE);
    _key_states.emplace_back("Apostrophe", GLFW_KEY_APOSTROPHE);
    _key_states.emplace_back("Minus", GLFW_KEY_MINUS);
    _key_states.emplace_back("Equal", GLFW_KEY_EQUAL);
    _key_states.emplace_back("Period", GLFW_KEY_PERIOD);
    _key_states.emplace_back("Semicolon", GLFW_KEY_SEMICOLON);
    _key_states.emplace_back("Slash", GLFW_KEY_SLASH);
    _key_states.emplace_back("LtBracket", GLFW_KEY_LEFT_BRACKET);
    _key_states.emplace_back("RtBracket", GLFW_KEY_RIGHT_BRACKET);
    _key_states.emplace_back("CapsLock", GLFW_KEY_CAPS_LOCK);
    _key_states.emplace_back("NumLock", GLFW_KEY_NUM_LOCK);
    _key_states.emplace_back("ScrollLock", GLFW_KEY_SCROLL_LOCK);
    _key_states.emplace_back("PrntScreen", GLFW_KEY_PRINT_SCREEN);
    _key_states.emplace_back("Pause", GLFW_KEY_PAUSE);

    _key_states.emplace_back("A", GLFW_KEY_A);
    _key_states.emplace_back("B", GLFW_KEY_B);
    _key_states.emplace_back("C", GLFW_KEY_C);
    _key_states.emplace_back("D", GLFW_KEY_D);
    _key_states.emplace_back("E", GLFW_KEY_E);
    _key_states.emplace_back("F", GLFW_KEY_F);
    _key_states.emplace_back("G", GLFW_KEY_G);
    _key_states.emplace_back("H", GLFW_KEY_H);
    _key_states.emplace_back("I", GLFW_KEY_I);
    _key_states.emplace_back("J", GLFW_KEY_J);
    _key_states.emplace_back("K", GLFW_KEY_K);
    _key_states.emplace_back("L", GLFW_KEY_L);
    _key_states.emplace_back("M", GLFW_KEY_M);
    _key_states.emplace_back("N", GLFW_KEY_N);
    _key_states.emplace_back("O", GLFW_KEY_O);
    _key_states.emplace_back("P", GLFW_KEY_P);
    _key_states.emplace_back("Q", GLFW_KEY_Q);
    _key_states.emplace_back("R", GLFW_KEY_R);
    _key_states.emplace_back("S", GLFW_KEY_S);
    _key_states.emplace_back("T", GLFW_KEY_T);
    _key_states.emplace_back("U", GLFW_KEY_U);
    _key_states.emplace_back("V", GLFW_KEY_V);
    _key_states.emplace_back("W", GLFW_KEY_W);
    _key_states.emplace_back("X", GLFW_KEY_X);
    _key_states.emplace_back("Y", GLFW_KEY_Y);
    _key_states.emplace_back("Z", GLFW_KEY_Z);

    _key_states.emplace_back("0", GLFW_KEY_0);
    _key_states.emplace_back("1", GLFW_KEY_1);
    _key_states.emplace_back("2", GLFW_KEY_2);
    _key_states.emplace_back("3", GLFW_KEY_3);
    _key_states.emplace_back("4", GLFW_KEY_4);
    _key_states.emplace_back("5", GLFW_KEY_5);
    _key_states.emplace_back("6", GLFW_KEY_6);
    _key_states.emplace_back("7", GLFW_KEY_7);
    _key_states.emplace_back("8", GLFW_KEY_8);
    _key_states.emplace_back("9", GLFW_KEY_9);

    _key_states.emplace_back("KeyPad0", GLFW_KEY_KP_0);
    _key_states.emplace_back("KeyPad1", GLFW_KEY_KP_1);
    _key_states.emplace_back("KeyPad2", GLFW_KEY_KP_2);
    _key_states.emplace_back("KeyPad3", GLFW_KEY_KP_3);
    _key_states.emplace_back("KeyPad4", GLFW_KEY_KP_4);
    _key_states.emplace_back("KeyPad5", GLFW_KEY_KP_5);
    _key_states.emplace_back("KeyPad6", GLFW_KEY_KP_6);
    _key_states.emplace_back("KeyPad7", GLFW_KEY_KP_7);
    _key_states.emplace_back("KeyPad8", GLFW_KEY_KP_8);
    _key_states.emplace_back("KeyPad9", GLFW_KEY_KP_9);

    _key_states.emplace_back("KpDecimal", GLFW_KEY_KP_DECIMAL);
    _key_states.emplace_back("KpPlus", GLFW_KEY_KP_ADD);
    _key_states.emplace_back("KpMinus", GLFW_KEY_KP_SUBTRACT);
    _key_states.emplace_back("KpAsterisk", GLFW_KEY_KP_MULTIPLY);
    _key_states.emplace_back("KpSlash", GLFW_KEY_KP_DIVIDE);
    _key_states.emplace_back("KpEqual", GLFW_KEY_KP_EQUAL);
    _key_states.emplace_back("KpEnter", GLFW_KEY_KP_ENTER);

    _key_states.emplace_back("LeftSuper", GLFW_KEY_LEFT_SUPER);
    _key_states.emplace_back("LeftShift", GLFW_KEY_LEFT_SHIFT);
    _key_states.emplace_back("LeftCtrl", GLFW_KEY_LEFT_CONTROL);
    _key_states.emplace_back("LeftAlt", GLFW_KEY_LEFT_ALT);
    _key_states.emplace_back("RightSuper", GLFW_KEY_RIGHT_SUPER);
    _key_states.emplace_back("RightShift", GLFW_KEY_RIGHT_SHIFT);
    _key_states.emplace_back("RightCtrl", GLFW_KEY_RIGHT_CONTROL);
    _key_states.emplace_back("RightAlt", GLFW_KEY_RIGHT_ALT);
    _key_states.emplace_back("Menu", GLFW_KEY_MENU);

    _key_states.emplace_back("F1", GLFW_KEY_F1);
    _key_states.emplace_back("F2", GLFW_KEY_F2);
    _key_states.emplace_back("F3", GLFW_KEY_F3);
    _key_states.emplace_back("F4", GLFW_KEY_F4);
    _key_states.emplace_back("F5", GLFW_KEY_F5);
    _key_states.emplace_back("F6", GLFW_KEY_F6);
    _key_states.emplace_back("F7", GLFW_KEY_F7);
    _key_states.emplace_back("F8", GLFW_KEY_F8);
    _key_states.emplace_back("F9", GLFW_KEY_F9);
    _key_states.emplace_back("F10", GLFW_KEY_F10);
    _key_states.emplace_back("F11", GLFW_KEY_F11);
    _key_states.emplace_back("F12", GLFW_KEY_F12);

    // mouse buttons
    _mouse_states.emplace_back("Pressure", GLFW_MOUSE_BUTTON_1);
    _mouse_states.emplace_back("Button0", GLFW_MOUSE_BUTTON_1);
    _mouse_states.emplace_back("Button1", GLFW_MOUSE_BUTTON_2);
    _mouse_states.emplace_back("Button2", GLFW_MOUSE_BUTTON_3);
    _mouse_states.emplace_back("Button3", GLFW_MOUSE_BUTTON_4);
    _mouse_states.emplace_back("Button4", GLFW_MOUSE_BUTTON_5);
    _mouse_states.emplace_back("Button5", GLFW_MOUSE_BUTTON_6);
    _mouse_states.emplace_back("Button6", GLFW_MOUSE_BUTTON_7);
    _mouse_states.emplace_back("Button7", GLFW_MOUSE_BUTTON_8);
}
//------------------------------------------------------------------------------
// ui input handling
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_feedback::multiply(bool value) const noexcept
  -> bool {
    return std::visit(
      overloaded(
        [=](std::monostate) { return value; },
        [=](bool mult) { return mult and value; },
        [=](float mult) { return (mult > 0.F) and value; }),
      constant);
}
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_feedback::multiply(float value) const noexcept
  -> float {
    return std::visit(
      overloaded(
        [=](std::monostate) { return value; },
        [=](bool mult) { return mult ? value : 0.F; },
        [=](float mult) { return mult * value; }),
      constant);
}
//------------------------------------------------------------------------------
template <typename T, typename S>
void glfw3_window_ui_input_feedback::apply_to(T& dst, const input_value<S>& inp)
  const noexcept {
    switch(action) {
        case input_feedback_action::copy:
            copy_to(dst, inp);
            break;
        case input_feedback_action::flip:
            flip(dst);
            break;
        case input_feedback_action::set_zero:
            set_zero(dst);
            break;
        case input_feedback_action::set_one:
            set_one(dst);
            break;
        case input_feedback_action::add:
            add_to(dst);
            break;
        case input_feedback_action::multiply_add:
            multiply_add_to(dst, inp);
            break;
    }
}
//------------------------------------------------------------------------------
void glfw3_window_ui_input_feedback::key_press_changed(
  const execution_context& ctx,
  glfw3_opengl_window& parent,
  const input_variable<bool>& inp) const noexcept {
    if(is_triggered(inp)) {
        parent._ui_input_feedback(ctx, *this, inp);
    }
}
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_feedback::is_under_threshold(
  const input_variable<bool>& inp) const noexcept -> bool {
    return std::visit(
      overloaded(
        [&](std::monostate) { return not inp; },
        [&](bool t) { return t or not inp; },
        [&](float t) { return (t >= 0.5F) or not inp; }),
      threshold);
}
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_feedback::is_over_threshold(
  const input_variable<bool>& inp) const noexcept -> bool {
    return std::visit(
      overloaded(
        [&](std::monostate) { return bool(inp); },
        [&](bool t) { return not t or inp; },
        [&](float t) { return (t <= 0.5F) or inp; }),
      threshold);
}
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_feedback::is_under_threshold(
  const input_variable<float>& inp) const noexcept -> bool {
    return std::visit(
      overloaded(
        [&](std::monostate) { return inp.get() < 0.F; },
        [&](bool t) { return t ? inp.get() <= 1.F : inp.get() <= 0.F; },
        [&](float t) { return (inp.get() <= t); }),
      threshold);
}
//------------------------------------------------------------------------------
auto glfw3_window_ui_input_feedback::is_over_threshold(
  const input_variable<float>& inp) const noexcept -> bool {
    return std::visit(
      overloaded(
        [&](std::monostate) { return inp.get() > 0.F; },
        [&](bool t) { return t ? inp.get() >= 1.F : inp.get() >= 0.F; },
        [&](float t) { return (inp.get() >= t); }),
      threshold);
}
//------------------------------------------------------------------------------
template <typename T>
auto glfw3_window_ui_input_feedback::is_triggered(
  const input_variable<T>& inp) const noexcept -> bool {
    switch(trigger) {
        case input_feedback_trigger::change:
            return true;
        case input_feedback_trigger::under_threshold:
            return is_under_threshold(inp);
        case input_feedback_trigger::over_threshold:
            return is_over_threshold(inp);
        case input_feedback_trigger::zero:
            return not inp;
        case input_feedback_trigger::one:
            return bool(inp);
    }
}
//------------------------------------------------------------------------------
template <typename T>
void glfw3_window_ui_toggle_state::apply_feedback(
  const execution_context& ctx,
  glfw3_opengl_window& parent,
  const glfw3_window_ui_input_state& uis,
  const glfw3_window_ui_input_feedback& fbk,
  const input_value<T>& inp) {
    fbk.apply_to(value, inp);
    if(toggled_on.assign(value)) {
        parent._forward_feedback(ctx, uis, toggled_on);
    }
}
//------------------------------------------------------------------------------
template <typename T>
void glfw3_window_ui_slider_state::apply_feedback(
  const execution_context& ctx,
  glfw3_opengl_window& parent,
  const glfw3_window_ui_input_state& uis,
  const glfw3_window_ui_input_feedback& fbk,
  const input_value<T>& inp) {
    fbk.apply_to(value, inp);
    if(value < min) {
        value = min;
    }
    if(value > max) {
        value = max;
    }
    if(position.assign(value)) {
        parent._forward_feedback(ctx, uis, position);
    }
}
//------------------------------------------------------------------------------
template <typename T>
void glfw3_opengl_window::_forward_feedback(
  const execution_context& ctx,
  const glfw3_window_ui_input_state& input,
  const input_value<T>& value) {
    if(_input_sink) {
        auto& sink = *_input_sink;
        sink.consume(
          {ctx.app_gui_device_id(), input.input_id, input.kind()}, value);
    }
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_feedback_targets::key_press_changed(
  const execution_context& ctx,
  glfw3_opengl_window& parent,
  const input_variable<bool>& inp) const noexcept -> bool {
    if(not targets.empty()) [[likely]] {
        for(auto& target : targets) {
            target.key_press_changed(ctx, parent, inp);
        }
        return true;
    } else {
        return false;
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::_ui_input_feedback(
  const execution_context& ctx,
  const glfw3_window_ui_input_feedback& fbk,
  const input_variable<bool>& inp) noexcept {
    if(const auto target{_find_ui_input(fbk.input_id)}) {
        target->apply([&](auto& ui_input) {
            ui_input.apply_feedback(ctx, *this, *target, fbk, inp);
        });
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::_feedback_key_press_change(
  const execution_context& ctx,
  const identifier device_id,
  const message_id key_id,
  const input_variable<bool>& inp) noexcept {
    if(const auto found{
         find(_ui_feedbacks, std::make_tuple(device_id, key_id))}) {
        if(not found->key_press_changed(ctx, *this, inp)) [[unlikely]] {
            _ui_feedbacks.erase(found.position());
        }
    }
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::add_ui_feedback(
  [[maybe_unused]] const identifier mapping_id, // TODO
  const identifier device_id,
  const message_id signal_id,
  const message_id input_id,
  input_feedback_trigger trigger,
  input_feedback_action action,
  std::variant<std::monostate, bool, float> threshold,
  std::variant<std::monostate, bool, float> constant) noexcept -> bool {
    auto& info = _ui_feedbacks[std::make_tuple(device_id, signal_id)];
    auto pos{
      std::find_if(info.targets.begin(), info.targets.end(), [=](auto& entry) {
          return entry.input_id == input_id;
      })};
    if(pos == info.targets.end()) {
        pos = info.targets.insert(
          pos, glfw3_window_ui_input_feedback{.input_id = input_id});
    }
    pos->threshold = threshold;
    pos->constant = constant;
    pos->trigger = trigger;
    pos->action = action;
    return true;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::_setup_ui_input(
  message_id input_id,
  glfw3_window_ui_state_variant state) -> bool {
    const auto pos{std::find_if(
      _ui_input_states.begin(), _ui_input_states.end(), [=](const auto& entry) {
          return entry.input_id == input_id;
      })};
    if(pos != _ui_input_states.end()) {
        pos->state = std::move(state);
        return true;
    } else {
        _ui_input_states.push_back(
          {.input_id = input_id, .state = std::move(state)});
        return true;
    }
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::_find_ui_input(message_id input_id) noexcept
  -> optional_reference<glfw3_window_ui_input_state> {
    const auto pos{std::find_if(
      _ui_input_states.begin(), _ui_input_states.end(), [=](const auto& entry) {
          return entry.input_id == input_id;
      })};
    if(pos != _ui_input_states.end()) {
        return &(*pos);
    }
    return {};
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::add_ui_button(
  [[maybe_unused]] const message_id input_id,
  [[maybe_unused]] const string_view label) -> bool {
    return _setup_ui_input(
      input_id, glfw3_window_ui_button_state{.label = label.to_string()});
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::add_ui_toggle(
  const message_id input_id,
  const string_view label,
  bool initial) -> bool {
    return _setup_ui_input(
      input_id,
      glfw3_window_ui_toggle_state{
        .toggled_on = initial, .label = label.to_string(), .value = initial});
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::set_ui_toggle(
  const message_id input_id,
  bool value) noexcept -> bool {
    if(auto found{_find_ui_input(input_id)}) {
        if(auto toggle{found->get_toggle()}) {
            toggle->toggled_on = value;
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::add_ui_slider(
  const message_id input_id,
  const string_view label,
  float min,
  float max,
  float initial,
  input_value_kind kind) -> bool {
    return _setup_ui_input(
      input_id,
      glfw3_window_ui_slider_state{
        .position = initial,
        .label = label.to_string(),
        .min = min,
        .max = max,
        .value = initial,
        .kind = kind});
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::set_ui_slider(
  const message_id input_id,
  float value) noexcept -> bool {
    if(auto found{_find_ui_input(input_id)}) {
        if(auto slider{found->get_slider()}) {
            slider->value = value;
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
glfw3_opengl_window::glfw3_opengl_window(
  application_config& cfg,
  const identifier instance_id,
  glfw3_opengl_provider& parent)
  : glfw3_opengl_window{cfg, instance_id, instance_id.name(), parent} {}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::handle_progress() noexcept -> bool {
    bool result = false;
    if(_window) {
        result = glfwGetKey(_window, GLFW_KEY_ESCAPE) != GLFW_PRESS;
    }
    return result;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::initialize(
  const launch_options& options,
  const video_options& video_opts,
  const span<GLFWmonitor* const> monitors) -> bool {
    if(const auto ver_maj{video_opts.gl_version_major()}) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, *ver_maj);
    }
    if(const auto ver_min{video_opts.gl_version_minor()}) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, *ver_min);
    }
    const auto compat = video_opts.gl_compatibility_context();
    if(compat) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        log_debug("using compatibility GL context")
          .arg("instance", _instance_id);
    } else if(not compat) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        log_debug("using core GL context").arg("instance", _instance_id);
    }
    if(video_opts.gl_debug_context()) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        log_debug("using debugging GL context").arg("instance", _instance_id);
    }

    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    glfwWindowHint(
      GLFW_RED_BITS, video_opts.color_bits().value_or(GLFW_DONT_CARE));
    glfwWindowHint(
      GLFW_BLUE_BITS, video_opts.color_bits().value_or(GLFW_DONT_CARE));
    glfwWindowHint(
      GLFW_GREEN_BITS, video_opts.color_bits().value_or(GLFW_DONT_CARE));
    glfwWindowHint(
      GLFW_ALPHA_BITS, video_opts.alpha_bits().value_or(GLFW_DONT_CARE));
    glfwWindowHint(
      GLFW_DEPTH_BITS, video_opts.depth_bits().value_or(GLFW_DONT_CARE));
    glfwWindowHint(
      GLFW_STENCIL_BITS, video_opts.stencil_bits().value_or(GLFW_DONT_CARE));

    glfwWindowHint(GLFW_SAMPLES, video_opts.samples().value_or(GLFW_DONT_CARE));

    GLFWmonitor* window_monitor = nullptr;
    int fallback_width = 1280, fallback_height = 800;
    if(video_opts.fullscreen()) {
        window_monitor = glfwGetPrimaryMonitor();
        if(const auto opt_mon_name{video_opts.display_name()}) {
            for(auto monitor : monitors) {
                string_view mon_name(glfwGetMonitorName(monitor));
                if(are_equal(*opt_mon_name, mon_name)) {
                    window_monitor = monitor;
                }
            }
        }
    }
    if(const auto mode{glfwGetVideoMode(
         window_monitor ? window_monitor : glfwGetPrimaryMonitor())}) {
        fallback_width = mode->width;
        fallback_height = mode->height;
    }

    if(video_opts.offscreen()) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    _window = glfwCreateWindow(
      video_opts.surface_width().value_or(fallback_width),
      video_opts.surface_height().value_or(fallback_height),
      c_str(options.application_title()),
      window_monitor,
      nullptr);

    if(_window) {
        glfwSetWindowUserPointer(_window, this);
        glfwSetScrollCallback(_window, &glfw3_opengl_window_scroll_callback);
        glfwSetWindowTitle(_window, c_str(options.application_title()));
        glfwGetWindowSize(_window, &_window_width, &_window_height);
        if(_window_width > 0 and _window_height > 0) {
            _norm_x_ndc = 1.F / float(_window_width);
            _norm_y_ndc = 1.F / float(_window_height);
            _aspect = _norm_y_ndc / _norm_x_ndc;
        }
        if(_gui.imgui.create_context) {
            glfwMakeContextCurrent(_window);
            _gui.imgui.create_context() >> _imgui_context;
            _gui.imgui.set_config_flags(_gui.imgui.config_nav_enable_keyboard);
            _gui.imgui.style_colors_dark();

            _gui.imgui.glfw_init_for_opengl(_window, true);
            _gui.imgui.opengl3_init("#version 150");
            glfwMakeContextCurrent(nullptr);
        }
        return true;
    } else {
        log_error("Failed to create GLFW window").arg("instance", _instance_id);
    }
    return false;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::video_kind() const noexcept -> video_context_kind {
    return video_context_kind::opengl;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::instance_id() const noexcept -> identifier {
    return _instance_id;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::is_offscreen() noexcept -> tribool {
    if(_window) {
        return glfwGetWindowAttrib(_window, GLFW_VISIBLE) == 0;
    }
    return indeterminate;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::has_framebuffer() noexcept -> tribool {
    if(_window) {
        return glfwGetWindowAttrib(_window, GLFW_VISIBLE) != 0;
    }
    return indeterminate;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::surface_size() noexcept -> std::tuple<int, int> {
    return {_window_width, _window_height};
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::surface_aspect() noexcept -> float {
    return _aspect;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::egl_ref() noexcept -> eglplus::egl_api_reference {
    return {};
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::egl_display() noexcept -> eglplus::display_handle {
    return {};
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::imgui_ref() noexcept -> guiplus::imgui_api_reference {
    return {_gui.imgui};
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::parent_context_changed(const video_context& vctx) {
    _parent_context = &vctx;
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::video_begin(execution_context&) {
    assert(_window);
    glfwMakeContextCurrent(_window);
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::video_end(execution_context&) {
    glfwMakeContextCurrent(nullptr);
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::video_commit(execution_context&) {
    assert(_window);
    if(_imgui_updated) {
        if(const ok draw_data{_gui.imgui.get_draw_data()}) {
            _gui.imgui.opengl3_render_draw_data(draw_data);
        }
    }
    glfwSwapBuffers(_window);
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::input_enumerate(
  execution_context& ctx,
  const callable_ref<
    void(const identifier, const message_id, const input_value_kinds) noexcept>
    callback) noexcept {
    const auto kb_id{ctx.keyboard_device_id()};
    const auto mouse_id{ctx.mouse_device_id()};
    // keyboard inputs
    for(const auto& ks : _key_states) {
        callback(
          kb_id, message_id{"Key", ks.key_id}, input_value_kind::absolute_norm);
    }

    // cursor device inputs
    for(const auto& ks : _mouse_states) {
        callback(
          mouse_id,
          message_id{"Cursor", ks.key_id},
          input_value_kind::absolute_norm);
    }

    callback(
      mouse_id,
      message_id{"Cursor", "PositionX"},
      input_value_kind::absolute_free | input_value_kind::absolute_norm);
    callback(
      mouse_id,
      message_id{"Cursor", "PositionY"},
      input_value_kind::absolute_free | input_value_kind::absolute_norm);

    callback(
      mouse_id, message_id{"Cursor", "MotionX"}, input_value_kind::relative);
    callback(
      mouse_id, message_id{"Cursor", "MotionY"}, input_value_kind::relative);

    // wheel inputs
    callback(
      mouse_id, message_id{"Wheel", "ScrollX"}, input_value_kind::relative);
    callback(
      mouse_id, message_id{"Wheel", "ScrollY"}, input_value_kind::relative);

    // ui input
    const identifier gui_id{ctx.app_gui_device_id()};
    for(const auto& entry : _ui_input_states) {
        callback(gui_id, entry.input_id, entry.kind());
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::input_connect(input_sink& sink) {
    _input_sink = std::addressof(sink);
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::input_disconnect() {
    _input_sink = nullptr;
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::mapping_begin(
  [[maybe_unused]] const identifier setup_id) {
    _enabled_signals.clear();
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::mapping_enable(
  const identifier device_id,
  const message_id signal_id) {
    _enabled_signals.insert(std::make_tuple(device_id, signal_id));
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::mapping_commit(
  execution_context& ctx,
  [[maybe_unused]] const identifier setup_id) {
    const auto kb_id{ctx.keyboard_device_id()};
    const auto mouse_id{ctx.mouse_device_id()};
    for(auto& ks : _key_states) {
        const auto key{std::make_tuple(kb_id, message_id{"Key", ks.key_id})};
        ks.enabled =
          _ui_feedbacks.contains(key) or _enabled_signals.contains(key);
    }

    for(auto& ks : _mouse_states) {
        ks.enabled =
          _enabled_signals.contains({mouse_id, {"Cursor", ks.key_id}});
    }

    _mouse_enabled =
      _enabled_signals.contains({mouse_id, {"Cursor", "PositionX"}}) or
      _enabled_signals.contains({mouse_id, {"Cursor", "PositionY"}}) or
      _enabled_signals.contains({mouse_id, {"Cursor", "MotionX"}}) or
      _enabled_signals.contains({mouse_id, {"Cursor", "MotionY"}});
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::update_gui(
  execution_context& exec_ctx,
  application& app) {
    _imgui_updated = false;
    if(_gui.imgui.begin) {
        assert(_parent_context);
        const auto activities{_provider.activities()};
        const auto& par_ctx = *_parent_context;
        const auto& state = exec_ctx.state();
        const auto frame_dur = state.frame_duration().value();
        const auto frames_per_second = state.frames_per_second();

        _gui.imgui.opengl3_new_frame();
        _gui.imgui.glfw_new_frame();
        _gui.imgui.new_frame();

        if(not activities.empty()) {
            bool visible{true};
            _gui.imgui.set_next_window_size({float(_window_width) * 0.8F, 0.F});
            if(_gui.imgui.begin(
                 "Activities", visible, _gui.imgui.window_no_resize)) {
                for(const auto& info : activities) {
                    const auto progress =
                      float(info.current_steps) / float(info.total_steps);
                    _gui.imgui.progress_bar(progress, info.title);
                }
                _gui.imgui.end();
            }
        }

        app.update_overlays(_gui);
        if(_imgui_visible) {
            if(_gui.imgui.begin(
                 "Application", &_imgui_visible, _gui.imgui.window_no_resize)) {
                _gui.imgui.text_buffered(
                  _format_buffer,
                  "Dimensions: {}x{}",
                  _window_width,
                  _window_height);

                _gui.imgui.text_buffered(
                  _format_buffer, "Frame number: {}", par_ctx.frame_number());
                _gui.imgui.text_buffered(
                  _format_buffer,
                  "Frame time: {:.2f} [ms]",
                  frame_dur * 1000.F);
                _gui.imgui.text_buffered(
                  _format_buffer,
                  "Frames per second: {:.1f}",
                  frames_per_second);
                _gui.imgui.text_buffered(
                  _format_buffer,
                  "Activities in progress: {}",
                  _provider.activities().size());

                if(_input_sink) {
                    auto& sink = *_input_sink;
                    const identifier gui_id{exec_ctx.app_gui_device_id()};
                    for(auto& entry : _ui_input_states) {
                        entry.apply(overloaded(
                          [&, this](glfw3_window_ui_button_state& button) {
                              if(button.pressed.assign(
                                   _gui.imgui.button(button.label).or_false())) {
                                  sink.consume(
                                    {gui_id, entry.input_id, entry.kind()},
                                    button.pressed);
                              }
                          },
                          [&, this](glfw3_window_ui_toggle_state& toggle) {
                              _gui.imgui.checkbox(toggle.label, toggle.value);
                              if(toggle.toggled_on.assign(toggle.value)) {
                                  sink.consume(
                                    {gui_id, entry.input_id, entry.kind()},
                                    toggle.toggled_on);
                              }
                          },
                          [&, this](glfw3_window_ui_slider_state& slider) {
                              if(_gui.imgui
                                   .slider_float(
                                     slider.label,
                                     slider.value,
                                     slider.min,
                                     slider.max)
                                   .or_false()) {
                                  if(slider.position.assign(slider.value)) {
                                      sink.consume(
                                        {gui_id, entry.input_id, entry.kind()},
                                        slider.position);
                                  }
                              }
                          }));
                    }
                }

                if(_gui.imgui.button("Hide").or_false()) {
                    _imgui_visible = false;
                }
                _gui.imgui.same_line();
                if(_gui.imgui.button("Quit").or_false()) {
                    glfwSetWindowShouldClose(_window, GLFW_TRUE);
                }
                if(_gui.imgui.is_item_hovered().or_false()) {
                    _gui.imgui.begin_tooltip();
                    _gui.imgui.text_unformatted("Closes the application");
                    _gui.imgui.end_tooltip();
                }
                _gui.imgui.end();
            }
            app.update_gui(_gui.imgui);
        }
        _gui.imgui.end_frame();
        _gui.imgui.render();
        _imgui_updated = true;
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::update_glfw(
  execution_context& exec_ctx,
  application& app) {
    if(glfwWindowShouldClose(_window)) {
        exec_ctx.stop_running();
    } else {
        glfwGetWindowSize(_window, &_window_width, &_window_height);

        if(_input_sink) {
            auto& sink = *_input_sink;
            const auto kb_id{exec_ctx.keyboard_device_id()};
            const auto mouse_id{exec_ctx.mouse_device_id()};

            if(_wheel_scroll_x.assign(_wheel_change_x)) {
                sink.consume(
                  {mouse_id, {"Wheel", "ScrollX"}, input_value_kind::relative},
                  _wheel_scroll_x);
            }
            _wheel_change_x = 0;

            if(_wheel_scroll_y.assign(_wheel_change_y)) {
                sink.consume(
                  {mouse_id, {"Wheel", "ScrollY"}, input_value_kind::relative},
                  _wheel_scroll_y);
            }
            _wheel_change_y = 0;

            if(_mouse_enabled) {
                const auto motion_adjust = 1.1;
                double mouse_x_pix{0}, mouse_y_pix{0};
                glfwGetCursorPos(_window, &mouse_x_pix, &mouse_y_pix);
                mouse_y_pix = _window_height - mouse_y_pix;

                if(_mouse_x_pix.assign(float(mouse_x_pix))) {
                    sink.consume(
                      {mouse_id,
                       {"Cursor", "PositionX"},
                       input_value_kind::absolute_free},
                      _mouse_x_pix);
                    if(_mouse_x_ndc.assign(
                         float(mouse_x_pix * _norm_x_ndc) - 0.5F)) {
                        sink.consume(
                          {mouse_id,
                           {"Cursor", "PositionX"},
                           input_value_kind::absolute_norm},
                          _mouse_x_ndc);
                        if(_mouse_x_delta.assign(float(
                             _mouse_x_ndc.delta() * motion_adjust * _aspect))) {
                            sink.consume(
                              {mouse_id,
                               {"Cursor", "MotionX"},
                               input_value_kind::relative},
                              _mouse_x_delta);
                        }
                    }
                }
                if(_mouse_y_pix.assign(float(mouse_y_pix))) {
                    sink.consume(
                      {mouse_id,
                       {"Cursor", "PositionY"},
                       input_value_kind::absolute_free},
                      _mouse_y_pix);
                    if(_mouse_y_ndc.assign(
                         float(mouse_y_pix * _norm_y_ndc) - 0.5F)) {
                        sink.consume(
                          {mouse_id,
                           {"Cursor", "PositionY"},
                           input_value_kind::absolute_norm},
                          _mouse_y_ndc);
                        if(_mouse_y_delta.assign(
                             float(_mouse_y_ndc.delta() * motion_adjust))) {
                            sink.consume(
                              {mouse_id,
                               {"Cursor", "MotionY"},
                               input_value_kind::relative},
                              _mouse_y_delta);
                        }
                    }
                }

                if(not _imgui_visible) {
                    for(auto& ks : _mouse_states) {
                        if(ks.enabled) {
                            if(ks.pressed.assign(
                                 glfwGetMouseButton(_window, ks.key_code) ==
                                 GLFW_PRESS)) {
                                sink.consume(
                                  {mouse_id,
                                   {"Cursor", ks.key_id},
                                   input_value_kind::absolute_norm},
                                  ks.pressed);
                            }
                        }
                    }
                }
            }
            if(not _imgui_visible) {
                for(auto& ks : _key_states) {
                    if(ks.enabled) {
                        const auto state = glfwGetKey(_window, ks.key_code);
                        const auto press = state == GLFW_PRESS;
                        if(ks.pressed.assign(press) or press) {
                            const message_id key_id{"Key", ks.key_id};
                            sink.consume(
                              {kb_id, key_id, input_value_kind::absolute_norm},
                              ks.pressed);
                            _feedback_key_press_change(
                              exec_ctx, kb_id, key_id, ks.pressed);
                        }
                    }
                }
            }

            const auto backtick_is_pressed =
              glfwGetKey(_window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS;
            if(_backtick_was_pressed != backtick_is_pressed) {
                if(_backtick_was_pressed) {
                    _imgui_visible = not _imgui_visible;
                }
                _backtick_was_pressed = backtick_is_pressed;
            }
        }
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::update(execution_context& exec_ctx, application& app) {
    update_gui(exec_ctx, app);
    update_glfw(exec_ctx, app);
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::clean_up() {
    if(_window) {
        if(_imgui_updated) {
            _gui.imgui.opengl3_shutdown();
            _gui.imgui.glfw_shutdown();
            _gui.imgui.destroy_context(_imgui_context);
        }
        glfwDestroyWindow(_window);
    }
}
#endif // EAGINE_APP_HAS_GLFW3
//------------------------------------------------------------------------------
// glfw3_opengl_provider
//------------------------------------------------------------------------------
glfw3_opengl_provider::glfw3_opengl_provider(main_ctx_parent parent)
  : main_ctx_object{"GLFW3Prvdr", parent} {
    set_progress_update_callback(
      main_context(), _get_progress_callback(), std::chrono::milliseconds{100});
    register_progress_observer(main_context(), *this);
}
//------------------------------------------------------------------------------
glfw3_opengl_provider::~glfw3_opengl_provider() noexcept {
    unregister_progress_observer(main_context(), *this);
    reset_progress_update_callback(main_context());
}
//------------------------------------------------------------------------------
auto glfw3_opengl_provider::_get_progress_callback() noexcept
  -> callable_ref<bool() noexcept> {
    return make_callable_ref<&glfw3_opengl_provider::_handle_progress>(this);
}
//------------------------------------------------------------------------------
auto glfw3_opengl_provider::_handle_progress() noexcept -> bool {
#if EAGINE_APP_HAS_GLFW3
    glfwPollEvents();
    for(auto& entry : _windows) {
        assert(std::get<1>(entry));
        if(not std::get<1>(entry)->handle_progress()) {
            return false;
        }
    }
#endif
    return true;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_provider::is_implemented() const noexcept -> bool {
    return EAGINE_APP_HAS_GLFW3 != 0;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_provider::implementation_name() const noexcept
  -> string_view {
    return {"glfw3"};
}
//------------------------------------------------------------------------------
auto glfw3_opengl_provider::is_initialized() -> bool {
#if EAGINE_APP_HAS_GLFW3
    return not _windows.empty();
#endif
    return false;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_provider::should_initialize(
  [[maybe_unused]] execution_context& exec_ctx) -> bool {
#if EAGINE_APP_HAS_GLFW3
    for(auto& entry : exec_ctx.options().video_requirements()) {
        auto& video_opts = std::get<1>(entry);
        if(video_opts.has_provider(implementation_name())) {
            return true;
        }
    }
#endif
    return false;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_provider::initialize(execution_context& exec_ctx) -> bool {
#if EAGINE_APP_HAS_GLFW3
    if(glfwInit()) {
        const auto monitors{[] {
            int monitor_count = 0;
            auto* monitor_list = glfwGetMonitors(&monitor_count);
            return memory::view(monitor_list, monitor_count);
        }()};

        log_info("GLFW monitors").arg_func([monitors](logger_backend& backend) {
            for(auto monitor : monitors) {
                backend.add_string(
                  "monitors",
                  "string",
                  string_view(glfwGetMonitorName(monitor)));
            }
        });

        auto& options = exec_ctx.options();
        for(auto& [inst, video_opts] : options.video_requirements()) {
            const bool should_create_window =
              video_opts.has_provider(implementation_name()) and
              (video_opts.video_kind() == video_context_kind::opengl);

            if(should_create_window) {
                if(const auto new_win{std::make_shared<glfw3_opengl_window>(
                     this->main_context().config(), inst, *this)}) {
                    if(new_win->initialize(options, video_opts, monitors)) {
                        _windows[inst] = new_win;
                    } else {
                        new_win->clean_up();
                    }
                }
            }
        }
        return true;
    } else {
        exec_ctx.log_error("GLFW initialization error");
    }
#endif // EAGINE_APP_HAS_GLFW3
    exec_ctx.log_error("GLFW is context is not supported");
    return false;
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::update(
  [[maybe_unused]] execution_context& exec_ctx,
  [[maybe_unused]] application& app) {
#if EAGINE_APP_HAS_GLFW3
    glfwPollEvents();
    for(auto& entry : _windows) {
        entry.second->update(exec_ctx, app);
    }
#endif // EAGINE_APP_HAS_GLFW3
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::clean_up(execution_context&) {
#if EAGINE_APP_HAS_GLFW3
    for(auto& entry : _windows) {
        entry.second->clean_up();
    }
    glfwTerminate();
#endif // EAGINE_APP_HAS_GLFW3
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::input_enumerate(
  [[maybe_unused]] callable_ref<void(shared_holder<input_provider>)> handler) {
#if EAGINE_APP_HAS_GLFW3
    for(auto& p : _windows) {
        handler(p.second);
    }
#endif // EAGINE_APP_HAS_GLFW3
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::video_enumerate(
  [[maybe_unused]] callable_ref<void(shared_holder<video_provider>)> handler) {
#if EAGINE_APP_HAS_GLFW3
    for(auto& p : _windows) {
        handler(p.second);
    }
#endif // EAGINE_APP_HAS_GLFW3
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::audio_enumerate(
  callable_ref<void(shared_holder<audio_provider>)>) {}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::activity_begun(
  [[maybe_unused]] const activity_progress_id_t parent_id,
  [[maybe_unused]] const activity_progress_id_t activity_id,
  [[maybe_unused]] const string_view title,
  [[maybe_unused]] const span_size_t total_steps) noexcept {
#if EAGINE_APP_HAS_GLFW3
    _activities.emplace_back();
    auto& info = _activities.back();
    info.activity_id = activity_id;
    info.parent_id = parent_id;
    info.total_steps = total_steps;
    info.title = to_string(title);
#endif
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::activity_finished(
  [[maybe_unused]] const activity_progress_id_t parent_id,
  [[maybe_unused]] const activity_progress_id_t activity_id,
  [[maybe_unused]] const string_view title,
  [[maybe_unused]] span_size_t total_steps) noexcept {
#if EAGINE_APP_HAS_GLFW3
    std::erase_if(_activities, [activity_id](const auto& activity) -> bool {
        return activity.activity_id == activity_id;
    });
#endif
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::activity_updated(
  [[maybe_unused]] const activity_progress_id_t parent_id,
  [[maybe_unused]] const activity_progress_id_t activity_id,
  [[maybe_unused]] const span_size_t current_steps,
  [[maybe_unused]] const span_size_t total_steps) noexcept {
#if EAGINE_APP_HAS_GLFW3
    const auto pos = std::find_if(
      _activities.begin(),
      _activities.end(),
      [activity_id](const auto& activity) -> bool {
          return activity.activity_id == activity_id;
      });
    if(pos != _activities.end()) [[likely]] {
        pos->current_steps = current_steps;
    }
#endif
}
//------------------------------------------------------------------------------
auto make_glfw3_opengl_provider(main_ctx_parent parent)
  -> shared_holder<hmi_provider> {
    return {std::make_shared<glfw3_opengl_provider>(parent)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
