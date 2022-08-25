/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.c_api;
import eagine.shapes;
import eagine.oglplus;

namespace eagine::app {
//------------------------------------------------------------------------------
// background_icosahedron
//------------------------------------------------------------------------------
background_icosahedron::background_icosahedron(
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

    gl.gen_buffers() >> _indices;
    shape.index_setup(glapi, _indices, temp);
}
//------------------------------------------------------------------------------
auto background_icosahedron::clean_up(video_context& vc) noexcept
  -> background_icosahedron& {
    const auto& gl = vc.gl_api();

    gl.delete_program(std::move(_prog));
    gl.delete_buffers(std::move(_indices));
    gl.delete_buffers(std::move(_positions));
    gl.delete_vertex_arrays(std::move(_vao));
    return *this;
}
//------------------------------------------------------------------------------
auto background_icosahedron::clear(
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
// background_skybox
//------------------------------------------------------------------------------
background_skybox::background_skybox(
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
background_skybox::background_skybox(
  video_context& vc,
  memory::const_block cube_blk) noexcept
  : background_skybox{vc, oglplus::texture_image_block{cube_blk}} {}
//------------------------------------------------------------------------------
auto background_skybox::clean_up(video_context& vc) noexcept
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
auto background_skybox::clear(
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

