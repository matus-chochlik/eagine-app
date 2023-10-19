/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "providers.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
struct lorem_ipsum_io final : msgbus::source_blob_io {

    static constexpr auto lorem_ipsum() noexcept -> string_view {
        return {
          "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
          "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
          "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
          "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor "
          "in reprehenderit in voluptate velit esse cillum dolore eu fugiat "
          "nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
          "sunt in culpa qui officia deserunt mollit anim id est laborum."};
    }

    auto total_size() noexcept -> span_size_t final {
        return lorem_ipsum().size();
    }

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final {
        const auto li{lorem_ipsum()};
        const auto sz{li.size() - offs};
        dst = head(dst, sz);
        return copy(as_bytes(head(skip(li, offs), dst.size())), dst).size();
    }
};
//------------------------------------------------------------------------------
struct lorem_ipsum_provider final : resource_provider_interface {
    auto has_resource(const url& locator) noexcept -> bool final {
        return locator.has_path("/lorem_ipsum");
    }

    auto get_resource_io(const url&)
      -> unique_holder<msgbus::source_blob_io> final {
        return {hold<lorem_ipsum_io>};
    }
};
//------------------------------------------------------------------------------
auto provider_text_lorem_ipsum(main_ctx_parent)
  -> unique_holder<resource_provider_interface> {
    return {hold<lorem_ipsum_provider>};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
