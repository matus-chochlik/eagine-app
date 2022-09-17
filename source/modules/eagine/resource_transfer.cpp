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
import eagine.msgbus;
import <utility>;

namespace eagine::app {
//------------------------------------------------------------------------------
export class resource_request_result {
public:
    resource_request_result(identifier_t req_id, url loc) noexcept
      : _request_id{req_id}
      , _locator(std::move(loc)) {}

    explicit operator bool() const noexcept {
        return _request_id != 0;
    }

    auto request_id() const noexcept -> identifier_t {
        return _request_id;
    }

    auto locator() const noexcept -> const url& {
        return _locator;
    }

private:
    identifier_t _request_id;
    url _locator;
};
//------------------------------------------------------------------------------
export struct resource_loader_signals {
    signal<void(identifier_t, const std::shared_ptr<shapes::generator>&) noexcept>
      shape_loaded;

    signal<void(identifier_t, oglplus::buffer_name) noexcept> buffer_loaded;

    signal<void(identifier_t, oglplus::texture_name) noexcept> texture_loaded;

    signal<void(identifier_t, const oglplus::glsl_string_ref&) noexcept>
      shader_source_loaded;

    signal<void(identifier_t, oglplus::shader_name) noexcept> shader_loaded;

    signal<void(identifier_t, oglplus::program_name) noexcept> program_loaded;
};
//------------------------------------------------------------------------------
export class resource_loader
  : public msgbus::resource_data_consumer_node
  , public resource_loader_signals {
    using base = msgbus::resource_data_consumer_node;

public:
    auto update() noexcept -> work_done;

    auto request_shape_generator(url locator) noexcept
      -> resource_request_result;

    auto request_gl_buffer(url locator) noexcept -> resource_request_result;

    auto request_gl_texture(url locator) noexcept -> resource_request_result;

    auto request_gl_shader_source(url locator) noexcept
      -> resource_request_result;

    auto request_gl_shader(url locator) noexcept -> resource_request_result;

    auto request_gl_program(url locator) noexcept -> resource_request_result;

private:
};
//------------------------------------------------------------------------------
} // namespace eagine::app

