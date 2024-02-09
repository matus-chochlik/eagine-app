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
import eagine.eglplus;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// I/O
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_io final : public compressed_buffer_source_blob_io {
public:
    eagitexi_cubemap_blur_io(
      main_ctx_parent,
      shared_provider_objects& shared) noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation final;

private:
    shared_provider_objects& _shared;
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_io::eagitexi_cubemap_blur_io(
  main_ctx_parent parent,
  shared_provider_objects& shared) noexcept
  : compressed_buffer_source_blob_io{"ITxCubBlur", parent, 1024 * 1024 * 6}
  , _shared{shared} {}
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
    auto _get_display_io(
      const url& locator,
      const eglplus::egl_api&,
      eglplus::display_handle) -> unique_holder<msgbus::source_blob_io>;

    auto _get_device_io(
      const url& locator,
      const eglplus::egl_api&,
      eglplus::device_handle) -> unique_holder<msgbus::source_blob_io>;

    auto _get_selected_device_io(const url& locator, const eglplus::egl_api&)
      -> unique_holder<msgbus::source_blob_io>;

    static auto is_valid_locator(const url& locator) noexcept -> bool;
    static auto valid_samples(int s) noexcept -> bool;

    shared_provider_objects& _shared;
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_provider::eagitexi_cubemap_blur_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PTxCubBlur", params.parent}
  , _shared{params.shared} {}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::is_valid_locator(
  const url& locator) noexcept -> bool {
    if(const auto source{locator.query().arg_url("source")}) {
        return source.has_scheme("eagitex") or
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
    if(locator.has_scheme("eagitex") and locator.has_path("cube_map_blur")) {
        const auto& q{locator.query()};
        return is_valid_locator(q.arg_url("source")) and
               valid_samples(q.arg_value_as<int>("samples").value_or(1));
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::_get_display_io(
  const url& locator,
  const eglplus::egl_api& eglapi,
  eglplus::display_handle handle) -> unique_holder<msgbus::source_blob_io> {
    if(const ok display{handle}) {
        const auto& [egl, EGL]{eglapi};
        const auto apis{egl.get_client_api_bits(display)};
        const bool has_gl{apis.has(EGL.opengl_bit)};
        const bool has_gles{apis.has(EGL.opengl_es_bit)};
        if(has_gl or has_gles) {
            return {hold<eagitexi_cubemap_blur_io>, as_parent(), _shared};
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::_get_device_io(
  const url& locator,
  const eglplus::egl_api& eglapi,
  eglplus::device_handle device) -> unique_holder<msgbus::source_blob_io> {
    if(const ok display{eglapi.get_platform_display(device)}) {
        return _get_display_io(locator, eglapi, display);
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::_get_selected_device_io(
  const url& locator,
  const eglplus::egl_api& eglapi) -> unique_holder<msgbus::source_blob_io> {
    bool select_device = false; // TODO
    if(select_device) {
        const auto& egl{eglapi.operations()};
        if(const ok dev_count{egl.query_devices.count()}) {
            const auto n{std_size(dev_count)};
            std::vector<eglplus::egl_types::device_type> devices;
            devices.resize(n);
            if(egl.query_devices(cover(devices))) {
                for(const auto cur_dev_idx : integer_range(n)) {
                    bool matching_device = true;
                    auto device = eglplus::device_handle(devices[cur_dev_idx]);
                    // TODO: try to match a device from configuration
                    if(matching_device) {
                        if(auto io{_get_device_io(locator, eglapi, device)}) {
                            return io;
                        }
                    }
                }
            }
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    if(has_resource(locator)) {
        if(const auto eglapi{_shared.apis.egl()}) {
            const auto& egl{eglapi->operations()};
            if(egl.EXT_device_enumeration) {
                if(auto io{_get_selected_device_io(locator, *eglapi)}) {
                    return io;
                }
            }
            if(const ok display{egl.get_display()}) {
                return _get_display_io(locator, *eglapi, display);
            }
        }
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

