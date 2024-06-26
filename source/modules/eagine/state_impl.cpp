/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import std;
import eagine.core.types;
import eagine.core.valid_if;
import eagine.core.runtime;
import eagine.core.units;
import eagine.core.main_ctx;

namespace eagine::app {
//------------------------------------------------------------------------------
context_state::context_state(main_ctx_parent parent)
  : main_ctx_object{"AppliState", parent}
  , _fixed_fps{cfg_extr<valid_if_positive<float>>(
      "application.video.fixed_fps",
      0.F)}
  , _sim_activity_for{cfg_init(
      "application.simulate.activity_for",
      std::chrono::duration<float>{0.F})} {
    if(app_config().fetch(
         "application.user_idle_interval", _user_idle_interval)) {
        log_info("user idle interval set to ${interval}")
          .arg("interval", _user_idle_interval);
    }
    if(_fixed_fps) {
        log_info("running with fixed ${fps} frames per second")
          .arg("fps", *_fixed_fps);
    }
}
//------------------------------------------------------------------------------
auto context_state::update_activity() noexcept -> context_state& {
    if(not _new_user_idle) {
        _user_active_time = context_state_view::clock_type::now();
    }
    _has_activity = _sim_activity_for.count() > _frame_time.value();
    _old_user_idle = _new_user_idle;
    _new_user_idle = true;
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
