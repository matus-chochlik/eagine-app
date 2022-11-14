/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app;

import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.identifier;
import eagine.core.value_tree;
import eagine.core.units;
import <optional>;
import <string>;

namespace eagine::app {
//------------------------------------------------------------------------------
// valtree_float_vector_builder
//------------------------------------------------------------------------------
class valtree_float_vector_builder
  : public valtree_builder_base<valtree_float_vector_builder> {
    using base = valtree_builder_base<valtree_float_vector_builder>;

public:
    using base::base;
    using base::do_add;

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        if(path.size() == 2) {
            if(!data.empty()) {
                if((path.starts_with("values")) || (path.starts_with("data"))) {
                    for(const auto v : data) {
                        _values.push_back(float(v));
                    }
                }
            }
        } else if(path.size() == 1) {
            if(data.has_single_value()) {
                if((path.starts_with("count")) || (path.starts_with("size"))) {
                    _values.reserve(std_size(extract(data)));
                }
            }
        }
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        if(path.size() == 2) {
            if(!data.empty()) {
                if((path.starts_with("values")) && (path.starts_with("data"))) {
                    for(const auto v : data) {
                        _values.push_back(float(v));
                    }
                }
            }
        }
    }

    void do_add(const basic_string_path& path, span<const float> data) noexcept {
        if(path.size() == 2) {
            if(!data.empty()) {
                if((path.starts_with("values")) && (path.starts_with("data"))) {
                    _values.insert(_values.end(), data.begin(), data.end());
                }
            }
        }
    }

    void finish() noexcept final {
        if(auto parent{_parent.lock()}) {
            if(const auto cont{extract(parent).continuation()}) {
                extract(cont).handle_float_vector(extract(parent), _values);
            }
        }
    }

private:
    std::vector<float> _values;
};
//------------------------------------------------------------------------------
auto make_valtree_float_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_float_vector_builder>(parent);
}
//------------------------------------------------------------------------------
// valtree_vec3_vector_builder
//------------------------------------------------------------------------------
class valtree_vec3_vector_builder
  : public valtree_builder_base<valtree_vec3_vector_builder> {
    using base = valtree_builder_base<valtree_vec3_vector_builder>;

public:
    using base::base;
    using base::do_add;

    template <typename T>
    auto _do_add(const basic_string_path& path, span<const T> data) noexcept
      -> bool {
        if(path.size() == 3) {
            if((path.starts_with("values")) || (path.starts_with("data"))) {
                if(data.has_single_value()) {
                    if((path.ends_with("x")) || (path.ends_with("r"))) {
                        _temp._v[0] = extract(data);
                        return true;
                    } else if((path.ends_with("y")) || (path.ends_with("g"))) {
                        _temp._v[1] = extract(data);
                        return true;
                    } else if((path.ends_with("z")) || (path.ends_with("b"))) {
                        _temp._v[2] = extract(data);
                        return true;
                    }
                }
                if(path.ends_with("_")) {
                    for(const auto i : integer_range(data.size())) {
                        _temp._v[_offs] = data[i];
                        if(++_offs == 3) {
                            _offs = 0;
                            _values.push_back(_temp);
                            _temp = {0.F, 0.F, 0.F};
                        }
                    }
                    return true;
                }
            }
        }
        return false;
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        if(!_do_add(path, data)) {
            if((path.size() == 1) && data.has_single_value()) {
                if((path.starts_with("count")) || (path.starts_with("size"))) {
                    _values.reserve(std_size(extract(data)));
                }
            }
        }
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        _do_add(path, data);
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.size() == 2) {
            if((path.starts_with("values")) || (path.starts_with("data"))) {
                _values.push_back(_temp);
                _temp = {0.F, 0.F, 0.F};
            }
        }
    }

    void finish() noexcept final {
        if(auto parent{_parent.lock()}) {
            extract(parent).handle_vec3_vector(extract(parent), _values);
        }
    }

