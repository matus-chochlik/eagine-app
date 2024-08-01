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
struct embedded_resource_io : simple_buffer_source_blob_io {
    embedded_resource_io(
      main_ctx_parent parent,
      const embedded_resource&) noexcept;
};
//------------------------------------------------------------------------------
embedded_resource_io::embedded_resource_io(
  main_ctx_parent parent,
  const embedded_resource& res) noexcept
  : simple_buffer_source_blob_io{
      "EmbdRsrcIO",
      parent,
      16 * 1024,
      [&](memory::buffer content) {
          content.clear();
          res.unpack(main_context().compressor(), content);
          return content;
      }} {}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct embedded_resource_provider final
  : main_ctx_object
  , resource_provider_interface {

    embedded_resource_provider(main_ctx_parent parent) noexcept
      : main_ctx_object{"EmbdResPvd", parent} {}

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> shared_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

    embedded_resource_loader _loader;
};
//------------------------------------------------------------------------------
auto embedded_resource_provider::has_resource(const url& locator) noexcept
  -> bool {
    const auto id{locator.path_identifier()};
    return _loader.has_resource(id);
}
//------------------------------------------------------------------------------
auto embedded_resource_provider::get_resource_io(const url& locator)
  -> shared_holder<msgbus::source_blob_io> {
    const auto id{locator.path_identifier()};
    return {hold<embedded_resource_io>, as_parent(), _loader.search(id)};
}
//------------------------------------------------------------------------------
void embedded_resource_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    std::string locator;
    const auto wrapped_callback{[&](const auto res_id, const auto&) {
        locator.clear();
        locator.append("eagires:///");
        append_to(string_view{identifier{res_id}.name()}, locator);
        callback(locator);
    }};
    _loader.for_each(wrapped_callback);
}
//------------------------------------------------------------------------------
auto provider_embedded(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<embedded_resource_provider>, p.parent};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
