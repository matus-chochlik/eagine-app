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
      const oglplus::vec4 edge_color,
      const oglplus::vec4 face_color,
      const float depth) noexcept;

    auto clean_up(video_context& vc) noexcept -> background_icosahedron&;

    auto clear(video_context& vc, const orbiting_camera& camera) noexcept
      -> background_icosahedron&;

private:
    oglplus::vec4 _ecolor;
    oglplus::vec4 _fcolor;
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
  const oglplus::vec4 edge_color,
  const oglplus::vec4 face_color,
  const float depth) noexcept
  : _ecolor{edge_color}
  , _fcolor{face_color}
  , _depth{depth} {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;
    memory::buffer temp;

    gl.clear_depth(_depth);

    gl.create_program() >> _prog;

    oglplus::owned_shader_name fs;
    if(gl.create_shader(GL.fragment_shader) >> fs) {
        const auto cleanup_fs = gl.delete_shader.raii(fs);

        const string_view fs_source = R"(
            #version 140
            in  vec4 geomColor;
            out vec4 fragColor;

            void main() {
                fragColor = geomColor;
            })";

        gl.shader_source(fs, oglplus::glsl_string_ref(fs_source));
        gl.compile_shader(fs);
        gl.attach_shader(_prog, fs);
    }

    oglplus::owned_shader_name vs;
    if(gl.create_shader(GL.vertex_shader) >> vs) {
        const auto cleanup_vs = gl.delete_shader.raii(vs);

        oglplus::owned_shader_name gs;
        if(gl.create_shader(GL.geometry_shader) >> gs) {
            const auto cleanup_gs = gl.delete_shader.raii(gs);

            const string_view gs_source = R"(
            #version 330

            layout(triangles) in;
            layout(triangle_strip, max_vertices = 3) out;
            out vec4 geomColor;
            uniform mat4 Camera;
            uniform vec4 Color;

            void main() {
				float m = mod(gl_PrimitiveIDIn / 1.618, 1.0);
				geomColor = vec4(mix(0.96, 1.04, m) * Color.rgb, Color.a);

				for(int i=0; i<3; ++i) {
					gl_Position = Camera * gl_in[i].gl_Position;
					EmitVertex();
				}
            })";
            gl.shader_source(gs, oglplus::glsl_string_ref(gs_source));
            gl.compile_shader(gs);
            gl.attach_shader(_prog, gs);

            const string_view vs_source = R"(
            #version 140
            in vec3 Position;
            uniform float Scale = 1.0;

            void main() {
                gl_Position = vec4(Position * Scale, 1.0);
             })";
            gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
        } else {
            const string_view vs_source = R"(
            #version 140
            in vec3 Position;
            out vec4 geomColor;
            uniform mat4 Camera;
            uniform vec4 Color;
            uniform float Scale = 1.0;

            void main() {
                gl_Position = Camera * vec4(Position * Scale, 1.0);
                geomColor = Color;
            })";
            gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
        }
        gl.compile_shader(vs);
        gl.attach_shader(_prog, vs);
    }

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
    const auto radius{(camera.skybox_distance())};

    gl.clear(GL.color_buffer_bit);
    gl.use_program(_prog);
    glapi.set_uniform(_prog, _camera_loc, camera.matrix(vc));
    glapi.set_uniform(_prog, _scale_loc, radius);
    gl.disable(GL.depth_test);
    gl.disable(GL.cull_face);
    gl.bind_vertex_array(_vao);
    gl.polygon_mode(GL.front_and_back, GL.fill);
    glapi.set_uniform(_prog, _color_loc, _fcolor);
    draw_using_instructions(glapi, view(_ops));
    gl.polygon_mode(GL.front_and_back, GL.line);
    glapi.set_uniform(_prog, _color_loc, _ecolor);
    draw_using_instructions(glapi, view(_ops));
    gl.polygon_mode(GL.front_and_back, GL.fill);
    gl.enable(GL.cull_face);
    gl.enable(GL.depth_test);
    gl.clear(GL.depth_buffer_bit);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
