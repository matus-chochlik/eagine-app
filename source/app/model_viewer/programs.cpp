/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "programs.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
model_viewer_programs::model_viewer_programs(
  execution_context& ctx,
  video_context& video) {
    load(url{"json:///DfaultProg"}, ctx, video);
    load(url{"json:///Nml2ClrPrg"}, ctx, video);
}
//------------------------------------------------------------------------------
void model_viewer_programs::_on_loaded() noexcept {
    loaded();
}
//------------------------------------------------------------------------------
auto model_viewer_programs::_load_handler() noexcept {
    return make_callable_ref<&model_viewer_programs::_on_loaded>(this);
}
//------------------------------------------------------------------------------
auto model_viewer_programs::are_all_loaded() const noexcept -> bool {
    for(auto& program : _loaded) {
        if(not program.is_loaded()) {
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::load_if_needed(
  execution_context& ctx,
  video_context& video) noexcept -> model_viewer_programs& {
    for(auto& program : _loaded) {
        program.load_if_needed(ctx, video);
    }
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::load(
  url locator,
  execution_context& ctx,
  video_context& video) -> model_viewer_programs& {
    _loaded.emplace_back(make_viewer_program(std::move(locator), ctx, video));
    _loaded.back().signals().loaded.connect(_load_handler());
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::use(video_context& video)
  -> model_viewer_programs& {
    assert(_selected < _loaded.size());
    _loaded[_selected].use(video);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::apply_bindings(
  video_context& video,
  const oglplus::vertex_attrib_bindings& attrib_bindings)
  -> model_viewer_programs& {
    assert(_selected < _loaded.size());
    _loaded[_selected].apply_bindings(video, attrib_bindings);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::set_camera(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_programs& {
    assert(_selected < _loaded.size());
    _loaded[_selected].set_camera(video, camera);
    return *this;
}
//------------------------------------------------------------------------------
auto model_viewer_programs::clean_up(
  execution_context& ctx,
  video_context& video) -> model_viewer_programs& {
    for(auto& program : _loaded) {
        program.load_if_needed(ctx, video);
    }
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app

