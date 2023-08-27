/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "resource.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
void model_viewer_resource_intf::signal_loaded() {
    model_viewer_resource_signals::loaded();
}
//------------------------------------------------------------------------------
void model_viewer_resources_base::settings(
  const guiplus::imgui_api& gui) noexcept {
    assert(_next_index < _names.size());
    if(gui.begin_combo("Current", _names[_next_index]).or_false()) {
        for(const auto i : index_range(_names)) {
            const bool is_selected{i == _next_index};
            const auto name{_names[i]};
            if(gui.selectable(name, is_selected).or_false()) {
                _next_index = i;
            }
            if(is_selected) {
                gui.set_item_default_focus();
            }
        }
        gui.end_combo();
    }
}
//------------------------------------------------------------------------------
void model_viewer_resources_base::update() noexcept {
    if(_selected_index != _next_index) {
        _selected_index = _next_index;
        _on_loaded();
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::app
