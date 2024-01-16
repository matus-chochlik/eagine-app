/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider:special_args;

import eagine.core;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
export auto handle_special_args(main_ctx& ctx) -> std::optional<int>;
//------------------------------------------------------------------------------
} // namespace eagine::app
