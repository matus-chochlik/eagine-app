/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:gpu_program;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.resource;
import eagine.oglplus;
import :context;

namespace eagine::app {
//------------------------------------------------------------------------------
export class gpu_program : public oglplus::gpu_program {
    using base = oglplus::gpu_program;

public:
    using base::create;

    auto create(video_context& vc) -> gpu_program& {
        base::create(vc.gl_api());
        return *this;
    }

    using base::label;

    auto label(video_context& vc, string_view lbl) -> gpu_program& {
        base::label(vc.gl_api(), lbl);
        return *this;
    }

    using base::add_shader;

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const oglplus::glsl_source_ref& shdr_src) -> gpu_program& {
        base::add_shader(vc.gl_api(), shdr_type, shdr_src);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const oglplus::glsl_source_ref& shdr_src,
      const string_view label) -> gpu_program& {
        base::add_shader(vc.gl_api(), shdr_type, shdr_src, label);
        return *this;
    }

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const embedded_resource& shdr_src_res) -> gpu_program& {
        return add_shader(
          vc,
          shdr_type,
          oglplus::glsl_string_ref{shdr_src_res.unpack(vc.parent())});
    }

    auto add_shader(
      video_context& vc,
      oglplus::shader_type shdr_type,
      const embedded_resource& shdr_src_res,
      const string_view label) -> gpu_program& {
        return add_shader(
          vc,
          shdr_type,
          oglplus::glsl_string_ref{shdr_src_res.unpack(vc.parent())},
          label);
    }

    using base::link;

    auto link(video_context& vc) -> gpu_program& {
        base::link(vc.gl_api());
        return *this;
    }

    using base::use;

    auto use(video_context& vc) -> gpu_program& {
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
      oglplus::uniform_location& loc) -> gpu_program& {
        base::query(vc.gl_api(), name, loc);
        return *this;
    }

    auto query(
      video_context& vc,
      string_view name,
      oglplus::uniform_block_index& ubi) -> gpu_program& {
        base::query(vc.gl_api(), name, ubi);
        return *this;
    }

    using base::set;

    template <typename T>
    auto set(video_context& vc, oglplus::uniform_location loc, T&& value)
      -> gpu_program& {
        base::set(vc.gl_api(), loc, std::forward<T>(value));
        return *this;
    }

    using base::bind;

    auto bind(
      video_context& vc,
      oglplus::vertex_attrib_location loc,
      string_view name) -> gpu_program& {
        base::bind(vc.gl_api(), loc, name);
        return *this;
    }

    auto bind(
      video_context& vc,
      oglplus::uniform_block_index blk_idx,
      oglplus::gl_types::uint_type binding) -> gpu_program& {
        base::bind(vc.gl_api(), blk_idx, binding);
        return *this;
    }

    using base::clean_up;

    auto clean_up(video_context& vc) -> gpu_program& {
        base::clean_up(vc.gl_api());
        return *this;
    }

    auto clean_up_later(video_context& vc) -> gpu_program& {
        vc.clean_up_later(*this);
        return *this;
    }
};
//------------------------------------------------------------------------------
} // namespace eagine::app

