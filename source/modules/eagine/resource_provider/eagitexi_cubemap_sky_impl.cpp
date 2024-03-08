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
class eagitexi_cubemap_sky_renderer final : public gl_blob_renderer {
public:
    eagitexi_cubemap_sky_renderer(
      gl_rendered_source_blob_io& parent,
      const gl_rendered_blob_params& params,
      const cubemap_scene& scene,
      shared_holder<gl_rendered_blob_context> context,
      int size) noexcept;

    ~eagitexi_cubemap_sky_renderer() noexcept final;

    auto render() noexcept -> msgbus::blob_preparation final;

private:
    static auto _tile_size() noexcept -> int;
    static auto _vs_source() noexcept -> string_view;
    static auto _fs_source() noexcept -> string_view;
    auto _build_screen() noexcept -> oglplus::geometry_and_bindings;
    auto _build_program(
      const gl_rendered_blob_params&,
      const cubemap_scene&) noexcept -> oglplus::program_object;

    void _render_tile() noexcept;
    void _save_tile() noexcept;

    auto _done_tiles() const noexcept -> span_size_t;
    auto _total_tiles() const noexcept -> span_size_t;

    main_ctx_buffer _buffer;

    const oglplus::geometry_and_bindings _screen;
    const oglplus::program_object _prog;
    const int _size;
    const int _tiles_per_side{1};
    int _tile_x{0};
    int _tile_y{0};
    int _face_index{0};

