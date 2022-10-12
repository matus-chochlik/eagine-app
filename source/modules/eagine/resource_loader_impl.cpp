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
      pending_resource_info& info,
      video_context& vc) noexcept
      : _parent{info}
      , _video{vc} {}

    void request_shader(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        auto& loader = _parent.loader();
        auto& GL = _video.gl_api().constants();
        const auto delegate = [&, this](oglplus::shader_type shdr_type) {
            if(auto src_request{loader.request_gl_shader(
                 url{to_string(extract(data))}, shdr_type, _video)}) {
                src_request.set_continuation(_parent);
                if(!_parent.add_gl_program_shader_request(
                     src_request.request_id())) [[unlikely]] {
                    src_request.info().mark_finished();
                    _parent.mark_finished();
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

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if(path.size() == 1) {
            if(path.front() == "label") {
                if(has_value(data)) {
                    _parent.add_label(extract(data));
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
                _parent.add_gl_program_input_binding(
                  std::move(_input_name),
                  {_attrib_kind, _attrib_variant_index});
            }
        }
    }

    void finish() noexcept final {
        _parent.mark_loaded();
    }

    void failed() noexcept final {
        _parent.mark_finished();
    }

private:
    pending_resource_info& _parent;
    video_context& _video;
    std::string _input_name;
    shapes::vertex_attrib_kind _attrib_kind;
    span_size_t _attrib_variant_index{0};
};
//------------------------------------------------------------------------------
// valtree_gl_texture_image_loader
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
    oglplus::gl_types::int_type x_offs{0};
    oglplus::gl_types::int_type y_offs{0};
    oglplus::gl_types::int_type z_offs{0};
    oglplus::gl_types::int_type level{0};
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
      pending_resource_info& info,
      video_context& vc) noexcept
      : _parent{info}
      , _video{vc}
      , _temp{_parent.loader().buffers().get(512 * 512)} {}

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
            } else if(path.front() == "height") {
                _success &= assign_if_fits(data, _params.height);
            } else if(path.front() == "depth") {
                _success &= assign_if_fits(data, _params.depth);
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
            }
        }
    }

    void do_add(const basic_string_path&, const auto&) noexcept {}

    void unparsed_data(span<const memory::const_block> data) noexcept final {
        for(const auto& blk : data) {
            memory::append_to(blk, _temp);
            // TODO: progressive image specification once we have enough data
            // for width * some constant so that the temp buffer doesn't get
            // too big
        }
    }

    void finish() noexcept final {
        if(_success) {
            _parent.add_gl_texture_image_data(_params, _temp);
            _parent.mark_loaded();
        } else {
            _parent.mark_finished();
        }
        _parent.loader().buffers().eat(std::move(_temp));
    }

    void failed() noexcept final {
        _parent.mark_finished();
        _parent.loader().buffers().eat(std::move(_temp));
    }

private:
    pending_resource_info& _parent;
    video_context& _video;
    resource_texture_image_params _params{};
    memory::buffer _temp;
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
      pending_resource_info& info,
      video_context& vc) noexcept
      : _parent{info}
      , _video{vc} {}

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
                    _parent.add_label(extract(data));
                }
            } else if(path.front() == "data_type") {
                _success &=
                  texture_data_type_from_string(data, _params.data_type);
            } else if(path.front() == "format") {
                _success &= texture_format_from_string(data, _params.format);
            } else if(path.front() == "iformat") {
                _success &= texture_iformat_from_string(data, _params.iformat);
            }
        }
    }

    void do_add(const basic_string_path&, const auto&) noexcept {}

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.empty()) {
            if(_success) {
                _parent.add_gl_texture_params(_params);
            }
        }
    }

    void finish() noexcept final {
        if(_success) {
            _parent.mark_loaded();
        } else {
            _parent.mark_finished();
        }
    }

    void failed() noexcept final {
        _parent.mark_finished();
    }

