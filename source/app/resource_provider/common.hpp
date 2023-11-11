/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_RESOURCE_PROVIDER_COMMON_HPP
#define EAGINE_RESOURCE_PROVIDER_COMMON_HPP

#include "driver.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
struct simple_string_source_blob_io
  : main_ctx_object
  , msgbus::source_blob_io {
protected:
    simple_string_source_blob_io(
      identifier id,
      main_ctx_parent parent,
      std::string content) noexcept;

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    const std::string _content;
};
//------------------------------------------------------------------------------
struct eagitex_provider_base
  : main_ctx_object
  , resource_provider_interface {
protected:
    eagitex_provider_base(identifier id, main_ctx_parent parent) noexcept;

    static auto valid_color(int c) noexcept -> bool {
        return (c >= 0) and (c <= 255);
    }

    static auto valid_dimension(int d) noexcept -> bool {
        return (d > 0) and (d <= 64 * 1024);
    }

    static auto valid_level(int l) noexcept -> bool {
        return (l >= 0);
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
