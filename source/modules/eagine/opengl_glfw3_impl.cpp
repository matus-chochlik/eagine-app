/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

#if __has_include(<GLFW/glfw3.h>)
#include <GLFW/glfw3.h>
#define EAGINE_APP_HAS_GLFW3 1
#else
#define EAGINE_APP_HAS_GLFW3 0
#endif

#if __has_include(<imgui.h>)
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#define EAGINE_APP_HAS_IMGUI 1
#else
#define EAGINE_APP_HAS_IMGUI 0
#endif

module eagine.app;

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
import <cmath>;
import <string>;
import <variant>;
import <vector>;
import <map>;

import <iostream>;
namespace eagine::app {
//------------------------------------------------------------------------------
#if EAGINE_APP_HAS_GLFW3
#if EAGINE_APP_HAS_IMGUI
struct glfw3_activity_progress_info {
    activity_progress_id_t activity_id{0};
    activity_progress_id_t parent_id{0};
    span_size_t current_steps{0};
    span_size_t total_steps{0};
    std::string title;
};
#endif
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

    void update(execution_context&);

    void clean_up();

    auto video_kind() const noexcept -> video_context_kind final;
    auto instance_id() const noexcept -> identifier final;

    auto is_offscreen() noexcept -> tribool final;
    auto has_framebuffer() noexcept -> tribool final;
    auto surface_size() noexcept -> std::tuple<int, int> final;
    auto surface_aspect() noexcept -> float final;

    void parent_context_changed(const video_context&) final;
    void video_begin(execution_context&) final;
    void video_end(execution_context&) final;
    void video_commit(execution_context&) final;

    void input_enumerate(const callable_ref<void(
                           const message_id,
                           const input_value_kinds) noexcept>) noexcept final;

    void input_connect(input_sink&) final;
    void input_disconnect() final;

    void mapping_begin(const identifier setup_id) final;
    void mapping_enable(const message_id signal_id) final;
    void mapping_commit(const identifier setup_id) final;

    auto add_ui_feedback(
      const message_id signal_id,
      const message_id input_id,
      input_feedback_trigger,
      input_feedback_action,
      std::variant<std::monostate, bool, float> threshold,
      std::variant<std::monostate, bool, float> multiplier) noexcept
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
    application_config_value<bool> _imgui_enabled;

    GLFWwindow* _window{nullptr};
    input_sink* _input_sink{nullptr};
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

    flat_set<message_id> _enabled_signals;

    std::vector<key_state> _key_states;
    std::vector<key_state> _mouse_states;

#if EAGINE_APP_HAS_IMGUI
    struct ui_input_feedback {
        message_id input_id;
        std::variant<std::monostate, bool, float> threshold;
        std::variant<std::monostate, bool, float> multiplier;
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
            dst = !dst;
        }
        void flip(float&) const noexcept {
            // TODO: what does this mean?
        }

