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
import eagine.eglplus;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// locator check functions
//------------------------------------------------------------------------------
auto is_valid_text_resource_url(const url& locator) noexcept -> bool {
    return locator and
           (locator.has_scheme("text") or locator.has_scheme("txt") or
            locator.has_path_suffix(".text") or
            locator.has_path_suffix(".txt"));
}
//------------------------------------------------------------------------------
auto is_valid_eagitex_resource_url(const url& locator) noexcept -> bool {
    return locator and (locator.has_scheme("eagitex") or
                        locator.has_path_suffix(".eagitex"));
}
//------------------------------------------------------------------------------
auto is_valid_eagitexi_resource_url(const url& locator) noexcept -> bool {
    return locator and (locator.has_scheme("eagitexi") or
                        locator.has_path_suffix(".eagitexi"));
}
//------------------------------------------------------------------------------
// simple_string_source_blob_io
//------------------------------------------------------------------------------
simple_string_source_blob_io::simple_string_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  std::string content) noexcept
  : main_ctx_object{id, parent}
  , _content{std::move(content)} {}
//------------------------------------------------------------------------------
auto simple_string_source_blob_io::total_size() noexcept -> span_size_t {
    return span_size(_content.size());
}
//------------------------------------------------------------------------------
auto simple_string_source_blob_io::fetch_fragment(
  const span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    return copy(head(skip(view(_content), offs), dst.size()), dst).size();
}
//------------------------------------------------------------------------------
// simple_buffer_source_blob_io
//------------------------------------------------------------------------------
simple_buffer_source_blob_io::simple_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t buffer_size) noexcept
  : main_ctx_object{id, parent}
  , _content{*this, buffer_size} {
    _content.clear();
}
//------------------------------------------------------------------------------
simple_buffer_source_blob_io::simple_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t buffer_size,
  std::function<memory::buffer(memory::buffer)> make_content) noexcept
  : main_ctx_object{id, parent}
  , _content{*this, make_content(main_context().buffers().get(buffer_size))} {}
//------------------------------------------------------------------------------
void simple_buffer_source_blob_io::append(const memory::const_block part) {
    memory::append_to(part, _content);
}
//------------------------------------------------------------------------------
void simple_buffer_source_blob_io::append(const string_view part) {
    append(as_bytes(part));
}
//------------------------------------------------------------------------------
void simple_buffer_source_blob_io::append(const byte b) {
    append(view_one(b));
}
//------------------------------------------------------------------------------
auto simple_buffer_source_blob_io::total_size() noexcept -> span_size_t {
    return span_size(_content.size());
}
//------------------------------------------------------------------------------
auto simple_buffer_source_blob_io::fetch_fragment(
  const span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    return copy(head(skip(view(_content), offs), dst.size()), dst).size();
}
//------------------------------------------------------------------------------
// compressed_buffer_source_blob_io
//------------------------------------------------------------------------------
auto compressed_buffer_source_blob_io::_append_compressed(
  const memory::const_block packed) noexcept -> bool {
    this->append(packed);
    return true;
}
//------------------------------------------------------------------------------
auto compressed_buffer_source_blob_io::_compress_handler() noexcept {
    return data_compressor::data_handler{
      this,
      member_function_constant_t<
        &compressed_buffer_source_blob_io::_append_compressed>{}};
}
//------------------------------------------------------------------------------
compressed_buffer_source_blob_io::compressed_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t buffer_size) noexcept
  : simple_buffer_source_blob_io{id, parent, buffer_size}
  , _compress{
      main_context().compressor(),
      _compress_handler(),
      default_data_compression_method()} {}
