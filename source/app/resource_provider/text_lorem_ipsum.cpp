/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "providers.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
struct lorem_ipsum_io final : msgbus::source_blob_io {
    span_size_t _repeats{1};

    lorem_ipsum_io(span_size_t repeats) noexcept
      : _repeats{repeats} {}

    static auto lorem_ipsum() noexcept -> string_view;

    auto total_size() noexcept -> span_size_t final {
        return lorem_ipsum().size() * _repeats;
    }

    auto fetch_fragment(span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;
};
//------------------------------------------------------------------------------
auto lorem_ipsum_io::lorem_ipsum() noexcept -> string_view {
    return {
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
      "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
      "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris "
      "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor "
      "in reprehenderit in voluptate velit esse cillum dolore eu fugiat "
      "nulla pariatur. Excepteur sint occaecat cupidatat non proident, "
      "sunt in culpa qui officia deserunt mollit anim id est laborum."};
}
//------------------------------------------------------------------------------
auto lorem_ipsum_io::fetch_fragment(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    const auto li{as_bytes(lorem_ipsum())};

    dst = head(dst, total_size() - offs);
    offs = offs % li.size();
    memory::block chnk{head(dst, li.size() - offs)};
    span_size_t done{copy(head(skip(li, offs), chnk), chnk).size()};
    chnk = skip(dst, done);

    while(chnk) {
        done += copy(head(li, chnk), chnk).size();
        chnk = skip(dst, done);
    }
    return done;
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
struct lorem_ipsum_provider final : resource_provider_interface {
    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;
};
//------------------------------------------------------------------------------
auto lorem_ipsum_provider::has_resource(const url& locator) noexcept -> bool {
    const auto& q{locator.query()};
    return locator.has_path("/lorem_ipsum") and
           q.arg_value_as<span_size_t>("repeat").value_or(1) > 0;
}
//------------------------------------------------------------------------------
auto lorem_ipsum_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    return {
      hold<lorem_ipsum_io>,
      locator.query().arg_value_as<span_size_t>("repeat").value_or(1)};
}
//------------------------------------------------------------------------------
void lorem_ipsum_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("text:///lorem_ipsum");
}
//------------------------------------------------------------------------------
auto provider_text_lorem_ipsum(const provider_parameters&)
  -> unique_holder<resource_provider_interface> {
    return {hold<lorem_ipsum_provider>};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
