/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:implementation;

import eagine.core.main_ctx;
import :interface;
import std;

namespace eagine::app {

export auto make_oalplus_openal_provider(main_ctx_parent)
  -> std::shared_ptr<hmi_provider>;

export auto make_eglplus_opengl_provider(main_ctx_parent)
  -> std::shared_ptr<hmi_provider>;

export auto make_glfw3_opengl_provider(main_ctx_parent)
  -> std::shared_ptr<hmi_provider>;

} // namespace eagine::app

