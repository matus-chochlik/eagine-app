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
static inline auto get_default_blob_timeout(const span_size_t size) noexcept
  -> std::chrono::seconds {
    return std::max(
      std::chrono::seconds{size / 1024}, std::chrono::seconds{30});
}
//------------------------------------------------------------------------------
auto resource_provider_interface::get_blob_timeout(
  const span_size_t size) noexcept -> std::chrono::seconds {
    return adjusted_duration(get_default_blob_timeout(size));
}
//------------------------------------------------------------------------------
auto resource_provider_interface::get_blob_priority(
  const msgbus::message_priority priority) noexcept
  -> msgbus::message_priority {
    return increased(priority);
}
//------------------------------------------------------------------------------
// resource_provider_driver
//------------------------------------------------------------------------------
void resource_provider_driver::_add(
  unique_holder<resource_provider_interface> provider) {
    assert(provider);
    _providers.emplace_back(std::move(provider));
}
//------------------------------------------------------------------------------
void resource_provider_driver::_populate() {
    const provider_parameters parameters{
      .parent = as_parent(), .shared = _shared};

    _add(provider_zip_archive(parameters));
    _add(provider_file(parameters));
    _add(provider_embedded(parameters));
    _add(provider_eagitexi_random(parameters));
    _add(provider_eagitexi_tiling(parameters));
    _add(provider_eagitexi_tiling_noise(parameters));
    _add(provider_eagitexi_tiling_transition(parameters));
    _add(provider_eagitexi_2d_single_rgb8(parameters));
    _add(provider_eagitex_2d_single_rgb8(parameters));
    _add(provider_eagitexi_sphere_volume(parameters));
    _add(provider_eagitex_sphere_volume(parameters));
    _add(provider_eagitexi_cubemap_sky(parameters));
    _add(provider_eagitex_cubemap_sky(parameters));
    _add(provider_eagitexi_cubemap_blur(parameters));
    _add(provider_eagitex_cubemap_levels_blur(parameters));
    _add(provider_eagiaudi_ogg_clip(parameters));
    _add(provider_shape(parameters));
    _add(provider_json_sky_parameters(parameters));
    _add(provider_text_tiling3(parameters));
    _add(provider_text_tiling4(parameters));
    _add(provider_text_tiling5(parameters));
    _add(provider_text_lorem_ipsum(parameters));
    _add(provider_text_resource_list(parameters));
}
//------------------------------------------------------------------------------
resource_provider_driver::resource_provider_driver(
  main_ctx_parent parent,
  external_apis& apis,
  old_resource_loader& old_loader,
  resource_loader& loader)
  : main_ctx_object{"RsrcPrDrvr", parent}
  , _shared{
      .apis = apis,
      .driver = *this,
      .old_loader = old_loader,
      .loader = loader} {
    _populate();
}
//------------------------------------------------------------------------------
auto resource_provider_driver::find_provider_of(const url& locator) noexcept
  -> optional_reference<resource_provider_interface> {
    for(const auto& provider : _providers) {
        if(provider->has_resource(locator)) {
            return provider;
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto resource_provider_driver::has_resource(const url& locator) noexcept
  -> tribool {
    if(find_provider_of(locator)) {
        return true;
    }
    return indeterminate;
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_resource_io(
  const endpoint_id_t,
  const url& locator) -> shared_holder<msgbus::source_blob_io> {
    if(const auto provider{find_provider_of(locator)}) {
        return provider->get_resource_io(locator);
    }
    return {};
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_blob_timeout(
  const endpoint_id_t,
  const url& locator,
  const span_size_t size) noexcept -> std::chrono::seconds {
    return adjusted_duration(
      find_provider_of(locator)
        .member(&resource_provider_interface::get_blob_timeout, size)
        .value_or(get_default_blob_timeout(size)));
}
//------------------------------------------------------------------------------
auto resource_provider_driver::get_blob_priority(
  const endpoint_id_t,
  const url& locator,
  const msgbus::message_priority priority) noexcept
  -> msgbus::message_priority {
    return find_provider_of(locator)
      .member(&resource_provider_interface::get_blob_priority, priority)
      .value_or(priority);
}
//------------------------------------------------------------------------------
auto resource_provider_driver::provider_count() const noexcept -> span_size_t {
    return span_size(_providers.size());
}
//------------------------------------------------------------------------------
auto resource_provider_driver::provider(span_size_t i) const noexcept
  -> resource_provider_interface& {
    assert(i < provider_count());
    assert(_providers[std_size(i)]);
    return *_providers[std_size(i)];
}
//------------------------------------------------------------------------------
} // namespace eagine::app
