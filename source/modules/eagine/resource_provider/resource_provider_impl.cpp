/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

module eagine.app.resource_provider;

import eagine.core;
import eagine.sslplus;
import eagine.msgbus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
auto resource_provider::_get_timeout() noexcept -> std::optional<timeout> {
    if(app_config().is_set("application.resource_provider.only_if_busy")) {
        return app_config()
          .get<std::chrono::seconds>(
            "application.resource_provider.max_idle_time")
          .and_then([&](auto max_idle_time) -> std::optional<timeout> {
              return {{max_idle_time}};
          });
    }
    return {};
}
//------------------------------------------------------------------------------
resource_provider::resource_provider(main_ctx_parent parent) noexcept
  : main_ctx_object{"ResrcPrvdr", parent}
  , _idle_too_long{_get_timeout()} {

    auto& ctx{main_context()};

    msgbus::router_address address{ctx};
    msgbus::connection_setup conn_setup(ctx);

    conn_setup.setup_connectors(_resource_server, address);
    conn_setup.setup_connectors(_resource_loader, address);
}
//------------------------------------------------------------------------------
auto resource_provider::is_done() noexcept -> bool {
    return _resource_server.is_done() or
           (_idle_too_long and _idle_too_long->is_expired());
}
//------------------------------------------------------------------------------
auto resource_provider::update() -> work_done {
    _resource_server.update_message_age();
    some_true something_done{_resource_server.update_and_process_all()};
    something_done(_resource_loader.update_and_process_all());
    if(something_done) {
        if(_idle_too_long and _resource_server.has_pending_blobs()) {
            _idle_too_long->reset();
        }
    }
    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
