/// @example app/001_clear/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
import std;
import eagine.core;
import eagine.oglplus;
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

        const auto sec = int(_ec.state().frame_time());

        gl.clear_color(
          (sec % 3 == 0) ? 1.F : 0.F,
          (sec % 3 == 1) ? 1.F : 0.F,
          (sec % 3 == 2) ? 1.F : 0.F,
          0.0F);

        gl.clear(GL.color_buffer_bit);

        _video.commit();
    }

    void clean_up() noexcept final {
        _video.end();
    }

private:
    execution_context& _ec;
    video_context& _video;
    timeout _is_done{std::chrono::seconds(10)};
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
              if(vc.init_gl_api()) {
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
