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
      : parent{info}
      , video{vc} {}

    void do_add(const basic_string_path&, const auto&) noexcept {}

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept {
        if((path.size() == 2) && (path.front() == "urls")) {
            auto& loader = parent.loader();
            auto& GL = video.gl_api().constants();
            const auto delegate = [&, this](oglplus::shader_type shdr_type) {
                if(auto src_request{loader.request_gl_shader(
                     url{to_string(extract(data))}, shdr_type, video)}) {
                    src_request.set_continuation(parent);
                    if(!parent.add_gl_program_shader_request(
                         src_request.request_id())) [[unlikely]] {
                        src_request.info().mark_finished();
                        parent.mark_finished();
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

    void failed() noexcept final {
        parent.mark_finished();
    }

private:
    pending_resource_info& parent;
    video_context& video;
};
//------------------------------------------------------------------------------
// pending_resource_info
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
  shapes::drawing_variant draw_var) noexcept {
    _state = _pending_gl_geometry_and_bindings_state{
      .video = video, .bindings = std::move(bindings), .draw_var = draw_var};
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
    }
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_json_text(
  const msgbus::blob_info&,
  const pending_resource_info&,
  const span_size_t,
  const memory::span<const memory::const_block> data) noexcept {
    if(is(resource_kind::value_tree)) {
        auto tree{valtree::from_json_data(data, _parent.main_context().log())};
        if(_continuation) {
            extract(_continuation)._handle_value_tree(*this, tree);
        }
        _parent.value_tree_loaded(_request_id, tree, _locator);
    } else if(is(resource_kind::value_tree_traversal)) {
        if(std::holds_alternative<_pending_valtree_traversal_state>(_state)) {
            for(auto chunk : data) {
                std::get<_pending_valtree_traversal_state>(_state)
                  .input.consume_data(chunk);
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
    if(is(resource_kind::value_tree)) {
    }
    mark_finished();
}
//------------------------------------------------------------------------------
void pending_resource_info::_handle_value_tree(
  const pending_resource_info& source,
  const valtree::compound& tree) noexcept {
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
    if(is(resource_kind::gl_geometry_and_bindings)) {
        if(std::holds_alternative<_pending_gl_geometry_and_bindings_state>(
             _state)) {
            auto temp{_parent.buffers().get()};
            auto& pggbs =
              std::get<_pending_gl_geometry_and_bindings_state>(_state);
            geometry_and_bindings geom{
              shape, pggbs.bindings, pggbs.draw_var, pggbs.video, temp};
            _parent.gl_geometry_and_bindings_loaded(
              _request_id, {geom}, _locator);
            if(geom) {
                geom.clean_up(pggbs.video);
            }
            _parent.buffers().eat(std::move(temp));
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
void pending_resource_info::_handle_gl_shader(
  const pending_resource_info& source,
  oglplus::owned_shader_name& shdr) noexcept {
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
                    if(pgps.pending_requests.empty()) {
                        gl.link_program(pgps.prog);
                        _parent.gl_program_loaded(
                          _request_id, pgps.prog, {pgps.prog}, _locator);

                        if(pgps.prog) {
                            gl.delete_program(std::move(pgps.prog));
                        }
                    } else {
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
auto resource_loader::request_value_tree_traversal(
  url locator,
  std::shared_ptr<valtree::value_tree_visitor> visitor,
  span_size_t max_token_size) noexcept -> resource_request_result {
    if(locator.has_path_suffix(".json") || locator.has_scheme("json")) {
        if(const auto src_request{_new_resource(
             stream_resource(
               std::move(locator),
               msgbus::message_priority::normal,
               std::chrono::hours{1}),
             resource_kind::json_text)}) {

            auto new_request{_new_resource(
              std::move(locator), resource_kind::value_tree_traversal)};
            new_request.info().add_valtree_stream_input(traverse_json_stream(
              std::move(visitor),
              max_token_size,
              buffers(),
              main_context().log()));
            src_request.set_continuation(new_request);
            return new_request;
        }
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
  oglplus::vertex_attrib_bindings bindins,
  shapes::drawing_variant draw_var) noexcept -> resource_request_result {
    if(const auto src_request{request_gl_shape(locator, vc)}) {
        auto new_request{_new_resource(
          std::move(locator), resource_kind::gl_geometry_and_bindings)};
        new_request.info().add_gl_geometry_and_bindings_context(
          vc, std::move(bindins), draw_var);
        src_request.set_continuation(new_request);
        return new_request;
    }
    return _cancelled_resource(
      locator, resource_kind::gl_geometry_and_bindings);
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
auto resource_loader::request_gl_buffer(url locator, video_context&) noexcept
  -> resource_request_result {
    return _cancelled_resource(locator);
}
//------------------------------------------------------------------------------
auto resource_loader::request_gl_texture(url locator, video_context&) noexcept
  -> resource_request_result {
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
         1024)}) {
        return new_request;
    }
    new_request.info().mark_finished();
    return _cancelled_resource(locator, resource_kind::gl_program);
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
