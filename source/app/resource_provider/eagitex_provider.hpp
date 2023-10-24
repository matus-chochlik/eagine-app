/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_RESOURCE_PROVIDER_EAGITEX_PROVIDER_HPP
#define EAGINE_RESOURCE_PROVIDER_EAGITEX_PROVIDER_HPP

import eagine.core;
import std;

#include "driver.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
auto provider_eagitex_2d_square_rgb8ub(main_ctx_parent, std::string path)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
