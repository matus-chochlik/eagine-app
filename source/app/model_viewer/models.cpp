/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "models.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_models::model_viewer_models(
  execution_context& ctx,
  video_context& video) {
    load(url{"json:///TraficCone"}, ctx, video);
    load(url{"json:///Guitar"}, ctx, video);
}
//------------------------------------------------------------------------------
void model_viewer_models::_on_loaded() noexcept {
    loaded();
}
//------------------------------------------------------------------------------
auto model_viewer_models::_load_handler() noexcept {
    return make_callable_ref<&model_viewer_models::_on_loaded>(this);
}
//------------------------------------------------------------------------------
auto model_viewer_models::are_all_loaded() const noexcept -> bool {
    for(auto& geometry : _loaded) {
        if(not geometry.is_loaded()) {
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
auto model_viewer_models::load_if_needed(
  execution_context& ctx,
  video_context& video) noexcept -> model_viewer_models& {
    for(auto& geometry : _loaded) {
        geometry.load_if_needed(ctx, video);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_models::load(
  url locator,
  execution_context& ctx,
  video_context& video) -> model_viewer_models& {
    _loaded.emplace_back(make_viewer_geometry(std::move(locator), ctx, video));
    _loaded.back().signals().loaded.connect(_load_handler());
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_models::use(video_context& video) -> model_viewer_models& {
    assert(_selected < _loaded.size());
    _loaded[_selected].use(video);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_models::bounding_sphere() noexcept -> oglplus::sphere {
    assert(_selected < _loaded.size());
    return _loaded[_selected].bounding_sphere();
}
//------------------------------------------------------------------------------
auto model_viewer_models::attrib_bindings() noexcept
  -> const oglplus::vertex_attrib_bindings& {
    assert(_selected < _loaded.size());
    return _loaded[_selected].attrib_bindings();
}
//------------------------------------------------------------------------------
auto model_viewer_models::draw(video_context& video) -> model_viewer_models& {
    assert(_selected < _loaded.size());
    _loaded[_selected].draw(video);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_models::clean_up(execution_context& ctx, video_context& video)
  -> model_viewer_models& {
    for(auto& geometry : _loaded) {
        geometry.clean_up(ctx, video);
    }
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

