/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:main;

import eagine.core.main_ctx;
import :interface;
import :context;
import <memory>;

namespace eagine::app {

export auto default_main(main_ctx& ctx, std::unique_ptr<launchpad> launcher)
  -> int {
    return app::execution_context(ctx)
      .prepare(std::move(launcher))
      .run()
      .result();
}

} // namespace eagine::app

