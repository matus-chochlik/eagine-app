/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_CAMERA_HPP
#define EAGINE_APP_CAMERA_HPP

#include "config/basic.hpp"
#include "context.hpp"
#include <eagine/oglplus/camera.hpp>
#include <eagine/oglplus/math/sign.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
/// @brief Extension of orbiting camera wrapper.
/// @ingroup application
class orbiting_camera : public oglplus::orbiting_camera {

public:
    using base = oglplus::orbiting_camera;
    using base::matrix;

    /// @brief Construction from a reference to a video context.
    auto matrix(video_context& vc) const noexcept {
        return base::matrix(vc.surface_aspect());
    }

    /// @brief Inddicates if the camera has changed and resets the flag.
    /// @see mark_changed
    auto has_changed() noexcept {
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
        return EAGINE_MSG_ID(Camera, Pressure);
    }

    /// @brief Returns the input slot for handling cursor pressure input signals.
    /// This can be bound for example to mouse button press input signal.
    /// @see connect_inputs
    /// @see basic_input_mapping
    auto pressure_input() noexcept -> input_slot {
        return {
          pressure_input_id(), EAGINE_THIS_MEM_FUNC_REF(_handle_pressure)};
    }

    constexpr auto dampening_input_id() const noexcept -> message_id {
        return EAGINE_MSG_ID(Camera, Dampening);
    }

    /// @brief Returns the input slot for handling motion dampening input signals.
    /// This can be bound for example to control or shift key press input signal.
    /// @see connect_inputs
    /// @see basic_input_mapping
    auto dampening_input() noexcept -> input_slot {
        return {
          dampening_input_id(), EAGINE_THIS_MEM_FUNC_REF(_handle_dampening)};
    }

    constexpr auto altitude_change_input_id() const noexcept -> message_id {
        return EAGINE_MSG_ID(Camera, Altitude);
    }

    /// @brief Returns the input slot for handling orbit change input signals.
    /// This can be bound for example to mouse wheel scroll input signal.
    /// @see connect_inputs
    /// @see basic_input_mapping
    auto altitude_change_input() noexcept -> input_slot {
        return {
          altitude_change_input_id(),
          EAGINE_THIS_MEM_FUNC_REF(_change_altitude)};
    }

    constexpr auto longitude_change_input_id() const noexcept -> message_id {
        return EAGINE_MSG_ID(Camera, Longitude);
    }

    /// @brief Returns the input slot for handling azimuth change input signals.
    /// This can be bound for example to left/right arrow key press input signals.
    /// @see connect_inputs
    /// @see basic_input_mapping
    auto longitude_change_input() noexcept -> input_slot {
        return {
          longitude_change_input_id(),
          EAGINE_THIS_MEM_FUNC_REF(_change_longitude)};
    }

    constexpr auto latitude_change_input_id() const noexcept -> message_id {
        return EAGINE_MSG_ID(Camera, Latitude);
    }

    /// @brief Returns the input slot for handling elevation change input signals.
    /// This can be bound for example to up/down arrow key press input signals.
    /// @see connect_inputs
    /// @see basic_input_mapping
    auto latitude_change_input() noexcept -> input_slot {
        return {
          latitude_change_input_id(),
          EAGINE_THIS_MEM_FUNC_REF(_change_latitude)};
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
} // namespace eagine::app

#if !EAGINE_APP_LIBRARY || defined(EAGINE_IMPLEMENTING_APP_LIBRARY)
#include <eagine/app/camera.inl>
#endif

#endif
