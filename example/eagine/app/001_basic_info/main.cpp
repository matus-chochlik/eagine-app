/// @example app/001_basic_info/main.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
import std;
import eagine.core;
import eagine.oglplus;
import eagine.oalplus;
import eagine.app;

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
        return _gl_info_printed and _al_info_printed;
    }

    void on_video_resize() noexcept final {}

    void update() noexcept final {
        _print_egl_info();
        _print_gl_info();
        _print_al_info();
        _video.commit(*this);
    }

    void clean_up() noexcept final {
        _video.end();
    }

private:
    void _print_egl_info() {
        const auto egl_cio{cio_print("EGL info:").to_be_continued()};

        _video.with_egl([&, this](auto& egl, auto& EGL) {
            if(const auto display{_video.egl_display()}) {
                if(const ok info{egl.query_string(display, EGL.vendor)}) {
                    egl_cio.print("Vendor: ${info}").arg("info", info);
                }

                if(const ok info{egl.query_string(display, EGL.version)}) {
                    egl_cio.print("Version: ${info}").arg("info", info);
                }

                const auto ext_cio{
                  egl_cio.print("EGL extensions:").to_be_continued()};

                for(auto name : egl.get_extensions(display)) {
                    ext_cio.print(name);
                }
                _egl_info_printed = true;
            }
        });
    }

    void _print_gl_info() {
        const auto gl_cio{cio_print("GL info:").to_be_continued()};

        _video.with_gl([&, this](auto& gl, auto& GL) {
            if(const ok info{gl.get_string(GL.vendor)}) {
                gl_cio.print("Vendor: ${info}").arg("info", info);
            }

            if(const ok info{gl.get_string(GL.renderer)}) {
                gl_cio.print("Renderer: ${info}").arg("info", info);
            }

            if(const ok info{gl.get_string(GL.version)}) {
                gl_cio.print("Version: ${info}").arg("info", info);
            }

            if(const ok info{gl.get_integer(GL.major_version)}) {
                gl_cio.print("Major version: ${info}").arg("info", info);
            }

            if(const ok info{gl.get_integer(GL.minor_version)}) {
                gl_cio.print("Minor version: ${info}").arg("info", info);
            }

            if(const ok info{gl.get_string(GL.shading_language_version)}) {
                gl_cio.print("GLSL version: ${info}").arg("info", info);
            }

            const auto ext_cio{
              gl_cio.print("GL extensions:").to_be_continued()};

            for(auto name : gl.get_extensions()) {
                ext_cio.print(name);
            }
            _gl_info_printed = true;
        });
    }

    void _print_al_info() {
        const auto al_cio{cio_print("AL info:").to_be_continued()};

        _audio.with_al([&, this](auto& al, auto& AL) {
            if(const ok info{al.get_string(AL.vendor)}) {
                al_cio.print("Vendor: ${info}").arg("info", info);
            }

            if(const ok info{al.get_string(AL.renderer)}) {
                al_cio.print("Renderer: ${info}").arg("info", info);
            }

            if(const ok info{al.get_string(AL.version)}) {
                al_cio.print("Version: ${info}").arg("info", info);
            }

            const auto ext_cio{
              al_cio.print("AL extensions:").to_be_continued()};

            for(auto name : al.get_extensions()) {
                ext_cio.print(name);
            }
            _al_info_printed = true;
        });
    }

    video_context& _video;
    audio_context& _audio;
    bool _egl_info_printed{false};
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

    auto check_requirements(video_context& vc) -> bool final {
        const auto check{[](auto& gl, auto& GL) {
            return gl.get_integer and gl.get_string and GL.vendor and
                   GL.renderer and GL.version and
                   GL.shading_language_version and GL.extensions;
        }};
        return vc.with_gl(check).or_false();
    }

    auto check_requirements(audio_context& ac) -> bool final {
        const auto check{[](auto& al, auto& AL) {
            return al.get_string and AL.vendor and AL.renderer and
                   AL.version and AL.extensions;
        }};
        return ac.with_al(check).or_false();
    }

    auto launch(execution_context& ec, const launch_options&)
      -> unique_holder<application> final {
        auto opt_vc{ec.video_ctx()};
        auto opt_ac{ec.audio_ctx()};
        if(opt_vc and opt_ac) {
            auto& vc = *opt_vc;
            auto& ac = *opt_ac;
            vc.begin();
            if(vc.init_gl_api(ec)) {
                if(check_requirements(vc)) {
                    ac.begin();
                    if(ac.init_al_api(ec)) {
                        if(check_requirements(ac)) {
                            return {hold<example_info>, ec, vc, ac};
                        } else {
                            ec.log_error("AL requirements not met");
                        }
                        ac.end();
                    } else {
                        ec.log_error("failed to initialize AL API");
                    }
                } else {
                    ec.log_error("GL requirements not met");
                }
                vc.end();
            } else {
                ec.log_error("failed to initialize GL API");
            }
        } else {
            if(!opt_vc) {
                ec.log_error("missing video context");
            }
            if(!opt_ac) {
                ec.log_error("missing audio context");
            }
        }
        return {};
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
