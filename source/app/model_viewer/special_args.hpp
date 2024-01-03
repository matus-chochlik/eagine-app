/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_RESOURCE_PROVIDER_SPECIAL_ARGS_HPP
#define EAGINE_RESOURCE_PROVIDER_SPECIAL_ARGS_HPP

namespace eagine::app {
//------------------------------------------------------------------------------
auto handle_special_args(main_ctx& ctx) -> std::optional<int>;
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
