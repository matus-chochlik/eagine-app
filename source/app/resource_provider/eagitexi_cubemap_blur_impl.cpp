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
      shared_provider_objects& shared,
      eglplus::initialized_display display,
      url source,
      int side) noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation final;

private:
    auto _build_screen() noexcept -> oglplus::geometry_and_bindings;
    auto _build_program() noexcept -> oglplus::program_object;

    void _make_header() noexcept;

    main_ctx_buffer _buffer;
    const oglplus::gl_api _glapi;

    const oglplus::geometry_and_bindings _screen;
    const oglplus::program_object _prog;
    url _source;
    const int _side;
};
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::_build_screen() noexcept
  -> oglplus::geometry_and_bindings {
    oglplus::shape_generator shape(
      _glapi, shapes::unit_screen(shapes::vertex_attrib_kind::position));

    return oglplus::geometry_and_bindings{_glapi, shape, _buffer};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::_build_program() noexcept
  -> oglplus::program_object {
    const auto& [gl, GL]{_glapi};

    // vertex shader
    const string_view vs_source{
      "#version 140\n"
      "in vec2 Position;"
      "out vec2 vertCoord;"
      "void main() {"
      "  gl_Position = vec4(Position, 0.0, 1.0);"
      "  vertCoord = gl_Position.xy;"
      "}"};

    const auto vs{gl.create_shader.object(GL.vertex_shader)};
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
    gl.compile_shader(vs);

    // fragment shader
    const string_view fs_source{
      "#version 140\n"
      "in vec2 vertCoord;"
      "out vec3 fragColor;"
      "void main() {"
      "  vec2 csq = pow(vertCoord, 2.0);"
      "  vec3 cubeCoord = vec3(vertCoord, sqrt(3.0-csq.x-csq.y));"
      "  fragColor = normalize(cubeCoord);"
      "}"};

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
void eagitexi_cubemap_blur_io::_make_header() noexcept {
    const auto& [egl, EGL]{eglapi()};

    std::stringstream hdr;
    hdr << R"({"level":0)";
    hdr << R"(,"width":)" << _side;
    hdr << R"(,"height":)" << _side;
    hdr << R"(,"channels":3)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"red")";
    hdr << R"(,"iformat":"rgb8")";
    hdr << R"(,"tag":["blur","cubemap"])";
    hdr << R"(,"metadata":{"renderer":{)";
    if(const auto vendor{egl.query_string(display(), EGL.vendor)}) {
        hdr << R"("vendor":")" << *vendor << R"("})";
        if(const auto version{egl.query_string(display(), EGL.version)}) {
            hdr << R"(,"version":")" << *version << R"("})";
        }
        if(egl.MESA_query_driver(display())) {
            if(const auto driver{egl.get_display_driver_name(display())}) {
                hdr << R"(,"driver":")" << *driver << R"("})";
            }
        }
    }
    hdr << R"(}})";
    hdr << R"(,"data_filter":"zlib")";
    hdr << '}';
    append(hdr.str());
}
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_io::eagitexi_cubemap_blur_io(
  main_ctx_parent parent,
  shared_provider_objects& shared,
  eglplus::initialized_display display,
  url source,
  int side) noexcept
  : gl_rendered_source_blob_io{"ITxCubBlur", parent, shared, std::move(display), side * side * 6}
  , _buffer{*this, 1024, nothing}
  , _screen{_build_screen()}
  , _prog{_build_program()}
  , _source{std::move(source)}
  , _side{side} {
    _make_header();
    finish(); // TODO
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::prepare() noexcept -> msgbus::blob_preparation {
    return msgbus::blob_preparation::finished;
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
        const auto side{q.arg_value_as<int>("side").value_or(1024)};

        gl_rendered_source_params params{
          .surface_width = side, .surface_height = side};

        if(auto display{
             eagitexi_cubemap_blur_io::open_display(_shared, params)}) {
            if(unique_holder<eagitexi_cubemap_blur_io> io{
                 default_selector,
                 as_parent(),
                 _shared,
                 std::move(display),
                 q.arg_url("source"),
                 side}) {
                if(io->create_context(params)) {
                    return io;
                }
            }
        }
    }
    return {};
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

