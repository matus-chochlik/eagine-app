/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.math;
import eagine.core.units;
import eagine.core.valid_if;

namespace eagine::app {
//------------------------------------------------------------------------------
auto orbiting_camera::update_orbit(const float inc) noexcept
  -> orbiting_camera& {
    _orbit_factor += (inc * _orbit_dir);
    if(_orbit_factor > 1.F) {
        _orbit_factor = 1.F;
        _orbit_dir.flip();
    }
    if(_orbit_factor < 0.F) {
        _orbit_factor = 0.F;
        _orbit_dir.flip();
    }
    _changed = true;
    return *this;
}
//------------------------------------------------------------------------------
auto orbiting_camera::update_turns(const float inc) noexcept
  -> orbiting_camera& {
    _turns += turns_(inc * _turn_dir);
    _changed = true;
    return *this;
}
//------------------------------------------------------------------------------
auto orbiting_camera::update_pitch(const float inc) noexcept
  -> orbiting_camera& {
    _pitch += right_angles_(inc * _pitch_dir);
    if(_pitch > _pitch_max) {
        _pitch = _pitch_max;
        _pitch_dir.flip();
    }
    if(_pitch < _pitch_min) {
        _pitch = _pitch_min;
        _pitch_dir.flip();
    }
    _changed = true;
    return *this;
}
//------------------------------------------------------------------------------
auto orbiting_camera::idle_update(
  const context_state_view& state,
  const valid_if_positive<float>& divisor) noexcept -> orbiting_camera& {
    const auto s = state.frame_duration().value() / divisor.value_or(1.F);
    return update_orbit(s).update_turns(s).update_pitch(s);
}
//------------------------------------------------------------------------------
auto orbiting_camera::idle_update(
  const execution_context& ec,
  const valid_if_positive<float>& divisor) noexcept -> orbiting_camera& {
    return idle_update(ec.state(), divisor);
}
//------------------------------------------------------------------------------
void orbiting_camera::_handle_pressure(const input& i) noexcept {
    _is_dragging = bool(i);
    _changed = true;
}
//------------------------------------------------------------------------------
void orbiting_camera::_handle_dampening(const input& i) noexcept {
    _dampen_motion = bool(i);
    _changed = true;
}
//------------------------------------------------------------------------------
void orbiting_camera::_change_altitude(const input& i) noexcept {
    _orbit_factor -= (float(i.get() * _motion_adjust()));
    if(_orbit_factor > 1.F) {
        _orbit_factor = 1.F;
    }
    if(_orbit_factor < 0.F) {
        _orbit_factor = 0.F;
    }
    _changed = true;
}
//------------------------------------------------------------------------------
void orbiting_camera::_change_longitude(const input& i) noexcept {
    _turns -= turns_(float(i.get() * 0.25 * _motion_adjust()));
    _changed = true;
}
//------------------------------------------------------------------------------
void orbiting_camera::_change_latitude(const input& i) noexcept {
    const auto max = right_angles_(1.F);
    _pitch -= right_angles_(float(i.get() * _motion_adjust()));
    if(_pitch > max) {
        _pitch = max;
    }
    if(_pitch < -max) {
        _pitch = -max;
    }
    _changed = true;
}
//------------------------------------------------------------------------------
auto orbiting_camera::connect_inputs(execution_context& ec)
  -> orbiting_camera& {
    ec.connect_input(pressure_input())
      .connect_input(dampening_input())
      .connect_input(altitude_change_input())
      .connect_input(longitude_change_input())
      .connect_input(latitude_change_input());
    return *this;
}
//------------------------------------------------------------------------------
auto orbiting_camera::basic_input_mapping(
  execution_context& ec,
  const identifier mapping_id) -> orbiting_camera& {
    ec.map_cursor_pressure(mapping_id, pressure_input_id())
      .map_key(mapping_id, dampening_input_id(), {"LeftCtrl"})
      .map_key(
        mapping_id,
        altitude_change_input_id(),
        {"KpPlus"},
        input_setup().trigger().multiply(0.10F))
      .map_key(
        mapping_id,
        altitude_change_input_id(),
        {"RtBracket"},
        input_setup().trigger().multiply(0.05F))
      .map_key(
        mapping_id,
        altitude_change_input_id(),
        {"KpMinus"},
        input_setup().trigger().multiply(0.10F).invert())
      .map_key(
        mapping_id,
        altitude_change_input_id(),
        {"LtBracket"},
        input_setup().trigger().multiply(0.05F).invert())
      .map_wheel_scroll_y(
        mapping_id,
        altitude_change_input_id(),
        input_setup().relative().multiply(0.2F))
      .map_key(
        mapping_id,
        longitude_change_input_id(),
        {"Left"},
        input_setup().trigger().multiply(0.10F))
      .map_key(
        mapping_id,
        longitude_change_input_id(),
        {"A"},
        input_setup().trigger().multiply(0.05F))
      .map_key(
        mapping_id,
        longitude_change_input_id(),
        {"Right"},
        input_setup().trigger().multiply(0.10F).invert())
      .map_key(
        mapping_id,
        longitude_change_input_id(),
        {"D"},
        input_setup().trigger().multiply(0.05F).invert())
      .map_cursor_motion_x(
        mapping_id,
        longitude_change_input_id(),
        input_setup().relative().multiply(2.F).only_if(_is_dragging))
      .map_key(
        mapping_id,
        latitude_change_input_id(),
        {"Down"},
        input_setup().trigger().multiply(0.10F))
      .map_key(
        mapping_id,
        latitude_change_input_id(),
        {"S"},
        input_setup().trigger().multiply(0.05F))
      .map_key(
        mapping_id,
        latitude_change_input_id(),
        {"Up"},
        input_setup().trigger().multiply(0.10F).invert())
      .map_key(
        mapping_id,
        latitude_change_input_id(),
        {"W"},
        input_setup().trigger().multiply(0.05F).invert())
      .map_cursor_motion_y(
        mapping_id,
        latitude_change_input_id(),
        input_setup().relative().multiply(2.F).only_if(_is_dragging));
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
