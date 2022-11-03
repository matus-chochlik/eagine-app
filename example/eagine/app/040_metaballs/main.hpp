/// @example app/040_metaballs/main.hpp
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef OGLPLUS_EXAMPLE_MAIN_HPP // NOLINT(llvm-header-guard)
#define OGLPLUS_EXAMPLE_MAIN_HPP

#include "resources.hpp"

import eagine.core;
import eagine.oglplus;
import eagine.app;

namespace eagine::app {
//------------------------------------------------------------------------------
class example : public timeouting_application {
public:
    example(execution_context&, video_context&);

    void update() noexcept final;
    void clean_up() noexcept final;

    auto video() noexcept -> auto& {
        return _video;
    }

    auto loader() noexcept -> auto& {
        return context().loader();
    }

    auto camera() noexcept -> auto& {
        return _camera;
    }

    auto frame_duration() noexcept {
        return context().state().frame_duration().value();
    }

    operator cleanup_group&() noexcept {
        return _cleanup;
    }

private:
    void _on_loaded(const gl_program_resource::load_info&) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&example::_on_loaded>(this);
    }

    cleanup_group _cleanup;
    video_context& _video;
    background_icosahedron _bg;
    volume_domain _volume;

    metaball_program _mball_prog;
    field_program _field_prog;
    surface_program _srfce_prog;
    pending_resource_requests _other;

    orbiting_camera _camera;
};

} // namespace eagine::app

#endif
