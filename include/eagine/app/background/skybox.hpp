/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_BACKGROUND_SKYBOX_HPP
#define EAGINE_APP_BACKGROUND_SKYBOX_HPP

#include <eagine/app/camera.hpp>
#include <eagine/memory/buffer.hpp>
#include <eagine/oglplus/gl_api.hpp>
#include <eagine/oglplus/glsl/string_ref.hpp>
#include <eagine/oglplus/math/vector.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/oglplus/utils/image_file.hpp>
#include <eagine/shapes/skybox.hpp>
#include <vector>

namespace eagine::app {
//------------------------------------------------------------------------------
class background_skybox {
public:
    background_skybox(
      video_context&,
      const oglplus::texture_image_block&) noexcept;
    background_skybox(video_context&, memory::const_block) noexcept;

    auto clean_up(video_context& vc) noexcept -> background_skybox&;

    auto clear(video_context& vc, const orbiting_camera& camera) noexcept
      -> background_skybox&;

private:
    oglplus::gl_types::int_type _tex_unit{0};
    oglplus::owned_texture_name _cube_tex;

    oglplus::owned_program_name _prog;
    oglplus::uniform_location _camera_loc;
    oglplus::uniform_location _scale_loc;
    oglplus::uniform_location _tex_loc;

    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _positions;
    oglplus::owned_buffer_name _coords;
    oglplus::owned_buffer_name _indices;

    std::vector<oglplus::shape_draw_operation> _ops;
};
//------------------------------------------------------------------------------
inline background_skybox::background_skybox(
  video_context& vc,
  const oglplus::texture_image_block& cube_img) noexcept {
    const auto& glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;
    memory::buffer temp;

    gl.gen_textures() >> _cube_tex;
    gl.active_texture(vc.gl_api().texture0 + _tex_unit);
    gl.bind_texture(GL.texture_cube_map, _cube_tex);
    gl.tex_parameter_i(GL.texture_cube_map, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_cube_map, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(
      GL.texture_cube_map, GL.texture_wrap_s, GL.clamp_to_edge);
    gl.tex_parameter_i(
      GL.texture_cube_map, GL.texture_wrap_t, GL.clamp_to_edge);
    glapi.spec_tex_image_cube(0, 0, cube_img);

    gl.create_program() >> _prog;

    oglplus::owned_shader_name fs;
    if(gl.create_shader(GL.fragment_shader) >> fs) {
        const auto cleanup_fs = gl.delete_shader.raii(fs);

        const string_view fs_source = R"(
        #version 330
        in  vec3 geomCoord;
        out vec4 fragColor;
        uniform samplerCube Tex;

        void main() {
            fragColor = texture(Tex, geomCoord);
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
            layout(triangle_strip, max_vertices = 48) out;
            in vec3 vertCoord[3];
            out vec3 geomCoord;
            uniform mat4 Camera;
            uniform float Scale = 1.0;

            void emit(vec3 bary) {
                vec3 Position =
                    gl_in[0].gl_Position.xyz * bary.x+
                    gl_in[1].gl_Position.xyz * bary.y+
                    gl_in[2].gl_Position.xyz * bary.z;
                Position = normalize(Position);
                gl_Position = Camera * vec4(Scale * Position, 1.0);
                geomCoord =
                    vertCoord[0] * bary.x+
                    vertCoord[1] * bary.y+
                    vertCoord[2] * bary.z;
                EmitVertex();
            }

            void subdivide0(vec3 a, vec3 b, vec3 c) {
                emit(a);
                emit(b);
                emit(c);
                EndPrimitive();
            }

            void subdivide1(vec3 a, vec3 b, vec3 c) {
                subdivide0(a, (a + b)/2, (c + a)/2);
                subdivide0(b, (b + c)/2, (a + b)/2);
                subdivide0(c, (c + a)/2, (b + c)/2);
                subdivide0((a + b)/2, (b + c)/2, (c + a)/2);
            }

            void subdivide2(vec3 a, vec3 b, vec3 c) {
                subdivide1(a, (a + b)/2, (c + a)/2);
                subdivide1(b, (b + c)/2, (a + b)/2);
                subdivide1(c, (c + a)/2, (b + c)/2);
                subdivide1((a + b)/2, (b + c)/2, (c + a)/2);
            }

            void main() {
                subdivide2(
                    vec3(1.0, 0.0, 0.0), 
                    vec3(0.0, 1.0, 0.0), 
                    vec3(0.0, 0.0, 1.0));
            })";
            gl.shader_source(gs, oglplus::glsl_string_ref(gs_source));
            gl.compile_shader(gs);
            gl.attach_shader(_prog, gs);

            const string_view vs_source = R"(
            #version 330
            layout(location=0) in vec4 Position;
            layout(location=1) in vec3 Coord;
            out vec3 vertCoord;

            void main() {
                gl_Position = Position;
                vertCoord = Coord;
            })";
            gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
        } else {
            const string_view vs_source = R"(
            #version 330
            layout(location=0) in vec3 Position;
            layout(location=1) in vec3 Coord;
            out vec3 geomCoord;
            uniform mat4 Camera;
            uniform float Scale = 1.0;

            void main() {
                gl_Position = Camera * vec4(Position * Scale, 1.0);
                geomCoord = Coord;
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
    gl.get_uniform_location(_prog, "Tex") >> _tex_loc;

    glapi.set_uniform(_prog, _tex_loc, _tex_unit);

    oglplus::shape_generator shape{
      glapi,
      shapes::skybox(
        shapes::vertex_attrib_kind::position |
        shapes::vertex_attrib_kind::face_coord)};

    _ops.resize(integer(shape.operation_count()));
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

    gl.gen_buffers() >> _coords;
    shape.attrib_setup(
      glapi,
      _vao,
      _coords,
      oglplus::vertex_attrib_location{1},
      eagine::shapes::vertex_attrib_kind::face_coord,
      temp);

    gl.gen_buffers() >> _indices;
    shape.index_setup(glapi, _indices, temp);
}
//------------------------------------------------------------------------------
inline background_skybox::background_skybox(
  video_context& vc,
  memory::const_block cube_blk) noexcept
  : background_skybox{vc, oglplus::texture_image_block{cube_blk}} {}
//------------------------------------------------------------------------------
inline auto background_skybox::clean_up(video_context& vc) noexcept
  -> background_skybox& {
    const auto& gl = vc.gl_api();

    gl.delete_textures(std::move(_cube_tex));
    gl.delete_program(std::move(_prog));
    gl.delete_buffers(std::move(_indices));
    gl.delete_buffers(std::move(_coords));
    gl.delete_buffers(std::move(_positions));
    gl.delete_vertex_arrays(std::move(_vao));
    return *this;
}
//------------------------------------------------------------------------------
inline auto background_skybox::clear(
  video_context& vc,
  const orbiting_camera& camera) noexcept -> background_skybox& {
    const auto glapi = vc.gl_api();
    const auto& [gl, GL] = glapi;

    gl.use_program(_prog);
    glapi.set_uniform(_prog, _camera_loc, camera.matrix(vc));
    glapi.set_uniform(_prog, _scale_loc, camera.skybox_distance());
    gl.bind_vertex_array(_vao);
    gl.disable(GL.depth_test);
    gl.disable(GL.cull_face);
    draw_using_instructions(glapi, view(_ops));
    gl.enable(GL.cull_face);
    gl.enable(GL.depth_test);

    gl.clear_depth(1.F);
    gl.clear(GL.depth_buffer_bit | GL.stencil_buffer_bit);
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
