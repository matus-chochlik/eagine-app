/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app;

import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.reflection;
import eagine.core.runtime;
import eagine.core.utility;
import eagine.core.value_tree;
import eagine.core.c_api;
import eagine.oglplus;
import eagine.shapes;
import <bitset>;
import <string>;

namespace eagine::app {
//------------------------------------------------------------------------------
// valtree_gl_program_builder
//------------------------------------------------------------------------------
class valtree_gl_program_builder
  : public valtree_builder_base<valtree_gl_program_builder> {
    using base = valtree_builder_base<valtree_gl_program_builder>;

public:
    valtree_gl_program_builder(
      const std::shared_ptr<pending_resource_info>& info,
      video_context& video) noexcept
      : base{info}
      , _video{video} {}

    using base::do_add;

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if(path.size() == 1) {
            if(path.starts_with("label")) {
                if(has_value(data)) {
                    if(auto parent{_parent.lock()}) {
                        extract(parent).add_label(extract(data));
                    }
                }
            }
        } else if((path.size() == 3) && data) {
            if((path.starts_with("inputs")) && (path.ends_with("attrib"))) {
                if(const auto kind{
                     from_string<shapes::vertex_attrib_kind>(extract(data))}) {
                    _attrib_kind = extract(kind);
                } else {
                    _input_name.clear();
                }
            }
            if(path.starts_with("shaders")) {
                if(path.ends_with("url")) {
                    if(data.has_single_value()) {
                        _shdr_locator = {to_string(extract(data))};
                    }
                } else if(path.ends_with("type")) {
                    if(const auto conv{
                         from_strings<oglplus::shader_type>(data)}) {
                        _shdr_type = extract(conv);
                    }
                }
            }
        }
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        if((path.size() == 3) && data) {
            if((path.starts_with("inputs")) && (path.ends_with("variant"))) {
                _attrib_variant_index = span_size(extract(data));
            }
        }
    }

    void add_object(const basic_string_path& path) noexcept final {
        if(path.size() == 2) {
            if(path.starts_with("inputs")) {
                _input_name = to_string(path.back());
                _attrib_kind = shapes::vertex_attrib_kind::position;
                _attrib_variant_index = 0;
            } else if(path.starts_with("shaders")) {
                _shdr_locator = {};
                _shdr_type = _video.gl_api().constants().fragment_shader;
            }
        }
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.size() == 2) {
            if(path.starts_with("inputs")) {
                if(!_input_name.empty()) {
                    if(auto parent{_parent.lock()}) {
                        extract(parent).add_gl_program_input_binding(
                          std::move(_input_name),
                          {_attrib_kind, _attrib_variant_index});
                    }
                }
            } else if(path.starts_with("shaders")) {
                if(_shdr_locator) {
                    if(auto parent{_parent.lock()}) {
                        auto& loader = extract(parent).loader();
                        if(auto src_request{loader.request_gl_shader(
                             _shdr_locator, _video, _shdr_type)}) {
                            src_request.set_continuation(parent);
                            if(!extract(parent).add_gl_program_shader_request(
                                 src_request.request_id())) [[unlikely]] {
                                src_request.info().mark_finished();
                                extract(parent).mark_finished();
                            }
                        }
                    }
                }
            }
        }
    }

