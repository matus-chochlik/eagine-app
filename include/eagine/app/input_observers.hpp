/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_INPUT_OBSERVERS_HPP
#define EAGINE_APP_INPUT_OBSERVERS_HPP

#include "input.hpp"
#include <eagine/timeout.hpp>
#include <utility>

namespace eagine::app {
//------------------------------------------------------------------------------
class trigger_released {
public:
    auto operator()(const input& i) noexcept -> trigger_released& {
        _was_released |= !i;
        return *this;
    }

    explicit operator bool() noexcept {
        return std::exchange(_was_released, false);
    }

private:
    bool _was_released{false};
};
//------------------------------------------------------------------------------
class button_tapped {
public:
    button_tapped() noexcept = default;

    button_tapped(std::chrono::duration<float> tap_timeout) noexcept
      : _tap_timeout{tap_timeout} {}

    auto operator()(const input& i) noexcept -> button_tapped& {
        if(!i && !_was_tapped) {
            if(!_tap_timeout) {
                _was_tapped |= true;
            }
            _tap_timeout.reset();
        }
        return *this;
    }

    explicit operator bool() noexcept {
        return std::exchange(_was_tapped, false);
    }

private:
    timeout _tap_timeout{std::chrono::milliseconds{250}};
    bool _was_tapped{false};
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
