/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.c_api;
import eagine.shapes;
import eagine.oglplus;

namespace eagine::app {
//------------------------------------------------------------------------------
// background_icosahedron
//------------------------------------------------------------------------------
void background_icosahedron::_init(auto& gl, auto& GL, auto& api) noexcept {
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
			uniform vec3 Offset = vec3(0.0);
			uniform float Scale = 1.0;

			void main() {
				gl_Position = vec4(Position * Scale + Offset, 1.0);
			})";
            gl.shader_source(vs, oglplus::glsl_string_ref(vs_source));
        } else {
            const string_view vs_source = R"(
			#version 140
			in vec3 Position;
			out vec4 geomColor;
			uniform mat4 Camera;
			uniform vec4 Color;
			uniform vec3 Offset = vec3(0.0);
			uniform float Scale = 1.0;

			void main() {
				gl_Position = Camera * vec4(Position * Scale + Offset, 1.0);
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
    gl.get_uniform_location(_prog, "Offset") >> _offset_loc;
    gl.get_uniform_location(_prog, "Scale") >> _scale_loc;
    gl.get_uniform_location(_prog, "Color") >> _color_loc;

    oglplus::shape_generator shape{
      api, shapes::unit_icosahedron(shapes::vertex_attrib_kind::position)};

    _ops.resize(integer(shape.operation_count()));
    shape.instructions(api, cover(_ops));

    gl.gen_vertex_arrays() >> _vao;
    gl.bind_vertex_array(_vao);

    gl.gen_buffers() >> _positions;
    shape.attrib_setup(
      api,
      _vao,
      _positions,
      oglplus::vertex_attrib_location{0},
      eagine::shapes::vertex_attrib_kind::position,
      temp);

    gl.gen_buffers() >> _indices;
    shape.index_setup(api, _indices, temp);
}
//------------------------------------------------------------------------------
background_icosahedron::background_icosahedron(
  video_context& video,
  const oglplus::vec4 edge_color,
  const oglplus::vec4 face_color,
  const float depth) noexcept
  : _ecolor{edge_color}
  , _fcolor{face_color}
  , _depth{depth} {
    video.with_gl(
      [this](auto& gl, auto& GL, auto& api) { _init(gl, GL, api); });
}
//------------------------------------------------------------------------------
void background_icosahedron::_clean_up(auto& gl) noexcept {
    gl.delete_program(std::move(_prog));
    gl.delete_buffers(std::move(_indices));
    gl.delete_buffers(std::move(_positions));
    gl.delete_vertex_arrays(std::move(_vao));
}
//------------------------------------------------------------------------------
auto background_icosahedron::clean_up(video_context& video) noexcept
  -> background_icosahedron& {
    video.with_gl([this](auto& gl) { _clean_up(gl); });
    return *this;
}
//------------------------------------------------------------------------------
auto background_icosahedron::clear(
  video_context& video,
  const mat4& cam_mat,
  const vec3& offset,
  const float radius) noexcept -> background_icosahedron& {
    video.with_gl([&, this](auto& gl, auto& GL, auto& api) {
        gl.clear(GL.color_buffer_bit);
        gl.use_program(_prog);
        api.set_uniform(_prog, _camera_loc, cam_mat);
        api.set_uniform(_prog, _offset_loc, offset);
        api.set_uniform(_prog, _scale_loc, radius);
        gl.disable(GL.depth_test);
        gl.disable(GL.cull_face);
        gl.bind_vertex_array(_vao);
        gl.polygon_mode(GL.front_and_back, GL.fill);
        api.set_uniform(_prog, _color_loc, _fcolor);
        draw_using_instructions(api, view(_ops));
        gl.polygon_mode(GL.front_and_back, GL.line);
        api.set_uniform(_prog, _color_loc, _ecolor);
        draw_using_instructions(api, view(_ops));
        gl.polygon_mode(GL.front_and_back, GL.fill);
        gl.enable(GL.cull_face);
        gl.enable(GL.depth_test);
        gl.clear(GL.depth_buffer_bit);
    });
    return *this;
}
//------------------------------------------------------------------------------
auto background_icosahedron::clear(
  video_context& video,
  const orbiting_camera& camera) noexcept -> background_icosahedron& {
    return clear(
      video,
      camera.matrix(video),
      camera.position().to_vector(),
      camera.skybox_distance());
}
//------------------------------------------------------------------------------
auto background_icosahedron::edge_color(oglplus::vec4 c) noexcept
  -> background_icosahedron& {
    _ecolor = c;
    return *this;
}
//------------------------------------------------------------------------------
auto background_icosahedron::face_color(oglplus::vec4 c) noexcept
  -> background_icosahedron& {
    _fcolor = c;
    return *this;
}
//------------------------------------------------------------------------------
// background_skybox
//------------------------------------------------------------------------------
void background_skybox::_init(auto& gl, auto& GL, auto& api) noexcept {
    memory::buffer temp;

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
            fragColor = textureLod(Tex, geomCoord, 0);
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
            layout(triangle_strip, max_vertices = 192) out;
            in vec3 vertCoord[3];
            out vec3 geomCoord;
            uniform mat4 Camera;
			uniform vec3 Offset = vec3(0.0);
            uniform float Scale = 1.0;

            void emit(vec3 bary) {
                vec3 Position =
                    gl_in[0].gl_Position.xyz * bary.x+
                    gl_in[1].gl_Position.xyz * bary.y+
                    gl_in[2].gl_Position.xyz * bary.z;
                Position = normalize(Position);
                gl_Position = Camera * vec4(Position * Scale + Offset, 1.0);
                geomCoord =
                    vertCoord[0] * bary.x+
                    vertCoord[1] * bary.y+
                    vertCoord[2] * bary.z;
				geomCoord = normalize(geomCoord);
                EmitVertex();
            }

            void subdivide0(vec3 a, vec3 b, vec3 c) {
                emit(a);
                emit(b);
                emit(c);
                EndPrimitive();
            }

            void subdivide1(vec3 a, vec3 b, vec3 c) {
                subdivide0(a, (a + b) * 0.5, (c + a) * 0.5);
                subdivide0(b, (b + c) * 0.5, (a + b) * 0.5);
                subdivide0(c, (c + a) * 0.5, (b + c) * 0.5);
                subdivide0((a + b) * 0.5, (b + c) * 0.5, (c + a) * 0.5);
            }

            void subdivide2(vec3 a, vec3 b, vec3 c) {
                subdivide1(a, (a + b) * 0.5, (c + a) * 0.5);
                subdivide1(b, (b + c) * 0.5, (a + b) * 0.5);
                subdivide1(c, (c + a) * 0.5, (b + c) * 0.5);
                subdivide1((a + b) * 0.5, (b + c) * 0.5, (c + a) * 0.5);
            }

            void subdivide3(vec3 a, vec3 b, vec3 c) {
                subdivide2(a, (a + b) * 0.5, (c + a) * 0.5);
                subdivide2(b, (b + c) * 0.5, (a + b) * 0.5);
                subdivide2(c, (c + a) * 0.5, (b + c) * 0.5);
                subdivide2((a + b) * 0.5, (b + c) * 0.5, (c + a) * 0.5);
            }

            void main() {
                subdivide3(
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
			uniform vec3 Offset = vec3(0.0);
            uniform float Scale = 1.0;

            void main() {
                gl_Position = Camera * vec4(Position * Scale + Offset, 1.0);
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
    gl.get_uniform_location(_prog, "Offset") >> _offset_loc;
    gl.get_uniform_location(_prog, "Scale") >> _scale_loc;
    gl.get_uniform_location(_prog, "Tex") >> _tex_loc;

    api.set_uniform(_prog, _tex_loc, oglplus::gl_types::int_type(_tex_unit));

    oglplus::shape_generator shape{
      api,
      shapes::skybox(
        shapes::vertex_attrib_kind::position |
        shapes::vertex_attrib_kind::face_coord)};

    _ops.resize(integer(shape.operation_count()));
    shape.instructions(api, cover(_ops));

    gl.gen_vertex_arrays() >> _vao;
    gl.bind_vertex_array(_vao);

    gl.gen_buffers() >> _positions;
    shape.attrib_setup(
      api,
      _vao,
      _positions,
      oglplus::vertex_attrib_location{0},
      eagine::shapes::vertex_attrib_kind::position,
      temp);

    gl.gen_buffers() >> _coords;
    shape.attrib_setup(
      api,
      _vao,
      _coords,
      oglplus::vertex_attrib_location{1},
      eagine::shapes::vertex_attrib_kind::face_coord,
      temp);

    gl.gen_buffers() >> _indices;
    shape.index_setup(api, _indices, temp);
}
//------------------------------------------------------------------------------
background_skybox::background_skybox(
  video_context& video,
  oglplus::gl_types::enum_type tex_unit) noexcept
  : _tex_unit{tex_unit} {
    video.with_gl(
      [&, this](auto& gl, auto& GL, auto& api) { _init(gl, GL, api); });
}
//------------------------------------------------------------------------------
auto background_skybox::set_skybox_unit(
  video_context& video,
  oglplus::texture_unit tu) noexcept -> background_skybox& {
    video.with_gl([&, this](auto&, auto&, auto& glapi) {
        glapi.use_program(_prog);
        glapi.set_uniform(_prog, _tex_loc, tu);
    });
    return *this;
}
//------------------------------------------------------------------------------
void background_skybox::_clean_up(auto& gl) noexcept {
    gl.delete_program(std::move(_prog));
    gl.delete_buffers(std::move(_indices));
    gl.delete_buffers(std::move(_coords));
    gl.delete_buffers(std::move(_positions));
    gl.delete_vertex_arrays(std::move(_vao));
}
//------------------------------------------------------------------------------
auto background_skybox::clean_up(video_context& video) noexcept
  -> background_skybox& {
    video.with_gl([this](auto& gl) { _clean_up(gl); });
    return *this;
}
//------------------------------------------------------------------------------
auto background_skybox::clear(
  video_context& video,
  const mat4& cam_mat,
  const vec3& offset,
  const float distance) noexcept -> background_skybox& {
    video.with_gl([&, this](auto& gl, auto& GL, auto& api) {
        gl.use_program(_prog);
        api.set_uniform(_prog, _camera_loc, cam_mat);
        api.set_uniform(_prog, _offset_loc, offset);
        api.set_uniform(_prog, _scale_loc, distance);
        gl.bind_vertex_array(_vao);
        gl.disable(GL.depth_test);
        gl.disable(GL.cull_face);
        draw_using_instructions(api, view(_ops));
        gl.enable(GL.cull_face);
        gl.enable(GL.depth_test);

        gl.clear_depth(1.F);
        gl.clear(GL.depth_buffer_bit | GL.stencil_buffer_bit);
    });
    return *this;
}
//------------------------------------------------------------------------------
auto background_skybox::clear(
  video_context& video,
  const orbiting_camera& camera) noexcept -> background_skybox& {
    return clear(
      video,
      camera.matrix(video),
      camera.position().to_vector(),
      camera.skybox_distance());
}
//------------------------------------------------------------------------------
} // namespace eagine::app

