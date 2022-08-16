/// @example app/001_basic_info/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include <eagine/app/main.hpp>
#include <eagine/app_config.hpp>
#include <eagine/timeout.hpp>

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/oalplus/al.hpp>
#include <eagine/oalplus/al_api.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class example_info
  : public main_ctx_object
  , public application {
public:
    example_info(execution_context& ec, video_context& vc, audio_context& ac)
      : main_ctx_object{"ApiInfo", ec}
      , _video{vc}
      , _audio{ac} {
        ec.connect_inputs().map_inputs().switch_input_mapping();
    }

    auto is_done() noexcept -> bool final {
        return _gl_info_printed && _al_info_printed;
    }

    void on_video_resize() noexcept final {}

    void update() noexcept final {
        _print_gl_info();
        _print_al_info();
        _video.commit();
    }

    void clean_up() noexcept final {
        _video.end();
    }

private:
    void _print_gl_info() {
        const auto gl_cio{cio_print("GL info:").to_be_continued()};

        const auto& [gl, GL] = _video.gl_api();

        if(const ok info{gl.get_string(GL.vendor)}) {
            gl_cio.print("Vendor: ${info}").arg("info", extract(info));
        }

        if(const ok info{gl.get_string(GL.renderer)}) {
            gl_cio.print("Renderer: ${info}").arg("info", extract(info));
        }

        if(const ok info{gl.get_string(GL.version)}) {
            gl_cio.print("Version: ${info}").arg("info", extract(info));
        }

        if(const ok info{gl.get_integer(GL.major_version)}) {
            gl_cio.print("Major version: ${info}").arg("info", extract(info));
        }

        if(const ok info{gl.get_integer(GL.minor_version)}) {
            gl_cio.print("Minor version: ${info}").arg("info", extract(info));
        }

        if(const ok info{gl.get_string(GL.shading_language_version)}) {
            gl_cio.print("GLSL version: ${info}").arg("info", extract(info));
        }

        const auto ext_cio{gl_cio.print("GL extensions:").to_be_continued()};

        if(const ok extensions{gl.get_extensions()}) {
            for(auto name : extensions) {
                ext_cio.print(name);
            }
        } else {
            ext_cio.error("failed to get GL extension list: ${message}")
              .arg("message", (!extensions).message());
        }
        _gl_info_printed = true;
    }

    void _print_al_info() {
        const auto al_cio{cio_print("AL info:").to_be_continued()};

        const auto& [al, AL] = _audio.al_api();

        if(const ok info{al.get_string(AL.vendor)}) {
            al_cio.print("Vendor: ${info}").arg("info", extract(info));
        }

        if(const ok info{al.get_string(AL.renderer)}) {
            al_cio.print("Renderer: ${info}").arg("info", extract(info));
        }

        if(const ok info{al.get_string(AL.version)}) {
            al_cio.print("Version: ${info}").arg("info", extract(info));
        }

        const auto ext_cio{al_cio.print("AL extensions:").to_be_continued()};

        if(const ok extensions{al.get_extensions()}) {
            for(auto name : extensions) {
                ext_cio.print(name);
            }
        } else {
            ext_cio.error("failed to get AL extension list: ${message}")
              .arg("message", (!extensions).message());
        }
        _al_info_printed = true;
    }

    video_context& _video;
    audio_context& _audio;
    bool _gl_info_printed{false};
    bool _al_info_printed{false};
};
//------------------------------------------------------------------------------
class example_launchpad : public launchpad {
public:
    auto setup(main_ctx&, launch_options& opts) -> bool final {
        opts.require_input();
        opts.require_video();
        opts.require_audio();
        return true;
    }

    auto check_requirements(video_context& vc) -> bool {
        const auto& [gl, GL] = vc.gl_api();

        return gl.get_integer && gl.get_string && GL.vendor && GL.renderer &&
               GL.version && GL.shading_language_version && GL.extensions;
    }

    auto check_requirements(audio_context& ac) -> bool {
        const auto& [al, AL] = ac.al_api();

        return al.get_string && AL.vendor && AL.renderer && AL.version &&
               AL.extensions;
    }

    auto launch(execution_context& ec, const launch_options&)
      -> std::unique_ptr<application> final {
        auto opt_vc{ec.video_ctx()};
        auto opt_ac{ec.audio_ctx()};
        if(opt_vc && opt_ac) {
            auto& vc = extract(opt_vc);
            auto& ac = extract(opt_ac);
            vc.begin();
            if(vc.init_gl_api()) {
                if(check_requirements(vc)) {
                    ac.begin();
                    if(ac.init_al_api()) {
                        if(check_requirements(ac)) {
                            return {std::make_unique<example_info>(ec, vc, ac)};
                        }
                        ac.end();
                    }
                }
                vc.end();
            }
        }
        return {};
    }
};
//------------------------------------------------------------------------------
auto establish(main_ctx&) -> std::unique_ptr<launchpad> {
    return {std::make_unique<example_launchpad>()};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
