/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "ostream_io.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
auto ostream_io::ostream() noexcept -> std::ostream& {
    return _content;
}
//------------------------------------------------------------------------------
auto ostream_io::total_size() noexcept -> span_size_t {
    return span_size(_content.view().size());
}
//------------------------------------------------------------------------------
auto ostream_io::fetch_fragment(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    const auto src{skip(memory::view(_content.view()), offs)};
    dst = head(dst, src);
    return copy(head(src, dst), dst).size();
}
//------------------------------------------------------------------------------
} // namespace eagine::app
