/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_app.hpp>
import eagine.core;
import eagine.shapes;
import eagine.msgbus;
//------------------------------------------------------------------------------
struct test_resource_manager_1 : eagitest::app_case {
    using launcher = eagitest::launcher<test_resource_manager_1>;

    test_resource_manager_1(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 1, "1"}
      , manager{context().resources()} {
        manager.add_parameters("TestText", {eagine::url{"txt:///TestText"}});
        too_long.reset();
    }

    auto is_done() noexcept -> bool final {
        return too_long or res.is_loaded();
    }

    void clean_up() noexcept final {
        check(res.is_loaded(), "text is loaded");
        bool content_is_ok{true};
        if(auto text{res.ref()}) {
            content_is_ok =
              text->starts_with("Lorem ipsum dolor sit amet") and
              text->ends_with("deserunt mollit anim id est laborum.");
        } else {
            content_is_ok = false;
        }
        check(content_is_ok, "content is ok");
    }

    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::resource_manager& manager;
    eagine::app::managed_resource<eagine::app::exp::plain_text_resource> res{
      manager,
      "TestText"};
};
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    enable_message_bus(ctx);
    ctx.preinitialize();

    eagitest::app_suite test{ctx, "resource manager", 1};
    test.once<test_resource_manager_1>();
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_app.hpp>
