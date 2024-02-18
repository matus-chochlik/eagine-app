/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider:common;

import eagine.core;
import eagine.msgbus;
import eagine.eglplus;
import eagine.oglplus;
import eagine.app;
import std;
import :driver;

namespace eagine::app {
//------------------------------------------------------------------------------
// locator check functions
//------------------------------------------------------------------------------
auto is_valid_text_resource_url(const url&) noexcept -> bool;
auto is_valid_eagitex_resource_url(const url&) noexcept -> bool;
auto is_valid_eagitexi_resource_url(const url&) noexcept -> bool;
//------------------------------------------------------------------------------
// Base I/O implementations
//------------------------------------------------------------------------------
struct simple_string_source_blob_io
  : main_ctx_object
  , msgbus::source_blob_io {
protected:
    simple_string_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      std::string content) noexcept;

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    const std::string _content;
};
//------------------------------------------------------------------------------
struct simple_buffer_source_blob_io
  : main_ctx_object
  , msgbus::source_blob_io {
protected:
    simple_buffer_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      span_size_t buffer_size) noexcept;

    simple_buffer_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      span_size_t buffer_size,
      std::function<memory::buffer(memory::buffer)>) noexcept;

    void append(const memory::const_block);
    void append(const string_view);
    void append(const byte);

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    main_ctx_buffer _content;
};
//------------------------------------------------------------------------------
class compressed_buffer_source_blob_io : public simple_buffer_source_blob_io {
protected:
    compressed_buffer_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      span_size_t buffer_size) noexcept;

    compressed_buffer_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      span_size_t buffer_size,
      std::function<memory::buffer(memory::buffer)>) noexcept;

    void compress(const memory::const_block) noexcept;
    void compress(const string_view) noexcept;
    void compress(const byte) noexcept;
    void finish() noexcept;

private:
    auto _append_compressed(const memory::const_block) noexcept -> bool;
    auto _compress_handler() noexcept;
    stream_compression _compress;
};
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
      identifier id,
      main_ctx_parent parent,
      shared_provider_objects& shared,
      egl_rendered_source_context,
      const gl_rendered_source_params& params) noexcept;

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
class gl_rendered_source_blob_io : public compressed_buffer_source_blob_io {
public:
    static auto create_context(
      shared_provider_objects&,
      const gl_rendered_source_params&) noexcept -> egl_rendered_source_context;

protected:
    gl_rendered_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      shared_provider_objects& shared,
      egl_rendered_source_context,
      const gl_rendered_source_params& params,
      span_size_t buffer_size) noexcept;

    auto shared() const noexcept -> shared_provider_objects&;
    auto resource_context() noexcept -> loaded_resource_context&;
    auto display() const noexcept -> eglplus::display_handle;
    auto egl_api() const noexcept -> const eglplus::egl_api&;
    auto gl_api() const noexcept -> const oglplus::gl_api&;

    auto make_current() const noexcept -> bool;

private:
    shared_holder<gl_rendered_source_blob_context> _gl_context;
};
//------------------------------------------------------------------------------
class ostream_io final : public msgbus::source_blob_io {
public:
    auto ostream() noexcept -> std::ostream&;

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    std::stringstream _content;
};
//------------------------------------------------------------------------------
// base provider implementations
//------------------------------------------------------------------------------
struct eagitex_provider_base
  : main_ctx_object
  , resource_provider_interface {
protected:
    eagitex_provider_base(identifier id, main_ctx_parent parent) noexcept;

    static auto valid_color(int c) noexcept -> bool {
        return (c >= 0) and (c <= 255);
    }

    static auto valid_dimension(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_level(int l) noexcept -> bool {
        return (l >= 0);
    }
};
//------------------------------------------------------------------------------
// file-system search paths
//------------------------------------------------------------------------------
class filesystem_search_paths
  : public main_ctx_object
  , public std::vector<std::filesystem::path> {
public:
    filesystem_search_paths(identifier id, main_ctx_parent parent) noexcept;

private:
};
//------------------------------------------------------------------------------
} // namespace eagine::app

