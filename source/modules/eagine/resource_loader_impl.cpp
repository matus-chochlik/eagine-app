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
import eagine.core.memory;
import eagine.core.string;
import eagine.core.container;
import eagine.core.reflection;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.shapes;
import eagine.oglplus;
import eagine.msgbus;

import <iostream>;

namespace eagine::app {
//------------------------------------------------------------------------------
// valtree_gl_program_builder
//------------------------------------------------------------------------------
class valtree_gl_program_builder
  : public valtree::object_builder_impl<valtree_gl_program_builder> {
public:
    valtree_gl_program_builder(
      const std::shared_ptr<pending_resource_info>& info,
      video_context& vc) noexcept
      : _parent{info}
      , _video{vc} {}

    void request_shader(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if(auto parent{_parent.lock()}) {
            auto& loader = extract(parent).loader();
            auto& GL = _video.gl_api().constants();
            const auto delegate = [&, this](oglplus::shader_type shdr_type) {
                if(auto src_request{loader.request_gl_shader(
                     url{to_string(extract(data))}, shdr_type, _video)}) {
                    src_request.set_continuation(parent);
                    if(!extract(parent).add_gl_program_shader_request(
                         src_request.request_id())) [[unlikely]] {
                        src_request.info().mark_finished();
                        extract(parent).mark_finished();
                    }
                }
            };
            if(path.back() == "fragment") {
                delegate(GL.fragment_shader);
            } else if(path.back() == "vertex") {
                delegate(GL.vertex_shader);
            } else if(path.back() == "geometry") {
                delegate(GL.geometry_shader);
            } else if(path.back() == "compute") {
                delegate(GL.compute_shader);
            } else if(path.back() == "tess_evaluation") {
                delegate(GL.tess_evaluation_shader);
            } else if(path.back() == "tess_control") {
                delegate(GL.tess_control_shader);
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if(path.size() == 1) {
            if(path.front() == "label") {
                if(has_value(data)) {
                    if(auto parent{_parent.lock()}) {
                        extract(parent).add_label(extract(data));
                    }
                }
            }
        } else if(path.size() == 2) {
            if(path.front() == "urls") {
                request_shader(path, data);
            }
        } else if((path.size() == 3) && data) {
            if((path.front() == "inputs") && (path.back() == "attrib")) {
                if(const auto kind{
                     from_string<shapes::vertex_attrib_kind>(extract(data))}) {
                    _attrib_kind = extract(kind);
                } else {
                    _input_name.clear();
                }
            }
        }
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        if((path.size() == 3) && data) {
            if((path.front() == "inputs") && (path.back() == "variant")) {
                _attrib_variant_index = span_size(extract(data));
            }
        }
    }

    void do_add(const basic_string_path&, const auto&) noexcept {}

    void add_object(const basic_string_path& path) noexcept final {
        if((path.size() == 2) && (path.front() == "inputs")) {
            _input_name = to_string(path.back());
            _attrib_kind = shapes::vertex_attrib_kind::position;
            _attrib_variant_index = 0;
        }
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if((path.size() == 2) && (path.front() == "inputs")) {
            if(!_input_name.empty()) {
                if(auto parent{_parent.lock()}) {
                    extract(parent).add_gl_program_input_binding(
                      std::move(_input_name),
                      {_attrib_kind, _attrib_variant_index});
                }
            }
        }
    }

    void finish() noexcept final {
        if(auto parent{_parent.lock()}) {
            extract(parent).mark_loaded();
        }
    }

    void failed() noexcept final {
        if(auto parent{_parent.lock()}) {
            extract(parent).mark_finished();
        }
    }

private:
    std::weak_ptr<pending_resource_info> _parent;
    video_context& _video;
    std::string _input_name;
    shapes::vertex_attrib_kind _attrib_kind;
    span_size_t _attrib_variant_index{0};
};
//------------------------------------------------------------------------------
// valtree_gl_texture_image_loader
//------------------------------------------------------------------------------
static auto texture_target_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    if(data.has_single_value()) {
        const auto str{extract(data)};
        if(str == "texture_2d") {
            v = 0x0DE1;
            return true;
        } else if(str == "texture_2d_array") {
            v = 0x8C1A;
            return true;
        } else if(str == "texture_3d") {
            v = 0x806F;
            return true;
        } else if(str == "texture_1d") {
            v = 0x0DE0;
            return true;
        } else if(str == "texture_1d_array") {
            v = 0x8C18;
            return true;
        } else if(str == "texture_cube_map") {
            v = 0x8513;
            return true;
        } else if(str == "texture_cube_map_positive_x") {
            v = 0x8515;
            return true;
        } else if(str == "texture_cube_map_negative_x") {
            v = 0x8516;
            return true;
        } else if(str == "texture_cube_map_positive_y") {
            v = 0x8517;
            return true;
        } else if(str == "texture_cube_map_negative_y") {
            v = 0x8518;
            return true;
        } else if(str == "texture_cube_map_positive_z") {
            v = 0x8519;
            return true;
        } else if(str == "texture_cube_map_negative_z") {
            v = 0x851A;
            return true;
        }
        // TODO
    }
    return false;
}
//------------------------------------------------------------------------------
static auto texture_data_type_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    if(data.has_single_value()) {
        const auto str{extract(data)};
        if(str == "unsigned_byte") {
            v = 0x1401;
            return true;
        } else if(str == "unsigned_short") {
            v = 0x1403;
            return true;
        } else if(str == "unsigned_int") {
            v = 0x1405;
            return true;
        } else if(str == "float") {
            v = 0x1406;
            return true;
        }
        // TODO
    }
    return false;
}
//------------------------------------------------------------------------------
static auto texture_iformat_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    if(data.has_single_value()) {
        const auto str{extract(data)};
        if(str == "rgba8") {
            v = 0x8058;
            return true;
        } else if(str == "rgb8") {
            v = 0x8051;
            return true;
        } else if(str == "r8") {
            v = 0x8229;
            return true;
        }
        // TODO
    }
    return false;
}
//------------------------------------------------------------------------------
static auto texture_format_from_string(
  span<const string_view> data,
  oglplus::gl_types::enum_type& v) noexcept -> bool {
    if(data.has_single_value()) {
        const auto str{extract(data)};
        if(str == "rgba") {
            v = 0x1908;
            return true;
        } else if(str == "rgb") {
            v = 0x1907;
            return true;
        } else if(str == "red") {
            v = 0x1903;
            return true;
        }
        // TODO
    }
    return false;
}
//------------------------------------------------------------------------------
struct resource_texture_image_params {
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
  : public valtree::object_builder_impl<valtree_gl_texture_image_loader> {
public:
    valtree_gl_texture_image_loader(
      std::shared_ptr<pending_resource_info> info,
      oglplus::texture_target target) noexcept
      : _parent{std::move(info)}
      , _target{target} {
        if(const auto parent{_parent.lock()}) {
            extract(parent).loader().buffers().get(512 * 512);
        }
    }

    auto append_image_data(const memory::const_block blk) noexcept -> bool {
        if(const auto parent{_parent.lock()}) {
            extract(parent)
              .loader()
              .log_debug("appending texture image data")
              .tag("apndImgDta")
              .arg("offset", _temp.size())
              .arg("size", blk.size());
        }
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

    template <std::integral T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept {
        if(path.size() == 1) {
            if(path.front() == "level") {
                _success &= assign_if_fits(data, _params.level);
            } else if(path.front() == "x_offs") {
                _success &= assign_if_fits(data, _params.x_offs);
            } else if(path.front() == "y_offs") {
                _success &= assign_if_fits(data, _params.y_offs);
            } else if(path.front() == "z_offs") {
                _success &= assign_if_fits(data, _params.z_offs);
            } else if(path.front() == "channels") {
                _success &= assign_if_fits(data, _params.channels);
            } else if(path.front() == "width") {
                _success &= assign_if_fits(data, _params.width);
                _params.dimensions = std::max(_params.dimensions, 1);
            } else if(path.front() == "height") {
                _success &= assign_if_fits(data, _params.height);
                if(_params.height > 1) {
                    _params.dimensions = std::max(_params.dimensions, 2);
                }
            } else if(path.front() == "depth") {
                _success &= assign_if_fits(data, _params.depth);
                if(_params.depth > 1) {
                    _params.dimensions = std::max(_params.dimensions, 3);
                }
            } else if(path.front() == "data_type") {
                _success &= assign_if_fits(data, _params.data_type);
            } else if(path.front() == "format") {
                _success &= assign_if_fits(data, _params.format);
            } else if(path.front() == "iformat") {
                _success &= assign_if_fits(data, _params.iformat);
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept {
        if(path.size() == 1) {
            if(path.front() == "data_type") {
                _success &=
                  texture_data_type_from_string(data, _params.data_type);
            } else if(path.front() == "format") {
                _success &= texture_format_from_string(data, _params.format);
            } else if(path.front() == "iformat") {
                _success &= texture_iformat_from_string(data, _params.iformat);
            } else if(path.front() == "data_filter") {
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

    void do_add(const basic_string_path&, const auto&) noexcept {}

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

    void finish() noexcept final {
        if(const auto parent{_parent.lock()}) {
            if(_success) {
                _decompression.finish();
                extract(parent).handle_gl_texture_image(
                  _target, _params, _temp);
            } else {
                extract(parent).mark_finished();
            }
            extract(parent).loader().buffers().eat(std::move(_temp));
        }
    }

    void failed() noexcept final {
        if(const auto parent{_parent.lock()}) {
            extract(parent).mark_finished();
            extract(parent).loader().buffers().eat(std::move(_temp));
        }
    }

private:
    std::weak_ptr<pending_resource_info> _parent;
    memory::buffer _temp;
    stream_decompression _decompression;
    oglplus::texture_target _target;
    resource_texture_image_params _params{};
    bool _success{true};
};
//------------------------------------------------------------------------------
// valtree_gl_texture_builder
//------------------------------------------------------------------------------
struct resource_texture_params {
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
  : public valtree::object_builder_impl<valtree_gl_texture_builder> {
public:
    valtree_gl_texture_builder(
      const std::shared_ptr<pending_resource_info>& info,
      video_context& vc,
      oglplus::texture_target tex_target,
      oglplus::texture_unit tex_unit) noexcept
      : _parent{info}
      , _video{vc}
      , _tex_target{tex_target}
      , _tex_unit{tex_unit} {}

    template <std::integral T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept {
        if(path.size() == 1) {
            if(path.front() == "levels") {
                _success &= assign_if_fits(data, _params.levels);
            } else if(path.front() == "width") {
                _success &= assign_if_fits(data, _params.width);
                _params.dimensions = std::max(_params.dimensions, 1);
            } else if(path.front() == "height") {
                _success &= assign_if_fits(data, _params.height);
                if(_params.height > 1) {
                    _params.dimensions = std::max(_params.dimensions, 2);
                }
            } else if(path.front() == "depth") {
                _success &= assign_if_fits(data, _params.depth);
                if(_params.depth > 1) {
                    _params.dimensions = std::max(_params.dimensions, 3);
                }
            } else if(path.front() == "data_type") {
                _success &= assign_if_fits(data, _params.data_type);
            } else if(path.front() == "format") {
                _success &= assign_if_fits(data, _params.format);
            } else if(path.front() == "iformat") {
                _success &= assign_if_fits(data, _params.iformat);
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      const span<const string_view> data) noexcept {
        if(path.size() == 1) {
            if(path.front() == "label") {
                if(has_value(data)) {
                    if(const auto parent{_parent.lock()}) {
                        extract(parent).add_label(extract(data));
                    }
                }
            } else if(path.front() == "data_type") {
                _success &=
                  texture_data_type_from_string(data, _params.data_type);
            } else if(path.front() == "format") {
                _success &= texture_format_from_string(data, _params.format);
            } else if(path.front() == "iformat") {
                _success &= texture_iformat_from_string(data, _params.iformat);
            }
        } else if(path.size() == 3) {
            if(path.front() == "images") {
                if(path.back() == "url") {
                    if(has_value(data)) {
                        _image_locator = {to_string(extract(data))};
                    } else {
                        _success = false;
                    }
                } else if(path.back() == "target") {
                    oglplus::gl_types::enum_type tgt{0};
                    if(texture_target_from_string(data, tgt)) {
                        _tex_target = oglplus::texture_target{tgt};
                    } else {
                        _success = false;
                    }
                }
            }
        }
    }

    void do_add(const basic_string_path&, const auto&) noexcept {}

    void add_object(const basic_string_path& path) noexcept final {
        if(path.size() == 2) {
            if(path.front() == "images") {
                _image_locator = {};
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
            if(path.front() == "images") {
                if(_image_locator) {
                    _image_requests.emplace_back(
                      std::move(_image_locator), _image_target);
                }
            }
        }
    }

    void finish() noexcept final {
        if(const auto parent{_parent.lock()}) {
            if(_success) {
                for(auto& [loc, tgt] : _image_requests) {
                    const auto img_request{
                      extract(parent).loader().request_gl_texture_image(
                        std::move(loc), tgt)};
                    img_request.set_continuation(parent);
                    extract(parent).add_gl_texture_image_request(
                      img_request.request_id());
                }
                _image_requests.clear();
                extract(parent).mark_loaded();
            } else {
                extract(parent).mark_finished();
            }
        }
    }

    void failed() noexcept final {
        if(const auto parent{_parent.lock()}) {
            extract(parent).mark_finished();
        }
    }

private:
    std::weak_ptr<pending_resource_info> _parent;
    video_context& _video;
    resource_texture_params _params{};
    url _image_locator;
    oglplus::texture_target _tex_target;
    oglplus::texture_target _image_target;
    oglplus::texture_unit _tex_unit;
    std::vector<std::tuple<url, oglplus::texture_target>> _image_requests;
    bool _success{true};
};
//------------------------------------------------------------------------------
// pending_resource_info
//------------------------------------------------------------------------------
void pending_resource_info::mark_loaded() noexcept {
    if(std::holds_alternative<_pending_gl_program_state>(_state)) {
        auto& pgps = std::get<_pending_gl_program_state>(_state);
        pgps.loaded = true;
        _finish_gl_program(pgps);
    }
    if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        auto& pgts = std::get<_pending_gl_texture_state>(_state);
        pgts.loaded = true;
        _finish_gl_texture(pgts);
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::mark_finished() noexcept {
    _kind = resource_kind::finished;
}
//------------------------------------------------------------------------------
auto pending_resource_info::is_done() const noexcept -> bool {
    return _kind == resource_kind::finished;
}
//------------------------------------------------------------------------------
void pending_resource_info::add_label(const string_view label) noexcept {
    if(std::holds_alternative<_pending_gl_program_state>(_state)) {
        auto& pgps = std::get<_pending_gl_program_state>(_state);
        pgps.video.get().gl_api().object_label(pgps.prog, label);
    } else if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        auto& pgts = std::get<_pending_gl_texture_state>(_state);
        pgts.video.get().gl_api().object_label(pgts.tex, label);
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::add_valtree_stream_input(
  valtree::value_tree_stream_input input) noexcept {
    _state = _pending_valtree_traversal_state{.input = std::move(input)};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_shape_generator(
  std::shared_ptr<shapes::generator> gen) noexcept {
    _state = _pending_shape_generator_state{.generator = std::move(gen)};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_shape_context(video_context& video) noexcept {
    _state = _pending_gl_shape_state{.video = video};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_geometry_and_bindings_context(
  video_context& video,
  oglplus::vertex_attrib_bindings bindings,
  span_size_t draw_var_idx) noexcept {
    _state = _pending_gl_geometry_and_bindings_state{
      .video = video,
      .bindings = std::move(bindings),
      .draw_var_idx = draw_var_idx};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_shader_context(
  video_context& video,
  oglplus::shader_type shdr_type) noexcept {
    _state = _pending_gl_shader_state{.video = video, .shdr_type = shdr_type};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_program_context(video_context& vc) noexcept {
    oglplus::owned_program_name prog;
    vc.gl_api().create_program() >> prog;
    _state = _pending_gl_program_state{.video = vc, .prog = std::move(prog)};
}
//------------------------------------------------------------------------------
auto pending_resource_info::add_gl_program_shader_request(
  identifier_t request_id) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_program_state>(_state)) {
        auto& pgps = std::get<_pending_gl_program_state>(_state);
        pgps.pending_requests.insert(request_id);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
auto pending_resource_info::add_gl_program_input_binding(
  std::string name,
  shapes::vertex_attrib_variant vav) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_program_state>(_state)) {
        auto& pgps = std::get<_pending_gl_program_state>(_state);
        pgps.input_bindings.add(std::move(name), vav);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_gl_texture_image(
  const oglplus::texture_target target,
  const resource_texture_image_params& params,
  const memory::const_block data) noexcept {
    if(const auto cont{continuation()}) {
        extract(cont)._handle_gl_texture_image(*this, target, params, data);
    }
}
//------------------------------------------------------------------------------
auto pending_resource_info::add_gl_texture_image_request(
  identifier_t request_id) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        auto& pgts = std::get<_pending_gl_texture_state>(_state);
        pgts.pending_requests.insert(request_id);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_texture_update_context(
  video_context& vc,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept {
    _state = _pending_gl_texture_update_state{
      .video = vc, .tex_target = target, .tex_unit = unit, .tex = tex};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_texture_context(
  video_context& vc,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept {
    oglplus::owned_texture_name tex;
    vc.gl_api().gen_textures() >> tex;
    _state = _pending_gl_texture_state{
      .video = vc,
      .tex_target = target,
      .tex_unit = unit,
      .tex = std::move(tex)};
}
//------------------------------------------------------------------------------
auto pending_resource_info::handle_gl_texture_params(
  const resource_texture_params& params) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        auto& pgts = std::get<_pending_gl_texture_state>(_state);
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
auto pending_resource_info::update() noexcept -> work_done {
    some_true something_done;
    if(std::holds_alternative<_pending_shape_generator_state>(_state)) {
        if(const auto shape_gen{
             std::get<_pending_shape_generator_state>(_state).generator}) {
            if(const auto cont{continuation()}) {
                extract(cont)._handle_shape_generator(*this, shape_gen);
            }
            _parent.shape_generator_loaded(
              {.request_id = _request_id,
               .locator = _locator,
               .generator = shape_gen});
            something_done();
        }
        _state = std::monostate{};
        mark_finished();
    }
    return something_done;
}
//------------------------------------------------------------------------------
void pending_resource_info::cleanup() noexcept {
    if(std::holds_alternative<_pending_gl_program_state>(_state)) {
        auto& pgps = std::get<_pending_gl_program_state>(_state);
        if(pgps.prog) {
            pgps.video.get().gl_api().delete_program(std::move(pgps.prog));
        }
        _state = std::monostate{};
    } else if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        auto& pgts = std::get<_pending_gl_texture_state>(_state);
        if(pgts.tex) {
            pgts.video.get().gl_api().delete_textures(std::move(pgts.tex));
        }
        _state = std::monostate{};
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_json_text(
  const msgbus::blob_info&,
  const pending_resource_info&,
  const span_size_t offset,
  const memory::span<const memory::const_block> data) noexcept {
    _parent.log_debug("loaded JSON data")
      .arg("requestId", _request_id)
      .arg("offset", offset)
      .arg("locator", _locator.str());

    if(is(resource_kind::value_tree)) {
        auto tree{valtree::from_json_data(data, _parent.main_context().log())};
        if(const auto cont{continuation()}) {
            extract(cont)._handle_value_tree(*this, tree);
        }
        _parent.value_tree_loaded(
          {.request_id = _request_id, .locator = _locator, .tree = tree});
    } else if(is(resource_kind::value_tree_traversal)) {
        if(std::holds_alternative<_pending_valtree_traversal_state>(_state)) {
            auto& pvts = std::get<_pending_valtree_traversal_state>(_state);
            bool should_continue{true};
            for(auto chunk : data) {
                if(!pvts.input.consume_data(chunk)) {
                    pvts.input.finish();
                    should_continue = false;
                    break;
                }
            }
            if(should_continue) {
                return;
            }
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_yaml_text(
  const msgbus::blob_info&,
  const pending_resource_info&,
  const span_size_t,
  const memory::span<const memory::const_block>) noexcept {
    _parent.log_debug("loaded YAML text")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::value_tree)) {
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_value_tree(
  const pending_resource_info& source,
  const valtree::compound& tree) noexcept {
    _parent.log_info("loaded value tree")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::shape_generator)) {
        if(auto gen{shapes::from_value_tree(tree, _parent)}) {
            add_shape_generator(std::move(gen));
        }
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_shape_generator(
  const pending_resource_info& source,
  const std::shared_ptr<shapes::generator>& gen) noexcept {
    _parent.log_info("loaded shape geometry generator")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::gl_shape)) {
        if(std::holds_alternative<_pending_gl_shape_state>(_state)) {
            auto& pgss = std::get<_pending_gl_shape_state>(_state);
            const oglplus::shape_generator shape{
              pgss.video.get().gl_api(), gen};
            if(const auto cont{continuation()}) {
                extract(cont)._handle_gl_shape(*this, shape);
            }
            _parent.gl_shape_loaded(
              {.request_id = _request_id, .locator = _locator, .shape = shape});
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_gl_shape(
  const pending_resource_info& source,
  const oglplus::shape_generator& shape) noexcept {
    _parent.log_info("loaded shape generator wrapper")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::gl_geometry_and_bindings)) {
        if(std::holds_alternative<_pending_gl_geometry_and_bindings_state>(
             _state)) {
            auto temp{_parent.buffers().get()};
            const auto cleanup_temp{_parent.buffers().eat_later(temp)};
            auto& pggbs =
              std::get<_pending_gl_geometry_and_bindings_state>(_state);
            if(!pggbs.bindings) {
                pggbs.bindings = oglplus::vertex_attrib_bindings{shape};
            }
            geometry_and_bindings geom{
              shape,
              pggbs.bindings,
              shape.draw_variant(pggbs.draw_var_idx),
              pggbs.video,
              temp};
            _parent.gl_geometry_and_bindings_loaded(
              {.request_id = _request_id,
               .locator = _locator,
               .shape = shape,
               .ref = geom});
            if(geom) {
                geom.clean_up(pggbs.video);
            }
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_glsl_strings(
  const msgbus::blob_info&,
  const pending_resource_info& source,
  [[maybe_unused]] const span_size_t offset,
  const memory::span<const memory::const_block> data) noexcept {
    _parent.log_info("loaded GLSL string collection")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::glsl_source)) {
        std::vector<const oglplus::gl_types::char_type*> gl_strs;
        std::vector<oglplus::gl_types::int_type> gl_ints;
        gl_strs.reserve(std_size(data.size()));
        gl_ints.reserve(std_size(data.size()));

        for(const auto& blk : data) {
            gl_strs.emplace_back(
              reinterpret_cast<const oglplus::gl_types::char_type*>(
                blk.data()));
            gl_ints.emplace_back(
              static_cast<const oglplus::gl_types::int_type>(blk.size()));
        }
        const oglplus::glsl_source_ref glsl_src{
          data.size(), gl_strs.data(), gl_ints.data()};
        if(const auto cont{continuation()}) {
            extract(cont)._handle_glsl_source(*this, glsl_src);
        }
        _parent.glsl_source_loaded(
          {.request_id = _request_id, .locator = _locator, .source = glsl_src});
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_glsl_source(
  const pending_resource_info& source,
  const oglplus::glsl_source_ref& glsl_src) noexcept {
    _parent.log_info("loaded GLSL source object")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::gl_shader)) {
        if(std::holds_alternative<_pending_gl_shader_state>(_state)) {
            auto& pgss = std::get<_pending_gl_shader_state>(_state);
            const auto& [gl, GL] = pgss.video.get().gl_api();

            oglplus::owned_shader_name shdr;
            gl.create_shader(pgss.shdr_type) >> shdr;
            gl.shader_source(shdr, glsl_src);
            gl.compile_shader(shdr);
            if(const auto cont{continuation()}) {
                extract(cont)._handle_gl_shader(*this, shdr);
            }
            _parent.gl_shader_loaded(
              {.request_id = _request_id,
               .locator = _locator,
               .type = pgss.shdr_type,
               .name = shdr,
               .ref = shdr});

            if(shdr) {
                gl.delete_shader(std::move(shdr));
            }
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
auto pending_resource_info::_finish_gl_program(
  _pending_gl_program_state& pgps) noexcept -> bool {
    if(pgps.loaded && pgps.pending_requests.empty()) {
        _parent.log_info("loaded and linked GL program object")
          .arg("requestId", _request_id)
          .arg("locator", _locator.str());

        const auto& gl = pgps.video.get().gl_api().operations();
        gl.link_program(pgps.prog);
        _parent.gl_program_loaded(
          {.request_id = _request_id,
           .locator = _locator,
           .name = pgps.prog,
           .ref = pgps.prog,
           .input_bindings = pgps.input_bindings});

        if(pgps.prog) {
            gl.delete_program(std::move(pgps.prog));
        }
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_gl_shader(
  const pending_resource_info& source,
  oglplus::owned_shader_name& shdr) noexcept {
    _parent.log_info("loaded and compiled GL shader object")
      .arg("requestId", _request_id)
      .arg("locator", source.locator().str());

    if(is(resource_kind::gl_program)) {
        if(std::holds_alternative<_pending_gl_program_state>(_state)) {
            auto& pgps = std::get<_pending_gl_program_state>(_state);
            if(pgps.prog) [[likely]] {
                const auto& gl = pgps.video.get().gl_api().operations();

                if(const auto pos{
                     pgps.pending_requests.find(source.request_id())};
                   pos != pgps.pending_requests.end()) {
                    gl.attach_shader(pgps.prog, shdr);

                    pgps.pending_requests.erase(pos);
                    if(!_finish_gl_program(pgps)) {
                        return;
                    }
                }
            }
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_gl_texture_image(
  const pending_resource_info& source,
  const oglplus::texture_target target,
  const resource_texture_image_params& params,
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
                  pgts.tex_target,
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
                  pgts.tex_target,
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
                  pgts.tex_target,
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
auto pending_resource_info::_finish_gl_texture(
  _pending_gl_texture_state& pgts) noexcept -> bool {
    if(pgts.loaded && pgts.pending_requests.empty()) {
        _parent.log_info("loaded and set-up GL texture object")
          .arg("requestId", _request_id)
          .arg("locator", _locator.str());

        const auto& gl = pgts.video.get().gl_api().operations();
        // TODO: call this earlier (before all images are loaded)?
        _parent.gl_texture_loaded(
          {.request_id = _request_id,
           .locator = _locator,
           .name = pgts.tex,
           .ref = pgts.tex});
        _parent.gl_texture_images_loaded(
          {.request_id = _request_id, .locator = _locator, .name = pgts.tex});

        if(pgts.tex) {
            gl.delete_textures(std::move(pgts.tex));
        }
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_source_data(
  const msgbus::blob_info& binfo,
  const pending_resource_info& rinfo,
  const span_size_t offset,
  const memory::span<const memory::const_block> data) noexcept {
    switch(rinfo._kind) {
        case resource_kind::json_text:
            _handle_json_text(binfo, rinfo, offset, data);
            break;
        case resource_kind::yaml_text:
            _handle_json_text(binfo, rinfo, offset, data);
            break;
        case resource_kind::glsl_text:
            _handle_glsl_strings(binfo, rinfo, offset, data);
            break;
        default:
            break;
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_source_finished(
  const pending_resource_info&) noexcept {
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_source_cancelled(
  const pending_resource_info&) noexcept {
    _parent.resource_cancelled(_request_id, _kind, _locator);
    mark_finished();
}
//------------------------------------------------------------------------------
// resource_request_result
//------------------------------------------------------------------------------
auto resource_request_result::info() const noexcept -> pending_resource_info& {
    assert(_info);
    return *_info;
}
//------------------------------------------------------------------------------
// resource_loader
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_data_appended(
  identifier_t request_id,
  const span_size_t offset,
  const memory::span<const memory::const_block> data,
  const msgbus::blob_info& binfo) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        if(const auto& prinfo{std::get<1>(*pos)}) {
            if(const auto continuation{extract(prinfo).continuation()}) {
                extract(continuation)
                  .handle_source_data(binfo, extract(prinfo), offset, data);
            }
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_finished(identifier_t request_id) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        if(const auto& prinfo{std::get<1>(*pos)}) {
            auto& rinfo{extract(prinfo)};
            if(const auto continuation{rinfo.continuation()}) {
                extract(continuation).handle_source_finished(rinfo);
            }
            rinfo.mark_finished();
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(
  identifier_t request_id) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        if(const auto& prinfo{std::get<1>(*pos)}) {
            auto& rinfo{extract(prinfo)};
            if(const auto continuation{rinfo.continuation()}) {
                extract(continuation).handle_source_cancelled(rinfo);
            }
            resource_cancelled(request_id, rinfo._kind, rinfo._locator);
            rinfo.mark_finished();
        }
    }
}
//------------------------------------------------------------------------------
auto resource_loader::_cancelled_resource(
  const identifier_t request_id,
  url& locator,
  const resource_kind kind) noexcept -> resource_request_result {
    auto result{_cancelled.emplace(
      request_id,
      std::make_shared<pending_resource_info>(
        *this, request_id, std::move(locator), kind))};
    return {*std::get<0>(result), true};
}
//------------------------------------------------------------------------------
auto resource_loader::_new_resource(
  const identifier_t request_id,
  url locator,
  resource_kind kind) noexcept -> resource_request_result {
    const auto result{_pending.emplace(
      request_id,
      std::make_shared<pending_resource_info>(
        *this, request_id, std::move(locator), kind))};
    return {*std::get<0>(result), false};
}
//------------------------------------------------------------------------------
void resource_loader::_init() noexcept {
    connect<&resource_loader::_handle_stream_data_appended>(
      this, blob_stream_data_appended);
    connect<&resource_loader::_handle_stream_finished>(
      this, blob_stream_finished);
    connect<&resource_loader::_handle_stream_cancelled>(
      this, blob_stream_cancelled);
}
//------------------------------------------------------------------------------
auto resource_loader::request_value_tree(url locator) noexcept
  -> resource_request_result {
    if(locator.has_path_suffix(".json") || locator.has_scheme("json")) {
        if(const auto src_request{_new_resource(
             fetch_resource_chunks(
               locator,
               16 * 1024,
               msgbus::message_priority::normal,
               std::chrono::seconds{15}),
             resource_kind::json_text)}) {
            auto new_request{
              _new_resource(std::move(locator), resource_kind::value_tree)};
            src_request.set_continuation(new_request);

            return new_request;
        }
    }

    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_json_traversal(
  url locator,
  std::shared_ptr<valtree::value_tree_visitor> visitor,
  span_size_t max_token_size) noexcept -> resource_request_result {
    if(const auto src_request{_new_resource(
         stream_resource(
           locator, msgbus::message_priority::normal, std::chrono::hours{1}),
         resource_kind::json_text)}) {

        auto new_request{_new_resource(
          std::move(locator), resource_kind::value_tree_traversal)};
        new_request.info().add_valtree_stream_input(traverse_json_stream(
          std::move(visitor), max_token_size, buffers(), main_context().log()));
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_json_traversal(
  url locator,
  std::shared_ptr<valtree::object_builder> builder,
  span_size_t max_token_size) noexcept -> resource_request_result {
    return request_json_traversal(
      std::move(locator),
      valtree::make_building_value_tree_visitor(std::move(builder)),
      max_token_size);
}
//------------------------------------------------------------------------------
auto resource_loader::request_value_tree_traversal(
  url locator,
  std::shared_ptr<valtree::value_tree_visitor> visitor,
  span_size_t max_token_size) noexcept -> resource_request_result {
    if(locator.has_path_suffix(".json") || locator.has_scheme("json")) {
        return request_json_traversal(
          std::move(locator), std::move(visitor), max_token_size);
    }
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_value_tree_traversal(
  url locator,
  std::shared_ptr<valtree::object_builder> builder,
  span_size_t max_token_size) noexcept -> resource_request_result {
    return request_value_tree_traversal(
      std::move(locator),
      valtree::make_building_value_tree_visitor(std::move(builder)),
      max_token_size);
}
//------------------------------------------------------------------------------
auto resource_loader::request_shape_generator(url locator) noexcept
  -> resource_request_result {
    if(auto gen{shapes::shape_from(locator, main_context())}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::shape_generator)};
        new_request.info().add_shape_generator(std::move(gen));
        return new_request;
    } else if(const auto src_request{request_value_tree(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::shape_generator)};
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::shape_generator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shape(url locator, video_context& vc) noexcept
  -> resource_request_result {
    if(const auto src_request{request_shape_generator(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::gl_shape)};
        new_request.info().add_gl_shape_context(vc);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_shape);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_geometry_and_bindings(
  url locator,
  video_context& vc,
  oglplus::vertex_attrib_bindings bindings,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    if(const auto src_request{request_gl_shape(locator, vc)}) {
        auto new_request{_new_resource(
          std::move(locator), resource_kind::gl_geometry_and_bindings)};
        new_request.info().add_gl_geometry_and_bindings_context(
          vc, std::move(bindings), draw_var_idx);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(
      locator, resource_kind::gl_geometry_and_bindings);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_geometry_and_bindings(
  url locator,
  video_context& vc,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    return request_gl_geometry_and_bindings(
      std::move(locator), vc, {}, draw_var_idx);
}
//------------------------------------------------------------------------------
auto resource_loader::request_glsl_source(url locator) noexcept
  -> resource_request_result {
    if(locator.has_path_suffix(".glsl") || locator.has_scheme("glsl")) {
        if(const auto src_request{_new_resource(
             fetch_resource_chunks(
               locator,
               16 * 1024,
               msgbus::message_priority::normal,
               std::chrono::seconds{15}),
             resource_kind::glsl_text)}) {
            auto new_request{
              _new_resource(std::move(locator), resource_kind::glsl_source)};
            src_request.set_continuation(new_request);
            return new_request;
        }
    }

    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(
  url locator,
  oglplus::shader_type shdr_type,
  video_context& vc) noexcept -> resource_request_result {
    if(const auto src_request{request_glsl_source(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::gl_shader)};
        new_request.info().add_gl_shader_context(vc, shdr_type);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_shader);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(url locator, video_context& vc) noexcept
  -> resource_request_result {
    if(const auto type_arg{locator.argument("shader_type")}) {
        const auto& GL = vc.gl_api().constants();
        const auto delegate = [&GL, &locator, &vc, this](auto shdr_type) {
            return request_gl_shader(std::move(locator), shdr_type, vc);
        };
        if(type_arg == string_view{"fragment"}) {
            return delegate(GL.fragment_shader);
        }
        if(type_arg == string_view{"vertex"}) {
            return delegate(GL.vertex_shader);
        }
        if(type_arg == string_view{"geometry"}) {
            return delegate(GL.geometry_shader);
        }
        if(type_arg == string_view{"compute"}) {
            return delegate(GL.compute_shader);
        }
        if(type_arg == string_view{"tess_evaluation"}) {
            return delegate(GL.tess_evaluation_shader);
        }
        if(type_arg == string_view{"tess_control"}) {
            return delegate(GL.tess_control_shader);
        }
    }
    return _cancelled_resource(locator, resource_kind::gl_shader);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_program(url locator, video_context& vc) noexcept
  -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_program)};
    new_request.info().add_gl_program_context(vc);

    if(const auto src_request{request_value_tree_traversal(
         locator,
         std::make_shared<valtree_gl_program_builder>(new_request, vc),
         128)}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_program);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture_image(
  url locator,
  oglplus::texture_target target) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_texture_image)};

    if(const auto src_request{request_json_traversal(
         locator,
         std::make_shared<valtree_gl_texture_image_loader>(new_request, target),
         128)}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_texture_image);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture_update(
  url locator,
  video_context& vc,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept -> resource_request_result {
    if(const auto src_request{request_gl_texture_image(locator, target)}) {
        auto new_request{
          _new_resource(locator, resource_kind::gl_texture_update)};
        new_request.info().add_gl_texture_update_context(vc, target, unit, tex);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_texture_update);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(
  url locator,
  video_context& vc,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_texture)};
    new_request.info().add_gl_texture_context(vc, target, unit);

    if(const auto src_request{request_json_traversal(
         locator,
         std::make_shared<valtree_gl_texture_builder>(
           new_request, vc, target, unit),
         128)}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_texture);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(url locator, video_context&) noexcept
  -> resource_request_result {
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::update() noexcept -> work_done {
    some_true something_done{base::update()};

    for(auto& [request_id, pinfo] : _cancelled) {
        assert(pinfo);
        auto& info{extract(pinfo)};
        resource_cancelled(request_id, info._kind, info._locator);
        something_done();
    }
    _cancelled.clear();

    std::swap(_pending, _finished);

    something_done(_finished.erase_if([this](auto& entry) {
        auto& [request_id, pinfo] = entry;
        assert(pinfo);
        auto& info{extract(pinfo)};
        if(info.is_done()) {
            info.cleanup();
        } else {
            _pending.emplace(request_id, std::move(pinfo));
        }
        return true;
    }) > 0);

    for(auto& entry : _pending) {
        auto& pinfo{std::get<1>(entry)};
        assert(pinfo);
        something_done(extract(pinfo).update());
    }

    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
