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

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.identifier;
import eagine.core.reflection;
import eagine.core.value_tree;
import eagine.core.units;

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

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    void do_add(const basic_string_path& path, span<const float> data) noexcept;

    auto finish() noexcept -> bool final;

private:
    std::vector<float> _values;
};
//------------------------------------------------------------------------------
template <std::integral T>
void valtree_float_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    if(path.size() == 2) {
        if(not data.empty()) {
            if((path.starts_with("values")) or (path.starts_with("data"))) {
                for(const auto v : data) {
                    _values.push_back(float(v));
                }
            }
        }
    } else if(path.size() == 1) {
        if(data.has_single_value()) {
            if((path.starts_with("count")) or (path.starts_with("size"))) {
                _values.reserve(std_size(*data));
            }
        }
    }
}
//------------------------------------------------------------------------------
template <std::floating_point T>
void valtree_float_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    if(path.size() == 2) {
        if(not data.empty()) {
            if((path.starts_with("values")) and (path.starts_with("data"))) {
                for(const auto v : data) {
                    _values.push_back(float(v));
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void valtree_float_vector_builder::do_add(
  const basic_string_path& path,
  span<const float> data) noexcept {
    if(path.size() == 2) {
        if(not data.empty()) {
            if((path.starts_with("values")) and (path.starts_with("data"))) {
                _values.insert(_values.end(), data.begin(), data.end());
            }
        }
    }
}
//------------------------------------------------------------------------------
auto valtree_float_vector_builder::finish() noexcept -> bool {
    if(auto parent{_parent.lock()}) {
        if(const auto cont{parent->continuation()}) {
            cont->handle_float_vector(*parent, _values);
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto make_valtree_float_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_float_vector_builder>, parent};
}
//------------------------------------------------------------------------------
// valtree_vec3_vector_builder
//------------------------------------------------------------------------------
class valtree_vec3_vector_builder
  : public valtree_builder_base<valtree_vec3_vector_builder> {
    using base = valtree_builder_base<valtree_vec3_vector_builder>;

public:
    using base::base;

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    using base::do_add;

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        if(not _do_add(path, data)) {
            if((path.size() == 1) and data.has_single_value()) {
                if((path.starts_with("count")) or (path.starts_with("size"))) {
                    _values.reserve(std_size(*data));
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
            if((path.starts_with("values")) or (path.starts_with("data"))) {
                _values.push_back(_temp);
                _temp = {0.F, 0.F, 0.F};
            }
        }
    }

    auto finish() noexcept -> bool final {
        if(auto parent{_parent.lock()}) {
            parent->handle_vec3_vector(*parent, _values);
            return true;
        }
        return false;
    }

private:
    template <typename T>
    auto _do_add(const basic_string_path& path, span<const T> data) noexcept
      -> bool;

    math::vector<float, 3, true> _temp{0.F, 0.F, 0.F};
    std::vector<math::vector<float, 3, true>> _values;
    std::size_t _offs{0U};
};
//------------------------------------------------------------------------------
template <typename T>
auto valtree_vec3_vector_builder::_do_add(
  const basic_string_path& path,
  span<const T> data) noexcept -> bool {
    if(path.size() == 3) {
        if((path.starts_with("values")) or (path.starts_with("data"))) {
            if(data.has_single_value()) {
                if((path.ends_with("x")) or (path.ends_with("r"))) {
                    _temp._v[0] = *data;
                    return true;
                } else if((path.ends_with("y")) or (path.ends_with("g"))) {
                    _temp._v[1] = *data;
                    return true;
                } else if((path.ends_with("z")) or (path.ends_with("b"))) {
                    _temp._v[2] = *data;
                    return true;
                }
            }
            if(path.ends_with("_")) {
                for(const auto i : index_range(data)) {
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
//------------------------------------------------------------------------------
auto make_valtree_vec3_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_vec3_vector_builder>, parent};
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

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    using base::do_add;

    template <typename T>
    void parse_param(const basic_string_path& path, span<const T> data) noexcept;

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
template <typename T>
void valtree_orbiting_camera_parameters_builder::parse_param(
  const basic_string_path& path,
  span<const T> data) noexcept {
    if((path.size() == 1) and data.has_single_value()) {
        const auto value{*data};
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
//------------------------------------------------------------------------------
auto make_valtree_camera_parameters_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  orbiting_camera& camera) noexcept -> unique_holder<valtree::object_builder> {
    return {hold<valtree_orbiting_camera_parameters_builder>, parent, camera};
}
//------------------------------------------------------------------------------
// input parameters
//------------------------------------------------------------------------------
class valtree_input_setup_builder
  : public valtree_builder_base<valtree_input_setup_builder> {
    using base = valtree_builder_base<valtree_input_setup_builder>;

public:
    valtree_input_setup_builder(
      const std::shared_ptr<pending_resource_info>& parent,
      execution_context& ctx) noexcept
      : base{parent}
      , _ctx{ctx} {}

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    using base::do_add;

    auto parse_msg_id(
      const span<const string_view> data,
      std::optional<message_id>& dest) noexcept -> bool;

    auto is_parsing_feedback() const noexcept -> bool {
        return _status_l1 == status_type_l1::parsing_feedback;
    }

    auto is_parsing_input() const noexcept -> bool {
        return _status_l1 == status_type_l1::parsing_input;
    }

    auto is_parsing_slot() const noexcept -> bool {
        return _status_l1 == status_type_l1::parsing_slot;
    }

    auto add_feedback() noexcept -> bool;

    auto add_input() noexcept -> bool;

    auto add_slot_mapping() noexcept -> bool;

    void do_add(
      const basic_string_path& path,
      const span<const bool> data) noexcept;

    template <std::floating_point T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept;

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept;

    void add_object(const basic_string_path& path) noexcept final;

    void finish_object(const basic_string_path& path) noexcept final;

    auto finish() noexcept -> bool final {
        _ctx.switch_input_mapping();
        return base::finish();
    }

    void reset() noexcept;

private:
    enum class status_type_l1 {
        unknown,
        parsing_input,
        parsing_slot,
        parsing_feedback
    };

    execution_context& _ctx;
    std::optional<message_id> _feedback_id;
    std::optional<message_id> _input_id;
    std::optional<message_id> _slot_id;
    std::optional<identifier> _device_id;
    std::optional<std::string> _type;
    std::optional<std::string> _label;
    std::variant<std::monostate, bool, float> _threshold;
    std::variant<std::monostate, bool, float> _constant;
    std::optional<float> _min, _max, _initial_float;
    std::optional<bool> _initial_bool;
    std::optional<input_feedback_trigger> _trigger;
    std::optional<input_feedback_action> _action;
    identifier _mapping_id;
    identifier _temp_id;
    span_size_t _str_data_offs{0};

    status_type_l1 _status_l1{status_type_l1::unknown};
};
//------------------------------------------------------------------------------
auto valtree_input_setup_builder::parse_msg_id(
  const span<const string_view> data,
  std::optional<message_id>& dest) noexcept -> bool {
    if(data.size() == 2) {
        if(identifier::can_be_encoded(data.front())) {
            if(identifier::can_be_encoded(data.back())) {
                dest =
                  message_id{identifier{data.front()}, identifier{data.back()}};
                _str_data_offs = 0;
                return true;
            } else {
                log_error("invalid method identifier ${data}")
                  .arg("data", *data);
            }
        } else {
            log_error("invalid class identifier ${data}").arg("data", *data);
        }
    } else if(data.has_single_value()) {
        if(_str_data_offs == 0) {
            if(identifier::can_be_encoded(*data)) {
                _temp_id = identifier{*data};
            } else {
                log_error("invalid class identifier ${data}").arg("data", *data);
            }
        } else if(_str_data_offs == 1) {
            if(identifier::can_be_encoded(*data)) {
                dest = message_id{_temp_id, identifier{*data}};
                _str_data_offs = 0;
                return true;
            } else {
                log_error("invalid method identifier ${data}")
                  .arg("data", *data);
            }
        } else {
            log_error("too many values for message id");
        }
    }
    _str_data_offs += data.size();
    return false;
}
//------------------------------------------------------------------------------
auto valtree_input_setup_builder::add_feedback() noexcept -> bool {
    assert(is_parsing_feedback());
    if(_device_id and _feedback_id and _input_id) {
        _ctx.add_ui_feedback(
          _mapping_id,
          *_device_id,
          *_feedback_id,
          *_input_id,
          _trigger.value_or(input_feedback_trigger::change),
          _action.value_or(input_feedback_action::copy),
          _threshold,
          _constant);
        reset();
    }
    return false;
}
//------------------------------------------------------------------------------
auto valtree_input_setup_builder::add_input() noexcept -> bool {
    assert(is_parsing_input());
    if(_input_id and _type and _label) {
        if(_type == "ui_button") {
            _ctx.add_ui_button(*_input_id, *_label);
            _status_l1 = status_type_l1::unknown;
        } else if(_type == "ui_toggle") {
            _ctx.add_ui_toggle(
              *_input_id, *_label, _initial_bool.value_or(false));
            _status_l1 = status_type_l1::unknown;
        } else if(_type == "ui_slider") {
            _ctx.add_ui_slider(
              *_input_id,
              *_label,
              _min.value_or(0.F),
              _max.value_or(1.F),
              _initial_float.value_or(0.5F));
            _status_l1 = status_type_l1::unknown;
        } else {
            log_error("invalid input type '${type}")
              .arg("signal", *_input_id)
              .arg("type", *_type);
        }
        reset();
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
auto valtree_input_setup_builder::add_slot_mapping() noexcept -> bool {
    assert(is_parsing_slot());
    if(_slot_id) {
        if(_device_id and _input_id and _type) {
            input_setup setup;
            if(_type == "trigger") {
                setup.trigger();
            } else if(_type == "relative") {
                setup.relative();
            } else if(_type == "absolute_free") {
                setup.absolute_free();
            } else if(_type == "absolute_norm") {
                setup.absolute_norm();
            }
            _ctx.map_input(
              _mapping_id, *_slot_id, *_device_id, *_input_id, setup);
            _input_id = {};
            _type = {};
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void valtree_input_setup_builder::do_add(
  const basic_string_path& path,
  const span<const bool> data) noexcept {
    if(data.has_single_value()) {
        if(path.ends_with("initial")) {
            _initial_bool = *data;
        } else if(path.ends_with("threshold")) {
            _threshold = *data;
        } else if(path.ends_with("constant")) {
            _constant = *data;
        }
    }
}
//------------------------------------------------------------------------------
template <std::floating_point T>
void valtree_input_setup_builder::do_add(
  const basic_string_path& path,
  const span<const T> data) noexcept {
    if(data.has_single_value()) {
        if(path.ends_with("initial")) {
            _initial_float = *data;
        } else if(path.ends_with("min")) {
            _min = *data;
        } else if(path.ends_with("max")) {
            _max = *data;
        } else if(path.ends_with("threshold")) {
            _threshold = float(*data);
        } else if(path.ends_with("constant")) {
            _constant = float(*data);
        }
    } else {
        log_error("too many values for ${what}").arg("what", path.back());
    }
}
//------------------------------------------------------------------------------
void valtree_input_setup_builder::do_add(
  const basic_string_path& path,
  const span<const string_view> data) noexcept {
    if(path.ends_with("device")) {
        if(data.has_single_value()) {
            if(not data->empty() and identifier::can_be_encoded(*data)) {
                _device_id = identifier{*data};
            }
        } else {
            log_error("too many values for device id").arg("count", data.size());
        }
    } else if(path.ends_with("label")) {
        if(data.has_single_value()) {
            if(not data->empty()) {
                _label = data->to_string();
            }
        } else {
            log_error("too many values for input label")
              .arg("count", data.size());
        }
    } else if(path.ends_with("type")) {
        if(data.has_single_value()) {
            if(not data->empty()) {
                _type = data->to_string();
            }
            if(is_parsing_slot()) {
                add_slot_mapping();
            }
        }
    } else if(path.ends_with("trigger")) {
        if(data.has_single_value()) {
            if(const auto trigger{from_string<input_feedback_trigger>(*data)}) {
                _trigger = *trigger;
            }
        }
    } else if(path.ends_with("action")) {
        if(data.has_single_value()) {
            if(const auto action{from_string<input_feedback_action>(*data)}) {
                _action = *action;
            }
        }
    } else if(path.ends_with("_")) {
        const auto parent{path.parent()};
        if(parent.ends_with("input")) {
            if(parse_msg_id(data, _input_id)) {
                if(parent.size() == 3) {
                    if(not is_parsing_feedback()) {
                        _status_l1 = status_type_l1::parsing_input;
                    }
                } else {
                    if(is_parsing_slot()) {
                        add_slot_mapping();
                    }
                }
            }
        } else if(parent.ends_with("slot")) {
            if(parse_msg_id(data, _slot_id)) {
                if(parent.size() == 3) {
                    _status_l1 = status_type_l1::parsing_slot;
                }
            }
        } else if(parent.ends_with("feedback")) {
            if(parse_msg_id(data, _feedback_id)) {
                if(parent.size() == 3) {
                    _status_l1 = status_type_l1::parsing_feedback;
                    if(not _device_id) {
                        if(_feedback_id->has_class("Key")) {
                            _device_id = {"Keyboard"};
                        }
                    }
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void valtree_input_setup_builder::add_object(
  const basic_string_path& path) noexcept {
    _str_data_offs = 0;
    if(path.size() == 1) {
        reset();
        _mapping_id = {};
        const auto entry{path.front()};
        if(not entry.empty() and (entry != "_") and (entry != "default")) {
            if(identifier::can_be_encoded(entry)) {
                _mapping_id = identifier{entry};
            } else {
                log_error("invalid input mapping identifier ${id}")
                  .arg("id", entry);
            }
        }
        log_info("loading input mapping ${id}")
          .tag("loadInpMap")
          .arg("id", _mapping_id);
    }
}
//------------------------------------------------------------------------------
void valtree_input_setup_builder::finish_object(
  const basic_string_path& path) noexcept {
    if(path.size() == 2) {
        if(is_parsing_input()) {
            add_input();
        } else if(is_parsing_feedback()) {
            add_feedback();
        }
    }
}
//------------------------------------------------------------------------------
void valtree_input_setup_builder::reset() noexcept {
    _status_l1 = status_type_l1::unknown;
    _feedback_id = {};
    _input_id = {};
    _slot_id = {};
    _type = {};
    _label = {};
    _threshold = {};
    _constant = {};
    _min = {};
    _max = {};
    _initial_float = {};
    _initial_bool = {};
    _trigger = {};
    _action = {};
}
//------------------------------------------------------------------------------
auto make_valtree_input_setup_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  execution_context& ctx) noexcept -> unique_holder<valtree::object_builder> {
    return {hold<valtree_input_setup_builder>, parent, ctx};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
