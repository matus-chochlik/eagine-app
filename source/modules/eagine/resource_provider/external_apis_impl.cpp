/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.resource_provider;

import eagine.core;
import eagine.eglplus;
import eagine.oglplus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
external_apis::external_apis(main_ctx_parent parent)
  : main_ctx_object{"ExternAPIs", parent} {}
//------------------------------------------------------------------------------
external_apis::~external_apis() noexcept {
    //
}
//------------------------------------------------------------------------------
auto external_apis::egl() noexcept -> optional_reference<eglplus::egl_api> {
    return _egl.ensure(as_parent()).ref();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
