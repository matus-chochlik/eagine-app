/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_STATE_VIEW_HPP
#define EAGINE_APP_STATE_VIEW_HPP

#include <eagine/quantities.hpp>
#include <eagine/value_with_history.hpp>
#include <chrono>

namespace eagine::app {
//------------------------------------------------------------------------------
/// @brief Alias for application state value with history.
/// @ingroup application
/// @see state_variable
/// @see context_state_view
template <typename T>
using state_value = value_with_history<T, 3>;

/// @brief Alias for application state variable with history.
/// @ingroup application
/// @see state_value
/// @see context_state_view
template <typename T>
using state_variable = variable_with_history<T, 3>;
//------------------------------------------------------------------------------
/// @brief Read-only view of application context state values.
/// @ingroup application
class context_state_view {
public:
    /// @brief The clock type for run-time duration measurements.
    using clock_type = std::chrono::steady_clock;

    /// @brief Returns the application run time duration.
    auto run_time() const noexcept -> clock_type::duration {
        return clock_type::now() - _start_time;
    }

    /// @brief Returns for how long was the user idle.
    /// @see user_is_idle
    /// @see user_became_active
    /// @see user_idle_too_long
    auto user_idle_time() const noexcept -> clock_type::duration {
        return clock_type::now() - _user_active_time;
    }

    /// @brief Returns the simulation time of the current frame.
    /// @see frame_duration
    auto frame_time() const noexcept -> seconds_t<float> {
        return seconds_(_frame_time.value());
    }

    /// @brief Returns the duration of the previous frame.
    /// @see frame_time
    /// @see frames_per_second
    auto frame_duration() const noexcept -> seconds_t<float> {
        return seconds_(_frame_time.delta());
    }

    /// @brief Return the number of frames per second.
    /// @see frame_time
    /// @see frame_duration
    auto frames_per_second() const noexcept -> float {
        const auto dur{frame_duration().value()};
        if(dur > 0.F) {
            return 1.F / dur;
        }
        return 0.F;
    }

    /// @brief Indicates if the example or the user is active in some way.
    /// @see user_idle_time
    auto is_active() const noexcept -> bool {
        return !_new_user_idle || _has_activity;
    }

    /// @brief Indicates if the user is idle (does not generate input events).
    /// @see user_became_active
    /// @see user_idle_time
    /// @see user_idle_too_long
    /// @see is_active
    auto user_is_idle() const noexcept -> value_with_history<bool, 2> {
        return {_new_user_idle, _old_user_idle};
    }

    /// @brief Indicates that the used became idle.
    /// @see user_is_idle
    /// @see user_became_active
    auto user_became_idle() const noexcept -> bool {
        return user_is_idle().delta() > 0;
    }

    /// @brief Indicates that the used became active (generated input events).
    /// @see user_is_idle
    /// @see user_became_idle
    auto user_became_active() const noexcept -> bool {
        return user_is_idle().delta() < 0;
    }

    /// @brief Indicates that the user became idle for too long (may depend on config).
    auto user_idle_too_long() const noexcept -> bool {
        return user_idle_time() > _user_idle_interval;
    }

protected:
    const clock_type::time_point _start_time{clock_type::now()};
    clock_type::time_point _user_active_time{clock_type::now()};
    clock_type::duration _user_idle_interval{std::chrono::seconds{1}};
    state_variable<float> _frame_time{0.0F};

    bool _has_activity{false};
    bool _old_user_idle{false};
    bool _new_user_idle{false};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
