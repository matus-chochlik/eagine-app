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
import eagine.core.memory;
import eagine.core.string;
import eagine.core.math;
import eagine.core.identifier;
import eagine.core.container;
import eagine.core.reflection;
import eagine.core.value_tree;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.shapes;
import eagine.oglplus;
import eagine.msgbus;

namespace eagine::app {
//------------------------------------------------------------------------------
// pending_resource_info
//------------------------------------------------------------------------------
void pending_resource_info::mark_loaded() noexcept {
    if(auto pgps{get_if<_pending_gl_program_state>(_state)}) {
        pgps->loaded = true;
        _finish_gl_program(*pgps);
    } else if(auto pgts{get_if<_pending_gl_texture_state>(_state)}) {
        pgts->loaded = true;
        _finish_gl_texture(*pgts);
    } else if(auto pgbs{get_if<_pending_gl_buffer_state>(_state)}) {
        pgbs->loaded = true;
        _finish_gl_buffer(*pgbs);
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
    _label = to_string(label);
}
//------------------------------------------------------------------------------
void pending_resource_info::apply_label() noexcept {
    if(not _label.empty()) {
        if(auto pgps{get_if<_pending_gl_program_state>(_state)}) {
            pgps->gl_context.gl_api().object_label(pgps->prog, _label);
        } else if(auto pgts{get_if<_pending_gl_texture_state>(_state)}) {
            pgts->gl_context.gl_api().object_label(pgts->tex, _label);
        } else if(auto pgbs{get_if<_pending_gl_buffer_state>(_state)}) {
            pgbs->gl_context.gl_api().object_label(pgbs->buf, _label);
        }
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::add_valtree_stream_input(
  valtree::value_tree_stream_input input) noexcept {
    _state = _pending_valtree_traversal_state{.input = std::move(input)};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_shape_generator(
  shared_holder<shapes::generator> gen) noexcept {
    _state = _pending_shape_generator_state{.generator = std::move(gen)};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_shape_context(
  const oglplus::shared_gl_api_context& gl_context) noexcept {
    _state = _pending_gl_shape_state{.gl_context = gl_context};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_geometry_and_bindings_context(
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::vertex_attrib_bindings bindings,
  span_size_t draw_var_idx) noexcept {
    _state = _pending_gl_geometry_and_bindings_state{
      .gl_context = gl_context,
      .bindings = std::move(bindings),
      .draw_var_idx = draw_var_idx};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_shader_context(
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::shader_type shdr_type) noexcept {
    _state = _pending_gl_shader_state{
      .gl_context = gl_context, .shdr_type = shdr_type};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_program_context(
  const oglplus::shared_gl_api_context& gl_context) noexcept {
    oglplus::owned_program_name prog;
    gl_context.gl_api().create_program() >> prog;
    _state = _pending_gl_program_state{
      .gl_context = gl_context, .prog = std::move(prog)};
}
//------------------------------------------------------------------------------
auto pending_resource_info::add_gl_program_shader_request(
  identifier_t request_id) noexcept -> bool {
    return get_if<_pending_gl_program_state>(_state)
      .and_then([request_id](auto& pgps) -> tribool {
          pgps.pending_requests.insert(request_id);
          return true;
      })
      .or_false();
}
//------------------------------------------------------------------------------
auto pending_resource_info::add_gl_program_input_binding(
  std::string name,
  shapes::vertex_attrib_variant vav) noexcept -> bool {
    return get_if<_pending_gl_program_state>(_state)
      .and_then([&](auto& pgps) -> tribool {
          pgps.input_bindings.add(std::move(name), vav);
          return true;
      })
      .or_false();
}
//------------------------------------------------------------------------------
void pending_resource_info::handle_gl_texture_image(
  const oglplus::texture_target target,
  resource_gl_texture_image_params& params,
  const memory::const_block data) noexcept {
    if(const auto cont{continuation()}) {
        cont->_handle_gl_texture_image(*this, target, params, data);
    } else {
        _handle_gl_texture_image(*this, target, params, data);
    }
}
//------------------------------------------------------------------------------
auto pending_resource_info::add_gl_texture_image_request(
  identifier_t request_id) noexcept -> bool {
    return get_if<_pending_gl_texture_state>(_state)
      .and_then([&](auto& pgts) -> tribool {
          pgts.pending_requests.insert(request_id);
          return true;
      })
      .or_false();
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_texture_update_context(
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept {
    _state = _pending_gl_texture_update_state{
      .gl_context = gl_context,
      .tex_target = target,
      .tex_unit = unit,
      .tex = tex};
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_texture_context(
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept {
    oglplus::owned_texture_name tex;
    gl_context.gl_api().gen_textures() >> tex;
    _state = _pending_gl_texture_state{
      .gl_context = gl_context,
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
        cont->_handle_gl_buffer_data(*this, target, params, data);
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::add_gl_buffer_context(
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::buffer_target target) noexcept {
    oglplus::owned_buffer_name buf;
    gl_context.gl_api().gen_buffers() >> buf;
    _state = _pending_gl_buffer_state{
      .gl_context = gl_context, .buf_target = target, .buf = std::move(buf)};
}
//------------------------------------------------------------------------------
auto pending_resource_info::update() noexcept -> work_done {
    some_true something_done;
    get_if<_pending_shape_generator_state>(_state).and_then(
      [&, this](auto& psgs) {
          if(const auto shape_gen{psgs.generator}) {
              if(const auto cont{continuation()}) {
                  cont->_handle_shape_generator(*this, shape_gen);
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
      });
    return something_done;
}
//------------------------------------------------------------------------------
void pending_resource_info::cleanup() noexcept {
    if(const auto pgps{get_if<_pending_gl_program_state>(_state)}) {
        if(pgps->prog) {
            pgps->gl_context.gl_api().delete_program(std::move(pgps->prog));
        }
        _state = std::monostate{};
    } else if(const auto pgts{get_if<_pending_gl_texture_state>(_state)}) {
        if(pgts->tex) {
            pgts->gl_context.gl_api().delete_textures(std::move(pgts->tex));
        }
        _state = std::monostate{};
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_plain_text(
  const msgbus::blob_info&,
  const pending_resource_info&,
  const span_size_t offset,
  const memory::span<const memory::const_block> data) noexcept {
    _parent.log_debug("loaded plain-text data")
      .arg("requestId", _request_id)
      .arg("offset", offset)
      .arg("locator", _locator.str());

    std::string text;
    span_size_t total_size{0};
    for(const auto chunk : data) {
        total_size = safe_add(total_size, chunk.size());
    }
    text.reserve(std_size(total_size));
    for(const auto chunk : data) {
        append_to(as_chars(chunk), text);
    }

    if(is(resource_kind::plain_text)) {
        _parent.plain_text_loaded(
          {.request_id = _request_id,
           .locator = _locator,
           .text = std::move(text)});
    }
    _parent.resource_loaded(_request_id, _kind, _locator);
    mark_finished();
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
            cont->_handle_value_tree(*this, tree);
        }
        _parent.value_tree_loaded(
          {.request_id = _request_id, .locator = _locator, .tree = tree});
        _parent.resource_loaded(_request_id, _kind, _locator);
    } else if(is(resource_kind::value_tree_traversal)) {
        if(const auto pvts{get_if<_pending_valtree_traversal_state>(_state)}) {
            bool should_continue{true};
            for(auto chunk : data) {
                if(not pvts->input.consume_data(chunk)) {
                    pvts->input.finish();
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
        // TODO
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
void pending_resource_info::_handle_string_list(
  const msgbus::blob_info&,
  const pending_resource_info&,
  const span_size_t offset,
  const memory::span<const memory::const_block> data) noexcept {
    _parent.log_debug("loaded string list data")
      .arg("requestId", _request_id)
      .arg("offset", offset)
      .arg("locator", _locator.str());

    std::vector<std::string> strings;
    resource_loader_signals::string_list_load_info load_info{
      .request_id = _request_id,
      .locator = _locator,
      .strings = strings,
      .values = strings};

    std::string line;
    const string_view sep{"\n"};
    for(const auto chunk : data) {
        auto text{as_chars(chunk)};
        while(not text.empty()) {
            if(const auto pos{memory::find_position(text, sep)}) {
                append_to(head(text, *pos), line);
                text = skip(text, *pos + sep.size());
                strings.emplace_back(std::move(line));
                if(is(resource_kind::string_list)) {
                    _parent.string_line_loaded(load_info);
                }
            } else {
                append_to(text, line);
                text = {};
            }
        }
    }
    if(not line.empty()) {
        strings.emplace_back(std::move(line));
        if(is(resource_kind::string_list)) {
            _parent.string_line_loaded(load_info);
        }
    }

    if(const auto cont{continuation()}) {
        if(cont->is(resource_kind::url_list)) {
            std::vector<url> urls;
            urls.reserve(strings.size());
            for(const auto& str : strings) {
                if(not str.empty()) {
                    if(url locator{str}) {
                        urls.emplace_back(std::move(locator));
                    }
                }
            }
            cont->_handle_url_list(*this, urls);
        }
    }

    if(is(resource_kind::string_list)) {
        _parent.string_list_loaded(load_info);
        _parent.resource_loaded(_request_id, _kind, _locator);
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_url_list(
  const pending_resource_info& source,
  const std::vector<url>& urls) noexcept {
    _parent.log_info("loaded URL list")
      .arg("requestId", _request_id)
      .arg("size", urls.size())
      .arg("locator", _locator.str());

    if(is(resource_kind::url_list)) {
        _parent.url_list_loaded(
          {.request_id = _request_id, .locator = _locator, .values = urls});
        _parent.resource_loaded(_request_id, _kind, _locator);
    }
    mark_finished();
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
        cont->_handle_vec3_vector(*this, values);
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
void pending_resource_info::handle_mat4_vector(
  const pending_resource_info& source,
  std::vector<math::matrix<float, 4, 4, true, true>>& values) noexcept {
    _parent.log_info("loaded mat4 values")
      .arg("requestId", _request_id)
      .arg("size", values.size())
      .arg("locator", _locator.str());

    if(const auto cont{continuation()}) {
        cont->_handle_mat4_vector(*this, values);
    }

    if(is(resource_kind::mat4_vector)) {
        _parent.mat4_vector_loaded(
          {.request_id = _request_id, .locator = _locator, .values = values});
        _parent.resource_loaded(_request_id, _kind, _locator);
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_mat4_vector(
  const pending_resource_info& source,
  const std::vector<math::matrix<float, 4, 4, true, true>>& values) noexcept {
    mark_finished();
}
//------------------------------------------------------------------------------
auto pending_resource_info::_apply_shape_modifiers(
  shared_holder<shapes::generator> gen) noexcept
  -> shared_holder<shapes::generator> {
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
  const shared_holder<shapes::generator>& gen) noexcept {
    _parent.log_info("loaded shape geometry generator")
      .arg("requestId", _request_id)
      .arg("locator", _locator.str());

    if(is(resource_kind::gl_shape)) {
        if(const auto pgss{get_if<_pending_gl_shape_state>(_state)}) {
            const oglplus::shape_generator shape{
              pgss->gl_context.gl_api(), _apply_shape_modifiers(gen)};
            if(const auto cont{continuation()}) {
                cont->_handle_gl_shape(*this, shape);
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
        if(const auto pggbs{
             get_if<_pending_gl_geometry_and_bindings_state>(_state)}) {
            auto temp{_parent.buffers().get()};
            const auto cleanup_temp{_parent.buffers().eat_later(temp)};
            if(not pggbs->bindings) {
                pggbs->bindings = oglplus::vertex_attrib_bindings{
                  oglplus::make_default_vertex_attrib_bindings(shape)};
            }
            gl_geometry_and_bindings geom{
              pggbs->gl_context.gl_api(),
              shape,
              pggbs->bindings,
              shape.draw_variant(pggbs->draw_var_idx),
              temp};
            _parent.gl_geometry_and_bindings_loaded(
              {.request_id = _request_id,
               .locator = _locator,
               .shape = shape,
               .ref = geom});
            _parent.resource_loaded(_request_id, _kind, _locator);
            if(geom) {
                geom.clean_up(pggbs->gl_context.gl_api());
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
            cont->_handle_glsl_source(*this, glsl_src);
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
        if(const auto pgss{get_if<_pending_gl_shader_state>(_state)}) {
            const auto& glapi{pgss->gl_context.gl_api()};
            const auto& [gl, GL] = glapi;

            oglplus::owned_shader_name shdr;
            gl.create_shader(pgss->shdr_type) >> shdr;
            gl.shader_source(shdr, glsl_src);
            if(gl.compile_shader(shdr)) {
                _parent
                  .log_info("loaded and compiled GL shader object (${locator})")
                  .arg("requestId", _request_id)
                  .arg("locator", "string", _locator.str());

                if(const auto cont{continuation()}) {
                    cont->_handle_gl_shader(*this, shdr);
                }
                _parent.gl_shader_loaded(
                  {.request_id = _request_id,
                   .locator = _locator,
                   .gl_context = pgss->gl_context,
                   .type = pgss->shdr_type,
                   .name = shdr,
                   .ref = shdr});
                _parent.resource_loaded(_request_id, _kind, _locator);
            } else {
                const std::string message{
                  glapi.shader_info_log(shdr).value_or("N/A")};
                _parent
                  .log_error("failed to load and link GL shader (${locator})")
                  .arg("requestId", _request_id)
                  .arg("message", "string", message)
                  .arg("locator", "string", _locator.str());
            }

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

        const auto& glapi{pgps.gl_context.gl_api()};
        const auto& gl = glapi.operations();

        if(gl.link_program(pgps.prog)) {
            _parent.log_info("loaded and linked GL program object (${locator})")
              .arg("requestId", _request_id)
              .arg("bindgCount", pgps.input_bindings.count())
              .arg("locator", "string", _locator.str());

            gl.use_program(pgps.prog);

            _parent.gl_program_loaded(
              {.request_id = _request_id,
               .locator = _locator,
               .gl_context = pgps.gl_context,
               .name = pgps.prog,
               .ref = pgps.prog,
               .input_bindings = pgps.input_bindings});
            _parent.resource_loaded(_request_id, _kind, _locator);
        } else {
            const std::string message{
              glapi.program_info_log(pgps.prog).value_or("N/A")};
            _parent.log_error("failed to load and link GL program (${locator})")
              .arg("requestId", _request_id)
              .arg("message", "string", message)
              .arg("locator", "string", _locator.str());
        }

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
    if(is(resource_kind::gl_program)) {
        if(const auto pgps{get_if<_pending_gl_program_state>(_state)}) {
            if(pgps->prog) [[likely]] {
                const auto& gl = pgps->gl_context.gl_api().operations();

                if(const auto found{eagine::find(
                     pgps->pending_requests, source.request_id())}) {
                    gl.attach_shader(pgps->prog, shdr);

                    pgps->pending_requests.erase(found.position());
                    if(not _finish_gl_program(*pgps)) {
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
        apply_label();

        const auto& gl = pgts.gl_context.gl_api().operations();
        gl.active_texture(pgts.tex_unit);

        if(pgts.pparams) {
            const auto& params{*pgts.pparams};
            for(const auto level : integer_range(pgts.levels)) {
                if(not pgts.level_images_done[std_size(level)]) {
                    _clear_gl_texture_image(pgts, params, level, {});
                }
            }

            if(pgts.generate_mipmap) {
                if(gl.generate_texture_mipmap) {
                    gl.generate_texture_mipmap(pgts.tex);
                } else {
                    gl.generate_mipmap(pgts.tex_target);
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
               .gl_context = pgts.gl_context,
               .target = pgts.tex_target,
               .name = pgts.tex,
               .ref = pgts.tex});
            _parent.gl_texture_images_loaded(
              {.request_id = _request_id,
               .locator = _locator,
               .gl_context = pgts.gl_context,
               .name = pgts.tex});
            _parent.resource_loaded(_request_id, _kind, _locator);

            if(pgts.tex) {
                gl.delete_textures(std::move(pgts.tex));
            }
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
        case resource_kind::plain_text:
            _handle_plain_text(binfo, rinfo, offset, data);
            break;
        case resource_kind::string_list:
            _handle_string_list(binfo, rinfo, offset, data);
            break;
        case resource_kind::json_text:
            _handle_json_text(binfo, rinfo, offset, data);
            break;
        case resource_kind::yaml_text:
            _handle_yaml_text(binfo, rinfo, offset, data);
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
// valtree_builder_common
//------------------------------------------------------------------------------
valtree_builder_common::valtree_builder_common(
  identifier id,
  shared_holder<pending_resource_info> info) noexcept
  : main_ctx_object{id, info->loader().as_parent()}
  , _parent{std::move(info)} {}
//------------------------------------------------------------------------------
// resource_request_result
//------------------------------------------------------------------------------
auto resource_request_result::info() const noexcept -> pending_resource_info& {
    assert(_info);
    return *_info;
}
//------------------------------------------------------------------------------
auto resource_request_result::set_continuation(
  const shared_holder<pending_resource_info>& cont) const noexcept
  -> const resource_request_result& {
    info().set_continuation(cont);
    return *this;
}
//------------------------------------------------------------------------------
auto resource_request_result::set_continuation(resource_request_result& cont)
  const noexcept -> const resource_request_result& {
    info().set_continuation(cont._info);
    return *this;
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
  const msgbus::blob_stream_chunk& chunk) noexcept {
    if(const auto found{find(_pending, chunk.request_id)}) {
        if(const auto& prinfo{*found}) {
            if(const auto continuation{prinfo->continuation()}) {
                continuation->handle_source_data(
                  chunk.info, *prinfo, chunk.offset, chunk.data);
            }
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_finished(identifier_t request_id) noexcept {
    if(const auto found{find(_pending, request_id)}) {
        if(const auto& prinfo{*found}) {
            auto& rinfo{*prinfo};
            if(const auto continuation{rinfo.continuation()}) {
                continuation->handle_source_finished(rinfo);
            }
            rinfo.mark_finished();
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(
  identifier_t request_id) noexcept {
    if(const auto found{find(_pending, request_id)}) {
        if(const auto& prinfo{*found}) {
            auto& rinfo{*prinfo};
            if(const auto continuation{rinfo.continuation()}) {
                continuation->handle_source_cancelled(rinfo);
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
auto resource_loader::request_plain_text(url locator) noexcept
  -> resource_request_result {
    if(
      locator.has_path_suffix(".txt") or locator.has_scheme("txt") or
      locator.has_scheme("text")) {
        if(const auto src_request{_new_resource(
             fetch_resource_chunks(
               locator,
               1024,
               msgbus::message_priority::normal,
               std::chrono::seconds{15}),
             resource_kind::plain_text)}) {
            auto new_request{
              _new_resource(std::move(locator), resource_kind::plain_text)};
            src_request.set_continuation(new_request);
            return new_request;
        }
    }
    return _cancelled_resource(locator, resource_kind::plain_text);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<std::string>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_plain_text(std::move(locator));
}
//------------------------------------------------------------------------------
auto resource_loader::request_string_list(url locator) noexcept
  -> resource_request_result {
    if(
      locator.has_scheme("txt") or locator.has_scheme("text") or
      locator.has_path_suffix(".txt") or locator.has_path_suffix(".text")) {
        if(const auto src_request{_new_resource(
             fetch_resource_chunks(
               locator,
               1024,
               msgbus::message_priority::normal,
               std::chrono::seconds{15}),
             resource_kind::string_list)}) {
            auto new_request{
              _new_resource(std::move(locator), resource_kind::string_list)};
            src_request.set_continuation(new_request);
            return new_request;
        }
    }
    return _cancelled_resource(locator, resource_kind::string_list);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<std::vector<std::string>>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_string_list(std::move(locator));
}
//------------------------------------------------------------------------------
auto resource_loader::request_url_list(url locator) noexcept
  -> resource_request_result {
    if(const auto src_request{request_string_list(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::url_list)};
        src_request.set_continuation(new_request);

        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::url_list);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<std::vector<url>>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_url_list(std::move(locator));
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
auto resource_loader::request(
  std::type_identity<std::vector<float>>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_float_vector(std::move(locator));
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
auto resource_loader::request(
  std::type_identity<std::vector<math::vector<float, 3, true>>>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_vec3_vector(std::move(locator));
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
auto resource_loader::request(
  std::type_identity<
    math::cubic_bezier_curves<math::vector<float, 3, true>, float>>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_smooth_vec3_curve(std::move(locator));
}
//------------------------------------------------------------------------------
auto resource_loader::request_mat4_vector(url locator) noexcept
  -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::mat4_vector)};

    if(const auto src_request{request_value_tree_traversal(
         locator, make_valtree_mat4_vector_builder(new_request))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::mat4_vector);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<std::vector<math::matrix<float, 4, 4, true, true>>>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_mat4_vector(std::move(locator));
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
auto resource_loader::request(
  std::type_identity<valtree::compound>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_value_tree(std::move(locator));
}
//------------------------------------------------------------------------------
auto resource_loader::request_json_traversal(
  url locator,
  shared_holder<valtree::value_tree_visitor> visitor,
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
  shared_holder<valtree::object_builder> builder) noexcept
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
  shared_holder<valtree::value_tree_visitor> visitor,
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
  shared_holder<valtree::object_builder> builder) noexcept
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
auto resource_loader::request(
  std::type_identity<shared_holder<shapes::generator>>,
  url locator,
  loaded_resource_context&) noexcept -> resource_request_result {
    return request_shape_generator(std::move(locator));
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shape(
  url locator,
  const oglplus::shared_gl_api_context& gl_context) noexcept
  -> resource_request_result {
    if(const auto src_request{request_shape_generator(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::gl_shape)};
        new_request.info().add_gl_shape_context(gl_context);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_shape);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<oglplus::shape_generator>,
  url locator,
  loaded_resource_context& ctx) noexcept -> resource_request_result {
    return request_gl_shape(std::move(locator), ctx.gl_context());
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_geometry_and_bindings(
  url locator,
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::vertex_attrib_bindings bindings,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    if(const auto src_request{request_gl_shape(locator, gl_context)}) {
        auto new_request{_new_resource(
          std::move(locator), resource_kind::gl_geometry_and_bindings)};
        new_request.info().add_gl_geometry_and_bindings_context(
          gl_context, std::move(bindings), draw_var_idx);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(
      locator, resource_kind::gl_geometry_and_bindings);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<gl_geometry_and_bindings>,
  url locator,
  loaded_resource_context& ctx,
  oglplus::vertex_attrib_bindings bindings,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    return request_gl_geometry_and_bindings(
      std::move(locator), ctx.gl_context(), bindings, draw_var_idx);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_geometry_and_bindings(
  url locator,
  const oglplus::shared_gl_api_context& gl_context,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    return request_gl_geometry_and_bindings(
      std::move(locator), gl_context, {}, draw_var_idx);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<gl_geometry_and_bindings>,
  url locator,
  loaded_resource_context& ctx,
  span_size_t draw_var_idx) noexcept -> resource_request_result {
    return request_gl_geometry_and_bindings(
      std::move(locator), ctx.gl_context(), draw_var_idx);
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
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::shader_type shdr_type) noexcept -> resource_request_result {
    if(const auto src_request{request_glsl_source(locator)}) {
        auto new_request{
          _new_resource(std::move(locator), resource_kind::gl_shader)};
        new_request.info().add_gl_shader_context(gl_context, shdr_type);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_shader);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<oglplus::owned_shader_name>,
  url locator,
  loaded_resource_context& ctx,
  oglplus::shader_type shdr_type) noexcept -> resource_request_result {
    return request_gl_shader(std::move(locator), ctx.gl_context(), shdr_type);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_shader(
  url locator,
  const oglplus::shared_gl_api_context& gl_context) noexcept
  -> resource_request_result {
    if(const auto type_arg{locator.argument("shader_type")}) {
        const auto& GL = gl_context.gl_api().constants();
        const auto delegate{[&GL, &locator, &gl_context, this](auto shdr_type) {
            return request_gl_shader(std::move(locator), gl_context, shdr_type);
        }};
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
auto resource_loader::request(
  std::type_identity<oglplus::owned_shader_name>,
  url locator,
  loaded_resource_context& ctx) noexcept -> resource_request_result {
    return request_gl_shader(std::move(locator), ctx.gl_context());
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_program(
  url locator,
  const oglplus::shared_gl_api_context& gl_context) noexcept
  -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_program)};
    new_request.info().add_gl_program_context(gl_context);

    if(const auto src_request{request_value_tree_traversal(
         locator, make_valtree_gl_program_builder(new_request, gl_context))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_program);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<oglplus::owned_program_name>,
  url locator,
  loaded_resource_context& ctx) noexcept -> resource_request_result {
    return request_gl_program(std::move(locator), ctx.gl_context());
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
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::texture_target target,
  oglplus::texture_unit unit,
  oglplus::texture_name tex) noexcept -> resource_request_result {
    if(const auto src_request{request_gl_texture_image(locator, target)}) {
        auto new_request{
          _new_resource(locator, resource_kind::gl_texture_update)};
        new_request.info().add_gl_texture_update_context(
          gl_context, target, unit, tex);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(locator, resource_kind::gl_texture_update);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(
  url locator,
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::texture_target target,
  oglplus::texture_unit unit) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_texture)};
    new_request.info().add_gl_texture_context(gl_context, target, unit);

    if(const auto src_request{request_json_traversal(
         locator,
         make_valtree_gl_texture_builder(
           new_request, gl_context, target, unit))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_texture);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<oglplus::owned_texture_name>,
  url locator,
  loaded_resource_context& ctx,
  oglplus::texture_target tex_target,
  oglplus::texture_unit tex_unit) noexcept -> resource_request_result {
    return request_gl_texture(
      std::move(locator), ctx.gl_context(), tex_target, tex_unit);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_buffer(
  url locator,
  const oglplus::shared_gl_api_context& gl_context,
  oglplus::buffer_target target) noexcept -> resource_request_result {
    auto new_request{_new_resource(locator, resource_kind::gl_buffer)};
    new_request.info().add_gl_buffer_context(gl_context, target);

    if(const auto src_request{request_value_tree_traversal(
         locator,
         make_valtree_gl_buffer_builder(new_request, gl_context, target))}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_buffer);
}
//------------------------------------------------------------------------------
auto resource_loader::request(
  std::type_identity<oglplus::owned_buffer_name>,
  url locator,
  loaded_resource_context& ctx,
  oglplus::buffer_target buf_target) noexcept -> resource_request_result {
    return request_gl_buffer(std::move(locator), ctx.gl_context(), buf_target);
}
//------------------------------------------------------------------------------
auto resource_loader::update_and_process_all() noexcept -> work_done {
    some_true something_done{base::update_and_process_all()};

    for(auto& [request_id, pinfo] : _cancelled) {
        assert(pinfo);
        auto& info{*pinfo};
        resource_cancelled(request_id, info._kind, info._locator);
        something_done();
    }
    _cancelled.clear();

    std::swap(_pending, _finished);

    something_done(_finished.erase_if([this](auto& entry) {
        auto& [request_id, pinfo] = entry;
        assert(pinfo);
        auto& info{*pinfo};
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
        something_done(pinfo->update());
    }

    return something_done;
}
//------------------------------------------------------------------------------
// pending_resource_requests
//------------------------------------------------------------------------------
pending_resource_requests::pending_resource_requests(
  resource_loader& loader) noexcept
  : _sig_bind{loader.load_status_changed.bind(
      make_callable_ref<&pending_resource_requests::_handle_load_status>(
        this))} {}
//------------------------------------------------------------------------------
pending_resource_requests::pending_resource_requests(
  execution_context& ctx) noexcept
  : pending_resource_requests{ctx.loader()} {}
//------------------------------------------------------------------------------
void pending_resource_requests::_handle_load_status(
  resource_load_status status,
  const identifier_t request_id,
  resource_kind,
  const url&) noexcept {
    if(status != resource_load_status::loading) {
        _request_ids.erase(request_id);
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::app
