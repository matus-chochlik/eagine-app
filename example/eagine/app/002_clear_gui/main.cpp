/// @example app/002_clear_gui/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
import std;
import eagine.core;
import eagine.oglplus;
import eagine.guiplus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
class example_clear : public application {
public:
    example_clear(execution_context& ec, video_context& vc)
      : _ec{ec}
      , _video{vc} {
        ec.connect_inputs().map_inputs().switch_input_mapping();
    }

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final {
        const auto [width, height] = _video.surface_size();
        const auto& gl = _video.gl_api();
        gl.viewport(width, height);
    }

    void update() noexcept final {
        const auto& [gl, GL] = _video.gl_api();

        auto [color_name, r, g, b] = _colors[_color_index];
        (void)color_name;
        gl.clear_color(r, g, b, 0.F);
        gl.clear(GL.color_buffer_bit);

        _video.commit();
    }

    void update_gui(const guiplus::imgui_api& gui) noexcept final {
        bool show_window{true};
        if(gui.begin("background", show_window).or_false()) {
            auto color_name{std::get<0>(_colors[_color_index])};
            if(gui.begin_combo("select color", color_name).or_false()) {
                for(const auto i : index_range(_colors)) {
                    const bool is_selected{i == _color_index};
                    color_name = std::get<0>(_colors[i]);
                    if(gui.selectable(color_name, is_selected).or_false()) {
                        _color_index = i;
                    }
                    if(is_selected) {
                        gui.set_item_default_focus();
                        _is_done.reset();
                    }
                }
                gui.end_combo();
            }
            gui.end();
        }
    }

    void clean_up() noexcept final {
        _video.end();
    }

private:
    execution_context& _ec;
    video_context& _video;
    timeout _is_done{std::chrono::seconds(10)};

    std::size_t _color_index{0U};
    const std::array<std::tuple<std::string_view, float, float, float>, 3>
      _colors{
        {{"red", 0.9F, 0.3F, 0.3F},
         {"green", 0.3F, 0.9F, 0.3F},
         {"blue", 0.3F, 0.3F, 0.9F}}};
};
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.no_audio().require_input().require_video();
        return true;
    }

    auto check_requirements(video_context& vc) -> bool final {
        const auto& [gl, GL] = vc.gl_api();

        return gl.clear_color and gl.clear and GL.color_buffer_bit;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        return ec.video_ctx().and_then(
          [this, &ec](auto& vc) -> unique_holder<application> {
              vc.begin();
              if(vc.init_gl_api(ec)) {
                  if(check_requirements(vc)) {
                      return {hold<example_clear>, ec, vc};
                  }
              }
              return {};
          });
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
