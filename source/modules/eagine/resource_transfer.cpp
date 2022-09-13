/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:resource_transfer;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.shapes;
import eagine.oglplus;
import <utility>;

namespace eagine::app {
//------------------------------------------------------------------------------
export struct resource_loader_signals {
    signal<void(identifier_t, const shapes::generator&) noexcept> shape_loaded;

    signal<void(identifier_t, oglplus::buffer_name) noexcept> buffer_loaded;

    signal<void(identifier_t, oglplus::texture_name) noexcept> texture_loaded;

    signal<void(identifier_t, const oglplus::glsl_string_ref&) noexcept>
      shader_source_loaded;

    signal<void(identifier_t, oglplus::shader_name) noexcept> shader_loaded;

    signal<void(identifier_t, oglplus::program_name) noexcept> program_loaded;
};
//------------------------------------------------------------------------------
export class resource_loader : public resource_loader_signals {
public:
    auto request_gl_buffer(const url& locator) noexcept
      -> std::pair<identifier_t, const url&>;

    auto request_gl_texture(const url& locator) noexcept
      -> std::pair<identifier_t, const url&>;

    auto request_gl_shader_source(const url& locator) noexcept
      -> std::pair<identifier_t, const url&>;

    auto request_gl_shader(const url& locator) noexcept
      -> std::pair<identifier_t, const url&>;

    auto request_gl_program(const url& locator) noexcept
      -> std::pair<identifier_t, const url&>;

private:
};
//------------------------------------------------------------------------------
} // namespace eagine::app

