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
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// I/O
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_io final : public compressed_buffer_source_blob_io {
public:
    eagitexi_cubemap_blur_io(main_ctx_parent) noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation final;

private:
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_io::eagitexi_cubemap_blur_io(
  main_ctx_parent parent) noexcept
  : compressed_buffer_source_blob_io{"ITxCubBlur", parent, 1024 * 1024 * 6} {}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::prepare() noexcept -> msgbus::blob_preparation {
    return msgbus::blob_preparation::finished;
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagitexi_cubemap_blur_provider(const provider_parameters&) noexcept;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    static auto is_valid_locator(const url& locator) noexcept -> bool;
    static auto valid_samples(int s) noexcept -> bool;

    external_apis& _apis;
    resource_loader& _loader;
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_provider::eagitexi_cubemap_blur_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PTxCubBlur", params.parent}
  , _apis{params.apis}
  , _loader{params.loader} {}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::is_valid_locator(
  const url& locator) noexcept -> bool {
    if(const auto source{locator.query().arg_url("source")}) {
        return source.has_scheme("eagitexi") or
               source.has_path_suffix(".eagitexi") or
               source.has_path_suffix(".eagitex");
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::valid_samples(int s) noexcept -> bool {
    return s > 0;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(locator.has_scheme("eagitexi") and locator.has_path("cube_map_blur")) {
        const auto& q{locator.query()};
        return is_valid_locator(q.arg_url("source")) and
               valid_samples(q.arg_value_as<int>("samples").value_or(1));
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    if(has_resource(locator)) {
        return {hold<eagitexi_cubemap_blur_io>, as_parent()};
    }
    return {};
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///cube_map_blur");
}
//------------------------------------------------------------------------------
auto provider_eagitexi_cubemap_blur(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_cubemap_blur_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app

