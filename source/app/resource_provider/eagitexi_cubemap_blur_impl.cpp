/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
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
// I/O
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_io final : public gl_rendered_source_blob_io {
public:
    eagitexi_cubemap_blur_io(
      main_ctx_parent,
      const gl_rendered_source_params& params,
      shared_holder<gl_rendered_source_blob_context>,
      url source,
      int size,
      int sharpness,
      int level) noexcept;

    ~eagitexi_cubemap_blur_io() noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation final;

private:
    static auto _tile_size() noexcept -> int;

    auto _build_screen() noexcept -> oglplus::geometry_and_bindings;
    auto _build_program(const gl_rendered_source_params&, int) noexcept
      -> oglplus::program_object;
    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;
    void _render_tile() noexcept;
    void _save_tile() noexcept;

    void _make_header(int level) noexcept;

    main_ctx_buffer _buffer;

    const oglplus::geometry_and_bindings _screen;
    const oglplus::program_object _prog;
    gl_texture_resource _cubemap;
    const int _size;
    const int _tiles_per_side{1};
    int _tile_x{0};
    int _tile_y{0};
    int _face_index{0};
    bool _finished{false};
};
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::_tile_size() noexcept -> int {
    return 16;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_io::_make_header(int level) noexcept {
    const auto& [egl, EGL]{egl_api()};

    std::stringstream hdr;
    hdr << R"({"level":)" << level;
    hdr << R"(,"width":)" << _size;
    hdr << R"(,"height":)" << _size;
    hdr << R"(,"depth":)" << 6;
    hdr << R"(,"channels":4)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"rgba")";
    hdr << R"(,"iformat":"rgba8")";
    hdr << R"(,"tag":["blur","cubemap"])";
    hdr << R"(,"metadata":{"renderer":{)";
    if(const auto vendor{egl.query_string(display(), EGL.vendor)}) {
        hdr << R"("vendor":")" << *vendor << R"(")";
        if(const auto version{egl.query_string(display(), EGL.version)}) {
            hdr << R"(,"version":")" << *version << R"(")";
        }
        if(egl.MESA_query_driver(display())) {
            if(const auto driver{egl.get_display_driver_name(display())}) {
                hdr << R"(,"driver":")" << *driver << R"(")";
            }
        }
    }
    hdr << R"(}})";
    hdr << R"(,"data_filter":"zlib")";
    hdr << '}';
    append(hdr.str());
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::_build_screen() noexcept
  -> oglplus::geometry_and_bindings {
    oglplus::shape_generator shape(
      gl_api(), shapes::unit_screen(shapes::vertex_attrib_kind::position));

    oglplus::geometry_and_bindings screen{gl_api(), shape, _buffer};
    screen.use(gl_api());

    return screen;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::_build_program(
  const gl_rendered_source_params& params,
  int sharpness) noexcept -> oglplus::program_object {
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
      "uniform samplerCube cubeMap;\n"
      "uniform int faceIdx;\n"
      "uniform int cubeSide;\n"
      "uniform int sharpness;\n"
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
      "  float ics = 1.0 / float(cubeSide-1);\n"
      "  vec4 accumColor = vec4(0.0);\n"
      "  float accumWeight = 0.0;\n"
      "  for(int f = 0; f < 6; ++f) {\n"
      "    mat3 sampleFace = cubeFaces[f];\n"
      "    for(int y = 0; y < cubeSide; ++y) {\n"
      "      for(int x = 0; x < cubeSide; ++x) {\n"
      "        vec2 sampleCoord = vec2(\n"
      "          mix(-1.0, 1.0, float(x)*ics),\n"
      "          mix(-1.0, 1.0, float(y)*ics));\n"
      "        vec3 sampleCubeCoord =\n"
      "          sampleFace[0]*sampleCoord.x+\n"
      "          sampleFace[1]*sampleCoord.y+\n"
      "          sampleFace[2];\n"
      "        vec4 sampleColor = texture(cubeMap, sampleCubeCoord);\n"
      "        float sampleWeight = pow(max(dot(\n"
      "            normalize(cubeCoord),\n"
      "            normalize(sampleCubeCoord)),\n"
      "          0.0), pow(2.0, float(sharpness)));\n"
      "        accumColor = accumColor + sampleColor * sampleWeight;\n"
      "        accumWeight += sampleWeight;\n"
      "      }\n"
      "    }\n"
      "  }\n"
      "  fragColor = accumColor / accumWeight;\n"
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
    gl.get_uniform_location(prog, "cubeSide").and_then([&, this](auto loc) {
        const auto side{std::min(
          std::max(params.surface_width.value(), params.surface_height.value()),
          128)};
        gl_api().set_uniform(prog, loc, side);
    });
    gl.get_uniform_location(prog, "sharpness").and_then([&, this](auto loc) {
        gl_api().set_uniform(prog, loc, sharpness);
    });
    gl.get_uniform_location(prog, "cubeMap").and_then([&, this](auto loc) {
        gl_api().set_uniform(prog, loc, 0);
    });

    return prog;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_io::_on_tex_loaded(
  const gl_texture_resource::load_info& loaded) noexcept {
    const auto& GL{gl_api().constants()};
    loaded.parameter_i(GL.texture_min_filter, GL.linear);
    loaded.parameter_i(GL.texture_mag_filter, GL.linear);
    loaded.parameter_i(GL.texture_wrap_s, GL.clamp_to_edge);
    loaded.parameter_i(GL.texture_wrap_t, GL.clamp_to_edge);
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_io::_render_tile() noexcept {
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
void eagitexi_cubemap_blur_io::_save_tile() noexcept {
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
auto eagitexi_cubemap_blur_io::prepare() noexcept -> msgbus::blob_preparation {
    if(not _finished) {
        make_current();
    }

    const auto& GL = gl_api().constants();
    if(_cubemap.load_if_needed(
         resource_context(), GL.texture_cube_map, GL.texture0)) {
        return msgbus::blob_preparation::working;
    }
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
    if(not _finished) {
        finish();
        _finished = true;
        return msgbus::blob_preparation::working;
    }
    return msgbus::blob_preparation::finished;
}
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_io::eagitexi_cubemap_blur_io(
  main_ctx_parent parent,
  const gl_rendered_source_params& params,
  shared_holder<gl_rendered_source_blob_context> context,
  url source,
  int size,
  int sharpness,
  int level) noexcept
  : gl_rendered_source_blob_io{"ITxCubBlur", parent, std::move(context), size * size * 6}
  , _buffer{*this, size * size * 4, nothing}
  , _screen{_build_screen()}
  , _prog{_build_program(params, sharpness)}
  , _cubemap{std::move(source), resource_context()}
  , _size{size}
  , _tiles_per_side{std::max(size / _tile_size(), 1)} {
    _cubemap.loaded.connect(
      make_callable_ref<&eagitexi_cubemap_blur_io::_on_tex_loaded>(this));

    const auto& [gl, GL]{gl_api()};
    gl.viewport(0, 0, _size, _size);
    gl.disable(GL.depth_test);

    _make_header(level);
}
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_io::~eagitexi_cubemap_blur_io() noexcept {
    _cubemap.clean_up(resource_context());
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
    shared_provider_objects& _shared;
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

        gl_rendered_source_params params{
          .surface_width = size, .surface_height = size};

        q.arg_value_as<int>("device_index")
          .and_then(_1.assign_to(params.device_index));

        if(auto context{eagitexi_cubemap_blur_io::create_context(
             as_parent(), _shared, params)}) {
            return {
              hold<eagitexi_cubemap_blur_io>,
              as_parent(),
              params,
              std::move(context),
              q.arg_url("source"),
              size,
              sharpness,
              level};
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::get_blob_timeout(const span_size_t) noexcept
  -> std::chrono::seconds {
    return std::chrono::minutes{10};
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

