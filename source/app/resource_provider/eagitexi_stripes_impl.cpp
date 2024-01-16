/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
struct stripes_r8_pixel_provider : pixel_provider_interface {
    stripes_r8_pixel_provider(int s) noexcept
      : size{s} {
        assert(size > 0);
    }

    auto pixel_byte_count() noexcept -> int final {
        return 1;
    }

    auto estimated_data_size(int, int, int) noexcept -> span_size_t final {
        return 1024;
    }

    auto pixel_byte(pixel_provider_coordinate c) noexcept -> byte final {
        c.x /= size;
        c.y /= size;
        c.z /= size;
        assert(c.component == 0);
        return ((c.x + c.y + c.z) % 2 == 0) ? 0x00U : 0xFFU;
    }

    int size;
};
//------------------------------------------------------------------------------
struct stripes_r8_pixel_provider_factory : pixel_provider_factory_interface {
    static auto valid_size(int s) noexcept -> bool {
        return s > 0;
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        const auto& q{locator.query()};
        return valid_size(q.arg_value_as<int>("size").value_or(1));
    }

    auto make_provider(const url& locator)
      -> unique_holder<pixel_provider_interface> final {
        const auto& q{locator.query()};
        return {
          hold<stripes_r8_pixel_provider>,
          q.arg_value_as<int>("size").value_or(8)};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_stripes_r8(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return provider_eagitexi_2d_r8(
      p.parent, "/2d_stripes_r8", {hold<stripes_r8_pixel_provider_factory>});
}
//------------------------------------------------------------------------------
} // namespace eagine::app
