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
// 2d_r8
//------------------------------------------------------------------------------
struct eagitexi_2d_r8_io final : simple_buffer_source_blob_io {

    auto make_header(
      memory::buffer,
      pixel_provider_interface& pixel_provider,
      const int w,
      const int h,
      const int l) -> memory::buffer;

    auto make_data(
      memory::buffer,
      pixel_provider_interface& pixel_provider,
      int w,
      int h) -> memory::buffer;

    eagitexi_2d_r8_io(
      main_ctx_parent parent,
      unique_holder<pixel_provider_interface> provider,
      int w,
      int h,
      int l);
};
//------------------------------------------------------------------------------
auto eagitexi_2d_r8_io::make_header(
  memory::buffer header,
  pixel_provider_interface&,
  const int w,
  const int h,
  const int l) -> memory::buffer {
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
    memory::copy_into(as_bytes(string_view{hdr.str()}), header);
    return header;
}
//------------------------------------------------------------------------------
auto eagitexi_2d_r8_io::make_data(
  memory::buffer data,
  pixel_provider_interface& pixel_provider,
  int w,
  int h) -> memory::buffer {
    const auto append{[&](memory::const_block packed) {
        memory::append_to(packed, data);
        return true;
    }};
    stream_compression compress{
      main_context().compressor(),
      {construct_from, append},
      default_data_compression_method()};

    std::array<byte, 4 * 1024> buf;
    std::size_t i{0U};

    assert(pixel_provider.pixel_byte_count() == 1);
    for(int y = 0; y < h; ++y) {
        for(int x = 0; x < w; ++x) {
            buf[i++] = pixel_provider.pixel_byte(
              {.width = w, .height = h, .x = x, .y = y});
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
    return data;
}
//------------------------------------------------------------------------------
eagitexi_2d_r8_io::eagitexi_2d_r8_io(
  main_ctx_parent parent,
  unique_holder<pixel_provider_interface> provider,
  int w,
  int h,
  int l)
  : simple_buffer_source_blob_io{
      "ITx2R8",
      parent,
      provider->estimated_data_size(w, h, 1),
      [&](memory::buffer content) {
          return make_data(
            make_header(std::move(content), *provider, w, h, l),
            *provider,
            w,
            h);
      }} {}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct eagitexi_2d_r8_provider final
  : main_ctx_object
  , resource_provider_interface {
    std::string url_path;
    shared_holder<pixel_provider_factory_interface> factory;

    eagitexi_2d_r8_provider(
      main_ctx_parent parent,
      std::string path,
      shared_holder<pixel_provider_factory_interface> f) noexcept;

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_lvl(int l) noexcept -> bool {
        return (l >= 0);
    }

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
eagitexi_2d_r8_provider::eagitexi_2d_r8_provider(
  main_ctx_parent parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface> f) noexcept
  : main_ctx_object{"PTx2R8", parent}
  , url_path{path}
  , factory{std::move(f)} {}
//------------------------------------------------------------------------------
auto eagitexi_2d_r8_provider::has_resource(const url& locator) noexcept
  -> bool {
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
//------------------------------------------------------------------------------
auto eagitexi_2d_r8_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagitexi_2d_r8_io>,
      as_parent(),
      factory->make_provider(locator),
      q.arg_value_as<int>("width").value_or(2),
      q.arg_value_as<int>("height").value_or(2),
      q.arg_value_as<int>("level").value_or(0)};
}
//------------------------------------------------------------------------------
void eagitexi_2d_r8_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi://" + url_path);
}
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
// 3d_r8
//------------------------------------------------------------------------------
struct eagitexi_3d_r8_io final : simple_buffer_source_blob_io {

    auto make_header(
      memory::buffer,
      pixel_provider_interface& pixel_provider,
      const int w,
      const int h,
      const int d,
      const int l) -> memory::buffer;

    auto make_data(
      memory::buffer,
      pixel_provider_interface& pixel_provider,
      int w,
      int h,
      int d) -> memory::buffer;

    eagitexi_3d_r8_io(
      main_ctx_parent parent,
      unique_holder<pixel_provider_interface> provider,
      int w,
      int h,
      int d,
      int l);
};
//------------------------------------------------------------------------------
auto eagitexi_3d_r8_io::make_header(
  memory::buffer header,
  pixel_provider_interface&,
  const int w,
  const int h,
  const int d,
  const int l) -> memory::buffer {
    std::stringstream hdr;
    hdr << R"({"level":)" << l;
    hdr << R"(,"width":)" << w;
    hdr << R"(,"height":)" << h;
    hdr << R"(,"depth":)" << d;
    hdr << R"(,"channels":1)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"red")";
    hdr << R"(,"iformat":"r8")";
    hdr << R"(,"tag":["generated"])";
    hdr << R"(,"data_filter":"zlib")";
    hdr << '}';
    memory::copy_into(as_bytes(string_view{hdr.str()}), header);
    return header;
}
//------------------------------------------------------------------------------
auto eagitexi_3d_r8_io::make_data(
  memory::buffer data,
  pixel_provider_interface& pixel_provider,
  int w,
  int h,
  int d) -> memory::buffer {
    const auto append{[&](memory::const_block packed) {
        memory::append_to(packed, data);
        return true;
    }};
    stream_compression compress{
      main_context().compressor(),
      {construct_from, append},
      default_data_compression_method()};

    std::array<byte, 16 * 1024> buf;
    std::size_t i{0U};

    assert(pixel_provider.pixel_byte_count() == 1);
    for(int z = 0; z < d; ++z) {
        for(int y = 0; y < h; ++y) {
            for(int x = 0; x < w; ++x) {
                buf[i++] = pixel_provider.pixel_byte(
                  {.width = w, .height = h, .depth = d, .x = x, .y = y, .z = z});
                if(i == buf.size()) {
                    compress.next(view(buf), data_compression_level::highest);
                    i = 0U;
                }
            }
        }
    }
    if(i > 0) {
        compress.next(
          head(view(buf), span_size(i)), data_compression_level::highest);
    }
    compress.finish();
    return data;
}
//------------------------------------------------------------------------------
eagitexi_3d_r8_io::eagitexi_3d_r8_io(
  main_ctx_parent parent,
  unique_holder<pixel_provider_interface> provider,
  int w,
  int h,
  int d,
  int l)
  : simple_buffer_source_blob_io{
      "ITx3R8",
      parent,
      provider->estimated_data_size(w, h, d),
      [&](memory::buffer content) {
          return make_data(
            make_header(std::move(content), *provider, w, h, d, l),
            *provider,
            w,
            h,
            d);
      }} {}
//------------------------------------------------------------------------------
struct eagitexi_3d_r8_provider final
  : main_ctx_object
  , resource_provider_interface {
    std::string url_path;
    shared_holder<pixel_provider_factory_interface> factory;

    eagitexi_3d_r8_provider(
      main_ctx_parent parent,
      std::string path,
      shared_holder<pixel_provider_factory_interface> f) noexcept;

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 0) and (d <= 16 * 1024);
    }

    static auto valid_lvl(int l) noexcept -> bool {
        return (l >= 0);
    }

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
eagitexi_3d_r8_provider::eagitexi_3d_r8_provider(
  main_ctx_parent parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface> f) noexcept
  : main_ctx_object{"PTx3R8", parent}
  , url_path{path}
  , factory{std::move(f)} {}
