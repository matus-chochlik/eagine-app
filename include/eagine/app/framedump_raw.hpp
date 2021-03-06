/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_FRAMEDUMP_RAW_HPP
#define EAGINE_APP_FRAMEDUMP_RAW_HPP

#include "config/basic.hpp"
#include "interface.hpp"
#include <eagine/main_ctx_fwd.hpp>
#include <memory>

namespace eagine::app {

auto make_raw_framedump(main_ctx_parent) -> std::shared_ptr<framedump>;

} // namespace eagine::app

#if !EAGINE_APP_LIBRARY || defined(EAGINE_IMPLEMENTING_APP_LIBRARY)
#include <eagine/app/framedump_raw.inl>
#endif

#endif
