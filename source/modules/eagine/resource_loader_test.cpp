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
// plain text
//------------------------------------------------------------------------------
struct test_request_plain_text : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_plain_text>;

    test_request_plain_text(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 1, "plain text"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_plain_text::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok =
          info.locator.has_scheme("txt") and info.locator.has_path("/TestText");
        content_is_ok = text->starts_with("Lorem ipsum dolor sit amet") and
                        text->ends_with("deserunt mollit anim id est laborum.");
    }

    auto is_done() noexcept -> bool final {
        return too_long or text.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(text, [] -> eagine::app::resource_request_params {
            return {eagine::url{"txt:///TestText"}};
        });
    }

    void clean_up() noexcept final {
        check(text.is_loaded(), "text is loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::plain_text_resource text;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// string list
//------------------------------------------------------------------------------
struct test_request_string_list : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_string_list>;

    test_request_string_list(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 2, "string list"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_string_list::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok =
          info.locator.has_scheme("txt") and info.locator.has_path("/TestText");

        if(strings->size() == 60) {
            content_is_ok = true;
            content_is_ok =
              content_is_ok and (strings.get()[0] ==
                                 "Lorem ipsum dolor sit amet, consectetur "
                                 "adipiscing elit, sed do eiusmod tempor");
            content_is_ok =
              content_is_ok and (strings.get()[1] ==
                                 "incididunt ut labore et dolore magna aliqua. "
                                 "Ut enim ad minim veniam, quis");
            content_is_ok =
              content_is_ok and (strings.get()[2] ==
                                 "nostrud exercitation ullamco laboris nisi ut "
                                 "aliquip ex ea commodo consequat.");
            content_is_ok =
              content_is_ok and (strings.get()[3] ==
                                 "Duis aute irure dolor in reprehenderit in "
                                 "voluptate velit esse cillum dolore eu");
            content_is_ok =
              content_is_ok and (strings.get()[4] ==
                                 "fugiat nulla pariatur. Excepteur sint "
                                 "occaecat cupidatat non proident, sunt");
            content_is_ok =
              content_is_ok and (strings.get()[5] ==
                                 "in culpa qui officia deserunt mollit anim "
                                 "id est laborum.");
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or strings.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(
          strings, [] -> eagine::app::resource_request_params {
              return {eagine::url{"txt:///TestText"}};
          });
    }

    void clean_up() noexcept final {
        check(strings.is_loaded(), "strings are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::string_list_resource strings;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// url list
//------------------------------------------------------------------------------
struct test_request_url_list : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_url_list>;

    test_request_url_list(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 3, "url list"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_url_list::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok =
          info.locator.has_scheme("txt") and info.locator.has_path("/TestURLs");

        if(urls->size() == 4) {
            content_is_ok = true;
            content_is_ok = content_is_ok and urls->at(0).has_scheme("file") and
                            urls->at(0).has_path("/proc/cpuinfo");
            content_is_ok = content_is_ok and urls->at(1).has_scheme("file") and
                            urls->at(1).has_path("/etc/hosts");
            content_is_ok = content_is_ok and urls->at(2).has_scheme("ftp") and
                            urls->at(2).has_domain("example.com") and
                            urls->at(2).has_path("/file.txt");
            content_is_ok = content_is_ok and
                            urls->at(3).has_scheme("https") and
                            urls->at(3).has_domain("oglplus.org") and
                            urls->at(3).has_path("/");
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or urls.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(urls, [] -> eagine::app::resource_request_params {
            return {eagine::url{"txt:///TestURLs"}};
        });
    }

    void clean_up() noexcept final {
        check(urls.is_loaded(), "URLs are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::url_list_resource urls;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// float list
//------------------------------------------------------------------------------
struct test_request_float_list : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_float_list>;

    test_request_float_list(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 4, "float list"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_float_list::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.locator.has_scheme("json") and
                        info.locator.has_path("/TestInts");

        if(ints->size() == 7) {
            content_is_ok = true;
            content_is_ok = content_is_ok and int(ints.get()[0]) == 1;
            content_is_ok = content_is_ok and int(ints.get()[1]) == 2;
            content_is_ok = content_is_ok and int(ints.get()[2]) == 3;
            content_is_ok = content_is_ok and int(ints.get()[3]) == 5;
            content_is_ok = content_is_ok and int(ints.get()[4]) == 8;
            content_is_ok = content_is_ok and int(ints.get()[5]) == 13;
            content_is_ok = content_is_ok and int(ints.get()[6]) == 21;
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or ints.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(ints, [] -> eagine::app::resource_request_params {
            return {eagine::url{"json:///TestInts"}};
        });
    }

    void clean_up() noexcept final {
        check(ints.is_loaded(), "ints are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::float_list_resource ints;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// vec3 list
//------------------------------------------------------------------------------
struct test_request_vec3_list : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_vec3_list>;

    test_request_vec3_list(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 5, "vec3 list"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_vec3_list::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.locator.has_scheme("json") and
                        info.locator.has_path("/TestVec3");

        if(vecs->size() == 4) {
            using eagine::are_equal;
            content_is_ok = true;
            content_is_ok = content_is_ok and are_equal(vecs.get()[0].x(), 1.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[0].y(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[0].z(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[1].x(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[1].y(), 2.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[1].z(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[2].x(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[2].y(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[2].z(), 3.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[3].x(), 4.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[3].y(), 5.F);
            content_is_ok = content_is_ok and are_equal(vecs.get()[3].z(), 6.F);
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or vecs.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(vecs, [] -> eagine::app::resource_request_params {
            return {eagine::url{"json:///TestVec3"}};
        });
    }

    void clean_up() noexcept final {
        check(vecs.is_loaded(), "vecs are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::vec3_list_resource vecs;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// mat4 list
//------------------------------------------------------------------------------
struct test_request_mat4_list : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_mat4_list>;

    test_request_mat4_list(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 6, "mat4 list"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_mat4_list::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.locator.has_scheme("json") and
                        info.locator.has_path("/TestMat4");

        if(mats->size() == 3) {
            using eagine::are_equal;
            content_is_ok = true;
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(0, 0), 1.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(0, 1), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(0, 2), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(0, 3), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(1, 0), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(1, 1), 2.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(1, 2), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(1, 3), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(2, 0), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(2, 1), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(2, 2), 3.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(2, 3), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(3, 0), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(3, 1), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(3, 2), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[0].get_rm(3, 3), 4.F);

            content_is_ok =
              content_is_ok and are_equal(mats.get()[1].get_rm(0, 0), 5.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[1].get_rm(1, 1), 6.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[1].get_rm(2, 2), 7.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[1].get_rm(3, 3), 8.F);

            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(0, 0), 0.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(0, 1), 1.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(0, 2), 2.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(0, 3), 3.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(1, 0), 4.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(1, 1), 5.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(1, 2), 6.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(1, 3), 7.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(2, 0), 8.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(2, 1), 9.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(2, 2), 10.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(2, 3), 11.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(3, 0), 12.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(3, 1), 13.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(3, 2), 14.F);
            content_is_ok =
              content_is_ok and are_equal(mats.get()[2].get_rm(3, 3), 15.F);
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or mats.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(mats, [] -> eagine::app::resource_request_params {
            return {eagine::url{"json:///TestMat4"}};
        });
    }

    void clean_up() noexcept final {
        check(mats.is_loaded(), "mats are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::mat4_list_resource mats;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    enable_message_bus(ctx);
    ctx.preinitialize();

    eagitest::app_suite test{ctx, "resource loader", 6};
    test.once<test_request_plain_text>();
    test.once<test_request_string_list>();
    test.once<test_request_url_list>();
    test.once<test_request_float_list>();
    test.once<test_request_vec3_list>();
    test.once<test_request_mat4_list>();
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_app.hpp>
