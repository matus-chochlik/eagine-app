/// @example app/028_compute_particles/resources.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_RESOURCES_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_RESOURCES_HPP

#include <eagine/oglplus/gl.hpp>
#include <eagine/oglplus/gl_api.hpp>

#include <eagine/app/fwd.hpp>
#include <eagine/oglplus/shapes/drawing.hpp>

namespace eagine::app {
class example;
//------------------------------------------------------------------------------
// particles
//------------------------------------------------------------------------------
class particles {
public:
    void init(example&);
    void emit(example&);
    void draw(example&);

    static auto origin_loc() noexcept {
        return oglplus::vertex_attrib_location{0};
    }

    static auto cursors_binding() noexcept {
        return 0U;
    }

    static auto random_binding() noexcept {
        return 0U;
    }

    static auto offsets_binding() noexcept {
        return 1U;
    }

    static auto velocities_binding() noexcept {
        return 2U;
    }

    static auto ages_binding() noexcept {
        return 3U;
    }

private:
    const unsigned _count{4096U};

    oglplus::owned_vertex_array_name _vao;

    oglplus::owned_buffer_name _origin;
    oglplus::owned_buffer_name _cursors;
    oglplus::owned_buffer_name _random;
    oglplus::owned_buffer_name _offsets;
    oglplus::owned_buffer_name _velocities;
    oglplus::owned_buffer_name _ages;
};
//------------------------------------------------------------------------------
// programs
//------------------------------------------------------------------------------
class emit_program {
public:
    void init(example&);
    void prepare_frame(example&);
    void bind_random(example&, oglplus::gl_types::uint_type);
    void bind_offsets(example&, oglplus::gl_types::uint_type);
    void bind_velocities(example&, oglplus::gl_types::uint_type);
    void bind_ages(example&, oglplus::gl_types::uint_type);
    void use(example&);

private:
    oglplus::owned_program_name _prog;
    oglplus::uniform_location _emit_position_loc;
    oglplus::uniform_location _delta_time_loc;
};
//------------------------------------------------------------------------------
class draw_program {
public:
    void init(example&);
    void prepare_frame(example&);
    void bind_origin_location(example&, oglplus::vertex_attrib_location);
    void bind_offsets(example&, oglplus::gl_types::uint_type);
    void bind_ages(example&, oglplus::gl_types::uint_type);
    void use(example&);

private:
    oglplus::owned_program_name _prog;
    oglplus::uniform_location _camera_mat_loc;
    oglplus::uniform_location _perspective_mat_loc;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
