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
        if(info.base.strings.size() == 60 and strings.size() == 60) {
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
// URL list
//------------------------------------------------------------------------------
struct test_request_url_list : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_url_list>;

    test_request_url_list(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 3, "URL list"}
      , urls{eagine::url{"txt:///TestURLs"}, ec} {
        urls.loaded.connect(
          make_callable_ref<&test_request_url_list::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::url_list_resource::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("txt") and
                        info.base.locator.has_path("/TestURLs");
        if(info.base.values.size() == 4 and urls.size() == 4) {
            content_is_ok = true;
            content_is_ok = content_is_ok and urls[0].has_scheme("file") and
                            urls[0].has_path("/proc/cpuinfo");
            content_is_ok = content_is_ok and urls[1].has_scheme("file") and
                            urls[1].has_path("/etc/hosts");
            content_is_ok = content_is_ok and urls[2].has_scheme("ftp") and
                            urls[2].has_domain("example.com") and
                            urls[2].has_path("/file.txt");
            content_is_ok = content_is_ok and urls[3].has_scheme("https") and
                            urls[3].has_domain("oglplus.org") and
                            urls[3].has_path("/");
        }
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not urls) {
            urls.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(urls.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        urls.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::url_list_resource urls;
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
      : eagitest::app_case{s, ec, 4, "float vector"}
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
      : eagitest::app_case{s, ec, 5, "vec3 vector"}
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
// smooth path
//------------------------------------------------------------------------------
struct test_request_smooth_vec3_curve : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_smooth_vec3_curve>;

    test_request_smooth_vec3_curve(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 6, "smooth vec3 curve"}
      , path{eagine::url{"json:///TestPath"}, ec} {
        path.loaded.connect(
          make_callable_ref<&test_request_smooth_vec3_curve::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::smooth_vec3_curve_resource::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestPath");
        if(info.base.curve.control_points().size() == 13) {
            using eagine::are_equal;
            content_is_ok = true;
            const auto& cps{path.control_points()};
            content_is_ok = content_is_ok and cps.size() == 13;
        }
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not path) {
            path.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(path.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        path.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::smooth_vec3_curve_resource path;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// mat4 vector
//------------------------------------------------------------------------------
struct test_request_mat4_vector : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_mat4_vector>;

    test_request_mat4_vector(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 7, "mat4 vector"}
      , mats{eagine::url{"json:///TestMat4"}, ec} {
        mats.loaded.connect(
          make_callable_ref<&test_request_mat4_vector::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::mat4_vector_resource::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestMat4");
        if(info.base.values.size() == 3 and mats.size() == 3) {
            using eagine::are_equal;
            using eagine::math::get_rm;
            content_is_ok = true;
            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 0>(mats[0]), 1.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 1>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 2>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 3>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 0>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 1>(mats[0]), 2.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 2>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 3>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 0>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 1>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 2>(mats[0]), 3.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 3>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 0>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 1>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 2>(mats[0]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 3>(mats[0]), 4.F);

            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 0>(mats[1]), 5.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 1>(mats[1]), 6.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 2>(mats[1]), 7.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 3>(mats[1]), 8.F);

            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 0>(mats[2]), 0.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 1>(mats[2]), 1.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 2>(mats[2]), 2.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<0, 3>(mats[2]), 3.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 0>(mats[2]), 4.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 1>(mats[2]), 5.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 2>(mats[2]), 6.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<1, 3>(mats[2]), 7.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 0>(mats[2]), 8.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 1>(mats[2]), 9.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 2>(mats[2]), 10.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<2, 3>(mats[2]), 11.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 0>(mats[2]), 12.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 1>(mats[2]), 13.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 2>(mats[2]), 14.F);
            content_is_ok =
              content_is_ok and are_equal(get_rm<3, 3>(mats[2]), 15.F);
        }
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not mats) {
            mats.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(mats.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        mats.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::mat4_vector_resource mats;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// value tree
//------------------------------------------------------------------------------
struct test_request_value_tree : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_value_tree>;

    test_request_value_tree(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 8, "value tree"}
      , tree{eagine::url{"json:///TestMesh"}, ec} {
        tree.loaded.connect(
          make_callable_ref<&test_request_value_tree::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::value_tree_resource::load_info& info) noexcept {
        using eagine::shapes::vertex_attrib_kind;

        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestMesh");
        content_is_ok = true;
        content_is_ok = content_is_ok and (tree.get<int>("vertex_count") == 4);
        content_is_ok = content_is_ok and
                        (tree.get<std::string>("index_type") == "unsigned_16");
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not tree) {
            tree.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(tree.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        tree.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::value_tree_resource tree;
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
      : eagitest::app_case{s, ec, 9, "shape generator"}
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
// mapped struct
//------------------------------------------------------------------------------
namespace eagine {
//------------------------------------------------------------------------------
struct test_point {
    int x{}, y{}, z{};
};

template <identifier_t Id>
constexpr auto data_member_mapping(
  const std::type_identity<test_point>,
  const selector<Id>) noexcept {
    return make_data_member_mapping<test_point, int, int, int>(
      {"x", &test_point::x}, {"y", &test_point::y}, {"z", &test_point::z});
}
//------------------------------------------------------------------------------
struct test_triangle {
    test_point a{}, b{}, c{};
};

template <identifier_t Id>
constexpr auto data_member_mapping(
  const std::type_identity<test_triangle>,
  const selector<Id>) noexcept {
    return make_data_member_mapping<
      test_triangle,
      test_point,
      test_point,
      test_point>(
      {"a", &test_triangle::a},
      {"b", &test_triangle::b},
      {"c", &test_triangle::c});
}
//------------------------------------------------------------------------------
struct test_tetrahedron {
    test_triangle base{};
    test_point apex{};
};

template <identifier_t Id>
constexpr auto data_member_mapping(
  const std::type_identity<test_tetrahedron>,
  const selector<Id>) noexcept {
    return make_data_member_mapping<test_tetrahedron, test_triangle, test_point>(
      {"base", &test_tetrahedron::base}, {"apex", &test_tetrahedron::apex});
}
} // namespace eagine
//------------------------------------------------------------------------------
struct test_request_mapped_struct : eagitest::app_case {
    using launcher = eagitest::launcher<test_request_mapped_struct>;

    test_request_mapped_struct(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 10, "mapped struct"}
      , object{eagine::url{"json:///TestThdn"}, ec} {
        object.loaded.connect(
          make_callable_ref<&test_request_mapped_struct::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::loaded_resource<eagine::test_tetrahedron>::load_info&
        info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestThdn");

        using eagine::are_equal;
        content_is_ok = true;
        content_is_ok = content_is_ok and are_equal(object.apex.x, 2);
        content_is_ok = content_is_ok and are_equal(object.apex.y, 3);
        content_is_ok = content_is_ok and are_equal(object.apex.z, 4);
        content_is_ok = content_is_ok and are_equal(object.base.a.x, 1);
        content_is_ok = content_is_ok and are_equal(object.base.a.y, 0);
        content_is_ok = content_is_ok and are_equal(object.base.a.z, 0);
        content_is_ok = content_is_ok and are_equal(object.base.b.x, 0);
        content_is_ok = content_is_ok and are_equal(object.base.b.y, 1);
        content_is_ok = content_is_ok and are_equal(object.base.b.z, 0);
        content_is_ok = content_is_ok and are_equal(object.base.c.x, 0);
        content_is_ok = content_is_ok and are_equal(object.base.c.y, 0);
        content_is_ok = content_is_ok and are_equal(object.base.c.z, 1);
    }

    auto is_loaded() const noexcept {
        return load_signal_received and locator_is_ok and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        if(not object) {
            object.load_if_needed(context());
        }
    }

    void clean_up() noexcept final {
        check(object.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        object.clean_up(context());
    }

    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::loaded_resource<eagine::test_tetrahedron> object;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
auto test_main(eagine::test_ctx& ctx) -> int {
    eagitest::app_suite test{ctx, "resource loader", 10};
    test.once<test_request_plain_text>();
    test.once<test_request_string_list>();
    test.once<test_request_url_list>();
    test.once<test_request_float_vector>();
    test.once<test_request_vec3_vector>();
    test.once<test_request_smooth_vec3_curve>();
    test.once<test_request_mat4_vector>();
    test.once<test_request_value_tree>();
    test.once<test_request_shape_generator>();
    test.once<test_request_mapped_struct>();
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_app.hpp>
