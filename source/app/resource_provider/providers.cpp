/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider:providers;

import eagine.core;
import eagine.app;
import :driver;

namespace eagine::app {
//------------------------------------------------------------------------------
export class external_apis;
struct provider_parameters {
    main_ctx_parent parent;
    shared_provider_objects& shared;
};
//------------------------------------------------------------------------------
auto provider_zip_archive(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
auto provider_file(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_embedded(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_shape(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitexi_random(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitexi_tiling(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
auto provider_eagitexi_tiling_noise(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
auto provider_eagitexi_tiling_transition(const provider_parameters& p)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_checks_r8(const provider_parameters& parent)
  -> unique_holder<resource_provider_interface>;

auto provider_eagitexi_2d_stripes_r8(const provider_parameters& parent)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitex_2d_single_rgb8(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
auto provider_eagitexi_2d_single_rgb8(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitex_sphere_volume(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
auto provider_eagitexi_sphere_volume(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitexi_cubemap_blur(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_text_lorem_ipsum(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_text_resource_list(const provider_parameters&)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
} // namespace eagine::app

