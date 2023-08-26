/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "resource.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
void model_viewer_resource_intf::signal_loaded() {
    model_viewer_resource_signals::loaded();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