private:
    video_context& _video;
    std::string _input_name;
    oglplus::shader_type _shdr_type;
    url _shdr_locator;
    shapes::vertex_attrib_kind _attrib_kind;
    span_size_t _attrib_variant_index{0};
};
//------------------------------------------------------------------------------
auto make_valtree_gl_program_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  video_context& video) noexcept -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_gl_program_builder>(parent, video);
}
//------------------------------------------------------------------------------
// valtree_gl_texture_image_loader
//------------------------------------------------------------------------------
template <typename T>
static auto gl_enum_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    if(const auto conv{from_strings<T>(data)}) {
        v = to_underlying(extract(conv));
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
static auto texture_target_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::texture_target>(data, v);
}
//------------------------------------------------------------------------------
static auto texture_data_type_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::pixel_data_type>(data, v);
}
//------------------------------------------------------------------------------
static auto texture_iformat_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::pixel_internal_format>(data, v);
}
//------------------------------------------------------------------------------
static auto texture_format_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::pixel_format>(data, v);
}
//------------------------------------------------------------------------------
static auto texture_swizzle_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::texture_swizzle_mode>(data, v);
}
//------------------------------------------------------------------------------
struct resource_gl_texture_image_params {
    oglplus::gl_types::int_type dimensions{0};
    oglplus::gl_types::int_type level{0};
    oglplus::gl_types::int_type x_offs{0};
    oglplus::gl_types::int_type y_offs{0};
    oglplus::gl_types::int_type z_offs{0};
    oglplus::gl_types::sizei_type channels{0};
    oglplus::gl_types::sizei_type width{0};
    oglplus::gl_types::sizei_type height{1};
    oglplus::gl_types::sizei_type depth{1};
    oglplus::gl_types::enum_type iformat{0};
    oglplus::gl_types::enum_type format{0};
    oglplus::gl_types::enum_type data_type{0};
};
//------------------------------------------------------------------------------
class valtree_gl_texture_image_loader
  : public valtree_builder_base<valtree_gl_texture_image_loader> {
    using base = valtree_builder_base<valtree_gl_texture_image_loader>;

public:
    valtree_gl_texture_image_loader(
      std::shared_ptr<pending_resource_info> info,
      oglplus::texture_target target,
      const resource_gl_texture_image_params& params) noexcept
      : base{std::move(info)}
      , _target{target}
      , _params{params} {
        if(const auto parent{_parent.lock()}) {
            extract(parent).loader().buffers().get(512 * 512);
        }
    }

    auto append_image_data(const memory::const_block blk) noexcept -> bool {
        log_debug("appending texture image data")
          .tag("apndImgDta")
          .arg("offset", _temp.size())
          .arg("size", blk.size());
        memory::append_to(blk, _temp);
        //  TODO: progressive image specification once we have enough
        //  data for width * some constant so that the temp buffer
        //  doesn't get too big
        return true;
    }

    auto init_decompression(data_compression_method method) noexcept -> bool {
        if(const auto parent{_parent.lock()}) {
            _decompression = stream_decompression{
              data_compressor{method, extract(parent).loader().buffers()},
              make_callable_ref<
                &valtree_gl_texture_image_loader::append_image_data>(this),
              method};
            return true;
        }
        return false;
    }

    using base::do_add;

    template <std::integral T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept {
        if(path.size() == 1) {
            if(path.starts_with("level")) {
                _success &= assign_if_fits(data, _params.level);
            } else if(path.starts_with("x_offs")) {
                _success &= assign_if_fits(data, _params.x_offs);
                _params.dimensions = std::max(_params.dimensions, 1);
            } else if(path.starts_with("y_offs")) {
                _success &= assign_if_fits(data, _params.y_offs);
                _params.dimensions = std::max(_params.dimensions, 2);
            } else if(path.starts_with("z_offs")) {
                _success &= assign_if_fits(data, _params.z_offs);
                _params.dimensions = std::max(_params.dimensions, 3);
            } else if(path.starts_with("channels")) {
                _success &= assign_if_fits(data, _params.channels);
            } else if(path.starts_with("width")) {
                _success &= assign_if_fits(data, _params.width);
                _params.dimensions = std::max(_params.dimensions, 1);
            } else if(path.starts_with("height")) {
                _success &= assign_if_fits(data, _params.height);
                if(_params.height > 1) {
                    _params.dimensions = std::max(_params.dimensions, 2);
                }
            } else if(path.starts_with("depth")) {
                _success &= assign_if_fits(data, _params.depth);
                if(_params.depth > 1) {
                    _params.dimensions = std::max(_params.dimensions, 3);
                }
            } else if(path.starts_with("data_type")) {
                _success &= assign_if_fits(data, _params.data_type);
            } else if(path.starts_with("format")) {
                _success &= assign_if_fits(data, _params.format);
            } else if(path.starts_with("iformat")) {
                _success &= assign_if_fits(data, _params.iformat);
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept {
        if(path.size() == 1) {
            if(path.starts_with("data_type")) {
                _success &=
                  texture_data_type_from_string(data, _params.data_type);
            } else if(path.starts_with("format")) {
                _success &= texture_format_from_string(data, _params.format);
            } else if(path.starts_with("iformat")) {
                _success &= texture_iformat_from_string(data, _params.iformat);
            } else if(path.starts_with("data_filter")) {
                if(data.has_single_value()) {
                    if(const auto method{
                         from_string<data_compression_method>(extract(data))}) {
                        _success &= init_decompression(extract(method));
                    } else {
                        _success = false;
                    }
                } else {
                    _success = false;
                }
            }
        }
    }

    void unparsed_data(span<const memory::const_block> data) noexcept final {
        if(!_decompression.is_initialized()) {
            _success &= init_decompression(data_compression_method::none);
        }
        if(_success) {
            for(const auto& blk : data) {
                _decompression.next(blk);
            }
        }
    }

    auto finish() noexcept -> bool final {
        if(const auto parent{_parent.lock()}) {
            if(_success) {
                _decompression.finish();
                extract(parent).handle_gl_texture_image(
                  _target, _params, _temp);
            } else {
                extract(parent).mark_finished();
            }
            extract(parent).loader().buffers().eat(std::move(_temp));
            return _success;
        }
        return false;
    }

    void failed() noexcept final {
        if(const auto parent{_parent.lock()}) {
            extract(parent).mark_finished();
            extract(parent).loader().buffers().eat(std::move(_temp));
        }
    }

private:
    memory::buffer _temp;
    stream_decompression _decompression;
    oglplus::texture_target _target;
    resource_gl_texture_image_params _params{};
    bool _success{true};
};
//------------------------------------------------------------------------------
auto make_valtree_gl_texture_image_loader(
  const std::shared_ptr<pending_resource_info>& parent,
  oglplus::texture_target target,
  const resource_gl_texture_image_params& params) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_gl_texture_image_loader>(
      parent, target, params);
}

//------------------------------------------------------------------------------
// valtree_gl_texture_builder
//------------------------------------------------------------------------------
struct resource_gl_texture_params {
    oglplus::gl_types::int_type dimensions{0};
    oglplus::gl_types::int_type levels{0};
    oglplus::gl_types::sizei_type width{0};
    oglplus::gl_types::sizei_type height{1};
    oglplus::gl_types::sizei_type depth{1};
    oglplus::gl_types::enum_type iformat{0};
    oglplus::gl_types::enum_type format{0};
    oglplus::gl_types::enum_type data_type{0};
};
//------------------------------------------------------------------------------
class valtree_gl_texture_builder
  : public valtree_builder_base<valtree_gl_texture_builder> {
    using base = valtree_builder_base<valtree_gl_texture_builder>;

public:
    valtree_gl_texture_builder(
      const std::shared_ptr<pending_resource_info>& info,
      video_context& video,
      oglplus::texture_target tex_target,
      oglplus::texture_unit tex_unit) noexcept
      : base{info}
      , _video{video}
      , _tex_target{tex_target}
      , _tex_unit{tex_unit} {}

    using base::do_add;

    template <std::integral T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept {
        if(path.size() == 1) {
            if(path.starts_with("levels")) {
                _success &= assign_if_fits(data, _params.levels);
            } else if(path.starts_with("width")) {
                _success &= assign_if_fits(data, _params.width);
                _params.dimensions = std::max(_params.dimensions, 1);
            } else if(path.starts_with("height")) {
                _success &= assign_if_fits(data, _params.height);
                if(_params.height > 1) {
                    _params.dimensions = std::max(_params.dimensions, 2);
                }
            } else if(path.starts_with("depth")) {
                _success &= assign_if_fits(data, _params.depth);
                if(_params.depth > 1) {
                    _params.dimensions = std::max(_params.dimensions, 3);
                }
            } else if(path.starts_with("data_type")) {
                _success &= assign_if_fits(data, _params.data_type);
            } else if(path.starts_with("format")) {
                _success &= assign_if_fits(data, _params.format);
            } else if(path.starts_with("iformat")) {
                _success &= assign_if_fits(data, _params.iformat);
            }
        } else if(path.size() == 3) {
            if(path.starts_with("images")) {
                if(path.ends_with("level")) {
                    _success &= assign_if_fits(data, _image_params.level);
                } else if(path.ends_with("x_offs")) {
                    _success &= assign_if_fits(data, _image_params.x_offs);
                    _image_params.dimensions =
                      std::max(_image_params.dimensions, 1);
                } else if(path.ends_with("y_offs")) {
                    _success &= assign_if_fits(data, _image_params.y_offs);
                    _image_params.dimensions =
                      std::max(_image_params.dimensions, 2);
                } else if(path.ends_with("z_offs")) {
                    _success &= assign_if_fits(data, _image_params.z_offs);
                    _image_params.dimensions =
                      std::max(_image_params.dimensions, 3);
                }
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept {
        if(path.size() == 1) {
            using It = oglplus::gl_types::int_type;
            using Et = oglplus::gl_types::enum_type;

            if(path.starts_with("label")) {
                if(has_value(data)) {
                    if(const auto parent{_parent.lock()}) {
                        extract(parent).add_label(extract(data));
                    }
                }
            } else if(path.starts_with("data_type")) {
                _success &=
                  texture_data_type_from_string(data, _params.data_type);
            } else if(path.starts_with("format")) {
                _success &= texture_format_from_string(data, _params.format);
            } else if(path.starts_with("iformat")) {
                _success &= texture_iformat_from_string(data, _params.iformat);
            } else if(path.starts_with("swizzle_r")) {
                Et e{0};
                if(_success &= texture_swizzle_from_string(data, e)) {
                    _i_params.emplace_back(0x8E42, It(e));
                }
            } else if(path.starts_with("swizzle_g")) {
                Et e{0};
                if(_success &= texture_swizzle_from_string(data, e)) {
                    _i_params.emplace_back(0x8E43, It(e));
                }
            } else if(path.starts_with("swizzle_b")) {
                Et e{0};
                if(_success &= texture_swizzle_from_string(data, e)) {
                    _i_params.emplace_back(0x8E44, It(e));
                }
            } else if(path.starts_with("swizzle_a")) {
                Et e{0};
                if(_success &= texture_swizzle_from_string(data, e)) {
                    _i_params.emplace_back(0x8E45, It(e));
                }
            }
        } else if(path.size() == 3) {
            if(path.starts_with("images")) {
                if(path.ends_with("url")) {
                    if(has_value(data)) {
                        _image_locator = {to_string(extract(data))};
                    } else {
                        _success = false;
                    }
                } else if(path.ends_with("target")) {
                    oglplus::gl_types::enum_type tgt{0};
                    if(texture_target_from_string(data, tgt)) {
                        _image_target = oglplus::texture_target{tgt};
                    } else {
                        log_error("invalid texture target '${name}'")
                          .arg("name", extract(data));
                        _success = false;
                    }
                }
            }
        }
    }

    void add_object(const basic_string_path& path) noexcept final {
        if(path.size() == 2) {
            if(path.starts_with("images")) {
                _image_locator = {};
                _image_params = {};
                _image_target = _tex_target;
            }
        }
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.empty()) {
            if(_success) {
                if(const auto parent{_parent.lock()}) {
                    _success &=
                      extract(parent).handle_gl_texture_params(_params);
                } else {
                    _success = false;
                }
            }
        } else if(path.size() == 2) {
            if(path.starts_with("images")) {
                if(_image_locator) {
                    _image_requests.emplace_back(
                      std::move(_image_locator), _image_target, _image_params);
                }
            }
        }
    }

    auto finish() noexcept -> bool final {
        if(const auto parent{_parent.lock()}) {
            if(_success) {
                for(auto& [loc, tgt, para] : _image_requests) {
                    const auto img_request{
                      extract(parent).loader().request_gl_texture_image(
                        std::move(loc), tgt, para)};
                    img_request.set_continuation(parent);
                    extract(parent).add_gl_texture_image_request(
                      img_request.request_id());
                }
                _image_requests.clear();
                for(const auto [param, value] : _i_params) {
                    extract(parent).handle_gl_texture_i_param(
                      oglplus::texture_parameter{param}, value);
                }
                extract(parent).mark_loaded();
            } else {
                extract(parent).mark_finished();
            }
            return _success;
        }
        return false;
    }

private:
    video_context& _video;
    resource_gl_texture_params _params{};
    url _image_locator;
    resource_gl_texture_image_params _image_params;
    oglplus::texture_target _image_target;
    oglplus::texture_target _tex_target;
    oglplus::texture_unit _tex_unit;
    std::vector<
      std::tuple<url, oglplus::texture_target, resource_gl_texture_image_params>>
      _image_requests;
    std::vector<
      std::tuple<oglplus::gl_types::enum_type, oglplus::gl_types::int_type>>
      _i_params;
    bool _success{true};
};
//------------------------------------------------------------------------------
auto make_valtree_gl_texture_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  video_context& video,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_gl_texture_builder>(
      parent, video, target, unit);
}
//------------------------------------------------------------------------------
auto pending_resource_info::handle_gl_texture_params(
  const resource_gl_texture_params& params) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        _parent.log_info("loaded GL texture storage parameters")
          .arg("levels", params.levels)
          .arg("width", params.width)
          .arg("height", params.height)
          .arg("depth", params.depth)
          .arg("dimensions", params.dimensions)
          .arg("iformat", params.iformat);

        auto& pgts = std::get<_pending_gl_texture_state>(_state);
        pgts.pparams = &params;
        pgts.levels = span_size(params.levels);

        auto& glapi = pgts.video.get().gl_api();
        glapi.operations().active_texture(pgts.tex_unit);
        glapi.bind_texture(pgts.tex_target, pgts.tex);
        if(params.dimensions == 3) {
            if(glapi.texture_storage3d) {
                return bool(glapi.texture_storage3d(
                  pgts.tex,
                  params.levels,
                  oglplus::pixel_internal_format{params.iformat},
                  params.width,
                  params.height,
                  params.depth));
            } else if(glapi.tex_storage3d) {
                return bool(glapi.tex_storage3d(
                  pgts.tex_target,
                  params.levels,
                  oglplus::pixel_internal_format{params.iformat},
                  params.width,
                  params.height,
                  params.depth));
            } else if(glapi.tex_image3d) {
                bool result{true};
                auto width = params.width;
                auto height = params.height;
                auto depth = params.depth;
                for(auto level : integer_range(params.levels)) {
                    result &= bool(glapi.tex_image3d(
                      pgts.tex_target,
                      level,
                      oglplus::pixel_internal_format{params.iformat},
                      width,
                      height,
                      depth,
                      0, // border
                      oglplus::pixel_format{params.format},
                      oglplus::pixel_data_type{params.data_type},
                      {}));
                    width = std::max(width / 2, 1);
                    height = std::max(height / 2, 1);
                    if(pgts.tex_target == glapi.texture_3d) {
                        depth = std::max(depth / 2, 1);
                    }
                }
                return result;
            }
        } else if(params.dimensions == 2) {
            if(glapi.texture_storage2d) {
                return bool(glapi.texture_storage2d(
                  pgts.tex,
                  params.levels,
                  oglplus::pixel_internal_format{params.iformat},
                  params.width,
                  params.height));
            } else if(glapi.tex_storage2d) {
                return bool(glapi.tex_storage2d(
                  pgts.tex_target,
                  params.levels,
                  oglplus::pixel_internal_format{params.iformat},
                  params.width,
                  params.height));
            } else if(glapi.tex_image2d) {
                bool result{true};
                auto width = params.width;
                auto height = params.height;
                for(auto level : integer_range(params.levels)) {
                    result &= bool(glapi.tex_image2d(
                      pgts.tex_target,
                      level,
                      oglplus::pixel_internal_format{params.iformat},
                      width,
                      height,
                      0, // border
                      oglplus::pixel_format{params.format},
                      oglplus::pixel_data_type{params.data_type},
                      {}));
                    width = std::max(width / 2, 1);
                    height = std::max(height / 2, 1);
                }
                return result;
            }
        } else if(params.dimensions == 1) {
            if(glapi.texture_storage1d) {
                return bool(glapi.texture_storage1d(
                  pgts.tex,
                  params.levels,
                  oglplus::pixel_internal_format{params.iformat},
                  params.width));
            } else if(glapi.tex_storage1d) {
                return bool(glapi.tex_storage1d(
                  pgts.tex_target,
                  params.levels,
                  oglplus::pixel_internal_format{params.iformat},
                  params.width));
            } else if(glapi.tex_image1d) {
                bool result{true};
                auto width = params.width;
                for(auto level : integer_range(params.levels)) {
                    result &= bool(glapi.tex_image1d(
                      pgts.tex_target,
                      level,
                      oglplus::pixel_internal_format{params.iformat},
                      width,
                      0, // border
                      oglplus::pixel_format{params.format},
                      oglplus::pixel_data_type{params.data_type},
                      {}));
                    width = std::max(width / 2, 1);
                }
                return result;
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_gl_texture_i_param(
  const oglplus::texture_parameter param,
  const oglplus::gl_types::int_type value) noexcept {
    _parent.log_info("setting GL texture parameter")
      .arg("requestId", _request_id)
      .arg("locator", locator().str());

    if(is(resource_kind::gl_texture)) {
        if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
            auto& pgts = std::get<_pending_gl_texture_state>(_state);
            if(pgts.tex) [[likely]] {
                const auto& glapi = pgts.video.get().gl_api();
                if(glapi.texture_parameter_i) {
                    glapi.texture_parameter_i(pgts.tex, param, value);
                } else if(glapi.tex_parameter_i) {
                    glapi.tex_parameter_i(pgts.tex_target, param, value);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_clear_gl_texture_image(
  const _pending_gl_texture_state& pgts,
  const resource_gl_texture_params& params,
  span_size_t level,
  const memory::const_block data) noexcept {
    _parent.log_info("clearing GL texture image")
      .arg("requestId", _request_id)
      .arg("level", level)
      .arg("dataSize", data.size())
      .arg("locator", locator().str());

    auto& glapi = pgts.video.get().gl_api();
    if(glapi.clear_tex_image) {
        glapi.clear_tex_image(
          pgts.tex,
          limit_cast<oglplus::gl_types::int_type>(level),
          oglplus::pixel_format{params.format},
          oglplus::pixel_data_type{params.data_type},
          data);
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_gl_texture_image(
  const pending_resource_info& source,
  const oglplus::texture_target target,
  const resource_gl_texture_image_params& params,
  const memory::const_block data) noexcept {
    _parent.log_info("loaded GL texture sub-image")
      .arg("requestId", _request_id)
      .arg("level", params.level)
      .arg("xoffs", params.x_offs)
      .arg("yoffs", params.y_offs)
      .arg("zoffs", params.z_offs)
      .arg("width", params.width)
      .arg("height", params.height)
      .arg("depth", params.depth)
      .arg("dimensions", params.dimensions)
      .arg("channels", params.channels)
      .arg("dataSize", data.size())
      .arg("locator", source.locator().str());

    auto add_image_data = [&](auto& glapi, auto& pgts) {
        if(params.dimensions == 3) {
            if(glapi.texture_sub_image3d) {
                glapi.texture_sub_image3d(
                  pgts.tex,
                  params.level,
                  params.x_offs,
                  params.y_offs,
                  params.z_offs,
                  params.width,
                  params.height,
                  params.depth,
                  oglplus::pixel_format{params.format},
                  oglplus::pixel_data_type{params.data_type},
                  data);
            } else if(glapi.tex_sub_image3d) {
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
                glapi.tex_sub_image3d(
                  target,
                  params.level,
                  params.x_offs,
                  params.y_offs,
                  params.z_offs,
                  params.width,
                  params.height,
                  params.depth,
                  oglplus::pixel_format{params.format},
                  oglplus::pixel_data_type{params.data_type},
                  data);
            }
        } else if(params.dimensions == 2) {
            if(glapi.texture_sub_image2d) {
                glapi.texture_sub_image2d(
                  pgts.tex,
                  params.level,
                  params.x_offs,
                  params.y_offs,
                  params.width,
                  params.height,
                  oglplus::pixel_format{params.format},
                  oglplus::pixel_data_type{params.data_type},
                  data);
            } else if(glapi.tex_sub_image2d) {
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
                glapi.tex_sub_image2d(
                  target,
                  params.level,
                  params.x_offs,
                  params.y_offs,
                  params.width,
                  params.height,
                  oglplus::pixel_format{params.format},
                  oglplus::pixel_data_type{params.data_type},
                  data);
            }
        } else if(params.dimensions == 1) {
            if(glapi.texture_sub_image1d) {
                glapi.texture_sub_image1d(
                  pgts.tex,
                  params.level,
                  params.x_offs,
                  params.width,
                  oglplus::pixel_format{params.format},
                  oglplus::pixel_data_type{params.data_type},
                  data);
            } else if(glapi.tex_sub_image1d) {
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
                glapi.tex_sub_image1d(
                  target,
                  params.level,
                  params.x_offs,
                  params.width,
                  oglplus::pixel_format{params.format},
                  oglplus::pixel_data_type{params.data_type},
                  data);
            }
        }
    };

    if(is(resource_kind::gl_texture)) {
        if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
            auto& pgts = std::get<_pending_gl_texture_state>(_state);
            if(pgts.tex) [[likely]] {
                if(const auto pos{
                     pgts.pending_requests.find(source.request_id())};
                   pos != pgts.pending_requests.end()) {
                    pgts.level_images_done.set(std_size(params.level), true);
                    add_image_data(pgts.video.get().gl_api(), pgts);

                    pgts.pending_requests.erase(pos);
                    if(!_finish_gl_texture(pgts)) {
                        return;
                    }
                }
            }
        }
    } else if(is(resource_kind::gl_texture_update)) {
        if(std::holds_alternative<_pending_gl_texture_update_state>(_state)) {
            auto& pgts = std::get<_pending_gl_texture_update_state>(_state);
            add_image_data(pgts.video.get().gl_api(), pgts);
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture_image(
  url locator,
  oglplus::texture_target target) noexcept -> resource_request_result {
    return request_gl_texture_image(
      std::move(locator), target, resource_gl_texture_image_params{});
}
//------------------------------------------------------------------------------
// valtree_gl_buffer_builder
//------------------------------------------------------------------------------
struct resource_gl_buffer_params {
    oglplus::gl_types::sizei_type data_size{0};
    oglplus::gl_types::enum_type data_type{0};
};
//------------------------------------------------------------------------------
class valtree_gl_buffer_builder
  : public valtree_builder_base<valtree_gl_buffer_builder> {
    using base = valtree_builder_base<valtree_gl_buffer_builder>;

public:
    valtree_gl_buffer_builder(
      const std::shared_ptr<pending_resource_info>& info,
      video_context& video,
      oglplus::buffer_target buf_target) noexcept
      : base{info}
      , _video{video}
      , _buf_target{buf_target} {}

    using base::do_add;

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if(path.size() == 1) {
            if(path.starts_with("label")) {
                if(has_value(data)) {
                    if(auto parent{_parent.lock()}) {
                        extract(parent).add_label(extract(data));
                    }
                }
            }
        }
    }

    // TODO
    void do_add(const basic_string_path&, const auto&) noexcept {}

private:
    video_context& _video;
    resource_gl_buffer_params _params{};
    url _image_locator;
    oglplus::buffer_target _buf_target;
    bool _success{true};
};
//------------------------------------------------------------------------------
auto make_valtree_gl_buffer_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  video_context& video,
  oglplus::buffer_target target) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_gl_buffer_builder>(parent, video, target);
}
//------------------------------------------------------------------------------
auto pending_resource_info::handle_gl_buffer_params(
  const resource_gl_buffer_params& params) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_buffer_state>(_state)) {
        _parent.log_info("loaded GL buffer storage parameters")
          .arg("dataSize", params.data_size);
        // TODO
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
auto pending_resource_info::_finish_gl_buffer(
  _pending_gl_buffer_state& pgbs) noexcept -> bool {
    if(pgbs.loaded && pgbs.pending_requests.empty()) {
        _parent.log_info("loaded and set-up GL buffer object")
          .arg("requestId", _request_id)
          .arg("locator", _locator.str());
        // TODO
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_gl_buffer_data(
  const pending_resource_info& source,
  const oglplus::buffer_target,
  const resource_gl_buffer_data_params& params,
  const memory::const_block data) noexcept {
    _parent.log_info("loaded GL buffer sub-image")
      .arg("requestId", _request_id)
      .arg("dataSize", data.size())
      .arg("locator", source.locator().str());
    // TODO
}
//------------------------------------------------------------------------------
} // namespace eagine::app