//------------------------------------------------------------------------------
auto eagitexi_3d_r8_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(locator.has_scheme("eagitexi") and locator.has_path(url_path)) {
        const auto& q{locator.query()};
        const bool args_ok =
          valid_dim(q.arg_value_as<int>("width").value_or(1)) and
          valid_dim(q.arg_value_as<int>("height").value_or(1)) and
          valid_dim(q.arg_value_as<int>("depth").value_or(1)) and
          valid_lvl(q.arg_value_as<int>("level").value_or(1));
        return args_ok and factory->has_resource(locator);
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_3d_r8_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagitexi_3d_r8_io>,
      as_parent(),
      factory->make_provider(locator),
      q.arg_value_as<int>("width").value_or(2),
      q.arg_value_as<int>("height").value_or(2),
      q.arg_value_as<int>("depth").value_or(2),
      q.arg_value_as<int>("level").value_or(0)};
}
//------------------------------------------------------------------------------
void eagitexi_3d_r8_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi://" + url_path);
}
//------------------------------------------------------------------------------
auto provider_eagitexi_3d_r8(
  main_ctx_parent parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface> factory)
  -> unique_holder<resource_provider_interface> {
    assert(factory);
    return {
      hold<eagitexi_3d_r8_provider>,
      parent,
      std::move(path),
      std::move(factory)};
}
//------------------------------------------------------------------------------
// 2d_rgb8
//------------------------------------------------------------------------------
struct eagitexi_2d_rgb8_io final : simple_buffer_source_blob_io {

