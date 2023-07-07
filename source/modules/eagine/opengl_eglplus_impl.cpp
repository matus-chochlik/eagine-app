/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.reflection;
import eagine.core.logging;
import eagine.core.utility;
import eagine.core.valid_if;
import eagine.core.c_api;
import eagine.core.main_ctx;
import eagine.eglplus;
import :types;

namespace eagine::app {
//------------------------------------------------------------------------------
// surface
//------------------------------------------------------------------------------
class eglplus_opengl_surface
  : public main_ctx_object
  , public video_provider {
public:
    eglplus_opengl_surface(main_ctx_parent parent, eglplus::egl_api& egl)
      : main_ctx_object{"EGLPbuffer", parent}
      , _egl_api{egl} {}

    auto get_context_attribs(
      execution_context&,
      const bool gl_otherwise_gles,
      const launch_options&,
      const video_options&) const -> eglplus::context_attributes;

    auto initialize(
      execution_context&,
      const eglplus::display_handle,
      const eglplus::egl_types::config_type,
      const launch_options&,
      const video_options&) -> bool;

    auto initialize(
      execution_context&,
      const eglplus::display_handle,
      const valid_if_nonnegative<span_size_t>& device_idx,
      const launch_options&,
      const video_options&) -> bool;

    auto initialize(
      execution_context&,
      const identifier instance,
      const launch_options&,
      const video_options&) -> bool;

    void clean_up();

    auto video_kind() const noexcept -> video_context_kind final;
    auto instance_id() const noexcept -> identifier final;

    auto is_offscreen() noexcept -> tribool final;
    auto has_framebuffer() noexcept -> tribool final;
    auto surface_size() noexcept -> std::tuple<int, int> final;
    auto surface_aspect() noexcept -> float final;

    auto egl_ref() noexcept -> eglplus::egl_api_reference final;
    auto egl_display() noexcept -> eglplus::display_handle final;

    void parent_context_changed(const video_context&) final;
    void video_begin(execution_context&) final;
    void video_end(execution_context&) final;
    void video_commit(execution_context&) final;

private:
    eglplus::egl_api& _egl_api;
    identifier _instance_id;
    eglplus::display_handle _display{};
    eglplus::owned_surface_handle _surface{};
    eglplus::owned_context_handle _context{};

    int _width{1};
    int _height{1};
};
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::get_context_attribs(
  execution_context&,
  const bool gl_otherwise_gles,
  const launch_options&,
  const video_options& video_opts) const -> eglplus::context_attributes {
    const auto& EGL = _egl_api.constants();

    const auto add_major_version = [&](auto attribs) {
        return attribs + (EGL.context_major_version |
                          (video_opts.gl_version_major().value_or(3)));
    };

    const auto add_minor_version = [&](auto attribs) {
        eglplus::context_attrib_traits::value_type fallback = 0;
        if(gl_otherwise_gles) {
            if(not video_opts.gl_compatibility_context()) {
                fallback = 3;
            }
        }
        return attribs + (EGL.context_minor_version |
                          (video_opts.gl_version_minor().value_or(fallback)));
    };

    const auto add_profile_mask = [&](auto attribs) {
        const auto compat = video_opts.gl_compatibility_context();
        if(compat) {
            return attribs + (EGL.context_opengl_profile_mask |
                              EGL.context_opengl_compatibility_profile_bit);
        } else {
            return attribs + (EGL.context_opengl_profile_mask |
                              EGL.context_opengl_core_profile_bit);
        }
    };

    const auto add_debugging = [&](auto attribs) {
        return attribs +
               (EGL.context_opengl_debug | video_opts.gl_debug_context());
    };

    const auto add_robustness = [&](auto attribs) {
        return attribs + (EGL.context_opengl_robust_access |
                          video_opts.gl_robust_access());
    };

    return add_robustness(add_debugging(add_profile_mask(
      add_minor_version(add_major_version(eglplus::context_attributes())))));
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::initialize(
  execution_context& exec_ctx,
  const eglplus::display_handle display,
  const eglplus::egl_types::config_type config,
  const launch_options& opts,
  const video_options& video_opts) -> bool {
    const auto& [egl, EGL] = _egl_api;

    const auto apis{egl.get_client_api_bits(display)};
    const bool has_gl = apis.has(EGL.opengl_bit);
    const bool has_gles = apis.has(EGL.opengl_es_bit);

    if(not has_gl and not has_gles) {
        log_info("display does not support any OpenAPI APIs;skipping");
        return false;
    }

    log_info("display device supports GL APIs")
      .arg("OpenGL", yes_no_maybe(has_gl))
      .arg("OpenGL_ES", yes_no_maybe(has_gles))
      .arg("PreferES", yes_no_maybe(video_opts.prefer_gles()));

    const bool gl_otherwise_gles = has_gl and not video_opts.prefer_gles();

    _width = video_opts.surface_width().value_or(1);
    _height = video_opts.surface_height().value_or(1);

    const auto surface_attribs = (EGL.width | _width) + (EGL.height | _height);
    if(ok surface{
         egl.create_pbuffer_surface(display, config, surface_attribs)}) {
        _surface = std::move(surface.get());

        const auto gl_api = gl_otherwise_gles
                              ? eglplus::client_api(EGL.opengl_api)
                              : eglplus::client_api(EGL.opengl_es_api);

        if(const ok bound{egl.bind_api(gl_api)}) {
            const auto context_attribs = get_context_attribs(
              exec_ctx, gl_otherwise_gles, opts, video_opts);

            if(ok ctxt{egl.create_context(
                 display, config, eglplus::context_handle{}, context_attribs)}) {
                _context = std::move(ctxt.get());
                return true;
            } else {
                log_error("failed to create context")
                  .arg("message", (not ctxt).message());
            }
        } else {
            log_error("failed to bind OpenGL API")
              .arg("message", (not bound).message());
        }
    } else {
        log_error("failed to create pbuffer ${width}x${height}")
          .arg("width", _width)
          .arg("height", _height)
          .arg("message", (not surface).message());
    }
    return false;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::initialize(
  execution_context& exec_ctx,
  const eglplus::display_handle display,
  const valid_if_nonnegative<span_size_t>& device_idx,
  const launch_options& opts,
  const video_options& video_opts) -> bool {
    const auto& [egl, EGL] = _egl_api;

    if(device_idx) {
        log_info("trying EGL device ${index}").arg("index", *device_idx);
    } else {
        exec_ctx.log_info("trying default EGL display device");
    }

    if(const ok initialized{egl.initialize(display)}) {
        if(const ok conf_driver_name{video_opts.driver_name()}) {
            if(egl.MESA_query_driver(display)) {
                if(const ok driver_name{egl.get_display_driver_name(display)}) {
                    if(are_equal(video_opts.driver_name(), driver_name)) {
                        log_info("using the ${driver} MESA display driver")
                          .arg("driver", "Identifier", driver_name);
                    } else {
                        log_info(
                          "${current} does not match the configured "
                          "${config} display driver; skipping")
                          .arg("current", "Identifier", driver_name)
                          .arg("config", "Identifier", conf_driver_name);
                        return false;
                    }
                } else {
                    log_error("failed to get EGL display driver name");
                    return false;
                }
            } else {
                log_info(
                  "cannot determine current display driver to match "
                  "with configured ${config} driver; skipping")
                  .arg("config", "Identifier", conf_driver_name);
                return false;
            }
        } else {
            if(egl.MESA_query_driver(display)) {
                if(const ok driver_name{egl.get_display_driver_name(display)}) {
                    log_info("using the ${driver} MESA display driver")
                      .arg("driver", "Identifier", driver_name);
                }
            }
        }

        _display = display;

        const auto config_attribs =
          (EGL.red_size | (video_opts.color_bits().value_or(EGL.dont_care))) +
          (EGL.green_size | (video_opts.color_bits().value_or(EGL.dont_care))) +
          (EGL.blue_size | (video_opts.color_bits().value_or(EGL.dont_care))) +
          (EGL.alpha_size | (video_opts.alpha_bits().value_or(EGL.dont_care))) +
          (EGL.depth_size | (video_opts.depth_bits().value_or(EGL.dont_care))) +
          (EGL.stencil_size |
           (video_opts.stencil_bits().value_or(EGL.dont_care))) +
          (EGL.color_buffer_type | EGL.rgb_buffer) +
          (EGL.surface_type | EGL.pbuffer_bit) +
          (EGL.renderable_type | (EGL.opengl_bit | EGL.opengl_es3_bit));

        if(const ok count{egl.choose_config.count(_display, config_attribs)}) {
            log_info("found ${count} suitable framebuffer configurations")
              .arg("count", count);

            if(const ok config{egl.choose_config(_display, config_attribs)}) {
                return initialize(exec_ctx, _display, config, opts, video_opts);
            } else {
                const string_view dont_care{"-"};
                log_error("no matching framebuffer configuration found")
                  .arg("color", "integer", video_opts.color_bits(), dont_care)
                  .arg("alpha", "integer", video_opts.alpha_bits(), dont_care)
                  .arg("depth", "integer", video_opts.depth_bits(), dont_care)
                  .arg(
                    "stencil", "integer", video_opts.stencil_bits(), dont_care)
                  .arg("message", (not config).message());
            }
        } else {
            log_error("failed to query framebuffer configurations")
              .arg("message", (not count).message());
        }
    } else {
        exec_ctx.log_error("failed to initialize EGL display")
          .arg("message", (not initialized).message());
    }
    return false;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::initialize(
  execution_context& exec_ctx,
  const identifier id,
  const launch_options& opts,
  const video_options& video_opts) -> bool {
    _instance_id = id;
    const auto& [egl, EGL] = _egl_api;

    const ok device_kind{video_opts.device_kind()};
    const ok device_path{video_opts.device_path()};
    const ok device_idx{video_opts.device_index()};
    const bool select_device =
      device_kind or device_path or device_idx or video_opts.driver_name();

    if(select_device and egl.EXT_device_enumeration) {
        if(const ok dev_count{egl.query_devices.count()}) {
            const auto n = std_size(dev_count);
            std::vector<eglplus::egl_types::device_type> devices;
            devices.resize(n);
            if(egl.query_devices(cover(devices))) {
                for(const auto cur_dev_idx : integer_range(n)) {
                    bool matching_device = true;
                    auto device = eglplus::device_handle(devices[cur_dev_idx]);

                    if(device_idx) {
                        if(std_size(device_idx) == cur_dev_idx) {
                            log_info("explicitly selected device ${index}")
                              .arg("index", device_idx);
                        } else {
                            matching_device = false;
                            log_info(
                              "current device index is ${current} but, "
                              "device ${config} requested; skipping")
                              .arg("current", cur_dev_idx)
                              .arg("config", device_idx);
                        }
                    }

                    if(device_kind) {
                        if(device_kind == video_device_kind::hardware) {
                            if(not egl.MESA_device_software(device)) {
                                log_info(
                                  "device ${index} seems to be hardware as "
                                  "explicitly specified by configuration")
                                  .arg("index", cur_dev_idx);
                            } else {
                                matching_device = false;
                                log_info(
                                  "device ${index} is software but, "
                                  "hardware device requested; skipping")
                                  .arg("index", cur_dev_idx);
                            }
                        } else if(device_kind == video_device_kind::software) {
                            if(not egl.EXT_device_drm(device)) {
                                log_info(
                                  "device ${index} seems to be software as "
                                  "explicitly specified by configuration")
                                  .arg("index", cur_dev_idx);
                            } else {
                                matching_device = false;
                                log_info(
                                  "device ${index} is hardware but, "
                                  "software device requested; skipping")
                                  .arg("index", cur_dev_idx);
                            }
                        }
                    }

                    if(device_path) {
                        if(egl.EXT_device_drm(device)) {
                            if(const ok path{egl.query_device_string(
                                 device, EGL.drm_device_file)}) {
                                if(are_equal(device_path, path)) {
                                    log_info(
                                      "using DRM device ${path} as "
                                      "explicitly specified by configuration")
                                      .arg("path", "FsPath", path);
                                } else {
                                    matching_device = false;
                                    log_info(
                                      "device file is ${current}, but "
                                      "${config} was requested; skipping")
                                      .arg("current", "FsPath", path)
                                      .arg("config", "FsPath", device_path);
                                }
                            }
                        } else {
                            log_warning(
                              "${config} requested by config, but cannot "
                              "determine current device file path")
                              .arg("config", "FsPath", device_path);
                        }
                    }

                    if(matching_device) {
                        if(const ok display{egl.get_platform_display(device)}) {

                            if(initialize(
                                 exec_ctx,
                                 display,
                                 signedness_cast(cur_dev_idx),
                                 opts,
                                 video_opts)) {
                                return true;
                            } else {
                                _egl_api.terminate(display);
                            }
                        }
                    }
                }
            }
        }
    } else {
        if(const ok display{egl.get_display()}) {
            return initialize(exec_ctx, display, -1, opts, video_opts);
        } else {
            exec_ctx.log_error("failed to get EGL display")
              .arg("message", (not display).message());
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void eglplus_opengl_surface::clean_up() {
    if(_display) {
        if(_context) {
            _egl_api.destroy_context(_display, std::move(_context));
        }
        if(_surface) {
            _egl_api.destroy_surface(_display, std::move(_surface));
        }
        _egl_api.terminate(_display);
    }
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::video_kind() const noexcept -> video_context_kind {
    return video_context_kind::opengl;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::instance_id() const noexcept -> identifier {
    return _instance_id;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::is_offscreen() noexcept -> tribool {
    return true;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::has_framebuffer() noexcept -> tribool {
    return true;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::surface_size() noexcept -> std::tuple<int, int> {
    return {_width, _height};
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::surface_aspect() noexcept -> float {
    return float(_width) / float(_height);
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::egl_ref() noexcept -> eglplus::egl_api_reference {
    return {_egl_api};
}
//------------------------------------------------------------------------------
auto eglplus_opengl_surface::egl_display() noexcept -> eglplus::display_handle {
    return _display;
}
//------------------------------------------------------------------------------
void eglplus_opengl_surface::parent_context_changed(const video_context&) {}
//------------------------------------------------------------------------------
void eglplus_opengl_surface::video_begin(execution_context&) {
    _egl_api.make_current(_display, _surface, _context);
}
//------------------------------------------------------------------------------
void eglplus_opengl_surface::video_end(execution_context&) {
    _egl_api.make_current.none(_display);
}
//------------------------------------------------------------------------------
void eglplus_opengl_surface::video_commit(execution_context&) {
    _egl_api.swap_buffers(_display, _surface);
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class eglplus_opengl_provider
  : public main_ctx_object
  , public hmi_provider {
public:
    eglplus_opengl_provider(main_ctx_parent parent)
      : main_ctx_object{"EGLPPrvdr", parent} {}

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

private:
    eglplus::egl_api _egl_api;

    std::map<identifier, std::shared_ptr<eglplus_opengl_surface>> _surfaces;
};
//------------------------------------------------------------------------------
auto eglplus_opengl_provider::is_implemented() const noexcept -> bool {
    return _egl_api.get_display and _egl_api.initialize and
           _egl_api.terminate and _egl_api.get_configs and
           _egl_api.choose_config and _egl_api.get_config_attrib and
           _egl_api.query_string and _egl_api.swap_buffers;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_provider::implementation_name() const noexcept
  -> string_view {
    return {"eglplus"};
}
//------------------------------------------------------------------------------
auto eglplus_opengl_provider::is_initialized() -> bool {
    return not _surfaces.empty();
}
//------------------------------------------------------------------------------
auto eglplus_opengl_provider::should_initialize(execution_context& exec_ctx)
  -> bool {
    for(auto& entry : exec_ctx.options().video_requirements()) {
        auto& video_opts = std::get<1>(entry);
        if(video_opts.has_provider(implementation_name())) {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto eglplus_opengl_provider::initialize(execution_context& exec_ctx) -> bool {
    if(_egl_api.get_display) {
        auto& options = exec_ctx.options();
        for(auto& [inst, video_opts] : options.video_requirements()) {
            const bool should_create_surface =
              video_opts.has_provider(implementation_name()) and
              (video_opts.video_kind() == video_context_kind::opengl);

            if(should_create_surface) {
                if(auto surface{std::make_shared<eglplus_opengl_surface>(
                     *this, _egl_api)}) {
                    if(surface->initialize(
                         exec_ctx, inst, options, video_opts)) {
                        _surfaces[inst] = std::move(surface);
                    } else {
                        surface->clean_up();
                    }
                }
            }
        }
        return true;
    }
    exec_ctx.log_error("EGL is context is not supported");
    return false;
}
//------------------------------------------------------------------------------
void eglplus_opengl_provider::update(execution_context&) {}
//------------------------------------------------------------------------------
void eglplus_opengl_provider::clean_up(execution_context&) {
    for(auto& entry : _surfaces) {
        entry.second->clean_up();
    }
}
//------------------------------------------------------------------------------
void eglplus_opengl_provider::input_enumerate(
  callable_ref<void(std::shared_ptr<input_provider>)>) {}
//------------------------------------------------------------------------------
void eglplus_opengl_provider::video_enumerate(
  callable_ref<void(std::shared_ptr<video_provider>)> handler) {
    for(auto& p : _surfaces) {
        handler(p.second);
    }
}
//------------------------------------------------------------------------------
void eglplus_opengl_provider::audio_enumerate(
  callable_ref<void(std::shared_ptr<audio_provider>)>) {}
//------------------------------------------------------------------------------
auto make_eglplus_opengl_provider(main_ctx_parent parent)
  -> std::shared_ptr<hmi_provider> {
    return {std::make_shared<eglplus_opengl_provider>(parent)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