        void multiply_add_to(bool& dst, const input_value<bool>& inp)
          const noexcept {
            dst = dst || multiply(inp.get());
        }
        void multiply_add_to(bool& dst, const input_value<float>& inp)
          const noexcept {
            dst = dst || (std::abs(multiply(inp.get())) > 0.F);
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

    friend struct ui_input_feedback;
    struct ui_button_state {
        input_variable<bool> pressed{false};
        std::string label;

        template <typename T>
        void apply_feedback(const ui_input_feedback&, const input_value<T>&);
    };

    struct ui_toggle_state {
        input_variable<bool> toggled_on{false};
        std::string label;
        bool value{false};

        template <typename T>
        void apply_feedback(const ui_input_feedback&, const input_value<T>&);
    };

    struct ui_slider_state {
        input_variable<float> position{0.5F};
        std::string label;
        float min{0.F};
        float max{1.F};
        float value{0.5F};
        input_value_kind kind{input_value_kind::absolute_norm};

        template <typename T>
        void apply_feedback(const ui_input_feedback&, const input_value<T>&);
    };

    using ui_state_variant =
      std::variant<ui_button_state, ui_toggle_state, ui_slider_state>;

    struct ui_input_state {
        message_id input_id;
        ui_state_variant state;

        auto kind() const noexcept -> input_value_kind;

        auto get_toggle() noexcept -> ui_toggle_state*;
        auto get_slider() noexcept -> ui_slider_state*;

        auto apply(const auto& func) noexcept {
            std::visit(func, state);
        }
    };

    auto _setup_ui_input(message_id input_id, ui_state_variant state) -> bool;
    auto _find_ui_input(message_id input_id) noexcept -> ui_input_state*;

    std::vector<ui_input_state> _ui_input_states;

    struct ui_feedback_targets {
        std::vector<ui_input_feedback> targets;

        auto key_press_changed(
          glfw3_opengl_window& parent,
          const input_variable<bool>&) const noexcept -> bool;
    };

    void _ui_input_feedback(
      const ui_input_feedback&,
      const input_variable<bool>&) noexcept;

    void _feedback_key_press_change(
      message_id key_id,
      const input_variable<bool>&) noexcept;

    flat_map<message_id, ui_feedback_targets> _ui_feedbacks;
#endif

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
    void update(execution_context&) final;
    void clean_up(execution_context&) final;

    void input_enumerate(
      callable_ref<void(std::shared_ptr<input_provider>)>) final;
    void video_enumerate(
      callable_ref<void(std::shared_ptr<video_provider>)>) final;
    void audio_enumerate(
      callable_ref<void(std::shared_ptr<audio_provider>)>) final;

#if EAGINE_APP_HAS_IMGUI
    auto activities() const noexcept
      -> span<const glfw3_activity_progress_info> {
        return view(_activities);
    }
#endif

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
    std::map<identifier, std::shared_ptr<glfw3_opengl_window>> _windows;
#if EAGINE_APP_HAS_IMGUI
    std::vector<glfw3_activity_progress_info> _activities;
#endif
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
  , _instance_id{instance_id}
  , _imgui_enabled{
      c,
      "application.imgui.enable",
      instance,
      EAGINE_APP_HAS_IMGUI != 0} {

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
    _mouse_states.emplace_back("Pressed", GLFW_MOUSE_BUTTON_1);
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
#if EAGINE_APP_HAS_IMGUI
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_feedback::multiply(bool value) const noexcept
  -> bool {
    return std::visit(
      overloaded(
        [=](std::monostate) { return value; },
        [=](bool mult) { return mult && value; },
        [=](float mult) { return (mult > 0.F) && value; }),
      multiplier);
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_feedback::multiply(float value) const noexcept
  -> float {
    return std::visit(
      overloaded(
        [=](std::monostate) { return value; },
        [=](bool mult) { return mult ? value : 0.F; },
        [=](float mult) { return mult * value; }),
      multiplier);
}
//------------------------------------------------------------------------------
template <typename T, typename S>
void glfw3_opengl_window::ui_input_feedback::apply_to(
  T& dst,
  const input_value<S>& inp) const noexcept {
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
        case input_feedback_action::multiply_add:
            multiply_add_to(dst, inp);
            break;
    }
}
//------------------------------------------------------------------------------
template <typename T>
void glfw3_opengl_window::ui_button_state::apply_feedback(
  const ui_input_feedback&,
  const input_value<T>&) {}
//------------------------------------------------------------------------------
template <typename T>
void glfw3_opengl_window::ui_toggle_state::apply_feedback(
  const ui_input_feedback& fbk,
  const input_value<T>& inp) {
    fbk.apply_to(value, inp);
}
//------------------------------------------------------------------------------
template <typename T>
void glfw3_opengl_window::ui_slider_state::apply_feedback(
  const ui_input_feedback& fbk,
  const input_value<T>& inp) {
    fbk.apply_to(value, inp);
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_state::kind() const noexcept
  -> input_value_kind {
    if(std::holds_alternative<ui_slider_state>(state)) {
        return std::get<ui_slider_state>(state).kind;
    }
    return input_value_kind::absolute_norm;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_state::get_toggle() noexcept
  -> glfw3_opengl_window::ui_toggle_state* {
    if(std::holds_alternative<ui_toggle_state>(state)) {
        return &(std::get<ui_toggle_state>(state));
    }
    return nullptr;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_state::get_slider() noexcept
  -> glfw3_opengl_window::ui_slider_state* {
    if(std::holds_alternative<ui_slider_state>(state)) {
        return &(std::get<ui_slider_state>(state));
    }
    return nullptr;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_feedback::is_under_threshold(
  const input_variable<bool>& inp) const noexcept -> bool {
    return std::visit(
      overloaded(
        [&](std::monostate) { return !inp; },
        [&](bool t) { return t || !inp; },
        [&](float t) { return (t >= 0.5F) || !inp; }),
      threshold);
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_feedback::is_over_threshold(
  const input_variable<bool>& inp) const noexcept -> bool {
    return std::visit(
      overloaded(
        [&](std::monostate) { return bool(inp); },
        [&](bool t) { return !t || inp; },
        [&](float t) { return (t <= 0.5F) || inp; }),
      threshold);
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_feedback::is_under_threshold(
  const input_variable<float>& inp) const noexcept -> bool {
    return std::visit(
      overloaded(
        [&](std::monostate) { return inp.get() < 0.F; },
        [&](bool t) { return t ? inp.get() <= 1.F : inp.get() <= 0.F; },
        [&](float t) { return (inp.get() <= t); }),
      threshold);
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_input_feedback::is_over_threshold(
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
auto glfw3_opengl_window::ui_input_feedback::is_triggered(
  const input_variable<T>& inp) const noexcept -> bool {
    switch(trigger) {
        case input_feedback_trigger::change:
            return true;
        case input_feedback_trigger::under_threshold:
            return is_under_threshold(inp);
        case input_feedback_trigger::over_threshold:
            return is_over_threshold(inp);
        case input_feedback_trigger::zero:
            return !inp;
        case input_feedback_trigger::one:
            return bool(inp);
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::ui_input_feedback::key_press_changed(
  glfw3_opengl_window& parent,
  const input_variable<bool>& inp) const noexcept {
    if(is_triggered(inp)) {
        parent._ui_input_feedback(*this, inp);
    }
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::ui_feedback_targets::key_press_changed(
  glfw3_opengl_window& parent,
  const input_variable<bool>& inp) const noexcept -> bool {
    if(!targets.empty()) [[likely]] {
        for(auto& target : targets) {
            target.key_press_changed(parent, inp);
        }
        return true;
    } else {
        return false;
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::_ui_input_feedback(
  const ui_input_feedback& fbk,
  const input_variable<bool>& inp) noexcept {
    if(const auto target{_find_ui_input(fbk.input_id)}) {
        extract(target).apply(
          [&](auto& ui_input) { ui_input.apply_feedback(fbk, inp); });
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::_feedback_key_press_change(
  message_id key_id,
  const input_variable<bool>& inp) noexcept {
    if(const auto pos{_ui_feedbacks.find(key_id)}; pos != _ui_feedbacks.end()) {
        if(!pos->second.key_press_changed(*this, inp)) [[unlikely]] {
            _ui_feedbacks.erase(pos);
        }
    }
}
//------------------------------------------------------------------------------
#endif
auto glfw3_opengl_window::add_ui_feedback(
  const message_id signal_id,
  const message_id input_id,
  input_feedback_trigger trigger,
  input_feedback_action action,
  std::variant<std::monostate, bool, float> threshold,
  std::variant<std::monostate, bool, float> multiplier) noexcept -> bool {
#if EAGINE_APP_HAS_IMGUI
    auto& info = _ui_feedbacks[signal_id];
    auto pos{
      std::find_if(info.targets.begin(), info.targets.end(), [=](auto& entry) {
          return entry.input_id == input_id;
      })};
    if(pos == info.targets.end()) {
        pos = info.targets.insert(pos, ui_input_feedback{.input_id = input_id});
    }
    pos->threshold = threshold;
    pos->threshold = multiplier;
    pos->trigger = trigger;
    pos->action = action;
    return true;
#else
    return false;
#endif
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::_setup_ui_input(
  message_id input_id,
  ui_state_variant state) -> bool {
#if EAGINE_APP_HAS_IMGUI
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
#else
    return false;
#endif
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::_find_ui_input(message_id input_id) noexcept
  -> ui_input_state* {
#if EAGINE_APP_HAS_IMGUI
    const auto pos{std::find_if(
      _ui_input_states.begin(), _ui_input_states.end(), [=](const auto& entry) {
          return entry.input_id == input_id;
      })};
    if(pos != _ui_input_states.end()) {
        return &(*pos);
    }
#endif
    return nullptr;
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::add_ui_button(
  [[maybe_unused]] const message_id input_id,
  [[maybe_unused]] const string_view label) -> bool {
#if EAGINE_APP_HAS_IMGUI
    return _setup_ui_input(
      input_id, ui_button_state{.label = label.to_string()});
#else
    return false;
#endif
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::add_ui_toggle(
  const message_id input_id,
  const string_view label,
  bool initial) -> bool {
#if EAGINE_APP_HAS_IMGUI
    return _setup_ui_input(
      input_id,
      ui_toggle_state{
        .toggled_on = initial, .label = label.to_string(), .value = initial});
#else
    return false;
#endif
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::set_ui_toggle(
  const message_id input_id,
  bool value) noexcept -> bool {
#if EAGINE_APP_HAS_IMGUI
    if(auto found{_find_ui_input(input_id)}) {
        if(auto toggle{extract(found).get_toggle()}) {
            extract(toggle).toggled_on = value;
            return true;
        }
    }
#endif
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
#if EAGINE_APP_HAS_IMGUI
    return _setup_ui_input(
      input_id,
      ui_slider_state{
        .position = initial,
        .label = label.to_string(),
        .min = min,
        .max = max,
        .value = initial,
        .kind = kind});
#else
    return false;
#endif
}
//------------------------------------------------------------------------------
auto glfw3_opengl_window::set_ui_slider(
  const message_id input_id,
  float value) noexcept -> bool {
#if EAGINE_APP_HAS_IMGUI
    if(auto found{_find_ui_input(input_id)}) {
        if(auto slider{extract(found).get_slider()}) {
            extract(slider).value = value;
            return true;
        }
    }
#endif
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
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, extract(ver_maj));
    }
    if(const auto ver_min{video_opts.gl_version_minor()}) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, extract(ver_min));
    }
    const auto compat = video_opts.gl_compatibility_context();
    if(compat) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
        log_debug("using compatibility GL context")
          .arg("instance", _instance_id);
    } else if(!compat) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        log_debug("using core GL context").arg("instance", _instance_id);
    }
    if(video_opts.gl_debug_context()) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        log_debug("using debugging GL context").arg("instance", _instance_id);
    }

    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_RED_BITS, video_opts.color_bits() / GLFW_DONT_CARE);
    glfwWindowHint(GLFW_BLUE_BITS, video_opts.color_bits() / GLFW_DONT_CARE);
    glfwWindowHint(GLFW_GREEN_BITS, video_opts.color_bits() / GLFW_DONT_CARE);
    glfwWindowHint(GLFW_ALPHA_BITS, video_opts.alpha_bits() / GLFW_DONT_CARE);
    glfwWindowHint(GLFW_DEPTH_BITS, video_opts.depth_bits() / GLFW_DONT_CARE);
    glfwWindowHint(
      GLFW_STENCIL_BITS, video_opts.stencil_bits() / GLFW_DONT_CARE);

    glfwWindowHint(GLFW_SAMPLES, video_opts.samples() / GLFW_DONT_CARE);

    GLFWmonitor* window_monitor = nullptr;
    int fallback_width = 1280, fallback_height = 800;
    if(video_opts.fullscreen()) {
        window_monitor = glfwGetPrimaryMonitor();
        if(const auto opt_mon_name{video_opts.display_name()}) {
            for(auto monitor : monitors) {
                string_view mon_name(glfwGetMonitorName(monitor));
                if(are_equal(extract(opt_mon_name), mon_name)) {
                    window_monitor = monitor;
                }
            }
        }
    }
    if(const auto mode{glfwGetVideoMode(
         window_monitor ? window_monitor : glfwGetPrimaryMonitor())}) {
        fallback_width = extract(mode).width;
        fallback_height = extract(mode).height;
    }

    if(video_opts.offscreen()) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    _window = glfwCreateWindow(
      video_opts.surface_width() / fallback_width,
      video_opts.surface_height() / fallback_height,
      c_str(options.application_title()),
      window_monitor,
      nullptr);

    if(_window) {
        glfwSetWindowUserPointer(_window, this);
        glfwSetScrollCallback(_window, &glfw3_opengl_window_scroll_callback);
        glfwSetWindowTitle(_window, c_str(options.application_title()));
        glfwGetWindowSize(_window, &_window_width, &_window_height);
        if(_window_width > 0 && _window_height > 0) {
            _norm_x_ndc = 1.F / float(_window_width);
            _norm_y_ndc = 1.F / float(_window_height);
            _aspect = _norm_y_ndc / _norm_x_ndc;
        }
        if(_imgui_enabled) {
#if EAGINE_APP_HAS_IMGUI
            glfwMakeContextCurrent(_window);
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            ImGui::StyleColorsDark();

            ImGui_ImplGlfw_InitForOpenGL(_window, true);
            ImGui_ImplOpenGL3_Init("#version 150");
            glfwMakeContextCurrent(nullptr);
#endif
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
#if EAGINE_APP_HAS_IMGUI
    if(_imgui_enabled) {
        if(_imgui_visible || !_provider.activities().empty()) {
            if(const auto draw_data{ImGui::GetDrawData()}) {
                ImGui_ImplOpenGL3_RenderDrawData(draw_data);
            }
        }
    }
#endif
    glfwSwapBuffers(_window);
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::input_enumerate(
  const callable_ref<void(const message_id, const input_value_kinds) noexcept>
    callback) noexcept {
    // keyboard inputs
    for(const auto& ks : _key_states) {
        callback(
          message_id{"Keyboard", ks.key_id}, input_value_kind::absolute_norm);
    }

    // cursor device inputs
    for(const auto& ks : _mouse_states) {
        callback(
          message_id{"Cursor", ks.key_id}, input_value_kind::absolute_norm);
    }

    callback(
      message_id{"Cursor", "PositionX"},
      input_value_kind::absolute_free | input_value_kind::absolute_norm);
    callback(
      message_id{"Cursor", "PositionY"},
      input_value_kind::absolute_free | input_value_kind::absolute_norm);

    callback(message_id{"Cursor", "MotionX"}, input_value_kind::relative);
    callback(message_id{"Cursor", "MotionY"}, input_value_kind::relative);

    // wheel inputs
    callback(message_id{"Wheel", "ScrollX"}, input_value_kind::relative);
    callback(message_id{"Wheel", "ScrollY"}, input_value_kind::relative);

#if EAGINE_APP_HAS_IMGUI
    // ui input
    for(const auto& entry : _ui_input_states) {
        callback(entry.input_id, entry.kind());
    }
#endif
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
void glfw3_opengl_window::mapping_enable(const message_id signal_id) {
    _enabled_signals.insert(signal_id);
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::mapping_commit(
  [[maybe_unused]] const identifier setup_id) {
    for(auto& ks : _key_states) {
        ks.enabled = _enabled_signals.contains({"Keyboard", ks.key_id});
    }

    for(auto& ks : _mouse_states) {
        ks.enabled = _enabled_signals.contains({"Cursor", ks.key_id});
    }

    _mouse_enabled =
      _enabled_signals.contains(message_id{"Cursor", "PositionX"}) ||
      _enabled_signals.contains(message_id{"Cursor", "PositionY"}) ||
      _enabled_signals.contains(message_id{"Cursor", "MotionX"}) ||
      _enabled_signals.contains(message_id{"Cursor", "MotionY"});
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::update(execution_context& exec_ctx) {
    if(_imgui_enabled) {
        assert(_parent_context);
#if EAGINE_APP_HAS_IMGUI
        const auto activities{_provider.activities()};
        const auto& par_ctx = *_parent_context;
        const auto& state = exec_ctx.state();
        const auto frame_dur = state.frame_duration().value();
        const auto frames_per_second = state.frames_per_second();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = 0;
        if(!activities.empty()) {
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            window_flags |= ImGuiWindowFlags_NoResize;
            ImGui::SetNextWindowSize(ImVec2(float(_window_width) * 0.8F, 0.F));
            ImGui::Begin("Activities", nullptr, window_flags);
            for(const auto& info : activities) {
                const auto progress =
                  float(info.current_steps) / float(info.total_steps);
                ImGui::TextUnformatted(info.title.c_str());
                ImGui::ProgressBar(progress, ImVec2(-1.F, 0.F));
            }
            ImGui::End();
        }

        if(_imgui_visible) {
            window_flags = 0;
            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            window_flags |= ImGuiWindowFlags_NoResize;
            ImGui::Begin("Application", &_imgui_visible, window_flags);
            // NOLINTNEXTLINE(hicpp-vararg)
            ImGui::Text("Dimensions: %dx%d", _window_width, _window_height);
            // NOLINTNEXTLINE(hicpp-vararg)
            ImGui::Text("Frame number: %ld", long(par_ctx.frame_number()));
            // NOLINTNEXTLINE(hicpp-vararg)
            ImGui::Text("Frame time: %.1f [ms]", frame_dur * 1000.F);
            // NOLINTNEXTLINE(hicpp-vararg)
            ImGui::Text("Frames per second: %.0f", frames_per_second);
            // NOLINTNEXTLINE(hicpp-vararg)
            ImGui::Text(
              "Activities in progress: %ld",
              long(_provider.activities().size()));

            if(_input_sink) {
                auto& sink = extract(_input_sink);
                for(auto& entry : _ui_input_states) {
                    entry.apply(overloaded(
                      [&, this](ui_button_state& button) {
                          if(button.pressed.assign(
                               ImGui::Button(button.label.c_str()))) {
                              sink.consume(
                                {entry.input_id, entry.kind()}, button.pressed);
                          }
                      },
                      [&, this](ui_toggle_state& toggle) {
                          ImGui::Checkbox(toggle.label.c_str(), &toggle.value);
                          if(toggle.toggled_on.assign(toggle.value)) {
                              sink.consume(
                                {entry.input_id, entry.kind()},
                                toggle.toggled_on);
                          }
                      },
                      [&, this](ui_slider_state& slider) {
                          if(ImGui::SliderFloat(
                               slider.label.c_str(),
                               &slider.value,
                               slider.min,
                               slider.max,
                               "%0.2f")) {
                              if(slider.position.assign(slider.value)) {
                                  sink.consume(
                                    {entry.input_id, entry.kind()},
                                    slider.position);
                              }
                          }
                      }));
                }
            }

            if(ImGui::Button("Hide")) {
                _imgui_visible = false;
            }
            ImGui::SameLine();
            if(ImGui::Button("Quit")) {
                glfwSetWindowShouldClose(_window, GLFW_TRUE);
            }
            if(ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                // NOLINTNEXTLINE(hicpp-vararg)
                ImGui::Text("Closes the application");
                ImGui::EndTooltip();
            }
            ImGui::End();
        }

        ImGui::EndFrame();
        ImGui::Render();
#endif
    }

    if(glfwWindowShouldClose(_window)) {
        exec_ctx.stop_running();
    } else {
        glfwGetWindowSize(_window, &_window_width, &_window_height);

        if(_input_sink) {
            auto& sink = extract(_input_sink);

            if(_wheel_scroll_x.assign(_wheel_change_x)) {
                sink.consume(
                  {message_id{"Wheel", "ScrollX"}, input_value_kind::relative},
                  _wheel_scroll_x);
            }
            _wheel_change_x = 0;

            if(_wheel_scroll_y.assign(_wheel_change_y)) {
                sink.consume(
                  {message_id{"Wheel", "ScrollY"}, input_value_kind::relative},
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
                      {message_id{"Cursor", "PositionX"},
                       input_value_kind::absolute_free},
                      _mouse_x_pix);
                    if(_mouse_x_ndc.assign(
                         float(mouse_x_pix * _norm_x_ndc) - 0.5F)) {
                        sink.consume(
                          {message_id{"Cursor", "PositionX"},
                           input_value_kind::absolute_norm},
                          _mouse_x_ndc);
                        if(_mouse_x_delta.assign(float(
                             _mouse_x_ndc.delta() * motion_adjust * _aspect))) {
                            sink.consume(
                              {message_id{"Cursor", "MotionX"},
                               input_value_kind::relative},
                              _mouse_x_delta);
                        }
                    }
                }
                if(_mouse_y_pix.assign(float(mouse_y_pix))) {
                    sink.consume(
                      {message_id{"Cursor", "PositionY"},
                       input_value_kind::absolute_free},
                      _mouse_y_pix);
                    if(_mouse_y_ndc.assign(
                         float(mouse_y_pix * _norm_y_ndc) - 0.5F)) {
                        sink.consume(
                          {message_id{"Cursor", "PositionY"},
                           input_value_kind::absolute_norm},
                          _mouse_y_ndc);
                        if(_mouse_y_delta.assign(
                             float(_mouse_y_ndc.delta() * motion_adjust))) {
                            sink.consume(
                              {message_id{"Cursor", "MotionY"},
                               input_value_kind::relative},
                              _mouse_y_delta);
                        }
                    }
                }

                if(!_imgui_visible) {
                    for(auto& ks : _mouse_states) {
                        if(ks.enabled) {
                            if(ks.pressed.assign(
                                 glfwGetMouseButton(_window, ks.key_code) ==
                                 GLFW_PRESS)) {
                                sink.consume(
                                  {{"Cursor", ks.key_id},
                                   input_value_kind::absolute_norm},
                                  ks.pressed);
                            }
                        }
                    }
                }
            }
            if(!_imgui_visible) {
                for(auto& ks : _key_states) {
                    if(ks.enabled) {
                        const auto state = glfwGetKey(_window, ks.key_code);
                        const auto press = state == GLFW_PRESS;
                        if(ks.pressed.assign(press) || press) {
                            const message_id key_id{"Keyboard", ks.key_id};
                            sink.consume(
                              {key_id, input_value_kind::absolute_norm},
                              ks.pressed);
                            _feedback_key_press_change(key_id, ks.pressed);
                        }
                    }
                }
            }

            if(_imgui_enabled) {
                const auto backtick_is_pressed =
                  glfwGetKey(_window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS;
                if(_backtick_was_pressed != backtick_is_pressed) {
                    if(_backtick_was_pressed) {
                        _imgui_visible = !_imgui_visible;
                    }
                    _backtick_was_pressed = backtick_is_pressed;
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void glfw3_opengl_window::clean_up() {
    if(_window) {
        if(_imgui_enabled) {
#if EAGINE_APP_HAS_IMGUI
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
#endif
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
        if(!extract(std::get<1>(entry)).handle_progress()) {
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
    return !_windows.empty();
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
        const auto monitors = []() {
            int monitor_count = 0;
            auto* monitor_list = glfwGetMonitors(&monitor_count);
            return memory::view(monitor_list, monitor_count);
        }();

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
              video_opts.has_provider(implementation_name()) &&
              (video_opts.video_kind() == video_context_kind::opengl);

            if(should_create_window) {
                if(const auto new_win{std::make_shared<glfw3_opengl_window>(
                     this->main_context().config(), inst, *this)}) {
                    if(extract(new_win).initialize(
                         options, video_opts, monitors)) {
                        _windows[inst] = new_win;
                    } else {
                        extract(new_win).clean_up();
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
  [[maybe_unused]] execution_context& exec_ctx) {
#if EAGINE_APP_HAS_GLFW3
    glfwPollEvents();
    for(auto& entry : _windows) {
        entry.second->update(exec_ctx);
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
  [[maybe_unused]] callable_ref<void(std::shared_ptr<input_provider>)> handler) {
#if EAGINE_APP_HAS_GLFW3
    for(auto& p : _windows) {
        handler(p.second);
    }
#endif // EAGINE_APP_HAS_GLFW3
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::video_enumerate(
  [[maybe_unused]] callable_ref<void(std::shared_ptr<video_provider>)> handler) {
#if EAGINE_APP_HAS_GLFW3
    for(auto& p : _windows) {
        handler(p.second);
    }
#endif // EAGINE_APP_HAS_GLFW3
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::audio_enumerate(
  callable_ref<void(std::shared_ptr<audio_provider>)>) {}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::activity_begun(
  [[maybe_unused]] const activity_progress_id_t parent_id,
  [[maybe_unused]] const activity_progress_id_t activity_id,
  [[maybe_unused]] const string_view title,
  [[maybe_unused]] const span_size_t total_steps) noexcept {
#if EAGINE_APP_HAS_GLFW3
#if EAGINE_APP_HAS_IMGUI
    _activities.emplace_back();
    auto& info = _activities.back();
    info.activity_id = activity_id;
    info.parent_id = parent_id;
    info.total_steps = total_steps;
    info.title = to_string(title);
#endif
#endif
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::activity_finished(
  [[maybe_unused]] const activity_progress_id_t parent_id,
  [[maybe_unused]] const activity_progress_id_t activity_id,
  [[maybe_unused]] const string_view title,
  [[maybe_unused]] span_size_t total_steps) noexcept {
#if EAGINE_APP_HAS_GLFW3
#if EAGINE_APP_HAS_IMGUI
    std::erase_if(_activities, [activity_id](const auto& activity) -> bool {
        return activity.activity_id == activity_id;
    });
#endif
#endif
}
//------------------------------------------------------------------------------
void glfw3_opengl_provider::activity_updated(
  [[maybe_unused]] const activity_progress_id_t parent_id,
  [[maybe_unused]] const activity_progress_id_t activity_id,
  [[maybe_unused]] const span_size_t current_steps,
  [[maybe_unused]] const span_size_t total_steps) noexcept {
#if EAGINE_APP_HAS_GLFW3
#if EAGINE_APP_HAS_IMGUI
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
#endif
}
//------------------------------------------------------------------------------
auto make_glfw3_opengl_provider(main_ctx_parent parent)
  -> std::shared_ptr<hmi_provider> {
    return {std::make_shared<glfw3_opengl_provider>(parent)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