    auto make_header(
      memory::buffer,
      pixel_provider_interface& pixel_provider,
      const int w,
      const int h,
      const int l) -> memory::buffer;

    auto make_data(
      memory::buffer,
      pixel_provider_interface& pixel_provider,
      int w,
      int h) -> memory::buffer;

    eagitexi_2d_rgb8_io(
      main_ctx_parent parent,
      unique_holder<pixel_provider_interface> provider,
      int w,
      int h,
      int l);
};
//------------------------------------------------------------------------------
auto eagitexi_2d_rgb8_io::make_header(
  memory::buffer header,
  pixel_provider_interface&,
  const int w,
  const int h,
  const int l) -> memory::buffer {
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
    memory::copy_into(as_bytes(string_view{hdr.str()}), header);
    return header;
}
//------------------------------------------------------------------------------
auto eagitexi_2d_rgb8_io::make_data(
  memory::buffer data,
  pixel_provider_interface& pixel_provider,
  int w,
  int h) -> memory::buffer {
    const auto append{[&](memory::const_block packed) {
        memory::append_to(packed, data);
        return true;
    }};
    stream_compression compress{
      main_context().compressor(),
      {construct_from, append},
      default_data_compression_method()};

    std::array<byte, 4 * 1024> buf;
    std::size_t i{0U};

    assert(pixel_provider.pixel_byte_count() >= 3);
    for(int y = 0; y < h; ++y) {
        for(int x = 0; x < w; ++x) {
            for(int c = 0; c < 3; ++c) {
                buf[i++] = pixel_provider.pixel_byte(
                  {.width = w, .height = h, .x = x, .y = y, .component = c});
                if(i == buf.size()) {
                    compress.next(view(buf), data_compression_level::highest);
                    i = 0U;
                }
            }
        }
    }
    if(i > 0) {
        compress.next(
          head(view(buf), span_size(i)), data_compression_level::highest);
    }
    compress.finish();
    return data;
}
//------------------------------------------------------------------------------
eagitexi_2d_rgb8_io::eagitexi_2d_rgb8_io(
  main_ctx_parent parent,
  unique_holder<pixel_provider_interface> provider,
  int w,
  int h,
  int l)
  : simple_buffer_source_blob_io{
      "ITx2RGB8",
      parent,
      provider->estimated_data_size(w, h, 1),
      [&](memory::buffer content) {
          return make_data(
            make_header(std::move(content), *provider, w, h, l),
            *provider,
            w,
            h);
      }} {}
//------------------------------------------------------------------------------
struct eagitexi_2d_rgb8_provider final
  : main_ctx_object
  , resource_provider_interface {
    std::string url_path;
    shared_holder<pixel_provider_factory_interface> factory;

    eagitexi_2d_rgb8_provider(
      main_ctx_parent parent,
      std::string path,
      shared_holder<pixel_provider_factory_interface> f) noexcept;

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_lvl(int l) noexcept -> bool {
        return (l >= 0);
    }

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
eagitexi_2d_rgb8_provider::eagitexi_2d_rgb8_provider(
  main_ctx_parent parent,
  std::string path,
  shared_holder<pixel_provider_factory_interface> f) noexcept
  : main_ctx_object{"PTx2RGB8", parent}
  , url_path{path}
  , factory{std::move(f)} {}
//------------------------------------------------------------------------------
auto eagitexi_2d_rgb8_provider::has_resource(const url& locator) noexcept
  -> bool {
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
//------------------------------------------------------------------------------
auto eagitexi_2d_rgb8_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagitexi_2d_rgb8_io>,
      as_parent(),
      factory->make_provider(locator),
      q.arg_value_as<int>("width").value_or(2),
      q.arg_value_as<int>("height").value_or(2),
      q.arg_value_as<int>("level").value_or(0)};
}
//------------------------------------------------------------------------------
void eagitexi_2d_rgb8_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    return callback("eagitexi://" + url_path);
}
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
