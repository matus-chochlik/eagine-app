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
class sky_parameters_io final : public msgbus::source_blob_io {
public:
    sky_parameters_io(shared_provider_objects& shared, url locator) noexcept;
    ~sky_parameters_io() noexcept;

    auto prepare() noexcept -> msgbus::blob_preparation_result final;

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    static auto _default_template() noexcept -> string_view;
    auto _frame_no() noexcept -> long;
    void _substitute() noexcept;
    auto _content() noexcept -> string_view;

    const url _locator;
    shared_provider_objects& _shared;
    std::optional<string_list_resource> _template_resource;
    resource_load_status _template_load_status{resource_load_status::not_found};
    std::string _substituted;
};
//------------------------------------------------------------------------------
sky_parameters_io::sky_parameters_io(
  shared_provider_objects& shared,
  url locator) noexcept
  : _locator{locator}
  , _shared{shared} {
    if(auto tpl_url{_locator.query().arg_url("template")}) {
        _template_resource.emplace(std::move(tpl_url), _shared.loader);
    } else {
        _template_load_status = resource_load_status::loaded;
    }
}
//------------------------------------------------------------------------------
sky_parameters_io::~sky_parameters_io() noexcept {
    if(_template_resource) {
        _template_resource->clean_up(_shared.loader);
        _template_resource.reset();
    }
}
//------------------------------------------------------------------------------
auto sky_parameters_io::_default_template() noexcept -> string_view {
    return {R"({"sun_elevation_deg":25.0,"tiling_url":"text:///TlngR4S512"})"};
}
//------------------------------------------------------------------------------
auto sky_parameters_io::_frame_no() noexcept -> long {
    return _locator.query().arg_value_as<long>("frame").value_or(0L);
}
//------------------------------------------------------------------------------
void sky_parameters_io::_substitute() noexcept {
    _substituted.clear();
    if(_template_resource) {
        for(auto& line : *_template_resource) {
            append_to(line, _substituted);
        }
    } else {
        append_to(_default_template(), _substituted);
    }
    if(_substituted.empty()) {
        _substituted = "{}";
    } else {
        // TODO: evaluate and substitute placeholders
    }
}
//------------------------------------------------------------------------------
auto sky_parameters_io::_content() noexcept -> string_view {
    if(_substituted.empty()) {
        return {"{}"};
    }
    return {_substituted};
}
//------------------------------------------------------------------------------
auto sky_parameters_io::prepare() noexcept -> msgbus::blob_preparation_result {
    if(_template_resource) {
        if(not *_template_resource) {
            loaded_resource_context context{_shared.loader};
            _template_resource->load_if_needed(context);
            return {msgbus::blob_preparation_status::working};
        }
    }
    if(_substituted.empty()) {
        _substitute();
        return {msgbus::blob_preparation_status::working};
    }
    return msgbus::blob_preparation_result::finished();
}
//------------------------------------------------------------------------------
auto sky_parameters_io::total_size() noexcept -> span_size_t {
    return _content().size();
}
//------------------------------------------------------------------------------
auto sky_parameters_io::fetch_fragment(
  span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    return copy(head(skip(_content(), offs), dst.size()), dst).size();
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class sky_parameters_provider final : public resource_provider_interface {
public:
    sky_parameters_provider(const provider_parameters& param) noexcept;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    shared_provider_objects& _shared;
};
//------------------------------------------------------------------------------
sky_parameters_provider::sky_parameters_provider(
  const provider_parameters& param) noexcept
  : _shared{param.shared} {}
//------------------------------------------------------------------------------
auto sky_parameters_provider::has_resource(const url& locator) noexcept
  -> bool {
    const auto& q{locator.query()};
    return locator.has_path("/sky_parameters");
}
//------------------------------------------------------------------------------
auto sky_parameters_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    return {hold<sky_parameters_io>, _shared, locator};
}
//------------------------------------------------------------------------------
void sky_parameters_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("json:///sky_parameters");
}
//------------------------------------------------------------------------------
auto provider_json_sky_parameters(const provider_parameters& param)
  -> unique_holder<resource_provider_interface> {
    return {hold<sky_parameters_provider>, param};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
