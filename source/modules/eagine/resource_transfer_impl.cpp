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
import eagine.core.string;
import eagine.core.reflection;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.shapes;
import eagine.oglplus;

namespace eagine::app {
//------------------------------------------------------------------------------
auto resource_loader::request_shape_generator(url locator) noexcept
  -> resource_request_result {
    const auto args{locator.query()};
    shapes::vertex_attrib_kinds attrs;
    for(const auto& info : enumerator_mapping(
          std::type_identity<shapes::vertex_attrib_kind>{}, default_selector)) {
        if(args.arg_has_value(info.name, true)) {
            attrs.set(info.enumerator);
        }
    }

    std::shared_ptr<shapes::generator> gen;
    if(locator.has_path("/cube")) {
        gen = shapes::unit_cube(attrs);
    }
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader_source(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_program(url locator) noexcept
  -> resource_request_result {
    return {0, std::move(locator)};
}
//------------------------------------------------------------------------------
auto resource_loader::update() noexcept -> work_done {
    some_true something_done{base::update()};

    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
