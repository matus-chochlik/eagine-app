/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
auto handle_special_args(main_ctx& ctx) -> std::optional<int> {
    return handle_common_special_args(ctx);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
