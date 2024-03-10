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
// Planet params
//------------------------------------------------------------------------------
struct cubemap_scene {
    template <typename T>
    static auto query_arg(const url&, string_view name, T fallback) noexcept
      -> T;

    cubemap_scene(const url& locator) noexcept;

    float planet_radius_m;
    float atmosphere_thickness_m;
    float above_ground_m;
    float sun_x;
    float sun_y;
    float sun_z;
};
//------------------------------------------------------------------------------
template <typename T>
auto cubemap_scene::query_arg(
  const url& locator,
  string_view name,
  T fallback) noexcept -> T {
    return locator.query().arg_value_as<T>(name).value_or(fallback);
}
//------------------------------------------------------------------------------
cubemap_scene::cubemap_scene(const url& l) noexcept
  : planet_radius_m{query_arg<float>(l, "planet_radius_m", 6'370'000.F)}
  , atmosphere_thickness_m{query_arg<float>(l, "planet_atmosphere_m", 100'000.F)}
  , above_ground_m{query_arg<float>(l, "above_ground_m", 100.F)}
  , sun_x{query_arg<float>(l, "sun_x", 1.0F)}
  , sun_y{query_arg<float>(l, "sun_y", 1.0F)}
  , sun_z{query_arg<float>(l, "sun_z", 1.0F)} {}
//------------------------------------------------------------------------------
// Renderer
//------------------------------------------------------------------------------
class eagitexi_cubemap_sky_renderer final : public eagitexi_cubemap_renderer {
public:
    eagitexi_cubemap_sky_renderer(
      gl_rendered_source_blob_io& parent,
      const gl_rendered_blob_params& params,
      const cubemap_scene& scene,
      shared_holder<gl_rendered_blob_context> context,
      int size) noexcept;

    ~eagitexi_cubemap_sky_renderer() noexcept final;

private:
    static auto _tile_size() noexcept -> int;
    auto _build_program(
      const gl_rendered_blob_params&,
      const cubemap_scene&) noexcept -> oglplus::program_object;
};
//------------------------------------------------------------------------------
eagitexi_cubemap_sky_renderer::eagitexi_cubemap_sky_renderer(
  gl_rendered_source_blob_io& parent,
  const gl_rendered_blob_params& params,
  const cubemap_scene& scene,
  shared_holder<gl_rendered_blob_context> context,
  int size) noexcept
  : eagitexi_cubemap_renderer{
      parent,
      "rendering sky cube-map",
      params,
      context,
      size,
      _tile_size()} {
    _init_program(_build_program(params, scene));
}
//------------------------------------------------------------------------------
eagitexi_cubemap_sky_renderer::~eagitexi_cubemap_sky_renderer() noexcept {
    (void)0; // TODO
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::_tile_size() noexcept -> int {
    return 8;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::_build_program(
  const gl_rendered_blob_params& params,
  const cubemap_scene& scene) noexcept -> oglplus::program_object {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};

    // program
    auto prog{glapi.create_program_object()};
    glapi.add_shader(prog, GL.vertex_shader, embedded<"iCmSkyVS">());
    glapi.add_shader(prog, GL.fragment_shader, embedded<"iCmSkyFS">());
    gl.link_program(prog);
    gl.use_program(prog);

    gl.bind_attrib_location(prog, _screen_position_loc(), "Position");

    glapi.try_set_uniform(prog, "planetRadius", scene.planet_radius_m);
    glapi.try_set_uniform(prog, "atmThickness", scene.atmosphere_thickness_m);
    glapi.try_set_uniform(prog, "aboveGround", scene.above_ground_m);
    glapi.try_set_uniform(prog, "sunX", scene.sun_x);
    glapi.try_set_uniform(prog, "sunY", scene.sun_y);
    glapi.try_set_uniform(prog, "sunZ", scene.sun_z);

    return prog;
}
//------------------------------------------------------------------------------
// I/O
//------------------------------------------------------------------------------
class eagitexi_cubemap_sky_io final : public gl_rendered_source_blob_io {
public:
    eagitexi_cubemap_sky_io(
      main_ctx_parent,
      const shared_provider_objects& shared,
      const gl_rendered_blob_params& params,
      const url& locator,
      int size) noexcept;

protected:
    auto make_renderer(shared_holder<gl_rendered_blob_context>) noexcept
      -> unique_holder<gl_blob_renderer> final;

private:
    void _make_header_bgn(int size) noexcept;
    void _make_header_end(eagitexi_cubemap_sky_renderer&) noexcept;

    const url _locator;
    const int _size;
};
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_io::_make_header_bgn(int size) noexcept {
    std::stringstream hdr;
    hdr << R"({"level":0)";
    hdr << R"(,"width":)" << size;
    hdr << R"(,"height":)" << size;
    hdr << R"(,"depth":)" << 6;
    hdr << R"(,"channels":4)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"rgba")";
    hdr << R"(,"iformat":"rgba8")";
    hdr << R"(,"tag":["sky","cubemap","generated"])";
    hdr << R"(,"data_filter":"zlib")";
    append(hdr.str());
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_io::_make_header_end(
  eagitexi_cubemap_sky_renderer& renderer) noexcept {
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
eagitexi_cubemap_sky_io::eagitexi_cubemap_sky_io(
  main_ctx_parent parent,
  const shared_provider_objects& shared,
  const gl_rendered_blob_params& params,
  const url& locator,
  int size) noexcept
  : gl_rendered_source_blob_io{"ITxSkySky", parent, shared, params, size * size * 6}
  , _locator{locator}
  , _size{size} {
    _make_header_bgn(size);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_io::make_renderer(
  shared_holder<gl_rendered_blob_context> context) noexcept
  -> unique_holder<gl_blob_renderer> {
    unique_holder<eagitexi_cubemap_sky_renderer> renderer{
      default_selector,
      *this,
      params(),
      cubemap_scene(_locator),
      std::move(context),
      _size};

    assert(renderer);
    _make_header_end(*renderer);

    return renderer;
}
//------------------------------------------------------------------------------
// image provider
//------------------------------------------------------------------------------
class eagitexi_cubemap_sky_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagitexi_cubemap_sky_provider(const provider_parameters&) noexcept;

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
      "application.resource_provider.cubemap_sky.device_index",
      -1};
};
//------------------------------------------------------------------------------
eagitexi_cubemap_sky_provider::eagitexi_cubemap_sky_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PTiCubeSky", params.parent}
  , _shared{params.shared} {}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(
      is_valid_eagitexi_resource_url(locator) and
      locator.has_path("cube_map_sky")) {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    if(has_resource(locator)) {
        const auto& q{locator.query()};
        const auto size{q.arg_value_as<int>("size").value_or(1024)};

        gl_rendered_blob_params params{
          .device_index = _device_index.value(),
          .surface_width = size,
          .surface_height = size};

        q.arg_value_as<int>("device_index")
          .and_then(_1.assign_to(params.device_index));

        return {
          hold<eagitexi_cubemap_sky_io>,
          as_parent(),
          _shared,
          params,
          locator,
          size};
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_provider::get_blob_timeout(const span_size_t) noexcept
  -> std::chrono::seconds {
    return adjusted_duration(std::chrono::minutes{10});
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///cube_map_sky");
}
//------------------------------------------------------------------------------
// texture I/O
//------------------------------------------------------------------------------
struct eagitex_cubemap_sky_io final : simple_string_source_blob_io {
    auto make_header(const url&, int size) -> std::string;

    eagitex_cubemap_sky_io(main_ctx_parent parent, const url& locator);
};
//------------------------------------------------------------------------------
eagitex_cubemap_sky_io::eagitex_cubemap_sky_io(
  main_ctx_parent parent,
  const url& locator)
  : simple_string_source_blob_io{
      "ITxCubeSky",
      parent,
      make_header(
        locator,
        locator.query().arg_value_as<int>("size").value_or(1024))} {}
//------------------------------------------------------------------------------
auto eagitex_cubemap_sky_io::make_header(const url& locator, int size)
  -> std::string {
    const auto& q{locator.query()};
    std::stringstream hdr;
    hdr << R"({"levels":1)";
    hdr << R"(,"width":)" << size;
    hdr << R"(,"height":)" << size;
    hdr << R"(,"channels":4)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"rgba")";
    hdr << R"(,"iformat":"rgba8")";
    hdr << R"(,"wrap_s":"clamp_to_edge")";
    hdr << R"(,"wrap_t":"clamp_to_edge")";
    hdr << R"(,"tag":["sky","cubemap","generated"])";
    hdr << R"(,"images":[)";
    hdr << R"({"url":"eagitexi:///cube_map_sky)";
    auto add{[&, first{true}]<typename T>(std::string_view name) mutable {
        if(const auto opt{q.arg_value_as<T>(name)}) {
            if(first) {
                hdr << '?';
                first = false;
            } else {
                hdr << '&';
            }
            hdr << name << '=' << *opt;
        }
    }};
    add.operator()<int>("size");
    add.operator()<float>("planet_radius_m");
    add.operator()<float>("planet_atmosphere_m");
    add.operator()<float>("above_ground_m");
    add.operator()<float>("sun_x");
    add.operator()<float>("sun_y");
    add.operator()<float>("sun_z");
    hdr << R"("}]})";

    return hdr.str();
}
//------------------------------------------------------------------------------
// texture provider
//------------------------------------------------------------------------------
class eagitex_cubemap_sky_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagitex_cubemap_sky_provider(const provider_parameters&) noexcept;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
};
//------------------------------------------------------------------------------
eagitex_cubemap_sky_provider::eagitex_cubemap_sky_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PTxCubeSky", params.parent} {}
//------------------------------------------------------------------------------
auto eagitex_cubemap_sky_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(
      is_valid_eagitex_resource_url(locator) and
      locator.has_path("cube_map_sky")) {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_sky_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    if(has_resource(locator)) {
        return {hold<eagitex_cubemap_sky_io>, as_parent(), locator};
    }
    return {};
}
//------------------------------------------------------------------------------
void eagitex_cubemap_sky_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitex:///cube_map_sky");
}
//------------------------------------------------------------------------------
// provider factory functions
//------------------------------------------------------------------------------
auto provider_eagitexi_cubemap_sky(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_cubemap_sky_provider>, params};
}
//------------------------------------------------------------------------------
auto provider_eagitex_cubemap_sky(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitex_cubemap_sky_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app

