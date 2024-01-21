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
struct tiling_transition_mask : interface<tiling_transition_mask> {
    virtual auto prepare() noexcept -> bool = 0;
    virtual auto get_width() noexcept -> valid_if_positive<int> = 0;
    virtual auto get_height() noexcept -> valid_if_positive<int> = 0;
    virtual auto level(int x, int y) noexcept -> bool = 0;
};
//------------------------------------------------------------------------------
class tiling_transition_checker final : public tiling_transition_mask {
public:
    tiling_transition_checker(
      main_ctx_parent,
      msgbus::resource_data_consumer_node&,
      const url& locator) noexcept;

    static auto is_valid_locator(const url& locator) noexcept -> bool;

    auto prepare() noexcept -> bool final {
        return true;
    }

    auto get_width() noexcept -> valid_if_positive<int> final {
        return _width;
    }
    auto get_height() noexcept -> valid_if_positive<int> final {
        return _height;
    }
    auto level(int x, int y) noexcept -> bool final {
        return (x / 4) + (y / 4) % 2 != 0;
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
    return q.arg_has_value("source", "checker") and q.has_arg<int>("width") and
           q.has_arg<int>("height");
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
    mp_list<tiling_transition_checker> _masks{};
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

    auto prepare() noexcept -> bool final;

private:
    auto level(int x, int y, int ox, int oy) noexcept -> bool;

    auto _code(const std::array<std::array<bool, 3>, 3>& k) noexcept -> byte;

    auto _element(int x, int y) noexcept -> byte;

    static auto _w(const shared_holder<tiling_transition_mask>& m) noexcept {
        return m.member(&tiling_transition_mask::get_width).value_or(64);
    }

    static auto _h(const shared_holder<tiling_transition_mask>& m) noexcept {
        return m.member(&tiling_transition_mask::get_height).value_or(64);
    }

    static auto _buf_size(
      const shared_holder<tiling_transition_mask>& m) noexcept {
        return _w(m) * _h(m);
    }

    shared_holder<tiling_transition_mask> _mask;
    int _width;
    int _height;
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
    append(R"(,"tag":["transition"])");
    append(R"(,"format":"red_integer","iformat":"r8ui")");
    std::stringstream hdr;
    hdr << R"(,"width":)" << _width;
    hdr << R"(,"height":)" << _height;
    append(hdr.str());
    append(R"(,"data_filter":"zlib"})");
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::level(int x, int y, int ox, int oy) noexcept
  -> bool {
    assert(_mask);
    return _mask->level(
      (x + _width + ox) % _width, (y + _height + oy) % _height);
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
auto eagitexi_tiling_transition_io::_element(int x, int y) noexcept -> byte {
    return _code(
      {{{{level(x, y, -1, -1), level(x, y, -1, 0), level(x, y, -1, 1)}},
        {{level(x, y, 0, -1), level(x, y, 0, 0), level(x, y, 0, 1)}},
        {{level(x, y, 1, -1), level(x, y, 1, 0), level(x, y, 1, 1)}}}});
}
//------------------------------------------------------------------------------
auto eagitexi_tiling_transition_io::prepare() noexcept -> bool {
    if(_mask->prepare()) {
        for(int i = 0; i < 1000; ++i) {
            if(_y >= _height) {
                if(_done) {
                    return false;
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

    return true;
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

