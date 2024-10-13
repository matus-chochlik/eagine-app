/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app:resource_basic;

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.identifier;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.progress;
import eagine.core.main_ctx;
import eagine.msgbus;
import eagine.shapes;
import eagine.oglplus;
import :resource_loader;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// plain_text_resource
//------------------------------------------------------------------------------
export class plain_text_resource final : public simple_resource<std::string> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// string_list_resource
//------------------------------------------------------------------------------
export class string_list_resource final
  : public simple_resource<std::vector<std::string>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// url_list_resource
//------------------------------------------------------------------------------
export class url_list_resource final
  : public simple_resource<std::vector<url>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// float_list_resource
//------------------------------------------------------------------------------
export class float_list_resource final
  : public simple_resource<std::vector<float>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// vec3_list_resource
//------------------------------------------------------------------------------
export class vec3_list_resource final
  : public simple_resource<std::vector<math::vector<float, 3>>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// mat4_list_resource
//------------------------------------------------------------------------------
export class mat4_list_resource final
  : public simple_resource<std::vector<math::matrix<float, 4, 4, true>>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// smooth_float_curve_resource
//------------------------------------------------------------------------------
export class smooth_float_curve_resource final
  : public simple_resource<math::cubic_bezier_curves<float, float>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// smooth_vec3_curve_resource
//------------------------------------------------------------------------------
export class smooth_vec3_curve_resource final
  : public simple_resource<
      math::cubic_bezier_curves<math::vector<float, 3>, float>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// glsl_string_resource
//------------------------------------------------------------------------------
export class glsl_string_resource final
  : public simple_resource<oglplus::glsl_string> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
// shape_generator_resource
//------------------------------------------------------------------------------
export class shape_generator_resource final
  : public simple_resource<shared_holder<shapes::generator>> {
public:
    auto kind() const noexcept -> identifier final;

    auto make_loader(
      const shared_holder<loaded_resource_context>&,
      resource_request_params params) noexcept -> shared_holder<loader> final;

    struct _loader;
};
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
