/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "eagitexi_provider.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
// 2d_r8
//------------------------------------------------------------------------------
struct eagitexi_2d_r8_io final
  : main_ctx_object
  , msgbus::source_blob_io {
    auto make_header(
      pixel_provider_interface& pixel_provider,
      const int w,
      const int h,
      const int l) {
        std::stringstream hdr;
        hdr << R"({"level":)" << l;
        hdr << R"(,"width":)" << w;
        hdr << R"(,"height":)" << h;
        hdr << R"(,"channels":1)";
        hdr << R"(,"data_type":"unsigned_byte")";
        hdr << R"(,"format":"red")";
        hdr << R"(,"iformat":"r8")";
        hdr << R"(,"tag":["generated"])";
        hdr << R"(,"data_filter":"zlib")";
        hdr << '}';
        auto header{main_context().buffers().get(
          pixel_provider.estimated_data_size(w, h, 1))};
        memory::copy_into(as_bytes(string_view{hdr.str()}), header);
        return header;
    }

    void make_data(pixel_provider_interface& pixel_provider, int w, int h) {
        const auto append{[this](memory::const_block packed) {
            memory::append_to(packed, _data);
            return true;
        }};
        stream_compression compress{
          main_context().compressor(),
          {construct_from, append},
          default_data_compression_method()};

        std::array<byte, 256> buf;
        std::size_t i{0U};

        assert(pixel_provider.pixel_byte_count() == 1);
        for(int y = 0; y < h; ++y) {
            for(int x = 0; x < w; ++x) {
                buf[i++] = pixel_provider.pixel_byte(x, y, 0, w, h, 1, 0);
                if(i == buf.size()) {
                    compress.next(view(buf), data_compression_level::highest);
                    i = 0U;
                }
            }
        }
        if(i > 0) {
            compress.next(
              head(view(buf), span_size(i)), data_compression_level::highest);
        }
        compress.finish();
    }

    eagitexi_2d_r8_io(
      main_ctx_parent parent,
      unique_holder<pixel_provider_interface> provider,
      int w,
      int h,
      int l)
      : main_ctx_object{"ITx2R8", parent}
      , _width{w}
      , _height{h}
      , _data{make_header(*provider, w, h, l)} {
        make_data(*provider, w, h);
    }

    ~eagitexi_2d_r8_io() noexcept {
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
struct eagitexi_2d_r8_provider final
  : main_ctx_object
  , resource_provider_interface {
    std::string url_path;
    shared_holder<pixel_provider_factory_interface> factory;

    eagitexi_2d_r8_provider(
      main_ctx_parent parent,
      std::string path,
      shared_holder<pixel_provider_factory_interface> f) noexcept
      : main_ctx_object{"PTx2R8", parent}
      , url_path{path}
      , factory{std::move(f)} {}

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_lvl(int l) noexcept -> bool {
        return (l >= 0);
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        if(locator.has_scheme("eagitexi") and locator.has_path(url_path)) {
            const auto& q{locator.query()};
            const bool args_ok =
              valid_dim(q.arg_value_as<int>("width").value_or(1)) and
              valid_dim(q.arg_value_as<int>("height").value_or(1)) and
              valid_lvl(q.arg_value_as<int>("level").value_or(1));
            return args_ok and factory->has_resource(locator);
        }
        return false;
    }

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final {
        const auto& q{locator.query()};
        return {
          hold<eagitexi_2d_r8_io>,
          as_parent(),
          factory->make_provider(locator),
          q.arg_value_as<int>("width").value_or(2),
          q.arg_value_as<int>("height").value_or(2),
          q.arg_value_as<int>("level").value_or(0)};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_r8(
  main_ctx_parent parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface> factory)
  -> unique_holder<resource_provider_interface> {
    assert(factory);
    return {
      hold<eagitexi_2d_r8_provider>,
      parent,
      std::move(path),
      std::move(factory)};
}
//------------------------------------------------------------------------------
// 2d_rgb8
//------------------------------------------------------------------------------
struct eagitexi_2d_rgb8_io final
  : main_ctx_object
  , msgbus::source_blob_io {
    auto make_header(
      pixel_provider_interface& pixel_provider,
      const int w,
      const int h,
      const int l) {
        std::stringstream hdr;
        hdr << R"({"level":)" << l;
        hdr << R"(,"width":)" << w;
        hdr << R"(,"height":)" << h;
        hdr << R"(,"channels":3)";
        hdr << R"(,"data_type":"unsigned_byte")";
        hdr << R"(,"format":"rgb")";
        hdr << R"(,"iformat":"rgb8")";
        hdr << R"(,"tag":["generated"])";
        hdr << R"(,"data_filter":"zlib")";
        hdr << '}';
        auto header{main_context().buffers().get(
          pixel_provider.estimated_data_size(w, h, 1))};
        memory::copy_into(as_bytes(string_view{hdr.str()}), header);
        return header;
    }

    void make_data(pixel_provider_interface& pixel_provider, int w, int h) {
        const auto append{[this](memory::const_block packed) {
            memory::append_to(packed, _data);
            return true;
        }};
        stream_compression compress{
          main_context().compressor(),
          {construct_from, append},
          default_data_compression_method()};

        assert(pixel_provider.pixel_byte_count() == 3);
        std::array<byte, 3> rgb{{}};
        const auto pix{view(rgb)};
        for(int y = 0; y < h; ++y) {
            for(int x = 0; x < w; ++x) {
                for(int c = 0; c < 3; ++c) {
                    rgb[std_size(c)] =
                      pixel_provider.pixel_byte(x, y, 0, w, h, 1, c);
                }
                compress.next(pix, data_compression_level::highest);
            }
        }
        compress.finish();
    }

    eagitexi_2d_rgb8_io(
      main_ctx_parent parent,
      unique_holder<pixel_provider_interface> provider,
      int w,
      int h,
      int l)
      : main_ctx_object{"ITx2RGB8", parent}
      , _width{w}
      , _height{h}
      , _data{make_header(*provider, w, h, l)} {
        make_data(*provider, w, h);
    }

    ~eagitexi_2d_rgb8_io() noexcept {
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
struct eagitexi_2d_rgb8_provider final
  : main_ctx_object
  , resource_provider_interface {
    std::string url_path;
    shared_holder<pixel_provider_factory_interface> factory;

    eagitexi_2d_rgb8_provider(
      main_ctx_parent parent,
      std::string path,
      shared_holder<pixel_provider_factory_interface> f) noexcept
      : main_ctx_object{"PTx2RGB8", parent}
      , url_path{path}
      , factory{std::move(f)} {}

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_lvl(int l) noexcept -> bool {
        return (l >= 0);
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        if(locator.has_scheme("eagitexi") and locator.has_path(url_path)) {
            const auto& q{locator.query()};
            const bool args_ok =
              valid_dim(q.arg_value_as<int>("width").value_or(1)) and
              valid_dim(q.arg_value_as<int>("height").value_or(1)) and
              valid_lvl(q.arg_value_as<int>("level").value_or(1));
            return args_ok and factory->has_resource(locator);
        }
        return false;
    }

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final {
        const auto& q{locator.query()};
        return {
          hold<eagitexi_2d_rgb8_io>,
          as_parent(),
          factory->make_provider(locator),
          q.arg_value_as<int>("width").value_or(2),
          q.arg_value_as<int>("height").value_or(2),
          q.arg_value_as<int>("level").value_or(0)};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_2d_rgb8(
  main_ctx_parent parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface> factory)
  -> unique_holder<resource_provider_interface> {
    assert(factory);
    return {
      hold<eagitexi_2d_rgb8_provider>,
      parent,
      std::move(path),
      std::move(factory)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
