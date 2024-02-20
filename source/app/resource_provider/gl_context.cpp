/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider:gl_context;

import eagine.core;
import eagine.msgbus;
import eagine.eglplus;
import eagine.oglplus;
import eagine.app;
import std;
import :driver;

namespace eagine::app {
//------------------------------------------------------------------------------
struct gl_rendered_source_params {
    valid_if_nonnegative<span_size_t> device_index{-1};
    valid_if_positive<int> surface_width{0};
    valid_if_positive<int> surface_height{0};
};
//------------------------------------------------------------------------------
struct egl_rendered_source_context {
    eglplus::initialized_display display;
    eglplus::owned_surface_handle surface;
    eglplus::owned_context_handle context;

    explicit operator bool() const noexcept {
        return display and surface and context;
    }
};
//------------------------------------------------------------------------------
class egl_context_handler final : public oglplus::gl_context_handler {
public:
    static auto create_context(
      shared_provider_objects&,
      const gl_rendered_source_params&,
      const eglplus::config_attributes,
      const eglplus::surface_attributes,
      const eglplus::context_attributes) noexcept
      -> egl_rendered_source_context;

    egl_context_handler(
      shared_provider_objects&,
      egl_rendered_source_context) noexcept;
    ~egl_context_handler() noexcept final;

    auto shared() const noexcept -> shared_provider_objects&;
    auto display() const noexcept -> eglplus::display_handle;
    auto egl_api() const noexcept -> const eglplus::egl_api&;
    auto make_current() noexcept -> bool final;

private:
    shared_provider_objects& _shared;
    eglplus::initialized_display _display;
    eglplus::owned_surface_handle _surface;
    eglplus::owned_context_handle _context;
};
//------------------------------------------------------------------------------
class gl_rendered_source_blob_context : public main_ctx_object {
public:
    gl_rendered_source_blob_context(
      main_ctx_parent parent,
      shared_provider_objects& shared,
      const gl_rendered_source_params& params,
      egl_rendered_source_context) noexcept;

    void debug_callback(
      oglplus::gl_types::enum_type source,
      oglplus::gl_types::enum_type type,
      oglplus::gl_types::uint_type id,
      oglplus::gl_types::enum_type severity,
      string_view message) const noexcept;

    auto shared() const noexcept -> shared_provider_objects&;
    auto resource_context() noexcept -> loaded_resource_context& {
        return _resource_context;
    }

    auto egl_api() const noexcept -> const eglplus::egl_api&;
    auto display() const noexcept -> eglplus::display_handle;
    auto gl_api() const noexcept -> const oglplus::gl_api&;

    auto make_current() const noexcept -> bool;

private:
    void _enable_debug() noexcept;
    void _init_fbo(const gl_rendered_source_params&) noexcept;

    shared_holder<egl_context_handler> _egl_context;
    loaded_resource_context _resource_context{shared().loader, _egl_context};
    oglplus::renderbuffer_object _color_rbo;
    oglplus::framebuffer_object _offscreen_fbo;
};
//------------------------------------------------------------------------------
} // namespace eagine::app