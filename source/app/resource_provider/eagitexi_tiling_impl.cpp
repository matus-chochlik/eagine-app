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
// tiling I/O
//------------------------------------------------------------------------------
class eagitexi_tiling_io final : public compressed_buffer_source_blob_io {
public:
    eagitexi_tiling_io(
      main_ctx_parent,
      msgbus::resource_data_consumer_node&,
      url);

    auto prepare() noexcept -> msgbus::blob_preparation final;

private:
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
  : compressed_buffer_source_blob_io{"ITxTlng", parent, 1024 * 1024}
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
  , _request_id{std::get<0>(consumer.stream_resource(std::move(source)))} {
    append(R"({"level":0,"channels":1,"data_type":"unsigned_byte")");
    append(R"(,"tag":["tiling"])");
    append(R"(,"format":"red_integer","iformat":"r8ui")");
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_io::prepare() noexcept -> msgbus::blob_preparation {
    const auto result = (_finished or _canceled)
                          ? msgbus::blob_preparation::finished
                          : msgbus::blob_preparation::working;
    if(_last) {
        _finished = true;
    }
    return result;
}
//------------------------------------------------------------------------------
void eagitexi_tiling_io::_process_cell(const byte b) {
    compress(view_one(b));
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
        finish();
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
// tiling image provider
//------------------------------------------------------------------------------
struct eagitexi_tiling_provider final
  : main_ctx_object
  , resource_provider_interface {

    msgbus::resource_data_consumer_node& consumer;

    eagitexi_tiling_provider(const provider_parameters& p) noexcept
      : main_ctx_object{"PTxTlng", p.parent}
      , consumer{p.consumer} {}

    auto valid_source(const url& locator) noexcept -> bool;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t) noexcept
      -> std::chrono::seconds final {
        return std::chrono::minutes{2};
    }

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
auto eagitexi_tiling_provider::valid_source(const url& locator) noexcept
  -> bool {
    return locator and
           (locator.has_scheme("text") or locator.has_path_suffix(".text") or
            locator.has_path_suffix(".txt"));
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_provider::has_resource(const url& locator) noexcept
  -> bool {
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
//------------------------------------------------------------------------------
auto eagitexi_tiling_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagitexi_tiling_io>,
      as_parent(),
      consumer,
      q.decoded_arg_value("source").or_default()};
}
//------------------------------------------------------------------------------
void eagitexi_tiling_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///tiling");
}
//------------------------------------------------------------------------------
// tiling image provider
//------------------------------------------------------------------------------
auto provider_eagitexi_tiling(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_tiling_provider>, p};
}
//------------------------------------------------------------------------------
// tiling data view
//------------------------------------------------------------------------------
class tiling_data;
class tiling_data_view {
public:
    tiling_data_view(
      const tiling_data& parent,
      span_size_t x,
      span_size_t y,
      span_size_t w,
      span_size_t h) noexcept
      : _parent{parent}
      , _xoffs{x}
      , _yoffs{y}
      , _width{w}
      , _height{h} {}

    auto get_cell(span_size_t x, span_size_t y) const noexcept -> float;

    auto get(float x, float y) const noexcept -> float;

private:
    auto _get_samples(float x, float y) const noexcept;
    auto _get_weights(float x, float y) const noexcept;

    template <std::size_t... I>
    auto _weighted_sample(float x, float y) const noexcept -> float;

    const tiling_data& _parent;
    span_size_t _xoffs;
    span_size_t _yoffs;
    span_size_t _width;
    span_size_t _height;
};
//------------------------------------------------------------------------------
auto tiling_data_view::_get_samples(float x, float y) const noexcept {
    using std::floor;

    const float o{0.5F};
    const std::array<std::array<span_size_t, 2>, 4> sample_points{
      {{{span_size_t(floor(x - o)), span_size_t(floor(y - o))}},
       {{span_size_t(floor(x + o)), span_size_t(floor(y - o))}},
       {{span_size_t(floor(x - o)), span_size_t(floor(y + o))}},
       {{span_size_t(floor(x + o)), span_size_t(floor(y + o))}}}};

    const auto val{[&, this](std::size_t i) {
        const auto [cx, cy]{sample_points[i]};
        return get_cell(cx, cy);
    }};

    return std::array<float, 4>{{val(0), val(1), val(2), val(3)}};
}
//------------------------------------------------------------------------------
auto tiling_data_view::_get_weights(float x, float y) const noexcept {
    const float mx{x + 0.5F - float(int(x + 0.5F))};
    const float my{y + 0.5F - float(int(y + 0.5F))};

    const std::array<std::array<float, 2>, 4> weights{
      {{{1.F - mx, 1.F - my}}, {{mx, 1.F - my}}, {{1.F - mx, my}}, {{mx, my}}}};

    const auto coef{[&](std::size_t i) {
        const auto [wx, wy]{weights[i]};
        return math::sigmoid01(wx * wy, 1.41F);
    }};

    return std::array<float, 4>{{coef(0), coef(1), coef(2), coef(3)}};
}
//------------------------------------------------------------------------------
template <std::size_t... I>
auto tiling_data_view::_weighted_sample(float x, float y) const noexcept
  -> float {
    const auto s{_get_samples(x, y)};
    const auto w{_get_weights(x, y)};

    return (... + (s[I] * w[I])) / (... + w[I]);
}
//------------------------------------------------------------------------------
auto tiling_data_view::get(float x, float y) const noexcept -> float {
    return _weighted_sample<0, 1, 2, 3>(x, y);
}
//------------------------------------------------------------------------------
// tiling data
//------------------------------------------------------------------------------
class tiling_data : public main_ctx_object {
public:
    tiling_data(main_ctx_parent, msgbus::resource_data_consumer_node&, url);

