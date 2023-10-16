/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.msgbus;
import eagine.app;
import std;

#include "driver.hpp"

namespace eagine {
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    signal_switch interrupted;
    const auto& log = ctx.log();

    enable_message_bus(ctx);

    log.info("resource provider starting up");

    ctx.system().preinitialize();

    msgbus::router_address address{ctx};
    msgbus::connection_setup conn_setup(ctx);

    msgbus::endpoint bus{main_ctx_object{"ResSvrEndp", ctx}};

    app::resource_provider_driver driver{ctx};
    msgbus::resource_data_server_node resource_server{bus, driver};
    conn_setup.setup_connectors(resource_server, address);

    auto idle_too_long =
      ctx.config()
        .get<std::chrono::seconds>("app.resource_provider.max_idle_time")
        .and_then([&](auto max_idle_time) {
            return std::optional<timeout>{{max_idle_time}};
        });

    const auto is_done{[&] {
        return interrupted or resource_server.is_done() or
               (idle_too_long and idle_too_long->is_expired());
    }};

    const auto handle_work_done{[&](work_done wd) -> std::chrono::microseconds {
        if(wd and idle_too_long) {
            idle_too_long->reset();
        }
        return wd ? std::chrono::microseconds{25}
                  : std::chrono::microseconds{1000};
    }};

    auto& wd = ctx.watchdog();
    wd.declare_initialized();
    while(not is_done()) {
        wd.notify_alive();
        std::this_thread::sleep_for(handle_work_done(
          resource_server.update_message_age().update_and_process_all()));
    }
    wd.announce_shutdown();
    return 0;
}
//------------------------------------------------------------------------------
} // namespace eagine
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    eagine::main_ctx_options options{.app_id = "ResrcPrvdr"};
    return eagine::main_impl(argc, argv, options, eagine::main);
}

