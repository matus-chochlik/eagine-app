/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:main;

import std;
import eagine.core.types;
import eagine.core.main_ctx;
import :interface;
import :context;

namespace eagine::app {

export auto default_main(main_ctx& ctx, unique_holder<launchpad> launcher)
  -> int {
    return app::execution_context(ctx)
      .prepare(std::move(launcher))
      .run()
      .result();
}

} // namespace eagine::app

