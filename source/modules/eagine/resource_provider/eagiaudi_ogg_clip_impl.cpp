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
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
class ogg_decode_context {
public:
    auto has_metadata() const noexcept -> bool;
    void load_metadata() const noexcept;
    auto channel_count() const noexcept -> int;
    auto format_name() const noexcept -> string_view;
    auto sample_rate() const noexcept -> string_view;

private:
};
//------------------------------------------------------------------------------
auto ogg_decode_context::has_metadata() const noexcept -> bool {
    // TODO
    return true;
}
//------------------------------------------------------------------------------
void ogg_decode_context::load_metadata() const noexcept {
    // TODO
}
//------------------------------------------------------------------------------
auto ogg_decode_context::channel_count() const noexcept -> int {
    // TODO
    return 1;
}
//------------------------------------------------------------------------------
auto ogg_decode_context::format_name() const noexcept -> string_view {
    // TODO
    return "mono16";
}
//------------------------------------------------------------------------------
auto ogg_decode_context::sample_rate() const noexcept -> string_view {
    return "12345";
}
//------------------------------------------------------------------------------
// OGG decoder / source blob io
//------------------------------------------------------------------------------
class eagiaudi_ogg_clip_io final : public compressed_buffer_source_blob_io {
public:
    eagiaudi_ogg_clip_io(
      main_ctx_parent,
      shared_holder<ogg_decode_context>) noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation_result final;

private:
    shared_holder<ogg_decode_context> _context;
    bool _header_done{false};
};
//------------------------------------------------------------------------------
eagiaudi_ogg_clip_io::eagiaudi_ogg_clip_io(
  main_ctx_parent parent,
  shared_holder<ogg_decode_context> context) noexcept
  : compressed_buffer_source_blob_io{"IAudioOgg", parent, 0}
  , _context{std::move(context)} {
    assert(_context);
}
//------------------------------------------------------------------------------
auto eagiaudi_ogg_clip_io::prepare() noexcept
  -> msgbus::blob_preparation_result {
    if(not _context->has_metadata()) {
        _context->load_metadata();
        return {msgbus::blob_preparation_status::working};
    }
    if(not _header_done) {
        std::stringstream hdr;
        hdr << R"({"channels":)" << _context->channel_count();
        hdr << R"(,"sample_rate":)" << _context->sample_rate();
        hdr << R"(,"format":)" << _context->format_name();
        append(hdr.str());
        append(R"(,"data_filter":"zlib"})");
        _header_done = true;
    }
    // TODO
    finish();
    return msgbus::blob_preparation_result::finished();
}
//------------------------------------------------------------------------------
// OGG decoder / sample provider
//------------------------------------------------------------------------------
class eagiaudi_ogg_clip_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagiaudi_ogg_clip_provider(const provider_parameters& params) noexcept;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> shared_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t) noexcept
      -> std::chrono::seconds final {
        return adjusted_duration(std::chrono::minutes{2});
    }

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    shared_provider_objects& _shared;
};
//------------------------------------------------------------------------------
eagiaudi_ogg_clip_provider::eagiaudi_ogg_clip_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PAudioOgg", params.parent}
  , _shared{params.shared} {}
//------------------------------------------------------------------------------
auto eagiaudi_ogg_clip_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(locator.has_path_suffix(".ogg")) {
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagiaudi_ogg_clip_provider::get_resource_io(const url& locator)
  -> shared_holder<msgbus::source_blob_io> {
    const auto& q{locator.query()};
    return {
      hold<eagiaudi_ogg_clip_io>,
      as_parent(),
      shared_holder<ogg_decode_context>{default_selector}};
}
//------------------------------------------------------------------------------
void eagiaudi_ogg_clip_provider::for_each_locator(
  callable_ref<void(string_view) noexcept>) noexcept {
    // TODO?
}
//------------------------------------------------------------------------------
// ogg clip provider
//------------------------------------------------------------------------------
auto provider_eagiaudi_ogg_clip(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagiaudi_ogg_clip_provider>, p};
}
//------------------------------------------------------------------------------
} // namespace eagine::app

