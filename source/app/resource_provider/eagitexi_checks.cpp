/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "eagitexi_provider.hpp"
#include "providers.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
struct checks_r8_pixel_provider : pixel_provider_interface {
    checks_r8_pixel_provider(int s) noexcept
      : size{s} {
        assert(s > 0);
    }

    auto pixel_byte_count() noexcept -> int final {
        return 1;
    }

    auto estimated_data_size(int, int, int) noexcept -> span_size_t final {
        return 1024;
    }

    auto pixel_byte(int x, int y, int z, int, int, int, int c) noexcept
      -> byte final {
        assert(c == 0);
        return (((x / size) + (y / size) + (z / size)) % 2 == 0) ? 0x00U
                                                                 : 0xFFU;
    }

    int size;
};
//------------------------------------------------------------------------------
struct checks_r8_pixel_provider_factory : pixel_provider_factory_interface {
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
          hold<checks_r8_pixel_provider>,
          q.arg_value_as<int>("size").value_or(8)};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_checks_r8(main_ctx_parent parent)
  -> unique_holder<resource_provider_interface> {
    return provider_eagitexi_2d_r8(
      parent, "/2d_checks_r8", {hold<checks_r8_pixel_provider_factory>});
}
//------------------------------------------------------------------------------
} // namespace eagine::app
