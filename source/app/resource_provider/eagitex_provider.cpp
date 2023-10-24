/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "eagitex_provider.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
// 2d_square_io
//------------------------------------------------------------------------------
struct eagitex_2d_square_io final
  : main_ctx_object
  , msgbus::source_blob_io {

    auto make_header(
      int s,
      const int c,
      string_view path,
      string_view data_type,
      string_view format,
      string_view iformat,
      string_view url_args) {
        int l = 0;
        for(int i = s; i > 0; i /= 2) {
            ++l;
        }
        std::stringstream hdr;
        hdr << R"({"levels":)" << l;
        hdr << R"(,"width":)" << s;
        hdr << R"(,"height":)" << s;
        hdr << R"(,"channels":)" << c;
        hdr << R"(,"data_type":")" << data_type << '"';
        hdr << R"(,"format":")" << format << '"';
        hdr << R"(,"iformat":")" << iformat << '"';
        hdr << R"(,"tag":["generated"])";
        hdr << R"(,"images":[)";

        l = 0;
        while(s > 0) {
            hdr << R"({"url":"eagitexi://)" << path;
            hdr << "?level=" << l;
            hdr << "+width=" << s;
            hdr << "+height=" << s;
            hdr << R"(","level":)" << l << '}';

            s /= 2;
            ++l;
        }
        hdr << ']' << '}';
        return hdr.str();
    }

    eagitex_2d_square_io(
      main_ctx_parent parent,
      int s,
      int c,
      string_view path,
      string_view data_type,
      string_view format,
      string_view iformat,
      string_view url_args)
      : main_ctx_object{"ETx2S", parent}
      , _header{make_header(s, c, path, data_type, format, iformat, url_args)} {
    }

    auto total_size() noexcept -> span_size_t final {
        return span_size(_header.size());
    }

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final {
        return copy(head(view(_header), dst.size()), dst).size();
    }

    std::string _header;
};
//------------------------------------------------------------------------------
// 2d_square_provider
//------------------------------------------------------------------------------
struct eagitex_2d_square_provider final
  : main_ctx_object
  , resource_provider_interface {

    eagitex_2d_square_provider(
      main_ctx_parent parent,
      std::string path,
      string_view data_type,
      string_view format,
      string_view iformat,
      int channels) noexcept
      : main_ctx_object{"PETx2S", parent}
      , _path{std::move(path)}
      , _data_type{data_type}
      , _format{format}
      , _iformat{iformat}
      , _channels{channels} {}

    static auto valid_dim(int d) noexcept -> bool {
        return (d > 1) and (d <= 128 * 1024) and
               math::is_positive_power_of_2(d);
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        if(locator.has_scheme("eagitex") and locator.has_path(_path)) {
            const auto& q{locator.query()};
            const bool args_ok =
              valid_dim(q.arg_value_as<int>("size").value_or(1));
            return args_ok;
        }
        return false;
    }

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final {
        const auto& q{locator.query()};
        return {
          hold<eagitex_2d_square_io>,
          as_parent(),
          q.arg_value_as<int>("size").value_or(256),
          _channels,
          _path,
          _data_type,
          _format,
          _iformat,
          "#TODO"};
    }

    std::string _path;
    string_view _data_type;
    string_view _format;
    string_view _iformat;
    int _channels;
};
//------------------------------------------------------------------------------
auto provider_eagitex_2d_square_rgb8ub(main_ctx_parent parent, std::string path)
  -> unique_holder<resource_provider_interface> {
    return {
      hold<eagitex_2d_square_provider>,
      parent,
      std::move(path),
      "unsigned_byte",
      "rgb",
      "rgb8",
      3};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
