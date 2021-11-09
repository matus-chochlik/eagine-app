/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/app/context.hpp>
#include <eagine/app_config.hpp>
#include <eagine/diagnostic.hpp>
#include <eagine/flat_set.hpp>
#include <eagine/main_ctx.hpp>
#include <eagine/maybe_unused.hpp>
#include <eagine/oglplus/config/basic.hpp>
#include <eagine/progress/backend.hpp>

//------------------------------------------------------------------------------
#if OGLPLUS_GLFW3_FOUND

#ifdef __clang__
EAGINE_DIAG_PUSH()
EAGINE_DIAG_OFF(documentation)
#endif
#include <GLFW/glfw3.h>
#ifdef __clang__
EAGINE_DIAG_POP()
#endif

#ifndef EAGINE_APP_USE_IMGUI
#define EAGINE_APP_USE_IMGUI 1
#endif

#if EAGINE_APP_USE_IMGUI
#ifdef __clang__
EAGINE_DIAG_PUSH()
EAGINE_DIAG_OFF(zero-as-null-pointer-constant)
#endif
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <vector>
#ifdef __clang__
EAGINE_DIAG_POP()
#endif
#endif // EAGINE_APP_USE_IMGUI

#endif // OGLPLUS_GLFW3_FOUND

namespace eagine::app {
//------------------------------------------------------------------------------
#if OGLPLUS_GLFW3_FOUND
#if EAGINE_APP_USE_IMGUI
struct glfw3_activity_progress_info {
    activity_progress_id_t activity_id{0};
    activity_progress_id_t parent_id{0};
    span_size_t current_steps{0};
    span_size_t total_steps{0};
    std::string title;
};
#endif
//------------------------------------------------------------------------------
struct glfw3_update_context {
#if EAGINE_APP_USE_IMGUI
    span<const glfw3_activity_progress_info> activities;
#endif
};
//------------------------------------------------------------------------------
class glfw3_opengl_window
  : public main_ctx_object
  , public video_provider
  , public input_provider {
public:
    glfw3_opengl_window(
      application_config&,
      const identifier instance_id,
      main_ctx_parent parent);

    glfw3_opengl_window(
      application_config&,
      const identifier instance_id,
      const string_view instance,
      main_ctx_parent parent);

    auto initialize(
      const launch_options&,
      const video_options&,
      const span<GLFWmonitor* const>) -> bool;

    void update(execution_context&, const glfw3_update_context&);

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

    auto add_ui_button(const std::string& label, const message_id)
      -> bool final;

    void on_scroll(const float x, const float y) {
        _wheel_change_x += x;
        _wheel_change_y += y;
    }

    auto handle_progress(const glfw3_update_context&) noexcept -> bool;

private:
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

#if EAGINE_APP_USE_IMGUI
    struct ui_button_state {
        std::string button_label;
        message_id button_id;
        input_variable<bool> pressed{false};

        ui_button_state(std::string label, const message_id id) noexcept
          : button_label{std::move(label)}
          , button_id{id} {}
    };
#endif

    flat_set<message_id> _enabled_signals;

    std::vector<key_state> _key_states;
    std::vector<key_state> _mouse_states;

#if EAGINE_APP_USE_IMGUI
    std::vector<ui_button_state> _ui_button_states;
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
EAGINE_LIB_FUNC
void glfw3_opengl_window_scroll_callback(GLFWwindow* window, double x, double y) {
    if(auto raw_that{glfwGetWindowUserPointer(window)}) {
        auto that = reinterpret_cast<glfw3_opengl_window*>(raw_that);
        that->on_scroll(float(x), float(y));
    }
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
glfw3_opengl_window::glfw3_opengl_window(
  application_config& c,
  const identifier instance_id,
  const string_view instance,
  main_ctx_parent parent)
  : main_ctx_object{EAGINE_ID(GLFW3Wndow), parent}
  , _instance_id{instance_id}
  , _imgui_enabled{
      c,
      "application.imgui.enable",
      instance,
      EAGINE_APP_USE_IMGUI != 0} {

    // keyboard keys/buttons
    _key_states.emplace_back(EAGINE_ID(Spacebar), GLFW_KEY_SPACE);
    _key_states.emplace_back(EAGINE_ID(Backspace), GLFW_KEY_BACKSPACE);
    _key_states.emplace_back(EAGINE_ID(Escape), GLFW_KEY_ESCAPE);
    _key_states.emplace_back(EAGINE_ID(Enter), GLFW_KEY_ENTER);
    _key_states.emplace_back(EAGINE_ID(Tab), GLFW_KEY_TAB);
    _key_states.emplace_back(EAGINE_ID(Left), GLFW_KEY_LEFT);
    _key_states.emplace_back(EAGINE_ID(Right), GLFW_KEY_RIGHT);
    _key_states.emplace_back(EAGINE_ID(Up), GLFW_KEY_UP);
    _key_states.emplace_back(EAGINE_ID(Down), GLFW_KEY_DOWN);
    _key_states.emplace_back(EAGINE_ID(PageUp), GLFW_KEY_PAGE_UP);
    _key_states.emplace_back(EAGINE_ID(PageDown), GLFW_KEY_PAGE_DOWN);
    _key_states.emplace_back(EAGINE_ID(Home), GLFW_KEY_HOME);
    _key_states.emplace_back(EAGINE_ID(End), GLFW_KEY_END);
    _key_states.emplace_back(EAGINE_ID(Insert), GLFW_KEY_INSERT);
    _key_states.emplace_back(EAGINE_ID(Delete), GLFW_KEY_DELETE);
    _key_states.emplace_back(EAGINE_ID(Apostrophe), GLFW_KEY_APOSTROPHE);
    _key_states.emplace_back(EAGINE_ID(Minus), GLFW_KEY_MINUS);
    _key_states.emplace_back(EAGINE_ID(Equal), GLFW_KEY_EQUAL);
    _key_states.emplace_back(EAGINE_ID(Period), GLFW_KEY_PERIOD);
    _key_states.emplace_back(EAGINE_ID(Semicolon), GLFW_KEY_SEMICOLON);
    _key_states.emplace_back(EAGINE_ID(Slash), GLFW_KEY_SLASH);
    _key_states.emplace_back(EAGINE_ID(LtBracket), GLFW_KEY_LEFT_BRACKET);
    _key_states.emplace_back(EAGINE_ID(RtBracket), GLFW_KEY_RIGHT_BRACKET);
    _key_states.emplace_back(EAGINE_ID(CapsLock), GLFW_KEY_CAPS_LOCK);
    _key_states.emplace_back(EAGINE_ID(NumLock), GLFW_KEY_NUM_LOCK);
    _key_states.emplace_back(EAGINE_ID(ScrollLock), GLFW_KEY_SCROLL_LOCK);
    _key_states.emplace_back(EAGINE_ID(PrntScreen), GLFW_KEY_PRINT_SCREEN);
    _key_states.emplace_back(EAGINE_ID(Pause), GLFW_KEY_PAUSE);

    _key_states.emplace_back(EAGINE_ID(A), GLFW_KEY_A);
    _key_states.emplace_back(EAGINE_ID(B), GLFW_KEY_B);
    _key_states.emplace_back(EAGINE_ID(C), GLFW_KEY_C);
    _key_states.emplace_back(EAGINE_ID(D), GLFW_KEY_D);
    _key_states.emplace_back(EAGINE_ID(E), GLFW_KEY_E);
    _key_states.emplace_back(EAGINE_ID(F), GLFW_KEY_F);
    _key_states.emplace_back(EAGINE_ID(G), GLFW_KEY_G);
    _key_states.emplace_back(EAGINE_ID(H), GLFW_KEY_H);
    _key_states.emplace_back(EAGINE_ID(I), GLFW_KEY_I);
    _key_states.emplace_back(EAGINE_ID(J), GLFW_KEY_J);
    _key_states.emplace_back(EAGINE_ID(K), GLFW_KEY_K);
    _key_states.emplace_back(EAGINE_ID(L), GLFW_KEY_L);
    _key_states.emplace_back(EAGINE_ID(M), GLFW_KEY_M);
    _key_states.emplace_back(EAGINE_ID(N), GLFW_KEY_N);
    _key_states.emplace_back(EAGINE_ID(O), GLFW_KEY_O);
    _key_states.emplace_back(EAGINE_ID(P), GLFW_KEY_P);
    _key_states.emplace_back(EAGINE_ID(Q), GLFW_KEY_Q);
    _key_states.emplace_back(EAGINE_ID(R), GLFW_KEY_R);
    _key_states.emplace_back(EAGINE_ID(S), GLFW_KEY_S);
    _key_states.emplace_back(EAGINE_ID(T), GLFW_KEY_R);
    _key_states.emplace_back(EAGINE_ID(U), GLFW_KEY_U);
    _key_states.emplace_back(EAGINE_ID(V), GLFW_KEY_V);
    _key_states.emplace_back(EAGINE_ID(W), GLFW_KEY_W);
    _key_states.emplace_back(EAGINE_ID(X), GLFW_KEY_X);
    _key_states.emplace_back(EAGINE_ID(Y), GLFW_KEY_Y);
    _key_states.emplace_back(EAGINE_ID(Z), GLFW_KEY_Z);

    _key_states.emplace_back(EAGINE_ID(0), GLFW_KEY_0);
    _key_states.emplace_back(EAGINE_ID(1), GLFW_KEY_1);
    _key_states.emplace_back(EAGINE_ID(2), GLFW_KEY_2);
    _key_states.emplace_back(EAGINE_ID(3), GLFW_KEY_3);
    _key_states.emplace_back(EAGINE_ID(4), GLFW_KEY_4);
    _key_states.emplace_back(EAGINE_ID(5), GLFW_KEY_5);
    _key_states.emplace_back(EAGINE_ID(6), GLFW_KEY_6);
    _key_states.emplace_back(EAGINE_ID(7), GLFW_KEY_7);
    _key_states.emplace_back(EAGINE_ID(8), GLFW_KEY_8);
    _key_states.emplace_back(EAGINE_ID(9), GLFW_KEY_9);

    _key_states.emplace_back(EAGINE_ID(KeyPad0), GLFW_KEY_KP_0);
    _key_states.emplace_back(EAGINE_ID(KeyPad1), GLFW_KEY_KP_1);
    _key_states.emplace_back(EAGINE_ID(KeyPad2), GLFW_KEY_KP_2);
    _key_states.emplace_back(EAGINE_ID(KeyPad3), GLFW_KEY_KP_3);
    _key_states.emplace_back(EAGINE_ID(KeyPad4), GLFW_KEY_KP_4);
    _key_states.emplace_back(EAGINE_ID(KeyPad5), GLFW_KEY_KP_5);
    _key_states.emplace_back(EAGINE_ID(KeyPad6), GLFW_KEY_KP_6);
    _key_states.emplace_back(EAGINE_ID(KeyPad7), GLFW_KEY_KP_7);
    _key_states.emplace_back(EAGINE_ID(KeyPad8), GLFW_KEY_KP_8);
    _key_states.emplace_back(EAGINE_ID(KeyPad9), GLFW_KEY_KP_9);

    _key_states.emplace_back(EAGINE_ID(KpDecimal), GLFW_KEY_KP_DECIMAL);
    _key_states.emplace_back(EAGINE_ID(KpPlus), GLFW_KEY_KP_ADD);
    _key_states.emplace_back(EAGINE_ID(KpMinus), GLFW_KEY_KP_SUBTRACT);
    _key_states.emplace_back(EAGINE_ID(KpAsterisk), GLFW_KEY_KP_MULTIPLY);
    _key_states.emplace_back(EAGINE_ID(KpSlash), GLFW_KEY_KP_DIVIDE);
    _key_states.emplace_back(EAGINE_ID(KpEqual), GLFW_KEY_KP_EQUAL);
    _key_states.emplace_back(EAGINE_ID(KpEnter), GLFW_KEY_KP_ENTER);

    _key_states.emplace_back(EAGINE_ID(LeftSuper), GLFW_KEY_LEFT_SUPER);
    _key_states.emplace_back(EAGINE_ID(LeftShift), GLFW_KEY_LEFT_SHIFT);
    _key_states.emplace_back(EAGINE_ID(LeftCtrl), GLFW_KEY_LEFT_CONTROL);
    _key_states.emplace_back(EAGINE_ID(LeftAlt), GLFW_KEY_LEFT_ALT);
    _key_states.emplace_back(EAGINE_ID(RightSuper), GLFW_KEY_RIGHT_SUPER);
    _key_states.emplace_back(EAGINE_ID(RightShift), GLFW_KEY_RIGHT_SHIFT);
    _key_states.emplace_back(EAGINE_ID(RightCtrl), GLFW_KEY_RIGHT_CONTROL);
    _key_states.emplace_back(EAGINE_ID(RightAlt), GLFW_KEY_RIGHT_ALT);
    _key_states.emplace_back(EAGINE_ID(Menu), GLFW_KEY_MENU);

    _key_states.emplace_back(EAGINE_ID(F1), GLFW_KEY_F1);
    _key_states.emplace_back(EAGINE_ID(F2), GLFW_KEY_F2);
    _key_states.emplace_back(EAGINE_ID(F3), GLFW_KEY_F3);
    _key_states.emplace_back(EAGINE_ID(F4), GLFW_KEY_F4);
    _key_states.emplace_back(EAGINE_ID(F5), GLFW_KEY_F5);
    _key_states.emplace_back(EAGINE_ID(F6), GLFW_KEY_F6);
    _key_states.emplace_back(EAGINE_ID(F7), GLFW_KEY_F7);
    _key_states.emplace_back(EAGINE_ID(F8), GLFW_KEY_F8);
    _key_states.emplace_back(EAGINE_ID(F9), GLFW_KEY_F9);
    _key_states.emplace_back(EAGINE_ID(F10), GLFW_KEY_F10);
    _key_states.emplace_back(EAGINE_ID(F11), GLFW_KEY_F11);
    _key_states.emplace_back(EAGINE_ID(F12), GLFW_KEY_F12);

    // mouse buttons
    _mouse_states.emplace_back(EAGINE_ID(Pressed), GLFW_MOUSE_BUTTON_1);
    _mouse_states.emplace_back(EAGINE_ID(Button0), GLFW_MOUSE_BUTTON_1);
    _mouse_states.emplace_back(EAGINE_ID(Button1), GLFW_MOUSE_BUTTON_2);
    _mouse_states.emplace_back(EAGINE_ID(Button2), GLFW_MOUSE_BUTTON_3);
    _mouse_states.emplace_back(EAGINE_ID(Button3), GLFW_MOUSE_BUTTON_4);
    _mouse_states.emplace_back(EAGINE_ID(Button4), GLFW_MOUSE_BUTTON_5);
    _mouse_states.emplace_back(EAGINE_ID(Button5), GLFW_MOUSE_BUTTON_6);
    _mouse_states.emplace_back(EAGINE_ID(Button6), GLFW_MOUSE_BUTTON_7);
    _mouse_states.emplace_back(EAGINE_ID(Button7), GLFW_MOUSE_BUTTON_8);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::add_ui_button(
  const std::string& label,
  const message_id id) -> bool {
#if EAGINE_APP_USE_IMGUI
    if(
      std::find_if(
        _ui_button_states.begin(),
        _ui_button_states.end(),
        [id](const auto& state) { return state.button_id == id; }) ==
      _ui_button_states.end()) {
        _ui_button_states.emplace_back(label, id);
        return true;
    }
#endif
    EAGINE_MAYBE_UNUSED(label);
    EAGINE_MAYBE_UNUSED(id);
    return false;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
glfw3_opengl_window::glfw3_opengl_window(
  application_config& cfg,
  const identifier instance_id,
  main_ctx_parent parent)
  : glfw3_opengl_window{cfg, instance_id, instance_id.name(), parent} {}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::handle_progress(
  const glfw3_update_context& upd_ctx) noexcept -> bool {
    bool result = false;
    EAGINE_MAYBE_UNUSED(upd_ctx);
    if(_window) {
        result = glfwGetKey(_window, GLFW_KEY_ESCAPE) != GLFW_PRESS;
        if(_imgui_enabled) {
#ifdef EAGINE_APP_USE_IMGUI
            if(!upd_ctx.activities.empty()) {
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                ImGuiWindowFlags window_flags = 0;
                // NOLINTNEXTLINE(hicpp-signed-bitwise)
                window_flags |= ImGuiWindowFlags_NoResize;
                ImGui::SetNextWindowSize(
                  ImVec2(float(_window_width) * 0.8F, 0.F));
                ImGui::Begin("Activities", nullptr, window_flags);
                for(const auto& info : upd_ctx.activities) {
                    const auto progress =
                      float(info.current_steps) / float(info.total_steps);
                    ImGui::TextUnformatted(info.title.c_str());
                    ImGui::ProgressBar(progress, ImVec2(-1.F, 0.F));
                }
                ImGui::End();
                ImGui::EndFrame();
                ImGui::Render();
                if(const auto draw_data{ImGui::GetDrawData()}) {
                    ImGui_ImplOpenGL3_RenderDrawData(draw_data);
                }
            }
#endif
            glfwSwapBuffers(_window);
        }
    }
    return result;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
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
          .arg(EAGINE_ID(instance), _instance_id);
    } else if(!compat) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        log_debug("using core GL context")
          .arg(EAGINE_ID(instance), _instance_id);
    }
    if(video_opts.gl_debug_context()) {
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        log_debug("using debugging GL context")
          .arg(EAGINE_ID(instance), _instance_id);
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
        if(auto opt_mon_name{video_opts.display_name()}) {
            for(auto monitor : monitors) {
                string_view mon_name(glfwGetMonitorName(monitor));
                if(are_equal(extract(opt_mon_name), mon_name)) {
                    window_monitor = monitor;
                }
            }
        }
    }
    if(auto mode{glfwGetVideoMode(
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
#if EAGINE_APP_USE_IMGUI
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
        log_error("Failed to create GLFW window")
          .arg(EAGINE_ID(instance), _instance_id);
    }
    return false;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::video_kind() const noexcept -> video_context_kind {
    return video_context_kind::opengl;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::instance_id() const noexcept -> identifier {
    return _instance_id;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::is_offscreen() noexcept -> tribool {
    if(_window) {
        return glfwGetWindowAttrib(_window, GLFW_VISIBLE) == 0;
    }
    return indeterminate;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::has_framebuffer() noexcept -> tribool {
    if(_window) {
        return glfwGetWindowAttrib(_window, GLFW_VISIBLE) != 0;
    }
    return indeterminate;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::surface_size() noexcept -> std::tuple<int, int> {
    return {_window_width, _window_height};
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_window::surface_aspect() noexcept -> float {
    return _aspect;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::parent_context_changed(const video_context& vctx) {
    _parent_context = &vctx;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::video_begin(execution_context&) {

    EAGINE_ASSERT(_window);
    glfwMakeContextCurrent(_window);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::video_end(execution_context&) {
    glfwMakeContextCurrent(nullptr);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::video_commit(execution_context&) {
    EAGINE_ASSERT(_window);
    if(_imgui_enabled && _imgui_visible) {
        if(const auto draw_data{ImGui::GetDrawData()}) {
            ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }
    }
    glfwSwapBuffers(_window);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::input_enumerate(
  const callable_ref<void(const message_id, const input_value_kinds) noexcept>
    callback) noexcept {
    // keyboard inputs
    for(const auto& ks : _key_states) {
        callback(
          message_id{EAGINE_ID(Keyboard), ks.key_id},
          input_value_kind::absolute_norm);
    }

    // cursor device inputs
    for(const auto& ks : _mouse_states) {
        callback(
          message_id{EAGINE_ID(Cursor), ks.key_id},
          input_value_kind::absolute_norm);
    }

    callback(
      EAGINE_MSG_ID(Cursor, PositionX),
      input_value_kind::absolute_free | input_value_kind::absolute_norm);
    callback(
      EAGINE_MSG_ID(Cursor, PositionY),
      input_value_kind::absolute_free | input_value_kind::absolute_norm);

    callback(EAGINE_MSG_ID(Cursor, MotionX), input_value_kind::relative);
    callback(EAGINE_MSG_ID(Cursor, MotionY), input_value_kind::relative);

    // wheel inputs
    callback(EAGINE_MSG_ID(Wheel, ScrollX), input_value_kind::relative);
    callback(EAGINE_MSG_ID(Wheel, ScrollY), input_value_kind::relative);

#if EAGINE_APP_USE_IMGUI
    // ui input
    for(const auto& bs : _ui_button_states) {
        callback(bs.button_id, input_value_kind::absolute_norm);
    }
#endif
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::input_connect(input_sink& sink) {
    _input_sink = std::addressof(sink);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::input_disconnect() {
    _input_sink = nullptr;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::mapping_begin(const identifier setup_id) {
    EAGINE_MAYBE_UNUSED(setup_id);
    _enabled_signals.clear();
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::mapping_enable(const message_id signal_id) {
    _enabled_signals.insert(signal_id);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::mapping_commit(const identifier setup_id) {
    EAGINE_MAYBE_UNUSED(setup_id);

    for(auto& ks : _key_states) {
        ks.enabled =
          _enabled_signals.contains({EAGINE_ID(Keyboard), ks.key_id});
    }

    for(auto& ks : _mouse_states) {
        ks.enabled = _enabled_signals.contains({EAGINE_ID(Cursor), ks.key_id});
    }

    _mouse_enabled =
      _enabled_signals.contains(EAGINE_MSG_ID(Cursor, PositionX)) ||
      _enabled_signals.contains(EAGINE_MSG_ID(Cursor, PositionY)) ||
      _enabled_signals.contains(EAGINE_MSG_ID(Cursor, MotionX)) ||
      _enabled_signals.contains(EAGINE_MSG_ID(Cursor, MotionY));
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_window::update(
  execution_context& exec_ctx,
  const glfw3_update_context& upd_ctx) {
    EAGINE_MAYBE_UNUSED(upd_ctx);

    if(_imgui_enabled && _imgui_visible) {
        EAGINE_ASSERT(_parent_context);
#if EAGINE_APP_USE_IMGUI
        const auto& par_ctx = *_parent_context;
        const auto& state = exec_ctx.state();
        const auto frame_dur = state.frame_duration().value();
        const auto frames_per_second = state.frames_per_second();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = 0;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        window_flags |= ImGuiWindowFlags_NoResize;
        ImGui::Begin("Application", nullptr, window_flags);
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
          "Activities in progress: %ld", long(upd_ctx.activities.size()));

#if EAGINE_APP_USE_IMGUI
        if(_input_sink) {
            auto& sink = extract(_input_sink);
            for(auto& bs : _ui_button_states) {
                if(bs.pressed.assign(ImGui::Button(bs.button_label.c_str()))) {
                    sink.consume(
                      {bs.button_id, input_value_kind::absolute_norm},
                      bs.pressed);
                }
            }
        }
#endif

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
                  {EAGINE_MSG_ID(Wheel, ScrollX), input_value_kind::relative},
                  _wheel_scroll_x);
            }
            _wheel_change_x = 0;

            if(_wheel_scroll_y.assign(_wheel_change_y)) {
                sink.consume(
                  {EAGINE_MSG_ID(Wheel, ScrollY), input_value_kind::relative},
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
                      {EAGINE_MSG_ID(Cursor, PositionX),
                       input_value_kind::absolute_free},
                      _mouse_x_pix);
                    if(_mouse_x_ndc.assign(
                         float(mouse_x_pix * _norm_x_ndc) - 0.5F)) {
                        sink.consume(
                          {EAGINE_MSG_ID(Cursor, PositionX),
                           input_value_kind::absolute_norm},
                          _mouse_x_ndc);
                        if(_mouse_x_delta.assign(float(
                             _mouse_x_ndc.delta() * motion_adjust * _aspect))) {
                            sink.consume(
                              {EAGINE_MSG_ID(Cursor, MotionX),
                               input_value_kind::relative},
                              _mouse_x_delta);
                        }
                    }
                }
                if(_mouse_y_pix.assign(float(mouse_y_pix))) {
                    sink.consume(
                      {EAGINE_MSG_ID(Cursor, PositionY),
                       input_value_kind::absolute_free},
                      _mouse_y_pix);
                    if(_mouse_y_ndc.assign(
                         float(mouse_y_pix * _norm_y_ndc) - 0.5F)) {
                        sink.consume(
                          {EAGINE_MSG_ID(Cursor, PositionY),
                           input_value_kind::absolute_norm},
                          _mouse_y_ndc);
                        if(_mouse_y_delta.assign(
                             float(_mouse_y_ndc.delta() * motion_adjust))) {
                            sink.consume(
                              {EAGINE_MSG_ID(Cursor, MotionY),
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
                                  {{EAGINE_ID(Cursor), ks.key_id},
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
                            sink.consume(
                              {{EAGINE_ID(Keyboard), ks.key_id},
                               input_value_kind::absolute_norm},
                              ks.pressed);
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
EAGINE_LIB_FUNC
void glfw3_opengl_window::clean_up() {
    if(_window) {
        if(_imgui_enabled) {
#if EAGINE_APP_USE_IMGUI
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
#endif
        }
        glfwDestroyWindow(_window);
    }
}
#endif // OGLPLUS_GLFW3_FOUND
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
#if OGLPLUS_GLFW3_FOUND
    std::map<identifier, std::shared_ptr<glfw3_opengl_window>> _windows;
#if EAGINE_APP_USE_IMGUI
    std::vector<glfw3_activity_progress_info> _activities;
#endif
#endif
    auto _get_progress_callback() noexcept -> callable_ref<bool() noexcept>;
    auto _handle_progress() noexcept -> bool;
};
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
glfw3_opengl_provider::glfw3_opengl_provider(main_ctx_parent parent)
  : main_ctx_object{EAGINE_ID(GLFW3Prvdr), parent} {
    set_progress_update_callback(
      main_context(), _get_progress_callback(), std::chrono::milliseconds{100});
    register_progress_observer(main_context(), *this);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
glfw3_opengl_provider::~glfw3_opengl_provider() noexcept {
    unregister_progress_observer(main_context(), *this);
    reset_progress_update_callback(main_context());
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_provider::_get_progress_callback() noexcept
  -> callable_ref<bool() noexcept> {
    return EAGINE_THIS_MEM_FUNC_REF(_handle_progress);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_provider::_handle_progress() noexcept -> bool {
#if OGLPLUS_GLFW3_FOUND
    glfw3_update_context upd_ctx{};
#ifdef EAGINE_APP_USE_IMGUI
    upd_ctx.activities = cover(_activities);
#endif
    glfwPollEvents();
    for(auto& entry : _windows) {
        EAGINE_ASSERT(std::get<1>(entry));
        if(!extract(std::get<1>(entry)).handle_progress(upd_ctx)) {
            return false;
        }
    }
#endif
    return true;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_provider::is_implemented() const noexcept -> bool {
    return OGLPLUS_GLFW3_FOUND != 0;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_provider::implementation_name() const noexcept
  -> string_view {
    return {"glfw3"};
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_provider::is_initialized() -> bool {
#if OGLPLUS_GLFW3_FOUND
    return !_windows.empty();
#endif
    return false;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_provider::should_initialize(execution_context& exec_ctx)
  -> bool {
    EAGINE_MAYBE_UNUSED(exec_ctx);
#if OGLPLUS_GLFW3_FOUND
    for(auto& [inst, video_opts] : exec_ctx.options().video_requirements()) {
        EAGINE_MAYBE_UNUSED(inst);
        if(video_opts.has_provider(implementation_name())) {
            return true;
        }
    }
#endif
    return false;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto glfw3_opengl_provider::initialize(execution_context& exec_ctx) -> bool {
#if OGLPLUS_GLFW3_FOUND
    if(glfwInit()) {
        const auto monitors = []() {
            int monitor_count = 0;
            auto* monitor_list = glfwGetMonitors(&monitor_count);
            return memory::view(monitor_list, monitor_count);
        }();

        log_info("GLFW monitors").arg_func([monitors](logger_backend& backend) {
            for(auto monitor : monitors) {
                backend.add_string(
                  EAGINE_ID(monitors),
                  EAGINE_ID(string),
                  string_view(glfwGetMonitorName(monitor)));
            }
        });

        auto& options = exec_ctx.options();
        for(auto& [inst, video_opts] : options.video_requirements()) {
            const bool should_create_window =
              video_opts.has_provider(implementation_name()) &&
              (video_opts.video_kind() == video_context_kind::opengl);

            if(should_create_window) {
                if(auto new_win{std::make_shared<glfw3_opengl_window>(
                     this->main_context().config(), inst, this->as_parent())}) {
                    if(extract(new_win).initialize(
                         options, video_opts, monitors)) {
                        _windows[inst] = std::move(new_win);
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
#endif // OGLPLUS_GLFW3_FOUND
    exec_ctx.log_error("GLFW is context is not supported");
    return false;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_provider::update(execution_context& exec_ctx) {
    EAGINE_MAYBE_UNUSED(exec_ctx);
#if OGLPLUS_GLFW3_FOUND
    glfw3_update_context upd_ctx{};
#ifdef EAGINE_APP_USE_IMGUI
    upd_ctx.activities = cover(_activities);
#endif
    glfwPollEvents();
    for(auto& entry : _windows) {
        entry.second->update(exec_ctx, upd_ctx);
    }
#endif // OGLPLUS_GLFW3_FOUND
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_provider::clean_up(execution_context&) {
#if OGLPLUS_GLFW3_FOUND
    for(auto& entry : _windows) {
        entry.second->clean_up();
    }
    glfwTerminate();
#endif // OGLPLUS_GLFW3_FOUND
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_provider::input_enumerate(
  callable_ref<void(std::shared_ptr<input_provider>)> handler) {
    EAGINE_MAYBE_UNUSED(handler);
#if OGLPLUS_GLFW3_FOUND
    for(auto& p : _windows) {
        handler(p.second);
    }
#endif // OGLPLUS_GLFW3_FOUND
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_provider::video_enumerate(
  callable_ref<void(std::shared_ptr<video_provider>)> handler) {
    EAGINE_MAYBE_UNUSED(handler);
#if OGLPLUS_GLFW3_FOUND
    for(auto& p : _windows) {
        handler(p.second);
    }
#endif // OGLPLUS_GLFW3_FOUND
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_provider::audio_enumerate(
  callable_ref<void(std::shared_ptr<audio_provider>)>) {}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_provider::activity_begun(
  const activity_progress_id_t parent_id,
  const activity_progress_id_t activity_id,
  const string_view title,
  const span_size_t total_steps) noexcept {
    EAGINE_MAYBE_UNUSED(parent_id);
    EAGINE_MAYBE_UNUSED(activity_id);
    EAGINE_MAYBE_UNUSED(title);
    EAGINE_MAYBE_UNUSED(total_steps);
#if OGLPLUS_GLFW3_FOUND
#if EAGINE_APP_USE_IMGUI
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
EAGINE_LIB_FUNC
void glfw3_opengl_provider::activity_finished(
  const activity_progress_id_t parent_id,
  const activity_progress_id_t activity_id,
  const string_view title,
  span_size_t total_steps) noexcept {
#if OGLPLUS_GLFW3_FOUND
#if EAGINE_APP_USE_IMGUI
    std::erase_if(_activities, [activity_id](const auto& activity) -> bool {
        return activity.activity_id == activity_id;
    });
#endif
#endif
    EAGINE_MAYBE_UNUSED(parent_id);
    EAGINE_MAYBE_UNUSED(activity_id);
    EAGINE_MAYBE_UNUSED(title);
    EAGINE_MAYBE_UNUSED(total_steps);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void glfw3_opengl_provider::activity_updated(
  const activity_progress_id_t parent_id,
  const activity_progress_id_t activity_id,
  const span_size_t current_steps,
  const span_size_t total_steps) noexcept {
#if OGLPLUS_GLFW3_FOUND
#if EAGINE_APP_USE_IMGUI
    const auto pos = std::find_if(
      _activities.begin(),
      _activities.end(),
      [activity_id](const auto& activity) -> bool {
          return activity.activity_id == activity_id;
      });
    if(EAGINE_LIKELY(pos != _activities.end())) {
        pos->current_steps = current_steps;
    }
#endif
#endif
    EAGINE_MAYBE_UNUSED(parent_id);
    EAGINE_MAYBE_UNUSED(activity_id);
    EAGINE_MAYBE_UNUSED(current_steps);
    EAGINE_MAYBE_UNUSED(total_steps);
}
//------------------------------------------------------------------------------
auto make_glfw3_opengl_provider(main_ctx_parent parent)
  -> std::shared_ptr<hmi_provider> {
    return {std::make_shared<glfw3_opengl_provider>(parent)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
