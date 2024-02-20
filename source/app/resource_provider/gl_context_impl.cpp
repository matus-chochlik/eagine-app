/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import eagine.shapes;
import eagine.eglplus;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// egl_context_handler
//------------------------------------------------------------------------------
auto egl_context_handler_display_bind_api(
  const eglplus::egl_api& eglapi,
  eglplus::initialized_display display) noexcept
  -> eglplus::initialized_display {
    if(display) {
        const auto& [egl, EGL]{eglapi};
        const auto apis{egl.get_client_api_bits(display)};
        const bool has_gl{apis.has(EGL.opengl_bit)};
        const bool has_gles{apis.has(EGL.opengl_es_bit)};
        if(has_gl) {
            egl.bind_api(EGL.opengl_api);
        } else if(has_gles) {
            egl.bind_api(EGL.opengl_es_api);
        } else {
            display.clean_up();
        }
    }
    return display;
}
//------------------------------------------------------------------------------
auto egl_context_handler_display_choose(
  const eglplus::egl_api& eglapi,
  const gl_rendered_source_params& params) noexcept
  -> eglplus::initialized_display {
    const bool select_device = params.device_index.has_value();
    if(select_device) {
        const auto& egl{eglapi.operations()};
        if(const ok dev_count{egl.query_devices.count()}) {
            const auto n{std_size(dev_count)};
            std::vector<eglplus::egl_types::device_type> devices;
            devices.resize(n);
            if(egl.query_devices(cover(devices))) {
                for(const auto cur_dev_idx : integer_range(n)) {
                    bool matching_device = true;
                    auto device = eglplus::device_handle(devices[cur_dev_idx]);

                    if(params.device_index) {
                        if(std_size(*params.device_index) != cur_dev_idx) {
                            matching_device = false;
                        }
                    }

                    // TODO: try additional parameters (vendor, driver name,...)
                    if(matching_device) {
                        return egl_context_handler_display_bind_api(
                          eglapi, egl.get_open_platform_display(device));
                    }
                }
            }
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto egl_context_handler_open_display(
  shared_provider_objects& shared,
  const gl_rendered_source_params& params) noexcept
  -> eglplus::initialized_display {
    if(const auto eglapi{shared.apis.egl()}) {
        const auto& egl{eglapi->operations()};
        if(egl.EXT_device_enumeration) {
            if(auto display{
                 egl_context_handler_display_choose(*eglapi, params)}) {
                return display;
            }
        }
        return egl_context_handler_display_bind_api(
          *eglapi, egl.get_open_display());
    }
    return {};
}
//------------------------------------------------------------------------------
auto egl_context_handler::create_context(
  shared_provider_objects& shared,
  const gl_rendered_source_params& params,
  const eglplus::config_attributes config_attribs,
  const eglplus::surface_attributes surface_attribs,
  const eglplus::context_attributes context_attribs) noexcept
  -> egl_rendered_source_context {

    if(auto display{egl_context_handler_open_display(shared, params)}) {
        const auto& egl{shared.apis.egl()->operations()};

        if(const ok config{egl.choose_config(display, config_attribs)}) {

            if(ok surface{egl.create_pbuffer_surface(
                 display, config, surface_attribs)}) {

                if(ok context{egl.create_context(
                     display,
                     config,
                     eglplus::context_handle{},
                     context_attribs)}) {
                    if(egl.make_current(display, surface, context)) {
                        return {
                          .display = std::move(display),
                          .surface = std::move(surface.get()),
                          .context = std::move(context.get())};
                    }
                }
            }
        }
    }
    return {};
}
//------------------------------------------------------------------------------
egl_context_handler::egl_context_handler(
  shared_provider_objects& shared,
  egl_rendered_source_context context) noexcept
  : _shared{shared}
  , _display{std::move(context.display)}
  , _surface{std::move(context.surface)}
  , _context{std::move(context.context)} {}
//------------------------------------------------------------------------------
egl_context_handler::~egl_context_handler() noexcept {
    if(_display) {
        if(_context) {
            egl_api().clean_up(std::move(_context), _display);
        }
        if(_surface) {
            egl_api().clean_up(std::move(_surface), _display);
        }
        _display.clean_up();
    }
}
//------------------------------------------------------------------------------
auto egl_context_handler::shared() const noexcept -> shared_provider_objects& {
    return _shared;
}
//------------------------------------------------------------------------------
auto egl_context_handler::display() const noexcept -> eglplus::display_handle {
    return _display;
}
//------------------------------------------------------------------------------
auto egl_context_handler::egl_api() const noexcept -> const eglplus::egl_api& {
    auto ref{_shared.apis.egl()};
    assert(ref);
    return *ref;
}
//------------------------------------------------------------------------------
auto egl_context_handler::make_current() noexcept -> bool {
    return egl_api().make_current(_display, _surface, _context).has_value();
}
//------------------------------------------------------------------------------
// gl_rendered_source_blob_context
//------------------------------------------------------------------------------
gl_rendered_source_blob_context::gl_rendered_source_blob_context(
  main_ctx_parent parent,
  shared_provider_objects& shared,
  const gl_rendered_source_params& params,
  egl_rendered_source_context context) noexcept
  : main_ctx_object{"GLRSBlbCtx", parent}
  , _egl_context{default_selector, shared, std::move(context)}
  , _color_rbo{_resource_context.gl_api().gen_renderbuffers.object()}
  , _offscreen_fbo{_resource_context.gl_api().gen_framebuffers.object()} {
    if(debug_build) {
        _enable_debug();
    }
    _init_fbo(params);
}
//------------------------------------------------------------------------------
static void gl_rendered_source_debug_callback(
  oglplus::gl_types::enum_type source,
  oglplus::gl_types::enum_type type,
  oglplus::gl_types::uint_type id,
  oglplus::gl_types::enum_type severity,
  oglplus::gl_types::sizei_type length,
  const oglplus::gl_types::char_type* message,
  const void* raw_pio) {
    assert(raw_pio);
    const auto msg = length >= 0 ? string_view(message, span_size(length))
                                 : string_view(message);

    static_cast<const gl_rendered_source_blob_context*>(raw_pio)->debug_callback(
      source, type, id, severity, msg);
}
//------------------------------------------------------------------------------
void gl_rendered_source_blob_context::debug_callback(
  [[maybe_unused]] oglplus::gl_types::enum_type source,
  [[maybe_unused]] oglplus::gl_types::enum_type type,
  [[maybe_unused]] oglplus::gl_types::uint_type id,
  [[maybe_unused]] oglplus::gl_types::enum_type severity,
  [[maybe_unused]] string_view message) const noexcept {
    log_debug(message)
      .tag("glDbgOutpt")
      .arg("severity", "DbgOutSvrt", severity)
      .arg("source", "DbgOutSrce", source)
      .arg("type", "DbgOutType", type)
      .arg("id", id);
}
//------------------------------------------------------------------------------
void gl_rendered_source_blob_context::_enable_debug() noexcept {
    const auto& [gl, GL]{gl_api()};
    if(gl.ARB_debug_output) {
        log_info("enabling GL debug output");

        gl.debug_message_callback(
          &gl_rendered_source_debug_callback, static_cast<const void*>(this));

        gl.debug_message_control(
          GL.dont_care, GL.dont_care, GL.dont_care, GL.true_);

        gl.debug_message_insert(
          GL.debug_source_application,
          GL.debug_type_other,
          0U, // ID
          GL.debug_severity_medium,
          "successfully enabled GL debug output");
    }
}
//------------------------------------------------------------------------------
void gl_rendered_source_blob_context::_init_fbo(
  const gl_rendered_source_params& params) noexcept {
    const auto& [gl, GL]{gl_api()};
    gl.bind_renderbuffer(GL.renderbuffer, _color_rbo);
    gl.renderbuffer_storage(
      GL.renderbuffer,
      GL.rgba8,
      params.surface_width.value(),
      params.surface_height.value());
    gl.bind_framebuffer(GL.draw_framebuffer, _offscreen_fbo);
    gl.framebuffer_renderbuffer(
      GL.draw_framebuffer, GL.color_attachment0, GL.renderbuffer, _color_rbo);
    gl.bind_framebuffer(GL.read_framebuffer, _offscreen_fbo);
    gl.named_framebuffer_read_buffer(_offscreen_fbo, GL.color_attachment0);
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_context::make_current() const noexcept -> bool {
    return _egl_context->make_current();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_context::shared() const noexcept
  -> shared_provider_objects& {
    return _egl_context->shared();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_context::display() const noexcept
  -> eglplus::display_handle {
    return _egl_context->display();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_context::egl_api() const noexcept
  -> const eglplus::egl_api& {
    return _egl_context->egl_api();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_context::gl_api() const noexcept
  -> const oglplus::gl_api& {
    return _resource_context.gl_api();
}
//------------------------------------------------------------------------------
} // namespace eagine::app