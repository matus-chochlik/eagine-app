/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
import eagine.core;
import eagine.msgbus;
import eagine.guiplus;
import eagine.oglplus;
import eagine.app;
import eagine.app.model_viewer;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
auto handle_special_args(main_ctx& ctx) -> std::optional<int> {
    return handle_common_special_args(ctx);
}
//------------------------------------------------------------------------------
//  Main
//------------------------------------------------------------------------------
auto viewer_main(main_ctx& ctx) -> int {
    if(const auto exit_code{handle_special_args(ctx)}) {
        return *exit_code;
    }
    enable_message_bus(ctx);
    return default_main(ctx, establish(ctx));
}
//------------------------------------------------------------------------------
} // namespace eagine::app
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    eagine::main_ctx_options options{.app_id = "ModelViewr"};
    return eagine::main_impl(argc, argv, options, eagine::app::viewer_main);
}
//------------------------------------------------------------------------------
