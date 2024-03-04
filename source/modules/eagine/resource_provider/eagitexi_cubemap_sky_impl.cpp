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
class eagitexi_cubemap_sky_renderer final : public gl_blob_renderer {
public:
    eagitexi_cubemap_sky_renderer(
      gl_rendered_source_blob_io& parent,
      const gl_rendered_blob_params& params,
      shared_holder<gl_rendered_blob_context> context,
      int size) noexcept;

    ~eagitexi_cubemap_sky_renderer() noexcept final;

    auto render() noexcept -> msgbus::blob_preparation final;

private:
    static auto _tile_size() noexcept -> int;
    auto _build_screen() noexcept -> oglplus::geometry_and_bindings;
    auto _build_program(const gl_rendered_blob_params&) noexcept
      -> oglplus::program_object;

    void _render_tile() noexcept;
    void _save_tile() noexcept;

    main_ctx_buffer _buffer;

    const oglplus::geometry_and_bindings _screen;
    const oglplus::program_object _prog;
    const int _size;
    const int _tiles_per_side{1};
    int _tile_x{0};
    int _tile_y{0};
    int _face_index{0};
};
//------------------------------------------------------------------------------
eagitexi_cubemap_sky_renderer::eagitexi_cubemap_sky_renderer(
  gl_rendered_source_blob_io& parent,
  const gl_rendered_blob_params& params,
  shared_holder<gl_rendered_blob_context> context,
  int size) noexcept
  : gl_blob_renderer{parent, std::move(context)}
  , _buffer{*this, size * size * 4, nothing}
  , _screen{_build_screen()}
  , _prog{_build_program(params)}
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
    return 32;
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
auto eagitexi_cubemap_sky_renderer::_build_program(
  const gl_rendered_blob_params& params) noexcept -> oglplus::program_object {
    const auto& [gl, GL]{gl_api()};

    // vertex shader
    const string_view vs_source{
      "#version 140\n"
      "in vec2 Position;\n"
      "out vec2 vertCoord;\n"
      "void main() {\n"
      "  gl_Position = vec4(Position, 0.0, 1.0);\n"
      "  vertCoord = Position;\n"
      "}\n"};

    const auto vs{gl.create_shader.object(GL.vertex_shader)};
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
    gl.compile_shader(vs);

    // fragment shader
    const string_view fs_source{
      "#version 140\n"
      "in vec2 vertCoord;\n"
      "out vec4 fragColor;\n"
      "uniform int faceIdx;\n"
      "const mat3[6] cubeFaces = mat3[6](\n"
      "  mat3( 0.0, 0.0,-1.0, 0.0,-1.0, 0.0, 1.0, 0.0, 0.0),\n"
      "  mat3( 0.0, 0.0, 1.0, 0.0,-1.0, 0.0,-1.0, 0.0, 0.0),\n"
      "  mat3( 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0),\n"
      "  mat3( 1.0, 0.0, 0.0, 0.0, 0.0,-1.0, 0.0,-1.0, 0.0),\n"
      "  mat3( 1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0, 1.0),\n"
      "  mat3(-1.0, 0.0, 0.0, 0.0,-1.0, 0.0, 0.0, 0.0,-1.0));\n"
      "void main() {\n"
      "  mat3 cubeFace = cubeFaces[faceIdx];\n"
      "  vec3 cubeCoord =\n"
      "    cubeFace[0]*vertCoord.x+\n"
      "    cubeFace[1]*vertCoord.y+\n"
      "    cubeFace[2];\n"
      "  fragColor = mix(\n"
      "    vec4(0.3, 0.2, 0.1, 0.0),\n"
      "    vec4(0.1, 0.1, 1.0, 0.5),\n"
      "    max(sign(cubeCoord.y), 0.0));\n"
      "}\n"};

    const auto fs{gl.create_shader.object(GL.fragment_shader)};
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_source));
    gl.compile_shader(fs);

    // program
    auto prog{gl.create_program.object()};
    gl.attach_shader(prog, vs);
    gl.attach_shader(prog, fs);
    gl.link_program(prog);
    gl.use_program(prog);

    gl.bind_attrib_location(prog, _screen.position_loc(), "Position");

    return prog;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_sky_renderer::_render_tile() noexcept {
    const auto& [gl, GL]{gl_api()};
    if((_tile_x == 0) and (_tile_y == 0)) {
        gl.disable(GL.scissor_test);
        gl.clear_color(0.5, 0.5, 0.5, 0.0);
        gl.clear(GL.color_buffer_bit);
        gl.get_uniform_location(_prog, "faceIdx").and_then([&](auto loc) {
            gl_api().set_uniform(_prog, loc, _face_index);
        });
    }
    gl.enable(GL.scissor_test);
    gl.scissor(
      _tile_x * _tile_size(),
      _tile_y * _tile_size(),
      _tile_size(),
      _tile_size());
    _screen.draw(gl_api());
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
      int size) noexcept;

protected:
    auto make_renderer(shared_holder<gl_rendered_blob_context>) noexcept
      -> unique_holder<gl_blob_renderer> final;

private:
    void _make_header_bgn(int size) noexcept;
    void _make_header_end(eagitexi_cubemap_sky_renderer&) noexcept;

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
    hdr << R"(,"tag":["sky","cubemap"])";
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
  int size) noexcept
  : gl_rendered_source_blob_io{"ITxSkySky", parent, shared, params, size * size * 6}
  , _size{size} {
    _make_header_bgn(size);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_sky_io::make_renderer(
  shared_holder<gl_rendered_blob_context> context) noexcept
  -> unique_holder<gl_blob_renderer> {
    unique_holder<eagitexi_cubemap_sky_renderer> renderer{
      default_selector, *this, params(), std::move(context), _size};

    assert(renderer);
    _make_header_end(*renderer);

    return renderer;
}
//------------------------------------------------------------------------------
// provider
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
  : main_ctx_object{"PTxCubeSky", params.parent}
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
          hold<eagitexi_cubemap_sky_io>, as_parent(), _shared, params, size};
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
auto provider_eagitexi_cubemap_sky(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_cubemap_sky_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app

