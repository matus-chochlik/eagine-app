/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider:driver;

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
export class external_apis;
export struct resource_provider_interface
  : abstract<resource_provider_interface> {
    virtual auto has_resource(const url&) noexcept -> bool = 0;

    virtual auto get_resource_io(const url&)
      -> unique_holder<msgbus::source_blob_io> = 0;

    virtual auto get_blob_timeout(const span_size_t size) noexcept
      -> std::chrono::seconds;

    virtual auto get_blob_priority(
      const msgbus::message_priority priority) noexcept
      -> msgbus::message_priority;

    virtual void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept = 0;
};
//------------------------------------------------------------------------------
export class resource_provider_driver final
  : public main_ctx_object
  , public msgbus::resource_server_driver {
public:
    resource_provider_driver(
      main_ctx_parent,
      external_apis&,
      msgbus::resource_data_consumer_node&);

    auto find_provider_of(const url&) noexcept
      -> optional_reference<resource_provider_interface>;

    auto has_resource(const url&) noexcept -> tribool final;

    auto get_resource_io(const endpoint_id_t, const url&)
      -> unique_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(
      const endpoint_id_t,
      const url&,
      const span_size_t size) noexcept -> std::chrono::seconds final;

    auto get_blob_priority(
      const endpoint_id_t,
      const url&,
      const msgbus::message_priority priority) noexcept
      -> msgbus::message_priority final;

    auto provider_count() const noexcept -> span_size_t;
    auto provider(span_size_t) const noexcept -> resource_provider_interface&;

private:
    void _add(unique_holder<resource_provider_interface>);
    void _populate(external_apis&, msgbus::resource_data_consumer_node&);

    std::vector<unique_holder<resource_provider_interface>> _providers;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

