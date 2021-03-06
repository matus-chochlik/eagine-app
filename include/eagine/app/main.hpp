/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_MAIN_HPP
#define EAGINE_APP_MAIN_HPP

#include "context.hpp"
#include <eagine/main_ctx.hpp>

namespace eagine {

auto main(main_ctx& ctx) -> int {
    return app::execution_context(ctx)
      .prepare(app::establish(ctx))
      .run()
      .result();
}

} // namespace eagine

#ifndef EAGINE_IMPLEMENTING_CORE_LIBRARY
auto main(int argc, const char** argv) -> int {
    return eagine::default_main(argc, argv, eagine::main);
}
#endif

#endif
