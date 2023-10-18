/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "providers.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
static inline auto get_default_blob_timeout(const span_size_t size) noexcept
  -> std::chrono::seconds {
    return std::max(
      std::chrono::seconds{size / 1024}, std::chrono::seconds{15});
}
//------------------------------------------------------------------------------
auto resource_provider_interface::get_blob_timeout(
  const span_size_t size) noexcept -> std::chrono::seconds {
    return get_default_blob_timeout(size);
}
//------------------------------------------------------------------------------
auto resource_provider_interface::get_blob_priority(
  const msgbus::message_priority priority) noexcept
  -> msgbus::message_priority {
    return priority;
}
//------------------------------------------------------------------------------
// resource_provider_driver
//------------------------------------------------------------------------------
void resource_provider_driver::_add(
  unique_holder<resource_provider_interface> provider) {
    assert(provider);
    _providers.emplace_back(std::move(provider));
}
//------------------------------------------------------------------------------
resource_provider_driver::resource_provider_driver(main_ctx_parent parent)
  : main_ctx_object{"RsrcPrDrvr", parent} {
    _add(provider_eagitexi_2d_single_rgb8(*this));
    _add(provider_text_lorem_ipsum(*this));
}
//------------------------------------------------------------------------------
auto resource_provider_driver::find_provider_of(const url& locator) noexcept
  -> optional_reference<resource_provider_interface> {
    for(const auto& provider : _providers) {
        if(provider->has_resource(locator)) {
            return provider;
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto resource_provider_driver::has_resource(const url& locator) noexcept
  -> tribool {
    if(find_provider_of(locator)) {
        return true;
    }
    return indeterminate;
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_resource_io(
  const identifier_t,
  const url& locator) -> unique_holder<msgbus::source_blob_io> {
    if(const auto provider{find_provider_of(locator)}) {
        return provider->get_resource_io(locator);
    }
    return {};
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_blob_timeout(
  const identifier_t,
  const url& locator,
  const span_size_t size) noexcept -> std::chrono::seconds {
    return find_provider_of(locator)
      .member(&resource_provider_interface::get_blob_timeout, size)
      .value_or(get_default_blob_timeout(size));
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_blob_priority(
  const identifier_t,
  const url& locator,
  const msgbus::message_priority priority) noexcept
  -> msgbus::message_priority {
    return find_provider_of(locator)
      .member(&resource_provider_interface::get_blob_priority, priority)
      .value_or(priority);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
