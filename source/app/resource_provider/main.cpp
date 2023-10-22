/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.sslplus;
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

    std::optional<msgbus::router> router(ctx);
    if(ctx.config().is_set("app.resource_provider.with_router")) {
        log.info("starting with message bus router");

        router.emplace(ctx);
        router->add_ca_certificate_pem(ca_certificate_pem(ctx));
        router->add_certificate_pem(msgbus::router_certificate_pem(ctx));
        msgbus::setup_acceptors(ctx, *router);
    }

    msgbus::endpoint consumer_bus{main_ctx_object{"ResConEndp", ctx}};
    msgbus::resource_data_consumer_node resource_consumer{consumer_bus};
    conn_setup.setup_connectors(resource_consumer, address);

    msgbus::endpoint provider_bus{main_ctx_object{"ResProEndp", ctx}};
    app::resource_provider_driver driver{ctx, resource_consumer};
    msgbus::resource_data_server_node resource_provider{provider_bus, driver};
    conn_setup.setup_connectors(resource_provider, address);

    const bool only_if_busy{
      ctx.config().is_set("app.resource_provider.only_if_busy")};

    auto idle_too_long{
      ctx.config()
        .get<std::chrono::seconds>("app.resource_provider.max_idle_time")
        .and_then([&](auto max_idle_time) {
            return std::optional<timeout>{{max_idle_time}};
        })};

    const auto is_done{[&] {
        return interrupted or resource_provider.is_done() or
               (idle_too_long and idle_too_long->is_expired());
    }};

    auto& wd = ctx.watchdog();
    wd.declare_initialized();

    while(not is_done()) {
        some_true something_done;
        wd.notify_alive();

        resource_provider.update_message_age();
        something_done(resource_provider.update_and_process_all());
        something_done(resource_consumer.update_and_process_all());
        if(router) {
            something_done(router->update(8));
        }
        if(something_done) {
            if(not only_if_busy or resource_provider.has_pending_blobs()) {
                idle_too_long->reset();
            }
            std::this_thread::yield();
        } else {
            std::this_thread::sleep_for(std::chrono::microseconds{1000});
        }
    }
    if(router) {
        router->finish();
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
//------------------------------------------------------------------------------