    auto is_loaded() const noexcept -> bool;

    auto width() const noexcept -> span_size_t {
        return _width;
    }

    auto height() const noexcept -> span_size_t {
        return _height;
    }

    auto get_cell(
      span_size_t xo,
      span_size_t yo,
      span_size_t x,
      span_size_t y,
      span_size_t w,
      span_size_t h) const noexcept -> float;

    auto whole() const noexcept -> tiling_data_view {
        return {*this, 0, 0, _width, _height};
    }

private:
    static auto _wrap_coord(
      span_size_t o,
      span_size_t i,
      span_size_t n,
      span_size_t m) noexcept -> span_size_t;

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

    std::vector<float> _data;
    std::string _tiling_line;
    span_size_t _width{0};
    span_size_t _height{0};
    bool _finished{false};
    bool _canceled{false};
};
//------------------------------------------------------------------------------
auto tiling_data_view::get_cell(span_size_t x, span_size_t y) const noexcept
  -> float {
    return _parent.get_cell(_xoffs, _yoffs, x, y, _width, _height);
}
//------------------------------------------------------------------------------
tiling_data::tiling_data(
  main_ctx_parent parent,
  msgbus::resource_data_consumer_node& consumer,
  url source)
  : main_ctx_object{"TilingData", parent}
  , _appended_binding{consumer.blob_stream_data_appended.bind(
      {this,
       member_function_constant_t<&tiling_data::_handle_stream_data_appended>{}})}
  , _finished_binding{consumer.blob_stream_finished.bind(
      {this,
       member_function_constant_t<&tiling_data::_handle_stream_finished>{}})}
  , _canceled_binding{consumer.blob_stream_cancelled.bind(
      {this,
       member_function_constant_t<&tiling_data::_handle_stream_canceled>{}})}
  , _request_id{std::get<0>(consumer.stream_resource(std::move(source)))} {
    _data.reserve(1024U * 1024U);
}
//------------------------------------------------------------------------------
void tiling_data::_process_cell(const byte b) {
    _data.push_back(float(b) / float(16));
}
//------------------------------------------------------------------------------
auto tiling_data::is_loaded() const noexcept -> bool {
    return _finished and (_width > 0) and (_height > 0);
}
//------------------------------------------------------------------------------
auto tiling_data::_wrap_coord(
  span_size_t o,
  span_size_t i,
  span_size_t n,
  span_size_t m) noexcept -> span_size_t {
    if(i < 0) {
        const auto k{-i / n + 1};
        i = i + k * n;
    }
    i = i % n;

    i = (i + o) % m;

    return i;
}
//------------------------------------------------------------------------------
auto tiling_data::get_cell(
  span_size_t xo,
  span_size_t yo,
  span_size_t x,
  span_size_t y,
  span_size_t w,
  span_size_t h) const noexcept -> float {
    x = _wrap_coord(xo, x, w, _width);
    y = _wrap_coord(yo, y, h, _height);
    const auto k{std_size(_width * y + x)};
    assert(k < _data.size());
    return _data[k];
}
//------------------------------------------------------------------------------
void tiling_data::_process_line(const string_view line) {
    if(not line.empty()) {
        if(_width == 0) {
            _width = line.size();
        }
        ++_height;
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
}
//------------------------------------------------------------------------------
void tiling_data::_handle_stream_data_appended(
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
void tiling_data::_handle_stream_finished(
  const msgbus::blob_id_t request_id) noexcept {
    if(_request_id == request_id) {
        _process_line(_tiling_line);
        _tiling_line.clear();
        _finished = true;
    }
}
//------------------------------------------------------------------------------
void tiling_data::_handle_stream_canceled(
  const msgbus::blob_id_t request_id) noexcept {
    if(_request_id == request_id) {
        _canceled = true;
    }
}
//------------------------------------------------------------------------------
// noise I/O
//------------------------------------------------------------------------------
class eagitexi_tiling_noise_io final : public simple_buffer_source_blob_io {
public:
    eagitexi_tiling_noise_io(
      main_ctx_parent,
      msgbus::resource_data_consumer_node&,
      span_size_t width,
      span_size_t height,
      url);

    auto prepare() noexcept -> msgbus::blob_preparation final;

private:
    auto _get_noise(float x, float y, std::size_t i) const noexcept -> float;

    span_size_t _width;
    span_size_t _height;

    tiling_data _tiling;
    std::vector<std::tuple<float, tiling_data_view>> _octaves;
    std::vector<float> _noise;
    std::size_t _pixel_index{0};
    std::size_t _value_index{0};
    bool _header_done{false};
    bool _values_done{false};
};
//------------------------------------------------------------------------------
eagitexi_tiling_noise_io::eagitexi_tiling_noise_io(
  main_ctx_parent parent,
  msgbus::resource_data_consumer_node& consumer,
  span_size_t width,
  span_size_t height,
  url locator)
  : simple_buffer_source_blob_io{"ITxTlgNois", parent, 1024 * 1024}
  , _width{width}
  , _height{height}
  , _tiling{as_parent(), consumer, std::move(locator)} {
    append(R"({"level":0,"channels":1,"data_type":"float")"
           R"(,"format":"red","iformat":"r32f")"
           R"(,"tag":["generated","noise","sudoku"])");
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_noise_io::_get_noise(float x, float y, std::size_t i)
  const noexcept -> float {
    const auto [div, data]{_octaves[i]};
    return data.get(x / div, y / div);
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_noise_io::prepare() noexcept -> msgbus::blob_preparation {
    if(not _tiling.is_loaded()) {
        return msgbus::blob_preparation::working;
    }
    if(_width == 0) {
        _width = _tiling.width();
    }
    if(_height == 0) {
        _height = _tiling.height();
    }
    if(_octaves.empty()) {
        _octaves.emplace_back(1.F, _tiling.whole());
        return msgbus::blob_preparation::working;
    }
    if(_noise.empty()) {
        _noise.resize(std_size(_tiling.width() * _tiling.height()));
        return msgbus::blob_preparation::working;
    }
    if(_pixel_index < _noise.size()) {
        for(int i = 0; i < 256 and _pixel_index < _noise.size();
            ++i, ++_pixel_index) {
            const auto y{float(_pixel_index / _tiling.width())};
            const auto x{float(_pixel_index % _tiling.width())};
            for(const auto o : index_range(_octaves)) {
                _noise[_pixel_index] = _get_noise(x, y, o);
            }
        }
        return msgbus::blob_preparation::working;
    }
    if(not _header_done) {
        std::stringstream header;
        header << R"(,"width":)" << _width;
        header << R"(,"height":)" << _height;
        header << R"(,"depth":1)";
        header << R"(,"data":[)";
        append(header.str());

        _header_done = true;
        return msgbus::blob_preparation::working;
    }

    if(_value_index < _noise.size()) {
        const auto strip_trailing_zeroes{[](std::string str) {
            while(not str.empty() and str.back() == '0') {
                str.pop_back();
            }
            if(not str.empty() and str.back() == '.') {
                str.push_back('0');
            }
            return str;
        }};
        const auto elem_str{[&](float elem) {
            return strip_trailing_zeroes(std::to_string(elem));
        }};
        for(int i = 0; i < 4096 and _value_index < _noise.size();
            ++i, ++_value_index) {
            if(_value_index) {
                append(",");
            }
            append(elem_str(_noise[_value_index]));
        }
        return msgbus::blob_preparation::working;
    }

    if(not _values_done) {
        append("]}");
        _values_done = true;
        return msgbus::blob_preparation::working;
    }

    return msgbus::blob_preparation::finished;
}
//------------------------------------------------------------------------------
// noise image provider
//------------------------------------------------------------------------------
struct eagitexi_tiling_noise_provider final
  : main_ctx_object
  , resource_provider_interface {

    msgbus::resource_data_consumer_node& consumer;

    eagitexi_tiling_noise_provider(const provider_parameters& p) noexcept
      : main_ctx_object{"PTxTlgNois", p.parent}
      , consumer{p.consumer} {}

    auto valid_source(const url& locator) noexcept -> bool;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t) noexcept
      -> std::chrono::seconds final {
        return std::chrono::minutes{5};
    }

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
auto eagitexi_tiling_noise_provider::valid_source(const url& locator) noexcept
  -> bool {
    return locator and
           (locator.has_scheme("text") or locator.has_path_suffix(".text") or
            locator.has_path_suffix(".txt"));
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_noise_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(locator.has_scheme("eagitexi") and locator.has_path("/tiling_noise")) {
        const auto& q{locator.query()};
        const bool args_ok =
          (q.arg_value_as<span_size_t>("width").value_or(1) > 0) and
          (q.arg_value_as<span_size_t>("height").value_or(1) > 0) and
          (q.decoded_arg_value("source")
             .and_then([this](auto source) -> tribool {
                 return valid_source(std::move(source));
             })
             .or_false());
        return args_ok;
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_noise_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagitexi_tiling_noise_io>,
      as_parent(),
      consumer,
      q.arg_value_as<span_size_t>("width").value_or(0),
      q.arg_value_as<span_size_t>("height").value_or(0),
      q.decoded_arg_value("source").or_default()};
}
//------------------------------------------------------------------------------
void eagitexi_tiling_noise_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///tiling_noise");
}
//------------------------------------------------------------------------------
// noise image provider
//------------------------------------------------------------------------------
auto provider_eagitexi_tiling_noise(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_tiling_noise_provider>, p};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
