/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "eagitex_provider.hpp"
#include "eagitexi_provider.hpp"
#include "providers.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
struct single_rgb8_pixel_provider : pixel_provider_interface {
    std::array<byte, 3> rgb;

    single_rgb8_pixel_provider(int r, int g, int b) noexcept
      : rgb{{limit_cast<byte>(r), limit_cast<byte>(g), limit_cast<byte>(b)}} {}

    auto pixel_byte_count() noexcept -> int final {
        return 3;
    }

    auto estimated_data_size(int, int, int) noexcept -> span_size_t final {
        return 1024;
    }

    auto pixel_byte(pixel_provider_coordinate c) noexcept -> byte final {
        assert(c.component >= 0 and c.component <= 3);
        return rgb[std_size(c.component)];
    }
};
//------------------------------------------------------------------------------
struct single_rgb8_pixel_provider_factory : pixel_provider_factory_interface {
    static auto valid_clr(int c) noexcept -> bool {
        return (c >= 0) and (c <= 255);
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        const auto& q{locator.query()};
        return valid_clr(q.arg_value_as<int>("r").value_or(0)) and
               valid_clr(q.arg_value_as<int>("g").value_or(0)) and
               valid_clr(q.arg_value_as<int>("b").value_or(0));
    }

    auto make_provider(const url& locator)
      -> unique_holder<pixel_provider_interface> final {
        const auto& q{locator.query()};
        return {
          hold<single_rgb8_pixel_provider>,
          q.arg_value_as<int>("r").value_or(1),
          q.arg_value_as<int>("g").value_or(1),
          q.arg_value_as<int>("b").value_or(1)};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_single_rgb8(main_ctx_parent parent)
  -> unique_holder<resource_provider_interface> {
    return provider_eagitexi_2d_rgb8(
      parent, "/2d_single_rgb8", {hold<single_rgb8_pixel_provider_factory>});
}
//------------------------------------------------------------------------------
auto provider_eagitex_2d_single_rgb8(main_ctx_parent parent)
  -> unique_holder<resource_provider_interface> {
    return provider_eagitex_2d_square_rgb8ub(parent, "/2d_square_single_rgb8");
}
//------------------------------------------------------------------------------
} // namespace eagine::app
