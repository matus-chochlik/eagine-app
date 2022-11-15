/// @example app/025_recursive_cube/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

import eagine.core;
import eagine.shapes;
import eagine.oglplus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
// program
//------------------------------------------------------------------------------
class cube_program : public gl_program_resource {
public:
    cube_program(execution_context&);
    void set_texture(execution_context&, oglplus::gl_types::int_type);
    void set_projection(execution_context&, const oglplus::trfmat<4>&);
    void prepare_frame(execution_context&);

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    oglplus::uniform_location projection_loc;
    oglplus::uniform_location modelview_loc;
    oglplus::uniform_location light_pos_loc;
    oglplus::uniform_location cube_tex_loc;

    radians_t<float> rad{0.F};
};
//------------------------------------------------------------------------------
// geometry
//------------------------------------------------------------------------------
class cube_geometry : public gl_geometry_and_bindings_resource {
public:
    cube_geometry(execution_context&);
};
//------------------------------------------------------------------------------
// draw buffers
//------------------------------------------------------------------------------
class cube_draw_buffers {
public:
    void init(execution_context&);
    void clean_up(execution_context&);

    auto back_fbo() const noexcept {
        return oglplus::framebuffer_name{objs.back().fbo};
    }

    auto front_tex_unit() const noexcept {
        return objs.front().tex_unit;
    }

    auto side() const noexcept {
        return tex_side;
    }

    void swap() {
        objs.swap();
    }

private:
    const oglplus::gl_types::sizei_type tex_side{512};

    struct _buffer_objects {
        oglplus::gl_types::int_type tex_unit{};
        oglplus::owned_texture_name tex{};
        oglplus::owned_renderbuffer_name rbo{};
        oglplus::owned_framebuffer_name fbo{};
    };

    double_buffer<_buffer_objects> objs{};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
