/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
///
#include <eagine/testing/unit_begin.hpp>
import eagine.core.types;
import eagine.core.main_ctx;
import eagine.app;

namespace eagitest {
//------------------------------------------------------------------------------
class app_suite : public suite {
public:
    app_suite(
      eagine::test_ctx& ctx,
      std::string_view name,
      suite_case_t cases) noexcept
      : suite{ctx.args().argc(), ctx.args().argv(), name, cases}
      , _ctx{ctx} {}

    [[nodiscard]] auto context() const noexcept -> eagine::test_ctx& {
        return _ctx;
    }

    template <typename Launcher, typename... Args>
    auto once(Args&&... args) -> app_suite&;

private:
    eagine::test_ctx& _ctx;
};
//------------------------------------------------------------------------------
template <typename Case>
class launcher : public eagine::app::launchpad {
public:
    launcher(app_suite& suite)
      : _suite{suite} {}

    auto launch(
      eagine::app::execution_context& ec,
      const eagine::app::launch_options&)
      -> eagine::unique_holder<eagine::app::application> final {
        return {eagine::hold<Case>, _suite, ec};
    }

private:
    app_suite& _suite;
};
//------------------------------------------------------------------------------
class app_case
  : public eagine::app::application
  , public case_ {
public:
    app_case(
      app_suite& suite,
      eagine::app::execution_context& ec,
      suite_case_t case_idx,
      std::string_view name) noexcept
      : case_{suite, case_idx, name}
      , _suite{suite}
      , _ec{ec} {}

    void on_video_resize() noexcept override {}
    void update() noexcept override {}

    auto suite() const noexcept -> app_suite& {
        return _suite;
    }

    auto context() const noexcept -> eagine::app::execution_context& {
        return _ec;
    }

private:
    app_suite& _suite;
    eagine::app::execution_context& _ec;
};
//------------------------------------------------------------------------------
} // namespace eagitest
