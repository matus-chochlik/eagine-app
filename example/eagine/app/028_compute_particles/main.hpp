/// @example app/028_compute_particles/main.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_MAIN_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_MAIN_HPP

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example : public application {
public:
    example(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

    auto ctx() noexcept -> auto& {
        return _ctx;
    }

    auto video() noexcept -> auto& {
        return _video;
    }

    auto loader() noexcept -> auto& {
        return _ctx.loader();
    }

    auto camera() noexcept -> auto& {
        return _camera;
    }

    auto emit_position() noexcept {
        return _path.position(ctx().state().frame_time().value() * 0.1F);
    }

    auto frame_duration() noexcept {
        return ctx().state().frame_duration().value();
    }

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    execution_context& _ctx;
    video_context& _video;
    background_icosahedron _bg;

    math::cubic_bezier_loop<oglplus::vec3, float> _path;

    emit_program _emit_prog;
    draw_program _draw_prog;
    particles _particles;

    orbiting_camera _camera;
    timeout _is_done{std::chrono::seconds{60}};
};

} // namespace eagine::app

#endif
