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
// transition mask interface
//------------------------------------------------------------------------------
struct tiling_transition_mask : interface<tiling_transition_mask> {
    virtual auto prepare() noexcept -> msgbus::blob_preparation = 0;
    virtual auto width() noexcept -> valid_if_positive<int> = 0;
    virtual auto height() noexcept -> valid_if_positive<int> = 0;
    virtual auto batch_size() noexcept -> int = 0;
    virtual auto value(int x, int y) noexcept -> bool = 0;
};
//------------------------------------------------------------------------------
// checker transition mask
//------------------------------------------------------------------------------
class tiling_transition_checker final : public tiling_transition_mask {
public:
    tiling_transition_checker(
      main_ctx_parent,
      msgbus::resource_data_consumer_node&,
      const url& locator) noexcept;

    static auto is_valid_locator(const url& locator) noexcept -> bool;

    auto prepare() noexcept -> msgbus::blob_preparation final {
        return msgbus::blob_preparation::finished;
    }

    auto width() noexcept -> valid_if_positive<int> final {
        return _width;
    }
    auto height() noexcept -> valid_if_positive<int> final {
        return _height;
    }
    auto batch_size() noexcept -> int final {
        return 25000;
    }
    auto value(int x, int y) noexcept -> bool final {
        x /= 4;
        y /= 4;
        return ((x % 2) + (y % 2)) % 2 != 0;
    }

private:
    int _width;
    int _height;
};
//------------------------------------------------------------------------------
tiling_transition_checker::tiling_transition_checker(
  main_ctx_parent,
  msgbus::resource_data_consumer_node&,
  const url& locator) noexcept
  : _width{locator.query().arg_value_as<int>("width").value_or(64)}
  , _height{locator.query().arg_value_as<int>("height").value_or(64)} {}
//------------------------------------------------------------------------------
auto tiling_transition_checker::is_valid_locator(const url& locator) noexcept
  -> bool {
    const auto& q{locator.query()};
    return q.arg_has_value("source", "checker");
}
//------------------------------------------------------------------------------
// transition mask from tiling
//------------------------------------------------------------------------------
class tiling_transition_tiling final
  : public main_ctx_object
  , public tiling_transition_mask {
public:
    tiling_transition_tiling(
      main_ctx_parent,
      msgbus::resource_data_consumer_node&,
      const url& locator) noexcept;

    static auto is_valid_locator(const url& locator) noexcept -> bool;

    auto prepare() noexcept -> msgbus::blob_preparation final;

    auto width() noexcept -> valid_if_positive<int> final;
    auto height() noexcept -> valid_if_positive<int> final;
    auto batch_size() noexcept -> int final;
    auto value(int x, int y) noexcept -> bool final;

private:
    static auto _get_source(const url&) noexcept -> url;

    void _handle_stream_data_appended(
      const msgbus::blob_stream_chunk& chunk) noexcept;
    void _handle_stream_finished(const msgbus::blob_id_t) noexcept;
    void _handle_stream_canceled(const msgbus::blob_id_t) noexcept;

    signal_binding _appended_binding;
    signal_binding _finished_binding;
    signal_binding _canceled_binding;

    identifier_t _request_id;
    std::string _tiling_line;
    std::vector<std::string> _tiling;
    int _threshold{8}; // TODO from url
    bool _first{true};
    bool _last{false};
    bool _finished{false};
    bool _canceled{false};
};
//------------------------------------------------------------------------------
auto tiling_transition_tiling::_get_source(const url& locator) noexcept -> url {
    return locator.query().arg_url("source");
}
//------------------------------------------------------------------------------
tiling_transition_tiling::tiling_transition_tiling(
  main_ctx_parent parent,
  msgbus::resource_data_consumer_node& consumer,
  const url& locator) noexcept
  : main_ctx_object{"TlgTrnsTlg", parent}
  , _appended_binding{consumer.blob_stream_data_appended.bind(
      {this,
       member_function_constant_t<
         &tiling_transition_tiling::_handle_stream_data_appended>{}})}
  , _finished_binding{consumer.blob_stream_finished.bind(
      {this,
       member_function_constant_t<
         &tiling_transition_tiling::_handle_stream_finished>{}})}
  , _canceled_binding{consumer.blob_stream_cancelled.bind(
      {this,
       member_function_constant_t<
         &tiling_transition_tiling::_handle_stream_canceled>{}})}
  , _request_id{std::get<0>(consumer.stream_resource(_get_source(locator)))} {}
