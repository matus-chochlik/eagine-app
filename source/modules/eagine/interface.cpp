/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:interface;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.utility;
import eagine.core.main_ctx;
import :types;
import :options;
import :input;

namespace eagine::app {
//------------------------------------------------------------------------------
export struct input_sink : interface<input_sink> {

    virtual void consume(
      const input_info&,
      const input_value<bool>&) noexcept = 0;
    virtual void consume(
      const input_info&,
      const input_value<int>&) noexcept = 0;
    virtual void consume(
      const input_info&,
      const input_value<float>&) noexcept = 0;
    virtual void consume(
      const input_info&,
      const input_value<double>&) noexcept = 0;
};
//------------------------------------------------------------------------------
export struct input_provider : interface<input_provider> {

    virtual auto instance_id() const noexcept -> identifier = 0;

    virtual void input_enumerate(
      execution_context&,
      const callable_ref<void(
        const identifier,
        const message_id,
        const input_value_kinds) noexcept>) = 0;

    virtual void input_connect(input_sink&) = 0;
    virtual void input_disconnect() = 0;

    virtual void mapping_begin(const identifier setup_id) = 0;
    virtual void mapping_enable(
      const identifier device_id,
      const message_id signal_id) = 0;
    virtual void mapping_commit(
      execution_context&,
      const identifier setup_id) = 0;

    virtual auto add_ui_feedback(
      const identifier mapping_id,
      const identifier device_id,
      const message_id signal_id,
      const message_id input_id,
      input_feedback_trigger,
      input_feedback_action,
      std::variant<std::monostate, bool, float> threshold,
      std::variant<std::monostate, bool, float> constant) noexcept -> bool = 0;

    virtual auto add_ui_button(const message_id, const string_view label)
      -> bool = 0;

    virtual auto add_ui_toggle(
      const message_id,
      const string_view label,
      bool initial) -> bool = 0;
    virtual auto set_ui_toggle(const message_id, bool value) noexcept
      -> bool = 0;

    virtual auto add_ui_slider(
      const message_id,
      const string_view label,
      float min,
      float max,
      float initial,
      input_value_kind kind) -> bool = 0;
    virtual auto set_ui_slider(const message_id, float value) noexcept
      -> bool = 0;
};
//------------------------------------------------------------------------------
export class video_context;
auto video_ctx(execution_context&, span_size_t = 0) noexcept
  -> optional_reference<video_context>;
export struct video_provider : interface<video_provider> {

    virtual auto video_kind() const noexcept -> video_context_kind = 0;
    virtual auto instance_id() const noexcept -> identifier = 0;

    virtual auto is_offscreen() noexcept -> tribool = 0;
    virtual auto has_framebuffer() noexcept -> tribool = 0;
    virtual auto surface_size() noexcept -> std::tuple<int, int> = 0;
    virtual auto surface_aspect() noexcept -> float = 0;

    virtual void parent_context_changed(const video_context&) = 0;
    virtual void video_begin(execution_context&) = 0;
    virtual void video_end(execution_context&) = 0;
    virtual void video_commit(execution_context&) = 0;
};
//------------------------------------------------------------------------------
export class audio_context;
auto audio_ctx(execution_context&, span_size_t = 0) noexcept
  -> optional_reference<audio_context>;
export struct audio_provider : interface<audio_provider> {

    virtual auto audio_kind() const noexcept -> audio_context_kind = 0;
    virtual auto instance_id() const noexcept -> identifier = 0;

    virtual void parent_context_changed(const audio_context&) = 0;
    virtual void audio_begin(execution_context&) = 0;
    virtual void audio_end(execution_context&) = 0;
    virtual void audio_commit(execution_context&) = 0;
};
//------------------------------------------------------------------------------
export struct hmi_provider : interface<hmi_provider> {

    virtual auto is_implemented() const noexcept -> bool = 0;
    virtual auto implementation_name() const noexcept -> string_view = 0;

    virtual auto is_initialized() -> bool = 0;
    virtual auto should_initialize(execution_context&) -> bool = 0;
    virtual auto initialize(execution_context&) -> bool = 0;
    virtual void update(execution_context&) = 0;
    virtual void clean_up(execution_context&) = 0;

    virtual void input_enumerate(
      callable_ref<void(std::shared_ptr<input_provider>)>) = 0;
    virtual void video_enumerate(
      callable_ref<void(std::shared_ptr<video_provider>)>) = 0;
    virtual void audio_enumerate(
      callable_ref<void(std::shared_ptr<audio_provider>)>) = 0;
};
//------------------------------------------------------------------------------
export struct framedump : interface<framedump> {

    virtual auto initialize(execution_context&, const video_options&)
      -> bool = 0;

    virtual auto get_buffer(const span_size_t size) -> memory::block = 0;

    virtual auto dump_frame(
      const long frame_number,
      const int width,
      const int height,
      const int elements,
      const span_size_t element_size,
      const framedump_pixel_format,
      const framedump_data_type,
      memory::block data) -> bool = 0;
};
//------------------------------------------------------------------------------
export struct application : interface<application> {

    virtual auto is_done() noexcept -> bool = 0;
    virtual void on_video_resize() noexcept = 0;
    virtual void update() noexcept = 0;
    virtual void clean_up() noexcept {}
};
//------------------------------------------------------------------------------
export struct launchpad : interface<launchpad> {

    virtual auto setup(main_ctx&, launch_options&) -> bool {
        return true;
    }

    [[nodiscard]] virtual auto launch(execution_context&, const launch_options&)
      -> unique_holder<application> = 0;

    [[nodiscard]] virtual auto check_requirements(video_context&) -> bool {
        return true;
    }

    [[nodiscard]] virtual auto check_requirements(audio_context&) -> bool {
        return true;
    }

    template <typename Application>
    auto launch_with_video(execution_context& ec)
      -> unique_holder<application> {
        return video_ctx(ec).and_then(
          [this, &ec](auto& vc) -> unique_holder<application> {
              vc.begin();
              if(vc.init_gl_api()) {
                  if(check_requirements(vc)) {
                      return {hold<Application>, ec, vc};
                  }
              }
              return {};
          });
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

