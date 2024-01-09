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
auto model_viewer_resources_base::_settings_height(
  optional_reference<model_viewer_resource_intf> impl) noexcept -> float {
    return 45.F + impl.member(&model_viewer_resource_intf::settings_height)
                    .value_or(0.F);
}
//------------------------------------------------------------------------------
void model_viewer_resources_base::_settings(
  const string_view head,
  optional_reference<model_viewer_resource_intf> impl,
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
    if(impl) {
        impl->settings(gui);
    }
    gui.pop_id();
}
//------------------------------------------------------------------------------
auto model_viewer_resources_base::_resource_name(const program_arg& arg)
  -> std::string {
    if(url::is_url(arg.next())) {
        return to_string(url{arg.next()}
                           .path_str()
                           .and_then([](auto p) {
                               return valid_if_not_empty<string_view>{
                                 strip_prefix(p, string_view{"/"})};
                           })
                           .value_or("unknown"));
    }
    return arg.next().get_string();
}
//------------------------------------------------------------------------------
auto model_viewer_resources_base::_resource_url(const program_arg& arg) -> url {
    if(url::is_url(arg.next())) {
        return url{arg.next()};
    }
    return url{arg.next().next()};
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
