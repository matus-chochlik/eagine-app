/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_GEOMETRY_HPP
#define EAGINE_APP_GEOMETRY_HPP

#include "context.hpp"
#include <eagine/oglplus/shapes/generator.hpp>
#include <eagine/oglplus/shapes/geometry.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class geometry_and_bindings : public oglplus::geometry_and_bindings {
    using base = oglplus::geometry_and_bindings;

public:
    using base::init;

    auto init(
      const std::shared_ptr<shapes::generator>& gen,
      video_context& vc,
      memory::buffer& temp) -> auto& {
        const auto& glapi = vc.gl_api();
        oglplus::shape_generator shape(glapi, gen);
        oglplus::geometry_and_bindings::init(glapi, shape, temp);
        return *this;
    }

    auto init(
      const std::shared_ptr<shapes::generator>& gen,
      execution_context& ec,
      video_context& vc) -> auto& {
        return init(gen, vc, ec.buffer());
    }

    auto clean_up(video_context& vc) -> auto& {
        base::clean_up(vc.gl_api());
        return *this;
    }

    using base::use;

    auto use(video_context& vc) -> auto& {
        base::use(vc.gl_api());
        return *this;
    }

    using base::draw;

    auto draw(video_context& vc) -> auto& {
        base::draw(vc.gl_api());
        return *this;
    }

    auto use_and_draw(video_context& vc) -> auto& {
        return use(vc).draw(vc);
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
