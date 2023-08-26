/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_MODELS_HPP
#define EAGINE_APP_MODEL_VIEWER_MODELS_HPP
#include "geometry.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class model_viewer_models {
public:
    model_viewer_models(execution_context&, video_context&);

    signal<void() noexcept> loaded;

    explicit operator bool() const noexcept {
        return are_all_loaded();
    }

    auto are_all_loaded() const noexcept -> bool;
    auto load_if_needed(execution_context&, video_context&) noexcept
      -> model_viewer_models&;

    auto load(url locator, execution_context&, video_context&)
      -> model_viewer_models&;

    auto use(video_context& video) -> model_viewer_models&;

    auto bounding_sphere() noexcept -> oglplus::sphere;
    auto attrib_bindings() noexcept -> const oglplus::vertex_attrib_bindings&;
    auto draw(video_context&) -> model_viewer_models&;

    auto clean_up(execution_context& ctx, video_context& video)
      -> model_viewer_models&;

private:
    void _on_loaded() noexcept;
    auto _load_handler() noexcept;

    std::vector<model_viewer_geometry> _loaded;
    std::size_t _selected{0U};
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
