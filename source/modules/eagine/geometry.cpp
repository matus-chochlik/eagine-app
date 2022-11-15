/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:geometry;

import eagine.core.types;
import eagine.core.memory;
import eagine.shapes;
import eagine.oglplus;
import :context;

namespace eagine::app {
//------------------------------------------------------------------------------
export class gl_geometry_and_bindings : public oglplus::geometry_and_bindings {
    using base = oglplus::geometry_and_bindings;

public:
    gl_geometry_and_bindings() noexcept = default;
    gl_geometry_and_bindings(gl_geometry_and_bindings&&) noexcept = default;
    gl_geometry_and_bindings(const gl_geometry_and_bindings&) = delete;
    auto operator=(gl_geometry_and_bindings&&) noexcept
      -> gl_geometry_and_bindings& = default;
    auto operator=(const gl_geometry_and_bindings&) = delete;
    ~gl_geometry_and_bindings() noexcept = default;

    gl_geometry_and_bindings(
      const oglplus::shape_generator& shape,
      const oglplus::vertex_attrib_bindings& bindings,
      const shapes::drawing_variant var,
      video_context& vc,
      memory::buffer& temp) noexcept
      : base{vc.gl_api(), shape, bindings, var, temp} {}

    gl_geometry_and_bindings(
      const std::shared_ptr<shapes::generator>& gen,
      const oglplus::vertex_attrib_bindings& bindings,
      const shapes::drawing_variant var,
      video_context& vc,
      memory::buffer& temp) noexcept
      : gl_geometry_and_bindings{
          oglplus::shape_generator{vc.gl_api(), gen},
          bindings,
          var,
          vc,
          temp} {}

    gl_geometry_and_bindings(
      const oglplus::shape_generator& shape,
      const oglplus::vertex_attrib_bindings& bindings,
      video_context& vc,
      memory::buffer& temp) noexcept
      : base{vc.gl_api(), shape, bindings, temp} {}

    gl_geometry_and_bindings(
      const oglplus::shape_generator& shape,
      const oglplus::vertex_attrib_bindings& bindings,
      video_context& vc) noexcept
      : gl_geometry_and_bindings{shape, bindings, vc, vc.parent().buffer()} {}

    gl_geometry_and_bindings(
      const std::shared_ptr<shapes::generator>& gen,
      const oglplus::vertex_attrib_bindings& bindings,
      video_context& vc,
      memory::buffer& temp) noexcept
      : gl_geometry_and_bindings{
          oglplus::shape_generator{vc.gl_api(), gen},
          bindings,
          vc,
          temp} {}

    gl_geometry_and_bindings(
      const oglplus::shape_generator& shape,
      video_context& vc,
      memory::buffer& temp) noexcept
      : base{vc.gl_api(), shape, temp} {}

    gl_geometry_and_bindings(
      const oglplus::shape_generator& shape,
      video_context& vc) noexcept
      : gl_geometry_and_bindings{shape, vc, vc.parent().buffer()} {}

    gl_geometry_and_bindings(
      const std::shared_ptr<shapes::generator>& gen,
      video_context& vc,
      memory::buffer& temp) noexcept
      : gl_geometry_and_bindings{
          oglplus::shape_generator{vc.gl_api(), gen},
          vc,
          temp} {}

    gl_geometry_and_bindings(
      const std::shared_ptr<shapes::generator>& gen,
      video_context& vc) noexcept
      : gl_geometry_and_bindings{gen, vc, vc.parent().buffer()} {}

    auto init(gl_geometry_and_bindings&& temp) noexcept -> auto& {
        *this = std::move(temp);
        return *this;
    }

    auto reinit(video_context& vc, gl_geometry_and_bindings&& temp) noexcept
      -> gl_geometry_and_bindings& {
        clean_up(vc);
        *this = std::move(temp);
        return *this;
    }

    using base::clean_up;

    auto clean_up(video_context& vc) noexcept -> gl_geometry_and_bindings& {
        base::clean_up(vc.gl_api());
        return *this;
    }

    using base::use;

    auto use(video_context& vc) const -> const gl_geometry_and_bindings& {
        base::use(vc.gl_api());
        return *this;
    }

    using base::draw;

    auto draw(video_context& vc) const -> const gl_geometry_and_bindings& {
        base::draw(vc.gl_api());
        return *this;
    }

    auto use_and_draw(video_context& vc) const
      -> const gl_geometry_and_bindings& {
        return use(vc).draw(vc);
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

