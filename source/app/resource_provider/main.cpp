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
class resource_provider : public main_ctx_object {
public:
    resource_provider(main_ctx_parent parent) noexcept;

    auto is_done() noexcept -> bool;

    void update();

    void finish();

private:
    msgbus::endpoint _consumer_bus{"ResConEndp", as_parent()};
    msgbus::endpoint _provider_bus{"ResProEndp", as_parent()};

    msgbus::resource_data_consumer_node _resource_consumer{_consumer_bus};
    app::resource_provider_driver _driver{as_parent(), _resource_consumer};
    msgbus::resource_data_server_node _resource_server{_provider_bus, _driver};

    std::optional<msgbus::router> _router;
    std::optional<timeout> _idle_too_long;
    bool _only_if_busy;
};
//------------------------------------------------------------------------------
resource_provider::resource_provider(main_ctx_parent parent) noexcept
  : main_ctx_object{"ResrcPrvdr", parent}
  , _idle_too_long{app_config()
                     .get<std::chrono::seconds>(
                       "app.resource_provider.max_idle_time")
                     .and_then(
                       [&](auto max_idle_time) -> std::optional<timeout> {
                           return {{max_idle_time}};
                       })}
  , _only_if_busy{app_config().is_set("app.resource_provider.only_if_busy")} {

    auto& ctx{main_context()};

    msgbus::router_address address{ctx};
    msgbus::connection_setup conn_setup(ctx);

    conn_setup.setup_connectors(_resource_server, address);
    conn_setup.setup_connectors(_resource_consumer, address);

    if(app_config().is_set("app.resource_provider.with_router")) {
        log_info("starting with message bus router");

        _router.emplace(ctx);
        _router->add_ca_certificate_pem(ca_certificate_pem(ctx));
        _router->add_certificate_pem(msgbus::router_certificate_pem(ctx));
        msgbus::setup_acceptors(ctx, *_router);
    }
}
//------------------------------------------------------------------------------
auto resource_provider::is_done() noexcept -> bool {
    return _resource_server.is_done() or
           (_idle_too_long and _idle_too_long->is_expired());
}
//------------------------------------------------------------------------------
void resource_provider::update() {
    _resource_server.update_message_age();
    some_true something_done{_resource_server.update_and_process_all()};
    something_done(_resource_consumer.update_and_process_all());
    if(_router) {
        something_done(_router->update(8));
    }
    if(something_done) {
        if(not _only_if_busy or _resource_server.has_pending_blobs()) {
            _idle_too_long->reset();
        }
        std::this_thread::yield();
    } else {
        std::this_thread::sleep_for(std::chrono::microseconds{1000});
    }
}
//------------------------------------------------------------------------------
void resource_provider::finish() {
    if(_router) {
        _router->finish();
    }
}
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    signal_switch interrupted;
    const auto& log = ctx.log();

    enable_message_bus(ctx);
    log.info("resource provider starting up");
    ctx.system().preinitialize();

    resource_provider provider{ctx};

    const auto is_done{[&] {
        return interrupted or provider.is_done();
    }};

    auto& wd = ctx.watchdog();
    wd.declare_initialized();

    while(not is_done()) {
        wd.notify_alive();
        provider.update();
    }
    provider.finish();
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
