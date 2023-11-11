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
auto provider_eagitexi_random(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_checks_r8(main_ctx_parent parent)
  -> unique_holder<resource_provider_interface>;

auto provider_eagitexi_2d_stripes_r8(main_ctx_parent parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitex_2d_single_rgb8(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
auto provider_eagitexi_2d_single_rgb8(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitex_sphere_volume(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
auto provider_eagitexi_sphere_volume(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_text_lorem_ipsum(main_ctx_parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
