/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.sslplus;
import eagine.msgbus;
import eagine.app;
import eagine.app.resource_provider;
import std;

namespace eagine {
namespace app {
//------------------------------------------------------------------------------
auto handle_special_args(main_ctx& ctx) -> std::optional<int> {
    return handle_common_special_args(ctx);
}
//------------------------------------------------------------------------------
} // namespace app
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto main(main_ctx& ctx) -> int {
    if(const auto exit_code{app::handle_special_args(ctx)}) {
        return *exit_code;
    }

    signal_switch interrupted;
    const auto& log = ctx.log();

    enable_message_bus(ctx);
    log.info("resource provider starting up");
    ctx.system().preinitialize();

    app::resource_provider provider{ctx};

    const auto is_done{[&] {
        return interrupted or provider.is_done();
    }};

    auto alive{ctx.watchdog().start_watch()};

    while(not is_done()) {
        alive.notify();
        provider.update();
    }
    provider.finish();

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
