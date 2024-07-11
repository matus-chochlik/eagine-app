/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app;

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.reflection;
import eagine.core.runtime;
import eagine.core.utility;
import eagine.core.container;
import eagine.core.value_tree;
import eagine.core.main_ctx;
import eagine.core.c_api;
import eagine.oglplus;
import eagine.shapes;

namespace eagine::app {
//------------------------------------------------------------------------------
// valtree_gl_program_builder
//------------------------------------------------------------------------------
class valtree_gl_program_builder
  : public valtree_builder_base<valtree_gl_program_builder> {
    using base = valtree_builder_base<valtree_gl_program_builder>;

public:
    valtree_gl_program_builder(
      const shared_holder<pending_resource_info>& info,
      const oglplus::shared_gl_api_context& gl_context) noexcept
      : base{"GLprgBuldr", info}
      , _gl_context{gl_context} {}

    auto max_token_size() noexcept -> span_size_t final {
        return 256;
    }

    using base::do_add;

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if(path.has_size(1)) {
            if(path.starts_with("label")) {
                if(data.has_single_value()) {
                    if(auto parent{_parent.lock()}) {
                        parent->add_label(*data);
                    }
                }
            }
        } else if((path.has_size(3)) and data) {
            if((path.starts_with("inputs")) and (path.ends_with("attrib"))) {
                if(const auto kind{
                     from_string<shapes::vertex_attrib_kind>(*data)}) {
                    _attrib_kind = *kind;
                } else {
                    _input_name.clear();
                }
            }
            if(path.starts_with("shaders")) {
                if(path.ends_with("url")) {
                    if(data.has_single_value()) {
                        _shdr_locator = {to_string(*data)};
                    }
                } else if(path.ends_with("type")) {
                    if(const auto conv{
                         from_strings<oglplus::shader_type>(data)}) {
                        _shdr_type = *conv;
                    }
                }
            }
        }
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        if((path.has_size(3)) and data) {
            if((path.starts_with("inputs")) and (path.ends_with("variant"))) {
                _attrib_variant_index = span_size(*data);
            }
        }
    }

    void add_object(const basic_string_path& path) noexcept final {
        if(path.has_size(2)) {
            if(path.starts_with("inputs")) {
                _input_name = to_string(path.back());
                _attrib_kind = shapes::vertex_attrib_kind::position;
                _attrib_variant_index = 0;
            } else if(path.starts_with("shaders")) {
                _shdr_locator = {};
                _shdr_type = _gl_context.gl_api().constants().fragment_shader;
            }
        }
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.has_size(2)) {
            if(path.starts_with("inputs")) {
                if(not _input_name.empty()) {
                    if(auto parent{_parent.lock()}) {
                        parent->add_gl_program_input_binding(
                          std::move(_input_name),
                          {_attrib_kind, _attrib_variant_index});
                    }
                }
            } else if(path.starts_with("shaders")) {
                if(_shdr_locator) {
                    if(auto parent{_parent.lock()}) {
                        auto& loader = parent->loader();
                        if(auto src_request{loader.request_gl_shader(
                             {.locator = _shdr_locator},
                             _gl_context,
                             _shdr_type)}) {
                            src_request.set_continuation(parent);
                            if(not parent->add_gl_program_shader_request(
                                 src_request.request_id())) [[unlikely]] {
                                src_request.info().mark_finished();
                                parent->mark_finished();
                            }
                        }
                    }
                }
            }
        }
    }

