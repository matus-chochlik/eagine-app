/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "common.hpp"
#include "providers.hpp"
#include <cassert>

namespace eagine::app {
//------------------------------------------------------------------------------
// tiling I/O
//------------------------------------------------------------------------------
class eagitexi_tiling_io final : public simple_buffer_source_blob_io {
public:
    eagitexi_tiling_io(
      main_ctx_parent,
      msgbus::resource_data_consumer_node&,
      url);

    auto prepare() noexcept -> bool final {
        const bool result = not(_finished or _canceled);
        if(_last) {
            _finished = true;
        }
        return result;
    }

private:
    auto _process_packed(memory::const_block) noexcept -> bool;
    void _process_cell(const byte);
    void _process_line(const string_view);

    void _handle_stream_data_appended(
      const msgbus::blob_stream_chunk& chunk) noexcept;
    void _handle_stream_finished(const msgbus::blob_id_t) noexcept;
    void _handle_stream_canceled(const msgbus::blob_id_t) noexcept;

    signal_binding _appended_binding;
    signal_binding _finished_binding;
    signal_binding _canceled_binding;

    identifier_t _request_id;

    stream_compression _compress;
    std::string _tiling_line;
    bool _first{true};
    bool _last{false};
    bool _finished{false};
    bool _canceled{false};
};
//------------------------------------------------------------------------------
eagitexi_tiling_io::eagitexi_tiling_io(
  main_ctx_parent parent,
  msgbus::resource_data_consumer_node& consumer,
  url source)
  : simple_buffer_source_blob_io{"ITxTlng", parent, 1024 * 1024}
  , _appended_binding{consumer.blob_stream_data_appended.bind(
      {this,
       member_function_constant_t<
         &eagitexi_tiling_io::_handle_stream_data_appended>{}})}
  , _finished_binding{consumer.blob_stream_finished.bind(
      {this,
       member_function_constant_t<
         &eagitexi_tiling_io::_handle_stream_finished>{}})}
  , _canceled_binding{consumer.blob_stream_cancelled.bind(
      {this,
       member_function_constant_t<
         &eagitexi_tiling_io::_handle_stream_canceled>{}})}
  , _request_id{std::get<0>(consumer.stream_resource(std::move(source)))}
  , _compress{
      main_context().compressor(),
      {this,
       member_function_constant_t<&eagitexi_tiling_io::_process_packed>{}},
      default_data_compression_method()} {
    append(R"({"level":0,"channels":1,"data_type":"unsigned_byte")");
    append(R"(,"tag":["tiling"])");
    append(R"(,"format":"red_integer","iformat":"r8ui")");
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_io::_process_packed(memory::const_block packed) noexcept
  -> bool {
    append(packed);
    return true;
}
//------------------------------------------------------------------------------
void eagitexi_tiling_io::_process_cell(const byte b) {
    _compress.next(view_one(b), data_compression_level::highest);
}
//------------------------------------------------------------------------------
void eagitexi_tiling_io::_process_line(const string_view line) {
    if(not line.empty()) {
        if(_first) {
            _first = false;
            std::stringstream hdr;
            hdr << R"(,"width":)" << line.size();
            hdr << R"(,"height":)" << line.size();
            hdr << R"(,"data_filter":"zlib")";
            hdr << '}';
            append(hdr.str());
        }
    }
    for(const char c : line) {
        if('0' <= c and c <= '9') {
            _process_cell(byte(c - '0'));
        } else if('A' <= c and c <= 'F') {
            _process_cell(byte(c - 'A' + 10));
        } else if('a' <= c and c <= 'f') {
            _process_cell(byte(c - 'a' + 10));
        } else {
            _process_cell(byte(0));
        }
    }
}
//------------------------------------------------------------------------------
void eagitexi_tiling_io::_handle_stream_data_appended(
  const msgbus::blob_stream_chunk& chunk) noexcept {
    if(_request_id == chunk.request_id) {
        const string_view sep{"\n"};
        for(const auto blk : chunk.data) {
            auto text{as_chars(blk)};
            while(not text.empty()) {
                if(const auto pos{memory::find_position(text, sep)}) {
                    append_to(head(text, *pos), _tiling_line);
                    text = skip(text, *pos + sep.size());
                    _process_line(_tiling_line);
                    _tiling_line.clear();
                } else {
                    append_to(text, _tiling_line);
                    text = {};
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void eagitexi_tiling_io::_handle_stream_finished(
  const msgbus::blob_id_t request_id) noexcept {
    if(_request_id == request_id) {
        _process_line(_tiling_line);
        _tiling_line.clear();
        _compress.finish();
        _last = true;
    }
}
//------------------------------------------------------------------------------
void eagitexi_tiling_io::_handle_stream_canceled(
  const msgbus::blob_id_t request_id) noexcept {
    if(_request_id == request_id) {
        _canceled = true;
    }
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct eagitexi_tiling_provider final
  : main_ctx_object
  , resource_provider_interface {

    msgbus::resource_data_consumer_node& consumer;

    eagitexi_tiling_provider(const provider_parameters& p) noexcept
      : main_ctx_object{"PTxTlng", p.parent}
      , consumer{p.consumer} {}

    auto valid_source(const url& locator) noexcept -> bool {
        return locator and (locator.has_scheme("text") or
                            locator.has_path_suffix(".text") or
                            locator.has_path_suffix(".txt"));
    }

    auto has_resource(const url& locator) noexcept -> bool final {
        if(locator.has_scheme("eagitexi") and locator.has_path("/tiling")) {
            const auto& q{locator.query()};
            const bool args_ok = q.decoded_arg_value("source")
                                   .and_then([this](auto source) -> tribool {
                                       return valid_source(std::move(source));
                                   })
                                   .or_false();
            return args_ok;
        }
        return false;
    }

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final {
        const auto& q{locator.query()};
        return {
          hold<eagitexi_tiling_io>,
          as_parent(),
          consumer,
          q.decoded_arg_value("source").or_default()};
    }
};
//------------------------------------------------------------------------------
auto provider_eagitexi_tiling(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_tiling_provider>, p};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
