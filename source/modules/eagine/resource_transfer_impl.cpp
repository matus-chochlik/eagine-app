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
    if(auto gen{shapes::shape_from(locator, main_context())}) {
        const auto request_id{get_request_id()};
        _shape_generators.emplace_back(request_id, std::move(gen));
        return {request_id, std::move(locator)};
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

    for(auto& [request_id, shape_gen] : _shape_generators) {
        shape_loaded(request_id, shape_gen);
    }
    _shape_generators.clear();

    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