private:
    oglplus::shared_gl_api_context _gl_context;
    std::string _input_name;
    oglplus::shader_type _shdr_type;
    url _shdr_locator;
    shapes::vertex_attrib_kind _attrib_kind;
    span_size_t _attrib_variant_index{0};
};
//------------------------------------------------------------------------------
auto make_valtree_gl_program_builder(
  const shared_holder<pending_resource_info>& parent,
  const oglplus::shared_gl_api_context& gl_context) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_gl_program_builder>, parent, gl_context};
}
//------------------------------------------------------------------------------
// valtree_gl_texture_image_loader
//------------------------------------------------------------------------------
template <typename T>
static auto gl_enum_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return from_strings<T>(data)
      .and_then([&](auto conv) -> tribool {
          v = to_underlying(conv);
          return true;
      })
      .value_or(false);
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
static auto texture_min_filter_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::texture_min_filter>(data, v);
}
//------------------------------------------------------------------------------
static auto texture_mag_filter_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::texture_min_filter>(data, v);
}
//------------------------------------------------------------------------------
static auto texture_wrap_mode_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    return gl_enum_from_string<oglplus::texture_wrap_mode>(data, v);
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
      shared_holder<pending_resource_info> info,
      oglplus::texture_target target,
      const resource_gl_texture_image_params& params) noexcept
      : base{"GLtxiBuldr", std::move(info)}
      , _tex_data{*this, 1024 * 1024 * 4, nothing}
      , _target{target}
      , _params{params} {}

    auto append_image_data(const memory::const_block blk) noexcept -> bool {
        log_debug("appending texture image data")
          .tag("apndImgDta")
          .arg("offset", _tex_data.size())
          .arg("size", blk.size());
        memory::append_to(blk, _tex_data);
        //  TODO: progressive image specification once we have enough
        //  data for width * some constant so that the temp buffer
        //  doesn't get too big
        return true;
    }

    auto init_decompression(data_compression_method method) noexcept -> bool {
        if(const auto parent{_parent.lock()}) {
            _decompression = stream_decompression{
              data_compressor{method, parent->loader().buffers()},
              make_callable_ref<
                &valtree_gl_texture_image_loader::append_image_data>(this),
              method};
            return true;
        }
        return false;
    }

    auto max_token_size() noexcept -> span_size_t final {
        return 256;
    }

    using base::do_add;

    template <std::integral T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept {
        if(path.has_size(1)) {
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
                _params.dimensions = std::max(_params.dimensions, 2);
            } else if(path.starts_with("depth")) {
                _success &= assign_if_fits(data, _params.depth);
                _params.dimensions = std::max(_params.dimensions, 3);
            } else if(path.starts_with("data_type")) {
                _success &= assign_if_fits(data, _params.data_type);
            } else if(path.starts_with("format")) {
                _success &= assign_if_fits(data, _params.format);
            } else if(path.starts_with("iformat")) {
                _success &= assign_if_fits(data, _params.iformat);
            }
        } else if(path.has_size(2)) {
            if(path.starts_with("data")) {
                _success &= append_image_data(as_bytes(data));
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const float> data) noexcept {
        if(path.has_size(2)) {
            if(path.starts_with("data")) {
                _success &= append_image_data(as_bytes(data));
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const double> data) noexcept {
        if(path.has_size(2)) {
            if(path.starts_with("data")) {
                _float_data.clear();
                _float_data.reserve(std_size(data.size()));
                for(const auto d : data) {
                    _float_data.push_back(float(d));
                }
                _success &= append_image_data(as_bytes(view(_float_data)));
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept {
        if(path.has_size(1)) {
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
                         from_string<data_compression_method>(*data)}) {
                        _success &= init_decompression(*method);
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
        if(not _decompression.is_initialized()) {
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
                parent->handle_gl_texture_image(_target, _params, _tex_data);
            }
            parent->mark_finished();
            return _success;
        }
        return false;
    }

    void failed() noexcept final {
        if(const auto parent{_parent.lock()}) {
            parent->mark_finished();
        }
    }

private:
    main_ctx_buffer _tex_data;
    std::vector<float> _float_data;
    stream_decompression _decompression;
    oglplus::texture_target _target;
    resource_gl_texture_image_params _params{};
    bool _success{true};
};
//------------------------------------------------------------------------------
auto make_valtree_gl_texture_image_loader(
  const shared_holder<pending_resource_info>& parent,
  oglplus::texture_target target,
  const resource_gl_texture_image_params& params) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_gl_texture_image_loader>, parent, target, params};
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
    bool generate_mipmap{false};
};
//------------------------------------------------------------------------------
class valtree_gl_texture_builder
  : public valtree_builder_base<valtree_gl_texture_builder> {
    using base = valtree_builder_base<valtree_gl_texture_builder>;

public:
    valtree_gl_texture_builder(
      const shared_holder<pending_resource_info>& info,
      const oglplus::shared_gl_api_context& gl_context,
      oglplus::texture_target tex_target,
      oglplus::texture_unit tex_unit) noexcept
      : base{"GLtexBuldr", info}
      , _gl_context{gl_context}
      , _tex_data{*this, 1024 * 1024 * 4}
      , _tex_target{tex_target}
      , _tex_unit{tex_unit} {}

    auto append_image_data(const memory::const_block blk) noexcept -> bool {
        log_debug("appending texture image data")
          .tag("apndImgDta")
          .arg("offset", _tex_data.size())
          .arg("size", blk.size());
        memory::append_to(blk, _tex_data);
        return true;
    }

    auto init_decompression(data_compression_method method) noexcept -> bool {
        if(const auto parent{_parent.lock()}) {
            _decompression = stream_decompression{
              data_compressor{method, parent->loader().buffers()},
              make_callable_ref<&valtree_gl_texture_builder::append_image_data>(
                this),
              method};
            return true;
        }
        return false;
    }

    auto max_token_size() noexcept -> span_size_t final {
        return 256;
    }

    using base::do_add;

    template <std::integral T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept {
        if(path.has_size(1)) {
            if(path.starts_with("levels")) {
                _success &= assign_if_fits(data, _params.levels);
            } else if(path.starts_with("width")) {
                _success &= assign_if_fits(data, _params.width);
                _params.dimensions = std::max(_params.dimensions, 1);
            } else if(path.starts_with("height")) {
                _success &= assign_if_fits(data, _params.height);
                _params.dimensions = std::max(_params.dimensions, 2);
            } else if(path.starts_with("depth")) {
                _success &= assign_if_fits(data, _params.depth);
                _params.dimensions = std::max(_params.dimensions, 3);
            } else if(path.starts_with("data_type")) {
                _success &= assign_if_fits(data, _params.data_type);
            } else if(path.starts_with("format")) {
                _success &= assign_if_fits(data, _params.format);
            } else if(path.starts_with("iformat")) {
                _success &= assign_if_fits(data, _params.iformat);
            }
        } else if(path.has_size(3)) {
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
      const span<const bool> data) noexcept {
        if(path.has_size(1)) {
            if(path.starts_with("generate_mipmap")) {
                _success &= assign_if_fits(data, _params.generate_mipmap);
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept {
        if(path.has_size(1)) {
            using It = oglplus::gl_types::int_type;
            using Et = oglplus::gl_types::enum_type;

            if(path.starts_with("label")) {
                if(data.has_single_value()) {
                    if(const auto parent{_parent.lock()}) {
                        parent->add_label(*data);
                    }
                }
            } else if(path.starts_with("data_type")) {
                _success &=
                  texture_data_type_from_string(data, _params.data_type);
            } else if(path.starts_with("format")) {
                _success &= texture_format_from_string(data, _params.format);
            } else if(path.starts_with("iformat")) {
                _success &= texture_iformat_from_string(data, _params.iformat);
            } else if(path.starts_with("min_filter")) {
                Et e{0};
                if(_success &= texture_min_filter_from_string(data, e)) {
                    _i_params.emplace_back(0x2801, It(e));
                }
            } else if(path.starts_with("mag_filter")) {
                Et e{0};
                if(_success &= texture_mag_filter_from_string(data, e)) {
                    _i_params.emplace_back(0x2800, It(e));
                }
            } else if(path.starts_with("wrap_s")) {
                Et e{0};
                if(_success &= texture_wrap_mode_from_string(data, e)) {
                    _i_params.emplace_back(0x2802, It(e));
                }
            } else if(path.starts_with("wrap_t")) {
                Et e{0};
                if(_success &= texture_wrap_mode_from_string(data, e)) {
                    _i_params.emplace_back(0x2803, It(e));
                }
            } else if(path.starts_with("wrap_r")) {
                Et e{0};
                if(_success &= texture_wrap_mode_from_string(data, e)) {
                    _i_params.emplace_back(0x8072, It(e));
                }
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
            } else if(path.starts_with("data_filter")) {
                if(data.has_single_value()) {
                    if(const auto method{
                         from_string<data_compression_method>(*data)}) {
                        _success &= init_decompression(*method);
                    } else {
                        _success = false;
                    }
                } else {
                    _success = false;
                }
            }
        } else if(path.has_size(3)) {
            if(path.starts_with("images")) {
                if(path.ends_with("url")) {
                    if(data.has_single_value()) {
                        _image_locator = {to_string(*data)};
                    } else {
                        _success = false;
                    }
                } else if(path.ends_with("target")) {
                    oglplus::gl_types::enum_type tgt{0};
                    if(texture_target_from_string(data, tgt)) {
                        _image_target = oglplus::texture_target{tgt};
                    } else {
                        log_error("invalid texture target '${name}'")
                          .arg("name", *data);
                        _success = false;
                    }
                }
            }
        }
    }

    void add_object(const basic_string_path& path) noexcept final {
        if(path.has_size(2)) {
            if(path.starts_with("images")) {
                _image_locator = {};
                _image_params = {};
                _image_target = _tex_target;
            }
        }
    }

    void unparsed_data(span<const memory::const_block> data) noexcept final {
        if(_success and _decompression.is_initialized()) {
            for(const auto& blk : data) {
                _decompression.next(blk);
            }
        }
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.empty()) {
            if(_success) {
                if(const auto parent{_parent.lock()}) {
                    _success &= parent->handle_gl_texture_params(_params);
                } else {
                    _success = false;
                }
            }
        } else if(path.has_size(2)) {
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
            if(_success and _decompression.is_initialized()) {
                resource_gl_texture_image_params img_params{
                  .dimensions = _params.dimensions,
                  .level = 0,
                  .width = _params.width,
                  .height = _params.height,
                  .depth = _params.depth,
                  .iformat = _params.iformat,
                  .format = _params.format,
                  .data_type = _params.data_type};
                parent->handle_gl_texture_image(
                  _tex_target, img_params, _tex_data);
            }
            if(_success) {
                for(auto& [loc, tgt, para] : _image_requests) {
                    const auto& rqparam{parent->parameters()};
                    const auto img_request{
                      parent->loader().request_gl_texture_image(
                        {.locator = std::move(loc),
                         .max_time = rqparam.max_time,
                         .priority = rqparam.priority},
                        tgt,
                        para)};
                    img_request.set_continuation(parent);
                    parent->add_gl_texture_image_request(
                      img_request.request_id());
                }
                _image_requests.clear();
                for(const auto [param, value] : _i_params) {
                    parent->handle_gl_texture_i_param(
                      oglplus::texture_parameter{param}, value);
                }
                if(_params.generate_mipmap) {
                    parent->handle_gl_texture_generate_mipmap();
                }
                parent->mark_loaded();
            } else {
                parent->mark_finished();
            }
            return _success;
        }
        return false;
    }

private:
    oglplus::shared_gl_api_context _gl_context;
    main_ctx_buffer _tex_data;
    stream_decompression _decompression;
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
  const shared_holder<pending_resource_info>& parent,
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_gl_texture_builder>, parent, gl_context, target, unit};
}
//------------------------------------------------------------------------------
static void _adjust_texture_dimensions(
  const oglplus::texture_target target,
  auto& pgts,
  auto& params) noexcept {
    const auto& GL{pgts.gl_context.gl_api().constants()};
    if(target == GL.texture_2d_array) {
        params.dimensions = std::max(params.dimensions, 3);
    } else if(target == GL.texture_1d_array) {
        params.dimensions = std::max(params.dimensions, 2);
    }
}
//------------------------------------------------------------------------------
static void _adjust_texture_z_offs(
  const oglplus::texture_target target,
  auto& pgts,
  auto& params) noexcept {
    const auto& GL{pgts.gl_context.gl_api().constants()};
    static const std::array<oglplus::texture_target, 6> cm_faces{
      {GL.texture_cube_map_positive_x,
       GL.texture_cube_map_negative_x,
       GL.texture_cube_map_positive_y,
       GL.texture_cube_map_negative_y,
       GL.texture_cube_map_positive_z,
       GL.texture_cube_map_negative_z}};

    int z_offs = 0;
    for(const auto cm_face : cm_faces) {
        if(target == cm_face) {
            params.z_offs = z_offs;
            params.dimensions = std::max(params.dimensions, 3);
            break;
        }
        ++z_offs;
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_adjust_gl_texture_params(
  const oglplus::texture_target target,
  const _pending_gl_texture_state& pgts,
  resource_gl_texture_params& params) noexcept {
    _adjust_texture_dimensions(target, pgts, params);
}
//------------------------------------------------------------------------------
void pending_resource_info::_adjust_gl_texture_params(
  const oglplus::texture_target target,
  const _pending_gl_texture_state& pgts,
  resource_gl_texture_image_params& params) noexcept {
    _adjust_texture_dimensions(target, pgts, params);
    _adjust_texture_z_offs(target, pgts, params);
}
//------------------------------------------------------------------------------
void pending_resource_info::_adjust_gl_texture_params(
  const oglplus::texture_target target,
  const _pending_gl_texture_update_state& pgts,
  resource_gl_texture_image_params& params) noexcept {
    _adjust_texture_dimensions(target, pgts, params);
    _adjust_texture_z_offs(target, pgts, params);
}
//------------------------------------------------------------------------------
auto pending_resource_info::_handle_pending_gl_texture_state(
  auto& gl,
  auto&,
  auto& glapi,
  const _pending_gl_texture_state& pgts,
  const resource_gl_texture_params& params) noexcept -> bool {

    gl.active_texture(pgts.tex_unit);
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
    return false;
}
//------------------------------------------------------------------------------
auto pending_resource_info::handle_gl_texture_params(
  resource_gl_texture_params& params) noexcept -> bool {
    if(const auto pgts{get_if<_pending_gl_texture_state>(_state)}) {
        _adjust_gl_texture_params(pgts->tex_target, *pgts, params);
        _parent.log_info("loaded GL texture storage parameters")
          .arg("levels", params.levels)
          .arg("width", params.width)
          .arg("height", params.height)
          .arg("depth", params.depth)
          .arg("dimensions", params.dimensions)
          .arg("iformat", params.iformat);

        pgts->pparams = &params;
        pgts->levels = span_size(params.levels);

        const auto& glapi{pgts->gl_context.gl_api()};
        const auto& [gl, GL] = glapi;
        return _handle_pending_gl_texture_state(gl, GL, glapi, *pgts, params);
    }
    return false;
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_gl_texture_i_param(
  const oglplus::texture_parameter param,
  const oglplus::gl_types::int_type value) noexcept {
    _parent.log_info("setting GL texture parameter")
      .arg("requestId", _request_id)
      .arg("locator", parameters().locator.str());

    if(is(resource_kind::gl_texture)) {
        if(const auto pgts{get_if<_pending_gl_texture_state>(_state)}) {
            if(pgts->tex) [[likely]] {
                const auto& gl{pgts->gl_context.gl_api().operations()};
                if(gl.texture_parameter_i) {
                    gl.texture_parameter_i(pgts->tex, param, value);
                } else if(gl.tex_parameter_i) {
                    gl.tex_parameter_i(pgts->tex_target, param, value);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_gl_texture_generate_mipmap() noexcept {
    _parent.log_info("requesting GL texture mipmap generation")
      .arg("requestId", _request_id)
      .arg("locator", parameters().locator.str());

    if(is(resource_kind::gl_texture)) {
        if(const auto pgts{get_if<_pending_gl_texture_state>(_state)}) {
            pgts->generate_mipmap = true;
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
      .arg("locator", parameters().locator.str());

    const auto& gl{pgts.gl_context.gl_api().operations()};
    if(gl.clear_tex_image) {
        gl.clear_tex_image(
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
  resource_gl_texture_image_params& tex_params,
  const memory::const_block data) noexcept {

    _parent.log_info("loaded GL texture sub-image")
      .arg("requestId", _request_id)
      .arg("level", tex_params.level)
      .arg("xoffs", tex_params.x_offs)
      .arg("yoffs", tex_params.y_offs)
      .arg("zoffs", tex_params.z_offs)
      .arg("width", tex_params.width)
      .arg("height", tex_params.height)
      .arg("depth", tex_params.depth)
      .arg("dimensions", tex_params.dimensions)
      .arg("channels", tex_params.channels)
      .arg("dataSize", data.size())
      .arg("locator", source.parameters().locator.str());

    auto add_image_data{[&](auto& glapi, auto& pgts, auto& params) {
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
    }};

    if(is(resource_kind::gl_texture)) {
        if(const auto pgts{get_if<_pending_gl_texture_state>(_state)}) {
            if(pgts->tex) [[likely]] {
                _adjust_gl_texture_params(target, *pgts, tex_params);
                pgts->level_images_done.set(std_size(tex_params.level), true);
                add_image_data(pgts->gl_context.gl_api(), *pgts, tex_params);

                if(const auto found{eagine::find(
                     pgts->pending_requests, source.request_id())}) {
                    pgts->pending_requests.erase(found.position());
                }
                if(not _finish_gl_texture(*pgts)) {
                    return;
                }
            }
        }
    } else if(is(resource_kind::gl_texture_update)) {
        if(const auto pgts{get_if<_pending_gl_texture_update_state>(_state)}) {
            _adjust_gl_texture_params(target, *pgts, tex_params);
            add_image_data(pgts->gl_context.gl_api(), *pgts, tex_params);
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture_image(
  const resource_request_params& params,
  oglplus::texture_target target) noexcept -> resource_request_result {
    return request_gl_texture_image(
      params, target, resource_gl_texture_image_params{});
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
      const shared_holder<pending_resource_info>& info,
      const oglplus::shared_gl_api_context& gl_context,
      oglplus::buffer_target buf_target) noexcept
      : base{"GLbufBuldr", info}
      , _gl_context{gl_context}
      , _buf_target{buf_target} {}

    auto max_token_size() noexcept -> span_size_t final {
        return 256;
    }

    using base::do_add;

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if(path.has_size(1)) {
            if(path.starts_with("label")) {
                if(data.has_single_value()) {
                    if(auto parent{_parent.lock()}) {
                        parent->add_label(*data);
                    }
                }
            }
        }
    }

    // TODO
    void do_add(const basic_string_path&, const auto&) noexcept {}

private:
    oglplus::shared_gl_api_context _gl_context;
    resource_gl_buffer_params _params{};
    url _image_locator;
    oglplus::buffer_target _buf_target;
    bool _success{true};
};
//------------------------------------------------------------------------------
auto make_valtree_gl_buffer_builder(
  const shared_holder<pending_resource_info>& parent,
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::buffer_target target) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_gl_buffer_builder>, parent, gl_context, target};
}
//------------------------------------------------------------------------------
auto pending_resource_info::handle_gl_buffer_params(
  const resource_gl_buffer_params& params) noexcept -> bool {
    if(const auto pgbs{get_if<_pending_gl_buffer_state>(_state)}) {
        _parent.log_info("loaded GL buffer storage parameters")
          .arg("dataSize", params.data_size);
        // TODO
        (void)pgbs;
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
auto pending_resource_info::_finish_gl_buffer(
  _pending_gl_buffer_state& pgbs) noexcept -> bool {
    if(pgbs.loaded and pgbs.pending_requests.empty()) {
        apply_label();

        _parent.log_info("loaded and set-up GL buffer object")
          .arg("requestId", _request_id)
          .arg("locator", _params.locator.str());
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
      .arg("locator", source.parameters().locator.str());
    // TODO
}
//------------------------------------------------------------------------------
} // namespace eagine::app
