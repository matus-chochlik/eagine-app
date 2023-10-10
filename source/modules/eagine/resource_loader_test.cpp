/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_app.hpp>
import eagine.core;
//------------------------------------------------------------------------------
// plain text
//------------------------------------------------------------------------------
struct test_request_plain_text : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_plain_text>;

    test_request_plain_text(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 1, "plain text"}
      , text{eagine::url{"txt:///TestText"}, ec} {
        text.loaded.connect(
          make_callable_ref<&test_request_plain_text::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::plain_text_resource::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("txt") and
                        info.base.locator.has_path("/TestText");
        content_is_ok = info.base.text.starts_with("Lorem ipsum") and
                        info.base.text.ends_with("est laborum.") and
                        text.starts_with("Lorem ipsum dolor sit amet");
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not text) {
            text.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(text.is_loaded(), "text is loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        text.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::plain_text_resource text;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::app_suite test{ctx, "resource loader", 1};
    test.once<test_request_plain_text>();
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_app.hpp>
