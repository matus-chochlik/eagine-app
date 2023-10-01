/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "driver.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
resource_provider_driver::resource_provider_driver(main_ctx_parent parent)
  : main_ctx_object{"RsrcPrDrvr", parent} {}
//------------------------------------------------------------------------------
auto resource_provider_driver::has_resource(const url&) noexcept -> tribool {
    return indeterminate;
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_resource_io(const identifier_t, const url&)
  -> unique_holder<msgbus::source_blob_io> {
    return {};
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_blob_timeout(
  const identifier_t,
  const span_size_t size) noexcept -> std::chrono::seconds {
    return std::chrono::seconds{size / 1024};
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_blob_priority(
  const identifier_t,
  const msgbus::message_priority priority) noexcept
  -> msgbus::message_priority {
    return priority;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
