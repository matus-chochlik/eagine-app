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
// random I/O
//------------------------------------------------------------------------------
struct eagitexi_random_io final
  : main_ctx_object
  , msgbus::source_blob_io {
    auto make_header(const int w, const int h, const int d, const int l);

    eagitexi_random_io(main_ctx_parent parent, int w, int h, int d, int l);

    auto total_size() noexcept -> span_size_t final;

    auto make_generator(span_size_t offs) noexcept;

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

    const span_size_t _size;
    std::string _header;
    std::independent_bits_engine<std::default_random_engine, 8U, byte> _engine{
      std::default_random_engine{std::random_device{}()}};
};
//------------------------------------------------------------------------------
auto eagitexi_random_io::make_header(
  const int w,
  const int h,
  const int d,
  const int l) {
    std::stringstream hdr;
    hdr << R"({"level":)" << l;
    hdr << R"(,"width":)" << w;
    if(h > 1) {
        hdr << R"(,"height":)" << h;
        if(d > 1) {
            hdr << R"(,"depth":)" << d;
        }
    }
    hdr << R"(,"channels":1)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"red")";
    hdr << R"(,"iformat":"r8")";
    hdr << R"(,"tag":["generated","random"])";
    hdr << '}';
    return hdr.str();
}
//------------------------------------------------------------------------------
eagitexi_random_io::eagitexi_random_io(
  main_ctx_parent parent,
  int w,
  int h,
  int d,
  int l)
  : main_ctx_object{"ITx2Rnd8", parent}
  , _size{span_size(w * h * d)}
  , _header{make_header(w, h, d, l)} {}
//------------------------------------------------------------------------------
auto eagitexi_random_io::total_size() noexcept -> span_size_t {
    return span_size(_header.size()) + _size;
}
//------------------------------------------------------------------------------
auto eagitexi_random_io::make_generator(span_size_t offs) noexcept {
    return [offs, this]() mutable -> byte {
        const auto idx = offs++;
        if(idx < span_size(_header.size())) {
            return byte(_header[std_size(idx)]);
        }
        return _engine();
    };
}
//------------------------------------------------------------------------------
auto eagitexi_random_io::fetch_fragment(
  const span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    return generate(head(dst, total_size() - offs), make_generator(offs)).size();
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct eagitexi_random_provider final
  : main_ctx_object
  , resource_provider_interface {

    eagitexi_random_provider(main_ctx_parent parent) noexcept
      : main_ctx_object{"PTx2Rnd8", parent} {}

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_lvl(int l) noexcept -> bool {
        return (l >= 0);
    }

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> shared_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
auto eagitexi_random_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(locator.has_scheme("eagitexi") and locator.has_path("/random")) {
        const auto& q{locator.query()};
        const bool args_ok =
          valid_dim(q.arg_value_as<int>("width").value_or(1)) and
          valid_dim(q.arg_value_as<int>("height").value_or(1)) and
          valid_dim(q.arg_value_as<int>("depth").value_or(1)) and
          valid_lvl(q.arg_value_as<int>("level").value_or(1));
        return args_ok;
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_random_provider::get_resource_io(const url& locator)
  -> shared_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagitexi_random_io>,
      as_parent(),
      q.arg_value_as<int>("width").value_or(8),
      q.arg_value_as<int>("height").value_or(1),
      q.arg_value_as<int>("depth").value_or(1),
      q.arg_value_as<int>("level").value_or(0)};
}
//------------------------------------------------------------------------------
void eagitexi_random_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///random");
}
//------------------------------------------------------------------------------
auto provider_eagitexi_random(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_random_provider>, p.parent};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