    activity_progress _prepare_progress{
      main_context().progress(),
      "rendering sky cube-map",
      _total_tiles()};
};
//------------------------------------------------------------------------------
eagitexi_cubemap_sky_renderer::eagitexi_cubemap_sky_renderer(
  gl_rendered_source_blob_io& parent,
  const gl_rendered_blob_params& params,
  const cubemap_scene& scene,
  shared_holder<gl_rendered_blob_context> context,
  int size) noexcept
  : gl_blob_renderer{parent, std::move(context)}
  , _buffer{*this, size * size * 4, nothing}
  , _screen{_build_screen()}
  , _prog{_build_program(params, scene)}
  , _size{size}
  , _tiles_per_side{std::max(_size / _tile_size(), 1)} {
    const auto& [gl, GL]{gl_api()};
    gl.viewport(0, 0, _size, _size);
    gl.disable(GL.depth_test);
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
auto eagitexi_cubemap_sky_renderer::_build_screen() noexcept
  -> oglplus::geometry_and_bindings {
    oglplus::geometry_and_bindings screen{
      gl_api(),
      oglplus::shape_generator{
        gl_api(), shapes::unit_screen(shapes::vertex_attrib_kind::position)},
      _buffer};
    screen.use(gl_api());

    return screen;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::_vs_source() noexcept -> string_view {
    return R"(
#version 140
in vec2 Position;
out vec2 vertCoord;
void main() {
	gl_Position = vec4(Position, 0.0, 1.0);
	vertCoord = Position;
};
    )";
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::_fs_source() noexcept -> string_view {
    return R"(
/* -------------------------------------------------------------------------- */
#version 140
in vec2 vertCoord;
out vec4 fragColor;
uniform int faceIdx;
uniform float planetRadius;
uniform float atmThickness;
uniform float aboveGround;
uniform float sunX;
uniform float sunY;
uniform float sunZ;
vec3 sunDirection = normalize(vec3(sunX, sunY, sunZ));
const float sunDot = 0.003;

struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Sphere {
	vec3 center;
	float radius;
};

Ray raySample(Ray ray, float rayDist) {
	return Ray(ray.origin + ray.direction * rayDist, ray.direction);
}

bool isValidHit(float param) {
	return param >= 0.0;
}

float sphereHit(Ray ray, Sphere sphere) {
    float rad2 = pow(sphere.radius, 2.0);
    vec3 l = sphere.center - ray.origin;
    float tca = dot(l, normalize(ray.direction));

    float d2 = dot(l, l) - pow(tca, 2.0);
    if(d2 > rad2) {
		return -1.0;
	}
    float thc = sqrt(rad2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 > 0.0) {
        return t0 / length(ray.direction);
    } 

    if (t1 > 0.0) {
        return t1 / length(ray.direction);
    }

    return -1.0;
}

Ray getViewRay() {
	mat3 cubeFace = mat3[6](
		mat3( 0.0, 0.0,-1.0, 0.0,-1.0, 0.0, 1.0, 0.0, 0.0),
		mat3( 0.0, 0.0, 1.0, 0.0,-1.0, 0.0,-1.0, 0.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0, 0.0,-1.0, 0.0,-1.0, 0.0),
		mat3( 1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0, 1.0),
		mat3(-1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0,-1.0))[faceIdx];
	return Ray(vec3(0.0), normalize(
		cubeFace[0]*vertCoord.x + cubeFace[1]*vertCoord.y + cubeFace[2]));
}

float vaporDensity(vec3 sample, Sphere planet, Sphere atmosphere) {
	Sphere clouds[2] = Sphere[2](
		Sphere(vec3(2100.0, 3100.0, 200.0), 1300.0),
		Sphere(vec3(1500.0, 3000.0, 200.0), 1200.0));
	float result = 0.0;
	for(int c=0; c<2; ++c) {
		float dist = distance(clouds[c].center, sample);
		float density = clamp(1.0 - dist / clouds[c].radius, 0.0, 1.0) * 0.001;
		result = max(result, density);
	}
	return result;
}

float airDensity(vec3 sample, Sphere planet, Sphere atmosphere) {
	float ds = distance(sample, atmosphere.center);
	float fact = (ds - planet.radius) / (atmosphere.radius - planet.radius);
	return pow(clamp(1.0 - fact, 0.0, 1.0), 1.25) * 0.000001;
}

float sunHit(Ray ray) {
	float d = dot(ray.direction, sunDirection);
	return max(sign(d + sunDot - 1.0), 0.0);
}

vec4 sunColor() {
	return vec4(1.0);
}

vec4 bgColor(Ray ray, float rayDist) {
	return mix(vec4(0.0), sunColor(), sunHit(ray));
}

vec4 skyColor(Ray viewRay, Sphere planet, Sphere atmosphere) {
	float atmHit = max(sphereHit(viewRay, atmosphere), 0.0);
	float dt = 0.5 + 0.5*dot(viewRay.direction, sunDirection);
	vec4 result = mix(
		vec4(0.25, 0.35, 0.50, 0.5),
		vec4(1.00, 0.90, 0.75, 1.0),
		pow(dt, 16.0));
	return result;
}

vec4 surfaceColor(Ray viewRay, float rayDist, Sphere planet) {
	float f = exp(-rayDist/500.0);
	return mix(vec4(0.23, 0.20, 0.17, 0.0), vec4(0.4, 0.3, 0.2, 0.0), f);
}

Sphere getPlanet() {
	return Sphere(
		vec3(0.0, -planetRadius-aboveGround, 0.0),
		planetRadius);
}

Sphere getAtmosphere() {
	return Sphere(
		vec3(0.0, -planetRadius-aboveGround, 0.0),
		planetRadius+atmThickness);
}

void main() {
	Ray viewRay = getViewRay();
	Sphere planet = getPlanet();
	float planetHit = sphereHit(viewRay, planet);
	if(isValidHit(planetHit)) {
		fragColor = surfaceColor(viewRay, planetHit, planet);
	} else {
		fragColor = skyColor(viewRay, planet, getAtmosphere());
	}
}
/* -------------------------------------------------------------------------- */
	)";
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::_build_program(
  const gl_rendered_blob_params& params,
  const cubemap_scene& scene) noexcept -> oglplus::program_object {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};

    // vertex shader
    const auto vs{gl.create_shader.object(GL.vertex_shader)};
    gl.shader_source(vs, oglplus::glsl_string_ref(_vs_source()));
    gl.compile_shader(vs);

    // fragment shader
    const auto fs{gl.create_shader.object(GL.fragment_shader)};
    gl.shader_source(fs, oglplus::glsl_string_ref(_fs_source()));
    gl.compile_shader(fs);
    gl_api().shader_info_log(fs).and_then(
      [](const auto& s) { std::cout << s << std::endl; });

    // program
    auto prog{gl.create_program.object()};
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    gl.bind_attrib_location(prog, _screen.position_loc(), "Position");

    glapi.try_set_uniform(prog, "planetRadius", scene.planet_radius_m);
    glapi.try_set_uniform(prog, "atmThickness", scene.atmosphere_thickness_m);
    glapi.try_set_uniform(prog, "aboveGround", scene.above_ground_m);
    glapi.try_set_uniform(prog, "sunX", scene.sun_x);
    glapi.try_set_uniform(prog, "sunY", scene.sun_y);
    glapi.try_set_uniform(prog, "sunZ", scene.sun_z);

    return prog;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_render_tile() noexcept {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};
    if((_tile_x == 0) and (_tile_y == 0)) {
        gl.disable(GL.scissor_test);
        gl.clear_color(0.5, 0.5, 0.5, 0.0);
        gl.clear(GL.color_buffer_bit);
        glapi.try_set_uniform(_prog, "faceIdx", _face_index);
    }
    gl.enable(GL.scissor_test);
    gl.scissor(
      _tile_x * _tile_size(),
      _tile_y * _tile_size(),
      _tile_size(),
      _tile_size());
    _screen.draw(glapi);
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_save_tile() noexcept {
    const auto& [gl, GL]{gl_api()};
    _buffer.resize(span_size(_size * _size * 4));

    gl.disable(GL.scissor_test);
    gl.finish();
    gl.read_pixels(
      0,
      0,
      oglplus::gl_types::sizei_type(_size),
      oglplus::gl_types::sizei_type(_size),
      GL.rgba,
      GL.unsigned_byte_,
      cover(_buffer));

    compress(view(_buffer));
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::_done_tiles() const noexcept
  -> span_size_t {
    return (_face_index * _tiles_per_side * _tiles_per_side) +
           (_tile_y * _tiles_per_side) + _tile_x;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::_total_tiles() const noexcept
  -> span_size_t {
    return 6 * _tiles_per_side * _tiles_per_side;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_renderer::render() noexcept
  -> msgbus::blob_preparation {
    if(_face_index < 6) {
        _render_tile();
        if(++_tile_x >= _tiles_per_side) {
            _tile_x = 0;
            if(++_tile_y >= _tiles_per_side) {
                _tile_y = 0;
                _save_tile();
                ++_face_index;
            }
        }
        if(_face_index < 6) {
            _prepare_progress.update_progress(_done_tiles());
        } else {
            _prepare_progress.finish();
        }
        return msgbus::blob_preparation::working;
    }
    return msgbus::blob_preparation::finished;
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

