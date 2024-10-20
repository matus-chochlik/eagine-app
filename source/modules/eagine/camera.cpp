/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:camera;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.math;
import eagine.core.units;
import eagine.core.utility;
import eagine.core.valid_if;
import eagine.oglplus;
import :types;
import :state;
import :input;
import :context;

namespace eagine::app {
//------------------------------------------------------------------------------
/// @brief Extension of orbiting camera wrapper.
/// @ingroup application
/// @see camera_transforms
export class orbiting_camera : public oglplus::orbiting_camera {

public:
    using base = oglplus::orbiting_camera;
    using base::matrix;

    /// @brief Get matrix using a reference to a video context.
    [[nodiscard]] auto matrix(video_context& vc) const noexcept {
        return base::matrix(vc.surface_aspect());
    }

    /// @brief Inddicates if the camera has changed and resets the flag.
    /// @see mark_changed
    [[nodiscard]] auto has_changed() noexcept {
        return std::exchange(_changed, false);
    }

    /// @brief Inddicates that the camera has changed.
    /// @see has_changed
    auto mark_changed() noexcept -> orbiting_camera& {
        _changed = true;
        return *this;
    }

    /// @brief Does a generic orbit update with given increment.
    auto update_orbit(const float inc) noexcept -> orbiting_camera&;

    /// @brief Does a generic azimuth update with given increment.
    auto update_turns(const float inc) noexcept -> orbiting_camera&;

    /// @brief Does a generic elevation update with given increment.
    auto update_pitch(const float inc) noexcept -> orbiting_camera&;

    /// @brief Sets the maximum value for the pitch (elevation) angle.
    /// @see set_pitch_min
    auto set_pitch_max(const radians_t<float> max) noexcept -> auto& {
        _pitch_max = max;
        return *this;
    }

    /// @brief Sets the minimum value for the pitch (elevation) angle.
    /// @see set_pitch_max
    auto set_pitch_min(const radians_t<float> min) noexcept -> auto& {
        _pitch_min = min;
        return *this;
    }

    /// @brief Does a generic combined update when the user does not provide input.
    /// @see update_orbit
    /// @see update_turns
    /// @see update_pitch
    auto idle_update(
      const context_state_view&,
      const valid_if_positive<float>& divisor = 2.F) noexcept
      -> orbiting_camera&;

    /// @brief Does a generic combined update when the user does not provide input.
    /// @see update_orbit
    /// @see update_turns
    /// @see update_pitch
    auto idle_update(
      const execution_context&,
      const valid_if_positive<float>& divisor = 2.F) noexcept
      -> orbiting_camera&;

    constexpr auto pressure_input_id() const noexcept -> message_id {
        return {"Camera", "Pressure"};
    }

    /// @brief Returns the input slot for handling cursor pressure input signals.
    /// This can be bound for example to mouse button press input signal.
    /// @see connect_inputs
    /// @see basic_input_mapping
    [[nodiscard]] auto pressure_input() noexcept -> input_slot {
        return {
          pressure_input_id(),
          make_callable_ref<&orbiting_camera::_handle_pressure>(this)};
    }

    [[nodiscard]] constexpr auto dampening_input_id() const noexcept
      -> message_id {
        return {"Camera", "Dampening"};
    }

    /// @brief Returns the input slot for handling motion dampening input signals.
    /// This can be bound for example to control or shift key press input signal.
    /// @see connect_inputs
    /// @see basic_input_mapping
    [[nodiscard]] auto dampening_input() noexcept -> input_slot {
        return {
          dampening_input_id(),
          make_callable_ref<&orbiting_camera::_handle_dampening>(this)};
    }

    [[nodiscard]] constexpr auto altitude_change_input_id() const noexcept
      -> message_id {
        return {"Camera", "Altitude"};
    }

    /// @brief Returns the input slot for handling orbit change input signals.
    /// This can be bound for example to mouse wheel scroll input signal.
    /// @see connect_inputs
    /// @see basic_input_mapping
    [[nodiscard]] auto altitude_change_input() noexcept -> input_slot {
        return {
          altitude_change_input_id(),
          make_callable_ref<&orbiting_camera::_change_altitude>(this)};
    }

    [[nodiscard]] constexpr auto longitude_change_input_id() const noexcept
      -> message_id {
        return {"Camera", "Longitude"};
    }

