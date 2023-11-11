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
struct embedded_resource_provider final
  : main_ctx_object
  , resource_provider_interface {

    embedded_resource_provider(main_ctx_parent parent) noexcept
      : main_ctx_object{"EmbdResPvd", parent} {}

    auto has_resource(const url& locator) noexcept -> bool final {
        const auto id{locator.path_identifier()};
        return _loader.has_resource(id);
    }

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final {
        const auto id{locator.path_identifier()};
        return {hold<embedded_resource_io>, as_parent(), _loader.search(id)};
    }

    embedded_resource_loader _loader;
};
//------------------------------------------------------------------------------
auto provider_embedded(main_ctx_parent parent)
  -> unique_holder<resource_provider_interface> {
    return {hold<embedded_resource_provider>, parent};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
