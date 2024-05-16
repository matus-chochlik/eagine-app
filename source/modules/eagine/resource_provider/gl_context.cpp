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
struct gl_rendered_blob_params {
    valid_if_nonnegative<span_size_t> device_index{-1};
    valid_if_positive<int> surface_width{0};
    valid_if_positive<int> surface_height{0};
};
//------------------------------------------------------------------------------
struct egl_rendered_blob_context {
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
      const shared_provider_objects&,
      const gl_rendered_blob_params&,
      const eglplus::config_attributes,
      const eglplus::surface_attributes,
      const eglplus::context_attributes) noexcept -> egl_rendered_blob_context;

    egl_context_handler(
      const shared_provider_objects&,
      egl_rendered_blob_context) noexcept;
    ~egl_context_handler() noexcept final;

    auto shared() const noexcept -> const shared_provider_objects&;
    auto display() const noexcept -> eglplus::display_handle;
    auto egl_api() const noexcept -> const eglplus::egl_api&;
    auto make_current() noexcept -> bool final;

private:
    const shared_provider_objects& _shared;
    eglplus::initialized_display _display;
    eglplus::owned_surface_handle _surface;
    eglplus::owned_context_handle _context;
};
//------------------------------------------------------------------------------
class gl_rendered_blob_context : public main_ctx_object {
public:
    gl_rendered_blob_context(
      main_ctx_parent parent,
      const shared_provider_objects& shared,
      const gl_rendered_blob_params& params,
      egl_rendered_blob_context) noexcept;

    void debug_callback(
      oglplus::gl_types::enum_type source,
      oglplus::gl_types::enum_type type,
      oglplus::gl_types::uint_type id,
      oglplus::gl_types::enum_type severity,
      string_view message) const noexcept;

    auto shared() const noexcept -> const shared_provider_objects&;
    auto resource_context() noexcept -> loaded_resource_context& {
        return _resource_context;
    }

    auto egl_api() const noexcept -> const eglplus::egl_api&;
    auto display() const noexcept -> eglplus::display_handle;
    auto gl_api() const noexcept -> const oglplus::gl_api&;

    auto make_current() const noexcept -> bool;

private:
    void _enable_debug() noexcept;
    void _init_fbo(const gl_rendered_blob_params&) noexcept;

    shared_holder<egl_context_handler> _egl_context;
    loaded_resource_context _resource_context;
    oglplus::renderbuffer_object _color_rbo;
    oglplus::framebuffer_object _offscreen_fbo;
};
//------------------------------------------------------------------------------
class gl_rendered_source_blob_io;
class gl_blob_renderer
  : public main_ctx_object
  , public abstract<gl_blob_renderer> {
public:
    gl_blob_renderer(
      gl_rendered_source_blob_io& parent,
      shared_holder<gl_rendered_blob_context>) noexcept;

    virtual auto render() noexcept -> msgbus::blob_preparation_result = 0;

    auto renderer_name() const noexcept -> valid_if_not_empty<std::string>;
    auto vendor_name() const noexcept -> valid_if_not_empty<std::string>;
    auto version() const noexcept -> valid_if_not_empty<std::string>;
    auto driver_name() const noexcept -> valid_if_not_empty<std::string>;

protected:
    auto resource_context() noexcept -> loaded_resource_context&;
    auto shared() const noexcept -> const shared_provider_objects&;
    auto display() const noexcept -> eglplus::display_handle;
    auto egl_api() const noexcept -> const eglplus::egl_api&;
    auto gl_api() const noexcept -> const oglplus::gl_api&;

    void compress(const memory::const_block) noexcept;
    void compress(const string_view) noexcept;
    void compress(const byte) noexcept;

private:
    gl_rendered_source_blob_io& _parent;
    shared_holder<gl_rendered_blob_context> _gl_context;
};
//------------------------------------------------------------------------------
class eagitexi_cubemap_renderer : public gl_blob_renderer {
public:
    auto render() noexcept -> msgbus::blob_preparation_result final;

protected:
    eagitexi_cubemap_renderer(
      gl_rendered_source_blob_io& parent,
      string_view progress_label,
      const gl_rendered_blob_params& params,
      shared_holder<gl_rendered_blob_context> context,
      int size,
      int tile_size) noexcept;

    static auto _screen_position_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    auto _init_program(oglplus::program_object prog) noexcept {
        _prog = std::move(prog);
    }

    virtual auto prepare_render() noexcept -> msgbus::blob_preparation_result;

    auto prog() const noexcept -> oglplus::program_name {
        return _prog;
    }

    auto cube_size_px() const noexcept -> int {
        return _size;
    }

private:
    auto _build_screen() noexcept -> oglplus::geometry_and_bindings;
    void _render_tile() noexcept;
    void _save_cube_face() noexcept;

    auto _done_tiles() const noexcept -> span_size_t;
    auto _total_tiles() const noexcept -> span_size_t;

    main_ctx_buffer _buffer;

    const std::chrono::steady_clock::time_point _start{
      std::chrono::steady_clock::now()};
    const oglplus::geometry_and_bindings _screen;
    oglplus::program_object _prog;
    const int _size;
    const int _tile_size;
    const int _tiles_per_side{1};
    int _tile_x{0};
    int _tile_y{0};
    int _face_index{0};
    oglplus::owned_sync _finishing_face;

    activity_progress _prepare_progress;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
