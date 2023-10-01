/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_RESOURCE_PROVIDER_DRIVER_HPP
#define EAGINE_RESOURCE_PROVIDER_DRIVER_HPP

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
class resource_provider_driver final
  : public main_ctx_object
  , public msgbus::resource_server_driver {
public:
    resource_provider_driver(main_ctx_parent);

    auto has_resource(const url&) noexcept -> tribool final;

    auto get_resource_io(const identifier_t, const url&)
      -> unique_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const identifier_t, const span_size_t size) noexcept
      -> std::chrono::seconds final;

    auto get_blob_priority(
      const identifier_t,
      const msgbus::message_priority priority) noexcept
      -> msgbus::message_priority final;

private:
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
