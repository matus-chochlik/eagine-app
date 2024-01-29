/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
///
#include <eagine/testing/unit_end.hpp>
import eagine.app;

namespace eagitest {
//------------------------------------------------------------------------------
template <typename Case, typename... Args>
auto app_suite::once(Args&&... args) -> app_suite& {
    try {
        using L = typename Case::launcher;
        eagine::app::execution_context(_ctx)
          .prepare({eagine::hold<L>, *this, std::forward<Args>(args)...})
          .run()
          .result();
    } catch(const abort_test_case&) {
    } catch(...) {
    }
    return *this;
}
//------------------------------------------------------------------------------
} // namespace eagitest