//------------------------------------------------------------------------------
compressed_buffer_source_blob_io::compressed_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t buffer_size,
  std::function<memory::buffer(memory::buffer)> func) noexcept
  : simple_buffer_source_blob_io{id, parent, buffer_size, func}
  , _compress{
      main_context().compressor(),
      _compress_handler(),
      default_data_compression_method()} {}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::compress(
  const memory::const_block blk) noexcept {
    _compress.next(blk, data_compression_level::highest);
}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::compress(const string_view str) noexcept {
    compress(as_bytes(str));
}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::compress(const byte byt) noexcept {
    compress(view_one(byt));
}
//------------------------------------------------------------------------------
void compressed_buffer_source_blob_io::finish() noexcept {
    _compress.finish();
}
//------------------------------------------------------------------------------
// gl_rendered_source_blob_io
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::config_attribs(
  shared_provider_objects& shared,
  const gl_rendered_source_params&) noexcept -> eglplus::config_attributes {
    const auto& EGL{shared.apis.egl()->constants()};
    return (EGL.buffer_size | 32) + (EGL.red_size | 8) + (EGL.green_size | 8) +
           (EGL.blue_size | 8) + (EGL.alpha_size | 8) +
           (EGL.depth_size | EGL.dont_care) +
           (EGL.stencil_size | EGL.dont_care) +
           (EGL.color_buffer_type | EGL.rgb_buffer) +
           (EGL.surface_type | EGL.pbuffer_bit) +
           (EGL.renderable_type | EGL.opengl_bit);
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::surface_attribs(
  shared_provider_objects& shared,
  const gl_rendered_source_params& params) noexcept
  -> eglplus::surface_attributes {
    const auto& EGL{shared.apis.egl()->constants()};
    return (EGL.width | params.surface_width.value()) +
           (EGL.height | params.surface_height.value());
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::context_attribs(
  shared_provider_objects& shared,
  const gl_rendered_source_params&) noexcept -> eglplus::context_attributes {
    const auto& EGL{shared.apis.egl()->constants()};
    return (EGL.context_opengl_profile_mask |
            EGL.context_opengl_core_profile_bit) +
           (EGL.context_major_version | 3) + (EGL.context_minor_version | 3) +
           (EGL.context_opengl_debug | debug_build) +
           (EGL.context_opengl_robust_access | true);
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::create_context(
  main_ctx_parent parent,
  shared_provider_objects& shared,
  const gl_rendered_source_params& params) noexcept
  -> shared_holder<gl_rendered_source_blob_context> {
    if(auto context{egl_context_handler::create_context(
         shared,
         params,
         config_attribs(shared, params),
         surface_attribs(shared, params),
         context_attribs(shared, params))}) {
        return {default_selector, parent, shared, params, std::move(context)};
    }
    return {};
}
//------------------------------------------------------------------------------
gl_rendered_source_blob_io::gl_rendered_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  shared_holder<gl_rendered_source_blob_context> gl_context,
  span_size_t buffer_size) noexcept
  : compressed_buffer_source_blob_io{id, parent, buffer_size}
  , _gl_context{std::move(gl_context)} {}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::make_current() const noexcept -> bool {
    return _gl_context->make_current();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::shared() const noexcept
  -> shared_provider_objects& {
    return _gl_context->shared();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::display() const noexcept
  -> eglplus::display_handle {
    return _gl_context->display();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::resource_context() noexcept
  -> loaded_resource_context& {
    return _gl_context->resource_context();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::egl_api() const noexcept
  -> const eglplus::egl_api& {
    return _gl_context->egl_api();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::gl_api() const noexcept
  -> const oglplus::gl_api& {
    return _gl_context->gl_api();
}
//------------------------------------------------------------------------------
// ostream_io
//------------------------------------------------------------------------------
auto ostream_io::ostream() noexcept -> std::ostream& {
    return _content;
}
//------------------------------------------------------------------------------
auto ostream_io::total_size() noexcept -> span_size_t {
    return span_size(_content.view().size());
}
//------------------------------------------------------------------------------
auto ostream_io::fetch_fragment(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    const auto src{skip(memory::view(_content.view()), offs)};
    dst = head(dst, src);
    return copy(head(src, dst), dst).size();
}
//------------------------------------------------------------------------------
// eagitex_provider_base
//------------------------------------------------------------------------------
eagitex_provider_base::eagitex_provider_base(
  identifier id,
  main_ctx_parent parent) noexcept
  : main_ctx_object{id, parent} {}
//------------------------------------------------------------------------------
// filesystem_search_paths
//------------------------------------------------------------------------------
filesystem_search_paths::filesystem_search_paths(
  identifier id,
  main_ctx_parent parent) noexcept
  : main_ctx_object{id, parent} {
    std::vector<std::string> paths;
    main_context().config().fetch("app.resource_provider.root_path", paths);
    main_context().config().fetch("app.resource_provider.root_paths", paths);

    using fspath = std::filesystem::path;

    if(const auto path{build_info()
                         .install_prefix()
                         .transform(_1.cast_to<std::string>())
                         .transform(_1.cast_to<fspath>())
                         .transform([](auto prefix) {
                             return prefix / "share" / "eagine" / "assets";
                         })}) {
        if(std::filesystem::is_directory(*path)) {
            this->emplace_back(*path);
        }
    }

    if(const fspath path{"/usr/share/eagine/assets"}; is_directory(path)) {
        if(not find(*this, path)) {
            this->emplace_back(path);
        }
    }

    for(fspath path : paths) {
        if(is_directory(path)) {
            if(not find(*this, path)) {
                this->emplace_back(std::move(path));
            }
        } else {
            log_warning("'${path}' is not a path to existing directory")
              .arg("path", "FsPath", path.string());
        }
    }
    log_info("configured resource directories: ${path}")
      .arg_func([&](logger_backend& backend) {
          for(auto& path : *this) {
              backend.add_string("path", "FsPath", path.string());
          }
      });
}
//------------------------------------------------------------------------------
} // namespace eagine::app

