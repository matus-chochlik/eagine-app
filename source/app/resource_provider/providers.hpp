/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_RESOURCE_PROVIDER_PROVIDERS_HPP
#define EAGINE_RESOURCE_PROVIDER_PROVIDERS_HPP

#include "driver.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_single_rgb(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_text_lorem_ipsum(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
