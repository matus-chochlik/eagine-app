/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "background.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
//  Wrapper
//------------------------------------------------------------------------------
auto model_viewer_background::clear(
  video_context& video,
  orbiting_camera& camera) -> model_viewer_background& {
    assert(_impl);
    _impl->clear(video, camera.matrix(video), camera.skybox_distance());
    return *this;
}
//------------------------------------------------------------------------------
//  Background
//------------------------------------------------------------------------------
class model_viewer_default_background : public model_viewer_background_intf {
public:
    model_viewer_default_background(execution_context&, video_context&);

    auto is_loaded() noexcept -> bool final;
    void load_if_needed(execution_context&, video_context&) final;
    void use(video_context&) final;
    void clear(video_context&, const mat4& camera, const float distance) final;
    void clean_up(execution_context&, video_context&) final;
    auto settings_height() -> float final;
    void settings(const guiplus::imgui_api&) noexcept final;

private:
    std::array<std::tuple<oglplus::vec4, oglplus::vec4, string_view>, 6>
      _color_presets{
        {{{0.5F, 0.5F, 0.5F, 1.F}, {0.3F, 0.3F, 0.3F, 1.F}, {"Custom"}},
         {{0.5F, 0.5F, 0.5F, 1.F}, {0.3F, 0.3F, 0.3F, 1.F}, {"Gray"}},
         {{0.2F, 0.2F, 0.2F, 1.F}, {0.1F, 0.1F, 0.1F, 1.F}, {"Dark"}},
         {{0.8F, 0.8F, 0.8F, 1.F}, {0.9F, 0.9F, 0.9F, 1.F}, {"Light"}},
         {{0.0F, 0.0F, 0.0F, 1.F}, {1.0F, 1.0F, 0.1F, 1.F}, {"Black/Yellow"}},
         {{0.0F, 0.0F, 0.0F, 1.F}, {1.0F, 0.5F, 1.0F, 1.F}, {"Black/Pink"}}}};
    std::size_t _selected_color{1U};

    auto _selected_custom_color() const noexcept -> bool {
        return _selected_color == 0U;
    }

    background_icosahedron _bg;
};
//------------------------------------------------------------------------------
model_viewer_default_background::model_viewer_default_background(
  execution_context&,
  video_context& video)
  : _bg{
      video,
      std::get<0>(_color_presets[_selected_color]),
      std::get<1>(_color_presets[_selected_color]),
      1.F} {}
//------------------------------------------------------------------------------
auto model_viewer_default_background::is_loaded() noexcept -> bool {
    return true;
}
//------------------------------------------------------------------------------
void model_viewer_default_background::load_if_needed(
  execution_context&,
  video_context&) {}
//------------------------------------------------------------------------------
void model_viewer_default_background::use(video_context&) {}
//------------------------------------------------------------------------------
void model_viewer_default_background::clear(
  video_context& video,
  const mat4& camera,
  const float distance) {
    _bg.clear(video, camera, distance);
}
//------------------------------------------------------------------------------
void model_viewer_default_background::clean_up(
  execution_context&,
  video_context& video) {
    _bg.clean_up(video);
}
//------------------------------------------------------------------------------
auto model_viewer_default_background::settings_height() -> float {
    return _selected_custom_color() ? 25.F + 45.F : 25.F;
}
//------------------------------------------------------------------------------
void model_viewer_default_background::settings(
  const guiplus::imgui_api& gui) noexcept {
    const auto color_name{[this](std::size_t index) {
        return std::get<2>(_color_presets[index]);
    }};
    std::size_t next_color = _selected_color;
    if(gui.begin_combo("Colors", color_name(next_color)).or_false()) {
        for(const auto i : index_range(_color_presets)) {
            const bool is_selected{i == next_color};
            if(gui.selectable(color_name(i), is_selected).or_false()) {
                next_color = i;
            }
            if(is_selected) {
                gui.set_item_default_focus();
            }
        }
        gui.end_combo();
    }
    if(_selected_color != next_color) {
        _selected_color = next_color;
        _bg.edge_color(std::get<0>(_color_presets[_selected_color]));
        _bg.face_color(std::get<1>(_color_presets[_selected_color]));
    }

    if(_selected_custom_color()) {
        auto color = _bg.edge_color();
        if(gui.color_edit("Edge", color)) {
            _bg.edge_color(color);
            std::get<0>(_color_presets[_selected_color]) = color;
        }
        color = _bg.edge_color();
        if(gui.color_edit("Face", color)) {
            _bg.face_color(color);
            std::get<1>(_color_presets[_selected_color]) = color;
        }
    }
}
//------------------------------------------------------------------------------
//  Default background
//------------------------------------------------------------------------------
auto make_viewer_resource(
  std::type_identity<model_viewer_background>,
  url,
  execution_context& ctx,
  video_context& video) -> model_viewer_background_holder {
    return {hold<model_viewer_default_background>, ctx, video};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
