/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.runtime;

namespace eagine::app {
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(const url& locator) noexcept
  -> std::pair<identifier_t, const url&> {
    return {0, locator};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(const url& locator) noexcept
  -> std::pair<identifier_t, const url&> {
    return {0, locator};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader_source(const url& locator) noexcept
  -> std::pair<identifier_t, const url&> {
    return {0, locator};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(const url& locator) noexcept
  -> std::pair<identifier_t, const url&> {
    return {0, locator};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_program(const url& locator) noexcept
  -> std::pair<identifier_t, const url&> {
    return {0, locator};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
