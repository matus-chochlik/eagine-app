/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app.resource_provider:external_apis;

import eagine.core;
import eagine.eglplus;
import eagine.oglplus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
export class external_apis : public main_ctx_object {
public:
    external_apis(external_apis&&) = delete;
    external_apis(const external_apis&) = delete;
    auto operator=(external_apis&&) = delete;
    auto operator=(const external_apis&) = delete;

    external_apis(main_ctx_parent);
    ~external_apis() noexcept;

    auto egl() noexcept -> optional_reference<eglplus::egl_api>;
    auto gl() noexcept -> optional_reference<oglplus::gl_api>;

private:
    unique_holder<eglplus::egl_api> _egl;
    unique_holder<oglplus::gl_api> _gl;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
