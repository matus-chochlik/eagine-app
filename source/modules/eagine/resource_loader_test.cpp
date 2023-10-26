/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_app.hpp>
import eagine.core;
import eagine.shapes;
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
// string list
//------------------------------------------------------------------------------
struct test_request_string_list : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_string_list>;

    test_request_string_list(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 2, "string list"}
      , strings{eagine::url{"txt:///TestText"}, ec} {
        strings.loaded.connect(
          make_callable_ref<&test_request_string_list::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::string_list_resource::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("txt") and
                        info.base.locator.has_path("/TestText");
        if(info.base.strings.size() == 6 and strings.size() == 6) {
            content_is_ok = true;
            content_is_ok =
              content_is_ok and (strings[0] ==
                                 "Lorem ipsum dolor sit amet, consectetur "
                                 "adipiscing elit, sed do eiusmod tempor");
            content_is_ok =
              content_is_ok and (strings[1] ==
                                 "incididunt ut labore et dolore magna aliqua. "
                                 "Ut enim ad minim veniam, quis");
            content_is_ok =
              content_is_ok and (strings[2] ==
                                 "nostrud exercitation ullamco laboris nisi ut "
                                 "aliquip ex ea commodo consequat.");
            content_is_ok =
              content_is_ok and (strings[3] ==
                                 "Duis aute irure dolor in reprehenderit in "
                                 "voluptate velit esse cillum dolore eu");
            content_is_ok =
              content_is_ok and (strings[4] ==
                                 "fugiat nulla pariatur. Excepteur sint "
                                 "occaecat cupidatat non proident, sunt");
            content_is_ok =
              content_is_ok and (strings[5] ==
                                 "in culpa qui officia deserunt mollit anim "
                                 "id est laborum.");
        }
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not strings) {
            strings.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(strings.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        strings.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::string_list_resource strings;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// float vector
//------------------------------------------------------------------------------
struct test_request_float_vector : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_float_vector>;

    test_request_float_vector(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 3, "float vector"}
      , ints{eagine::url{"json:///TestInts"}, ec} {
        ints.loaded.connect(
          make_callable_ref<&test_request_float_vector::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::float_vector_resource::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestInts");
        if(info.base.values.size() == 7 and ints.size() == 7) {
            content_is_ok = true;
            content_is_ok = content_is_ok and int(info.base.values[0]) == 1;
            content_is_ok = content_is_ok and int(ints[1]) == 2;
            content_is_ok = content_is_ok and int(ints[2]) == 3;
            content_is_ok = content_is_ok and int(ints[3]) == 5;
            content_is_ok = content_is_ok and int(ints[4]) == 8;
            content_is_ok = content_is_ok and int(ints[5]) == 13;
            content_is_ok = content_is_ok and int(info.base.values[6]) == 21;
        }
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not ints) {
            ints.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(ints.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        ints.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::float_vector_resource ints;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// vec3 vector
//------------------------------------------------------------------------------
struct test_request_vec3_vector : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_vec3_vector>;

    test_request_vec3_vector(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 4, "vec3 vector"}
      , vecs{eagine::url{"json:///TestVec3"}, ec} {
        vecs.loaded.connect(
          make_callable_ref<&test_request_vec3_vector::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::vec3_vector_resource::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestVec3");
        if(info.base.values.size() == 4 and vecs.size() == 4) {
            using eagine::are_equal;
            content_is_ok = true;
            content_is_ok = content_is_ok and are_equal(vecs[0].x(), 1.F);
            content_is_ok = content_is_ok and are_equal(vecs[0].y(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs[0].z(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs[1].x(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs[1].y(), 2.F);
            content_is_ok = content_is_ok and are_equal(vecs[1].z(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs[2].x(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs[2].y(), 0.F);
            content_is_ok = content_is_ok and are_equal(vecs[2].z(), 3.F);
            content_is_ok = content_is_ok and are_equal(vecs[3].x(), 4.F);
            content_is_ok = content_is_ok and are_equal(vecs[3].y(), 5.F);
            content_is_ok = content_is_ok and are_equal(vecs[3].z(), 6.F);
        }
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not vecs) {
            vecs.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(vecs.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        vecs.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::vec3_vector_resource vecs;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// shape generator
//------------------------------------------------------------------------------
struct test_request_shape_generator : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_shape_generator>;

    test_request_shape_generator(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 5, "shape generator"}
      , shape{eagine::url{"json:///TestMesh"}, ec} {
        shape.loaded.connect(
          make_callable_ref<&test_request_shape_generator::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::shape_generator_resource::load_info& info) noexcept {
        using eagine::shapes::vertex_attrib_kind;

        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestMesh");
        content_is_ok = true;
        content_is_ok = content_is_ok and (shape->vertex_count() == 4);
        content_is_ok = content_is_ok and (shape->index_count() == 12);
        content_is_ok =
          content_is_ok and
          (shape->index_type() == eagine::shapes::index_data_type::unsigned_16);
        content_is_ok =
          content_is_ok and (shape->has_variant(vertex_attrib_kind::position));
        content_is_ok = content_is_ok and
                        (shape->has_variant({vertex_attrib_kind::color, 0}));
        content_is_ok = content_is_ok and
                        (shape->has_variant({vertex_attrib_kind::color, 1}));
        content_is_ok =
          content_is_ok and (shape->has_variant(vertex_attrib_kind::occlusion));
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not shape) {
            shape.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(shape.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        shape.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::shape_generator_resource shape;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::app_suite test{ctx, "resource loader", 5};
    test.once<test_request_plain_text>();
    test.once<test_request_string_list>();
    test.once<test_request_float_vector>();
    test.once<test_request_vec3_vector>();
    test.once<test_request_shape_generator>();
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_app.hpp>
