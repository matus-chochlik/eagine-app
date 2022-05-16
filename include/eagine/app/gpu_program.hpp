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
#include <eagine/embed.hpp>
#include <eagine/oglplus/glsl/string_ref.hpp>
#include <eagine/oglplus/glsl_program.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
class glsl_program : public oglplus::glsl_program {
    using base = oglplus::glsl_program;

public:
    using base::create;

    auto create(video_context& vc) -> glsl_program& {
        base::create(vc.gl_api());
        return *this;
    }

    using base::label;

    auto label(video_context& vc, string_view lbl) -> glsl_program& {
        base::label(vc.gl_api(), lbl);
        return *this;
    }

    using base::add_shader;

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const oglplus::glsl_source_ref& shdr_src) -> glsl_program& {
        base::add_shader(vc.gl_api(), shdr_type, shdr_src);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const oglplus::glsl_source_ref& shdr_src,
      const string_view label) -> glsl_program& {
        base::add_shader(vc.gl_api(), shdr_type, shdr_src, label);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      const oglplus::shader_source_block& shdr_src_blk) -> glsl_program& {
        base::add_shader(vc.gl_api(), shdr_src_blk);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      const oglplus::shader_source_block& shdr_src_blk,
      const string_view label) -> glsl_program& {
        base::add_shader(vc.gl_api(), shdr_src_blk, label);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const embedded_resource& shdr_src_res) -> glsl_program& {
        return add_shader(
          vc,
          shdr_type,
          oglplus::glsl_string_ref{shdr_src_res.unpack(vc.parent())});
    }

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const embedded_resource& shdr_src_res,
      const string_view label) -> glsl_program& {
        return add_shader(
          vc,
          shdr_type,
          oglplus::glsl_string_ref{shdr_src_res.unpack(vc.parent())},
          label);
    }

    auto add_shader(video_context& vc, const embedded_resource& shdr_src_res)
      -> glsl_program& {
        return add_shader(
          vc, oglplus::shader_source_block{shdr_src_res.unpack(vc.parent())});
    }

    auto add_shader(
      video_context& vc,
      const embedded_resource& shdr_src_res,
      const string_view label) -> glsl_program& {
        return add_shader(
          vc,
          oglplus::shader_source_block{shdr_src_res.unpack(vc.parent())},
          label);
    }

    using base::link;

    auto link(video_context& vc) -> glsl_program& {
        base::link(vc.gl_api());
        return *this;
    }

    using base::build;

    auto build(
      video_context& vc,
      const oglplus::program_source_block& prog_src_blk) -> glsl_program& {
        base::build(vc.gl_api(), prog_src_blk);
        return *this;
    }

    auto build(video_context& vc, const embedded_resource& prog_src_res)
      -> glsl_program& {
        return build(
          vc, oglplus::program_source_block{prog_src_res.unpack(vc.parent())});
    }

    using base::init;

    auto init(
      video_context& vc,
      const oglplus::program_source_block& prog_src_blk) -> glsl_program& {
        base::init(vc.gl_api(), prog_src_blk);
        return *this;
    }

    using base::use;

    auto use(video_context& vc) -> glsl_program& {
        base::use(vc.gl_api());
        return *this;
    }

    using base::get_uniform_location;

    auto get_uniform_location(video_context& vc, string_view name) -> auto {
        return base::get_uniform_location(vc.gl_api(), name);
    }

    using base::get_uniform_block_index;

    auto get_uniform_block_index(video_context& vc, string_view name) -> auto {
        return base::get_uniform_block_index(vc.gl_api(), name);
    }

    using base::query;

    auto query(
      video_context& vc,
      string_view name,
      oglplus::uniform_location& loc) -> glsl_program& {
        base::query(vc.gl_api(), name, loc);
        return *this;
    }

    auto query(
      video_context& vc,
      string_view name,
      oglplus::uniform_block_index& ubi) -> glsl_program& {
        base::query(vc.gl_api(), name, ubi);
        return *this;
    }

    using base::set;

    template <typename T>
    auto set(video_context& vc, oglplus::uniform_location loc, T&& value)
      -> glsl_program& {
        base::set(vc.gl_api(), loc, std::forward<T>(value));
        return *this;
    }

    using base::bind;

    auto bind(
      video_context& vc,
      oglplus::vertex_attrib_location loc,
      string_view name) -> glsl_program& {
        base::bind(vc.gl_api(), loc, name);
        return *this;
    }

    auto bind(
      video_context& vc,
      oglplus::uniform_block_index blk_idx,
      oglplus::gl_types::uint_type binding) -> glsl_program& {
        base::bind(vc.gl_api(), blk_idx, binding);
        return *this;
    }

    using base::clean_up;

    auto clean_up(video_context& vc) -> glsl_program& {
        base::clean_up(vc.gl_api());
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