private:
    math::vector<float, 3, true> _temp{0.F, 0.F, 0.F};
    std::vector<math::vector<float, 3, true>> _values;
    std::size_t _offs{0U};
};
//------------------------------------------------------------------------------
auto make_valtree_vec3_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_vec3_vector_builder>(parent);
}
//------------------------------------------------------------------------------
// camera parameters
//------------------------------------------------------------------------------
class valtree_orbiting_camera_parameters_builder
  : public valtree_builder_base<valtree_orbiting_camera_parameters_builder> {
    using base =
      valtree_builder_base<valtree_orbiting_camera_parameters_builder>;

public:
    valtree_orbiting_camera_parameters_builder(
      const std::shared_ptr<pending_resource_info>& parent,
      orbiting_camera& camera) noexcept
      : base{parent}
      , _camera{camera} {}

    using base::do_add;

    template <typename T>
    void parse_param(
      const basic_string_path& path,
      span<const T> data) noexcept {
        if((path.size() == 1) && data.has_single_value()) {
            const auto value{extract(data)};
            if(value > 0.F) {
                if(path.starts_with("near")) {
                    _camera.set_near(value);
                } else if(path.starts_with("far")) {
                    _camera.set_far(value);
                } else if(path.starts_with("orbit_min")) {
                    _camera.set_orbit_min(value);
                } else if(path.starts_with("orbit_max")) {
                    _camera.set_orbit_max(value);
                } else if(path.starts_with("fov_deg")) {
                    _camera.set_fov(degrees_(value));
                } else if(path.starts_with("fov_rad")) {
                    _camera.set_fov(radians_(value));
                }
            }
            if(path.starts_with("azimuth_deg")) {
                _camera.set_azimuth(degrees_(value));
            } else if(path.starts_with("azimuth_rad")) {
                _camera.set_azimuth(radians_(value));
            } else if(path.starts_with("elevation_deg")) {
                _camera.set_elevation(degrees_(value));
            } else if(path.starts_with("elevation_rad")) {
                _camera.set_elevation(radians_(value));
            }
        }
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        parse_param(path, data);
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        parse_param(path, data);
    }

private:
    orbiting_camera& _camera;
};
//------------------------------------------------------------------------------
auto make_valtree_camera_parameters_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  orbiting_camera& camera) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_orbiting_camera_parameters_builder>(
      parent, camera);
}
//------------------------------------------------------------------------------
// input parameters
//------------------------------------------------------------------------------
class valtree_orbiting_input_setup_builder
  : public valtree_builder_base<valtree_orbiting_input_setup_builder> {
    using base = valtree_builder_base<valtree_orbiting_input_setup_builder>;

public:
    valtree_orbiting_input_setup_builder(
      const std::shared_ptr<pending_resource_info>& parent,
      execution_context& ctx) noexcept
      : base{parent}
      , _ctx{ctx} {}

    using base::do_add;

    auto parse_msg_id(
      const span<const string_view> data,
      std::optional<message_id>& dest) noexcept -> bool {
        if(data.size() == 2) {
            if(identifier::can_be_encoded(data.front())) {
                if(identifier::can_be_encoded(data.back())) {
                    dest = message_id{
                      identifier{data.front()}, identifier{data.back()}};
                    return true;
                } else {
                    log_error("invalid method identifier ${data}")
                      .arg("data", extract(data));
                }
            } else {
                log_error("invalid class identifier ${data}")
                  .arg("data", extract(data));
            }
        } else if(data.has_single_value()) {
            if(_str_data_offs == 0) {
                if(identifier::can_be_encoded(extract(data))) {
                    _temp_id = identifier{extract(data)};
                } else {
                    log_error("invalid class identifier ${data}")
                      .arg("data", extract(data));
                }
            } else if(_str_data_offs == 1) {
                if(identifier::can_be_encoded(extract(data))) {
                    dest = message_id{_temp_id, identifier{extract(data)}};
                    return true;
                } else {
                    log_error("invalid method identifier ${data}")
                      .arg("data", extract(data));
                }
            } else {
                log_error("too many values for message id");
            }
        }
        _str_data_offs += data.size();
        return false;
    }

    auto is_parsing_input() const noexcept -> bool {
        return _status_l1 == status_type_l1::parsing_input;
    }

    auto is_parsing_slot() const noexcept -> bool {
        return _status_l1 == status_type_l1::parsing_slot;
    }

    auto add_input() noexcept -> bool {
        assert(is_parsing_input());
        if(_input_id && _type && _label) {
            if(_type == "ui_button") {
                _ctx.add_ui_button(extract(_input_id), extract(_label));
                _status_l1 = status_type_l1::unknown;
            } else if(_type == "ui_toggle") {
                _ctx.add_ui_toggle(
                  extract(_input_id),
                  extract(_label),
                  extract_or(_initial_bool, false));
                _status_l1 = status_type_l1::unknown;
            } else if(_type == "ui_slider") {
                _ctx.add_ui_slider(
                  extract(_input_id),
                  extract(_label),
                  extract_or(_min, 0.F),
                  extract_or(_max, 1.F),
                  extract_or(_initial_float, 0.5F));
                _status_l1 = status_type_l1::unknown;
            } else {
                log_error("invalid input type '${type}")
                  .arg("signal", extract(_input_id))
                  .arg("type", extract(_type));
            }
            return true;
        }
        return false;
    }

    auto add_slot_mapping() noexcept -> bool {
        assert(is_parsing_slot());
        if(_slot_id) {
            if(_input_id && _type) {
                if(_type == "trigger") {
                    _ctx.map_input(
                      extract(_slot_id),
                      extract(_input_id),
                      input_setup().trigger());
                    _input_id = {};
                    _type = {};
                } else {
                    log_error("invalid signal type '${type}")
                      .arg("slot", extract(_slot_id))
                      .arg("signal", extract(_input_id))
                      .arg("type", extract(_type));
                }
            }
        }
        return false;
    }

    void do_add(
      const basic_string_path& path,
      const span<const bool> data) noexcept {
        if(path.ends_with("initial")) {
            if(data.has_single_value()) {
                _initial_bool = extract(data);
            }
        }
    }

    template <std::floating_point T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept {
        if(
          path.ends_with("min") || path.ends_with("max") ||
          path.ends_with("initial")) {
            if(data.has_single_value()) {
                if(path.ends_with("initial")) {
                    _initial_float = extract(data);
                } else if(path.ends_with("min")) {
                    _min = extract(data);
                } else if(path.ends_with("max")) {
                    _max = extract(data);
                }
            } else {
                log_error("too many values for ${what}")
                  .arg("what", path.back());
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept {
        if(path.ends_with("label")) {
            if(data.has_single_value()) {
                if(!extract(data).empty()) {
                    _label = extract(data).to_string();
                }
            } else {
                log_error("too many values for input label")
                  .arg("count", data.size());
            }
        } else if(path.ends_with("type")) {
            if(data.has_single_value()) {
                if(!extract(data).empty()) {
                    _type = extract(data).to_string();
                }
                if(is_parsing_slot()) {
                    add_slot_mapping();
                }
            }
        } else if(path.ends_with("_")) {
            const auto parent{path.parent()};
            if(parent.ends_with("input")) {
                if(parse_msg_id(data, _input_id)) {
                    if(parent.size() == 2) {
                        _status_l1 = status_type_l1::parsing_input;
                    } else {
                        if(is_parsing_slot()) {
                            add_slot_mapping();
                        }
                    }
                }
            } else if(parent.ends_with("slot")) {
                if(parse_msg_id(data, _slot_id)) {
                    if(parent.size() == 2) {
                        _status_l1 = status_type_l1::parsing_slot;
                    }
                }
            }
        }
    }

    void add_object(const basic_string_path& path) noexcept final {
        _str_data_offs = 0;
        if(path.is("_")) {
            _status_l1 = status_type_l1::unknown;
            _input_id = {};
            _slot_id = {};
            _type = {};
            _label = {};
            _min = {};
            _max = {};
            _initial_float = {};
        }
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.is("_")) {
            if(is_parsing_input()) {
                add_input();
            }
        }
    }

    void finish() noexcept final {
        _ctx.switch_input_mapping();
        base::finish();
    }

private:
    enum class status_type_l1 { unknown, parsing_input, parsing_slot };

    execution_context& _ctx;
    std::optional<message_id> _input_id;
    std::optional<message_id> _slot_id;
    std::optional<std::string> _type;
    std::optional<std::string> _label;
    std::optional<float> _min, _max, _initial_float;
    std::optional<bool> _initial_bool;
    identifier _temp_id;
    span_size_t _str_data_offs{0};

    status_type_l1 _status_l1{status_type_l1::unknown};
};
//------------------------------------------------------------------------------
auto make_valtree_input_setup_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  execution_context& ctx) noexcept -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_orbiting_input_setup_builder>(parent, ctx);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
