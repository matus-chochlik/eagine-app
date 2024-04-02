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
// Renderer
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_renderer final : public eagitexi_cubemap_renderer {
public:
    eagitexi_cubemap_blur_renderer(
      gl_rendered_source_blob_io& parent,
      const gl_rendered_blob_params& params,
      shared_holder<gl_rendered_blob_context> context,
      url source,
      int size,
      int sharpness) noexcept;

    ~eagitexi_cubemap_blur_renderer() noexcept final;

    auto prepare_render() noexcept -> msgbus::blob_preparation_result final;

private:
    static auto _tile_size() noexcept -> int;
    static auto _vs_source() noexcept -> string_view;
    static auto _fs_source() noexcept -> string_view;
    auto _build_program(const gl_rendered_blob_params&, int) noexcept
      -> oglplus::program_object;
    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;

    gl_texture_resource _cubemap;

    const signal_binding _sig_binding{
      _cubemap.loaded.bind_to<&eagitexi_cubemap_blur_renderer::_on_tex_loaded>(
        this)};
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_renderer::eagitexi_cubemap_blur_renderer(
  gl_rendered_source_blob_io& parent,
  const gl_rendered_blob_params& params,
  shared_holder<gl_rendered_blob_context> context,
  url source,
  int size,
  int sharpness) noexcept
  : eagitexi_cubemap_renderer{parent, "blurring cube-map", params, std::move(context), size, _tile_size()}
  , _cubemap{std::move(source), resource_context()} {
    _init_program(_build_program(params, sharpness));
}
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_renderer::~eagitexi_cubemap_blur_renderer() noexcept {
    _cubemap.clean_up(resource_context());
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_renderer::_tile_size() noexcept -> int {
    return 16;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_renderer::_build_program(
  const gl_rendered_blob_params& params,
  int sharpness) noexcept -> oglplus::program_object {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};

    // program
    auto prog{glapi.create_program_object()};
    glapi.add_shader(prog, GL.vertex_shader, embedded<"iCmBlurVS">());
    glapi.add_shader(prog, GL.fragment_shader, embedded<"iCmBlurFS">());
    gl.link_program(prog);
    gl.use_program(prog);

    gl.bind_attrib_location(prog, _screen_position_loc(), "Position");
    glapi.try_set_uniform(
      prog,
      "cubeSide",
      std::min(
        std::max(params.surface_width.value(), params.surface_height.value()),
        128));

    glapi.try_set_uniform(prog, "sharpness", sharpness);
    glapi.try_set_uniform(prog, "cubeMap", 0);

    return prog;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_renderer::_on_tex_loaded(
  const gl_texture_resource::load_info& loaded) noexcept {
    const auto& GL{gl_api().constants()};
    loaded.parameter_i(GL.texture_min_filter, GL.linear);
    loaded.parameter_i(GL.texture_mag_filter, GL.linear);
    loaded.parameter_i(GL.texture_wrap_s, GL.clamp_to_edge);
    loaded.parameter_i(GL.texture_wrap_t, GL.clamp_to_edge);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_renderer::prepare_render() noexcept
  -> msgbus::blob_preparation_result {
    const auto& GL{gl_api().constants()};
    if(_cubemap.load_if_needed(
         resource_context(), GL.texture_cube_map, GL.texture0)) {
        return {msgbus::blob_preparation_status::working};
    }
    return msgbus::blob_preparation_result::finished();
}
//------------------------------------------------------------------------------
// I/O
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_io final : public gl_rendered_source_blob_io {
public:
    eagitexi_cubemap_blur_io(
      main_ctx_parent,
      const shared_provider_objects& shared,
      const gl_rendered_blob_params& params,
      url source,
      int size,
      int sharpness,
      int level) noexcept;

protected:
    auto make_renderer(shared_holder<gl_rendered_blob_context>) noexcept
      -> unique_holder<gl_blob_renderer> final;

private:
    void _make_header_bgn(int size, int level) noexcept;
    void _make_header_end(eagitexi_cubemap_blur_renderer&) noexcept;

    const url _source;
    const int _size;
    const int _sharpness;
};
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_io::_make_header_bgn(int size, int level) noexcept {
    std::stringstream hdr;
    hdr << R"({"level":)" << level;
    hdr << R"(,"width":)" << size;
    hdr << R"(,"height":)" << size;
    hdr << R"(,"depth":)" << 6;
    hdr << R"(,"channels":4)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"rgba")";
    hdr << R"(,"iformat":"rgba8")";
    hdr << R"(,"tag":["blur","cubemap"])";
    hdr << R"(,"data_filter":"zlib")";
    append(hdr.str());
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_io::_make_header_end(
  eagitexi_cubemap_blur_renderer& renderer) noexcept {
    std::stringstream hdr;

    if(const auto vendor_name{renderer.renderer_name()}) {
        hdr << R"(,"metadata":{"renderer":{)";
        hdr << R"("vendor":")" << *vendor_name << R"(")";
        if(const auto renderer_name{renderer.renderer_name()}) {
            hdr << R"(,"name":")" << *renderer_name << R"(")";
        }
        if(const auto version{renderer.version()}) {
            hdr << R"(,"version":")" << *version << R"(")";
        }
        if(const auto driver_name{renderer.driver_name()}) {
            hdr << R"(,"driver":")" << *driver_name << R"(")";
        }
        hdr << R"(}})";
    }
    hdr << "}";
    append(hdr.str());
}
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_io::eagitexi_cubemap_blur_io(
  main_ctx_parent parent,
  const shared_provider_objects& shared,
  const gl_rendered_blob_params& params,
  url source,
  int size,
  int sharpness,
  int level) noexcept
  : gl_rendered_source_blob_io{"ITxCubBlur", parent, shared, params, size * size * 6}
  , _source{std::move(source)}
  , _size{size}
  , _sharpness{sharpness} {
    _make_header_bgn(size, level);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::make_renderer(
  shared_holder<gl_rendered_blob_context> context) noexcept
  -> unique_holder<gl_blob_renderer> {
    unique_holder<eagitexi_cubemap_blur_renderer> renderer{
      default_selector,
      *this,
      params(),
      std::move(context),
      _source,
      _size,
      _sharpness};

    assert(renderer);
    _make_header_end(*renderer);

    return renderer;
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagitexi_cubemap_blur_provider(const provider_parameters&) noexcept;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t size) noexcept
      -> std::chrono::seconds final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    const shared_provider_objects& _shared;
    application_config_value<int> _device_index{
      main_context().config(),
      "application.resource_provider.cubemap_blur.device_index",
      -1};
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_provider::eagitexi_cubemap_blur_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PTxCubBlur", params.parent}
  , _shared{params.shared} {}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(
      is_valid_eagitexi_resource_url(locator) and
      locator.has_path("cube_map_blur")) {
        return is_valid_eagitex_resource_url(locator.query().arg_url("source"));
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    if(has_resource(locator)) {
        const auto& q{locator.query()};
        const auto size{q.arg_value_as<int>("size").value_or(1024)};
        const auto sharpness{q.arg_value_as<int>("sharpness").value_or(8)};
        const auto level{q.arg_value_as<int>("level").value_or(0)};

        gl_rendered_blob_params params{
          .device_index = _device_index.value(),
          .surface_width = size,
          .surface_height = size};

        q.arg_value_as<int>("device_index")
          .and_then(_1.assign_to(params.device_index));

        return {
          hold<eagitexi_cubemap_blur_io>,
          as_parent(),
          _shared,
          params,
          q.arg_url("source"),
          size,
          sharpness,
          level};
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::get_blob_timeout(const span_size_t) noexcept
  -> std::chrono::seconds {
    return adjusted_duration(std::chrono::minutes{10});
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///cube_map_blur");
}
//------------------------------------------------------------------------------
auto provider_eagitexi_cubemap_blur(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_cubemap_blur_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app

