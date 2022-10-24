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
class example : public application {
public:
    example(execution_context&, video_context&);

    auto is_done() noexcept -> bool final {
        return _is_done.is_expired();
    }

    void on_video_resize() noexcept final;
    void update() noexcept final;
    void clean_up() noexcept final;

    auto ctx() noexcept -> auto& {
        return _ctx;
    }

    auto video() noexcept -> auto& {
        return _video;
    }

    auto loader() noexcept -> auto& {
        return _ctx.loader();
    }

    auto camera() noexcept -> auto& {
        return _camera;
    }

    auto frame_duration() noexcept {
        return ctx().state().frame_duration().value();
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
    execution_context& _ctx;
    video_context& _video;
    background_icosahedron _bg;
    volume_domain _volume;

    metaball_program _mball_prog;
    field_program _field_prog;
    surface_program _srfce_prog;

    orbiting_camera _camera;
    timeout _is_done{std::chrono::seconds{60}};
};

} // namespace eagine::app

#endif
