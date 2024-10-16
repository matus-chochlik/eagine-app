/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
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
struct single_rgb8_pixel_provider : pixel_provider_interface {
    std::array<byte, 3> rgb;

    single_rgb8_pixel_provider(int r, int g, int b) noexcept
      : rgb{{limit_cast<byte>(r), limit_cast<byte>(g), limit_cast<byte>(b)}} {}

    auto pixel_byte_count() noexcept -> int final {
        return 3;
    }

    auto estimated_data_size(int, int, int) noexcept -> span_size_t final {
        return 1024;
    }

    auto pixel_byte(pixel_provider_coordinate c) noexcept -> byte final {
        assert(c.component >= 0 and c.component <= 3);
        return rgb[std_size(c.component)];
    }
};
//------------------------------------------------------------------------------
struct single_rgb8_pixel_provider_factory : pixel_provider_factory_interface {
    static auto valid_clr(int c) noexcept -> bool {
        return (c >= 0) and (c <= 255);
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        const auto& q{locator.query()};
        using math::name_to_rgb;
        if(q.decoded_arg_value("color").and_then(name_to_rgb).has_value()) {
            return true;
        }
        return valid_clr(q.arg_value_as<int>("r").value_or(0)) or
               valid_clr(q.arg_value_as<int>("g").value_or(0)) or
               valid_clr(q.arg_value_as<int>("b").value_or(0));
    }

    auto make_provider(const url& locator)
      -> unique_holder<pixel_provider_interface> final {
        const auto& q{locator.query()};
        using math::name_to_rgb_int;
        if(const auto rgb{q.arg_value("color").and_then(name_to_rgb_int)}) {
            const auto [r, g, b] = *rgb;
            return {hold<single_rgb8_pixel_provider>, r, g, b};
        }
        return {
          hold<single_rgb8_pixel_provider>,
          q.arg_value_as<int>("r").value_or(1),
          q.arg_value_as<int>("g").value_or(1),
          q.arg_value_as<int>("b").value_or(1)};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_single_rgb8(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return provider_eagitexi_2d_rgb8(
      p.parent, "/2d_single_rgb8", {hold<single_rgb8_pixel_provider_factory>});
}
//------------------------------------------------------------------------------
// eagitex
//------------------------------------------------------------------------------
struct single_rgb8_eagitex_io final : simple_string_source_blob_io {
    auto make_header(int s, int r, int g, int b) -> std::string;

    single_rgb8_eagitex_io(main_ctx_parent parent, int s, int r, int g, int b)
      : simple_string_source_blob_io{
          "ITxSiRGB8",
          parent,
          make_header(s, r, g, b)} {}
};
//------------------------------------------------------------------------------
auto single_rgb8_eagitex_io::make_header(int s, int r, int g, int b)
  -> std::string {
    int l = 0;
    for(int i = s; i > 0; i /= 2) {
        ++l;
    }
    std::stringstream hdr;
    hdr << R"({"levels":)" << l;
    hdr << R"(,"width":)" << s;
    hdr << R"(,"height":)" << s;
    hdr << R"(,"channels":3)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"rgb")";
    hdr << R"(,"iformat":"rgb8")";
    hdr << R"(,"tag":["generated","single-color"])";
    hdr << R"(,"images":[)";

    l = 0;
    while(s > 0) {
        if(l) {
            hdr << ',';
        }
        hdr << R"({"url":"eagitexi:///2d_single_rgb8)";
        hdr << "?level=" << l;
        hdr << "+width=" << s;
        hdr << "+height=" << s;
        hdr << "+r=" << r;
        hdr << "+g=" << g;
        hdr << "+b=" << b;
        hdr << '"';
        hdr << '}';

        s /= 2;
        ++l;
    }
    hdr << ']' << '}';
    return hdr.str();
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct single_rgb8_eagitex_provider final : eagitex_provider_base {

    single_rgb8_eagitex_provider(main_ctx_parent parent) noexcept
      : eagitex_provider_base{"PTxSiRGB8", parent} {}

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> shared_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
auto single_rgb8_eagitex_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(locator.has_scheme("eagitex") and locator.has_path("/2d_single_rgb8")) {
        const auto& q{locator.query()};
        const bool args_ok =
          valid_dimension(q.arg_value_as<int>("size").value_or(1));
        using math::name_to_rgb;
        if(q.decoded_arg_value("color").and_then(name_to_rgb).has_value()) {
            return args_ok;
        }
        if(
          valid_color(q.arg_value_as<int>("r").value_or(0)) or
          valid_color(q.arg_value_as<int>("g").value_or(0)) or
          valid_color(q.arg_value_as<int>("b").value_or(0))) {
            return args_ok;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto single_rgb8_eagitex_provider::get_resource_io(const url& locator)
  -> shared_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    const auto size{q.arg_value_as<int>("size").value_or(64)};
    using math::name_to_rgb_int;
    if(const auto rgb{q.decoded_arg_value("color").and_then(name_to_rgb_int)}) {
        const auto [r, g, b] = *rgb;
        return {hold<single_rgb8_eagitex_io>, as_parent(), size, r, g, b};
    }
    return {
      hold<single_rgb8_eagitex_io>,
      as_parent(),
      size,
      q.arg_value_as<int>("r").value_or(0),
      q.arg_value_as<int>("g").value_or(0),
      q.arg_value_as<int>("b").value_or(0)};
}
//------------------------------------------------------------------------------
void single_rgb8_eagitex_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitex:///2d_single_rgb8");
}
//------------------------------------------------------------------------------
auto provider_eagitex_2d_single_rgb8(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<single_rgb8_eagitex_provider>, p.parent};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
