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
// planet parameters
//------------------------------------------------------------------------------
struct cubemap_scene {
    float planet_radius_m{6'370'000.F};
    float atmosphere_thickness_m{100'000.F};
    float vapor_thickness_ratio{0.03F};
    float cloud_altitude_m{2'500.F};
    float cloud_thickness_m{1'200.F};
    float cloudiness_factor{0.5F};
    float above_ground_m{25.F};
    float sun_azimuth_deg{0.F};
    float sun_elevation_deg{45.F};
    float sun_apparent_angle{0.07F};

    std::string tiling_url;

    template <typename T>
    static auto query_arg(const url&, string_view name, T fallback) noexcept
      -> T;

    cubemap_scene(const url& locator) noexcept;

    auto sun_coord() const noexcept
      -> math::unit_spherical_coordinate<float, true>;

    auto sun_xyz() const noexcept -> math::vector<float, 3, true>;
};
//------------------------------------------------------------------------------
template <identifier_t Id>
constexpr auto data_member_mapping(
  const std::type_identity<cubemap_scene>,
  const selector<Id>) noexcept {
    return make_data_member_mapping<
      cubemap_scene,
      float,
      float,
      float,
      float,
      float,
      float,
      float,
      float,
      float,
      float,
      std::string>(
      {"planet_radius_m", &cubemap_scene::planet_radius_m},
      {"atmosphere_thickness_m", &cubemap_scene::atmosphere_thickness_m},
      {"vapor_thickness_ratio", &cubemap_scene::vapor_thickness_ratio},
      {"cloud_altitude_m", &cubemap_scene::cloud_altitude_m},
      {"cloud_thickness_m", &cubemap_scene::cloud_thickness_m},
      {"cloudiness_factor", &cubemap_scene::cloudiness_factor},
      {"above_ground_m", &cubemap_scene::above_ground_m},
      {"sun_azimuth_deg", &cubemap_scene::sun_azimuth_deg},
      {"sun_elevation_deg", &cubemap_scene::sun_elevation_deg},
      {"sun_apparent_angle", &cubemap_scene::sun_apparent_angle},
      {"tiling_url", &cubemap_scene::tiling_url});
}
//------------------------------------------------------------------------------
cubemap_scene::cubemap_scene(const url& l) noexcept
  : planet_radius_m{query_arg<float>(l, "planet_radius_m", 6'370'000.F)}
  , atmosphere_thickness_m{query_arg<float>(l, "atm_thickness_m", 100'000.F)}
  , vapor_thickness_ratio{query_arg<float>(l, "vapor_thickness_ratio", 0.03F)}
  , cloud_altitude_m{query_arg<float>(l, "cloud_altitude_m", 2'500.F)}
  , cloud_thickness_m{query_arg<float>(l, "cloud_thickness_m", 1'200.F)}
  , cloudiness_factor{query_arg<float>(l, "cloudiness_factor", 0.5F)}
  , above_ground_m{query_arg<float>(l, "above_ground_m", 100.F)}
  , sun_azimuth_deg{query_arg<float>(l, "sun_azimuth_deg", 0.0F)}
  , sun_elevation_deg{query_arg<float>(l, "sun_elevation_deg", 45.0F)}
  , sun_apparent_angle{query_arg<float>(l, "sun_apparent_angle", 0.07F)} {}
//------------------------------------------------------------------------------
template <typename T>
auto cubemap_scene::query_arg(
  const url& locator,
  string_view name,
  T fallback) noexcept -> T {
    return locator.query().arg_value_as<T>(name).value_or(fallback);
}
//------------------------------------------------------------------------------
auto cubemap_scene::sun_coord() const noexcept
  -> math::unit_spherical_coordinate<float, true> {
    return {degrees_(sun_azimuth_deg), degrees_(sun_elevation_deg)};
}
//------------------------------------------------------------------------------
auto cubemap_scene::sun_xyz() const noexcept -> math::vector<float, 3, true> {
    return math::to_cartesian(sun_coord());
}
//------------------------------------------------------------------------------
// Renderer
//------------------------------------------------------------------------------
class eagitexi_cubemap_sky_renderer_base : public eagitexi_cubemap_renderer {
protected:
    eagitexi_cubemap_sky_renderer_base(
      gl_rendered_source_blob_io& parent,
      const shared_provider_objects& shared,
      const gl_rendered_blob_params& params,
      shared_holder<gl_rendered_blob_context> context,
      int size) noexcept
      : eagitexi_cubemap_renderer{
          parent,
          "rendering sky cube-map",
          params,
          context,
          size,
          _tile_size(parent, size)} {}

private:
    static auto _tile_size(gl_rendered_source_blob_io&, int) noexcept -> int;
};
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer_base::_tile_size(
  gl_rendered_source_blob_io& p,
  int size) noexcept -> int {
    if(const auto size{p.app_config().get<int>(
         "application.resource_provider.cubemap_sky.tile_size")}) {
        return *size;
    }
    if(size <= 256) {
        return 8;
    }
    if(size <= 1024) {
        return 4;
    }
    if(size <= 2048) {
        return 2;
    }
    return 1;
}
//------------------------------------------------------------------------------
class eagitexi_cubemap_sky_renderer final
  : public eagitexi_cubemap_sky_renderer_base {
public:
    eagitexi_cubemap_sky_renderer(
      gl_rendered_source_blob_io& parent,
      const shared_provider_objects& shared,
      const gl_rendered_blob_params& params,
      const cubemap_scene& scene,
      shared_holder<gl_rendered_blob_context> context,
      int size) noexcept;

    ~eagitexi_cubemap_sky_renderer() noexcept final;

    auto prepare_render() noexcept -> msgbus::blob_preparation_result final;

private:
    void _process_cell(byte);
    void _process_line(const string_view);

    void _line_loaded(resource_loader::string_list_load_info& info) noexcept;
    void _loaded(const loaded_resource_base& info) noexcept;

    auto _build_program(
      const gl_rendered_blob_params&,
      const cubemap_scene&) noexcept -> oglplus::program_object;
    void _setup_tex_storage() noexcept;
    void _add_tex_image_row() noexcept;
    void _set_tex_parameters() noexcept;

    const shared_provider_objects& _shared;
    oglplus::gl_types::sizei_type _tiling_side{0};
    oglplus::gl_types::int_type _tiling_yofs{0};
    oglplus::texture_object _tiling_tex;
    main_ctx_buffer _tiling_line;
    string_list_resource _tiling;
    const signal_binding _line_binding{
      _shared.loader.string_line_loaded
        .bind_to<&eagitexi_cubemap_sky_renderer::_line_loaded>(this)};
    const signal_binding _done_binding{
      _tiling.load_event.bind_to<&eagitexi_cubemap_sky_renderer::_loaded>(
        this)};
    msgbus::blob_preparation_context _prep_status;
};
//------------------------------------------------------------------------------
eagitexi_cubemap_sky_renderer::eagitexi_cubemap_sky_renderer(
  gl_rendered_source_blob_io& parent,
  const shared_provider_objects& shared,
  const gl_rendered_blob_params& params,
  const cubemap_scene& scene,
  shared_holder<gl_rendered_blob_context> context,
  int size) noexcept
  : eagitexi_cubemap_sky_renderer_base{parent, shared, params, context, size}
  , _tiling_tex{gl_api().create_texture_object(gl_api().texture_2d)}
  , _shared{shared}
  , _tiling_line{*this, 1024}
  , _tiling{url{scene.tiling_url}, shared.loader} {
    _tiling_line.clear();
    _init_program(_build_program(params, scene));
}
//------------------------------------------------------------------------------
eagitexi_cubemap_sky_renderer::~eagitexi_cubemap_sky_renderer() noexcept {
    _tiling.clean_up(_shared.loader);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::prepare_render() noexcept
  -> msgbus::blob_preparation_result {
    loaded_resource_context context{_shared.loader};
    return _prep_status(_tiling.load_if_needed(context));
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
    // TODO: remove this
    std::array<char, 1024> temp;
    if(auto info{gl.get_program_info_log(prog, cover(temp))}) {
        std::cerr << *info << std::endl;
    }

    gl.bind_attrib_location(prog, _screen_position_loc(), "Position");

    glapi.try_set_uniform(prog, "planetRadius", scene.planet_radius_m);
    glapi.try_set_uniform(prog, "atmThickness", scene.atmosphere_thickness_m);
    glapi.try_set_uniform(prog, "vaporThickness", scene.vapor_thickness_ratio);
    glapi.try_set_uniform(prog, "cloudAltitude", scene.cloud_altitude_m);
    glapi.try_set_uniform(prog, "cloudThickness", scene.cloud_thickness_m);
    glapi.try_set_uniform(prog, "cloudiness", scene.cloudiness_factor);
    glapi.try_set_uniform(prog, "aboveGround", scene.above_ground_m);
    glapi.try_set_uniform(prog, "sunApparentAngle", scene.sun_apparent_angle);
    glapi.try_set_uniform(prog, "sunDirection", scene.sun_xyz());

    return prog;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_setup_tex_storage() noexcept {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_2d, _tiling_tex);
    if(gl.texture_storage2d) {
        gl.texture_storage2d(_tiling_tex, 1, GL.r8, _tiling_side, _tiling_side);
    } else if(glapi.tex_storage2d) {
        gl.tex_storage2d(GL.texture_2d, 1, GL.r8, _tiling_side, _tiling_side);
    } else if(glapi.tex_image2d) {
        gl.tex_image2d(
          GL.texture_2d,
          0,
          GL.r8,
          _tiling_side,
          _tiling_side,
          0, // border
          GL.red,
          GL.unsigned_byte_,
          {});
    }
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_add_tex_image_row() noexcept {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};
    if(gl.texture_sub_image2d) {
        gl.texture_sub_image2d(
          _tiling_tex,
          0,
          0,
          _tiling_yofs,
          _tiling_side,
          1,
          GL.red,
          GL.unsigned_byte_,
          view(_tiling_line));
    } else if(gl.tex_sub_image2d) {
        gl.active_texture(GL.texture0);
        gl.bind_texture(GL.texture_2d, _tiling_tex);
        gl.tex_sub_image2d(
          GL.texture_2d,
          0,
          0,
          _tiling_yofs,
          _tiling_side,
          1,
          GL.red,
          GL.unsigned_byte_,
          view(_tiling_line));
    }
    _tiling_yofs += 1;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_set_tex_parameters() noexcept {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};
    if(gl.texture_parameter_i) {
        gl.texture_parameter_i(_tiling_tex, GL.texture_wrap_s, GL.repeat);
        gl.texture_parameter_i(_tiling_tex, GL.texture_wrap_t, GL.repeat);
        gl.texture_parameter_i(_tiling_tex, GL.texture_min_filter, GL.linear);
        gl.texture_parameter_i(_tiling_tex, GL.texture_mag_filter, GL.linear);
    } else if(gl.tex_parameter_i) {
        gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.repeat);
        gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.repeat);
        gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.linear);
        gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    }
    glapi.try_set_uniform(prog(), "tilingSide", float(_tiling_side));
    glapi.try_set_uniform(prog(), "tilingTex", GL.texture0);
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_process_cell(byte b) {
    b = b << 4U | b;
    append_to(view_one(b), _tiling_line);
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_process_line(const string_view line) {
    for(const char c : line) {
        _process_cell(hex_char2byte(c).value_or(byte{}));
    }
    if(not _tiling_side) {
        if(assign_if_fits(_tiling_line.size(), _tiling_side)) {
            _setup_tex_storage();
        }
    }
    if(_tiling_side and (_tiling_side == _tiling_line.size())) {
        if(_tiling_yofs < _tiling_side) {
            _add_tex_image_row();
            _tiling_line.clear();
        }
    }
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_line_loaded(
  resource_loader::string_list_load_info& info) noexcept {
    if(_tiling.originated(info)) {
        for(const auto& line : info.strings) {
            if(not line.empty()) {
                if(math::is_positive_power_of_2(line.size())) {
                    _process_line(line);
                }
            }
        }
        info.strings.clear();
    }
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_loaded(
  const loaded_resource_base&) noexcept {
    _set_tex_parameters();
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
    auto load_resources() noexcept -> msgbus::blob_preparation_result final;
    auto make_renderer(shared_holder<gl_rendered_blob_context>) noexcept
      -> unique_holder<gl_blob_renderer> final;

private:
    void _make_header_bgn(int size) noexcept;
    void _make_header_end(eagitexi_cubemap_sky_renderer&) noexcept;

    const shared_provider_objects& _shared;
    const url _locator;
    const int _size;
    cubemap_scene _scene{_locator};
    std::optional<resource_request_result> _scene_request{};
    resource_load_status _scene_load_status{resource_load_status::not_found};
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
  , _shared{shared}
  , _locator{locator}
  , _size{size} {
    if(const auto params_url{_locator.query().arg_url("params")}) {
        _scene_request.emplace(shared.loader.request_value_tree_traversal(
          params_url, make_mapped_struct_loader(_scene, _scene_load_status)));
    } else {
        _scene_load_status = resource_load_status::loaded;
    }
    _make_header_bgn(size);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_io::load_resources() noexcept
  -> msgbus::blob_preparation_result {
    if(_scene_request) {
        if(not _scene_request->info().is_done()) {
            return {msgbus::blob_preparation_status::working};
        }
        _scene_request.reset();
    }
    return msgbus::blob_preparation_result::finished();
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_io::make_renderer(
  shared_holder<gl_rendered_blob_context> context) noexcept
  -> unique_holder<gl_blob_renderer> {
    unique_holder<eagitexi_cubemap_sky_renderer> renderer{
      default_selector,
      *this,
      _shared,
      params(),
      _scene,
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
    return adjusted_duration(std::chrono::hours{24});
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
    add.operator()<float>("sun_azimuth_deg");
    add.operator()<float>("sun_elevation_deg");
    add.operator()<std::string>("params");
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

