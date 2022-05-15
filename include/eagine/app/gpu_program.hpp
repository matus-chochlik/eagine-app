/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#ifndef EAGINE_APP_GPU_PROGRAM_HPP
#define EAGINE_APP_GPU_PROGRAM_HPP

#include "context.hpp"
#include <eagine/oglplus/gl_api/object_name.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class glsl_program : public oglplus::owned_program_name {
    using base = oglplus::owned_program_name;

public:
    auto create(video_context& vc) -> glsl_program& {
        vc.gl_api().create_program() >> static_cast<base&>(*this);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const oglplus::glsl_source_ref& shdr_src) -> glsl_program& {
        vc.gl_api().add_shader(*this, shdr_type, shdr_src);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      const oglplus::shader_source_block& shdr_src_blk) -> glsl_program& {
        vc.gl_api().add_shader(*this, shdr_src_blk);
        return *this;
    }

    auto link(video_context& vc) -> glsl_program& {
        vc.gl_api().link_program(*this);
        return *this;
    }

    auto build(
      video_context& vc,
      const oglplus::program_source_block& prog_src_blk) -> glsl_program& {
        vc.gl_api().build_program(*this, prog_src_blk);
        return *this;
    }

    auto init(
      video_context& vc,
      const oglplus::program_source_block& prog_src_blk) -> glsl_program& {
        return create(vc).build(vc, prog_src_blk);
    }

    auto use(video_context& vc) -> glsl_program& {
        vc.gl_api().use_program(*this);
        return *this;
    }

    auto get_uniform_location(video_context& vc, string_view name) -> auto {
        return vc.gl_api().get_uniform_location(*this, name);
    }

    auto query(
      video_context& vc,
      string_view name,
      oglplus::uniform_location& loc) -> glsl_program& {
        get_uniform_location(vc, name) >> loc;
        return *this;
    }

    template <typename T>
    auto set(video_context& vc, oglplus::uniform_location loc, T&& value)
      -> glsl_program& {
        vc.gl_api().set_uniform(*this, loc, std::forward<T>(value));
        return *this;
    }

    auto bind(
      video_context& vc,
      oglplus::vertex_attrib_location loc,
      string_view name) -> glsl_program& {
        vc.gl_api().bind_attrib_location(*this, loc, name);
        return *this;
    }

    auto clean_up(video_context& vc) -> glsl_program& {
        vc.gl_api().delete_program(static_cast<base&&>(*this));
        return *this;
    }

    auto clean_up_later(video_context& vc) -> glsl_program& {
        vc.clean_up_later(*this);
        return *this;
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

#endif