private:
    pending_resource_info& _parent;
    video_context& _video;
    resource_texture_params _params{};
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
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::mark_finished() noexcept {
    _parent.forget_resource(_request_id);
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
auto pending_resource_info::add_gl_program_shader(
  identifier_t request_id,
  oglplus::owned_shader_name& shdr) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_program_state>(_state)) {
        auto& pgps = std::get<_pending_gl_program_state>(_state);
        if(const auto pos{pgps.pending_requests.find(request_id)};
           pos != pgps.pending_requests.end()) {
            pgps.pending_requests.erase(pos);
            if(pgps.pending_requests.empty()) {
            }
            return true;
        }
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
void pending_resource_info::add_gl_texture_image_context(
  video_context& vc,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept {
    _state = _pending_gl_texture_image_state{
      .video = vc, .tex_target = target, .tex_unit = unit, .tex = tex};
}
//------------------------------------------------------------------------------
auto pending_resource_info::add_gl_texture_image_data(
  const resource_texture_image_params&,
  const memory::const_block) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_texture_image_state>(_state)) {
        auto& pgtis = std::get<_pending_gl_texture_image_state>(_state);
        (void)pgtis;
        // TODO
        return true;
    }
    return false;
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
auto pending_resource_info::add_gl_texture_params(
  const resource_texture_params& params) noexcept -> bool {
    if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        auto& pgts = std::get<_pending_gl_texture_state>(_state);
        auto& glapi = pgts.video.get().gl_api();
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
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
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
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
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
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
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
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
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
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
                return bool(glapi.tex_storage1d(
                  pgts.tex_target,
                  params.levels,
                  oglplus::pixel_internal_format{params.iformat},
                  params.width));
            } else if(glapi.tex_image1d) {
                bool result{true};
                auto width = params.width;
                glapi.operations().active_texture(pgts.tex_unit);
                glapi.bind_texture(pgts.tex_target, pgts.tex);
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
            if(_continuation) {
                _continuation->_handle_shape_generator(*this, shape_gen);
            }
            _parent.shape_generator_loaded(_request_id, shape_gen, _locator);
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
  const span_size_t,
  const memory::span<const memory::const_block> data) noexcept {
    _parent.log_debug("loaded JSON text")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::value_tree)) {
        auto tree{valtree::from_json_data(data, _parent.main_context().log())};
        if(_continuation) {
            extract(_continuation)._handle_value_tree(*this, tree);
        }
        _parent.value_tree_loaded(_request_id, tree, _locator);
    } else if(is(resource_kind::value_tree_traversal)) {
        if(std::holds_alternative<_pending_valtree_traversal_state>(_state)) {
            auto& pvts = std::get<_pending_valtree_traversal_state>(_state);
            bool should_continue{true};
            for(auto chunk : data) {
                if(!pvts.input.consume_data(chunk)) {
                    should_continue = false;
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
            if(_continuation) {
                extract(_continuation)._handle_gl_shape(*this, shape);
            }
            _parent.gl_shape_loaded(_request_id, shape, _locator);
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
              _request_id, {geom}, _locator);
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
        if(_continuation) {
            extract(_continuation)._handle_glsl_source(*this, glsl_src);
        }
        _parent.glsl_source_loaded(_request_id, glsl_src, _locator);
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
            if(_continuation) {
                extract(_continuation)._handle_gl_shader(*this, shdr);
            }
            _parent.gl_shader_loaded(
              _request_id, pgss.shdr_type, shdr, {shdr}, _locator);

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
          _request_id, pgps.prog, {pgps.prog}, pgps.input_bindings, _locator);

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
      .arg("locator", _locator.str());

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
// resource_loader
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_data_appended(
  identifier_t request_id,
  const span_size_t offset,
  const memory::span<const memory::const_block> data,
  const msgbus::blob_info& binfo) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        const auto& rinfo = std::get<1>(*pos);
        if(rinfo._continuation) {
            extract(rinfo._continuation)
              .handle_source_data(binfo, rinfo, offset, data);
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_finished(identifier_t request_id) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        const auto& rinfo = std::get<1>(*pos);
        if(rinfo._continuation) {
            extract(rinfo._continuation).handle_source_finished(rinfo);
        }
        _pending.erase(pos);
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(
  identifier_t request_id) noexcept {
    if(const auto pos{_pending.find(request_id)}; pos != _pending.end()) {
        const auto& [request_id, rinfo] = *pos;
        if(rinfo._continuation) {
            extract(rinfo._continuation).handle_source_cancelled(rinfo);
        }
        resource_cancelled(request_id, rinfo._kind, rinfo._locator);
        _pending.erase(pos);
    }
}
//------------------------------------------------------------------------------
auto resource_loader::_cancelled_resource(
  const identifier_t request_id,
  url& locator,
  const resource_kind kind) noexcept -> resource_request_result {
    auto result{_cancelled.emplace(
      request_id,
      pending_resource_info{*this, request_id, std::move(locator), kind})};
    return {*std::get<0>(result), true};
}
//------------------------------------------------------------------------------
auto resource_loader::_new_resource(
  const identifier_t request_id,
  url locator,
  resource_kind kind) noexcept -> resource_request_result {
    auto result{_pending.emplace(
      request_id,
      pending_resource_info{*this, request_id, std::move(locator), kind})};
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
               4096,
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
           std::move(locator),
           msgbus::message_priority::normal,
           std::chrono::hours{1}),
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
               4096,
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
         std::make_unique<valtree_gl_program_builder>(new_request.info(), vc),
         128)}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_program);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture_image(
  url locator,
  video_context& vc,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_texture_image)};
    new_request.info().add_gl_texture_image_context(vc, target, unit, tex);

    if(const auto src_request{request_json_traversal(
         locator,
         std::make_shared<valtree_gl_texture_image_loader>(
           new_request.info(), vc),
         128)}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_texture_image);
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
         std::make_shared<valtree_gl_texture_builder>(new_request.info(), vc),
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
void resource_loader::forget_resource(identifier_t request_id) noexcept {
    _deleted.insert(request_id);
}
//------------------------------------------------------------------------------
auto resource_loader::update() noexcept -> work_done {
    some_true something_done{base::update()};

    for(auto& [request_id, rinfo] : _cancelled) {
        resource_cancelled(request_id, rinfo._kind, rinfo._locator);
        something_done();
    }
    _cancelled.clear();

    for(auto pos{_pending.begin()}; pos != _pending.end();) {
        auto& [request_id, info] = *pos;
        if(info.is_done() || _deleted.contains(request_id)) {
            info.cleanup();
            pos = _pending.erase(pos);
            something_done();
        } else {
            something_done(info.update());
            ++pos;
        }
    }
    _deleted.clear();

    return something_done;
}
//------------------------------------------------------------------------------
} // namespace eagine::app
