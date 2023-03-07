/// @example app/028_compute_particles/main.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_EXAMPLE_MAIN_HPP // NOLINT(llvm-header-guard)
#define EAGINE_APP_EXAMPLE_MAIN_HPP

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example : public timeouting_application {
public:
    example(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

    auto video() noexcept -> auto& {
        return _video;
    }

    auto camera() noexcept -> auto& {
        return _camera;
    }

    auto emit_position() noexcept {
        return _path.position(context().state().frame_time().value() * 0.1F);
    }

    auto frame_duration() noexcept {
        return context().state().frame_duration().value();
    }

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    video_context& _video;
    background_icosahedron _bg;

    emit_program _emit_prog;
    draw_program _draw_prog;
    particles _particles;
    particle_path _path;

    orbiting_camera _camera;
};

} // namespace eagine::app

#endif
