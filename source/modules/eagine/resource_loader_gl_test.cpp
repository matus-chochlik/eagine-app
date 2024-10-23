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
import eagine.oglplus;
//------------------------------------------------------------------------------
// GL shader include
//------------------------------------------------------------------------------
struct test_request_gl_shader_include : eagitest::app_case {
    using launcher = eagitest::launcher_with_gl<test_request_gl_shader_include>;

    test_request_gl_shader_include(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 1, "GL shader include"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_gl_shader_include::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        load_signal_received = true;
        locator_is_ok = info.locator.has_scheme("glsl") and
                        info.locator.has_path("/TestMndbrt");
        content_is_ok = incl->path() == "/mandelbrot";
    }

    auto is_done() noexcept -> bool final {
        return too_long or incl.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(
          incl,
          context().shared_resource_context(),
          [] -> eagine::app::resource_request_params {
              return {eagine::url{"glsl:///TestMndbrt?path=%2Fmandelbrot"}};
          });
    }

    void clean_up() noexcept final {
        check(incl.is_loaded(), "resource is loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::gl_shader_include_resource incl;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// GL shader includes
//------------------------------------------------------------------------------
struct test_request_gl_shader_includes : eagitest::app_case {
    using launcher =
      eagitest::launcher_with_gl<test_request_gl_shader_includes>;

    test_request_gl_shader_includes(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 2, "GL shader includes"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_gl_shader_includes::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        if(info.locator.has_scheme("txt") and info.locator.has_path("/TestIncls")) {
            load_signal_received = true;
            locator_is_ok = true;
            content_is_ok = incls->size() == 2;
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or incls.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(
          incls,
          context().shared_resource_context(),
          [] -> eagine::app::resource_request_params {
              return {eagine::url{"txt:///TestIncls"}};
          });
    }

    void clean_up() noexcept final {
        check(incls.is_loaded(), "resource is loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::exp::gl_shader_includes_resource incls;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// GL shader from GLSL
//------------------------------------------------------------------------------
struct test_request_gl_shader_1 : eagitest::app_case {
    using launcher = eagitest::launcher_with_gl<test_request_gl_shader_1>;

    test_request_gl_shader_1(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 3, "GL shader 1"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_gl_shader_1::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        if(
          info.locator.has_scheme("glsl_frag") and
          info.locator.has_path("/TestFrag1")) {
            load_signal_received = true;
            locator_is_ok = true;
            content_is_ok = shdr.get().has_value();
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or shdr.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(
          shdr,
          context().shared_resource_context(),
          [] -> eagine::app::resource_request_params {
              return {eagine::url{"glsl_frag:///TestFrag1"}};
          });
    }

    void clean_up() noexcept final {
        check(shdr.is_loaded(), "resource is loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{10}};
    eagine::app::exp::gl_shader_resource shdr;
    bool load_signal_received{false};
    bool locator_is_ok{false};
    bool content_is_ok{false};
};
//------------------------------------------------------------------------------
// GL shader from EAGishdr with includes
//------------------------------------------------------------------------------
struct test_request_gl_shader_2 : eagitest::app_case {
    using launcher = eagitest::launcher_with_gl<test_request_gl_shader_2>;

    test_request_gl_shader_2(auto& s, auto& ec)
      : eagitest::app_case{s, ec, 4, "GL shader 2"}
      , loader{context().loader()} {
        loader.resource_loaded.connect(
          make_callable_ref<&test_request_gl_shader_2::on_loaded>(this));
        too_long.reset();
    }

    void on_loaded(
      const eagine::app::resource_interface::load_info& info) noexcept {
        if(
          info.locator.has_scheme("json") and
          info.locator.has_path("/TestFragE1")) {
            load_signal_received = true;
            locator_is_ok = true;
            content_is_ok = shdr.get().has_value();
        }
    }

    auto is_done() noexcept -> bool final {
        return too_long or shdr.is_loaded();
    }

    void update() noexcept final {
        loader.load_if_needed(
          shdr,
          context().shared_resource_context(),
          [] -> eagine::app::resource_request_params {
              return {eagine::url{"json:///TestFragE1"}};
          });
    }

    void clean_up() noexcept final {
        check(shdr.is_loaded(), "resource is loaded");
        check(load_signal_received, "load signal received");
        check(locator_is_ok, "locator is ok");
        check(content_is_ok, "content is ok");
    }

    eagine::app::resource_loader& loader;
    eagine::timeout too_long{std::chrono::seconds{15}};
    eagine::app::exp::gl_shader_resource shdr;
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

    eagitest::app_suite test{ctx, "resource loader GL", 4};
    test.once<test_request_gl_shader_include>();
    test.once<test_request_gl_shader_includes>();
    test.once<test_request_gl_shader_1>();
    test.once<test_request_gl_shader_2>();
    return test.exit_code();
}
//------------------------------------------------------------------------------
auto main(int argc, const char** argv) -> int {
    return eagine::test_main_impl(argc, argv, test_main);
}
//------------------------------------------------------------------------------
#include <eagine/testing/unit_end_app.hpp>
