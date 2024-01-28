/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:implementation;

import std;
import eagine.core.types;
import eagine.core.main_ctx;
import :interface;

namespace eagine::app {

export auto make_oalplus_openal_provider(main_ctx_parent)
  -> shared_holder<hmi_provider>;

export auto make_eglplus_opengl_provider(main_ctx_parent)
  -> shared_holder<hmi_provider>;

export auto make_glfw3_opengl_provider(main_ctx_parent)
  -> shared_holder<hmi_provider>;

} // namespace eagine::app

