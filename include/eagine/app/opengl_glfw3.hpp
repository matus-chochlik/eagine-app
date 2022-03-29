/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_OPENGL_GLFW3_HPP
#define EAGINE_APP_OPENGL_GLFW3_HPP

#include "config/basic.hpp"
#include "interface.hpp"
#include <eagine/main_ctx_fwd.hpp>
#include <memory>

namespace eagine::app {

auto make_glfw3_opengl_provider(main_ctx_parent)
  -> std::shared_ptr<hmi_provider>;

} // namespace eagine::app

#if !EAGINE_APP_LIBRARY || defined(EAGINE_IMPLEMENTING_APP_LIBRARY)
#include <eagine/app/opengl_glfw3.inl>
#endif

#endif
