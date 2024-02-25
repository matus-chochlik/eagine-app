/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/testing/unit_begin_app.hpp>
import eagine.core;
import eagine.msgbus;

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
//------------------------------------------------------------------------------
} // namespace eagine
//------------------------------------------------------------------------------
struct test_manage_mapped_struct_1 : eagitest::app_case {
    using launcher = eagitest::launcher<test_manage_mapped_struct_1>;

    test_manage_mapped_struct_1(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 1, "mapped struct is loaded"}
      , object{manager, ec, eagine::url{"json:///TestThdn1"}} {
        too_long.reset();
    }

    auto is_loaded() const noexcept {
        return object.is_loaded() and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        manager.update();
        if(object.is_loaded() and not content_is_ok) {
            using eagine::are_equal;
            content_is_ok = true;
            content_is_ok = content_is_ok and are_equal(object->apex.x, 2);
            content_is_ok = content_is_ok and are_equal(object->apex.y, 3);
            content_is_ok = content_is_ok and are_equal(object->apex.z, 4);
            content_is_ok = content_is_ok and are_equal(object->base.a.x, 1);
            content_is_ok = content_is_ok and are_equal(object->base.a.y, 0);
            content_is_ok = content_is_ok and are_equal(object->base.a.z, 0);
            content_is_ok = content_is_ok and are_equal(object->base.b.x, 0);
            content_is_ok = content_is_ok and are_equal(object->base.b.y, 1);
            content_is_ok = content_is_ok and are_equal(object->base.b.z, 0);
            content_is_ok = content_is_ok and are_equal(object->base.c.x, 0);
            content_is_ok = content_is_ok and are_equal(object->base.c.y, 0);
            content_is_ok = content_is_ok and are_equal(object->base.c.z, 1);
        }
    }

    void clean_up() noexcept final {
        check(object.is_loaded(), "values are loaded");
        check(content_is_ok, "content is ok");

        manager.clean_up();
    }

private:
    using resource_t = eagine::app::managed_resource<eagine::test_tetrahedron>;

    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::basic_resource_manager<resource_t> manager;
    resource_t object;
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
struct test_manage_mapped_struct_2 : eagitest::app_case {
    using launcher = eagitest::launcher<test_manage_mapped_struct_2>;

    test_manage_mapped_struct_2(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 2, "mapped struct signal"}
      , object{manager, ec, eagine::url{"json:///TestThdn2"}} {
        object.connect(
          make_callable_ref<&test_manage_mapped_struct_2::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::loaded_resource<eagine::test_tetrahedron>::load_info&
        info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.base.locator.has_scheme("json") and
                        info.base.locator.has_path("/TestThdn2");

        using eagine::are_equal;
        content_is_ok = true;
        content_is_ok = content_is_ok and are_equal(object->apex.x, 5);
        content_is_ok = content_is_ok and are_equal(object->apex.y, 6);
        content_is_ok = content_is_ok and are_equal(object->apex.z, 7);
        content_is_ok = content_is_ok and are_equal(object->base.a.x, 2);
        content_is_ok = content_is_ok and are_equal(object->base.a.y, 0);
        content_is_ok = content_is_ok and are_equal(object->base.a.z, 0);
        content_is_ok = content_is_ok and are_equal(object->base.b.x, 0);
        content_is_ok = content_is_ok and are_equal(object->base.b.y, 3);
        content_is_ok = content_is_ok and are_equal(object->base.b.z, 0);
        content_is_ok = content_is_ok and are_equal(object->base.c.x, 0);
        content_is_ok = content_is_ok and are_equal(object->base.c.y, 0);
        content_is_ok = content_is_ok and are_equal(object->base.c.z, 4);
    }

    auto is_loaded() const noexcept {
        return object.is_loaded() and content_is_ok;
    }

    auto is_done() noexcept -> bool final {
        return too_long or is_loaded();
    }

    void update() noexcept final {
        manager.update();
    }

    void clean_up() noexcept final {
        check(object.is_loaded(), "values are loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");

        manager.clean_up();
    }

private:
    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::basic_resource_manager<eagine::test_tetrahedron> manager;
    eagine::app::managed_resource<eagine::test_tetrahedron> object;
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

    eagitest::app_suite test{ctx, "resource manager", 2};
    test.once<test_manage_mapped_struct_1>();
    test.once<test_manage_mapped_struct_2>();
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_app.hpp>
