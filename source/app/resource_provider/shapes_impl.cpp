/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module eagine.app.resource_provider;

import eagine.core;
import eagine.shapes;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class shape_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    shape_provider(const provider_parameters&);

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    std::string _domain;
};
//------------------------------------------------------------------------------
shape_provider::shape_provider(const provider_parameters& params)
  : main_ctx_object{"ShapePrvdr", params.parent} {}
//------------------------------------------------------------------------------
auto shape_provider::has_resource(const url& locator) noexcept -> bool {
    return shapes::has_shape_from(locator);
}
//------------------------------------------------------------------------------
auto shape_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    unique_holder<ostream_io> io{default_selector};
    if(auto gen{shapes::shape_from(locator, main_context())}) {
        shapes::to_json_options options{};
        if(parse_from(locator, *gen, options)) {
            shapes::to_json(io->ostream(), *gen, options);
        }
    }
    return io;
}
//------------------------------------------------------------------------------
void shape_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    shapes::for_each_shape_locator(_domain, callback);
}
//------------------------------------------------------------------------------
auto provider_shape(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<shape_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