//------------------------------------------------------------------------------
auto tiling_transition_tiling::is_valid_locator(const url& locator) noexcept
  -> bool {
    if(const auto source{locator.query().arg_url("source")}) {
        return source.has_scheme("text") or source.has_path_suffix(".text") or
               source.has_path_suffix(".txt");
    }
    return false;
}
//------------------------------------------------------------------------------
auto tiling_transition_tiling::prepare() noexcept -> msgbus::blob_preparation {
    const auto result = (_finished or _canceled)
                          ? msgbus::blob_preparation::finished
                          : msgbus::blob_preparation::working;
    if(_last) {
        _finished = true;
    }
    return result;
}
//------------------------------------------------------------------------------
auto tiling_transition_tiling::width() noexcept -> valid_if_positive<int> {
    if(not _tiling.empty()) {
        return {limit_cast<int>(_tiling.front().size())};
    }
    return {};
}
//------------------------------------------------------------------------------
auto tiling_transition_tiling::height() noexcept -> valid_if_positive<int> {
    return {limit_cast<int>(_tiling.size())};
}
//------------------------------------------------------------------------------
auto tiling_transition_tiling::batch_size() noexcept -> int {
    return 20000;
}
//------------------------------------------------------------------------------
auto tiling_transition_tiling::value(int x, int y) noexcept -> bool {
    const auto evaluate{[this](const char c) {
        const auto compare{[this](const int i) {
            return i >= _threshold;
        }};
        if('0' <= c and c <= '9') {
            return compare(int(c - '0'));
        } else if('A' <= c and c <= 'F') {
            return compare(int(c - 'A' + 10));
        } else if('a' <= c and c <= 'f') {
            return compare(int(c - 'a' + 10));
        } else {
            return compare(int(0));
        }
    }};
    if(not _tiling.empty()) {
        y = y % limit_cast<int>(_tiling.size());
        if(not _tiling[std_size(y)].empty()) {
            x = x % limit_cast<int>(_tiling[std_size(y)].size());
            return evaluate(_tiling[std_size(y)][std_size(x)]);
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void tiling_transition_tiling::_handle_stream_data_appended(
  const msgbus::blob_stream_chunk& chunk) noexcept {
    if(_request_id == chunk.request_id) {
        const string_view sep{"\n"};
        for(const auto blk : chunk.data) {
            auto text{as_chars(blk)};
            while(not text.empty()) {
                if(const auto pos{memory::find_position(text, sep)}) {
                    append_to(head(text, *pos), _tiling_line);
                    text = skip(text, *pos + sep.size());
                    _tiling.emplace_back(std::move(_tiling_line));
                } else {
                    append_to(text, _tiling_line);
                    text = {};
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void tiling_transition_tiling::_handle_stream_finished(
  const msgbus::blob_id_t request_id) noexcept {
    if(_request_id == request_id) {
        _tiling.emplace_back(std::move(_tiling_line));
        _last = true;
    }
}
//------------------------------------------------------------------------------
void tiling_transition_tiling::_handle_stream_canceled(
  const msgbus::blob_id_t request_id) noexcept {
    if(_request_id == request_id) {
        _canceled = true;
    }
}
//------------------------------------------------------------------------------
// transition mask factory
//------------------------------------------------------------------------------
class tiling_transition_mask_factory : public main_ctx_object {
public:
    tiling_transition_mask_factory(
      main_ctx_parent parent,
      msgbus::resource_data_consumer_node& consumer) noexcept
      : main_ctx_object{"TiTrMskFac", parent}
      , _consumer{consumer} {}

    auto is_valid_locator(const url& locator) noexcept -> bool;

    auto make_mask(const url& locator) -> unique_holder<tiling_transition_mask>;

private:
    template <typename... M>
    static auto _is_valid_locator(const url& locator, mp_list<M...>) noexcept
      -> bool {
        return (false or ... or M::is_valid_locator(locator));
    }

    static auto _make_mask(const url&, mp_list<>) noexcept
      -> unique_holder<tiling_transition_mask> {
        return {};
    }

    template <typename M, typename... Ms>
    auto _make_mask(const url& locator, mp_list<M, Ms...>) noexcept
      -> unique_holder<tiling_transition_mask> {
        if(M::is_valid_locator(locator)) {
            return {hold<M>, as_parent(), _consumer, locator};
        }
        return _make_mask(locator, mp_list<Ms...>{});
    }

    msgbus::resource_data_consumer_node& _consumer;
    mp_list<tiling_transition_checker, tiling_transition_tiling> _masks{};
};
//------------------------------------------------------------------------------
auto tiling_transition_mask_factory::is_valid_locator(
  const url& locator) noexcept -> bool {
    return _is_valid_locator(locator, _masks);
}
//------------------------------------------------------------------------------
auto tiling_transition_mask_factory::make_mask(const url& locator)
  -> unique_holder<tiling_transition_mask> {
    return _make_mask(locator, _masks);
}
//------------------------------------------------------------------------------
// tiling transition image I/O
//------------------------------------------------------------------------------
class eagitexi_tiling_transition_io final
  : public compressed_buffer_source_blob_io {
public:
    eagitexi_tiling_transition_io(
      main_ctx_parent,
      shared_holder<tiling_transition_mask>) noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation final;

private:
    auto value(int x, int y, int ox, int oy) noexcept -> bool;

    static auto _code(const std::array<std::array<bool, 3>, 3>& k) noexcept
      -> byte;

    auto _code_index(byte c) noexcept -> byte;

    auto _element(int x, int y) noexcept -> byte;

    static auto _make_code_map() noexcept -> flat_set<byte>;

    static auto _w(const shared_holder<tiling_transition_mask>& m) noexcept {
        return m.member(&tiling_transition_mask::width).value_or(64);
    }

    static auto _h(const shared_holder<tiling_transition_mask>& m) noexcept {
        return m.member(&tiling_transition_mask::height).value_or(64);
    }

    static auto _buf_size(
      const shared_holder<tiling_transition_mask>& m) noexcept {
        return _w(m) * _h(m);
    }

    const flat_set<byte> _code_map{_make_code_map()};
    const shared_holder<tiling_transition_mask> _mask;
    const int _width;
    const int _height;
    int _x{0};
    int _y{0};
    bool _done{false};
};
//------------------------------------------------------------------------------
eagitexi_tiling_transition_io::eagitexi_tiling_transition_io(
  main_ctx_parent parent,
  shared_holder<tiling_transition_mask> mask) noexcept
  : compressed_buffer_source_blob_io{"TiTrTexIO", parent, _buf_size(mask)}
  , _mask{std::move(mask)}
  , _width{_w(_mask)}
  , _height{_h(_mask)} {
    append(R"({"level":0,"channels":1,"data_type":"unsigned_byte")");
    append(R"(,"tag":["generator","transition"])");
    append(R"(,"format":"red_integer","iformat":"r8ui")");
    std::stringstream hdr;
    hdr << R"(,"width":)" << _width;
    hdr << R"(,"height":)" << _height;
    append(hdr.str());
    append(R"(,"data_filter":"zlib"})");
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::value(int x, int y, int ox, int oy) noexcept
  -> bool {
    assert(_mask);
    return _mask->value(
      (x + _width + ox) % _width, (y + _height + oy) % _height);
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::_make_code_map() noexcept
  -> flat_set<byte> {
    flat_set<byte> result;
    result.reserve(48);

    std::array<std::array<bool, 3>, 3> k{};
    for(std::size_t i = 0; i < 512; ++i) {
        for(std::size_t by = 0; by < 3; ++by) {
            for(std::size_t bx = 0; bx < 3; ++bx) {
                const std::size_t b{(0x1U << (by * 3U + bx))};
                k[bx][by] = ((i & b) == b);
            }
        }
        result.insert(_code(k));
    }
    return result;
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::_code(
  const std::array<std::array<bool, 3>, 3>& k) noexcept -> byte {
    byte b{0x00U};
    if(k[1][1]) {
        b = 0xFFU;
    } else {
        if(k[1][0]) {
            b |= 0x01U;
        }
        if(k[0][1]) {
            b |= 0x02U;
        }
        if(k[2][1]) {
            b |= 0x04U;
        }
        if(k[1][2]) {
            b |= 0x08U;
        }

        if(k[0][0] and not k[1][0] and not k[0][1]) {
            b |= 0x10U;
        }
        if(k[2][0] and not k[1][0] and not k[2][1]) {
            b |= 0x20U;
        }
        if(k[0][2] and not k[0][1] and not k[1][2]) {
            b |= 0x40U;
        }
        if(k[2][2] and not k[2][1] and not k[1][2]) {
            b |= 0x80U;
        }
    }
    return b;
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::_code_index(byte c) noexcept -> byte {
    using std::distance;
    return limit_cast<byte>(distance(_code_map.begin(), _code_map.find(c)));
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::_element(int x, int y) noexcept -> byte {
    return _code_index(_code(
      {{{{value(x, y, -1, -1), value(x, y, -1, 0), value(x, y, -1, 1)}},
        {{value(x, y, 0, -1), value(x, y, 0, 0), value(x, y, 0, 1)}},
        {{value(x, y, 1, -1), value(x, y, 1, 0), value(x, y, 1, 1)}}}}));
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::prepare() noexcept
  -> msgbus::blob_preparation {
    if(_mask->prepare() == msgbus::blob_preparation::finished) {
        for(int i = 0, n = _mask->batch_size(); i < n; ++i) {
            if(_y >= _height) {
                if(_done) {
                    return msgbus::blob_preparation::finished;
                } else {
                    finish();
                    _done = true;
                    break;
                }
            }
            compress(_element(_x, _y));
            if(++_x >= _width) {
                _x = 0;
                ++_y;
            }
        }
    }

    return msgbus::blob_preparation::working;
}
//------------------------------------------------------------------------------
// tiling transition image provider
//------------------------------------------------------------------------------
class eagitexi_tiling_transition_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagitexi_tiling_transition_provider(const provider_parameters& p) noexcept
      : main_ctx_object{"PTxTTrnstn", p.parent}
      , _mask_factory{as_parent(), p.consumer} {}

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t) noexcept
      -> std::chrono::seconds final {
        return std::chrono::minutes{2};
    }

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    tiling_transition_mask_factory _mask_factory;
};
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_provider::has_resource(
  const url& locator) noexcept -> bool {
    if(locator.has_scheme("eagitexi") and locator.has_path("/tiling_transition")) {
        return _mask_factory.is_valid_locator(locator);
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagitexi_tiling_transition_io>,
      as_parent(),
      _mask_factory.make_mask(locator)};
}
//------------------------------------------------------------------------------
void eagitexi_tiling_transition_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///tiling_transition?source=checker");
}
//------------------------------------------------------------------------------
// tiling transition image provider
//------------------------------------------------------------------------------
auto provider_eagitexi_tiling_transition(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_tiling_transition_provider>, p};
}
//------------------------------------------------------------------------------
} // namespace eagine::app

