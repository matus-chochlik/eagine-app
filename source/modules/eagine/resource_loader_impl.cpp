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
import eagine.core.math;
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
// resource_loader_signals
//------------------------------------------------------------------------------
auto resource_loader_signals::gl_shader_load_info::gl_api() const noexcept
  -> const oglplus::gl_api& {
    return video.gl_api();
}
//------------------------------------------------------------------------------
auto resource_loader_signals::gl_program_load_info::gl_api() const noexcept
  -> const oglplus::gl_api& {
    return video.gl_api();
}
//------------------------------------------------------------------------------
auto resource_loader_signals::gl_texture_load_info::gl_api() const noexcept
  -> const oglplus::gl_api& {
    return video.gl_api();
}
//------------------------------------------------------------------------------
auto resource_loader_signals::gl_buffer_load_info::gl_api() const noexcept
  -> const oglplus::gl_api& {
    return video.gl_api();
}
//------------------------------------------------------------------------------
// pending_resource_info
//------------------------------------------------------------------------------
void pending_resource_info::mark_loaded() noexcept {
    if(std::holds_alternative<_pending_gl_program_state>(_state)) {
        auto& pgps = std::get<_pending_gl_program_state>(_state);
        pgps.loaded = true;
        _finish_gl_program(pgps);
    } else if(std::holds_alternative<_pending_gl_texture_state>(_state)) {
        auto& pgts = std::get<_pending_gl_texture_state>(_state);
        pgts.loaded = true;
        _finish_gl_texture(pgts);
    } else if(std::holds_alternative<_pending_gl_buffer_state>(_state)) {
        auto& pgbs = std::get<_pending_gl_buffer_state>(_state);
        pgbs.loaded = true;
        _finish_gl_buffer(pgbs);
    } else if(_kind == resource_kind::camera_parameters) {
        _parent.resource_loaded(_request_id, _kind, _locator);
    } else if(_kind == resource_kind::input_setup) {
        _parent.resource_loaded(_request_id, _kind, _locator);
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
    } else if(std::holds_alternative<_pending_gl_buffer_state>(_state)) {
        auto& pgts = std::get<_pending_gl_buffer_state>(_state);
        pgts.video.get().gl_api().object_label(pgts.buf, label);
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
void pending_resource_info::add_gl_program_context(
  video_context& video) noexcept {
    oglplus::owned_program_name prog;
    video.gl_api().create_program() >> prog;
    _state = _pending_gl_program_state{.video = video, .prog = std::move(prog)};
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
  const resource_gl_texture_image_params& params,
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
  video_context& video,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept {
    _state = _pending_gl_texture_update_state{
      .video = video, .tex_target = target, .tex_unit = unit, .tex = tex};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_texture_context(
  video_context& video,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept {
    oglplus::owned_texture_name tex;
    video.gl_api().gen_textures() >> tex;
    _state = _pending_gl_texture_state{
      .video = video,
      .tex_target = target,
      .tex_unit = unit,
      .tex = std::move(tex)};
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_gl_buffer_data(
  const oglplus::buffer_target target,
  const resource_gl_buffer_data_params& params,
  const memory::const_block data) noexcept {
    if(const auto cont{continuation()}) {
        extract(cont)._handle_gl_buffer_data(*this, target, params, data);
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_buffer_context(
  video_context& video,
  oglplus::buffer_target target) noexcept {
    oglplus::owned_buffer_name buf;
    video.gl_api().gen_buffers() >> buf;
    _state = _pending_gl_buffer_state{
      .video = video, .buf_target = target, .buf = std::move(buf)};
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
            _parent.resource_loaded(_request_id, _kind, _locator);
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
        _parent.resource_loaded(_request_id, _kind, _locator);
    } else if(is(resource_kind::value_tree_traversal)) {
        if(std::holds_alternative<_pending_valtree_traversal_state>(_state)) {
            auto& pvts = std::get<_pending_valtree_traversal_state>(_state);
            bool should_continue{true};
            for(auto chunk : data) {
                if(not pvts.input.consume_data(chunk)) {
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
void pending_resource_info::handle_float_vector(
  const pending_resource_info&,
  std::vector<float>& values) noexcept {
    _parent.log_info("loaded float values")
      .arg("requestId", _request_id)
      .arg("size", values.size())
      .arg("locator", _locator.str());

    if(is(resource_kind::float_vector)) {
        _parent.float_vector_loaded(
          {.request_id = _request_id, .locator = _locator, .values = values});
        _parent.resource_loaded(_request_id, _kind, _locator);
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_vec3_vector(
  const pending_resource_info& source,
  std::vector<math::vector<float, 3, true>>& values) noexcept {
    _parent.log_info("loaded vec3 values")
      .arg("requestId", _request_id)
      .arg("size", values.size())
      .arg("locator", _locator.str());

    if(const auto cont{continuation()}) {
        extract(cont)._handle_vec3_vector(*this, values);
    }

    if(is(resource_kind::vec3_vector)) {
        _parent.vec3_vector_loaded(
          {.request_id = _request_id, .locator = _locator, .values = values});
        _parent.resource_loaded(_request_id, _kind, _locator);
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_vec3_vector(
  const pending_resource_info& source,
  const std::vector<math::vector<float, 3, true>>& values) noexcept {
    if(is(resource_kind::smooth_vec3_curve)) {
        /// TODO: other kinds of curves
        math::cubic_bezier_loop<math::vector<float, 3, true>, float> curve{
          view(values)};
        _parent.smooth_vec3_curve_loaded(
          {.request_id = _request_id, .locator = _locator, .curve = curve});
        _parent.resource_loaded(_request_id, _kind, _locator);
    }
    mark_finished();
}
//------------------------------------------------------------------------------
auto pending_resource_info::_apply_shape_modifiers(
  std::shared_ptr<shapes::generator> gen) noexcept
  -> std::shared_ptr<shapes::generator> {
    if(_locator.query().arg_has_value("to_patches", true)) {
        _parent.log_info("applying 'to_patches' shape modifier")
          .arg("requestId", _request_id)
          .arg("locator", _locator.str());
        gen = shapes::to_patches(std::move(gen));
    }
    return gen;
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_shape_generator(
  const pending_resource_info&,
  const std::shared_ptr<shapes::generator>& gen) noexcept {
    _parent.log_info("loaded shape geometry generator")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::gl_shape)) {
        if(std::holds_alternative<_pending_gl_shape_state>(_state)) {
            auto& pgss = std::get<_pending_gl_shape_state>(_state);
            const oglplus::shape_generator shape{
              pgss.video.get().gl_api(), _apply_shape_modifiers(gen)};
            if(const auto cont{continuation()}) {
                extract(cont)._handle_gl_shape(*this, shape);
            }
            _parent.gl_shape_loaded(
              {.request_id = _request_id, .locator = _locator, .shape = shape});
            _parent.resource_loaded(_request_id, _kind, _locator);
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
            if(not pggbs.bindings) {
                pggbs.bindings = oglplus::vertex_attrib_bindings{shape};
            }
            gl_geometry_and_bindings geom{
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
            _parent.resource_loaded(_request_id, _kind, _locator);
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
        gl_strs.reserve(data.std_size());
        gl_ints.reserve(data.std_size());

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
        _parent.resource_loaded(_request_id, _kind, _locator);
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
               .video = pgss.video,
               .type = pgss.shdr_type,
               .name = shdr,
               .ref = shdr});
            _parent.resource_loaded(_request_id, _kind, _locator);

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
    if(pgps.loaded and pgps.pending_requests.empty()) {
        _parent.log_info("loaded and linked GL program object")
          .arg("requestId", _request_id)
          .arg("bindgCount", pgps.input_bindings.count())
          .arg("locator", _locator.str());

        const auto& gl = pgps.video.get().gl_api().operations();
        gl.link_program(pgps.prog);
        gl.use_program(pgps.prog);
        _parent.gl_program_loaded(
          {.request_id = _request_id,
           .locator = _locator,
           .video = pgps.video,
           .name = pgps.prog,
           .ref = pgps.prog,
           .input_bindings = pgps.input_bindings});
        _parent.resource_loaded(_request_id, _kind, _locator);

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
                    if(not _finish_gl_program(pgps)) {
                        return;
                    }
                }
            }
        }
    }
    mark_finished();
}
//------------------------------------------------------------------------------
auto pending_resource_info::_finish_gl_texture(
  _pending_gl_texture_state& pgts) noexcept -> bool {
    if(pgts.loaded and pgts.pending_requests.empty()) {
        const auto& gl = pgts.video.get().gl_api().operations();
        gl.active_texture(pgts.tex_unit);

        assert(pgts.pparams);
        const auto& params{*pgts.pparams};
        for(const auto level : integer_range(pgts.levels)) {
            if(not pgts.level_images_done[std_size(level)]) {
                _clear_gl_texture_image(pgts, params, level, {});
            }
        }

        _parent.log_info("loaded and set-up GL texture object")
          .arg("requestId", _request_id)
          .arg("levels", pgts.levels)
          .arg("images", pgts.level_images_done.count())
          .arg("locator", _locator.str());

        // TODO: call this earlier (before all images are loaded)?
        _parent.gl_texture_loaded(
          {.request_id = _request_id,
           .locator = _locator,
           .video = pgts.video,
           .target = pgts.tex_target,
           .name = pgts.tex,
           .ref = pgts.tex});
        _parent.gl_texture_images_loaded(
          {.request_id = _request_id,
           .locator = _locator,
           .video = pgts.video,
           .name = pgts.tex});
        _parent.resource_loaded(_request_id, _kind, _locator);

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
auto resource_loader::_is_json_resource(const url& locator) const noexcept
  -> bool {
    return locator.has_path_suffix(".json") or locator.has_scheme("json");
}
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
auto resource_loader::request_float_vector(url locator) noexcept
  -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::float_vector)};

    if(const auto src_request{request_value_tree_traversal(
         locator, make_valtree_float_vector_builder(new_request))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::float_vector);
}
//------------------------------------------------------------------------------
auto resource_loader::request_vec3_vector(url locator) noexcept
  -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::vec3_vector)};

    if(const auto src_request{request_value_tree_traversal(
         locator, make_valtree_vec3_vector_builder(new_request))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::vec3_vector);
}
//------------------------------------------------------------------------------
auto resource_loader::request_smooth_vec3_curve(url locator) noexcept
  -> resource_request_result {
    if(const auto src_request{request_vec3_vector(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::smooth_vec3_curve)};
        src_request.set_continuation(new_request);

        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::smooth_vec3_curve);
}
//------------------------------------------------------------------------------
auto resource_loader::request_value_tree(url locator) noexcept
  -> resource_request_result {
    if(_is_json_resource(locator)) {
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

    return _cancelled_resource(locator, resource_kind::value_tree);
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
  std::shared_ptr<valtree::object_builder> builder) noexcept
  -> resource_request_result {
    const auto max_token_size{builder->max_token_size()};
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
    if(_is_json_resource(locator)) {
        return request_json_traversal(
          std::move(locator), std::move(visitor), max_token_size);
    }
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_value_tree_traversal(
  url locator,
  std::shared_ptr<valtree::object_builder> builder) noexcept
  -> resource_request_result {
    const auto max_token_size{builder->max_token_size()};
    return request_value_tree_traversal(
      std::move(locator),
      valtree::make_building_value_tree_visitor(std::move(builder)),
      max_token_size);
}
//------------------------------------------------------------------------------
auto resource_loader::request_camera_parameters(
  url locator,
  orbiting_camera& camera) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::camera_parameters)};

    if(const auto src_request{request_value_tree_traversal(
         locator,
         make_valtree_camera_parameters_builder(new_request, camera))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::camera_parameters);
}
//------------------------------------------------------------------------------
auto resource_loader::request_input_setup(
  url locator,
  execution_context& ctx) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::input_setup)};

    if(const auto src_request{request_value_tree_traversal(
         locator, make_valtree_input_setup_builder(new_request, ctx))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::input_setup);
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
auto resource_loader::request_gl_shape(
  url locator,
  video_context& video) noexcept -> resource_request_result {
    if(const auto src_request{request_shape_generator(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::gl_shape)};
        new_request.info().add_gl_shape_context(video);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_shape);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_geometry_and_bindings(
  url locator,
  video_context& video,
  oglplus::vertex_attrib_bindings bindings,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    if(const auto src_request{request_gl_shape(locator, video)}) {
        auto new_request{_new_resource(
          std::move(locator), resource_kind::gl_geometry_and_bindings)};
        new_request.info().add_gl_geometry_and_bindings_context(
          video, std::move(bindings), draw_var_idx);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(
      locator, resource_kind::gl_geometry_and_bindings);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_geometry_and_bindings(
  url locator,
  video_context& video,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    return request_gl_geometry_and_bindings(
      std::move(locator), video, {}, draw_var_idx);
}
//------------------------------------------------------------------------------
auto resource_loader::request_glsl_source(url locator) noexcept
  -> resource_request_result {
    if(locator.has_path_suffix(".glsl") or locator.has_scheme("glsl")) {
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
  video_context& video,
  oglplus::shader_type shdr_type) noexcept -> resource_request_result {
    if(const auto src_request{request_glsl_source(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::gl_shader)};
        new_request.info().add_gl_shader_context(video, shdr_type);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_shader);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(
  url locator,
  video_context& video) noexcept -> resource_request_result {
    if(const auto type_arg{locator.argument("shader_type")}) {
        const auto& GL = video.gl_api().constants();
        const auto delegate = [&GL, &locator, &video, this](auto shdr_type) {
            return request_gl_shader(std::move(locator), video, shdr_type);
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
auto resource_loader::request_gl_program(
  url locator,
  video_context& video) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_program)};
    new_request.info().add_gl_program_context(video);

    if(const auto src_request{request_value_tree_traversal(
         locator, make_valtree_gl_program_builder(new_request, video))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_program);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture_image(
  url locator,
  oglplus::texture_target target,
  const resource_gl_texture_image_params& params) noexcept
  -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_texture_image)};

    if(const auto src_request{request_json_traversal(
         locator,
         make_valtree_gl_texture_image_loader(new_request, target, params))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_texture_image);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture_update(
  url locator,
  video_context& video,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept -> resource_request_result {
    if(const auto src_request{request_gl_texture_image(locator, target)}) {
        auto new_request{
          _new_resource(locator, resource_kind::gl_texture_update)};
        new_request.info().add_gl_texture_update_context(
          video, target, unit, tex);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_texture_update);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(
  url locator,
  video_context& video,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_texture)};
    new_request.info().add_gl_texture_context(video, target, unit);

    if(const auto src_request{request_json_traversal(
         locator,
         make_valtree_gl_texture_builder(new_request, video, target, unit))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_texture);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(
  url locator,
  video_context& video,
  oglplus::buffer_target target) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_buffer)};
    new_request.info().add_gl_buffer_context(video, target);

    if(const auto src_request{request_value_tree_traversal(
         locator,
         make_valtree_gl_buffer_builder(new_request, video, target))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_buffer);
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
// pending_resource_requests
//------------------------------------------------------------------------------
pending_resource_requests::pending_resource_requests(
  execution_context& ctx) noexcept
  : pending_resource_requests{ctx.loader()} {}
//------------------------------------------------------------------------------
} // namespace eagine::app