    /// @brief Returns the input slot for handling azimuth change input signals.
    /// This can be bound for example to left/right arrow key press input signals.
    /// @see connect_inputs
    /// @see basic_input_mapping
    [[nodiscard]] auto longitude_change_input() noexcept -> input_slot {
        return {
          longitude_change_input_id(),
          make_callable_ref<&orbiting_camera::_change_longitude>(this)};
    }

    [[nodiscard]] constexpr auto latitude_change_input_id() const noexcept
      -> message_id {
        return {"Camera", "Latitude"};
    }

    /// @brief Returns the input slot for handling elevation change input signals.
    /// This can be bound for example to up/down arrow key press input signals.
    /// @see connect_inputs
    /// @see basic_input_mapping
    [[nodiscard]] auto latitude_change_input() noexcept -> input_slot {
        return {
          latitude_change_input_id(),
          make_callable_ref<&orbiting_camera::_change_latitude>(this)};
    }

    /// @brief Connects the camera input slots to the execution context.
    /// @see basic_input_mapping
    auto connect_inputs(execution_context& ec) -> orbiting_camera&;

    /// @brief Specifies a named key binding for the camera input slots.
    /// @see connect_inputs
    auto basic_input_mapping(execution_context& ec, const identifier mapping_id)
      -> orbiting_camera&;

    /// @brief Specifies the default key binding for the camera input slots.
    /// @see connect_inputs
    auto basic_input_mapping(execution_context& ec) -> auto& {
        return basic_input_mapping(ec, {});
    }

private:
    radians_t<float> _pitch_max{radians_(1.F)};
    radians_t<float> _pitch_min{radians_(-1.F)};
    oglplus::sign _orbit_dir;
    oglplus::sign _turn_dir;
    oglplus::sign _pitch_dir;

    bool _changed{true};
    bool _is_dragging{false};
    bool _dampen_motion{false};

    auto _motion_adjust() const noexcept {
        return _dampen_motion ? 0.2 : 1.0;
    }

    void _handle_pressure(const input& i) noexcept;
    void _handle_dampening(const input& i) noexcept;
    void _change_altitude(const input& i) noexcept;
    void _change_longitude(const input& i) noexcept;
    void _change_latitude(const input& i) noexcept;
};
//------------------------------------------------------------------------------
/// @brief Class doing various transforms using camera matrix and video context.
/// @ingroup application
/// @see orbiting_camera
export class camera_transforms {
public:
    ///@brief Construction from reference to camera and a video context.
    constexpr camera_transforms(
      orbiting_camera& camera,
      video_context& video) noexcept
      : _video{video}
      , _camera_matrix{camera.matrix(video)} {}

    /// @brief Returns the underlying camera matrix.
    [[nodiscard]] constexpr auto matrix() const noexcept -> const mat4& {
        return _camera_matrix;
    }

    /// @brief Returns the underlying camera matrix.
    /// @see matrix
    operator const mat4&() const noexcept {
        return _camera_matrix;
    }

    /// @brief Multiples the given homogeneous vector with the camera matrix and
    /// does perspective correction.
    [[nodiscard]] auto world_to_ndc(const vec4& v) noexcept -> vec3 {
        auto temp{multiply(_camera_matrix, v)};
        temp = temp / temp.w();
        return vec3{temp};
    }

    /// @brief Converts the given vector in world coordinates to NDC.
    [[nodiscard]] auto world_position_to_ndc(const vec3& p) noexcept {
        return world_to_ndc(vec4{p, 1.F});
    }

    /// @brief Converts NDC to screen coordinates.
    [[nodiscard]] auto ndc_to_screen(const vec3& ndc, const vec2& scr) noexcept {
        vec2 temp{ndc};
        temp = temp * 0.5F;
        temp = temp + vec2(0.5F);
        temp = temp * scr;
        temp = vec2{temp.x(), scr.y() - temp.y()};
        return temp;
    }

    /// @brief Converts NDC to screen coordinates.
    [[nodiscard]] auto ndc_to_screen(const vec3& ndc) noexcept {
        return ndc_to_screen(ndc, _video.surface_size_vec2());
    }

    /// @brief Converts the given vector in world coordinates to screen coordinates.
    [[nodiscard]] auto world_position_to_screen(const vec3& p) noexcept {
        return ndc_to_screen(world_position_to_ndc(p));
    }

private:
    video_context& _video;
    const mat4 _camera_matrix;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

