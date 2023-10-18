/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_RESOURCE_PROVIDER_EAGITEXI_BASE_HPP
#define EAGINE_RESOURCE_PROVIDER_EAGITEXI_BASE_HPP

import eagine.core;
import std;

#include "driver.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
struct pixel_provider_interface : interface<pixel_provider_interface> {
    virtual auto pixel_byte_count() noexcept -> int = 0;

    virtual auto estimated_data_size(int w, int h, int d) noexcept
      -> span_size_t = 0;

    virtual auto pixel_byte(
      int x,
      int y,
      int z,
      int w,
      int h,
      int d,
      int c) noexcept -> byte = 0;
};
//------------------------------------------------------------------------------
struct pixel_provider_factory_interface
  : interface<pixel_provider_factory_interface> {
    virtual auto has_resource(const url&) noexcept -> bool = 0;

    virtual auto make_provider(const url&)
      -> unique_holder<pixel_provider_interface> = 0;
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_r8(
  main_ctx_parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface>)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_rgb8(
  main_ctx_parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface>)
  -> unique_holder<resource_provider_interface>;
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
