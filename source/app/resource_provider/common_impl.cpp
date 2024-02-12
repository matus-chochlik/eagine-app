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
  span_size_t size) noexcept
  : main_ctx_object{id, parent}
  , _content{*this, size} {
    _content.clear();
}
//------------------------------------------------------------------------------
simple_buffer_source_blob_io::simple_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t size,
  std::function<memory::buffer(memory::buffer)> make_content) noexcept
  : main_ctx_object{id, parent}
  , _content{*this, make_content(main_context().buffers().get(size))} {}
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
  span_size_t size) noexcept
  : simple_buffer_source_blob_io{id, parent, size}
  , _compress{
      main_context().compressor(),
      _compress_handler(),
      default_data_compression_method()} {}
//------------------------------------------------------------------------------
compressed_buffer_source_blob_io::compressed_buffer_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  span_size_t size,
  std::function<memory::buffer(memory::buffer)> func) noexcept
  : simple_buffer_source_blob_io{id, parent, size, func}
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
gl_rendered_source_blob_io::gl_rendered_source_blob_io(
  identifier id,
  main_ctx_parent parent,
  shared_provider_objects& shared,
  gl_rendered_source_context context,
  span_size_t size) noexcept
  : compressed_buffer_source_blob_io{id, parent, size}
  , _shared{shared}
  , _display{std::move(context.display)}
  , _surface{std::move(context.surface)}
  , _context{std::move(context.context)} {}
//------------------------------------------------------------------------------
gl_rendered_source_blob_io::~gl_rendered_source_blob_io() noexcept {
    if(_display) {
        if(_context) {
            eglapi().clean_up(std::move(_context), _display);
        }
        if(_surface) {
            eglapi().clean_up(std::move(_surface), _display);
        }
        _display.clean_up();
    }
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io_display_bind_api(
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
auto gl_rendered_source_blob_io_display_choose(
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
                        return gl_rendered_source_blob_io_display_bind_api(
                          eglapi, egl.get_open_platform_display(device));
                    }
                }
            }
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io_open_display(
  shared_provider_objects& shared,
  const gl_rendered_source_params& params) noexcept
  -> eglplus::initialized_display {
    if(const auto eglapi{shared.apis.egl()}) {
        const auto& egl{eglapi->operations()};
        if(egl.EXT_device_enumeration) {
            if(auto display{
                 gl_rendered_source_blob_io_display_choose(*eglapi, params)}) {
                return display;
            }
        }
        return gl_rendered_source_blob_io_display_bind_api(
          *eglapi, egl.get_open_display());
    }
    return {};
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::create_context(
  shared_provider_objects& shared,
  const gl_rendered_source_params& params) noexcept
  -> gl_rendered_source_context {

    if(auto display{gl_rendered_source_blob_io_open_display(shared, params)}) {
        const auto& [egl, EGL]{*shared.apis.egl()};

        const auto config_attribs =
          (EGL.red_size | 8) + (EGL.green_size | 8) + (EGL.blue_size | 8) +
          (EGL.alpha_size | 8) + (EGL.depth_size | EGL.dont_care) +
          (EGL.stencil_size | EGL.dont_care) +
          (EGL.color_buffer_type | EGL.rgb_buffer) +
          (EGL.surface_type | EGL.pbuffer_bit) +
          (EGL.renderable_type | (EGL.opengl_bit | EGL.opengl_es3_bit));

        if(const ok config{egl.choose_config(display, config_attribs)}) {

            const auto surface_attribs =
              (EGL.width | params.surface_width.value()) +
              (EGL.height | params.surface_height.value());

            if(ok surface{egl.create_pbuffer_surface(
                 display, config, surface_attribs)}) {
                const auto context_attribs =
                  (EGL.context_opengl_profile_mask |
                   EGL.context_opengl_core_profile_bit) +
                  (EGL.context_major_version | 3) +
                  (EGL.context_minor_version | 3) +
                  (EGL.context_opengl_robust_access | true);

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
auto gl_rendered_source_blob_io::make_current() const noexcept -> bool {
    return eglapi().make_current(_display, _surface, _context).has_value();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::swap_buffers() const noexcept -> bool {
    return eglapi().swap_buffers(_display, _surface).has_value();
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::eglapi() const noexcept
  -> const eglplus::egl_api& {
    auto ref{_shared.apis.egl()};
    assert(ref);
    return *ref;
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::glapi() const noexcept
  -> const oglplus::gl_api& {
    return _glapi;
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::display() const noexcept
  -> eglplus::display_handle {
    return _display;
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::surface() const noexcept
  -> eglplus::surface_handle {
    return _surface;
}
//------------------------------------------------------------------------------
auto gl_rendered_source_blob_io::context() const noexcept
  -> eglplus::context_handle {
    return _context;
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

