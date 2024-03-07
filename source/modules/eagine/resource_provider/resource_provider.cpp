/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import eagine.app;
import std;

export import :external_apis;
export import :gl_context;
export import :driver;
export import :providers;
export import :common;
export import :eagitexi_provider;

namespace eagine::app {
//------------------------------------------------------------------------------
export class resource_provider : public main_ctx_object {
public:
    resource_provider(main_ctx_parent parent) noexcept;

    auto is_done() noexcept -> bool;

    auto update() -> work_done;

private:
    app::external_apis _apis{as_parent()};

    msgbus::endpoint _consumer_bus{"ResConEndp", as_parent()};
    msgbus::endpoint _provider_bus{"ResProEndp", as_parent()};

    app::resource_loader _resource_loader{_consumer_bus};
    app::resource_provider_driver _driver{as_parent(), _apis, _resource_loader};
    msgbus::resource_data_server_node _resource_server{_provider_bus, _driver};

    auto _get_timeout() noexcept -> std::optional<timeout>;

    std::optional<timeout> _idle_too_long;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
