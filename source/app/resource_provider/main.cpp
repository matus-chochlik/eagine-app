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

namespace eagine {

auto main(main_ctx& ctx) -> int {
    enable_message_bus(ctx);
    return 0;
}

} // namespace eagine

auto main(int argc, const char** argv) -> int {
    eagine::main_ctx_options options;
    options.app_id = "RsrcMgrExe";
    return eagine::main_impl(argc, argv, options, eagine::main);
}

