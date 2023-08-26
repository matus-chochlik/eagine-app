/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_PROGRAMS_HPP
#define EAGINE_APP_MODEL_VIEWER_PROGRAMS_HPP
#include "program.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class model_viewer_programs {
public:
    model_viewer_programs(execution_context&, video_context&);

    signal<void() noexcept> loaded;

    explicit operator bool() const noexcept {
        return are_all_loaded();
    }

    auto are_all_loaded() const noexcept -> bool;
    auto load_if_needed(execution_context&, video_context&) noexcept
      -> model_viewer_programs&;

    auto load(url locator, execution_context&, video_context&)
      -> model_viewer_programs&;

    auto use(video_context& video) -> model_viewer_programs&;

    auto apply_bindings(
      video_context&,
      const oglplus::vertex_attrib_bindings& attrib_bindings)
      -> model_viewer_programs&;

    auto set_camera(video_context&, orbiting_camera& camera)
      -> model_viewer_programs&;

    auto clean_up(execution_context& ctx, video_context& video)
      -> model_viewer_programs&;

private:
    void _on_loaded() noexcept;
    auto _load_handler() noexcept;

    std::vector<model_viewer_program> _loaded;
    std::size_t _selected{0U};
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
