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
import :gl_context;

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
public:
    void compress(const memory::const_block) noexcept;
    void compress(const string_view) noexcept;
    void compress(const byte) noexcept;

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

    void finish() noexcept;

private:
    auto _append_compressed(const memory::const_block) noexcept -> bool;
    auto _compress_handler() noexcept;
    stream_compression _compress;
};
//------------------------------------------------------------------------------
class gl_rendered_source_blob_io : public compressed_buffer_source_blob_io {
public:
    auto prepare() noexcept -> msgbus::blob_preparation final;

    auto shared() const noexcept -> shared_provider_objects& {
        return _shared;
    }

    auto params() const noexcept -> const gl_rendered_blob_params& {
        return _params;
    }

protected:
    gl_rendered_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      shared_provider_objects& shared,
      const gl_rendered_blob_params& params,
      span_size_t buffer_size) noexcept;

    virtual auto config_attribs(shared_provider_objects&) noexcept
      -> eglplus::config_attributes;

    virtual auto surface_attribs(shared_provider_objects&) noexcept
      -> eglplus::surface_attributes;

    virtual auto context_attribs(shared_provider_objects&) noexcept
      -> eglplus::context_attributes;

    virtual auto make_renderer(shared_holder<gl_rendered_blob_context>)
      -> shared_holder<gl_blob_renderer> = 0;

private:
    auto _create_context() noexcept -> shared_holder<gl_rendered_blob_context>;

    shared_provider_objects& _shared;
    gl_rendered_blob_params _params;
    shared_holder<gl_blob_renderer> _renderer;
    bool _finished{false};
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

