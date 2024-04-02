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
class resource_list_io final : public simple_buffer_source_blob_io {
public:
    resource_list_io(shared_provider_objects& shared) noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation_result final;

private:
    shared_provider_objects& _shared;
    span_size_t _provider_index{0};
};
//------------------------------------------------------------------------------
resource_list_io::resource_list_io(shared_provider_objects& shared) noexcept
  : simple_buffer_source_blob_io{"IResrcList", shared.loader.as_parent(), 16 * 1024}
  , _shared{shared} {
    append("text:///resource_list\n");
}
//------------------------------------------------------------------------------
auto resource_list_io::prepare() noexcept -> msgbus::blob_preparation_result {
    if(_provider_index < _shared.driver.provider_count()) {
        const auto append_locator{[this](string_view locator) {
            append(locator);
            append("\n");
        }};
        _shared.driver.provider(_provider_index)
          .for_each_locator({construct_from, append_locator});
        ++_provider_index;
        return {msgbus::blob_preparation_status::working};
    }
    return msgbus::blob_preparation_result::finished();
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct resource_list_provider final : resource_provider_interface {
    shared_provider_objects& _shared;

    resource_list_provider(shared_provider_objects& shared) noexcept
      : _shared{shared} {}

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
auto resource_list_provider::has_resource(const url& locator) noexcept -> bool {
    return locator.has_path("/resource_list");
}
//------------------------------------------------------------------------------
auto resource_list_provider::get_resource_io(const url&)
  -> unique_holder<msgbus::source_blob_io> {
    return {hold<resource_list_io>, _shared};
}
//------------------------------------------------------------------------------
void resource_list_provider::for_each_locator(
  callable_ref<void(string_view) noexcept>) noexcept {}
//------------------------------------------------------------------------------
auto provider_text_resource_list(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<resource_list_provider>, p.shared};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
