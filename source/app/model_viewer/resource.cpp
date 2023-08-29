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
  const string_view head,
  const guiplus::imgui_api& gui) noexcept {
    gui.separator_text(head);
    gui.push_id(head);
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
    gui.pop_id();
}
//------------------------------------------------------------------------------
void model_viewer_resources_base::update() noexcept {
    if(_selected_index != _next_index) {
        _selected_index = _next_index;
        _on_selected();
    }
}
//------------------------------------------------------------------------------
auto model_viewer_resources_base::all_resource_count() noexcept -> span_size_t {
    return span_size(_names.size());
}
//------------------------------------------------------------------------------
void model_viewer_resources_base::_add_name(std::string name) {
    _names.emplace_back(std::move(name));
}
//------------------------------------------------------------------------------
void model_viewer_resources_base::_on_loaded() noexcept {
    loaded();
    selected();
}
//------------------------------------------------------------------------------
void model_viewer_resources_base::_on_selected() noexcept {
    selected();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
