/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// eagitexi
//------------------------------------------------------------------------------
struct sphere_volume_pixel_provider : pixel_provider_interface {

    auto pixel_byte_count() noexcept -> int final {
        return 1;
    }

    auto estimated_data_size(int, int, int) noexcept -> span_size_t final {
        // TODO
        return 1024;
    }

    auto pixel_byte(pixel_provider_coordinate c) noexcept -> byte final {
        assert(c.component == 0);
        // TODO
        return 0x00U;
    }
};
//------------------------------------------------------------------------------
struct sphere_volume_pixel_provider_factory : pixel_provider_factory_interface {
    auto has_resource(const url&) noexcept -> bool final {
        return true;
    }

    auto make_provider(const url&)
      -> unique_holder<pixel_provider_interface> final {
        return {hold<sphere_volume_pixel_provider>};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_sphere_volume(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return provider_eagitexi_3d_r8(
      p.parent, "/sphere_volume", {hold<sphere_volume_pixel_provider_factory>});
}
//------------------------------------------------------------------------------
// eagitex
//------------------------------------------------------------------------------
struct sphere_volume_eagitex_io final : simple_string_source_blob_io {
    auto make_header(int s) -> std::string;

    sphere_volume_eagitex_io(main_ctx_parent parent, int s)
      : simple_string_source_blob_io{"ITxSphR8", parent, make_header(s)} {}
};
//------------------------------------------------------------------------------
auto sphere_volume_eagitex_io::make_header(int s) -> std::string {
    std::stringstream hdr;
    hdr << R"({"levels":1)";
    hdr << R"(,"width":)" << s;
    hdr << R"(,"height":)" << s;
    hdr << R"(,"depth":)" << s;
    hdr << R"(,"channels":1)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"red")";
    hdr << R"(,"iformat":"r8")";
    hdr << R"(,"tag":["generated","volume","sphere"])";
    hdr << R"(,"images":[)";
    hdr << R"({"url":"eagitexi:///sphere_volume)";
    hdr << "?level=0";
    hdr << "+width=" << s;
    hdr << "+height=" << s;
    hdr << "+depth=" << s;
    hdr << '"';
    hdr << '}';
    hdr << ']' << '}';
    return hdr.str();
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct sphere_volume_eagitex_provider final : eagitex_provider_base {

    sphere_volume_eagitex_provider(main_ctx_parent parent) noexcept
      : eagitex_provider_base{"PTxSphR8", parent} {}

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
auto sphere_volume_eagitex_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(locator.has_scheme("eagitex") and locator.has_path("/sphere_volume")) {
        const auto& q{locator.query()};
        const bool args_ok =
          valid_dimension(q.arg_value_as<int>("size").value_or(1));
        return args_ok;
    }
    return false;
}
//------------------------------------------------------------------------------
auto sphere_volume_eagitex_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<sphere_volume_eagitex_io>,
      as_parent(),
      q.arg_value_as<int>("size").value_or(0)};
}
//------------------------------------------------------------------------------
void sphere_volume_eagitex_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    return callback("eagitex:///sphere_volume");
}
//------------------------------------------------------------------------------
auto provider_eagitex_sphere_volume(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<sphere_volume_eagitex_provider>, p.parent};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
