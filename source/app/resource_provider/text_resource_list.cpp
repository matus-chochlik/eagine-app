/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "common.hpp"
#include "providers.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
class resource_list_io final : public simple_buffer_source_blob_io {
public:
    resource_list_io(resource_provider_driver& driver) noexcept;

    auto prepare() noexcept -> bool final;

private:
    resource_provider_driver& _driver;
    span_size_t _provider_index{0};
};
//------------------------------------------------------------------------------
resource_list_io::resource_list_io(resource_provider_driver& driver) noexcept
  : simple_buffer_source_blob_io{"IResrcList", driver, 16 * 1024}
  , _driver{driver} {
    append("text:///resource_list");
}
//------------------------------------------------------------------------------
auto resource_list_io::prepare() noexcept -> bool {
    if(_provider_index < _driver.provider_count()) {
        const auto append_locator{[this](string_view locator) {
            append(locator);
            append("\n");
        }};
        _driver.provider(_provider_index)
          .for_each_locator({construct_from, append_locator});
        ++_provider_index;
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct resource_list_provider final : resource_provider_interface {
    resource_provider_driver& _driver;

    resource_list_provider(resource_provider_driver& driver) noexcept
      : _driver{driver} {}

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
    return {hold<resource_list_io>, _driver};
}
//------------------------------------------------------------------------------
void resource_list_provider::for_each_locator(
  callable_ref<void(string_view) noexcept>) noexcept {}
//------------------------------------------------------------------------------
auto provider_text_resource_list(const provider_parameters& p)
  -> unique_holder<resource_provider_interface> {
    return {hold<resource_list_provider>, p.driver};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
