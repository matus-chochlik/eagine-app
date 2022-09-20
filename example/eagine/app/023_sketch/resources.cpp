/// @example app/023_sketch/resources.cpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// sketch program
//------------------------------------------------------------------------------
void sketch_program::init(video_context& vc) {
    create(vc)
      .build(vc, embed<"SketchProg">("sketch.oglpprog"))
      .use(vc)
      .query(vc, "Model", _model_loc)
      .query(vc, "View", _view_loc)
      .query(vc, "Projection", _projection_loc)
      .query(vc, "viewportDimensions", _vp_dim_loc);
}
//------------------------------------------------------------------------------
void sketch_program::prepare_frame(
  video_context& vc,
  orbiting_camera& camera,
  float t) {
    const auto [width, height] = vc.surface_size();
    const auto a = t * 0.125F;
    use(vc)
      .set(vc, _model_loc, oglplus::matrix_rotation_x(right_angles_(a))())
      .set(vc, _view_loc, camera.transform_matrix())
      .set(vc, _projection_loc, camera.perspective_matrix(vc.surface_aspect()))
      .set(vc, _vp_dim_loc, oglplus::vec2(width, height));
}
//------------------------------------------------------------------------------
void sketch_program::bind_position_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Position");
}
//------------------------------------------------------------------------------
void sketch_program::bind_normal_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Normal");
}
//------------------------------------------------------------------------------
void sketch_program::bind_coord_location(
  video_context& vc,
  oglplus::vertex_attrib_location loc) {
    bind(vc, loc, "Coord");
}
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
void shape_geometry::init(video_context& vc) {
    geometry_and_bindings::init(
      shapes::add_triangle_adjacency(
        shapes::unit_twisted_torus(
          shapes::vertex_attrib_kind::position |
            shapes::vertex_attrib_kind::normal |
            shapes::vertex_attrib_kind::wrap_coord,
          9,
          13,
          9,
          0.5F),
        vc.parent()),
      vc);
}
//------------------------------------------------------------------------------
// sketch texture
//------------------------------------------------------------------------------
void sketch_texture::init(video_context& vc) {
    const auto& [gl, GL] = vc.gl_api();
    const int side = 512;
    std::vector<float> scratches(std_size(side * side));

    auto scratch = [&](auto x, auto y, auto layer) {
        const auto k =
          std_size(((side + y) % side) * side + ((side + x) % side));
        if(scratches[k] < layer) {
            scratches[k] = layer;
        }
    };

    for(const auto i : integer_range(3000)) {
        std::array<float, 6> rand{};
        vc.parent().random_uniform_01(cover(rand));

        const auto xmin = std::min(rand[0], rand[1]);
        const auto xmax = std::max(rand[0], rand[1]);
        const auto ymin = std::min(rand[2], rand[3]);
        const auto ymax = std::max(rand[2], rand[3]);
        const auto w = int((xmax - xmin) * float(side));
        const auto h = int((ymax - ymin) * float(side));
        const auto l = (rand[5] - 0.5F) * 0.4F;

        for(int t : integer_range(2)) {
            if(w > h) {
                const auto layer = rand[4];
                for(auto x : integer_range(w)) {
                    const float c = l * float(x + 1) / float(w);
                    scratch(
                      int(float(x) + xmin * side) + i,
                      int(float(h + t) * c + ymin * side),
                      layer);
                }
            } else {
                const auto layer = rand[4] * 0.85F;
                for(auto y : integer_range(h)) {
                    const float c = l * float(y + 1) / float(h);
                    scratch(
                      int(float(w + t) * c + xmin * side),
                      int(float(y) + ymin * side) + i,
                      layer);
                }
            }
        }
    }

    gl.gen_textures() >> _tex;
    gl.active_texture(GL.texture0);
    gl.bind_texture(GL.texture_2d, _tex);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_min_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_mag_filter, GL.linear);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_s, GL.mirrored_repeat);
    gl.tex_parameter_i(GL.texture_2d, GL.texture_wrap_t, GL.mirrored_repeat);
    gl.tex_image2d(
      GL.texture_2d,
      0,
      GL.red,
      side,
      side,
      0,
      GL.red,
      GL.float_,
      as_bytes(view(scratches)));
}
//------------------------------------------------------------------------------
void sketch_texture::clean_up(video_context& vc) {
    const auto& gl = vc.gl_api();
    gl.delete_textures(std::move(_tex));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
