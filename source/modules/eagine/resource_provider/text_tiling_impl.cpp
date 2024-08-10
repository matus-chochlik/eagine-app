/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
template <unsigned Rank>
class tiling_io final
  : public main_ctx_object
  , public msgbus::source_blob_io {
public:
    tiling_io(
      main_ctx_parent parent,
      span_size_t width,
      span_size_t height) noexcept
      : main_ctx_object{"TilingIO", parent}
      , _width{width}
      , _height{height} {}

    tiling_io(main_ctx_parent parent, span_size_t size) noexcept
      : tiling_io{parent, size, size} {}

    auto prepare() noexcept -> msgbus::blob_preparation_result final;

    auto total_size() noexcept -> span_size_t final {
        return (_width + 1) * _height;
    }

    auto fetch_fragment(span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    const span_size_t _width;
    const span_size_t _height;

    default_sudoku_board_traits<Rank> _traits{};
    basic_sudoku_tiling<Rank> _tiling{
      _traits,
      _traits.make_generator().generate_medium()};
    basic_sudoku_tile_patch<Rank> _patch{_width, _height};
    using fill_state = typename basic_sudoku_tiling<Rank>::patch_fill_state;
    fill_state _fill{_tiling, _patch, 0, 0};

    activity_progress _prepare_progress{
      main_context().progress(),
      "creating tiling",
      _fill.total_count()};
};
//------------------------------------------------------------------------------
template <unsigned Rank>
auto tiling_io<Rank>::prepare() noexcept -> msgbus::blob_preparation_result {
    if(not _fill.is_done()) {
        for(int s = 0; s < int(1U << (10U - Rank)); ++s) {
            if(_fill.is_done()) {
                break;
            }
            _fill.next();
        }
        _prepare_progress.update_progress(_fill.current_count());
        if(_fill.is_done()) {
            _prepare_progress.finish();
        }
        return {_fill.current_count(), _fill.total_count()};
    }
    return {msgbus::blob_preparation_status::finished};
}
//------------------------------------------------------------------------------
template <unsigned Rank>
auto tiling_io<Rank>::fetch_fragment(
  span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    span_size_t done{0};
    for(byte& b : dst) {
        if(offs >= total_size()) [[unlikely]] {
            break;
        }
        const auto x{int(offs % (_width + 1))};
        if(x == _width) {
            b = byte('\n');
        } else {
            const auto y{int(offs / (_width + 1))};
            if(auto g{_traits.to_string(_patch.get_glyph(x, y))}) {
                b = byte(g->front());
            } else {
                b = byte('.');
            }
        }
        ++offs;
        ++done;
    }
    return done;
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
template <unsigned Rank>
class tiling_provider final
  : public main_ctx_object
  , public resource_provider_interface {

    static auto _path(unsigned_constant<3U>) noexcept -> string_view {
        return "/tiling3";
    }
    static auto _loc(unsigned_constant<3U>) noexcept -> string_view {
        return "text:///tiling3";
    }
    static auto _path(unsigned_constant<4U>) noexcept -> string_view {
        return "/tiling4";
    }
    static auto _loc(unsigned_constant<4U>) noexcept -> string_view {
        return "text:///tiling4";
    }
    static auto _path(unsigned_constant<5U>) noexcept -> string_view {
        return "/tiling5";
    }
    static auto _loc(unsigned_constant<5U>) noexcept -> string_view {
        return "text:///tiling5";
    }

public:
    tiling_provider(const provider_parameters& params) noexcept
      : main_ctx_object{"TilngPrvdr", params.parent}
      , _shared{params.shared} {}

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> shared_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t size) noexcept
      -> std::chrono::seconds final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    const shared_provider_objects& _shared;
};
//------------------------------------------------------------------------------
template <unsigned Rank>
auto tiling_provider<Rank>::has_resource(const url& locator) noexcept -> bool {
    const auto& q{locator.query()};
    return locator.has_path(_path(unsigned_constant<Rank>{})) and
           q.arg_value_as<span_size_t>("size").value_or(64) > 0;
}
//------------------------------------------------------------------------------
template <unsigned Rank>
auto tiling_provider<Rank>::get_resource_io(const url& locator)
  -> shared_holder<msgbus::source_blob_io> {
    return {
      hold<tiling_io<Rank>>,
      as_parent(),
      locator.query().arg_value_as<span_size_t>("size").value_or(64)};
}
//------------------------------------------------------------------------------
template <unsigned Rank>
auto tiling_provider<Rank>::get_blob_timeout(const span_size_t size) noexcept
  -> std::chrono::seconds {
    return adjusted_duration(std::chrono::seconds{size});
}
//------------------------------------------------------------------------------
template <unsigned Rank>
void tiling_provider<Rank>::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback(_loc(unsigned_constant<Rank>{}));
}
//------------------------------------------------------------------------------
// factory functions
//------------------------------------------------------------------------------
auto provider_text_tiling3(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<tiling_provider<3U>>, params};
}
//------------------------------------------------------------------------------
auto provider_text_tiling4(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<tiling_provider<4U>>, params};
}
//------------------------------------------------------------------------------
auto provider_text_tiling5(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<tiling_provider<5U>>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
