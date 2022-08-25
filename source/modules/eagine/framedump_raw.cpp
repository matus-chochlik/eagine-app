/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:framedump_raw;

import eagine.core.types;
import eagine.core.main_ctx;
import :interface;
import <memory>;

namespace eagine::app {

export auto make_raw_framedump(main_ctx_parent) -> std::shared_ptr<framedump>;

} // namespace eagine::app

