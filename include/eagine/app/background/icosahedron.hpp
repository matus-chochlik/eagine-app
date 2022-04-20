/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_BACKGROUND_ICOSAHEDRON_HPP
#define EAGINE_APP_BACKGROUND_ICOSAHEDRON_HPP

#include <eagine/app/camera.hpp>
#include <eagine/memory/buffer.hpp>
#include <eagine/oglplus/gl_api.hpp>
#include <eagine/oglplus/glsl/string_ref.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/shapes/icosahedron.hpp>
#include <vector>

namespace eagine::app {
//------------------------------------------------------------------------------
class background_icosahedron {
public:
    background_icosahedron(
      video_context& vc,
      const oglplus::vec4 ica,
      const oglplus::vec4 ca,
      const float depth) noexcept;

    auto clean_up(video_context& vc) noexcept -> background_icosahedron&;

    auto clear(video_context& vc, const orbiting_camera& camera) noexcept
      -> background_icosahedron&;

private:
    oglplus::vec4 _icolor;
    float _red;
    float _green;
    float _blue;
    float _alpha;
    float _depth;

    oglplus::owned_program_name _prog;
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _scale_loc;
    oglplus::uniform_location _color_loc;

    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _indices;

    std::vector<oglplus::shape_draw_operation> _ops;
};
//------------------------------------------------------------------------------
inline background_icosahedron::background_icosahedron(
  video_context& vc,
  const oglplus::vec4 ica,
  const oglplus::vec4 ca,
  const float depth) noexcept
  : _icolor{ica}
  , _red{ca.x()}
  , _green{ca.y()}
  , _blue{ca.z()}
  , _alpha{ca.w()}
  , _depth{depth} {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;
    memory::buffer temp;

    gl.clear_color(_red, _green, _blue, _alpha);
    gl.clear_depth(_depth);

    gl.create_program() >> _prog;

    // vertex shader
    const string_view vs_source = R"(
		#version 140
		in vec3 Position;
		uniform mat4 Camera;
		uniform float Scale = 1.0;

		void main() {
			gl_Position = Camera * vec4(Position * Scale, 1.0);
		})";
    oglplus::owned_shader_name vs;
    gl.create_shader(GL.vertex_shader) >> vs;
    const auto cleanup_vs = gl.delete_shader.raii(vs);
    gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
    gl.compile_shader(vs);

    // fragment shader
    const string_view fs_source = R"(
		#version 140
		uniform vec4 Color;
		out vec4 fragColor;

		void main() {
			fragColor = Color;
		})";
    oglplus::owned_shader_name fs;
    gl.create_shader(GL.fragment_shader) >> fs;
    const auto cleanup_fs = gl.delete_shader.raii(fs);
    gl.shader_source(fs, oglplus::glsl_string_ref(fs_source));
    gl.compile_shader(fs);

    gl.attach_shader(_prog, vs);
    gl.attach_shader(_prog, fs);
    gl.link_program(_prog);
    gl.use_program(_prog);

    gl.get_uniform_location(_prog, "Camera") >> _camera_loc;
    gl.get_uniform_location(_prog, "Scale") >> _scale_loc;
    gl.get_uniform_location(_prog, "Color") >> _color_loc;

    oglplus::shape_generator shape{
      glapi, shapes::unit_icosahedron(shapes::vertex_attrib_kind::position)};

    _ops.resize(std_size(shape.operation_count()));
    shape.instructions(glapi, cover(_ops));

    gl.gen_vertex_arrays() >> _vao;
    gl.bind_vertex_array(_vao);

    gl.gen_buffers() >> _positions;
    shape.attrib_setup(
      glapi,
      _vao,
      _positions,
      oglplus::vertex_attrib_location{0},
      eagine::shapes::vertex_attrib_kind::position,
      temp);

    gl.gen_buffers() >> _indices;
    shape.index_setup(glapi, _indices, temp);
}
//------------------------------------------------------------------------------
inline auto background_icosahedron::clean_up(video_context& vc) noexcept
  -> background_icosahedron& {
    const auto& gl = vc.gl_api();

    gl.delete_program(std::move(_prog));
    gl.delete_buffers(std::move(_indices));
    gl.delete_buffers(std::move(_positions));
    gl.delete_vertex_arrays(std::move(_vao));
    return *this;
}
//------------------------------------------------------------------------------
inline auto background_icosahedron::clear(
  video_context& vc,
  const orbiting_camera& camera) noexcept -> background_icosahedron& {
    const auto glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;
    const auto radius{(camera.orbit() + camera.far()) * 1.25F};

    gl.clear(GL.color_buffer_bit);
    gl.use_program(_prog);
    glapi.set_uniform(_prog, _camera_loc, camera.matrix(vc));
    glapi.set_uniform(_prog, _scale_loc, radius);
    glapi.set_uniform(_prog, _color_loc, _icolor);
    gl.bind_vertex_array(_vao);
    gl.polygon_mode(GL.front_and_back, GL.line);
    gl.disable(GL.depth_test);
    gl.disable(GL.cull_face);
    draw_using_instructions(glapi, view(_ops));
    gl.enable(GL.cull_face);
    gl.enable(GL.depth_test);
    gl.polygon_mode(GL.front_and_back, GL.fill);
    gl.clear(GL.depth_buffer_bit);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
