/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "geometry.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  Default geometry
//------------------------------------------------------------------------------
class default_model_viewer_geometry
  : public model_viewer_geometry_intf
  , public gl_geometry_and_bindings {
public:
    default_model_viewer_geometry(
      const shared_holder<shapes::generator>&,
      video_context&);

    void use(video_context&) final;

    auto bounding_sphere() noexcept -> oglplus::sphere final;
    void clean_up(execution_context&, video_context&) final;

private:
    oglplus::sphere _bounding_sphere;
};
//------------------------------------------------------------------------------
default_model_viewer_geometry::default_model_viewer_geometry(
  const shared_holder<shapes::generator>& gen,
  video_context& video) {
    const auto& glapi = video.gl_api();

    oglplus::shape_generator shape(glapi, gen);
    _bounding_sphere = shape.bounding_sphere();

    gl_geometry_and_bindings::init({shape, video});
}
//------------------------------------------------------------------------------
void default_model_viewer_geometry::use(video_context&) {}
//------------------------------------------------------------------------------
auto default_model_viewer_geometry::bounding_sphere() noexcept
  -> oglplus::sphere {
    return _bounding_sphere;
}
//------------------------------------------------------------------------------
void default_model_viewer_geometry::clean_up(
  execution_context&,
  video_context& video) {
    gl_geometry_and_bindings::clean_up(video);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
