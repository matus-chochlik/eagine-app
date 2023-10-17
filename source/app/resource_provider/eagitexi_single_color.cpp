/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "providers.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
struct eagitexi_2d_single_rgb_io final
  : main_ctx_object
  , msgbus::source_blob_io {
    auto make_header(const int w, const int h, const int l) {
        std::stringstream hdr;
        hdr << R"({"level":)" << l;
        hdr << R"(,"width":)" << w;
        hdr << R"(,"height":)" << h;
        hdr << R"(,"channels":3)";
        hdr << R"(,"data_type":"unsigned_byte")";
        hdr << R"(,"format":"rgb")";
        hdr << R"(,"iformat":"rgb8")";
        hdr << R"(,"tag":["generated"])";
        hdr << R"(,"data_filer":"zlib")";
        hdr << '}';
        auto buf{main_context().buffers().get(1024)};
        memory::copy_into(as_bytes(string_view{hdr.str()}), buf);
        return buf;
    }

    void make_data(byte r, byte g, byte b, int n) {
        const auto append{[this](memory::const_block packed) {
            memory::append_to(packed, _data);
            return true;
        }};
        stream_compression cmp{
          main_context().compressor(),
          {construct_from, append},
          default_data_compression_method()};

        const std::array<byte, 3> rgb{{r, g, b}};
        for(int i = 0; i < n; ++i) {
            cmp.next(view(rgb), data_compression_level::highest);
        }
        cmp.finish();
    }

    eagitexi_2d_single_rgb_io(
      int r,
      int g,
      int b,
      int w,
      int h,
      int l,
      main_ctx_parent parent)
      : main_ctx_object{"IT2dRGB", parent}
      , _width{w}
      , _height{h}
      , _data{make_header(w, h, l)} {
        make_data(
          limit_cast<byte>(r), limit_cast<byte>(g), limit_cast<byte>(b), w * h);
    }

    ~eagitexi_2d_single_rgb_io() noexcept {
        main_context().buffers().eat(std::move(_data));
    }

    auto total_size() noexcept -> span_size_t final {
        return span_size(_data.size());
    }

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final {
        return copy(head(view(_data), dst.size()), dst).size();
    }

    int _width;
    int _height;
    memory::buffer _data;
};
//------------------------------------------------------------------------------
struct eagitexi_2d_single_rgb_provider final
  : main_ctx_object
  , resource_provider_interface {

    eagitexi_2d_single_rgb_provider(main_ctx_parent parent) noexcept
      : main_ctx_object{"GT2dRGB", parent} {}

    static auto valid_clr(int c) noexcept -> bool {
        return (c >= 0) and (c <= 255);
    }

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_lvl(int l) noexcept -> bool {
        return (l >= 0);
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        if(locator.has_scheme("eagitexi") and locator.has_path("/2d_single_rgb")) {
            const auto& q{locator.query()};
            const bool args_ok =
              valid_lvl(q.arg_value_as<int>("r").value_or(0)) and
              valid_lvl(q.arg_value_as<int>("g").value_or(0)) and
              valid_lvl(q.arg_value_as<int>("b").value_or(0)) and
              valid_dim(q.arg_value_as<int>("width").value_or(1)) and
              valid_dim(q.arg_value_as<int>("height").value_or(1)) and
              valid_lvl(q.arg_value_as<int>("level").value_or(1));
            return args_ok;
        }
        return false;
    }

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final {
        const auto& q{locator.query()};
        return {
          hold<eagitexi_2d_single_rgb_io>,
          q.arg_value_as<int>("r").value_or(0),
          q.arg_value_as<int>("g").value_or(0),
          q.arg_value_as<int>("b").value_or(0),
          q.arg_value_as<int>("width").value_or(2),
          q.arg_value_as<int>("height").value_or(2),
          q.arg_value_as<int>("length").value_or(0),
          as_parent()};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_single_rgb(main_ctx_parent parent)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_2d_single_rgb_provider>, parent};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
