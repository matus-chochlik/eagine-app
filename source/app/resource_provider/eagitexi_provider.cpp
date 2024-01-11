/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider:eagitexi_provider;

import eagine.core;
import std;
import :driver;
import :providers;

namespace eagine::app {
//------------------------------------------------------------------------------
struct pixel_provider_coordinate {
    int width{1};
    int height{1};
    int depth{1};
    int x{0};
    int y{0};
    int z{0};
    int component{0};
};
//------------------------------------------------------------------------------
struct pixel_provider_interface : interface<pixel_provider_interface> {
    virtual auto pixel_byte_count() noexcept -> int = 0;

    virtual auto estimated_data_size(int w, int h, int d) noexcept
      -> span_size_t = 0;

    virtual auto pixel_byte(pixel_provider_coordinate) noexcept -> byte = 0;
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
auto provider_eagitexi_3d_r8(
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

