/// @example app/024_overdraw/main.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_EXAMPLE_MAIN_HPP // NOLINT(llvm-header-guard)
#define EAGINE_APP_EXAMPLE_MAIN_HPP

#include "resources.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class example : public timeouting_application {
public:
    example(execution_context&, video_context&);

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

    auto video() noexcept -> auto& {
        return _video;
    }

    auto loader() noexcept -> auto& {
        return context().loader();
    }

    auto cleanup() noexcept -> auto& {
        return _cleanup;
    }

    auto camera() noexcept -> auto& {
        return _camera;
    }

private:
    void _on_resource_loaded(const loaded_resource_base&) noexcept;

    cleanup_group _cleanup;
    video_context& _video;

    draw_program _draw_prog;
    screen_program _screen_prog;
    shape_geometry _shape;
    screen_geometry _screen;
    draw_buffers _draw_bufs;

    orbiting_camera _camera;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
