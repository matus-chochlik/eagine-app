/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:old_resource_loader;

import std;
import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.container;
import eagine.core.identifier;
import eagine.core.reflection;
import eagine.core.serialization;
import eagine.core.value_tree;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.logging;
import eagine.core.progress;
import eagine.core.main_ctx;
import eagine.shapes;
import eagine.oglplus;
import eagine.msgbus;

namespace eagine {
namespace app {

export class loaded_resource_context;
export class execution_context;
export class orbiting_camera;
export class gl_geometry_and_bindings;
export class resource_request_result;
export class old_resource_loader;
//
struct resource_gl_texture_image_params;
struct resource_gl_texture_params;
struct resource_gl_buffer_data_params;
struct resource_gl_buffer_params;
//------------------------------------------------------------------------------
/// @brief Resource loading status.
/// @see old_resource_loader
/// @see loaded resource
/// @see resource_loader_signals
export enum class resource_load_status : std::uint8_t {
    loading,
    loaded,
    cancelled,
    not_found,
    error
};
//------------------------------------------------------------------------------
/// @brief Resource kind enumeration.
/// @see old_resource_loader
/// @see resource_loader_signals
export enum class resource_kind {
    /// @brief Unknown resource type.
    unknown,
    /// @brief Plain text.
    plain_text,
    /// @brief JSON text.
    json_text,
    /// @brief YAML text.
    yaml_text,
    /// @brief Vector of string values.
    string_list,
    /// @brief Vector of URL values.
    url_list,
    /// @brief Vector of floating-point values.
    float_vector,
    /// @brief Vector of vec3 values.
    vec3_vector,
    /// @brief Smooth bezier curve of vec3 points.
    smooth_vec3_curve,
    /// @brief Vector of mat4 values.
    mat4_vector,
    /// @brief Camera parameters.
    camera_parameters,
    /// @brief User input setup on the execution context.
    input_setup,
    /// @brief GLSL text.
    glsl_text,
    /// @brief Shape generator.
    shape_generator,
    /// @brief OGLplus shape wrapper.
    gl_shape,
    /// @brief Shape geometry buffers, vao instructions and attribute bindings.
    gl_geometry_and_bindings,
    /// @brief Value tree.
    value_tree,
    /// @brief Value tree stream traversal.
    value_tree_traversal,
    /// @brief GLSL source string collection.
    glsl_source,
    /// @brief GL shader source include string.
    gl_shader_include,
    /// @brief GL shader object.
    gl_shader,
    /// @brief GL program object.
    gl_program,
    /// @brief GL texture object.
    gl_texture,
    /// @brief GL texture image parameters and data.
    gl_texture_image,
    /// @brief GL texture image update.
    gl_texture_update,
    /// @brief GL buffer object.
    gl_buffer,
    /// @brief GL buffer content update.
    gl_buffer_update,
    /// @brief Arbitrary structure with defined attribute mapping.
    mapped_struct,
    /// @brief Marks that resource request is finished.
    finished
};
//------------------------------------------------------------------------------
/// @brief Structure containing parameters for a resource request.
/// @see old_resource_loader
/// @see resource_request_result
export using msgbus::resource_request_params;
//------------------------------------------------------------------------------
class pending_resource_info
  : public std::enable_shared_from_this<pending_resource_info> {
public:
    pending_resource_info(
      old_resource_loader& loader,
      identifier_t req_id,
      const resource_request_params&,
      resource_kind k) noexcept;

    [[nodiscard]] auto request_id() const noexcept -> identifier_t {
        return _request_id;
    }

    [[nodiscard]] auto parameters() const noexcept
      -> const resource_request_params& {
        return _params;
    }

    [[nodiscard]] auto is(resource_kind kind) const noexcept -> bool {
        return _kind == kind;
    }

    [[nodiscard]] auto is_finished() const noexcept -> bool {
        return is(resource_kind::finished);
    }

    [[nodiscard]] auto continuation() const noexcept
      -> shared_holder<pending_resource_info> {
        return _continuation.lock();
    }

    void set_continuation(
      const shared_holder<pending_resource_info>& cont) noexcept {
        _continuation = cont;
    }

    [[nodiscard]] auto loader() noexcept -> old_resource_loader& {
        return _parent;
    }

    void mark_loaded() noexcept;
    void mark_finished() noexcept;
    [[nodiscard]] auto is_done() const noexcept -> bool;

    void add_label(const string_view) noexcept;
    void apply_label() noexcept;

    void add_valtree_stream_input(
      valtree::value_tree_stream_input input) noexcept;

    template <mapped_struct T>
    void handle_mapped_struct(
      const pending_resource_info& source,
      T& object) noexcept;

    void handle_float_vector(
      const pending_resource_info& source,
      std::vector<float>& values) noexcept;

    void handle_vec3_vector(
      const pending_resource_info& source,
      std::vector<math::vector<float, 3>>& values) noexcept;

    void handle_mat4_vector(
      const pending_resource_info& source,
      std::vector<math::matrix<float, 4, 4, true>>& values) noexcept;

    void add_shape_generator(shared_holder<shapes::generator> gen) noexcept;
    void add_gl_shape_context(const oglplus::shared_gl_api_context&) noexcept;
    void add_gl_geometry_and_bindings_context(
      const oglplus::shared_gl_api_context&,
      oglplus::vertex_attrib_bindings,
      shapes::drawing_variant) noexcept;

    void add_gl_shader_include_context(
      const oglplus::shared_gl_api_context&,
      std::string include_path) noexcept;
    void add_gl_shader_context(
      const oglplus::shared_gl_api_context&,
      oglplus::shader_type) noexcept;
    void add_gl_program_context(const oglplus::shared_gl_api_context&) noexcept;
    auto add_gl_program_shader_request(identifier_t request_id) noexcept
      -> bool;
    auto add_gl_program_input_binding(
      std::string name,
      shapes::vertex_attrib_variant) noexcept -> bool;

    void handle_gl_texture_i_param(
      const oglplus::texture_parameter param,
      const oglplus::gl_types::int_type value) noexcept;
    void handle_gl_texture_generate_mipmap() noexcept;
    void handle_gl_texture_image(
      const oglplus::texture_target,
      resource_gl_texture_image_params&,
      const memory::const_block) noexcept;
    auto add_gl_texture_image_request(identifier_t request_id) noexcept -> bool;
    void add_gl_texture_update_context(
      const oglplus::shared_gl_api_context&,
      oglplus::texture_target,
      oglplus::texture_unit,
      oglplus::texture_name) noexcept;

    void add_gl_texture_context(
      const oglplus::shared_gl_api_context&,
      oglplus::texture_target,
      oglplus::texture_unit) noexcept;
    auto handle_gl_texture_params(
      shared_holder<resource_gl_texture_params>&) noexcept -> bool;

    void add_gl_buffer_context(
      const oglplus::shared_gl_api_context&,
      oglplus::buffer_target) noexcept;
    void handle_gl_buffer_data(
      const oglplus::buffer_target,
      const resource_gl_buffer_data_params&,
      const memory::const_block) noexcept;
    auto handle_gl_buffer_params(const resource_gl_buffer_params&) noexcept
      -> bool;

    auto update() noexcept -> work_done;
    void cleanup() noexcept;

    // continuation handlers
    void handle_source_data(
      const msgbus::blob_stream_chunk&,
      const pending_resource_info& source) noexcept;

    void handle_source_finished(const pending_resource_info&) noexcept;
    void handle_source_cancelled(const pending_resource_info&) noexcept;

private:
    friend class old_resource_loader;
    friend class resource_request_result;

    // the pending resource state
    struct _pending_valtree_traversal_state {
        valtree::value_tree_stream_input input;
    };

    struct _pending_shape_generator_state {
        shared_holder<shapes::generator> generator;
    };

    struct _pending_gl_shape_state {
        oglplus::shared_gl_api_context gl_context;
    };

    struct _pending_gl_geometry_and_bindings_state {
        oglplus::shared_gl_api_context gl_context;
        oglplus::vertex_attrib_bindings bindings;
        span_size_t draw_var_idx;
    };

    struct _pending_gl_shader_include_state {
        oglplus::shared_gl_api_context gl_context;
        std::string include_path;
    };

    struct _pending_gl_shader_state {
        oglplus::shared_gl_api_context gl_context;
        oglplus::shader_type shdr_type;
    };

    struct _pending_gl_program_state {
        oglplus::shared_gl_api_context gl_context;
        oglplus::owned_program_name prog;
        oglplus::program_input_bindings input_bindings;
        flat_set<identifier_t> pending_requests;
        bool loaded{false};
    };

    struct _pending_gl_texture_update_state {
        oglplus::shared_gl_api_context gl_context;
        oglplus::texture_target tex_target;
        oglplus::texture_unit tex_unit;
        oglplus::texture_name tex;
    };

    struct _pending_gl_texture_state {
        oglplus::shared_gl_api_context gl_context;
        shared_holder<const resource_gl_texture_params> params;
        flat_set<identifier_t> pending_requests;
        std::bitset<32> level_images_done;
        span_size_t levels{0};
        oglplus::texture_target tex_target;
        oglplus::texture_unit tex_unit;
        oglplus::owned_texture_name tex;
        bool generate_mipmap{false};
        bool loaded{false};
    };

    struct _pending_gl_buffer_update_state {
        oglplus::shared_gl_api_context gl_context;
        oglplus::buffer_target buf_target;
        oglplus::buffer_name buf;
    };

    struct _pending_gl_buffer_state {
        oglplus::shared_gl_api_context gl_context;
        oglplus::buffer_target buf_target;
        oglplus::owned_buffer_name buf;
        flat_set<identifier_t> pending_requests;
        bool loaded{false};
    };

    // progress_update
    void _preparation_progressed(float) noexcept;
    void _streaming_progressed(const msgbus::blob_stream_chunk&) noexcept;

    // resource loaded handlers
    void _handle_plain_text(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void _handle_json_text(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void _handle_yaml_text(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void _handle_value_tree(
      const pending_resource_info& source,
      const valtree::compound& tree) noexcept;

    void _handle_string_list(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void _handle_url_list(
      const pending_resource_info& source,
      const std::vector<url>& urls) noexcept;

    void _handle_vec3_vector(
      const pending_resource_info& source,
      const std::vector<math::vector<float, 3>>& values) noexcept;

    void _handle_mat4_vector(
      const pending_resource_info& source,
      const std::vector<math::matrix<float, 4, 4, true>>& values) noexcept;

    auto _apply_shape_modifiers(shared_holder<shapes::generator>) noexcept
      -> shared_holder<shapes::generator>;
    void _handle_shape_generator(
      const pending_resource_info& source,
      const shared_holder<shapes::generator>& gen) noexcept;
    void _handle_gl_shape(
      const pending_resource_info& source,
      const oglplus::shape_generator& shape) noexcept;

    void _handle_glsl_strings(
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void _handle_glsl_source(
      const pending_resource_info& source,
      const oglplus::glsl_source_ref& glsl_src) noexcept;

    void _handle_gl_shader_include(
      const pending_resource_info& source,
      const std::string_view text) noexcept;

    auto _finish_gl_program(_pending_gl_program_state&) noexcept -> bool;
    void _handle_gl_shader(
      const pending_resource_info& source,
      oglplus::owned_shader_name& shdr) noexcept;

    auto _finish_gl_texture(_pending_gl_texture_state&) noexcept -> bool;
    void _clear_gl_texture_image(
      const _pending_gl_texture_state&,
      const resource_gl_texture_params&,
      span_size_t level,
      const memory::const_block) noexcept;
    void _adjust_gl_texture_params(
      const oglplus::texture_target,
      const _pending_gl_texture_state&,
      resource_gl_texture_params&) noexcept;
    void _adjust_gl_texture_params(
      const oglplus::texture_target,
      const _pending_gl_texture_state&,
      resource_gl_texture_image_params&) noexcept;
    void _adjust_gl_texture_params(
      const oglplus::texture_target,
      const _pending_gl_texture_update_state&,
      resource_gl_texture_image_params&) noexcept;
    auto _handle_pending_gl_texture_state(
      auto& gl,
      auto& GL,
      auto& glapi,
      const _pending_gl_texture_state& pgts,
      const resource_gl_texture_params& params) noexcept -> bool;
    void _handle_gl_texture_image(
      const pending_resource_info& source,
      const oglplus::texture_target,
      resource_gl_texture_image_params&,
      const memory::const_block) noexcept;

    auto _finish_gl_buffer(_pending_gl_buffer_state&) noexcept -> bool;
    void _handle_gl_buffer_data(
      const pending_resource_info& source,
      const oglplus::buffer_target,
      const resource_gl_buffer_data_params&,
      const memory::const_block) noexcept;

    old_resource_loader& _parent;
    const identifier_t _request_id;
    const resource_request_params _params;
    span_size_t _received_size{0};
    activity_progress _preparation;
    activity_progress _streaming;

    std::string _label;
    weak_holder<pending_resource_info> _continuation{};

    std::variant<
      std::monostate,
      _pending_valtree_traversal_state,
      _pending_shape_generator_state,
      _pending_gl_shape_state,
      _pending_gl_geometry_and_bindings_state,
      _pending_gl_shader_include_state,
      _pending_gl_shader_state,
      _pending_gl_program_state,
      _pending_gl_texture_update_state,
      _pending_gl_texture_state,
      _pending_gl_buffer_update_state,
      _pending_gl_buffer_state>
      _state;

    resource_kind _kind{resource_kind::unknown};
};
//------------------------------------------------------------------------------
// value tree builders
//------------------------------------------------------------------------------
class valtree_builder_common : public main_ctx_object {
public:
    valtree_builder_common(
      identifier id,
      shared_holder<pending_resource_info> info) noexcept;

protected:
    weak_holder<pending_resource_info> _parent;
};
//------------------------------------------------------------------------------
template <typename Derived>
class valtree_builder_base
  : public valtree::object_builder_impl<Derived>
  , public valtree_builder_common {
public:
    using valtree_builder_common::valtree_builder_common;

    void do_add(const basic_string_path&, const auto&) noexcept {}

    auto finish() noexcept -> bool override {
        if(auto parent{_parent.lock()}) {
            parent->mark_loaded();
            return true;
        }
        return false;
    }

    void failed() noexcept override {
        if(auto parent{_parent.lock()}) {
            parent->mark_loaded();
        }
    }
};
//------------------------------------------------------------------------------
// valtree_mapped_struct_builder
//------------------------------------------------------------------------------
template <mapped_struct O>
class valtree_mapped_struct_builder final
  : public valtree_builder_base<valtree_mapped_struct_builder<O>> {
    using base = valtree_builder_base<valtree_mapped_struct_builder<O>>;

public:
    using base::base;
    using base::do_add;

    auto max_token_size() noexcept -> span_size_t final {
        return max_identifier_length(_object);
    }

    template <typename T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        _forwarder.forward_data(path, data, _object);
    }

    auto finish() noexcept -> bool final;

private:
    O _object{};
    valtree::object_builder_data_forwarder _forwarder;
};
//------------------------------------------------------------------------------
template <mapped_struct O>
auto valtree_mapped_struct_builder<O>::finish() noexcept -> bool {
    if(auto parent{this->_parent.lock()}) {
        parent->handle_mapped_struct(*parent, _object);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
export template <mapped_struct O>
auto make_mapped_struct_builder(
  const shared_holder<pending_resource_info>& parent,
  std::type_identity<O> = {}) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_mapped_struct_builder<O>>, "StrctBuldr", parent};
}
//------------------------------------------------------------------------------
// valtree_mapped_struct_loader
//------------------------------------------------------------------------------
template <mapped_struct O>
class valtree_mapped_struct_loader final
  : public valtree::object_builder_impl<valtree_mapped_struct_loader<O>> {

public:
    valtree_mapped_struct_loader(
      O& object,
      resource_load_status& status) noexcept
      : _object{object}
      , _status{status} {
        _status = resource_load_status::loading;
    }

    auto max_token_size() noexcept -> span_size_t final {
        return max_identifier_length(_object);
    }

    template <typename T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept {
        _forwarder.forward_data(path, data, _object);
    }

    auto finish() noexcept -> bool final {
        _status = resource_load_status::loaded;
        return true;
    }
    void failed() noexcept final {
        _status = resource_load_status::error;
    }

private:
    valtree::object_builder_data_forwarder _forwarder;
    O& _object{};
    resource_load_status& _status;
};
//------------------------------------------------------------------------------
export template <mapped_struct O>
auto make_mapped_struct_loader(O& object, resource_load_status& status) noexcept
  -> unique_holder<valtree::object_builder> {
    return {hold<valtree_mapped_struct_loader<O>>, object, status};
}
//------------------------------------------------------------------------------
// other builders
//------------------------------------------------------------------------------
auto make_valtree_float_vector_builder(
  const shared_holder<pending_resource_info>& parent) noexcept
  -> unique_holder<valtree::object_builder>;
auto make_valtree_vec3_vector_builder(
  const shared_holder<pending_resource_info>& parent) noexcept
  -> unique_holder<valtree::object_builder>;
auto make_valtree_mat4_vector_builder(
  const shared_holder<pending_resource_info>& parent) noexcept
  -> unique_holder<valtree::object_builder>;
auto make_valtree_camera_parameters_builder(
  const shared_holder<pending_resource_info>& parent,
  orbiting_camera&) noexcept -> unique_holder<valtree::object_builder>;
auto make_valtree_input_setup_builder(
  const shared_holder<pending_resource_info>& parent,
  execution_context&) noexcept -> unique_holder<valtree::object_builder>;
//------------------------------------------------------------------------------
auto make_valtree_gl_program_builder(
  const shared_holder<pending_resource_info>& parent,
  const oglplus::shared_gl_api_context&) noexcept
  -> unique_holder<valtree::object_builder>;
auto make_valtree_gl_texture_image_loader(
  const shared_holder<pending_resource_info>& parent,
  oglplus::texture_target,
  const shared_holder<resource_gl_texture_image_params>& params) noexcept
  -> unique_holder<valtree::object_builder>;
auto make_valtree_gl_texture_builder(
  const shared_holder<pending_resource_info>& parent,
  const oglplus::shared_gl_api_context&,
  oglplus::texture_target,
  oglplus::texture_unit) noexcept -> unique_holder<valtree::object_builder>;
auto make_valtree_gl_buffer_builder(
  const shared_holder<pending_resource_info>& parent,
  const oglplus::shared_gl_api_context&,
  oglplus::buffer_target) noexcept -> unique_holder<valtree::object_builder>;
//------------------------------------------------------------------------------
/// @brief Result of resource request operation.
/// @see resource_kind
/// @see resource_request_params
export class resource_request_result {
public:
    resource_request_result(
      shared_holder<pending_resource_info> info,
      bool cancelled) noexcept
      : _info{std::move(info)}
      , _was_cancelled{cancelled} {}

    resource_request_result(
      const std::pair<identifier_t, shared_holder<pending_resource_info>>& p,
      bool cancelled) noexcept
      : resource_request_result{std::get<1>(p), cancelled} {}

    /// @brief Indicates if the request is valid.
    explicit operator bool() const noexcept {
        return not _was_cancelled;
    }

    [[nodiscard]] auto info() const noexcept -> pending_resource_info&;

    operator shared_holder<pending_resource_info>() const noexcept {
        return _info;
    }

    /// @brief Returns a reference to the parent loader.
    [[nodiscard]] auto loader() const noexcept -> old_resource_loader& {
        return info().loader();
    }

    /// @brief Returns the unique id of the request.
    [[nodiscard]] auto request_id() const noexcept -> identifier_t {
        return info().request_id();
    }

    /// @brief Returns the request parameters.
    [[nodiscard]] auto parameters() const noexcept
      -> const resource_request_params& {
        return info()._params;
    }

    /// @brief Returns the locator of the requested resource.
    [[nodiscard]] auto locator() const noexcept -> const url& {
        return parameters().locator;
    }

    /// @brief Sets the reference to the continuation request of this request.
    auto set_continuation(const shared_holder<pending_resource_info>& cont)
      const noexcept -> const resource_request_result&;

    /// @brief Sets the reference to the continuation request of this request.
    auto set_continuation(resource_request_result& cont) const noexcept
      -> const resource_request_result&;

private:
    shared_holder<pending_resource_info> _info;
    bool _was_cancelled;
};
//------------------------------------------------------------------------------
/// @brief Collection of signals emitted by the resource loader.
/// @see old_resource_loader
export struct resource_loader_signals {
    /// @brief Emitted when the status of resource loading changed.
    signal<
      void(resource_load_status, identifier_t, resource_kind, const url&) noexcept>
      load_status_changed;

    void resource_loaded(
      identifier_t request_id,
      resource_kind kind,
      const url& locator) noexcept {
        load_status_changed(
          resource_load_status::loaded, request_id, kind, locator);
    }

    void resource_cancelled(
      identifier_t request_id,
      resource_kind kind,
      const url& locator) noexcept {
        load_status_changed(
          resource_load_status::cancelled, request_id, kind, locator);
    }

    template <typename T>
    struct get_load_info;

    /// @brief Returns the load info type for the specified resource type T.
    template <typename T>
    using load_info_t = typename get_load_info<T>::type;

    /// @brief Type of parameter of the shape_generator_loaded signal.
    /// @see shape_generator_loaded
    struct shape_generator_load_info {
        const identifier_t request_id;
        const url& locator;
        const shared_holder<shapes::generator>& generator;
    };

    template <>
    struct get_load_info<shared_holder<shapes::generator>>
      : std::type_identity<shape_generator_load_info> {};

    /// @brief Emitted when a shape generator is loaded.
    signal<void(const shape_generator_load_info&) noexcept>
      shape_generator_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<shared_holder<shapes::generator>>) noexcept -> auto& {
        return shape_generator_loaded;
    }

    /// @brief Type of parameter of the gl_shape_loaded signal.
    /// @see gl_shape_loaded
    struct gl_shape_load_info {
        const identifier_t request_id;
        const url& locator;
        const oglplus::shape_generator& shape;
    };

    template <>
    struct get_load_info<oglplus::shape_generator>
      : std::type_identity<gl_shape_load_info> {};

    /// @brief Emitted when a oglplus shape wrapper is loaded.
    signal<void(const gl_shape_load_info&) noexcept> gl_shape_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<oglplus::shape_generator>) noexcept -> auto& {
        return gl_shape_loaded;
    }

    /// @brief Type of parameter of the gl_geometry_and_bindings_loaded signal.
    /// @see gl_geometry_and_bindings_loaded
    struct gl_geometry_and_bindings_load_info {
        const identifier_t request_id;
        const url& locator;
        const oglplus::shape_generator& shape;
        gl_geometry_and_bindings& ref;
    };

    template <>
    struct get_load_info<gl_geometry_and_bindings>
      : std::type_identity<gl_geometry_and_bindings_load_info> {};

    /// @brief Emitted when a geometry and attribute bindings wrapper is loaded.
    signal<void(const gl_geometry_and_bindings_load_info&) noexcept>
      gl_geometry_and_bindings_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<gl_geometry_and_bindings>) noexcept -> auto& {
        return gl_geometry_and_bindings_loaded;
    }

    /// @brief Type of parameter of the value_tree_loaded signal.
    /// @see value_tree_loaded
    struct value_tree_load_info {
        const identifier_t request_id;
        const url& locator;
        const valtree::compound& tree;

        /// @brief Loads the data from the value tree resource into an object.
        auto load(mapped_struct auto& object) const noexcept -> bool {
            valtree_deserializer_backend backend{tree};
            const auto errors{deserialize(object, backend)};
            return not errors;
        }

        /// @brief Loads the data from the value tree resource into an object.
        auto load(mapped_struct auto& object, const basic_string_path& path)
          const noexcept -> bool {
            valtree_deserializer_backend backend{tree, tree.find(path)};
            const auto errors{deserialize(object, backend)};
            return not errors;
        }
    };

    template <>
    struct get_load_info<valtree::compound>
      : std::type_identity<value_tree_load_info> {};

    /// @brief Emitted when a value tree is loaded.
    signal<void(const value_tree_load_info&) noexcept> value_tree_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<valtree::compound>) noexcept -> auto& {
        return value_tree_loaded;
    }

    /// @brief Type of parameter of the plain_text_loaded signal.
    /// @see plain_text_loaded
    struct plain_text_load_info {
        const identifier_t request_id;
        const url& locator;
        std::string text;
    };

    template <>
    struct get_load_info<std::string>
      : std::type_identity<plain_text_load_info> {};

    /// @brief Emitted when plain text is loaded.
    signal<void(const plain_text_load_info&) noexcept> plain_text_loaded;

    [[nodiscard]] auto load_signal(std::type_identity<std::string>) noexcept
      -> auto& {
        return plain_text_loaded;
    }

    /// @brief Type of parameter of the string_list_loaded signal.
    /// @see string_list_loaded
    struct string_list_load_info {
        const identifier_t request_id;
        const url& locator;
        std::vector<std::string>& strings;
        const std::vector<std::string>& values{strings};
    };

    template <>
    struct get_load_info<std::vector<std::string>>
      : std::type_identity<string_list_load_info> {};

    signal<void(string_list_load_info&) noexcept> string_line_loaded;

    /// @brief Emitted when list of strings is loaded.
    signal<void(const string_list_load_info&) noexcept> string_list_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<std::vector<std::string>>) noexcept -> auto& {
        return string_list_loaded;
    }

    /// @brief Type of parameter of the url_list_loaded signal.
    /// @see url_list_loaded
    struct url_list_load_info {
        const identifier_t request_id;
        const url& locator;
        const std::vector<url>& values;
    };

    template <>
    struct get_load_info<std::vector<url>>
      : std::type_identity<url_list_load_info> {};

    /// @brief Emitted when list of strings is loaded.
    signal<void(const url_list_load_info&) noexcept> url_list_loaded;

    [[nodiscard]] auto load_signal(std::type_identity<std::vector<url>>) noexcept
      -> auto& {
        return url_list_loaded;
    }

    /// @brief Type of parameter of the float_vector_loaded signal.
    /// @see float_vector_loaded
    struct float_vector_load_info {
        const identifier_t request_id;
        const url& locator;
        std::vector<float>& values;
    };

    template <>
    struct get_load_info<std::vector<float>>
      : std::type_identity<float_vector_load_info> {};

    /// @brief Emitted when a vector of floating-point values is loaded.
    signal<void(const float_vector_load_info&) noexcept> float_vector_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<std::vector<float>>) noexcept -> auto& {
        return float_vector_loaded;
    }

    /// @brief Type of parameter of the vec3_vector_loaded signal.
    /// @see vec3_vector_loaded
    struct vec3_vector_load_info {
        const identifier_t request_id;
        const url& locator;
        std::vector<math::vector<float, 3>>& values;
    };

    template <>
    struct get_load_info<std::vector<math::vector<float, 3>>>
      : std::type_identity<vec3_vector_load_info> {};

    /// @brief Emitted when a vector of vec3 values is loaded.
    signal<void(const vec3_vector_load_info&) noexcept> vec3_vector_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<std::vector<math::vector<float, 3>>>) noexcept
      -> auto& {
        return vec3_vector_loaded;
    }

    /// @brief Type of parameter of the smooth_vec3_curve_loaded signal.
    /// @see smooth_vec3_curve_loaded
    struct smooth_vec3_curve_load_info {
        const identifier_t request_id;
        const url& locator;
        math::cubic_bezier_curves<math::vector<float, 3>, float>& curve;
    };

    template <>
    struct get_load_info<math::cubic_bezier_curves<math::vector<float, 3>, float>>
      : std::type_identity<smooth_vec3_curve_load_info> {};

    /// @brief Emitted when a smooth closed loop of vec3 values is loaded.
    signal<void(const smooth_vec3_curve_load_info&) noexcept>
      smooth_vec3_curve_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<
        math::cubic_bezier_curves<math::vector<float, 3>, float>>) noexcept
      -> auto& {
        return smooth_vec3_curve_loaded;
    }

    /// @brief Type of parameter of the mat4_vector_loaded signal.
    /// @see mat4_vector_loaded
    struct mat4_vector_load_info {
        const identifier_t request_id;
        const url& locator;
        std::vector<math::matrix<float, 4, 4, true>>& values;
    };

    template <>
    struct get_load_info<std::vector<math::matrix<float, 4, 4, true>>>
      : std::type_identity<mat4_vector_load_info> {};

    /// @brief Emitted when a vector of mat4 values is loaded.
    signal<void(const mat4_vector_load_info&) noexcept> mat4_vector_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<std::vector<math::matrix<float, 4, 4, true>>>) noexcept
      -> auto& {
        return mat4_vector_loaded;
    }

    /// @brief Type of parameter of the glsl_source_loaded signal.
    /// @see glsl_source_loaded
    struct glsl_source_load_info {
        const identifier_t request_id;
        const url& locator;
        const oglplus::glsl_source_ref& source;
    };

    /// @brief Emitted when a GLSL source code is loaded.
    signal<void(const glsl_source_load_info&) noexcept> glsl_source_loaded;

    /// @brief Type of parameter of the gl_shader_include_loaded signal.
    /// @see gl_shader_include_loaded
    struct gl_shader_include_load_info {
        const identifier_t request_id;
        const url& locator;
        const std::string_view include_path;
        const std::string_view shader_source;
    };

    /// @brief Emitted when a GL shader include string is loaded.
    signal<void(const gl_shader_include_load_info&) noexcept>
      gl_shader_include_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<oglplus::shader_include>) noexcept -> auto& {
        return gl_shader_include_loaded;
    }

    /// @brief Type of parameter of the gl_shader_loaded signal.
    /// @see gl_shader_loaded
    struct gl_shader_load_info {
        const identifier_t request_id;
        const url& locator;
        oglplus::shared_gl_api_context gl_context;
        const oglplus::shader_type type;
        const oglplus::shader_name name;
        oglplus::owned_shader_name& ref;
    };

    template <>
    struct get_load_info<oglplus::owned_shader_name>
      : std::type_identity<gl_shader_load_info> {};

    /// @brief Emitted when a GL shader is successfully created and compiled.
    signal<void(const gl_shader_load_info&) noexcept> gl_shader_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<oglplus::owned_shader_name>) noexcept -> auto& {
        return gl_shader_loaded;
    }

    /// @brief Type of parameter of the gl_program_loaded signal.
    /// @see gl_program_loaded
    struct gl_program_load_info {
        const identifier_t request_id;
        const url& locator;
        oglplus::shared_gl_api_context gl_context;
        const oglplus::program_name name;
        oglplus::owned_program_name& ref;
        oglplus::program_input_bindings& input_bindings;

        auto apply_input_bindings(
          const oglplus::vertex_attrib_bindings& attrib_bindings) const noexcept
          -> bool {
            return input_bindings.apply(
              gl_context.gl_api(), name, attrib_bindings);
        }

        auto use_program() const noexcept {
            return gl_context.gl_api().use_program(name);
        }

        auto get_uniform_location(const string_view var_name) const noexcept {
            return gl_context.gl_api().get_uniform_location(name, var_name);
        }

        template <typename T>
        auto set_uniform(oglplus::uniform_location loc, const T& value)
          const noexcept {
            return gl_context.gl_api().set_uniform(name, loc, value);
        }

        template <typename T>
        auto set_uniform(const string_view var_name, const T& value)
          const noexcept {
            oglplus::uniform_location loc;
            get_uniform_location(var_name) >> loc;
            return set_uniform(loc, value);
        }

        auto get_shader_storage_block_index(
          const string_view var_name) const noexcept {
            return gl_context.gl_api().get_shader_storage_block_index(
              name, var_name);
        }

        auto shader_storage_block_binding(
          oglplus::shader_storage_block_index index,
          oglplus::gl_types::uint_type binding) const noexcept {
            return gl_context.gl_api().shader_storage_block_binding(
              name, index, binding);
        }

        auto shader_storage_block_binding(
          const string_view var_name,
          oglplus::gl_types::uint_type binding) const noexcept {
            oglplus::shader_storage_block_index index;
            get_shader_storage_block_index(var_name) >> index;
            return shader_storage_block_binding(index, binding);
        }
    };

    template <>
    struct get_load_info<oglplus::owned_program_name>
      : std::type_identity<gl_program_load_info> {};

    /// @brief Emitted when a GL program is successfully created and linked.
    signal<void(const gl_program_load_info&) noexcept> gl_program_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<oglplus::owned_program_name>) noexcept -> auto& {
        return gl_program_loaded;
    }

    /// @brief Type of parameter of the gl_texture_loaded signal.
    /// @see gl_texture_loaded
    struct gl_texture_load_info {
        const identifier_t request_id;
        const url& locator;
        oglplus::shared_gl_api_context gl_context;
        const oglplus::texture_target target;
        const oglplus::texture_name name;
        oglplus::owned_texture_name& ref;

        template <typename Param, typename Value>
        auto parameter_f(Param param, Value value) const noexcept {
            return gl_context.gl_api().tex_parameter_f(target, param, value);
        }

        template <typename Param, typename Value>
        auto parameter_i(Param param, Value value) const noexcept {
            return gl_context.gl_api().tex_parameter_i(target, param, value);
        }

        auto generate_mipmap() const noexcept {
            return gl_context.gl_api().operations().generate_mipmap(target);
        }
    };

    template <>
    struct get_load_info<oglplus::owned_texture_name>
      : std::type_identity<gl_texture_load_info> {};

    /// @brief Emitted when a GL texture is successfully created and its parameters set-up.
    signal<void(const gl_texture_load_info&) noexcept> gl_texture_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<oglplus::owned_texture_name>) noexcept -> auto& {
        return gl_texture_loaded;
    }

    /// @brief Type of parameter of the gl_texture_images_loaded signal.
    /// @see gl_texture_image_loaded
    struct gl_texture_images_load_info {
        const identifier_t request_id;
        const url& locator;
        oglplus::shared_gl_api_context gl_context;
        const oglplus::texture_name name;
    };

    /// @brief Emitted when all GL texture images are successfully loaded.
    signal<void(const gl_texture_images_load_info&) noexcept>
      gl_texture_images_loaded;

    /// @brief Type of parameter of the gl_buffer_loaded signal.
    /// @see gl_buffer_loaded
    struct gl_buffer_load_info {
        const identifier_t request_id;
        const url& locator;
        oglplus::shared_gl_api_context gl_context;
        const oglplus::buffer_name name;
        oglplus::owned_buffer_name& ref;
    };

    template <>
    struct get_load_info<oglplus::owned_buffer_name>
      : std::type_identity<gl_buffer_load_info> {};

    /// @brief Emitted when a GL buffer is successfully created and set-up.
    signal<void(const gl_buffer_load_info&) noexcept> gl_buffer_loaded;

    [[nodiscard]] auto load_signal(
      std::type_identity<oglplus::owned_buffer_name>) noexcept -> auto& {
        return gl_buffer_loaded;
    }

    class mapped_struct_load_info {
    public:
        const identifier_t request_id;
        const url& locator;

        template <typename O>
        mapped_struct_load_info(
          const identifier_t req_id,
          const url& loc,
          O& object) noexcept
          : request_id{req_id}
          , locator{loc}
          , _typeidx{typeid(std::remove_cv_t<O>)}
          , _ptr{&object} {}

        template <typename O>
        auto has_type(std::type_identity<O> = {}) const noexcept -> bool {
            return (_typeidx == typeid(std::remove_cv_t<O>));
        }

        template <typename O>
        auto as(std::type_identity<O> tid = {}) const noexcept
          -> optional_reference<O> {
            if(_ptr and has_type(tid)) {
                return {static_cast<O*>(_ptr)};
            }
            return {};
        }

        template <typename O>
        auto move_to(O& object) const
          noexcept(std::is_nothrow_move_assignable_v<O>) -> bool {
            if(_ptr and has_type<O>()) {
                object = std::move(*static_cast<O*>(_ptr));
                return true;
            }
            return false;
        }

    private:
        std::type_index _typeidx;
        void* _ptr;
    };

    template <typename T>
    struct get_load_info : std::type_identity<mapped_struct_load_info> {};

    /// @brief Emitted when a mapped struct is successfully created and set-up.
    signal<void(const mapped_struct_load_info&) noexcept> mapped_struct_loaded;

    template <mapped_struct O>
    [[nodiscard]] auto load_signal(std::type_identity<O>) noexcept -> auto& {
        return mapped_struct_loaded;
    }
};
//------------------------------------------------------------------------------
template <typename T>
concept resource_load_event_observer = requires(
  T v,
  resource_load_status status,
  identifier_t request_id,
  resource_kind kind,
  const url& locator) {
    v.handle_resource_load_event(status, request_id, kind, locator);
};

template <typename T>
concept resource_blob_preparation_progressed_observer =
  requires(T v, const msgbus::blob_stream_chunk& chunk) {
      v.handle_blob_preparation_progressed(chunk);
  };

template <typename T>
concept resource_blob_stream_data_appended_observer =
  requires(T v, const msgbus::blob_stream_chunk& chunk) {
      v.handle_blob_stream_data_appended(chunk);
  };

template <typename T>
concept resource_shape_generator_loaded_observer = requires(
  T v,
  const resource_loader_signals::shape_generator_load_info& info) {
    v.handle_shape_generator_loaded(info);
};

template <typename T>
concept resource_gl_shape_loaded_observer =
  requires(T v, const resource_loader_signals::gl_shape_load_info& info) {
      v.handle_gl_shape_loaded(info);
  };

template <typename T>
concept resource_gl_geometry_and_bindings_loaded_observer = requires(
  T v,
  const resource_loader_signals::gl_geometry_and_bindings_load_info& info) {
    v.handle_gl_geometry_and_bindings_loaded(info);
};

template <typename T>
concept resource_value_tree_loaded_observer =
  requires(T v, const resource_loader_signals::value_tree_load_info& info) {
      v.handle_value_tree_loaded(info);
  };

template <typename T>
concept resource_glsl_source_loaded_observer =
  requires(T v, const resource_loader_signals::glsl_source_load_info& info) {
      v.handle_glsl_source_loaded(info);
  };

template <typename T>
concept resource_gl_shader_include_loaded_observer = requires(
  T v,
  const resource_loader_signals::gl_shader_include_load_info& info) {
    v.handle_gl_shader_include_loaded(info);
};

template <typename T>
concept resource_gl_shader_loaded_observer =
  requires(T v, const resource_loader_signals::gl_shader_load_info& info) {
      v.handle_gl_shader_loaded(info);
  };

template <typename T>
concept resource_gl_program_loaded_observer =
  requires(T v, const resource_loader_signals::gl_program_load_info& info) {
      v.handle_gl_program_loaded(info);
  };

template <typename T>
concept resource_gl_texture_loaded_observer =
  requires(T v, const resource_loader_signals::gl_texture_load_info& info) {
      v.handle_gl_texture_loaded(info);
  };

template <typename T>
concept resource_gl_buffer_loaded_observer =
  requires(T v, const resource_loader_signals::gl_buffer_load_info& info) {
      v.handle_gl_buffer_loaded(info);
  };
//------------------------------------------------------------------------------
/// @brief Loader of resources of various types.
/// @see resource_request_result
/// @see pending_resource_requests
export class old_resource_loader
  : public msgbus::resource_data_consumer_node
  , public resource_loader_signals {
    using base = msgbus::resource_data_consumer_node;

public:
    /// @brief Initializing constructor
    old_resource_loader(msgbus::endpoint& bus)
      : resource_data_consumer_node{bus} {
        _init();
    }

    template <typename O>
    void connect_observer(O& observer) noexcept {
        if constexpr(resource_blob_preparation_progressed_observer<O>) {
            connect<&O::handle_blob_preparation_progressed>(
              &observer, this->blob_preparation_progressed);
        }
        if constexpr(resource_blob_stream_data_appended_observer<O>) {
            connect<&O::handle_blob_stream_data_appended>(
              &observer, this->blob_stream_data_appended);
        }
        if constexpr(resource_load_event_observer<O>) {
            connect<&O::handle_resource_load_event>(
              &observer, &this->load_status_changed);
        }
        if constexpr(resource_shape_generator_loaded_observer<O>) {
            connect<&O::handle_shape_generator_loaded>(
              &observer, this->shape_generator_loaded);
        }
        if constexpr(resource_gl_shape_loaded_observer<O>) {
            connect<&O::handle_gl_shape_loaded>(
              &observer, this->gl_shape_loaded);
        }
        if constexpr(resource_gl_geometry_and_bindings_loaded_observer<O>) {
            connect<&O::handle_gl_geometry_and_bindings_loaded>(
              &observer, this->gl_geometry_and_bindings_loaded);
        }
        if constexpr(resource_value_tree_loaded_observer<O>) {
            connect<&O::handle_value_tree_loaded>(
              &observer, this->value_tree_loaded);
        }
        if constexpr(resource_glsl_source_loaded_observer<O>) {
            connect<&O::handle_glsl_source_loaded>(
              &observer, this->glsl_source_loaded);
        }
        if constexpr(resource_gl_shader_include_loaded_observer<O>) {
            connect<&O::handle_gl_shader_include_loaded>(
              &observer, this->gl_shader_include_loaded);
        }
        if constexpr(resource_gl_shader_loaded_observer<O>) {
            connect<&O::handle_gl_shader_loaded>(
              &observer, this->gl_shader_loaded);
        }
        if constexpr(resource_gl_program_loaded_observer<O>) {
            connect<&O::handle_gl_program_loaded>(
              &observer, this->gl_program_loaded);
        }
        if constexpr(resource_gl_texture_loaded_observer<O>) {
            connect<&O::handle_gl_texture_loaded>(
              &observer, this->gl_texture_loaded);
        }
        if constexpr(resource_gl_buffer_loaded_observer<O>) {
            connect<&O::handle_gl_buffer_loaded>(
              &observer, this->gl_buffer_loaded);
        }
    }

    /// @brief Indicates if this loader has any pending requests.
    [[nodiscard]] auto has_pending_requests() const noexcept -> bool {
        return not _pending.empty();
    }

    void forget_resource(identifier_t request_id) noexcept;

    /// @brief Does some work and updates internal state (should be called periodically).
    auto update_and_process_all() noexcept -> work_done final;

    /// @brief Requests plain text resource.
    auto request_plain_text(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<std::string>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests string-list resource.
    auto request_string_list(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<std::vector<std::string>>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests URL-list resource.
    auto request_url_list(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<std::vector<url>>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests a float vector resource.
    auto request_float_vector(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<std::vector<float>>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests a float vector resource.
    auto request_vec3_vector(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<std::vector<math::vector<float, 3>>>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests a smooth vec3 curve resource.
    auto request_smooth_vec3_curve(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<
        math::cubic_bezier_curves<math::vector<float, 3>, float>>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests a 4x4 matrix resource.
    auto request_mat4_vector(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<std::vector<math::matrix<float, 4, 4, true>>>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests a value tree object resource.
    auto request_value_tree(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<valtree::compound>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    auto request_json_traversal(
      const resource_request_params&,
      shared_holder<valtree::value_tree_visitor>,
      span_size_t max_token_size) noexcept -> resource_request_result;

    auto request_json_traversal(
      const resource_request_params&,
      shared_holder<valtree::object_builder>) noexcept
      -> resource_request_result;

    /// @brief Requests a value tree object traversal by the specified visitor.
    auto request_value_tree_traversal(
      const resource_request_params&,
      shared_holder<valtree::value_tree_visitor>,
      span_size_t max_token_size) noexcept -> resource_request_result;

    /// @brief Requests a value tree object traversal by the specified builder.
    auto request_value_tree_traversal(
      const resource_request_params&,
      shared_holder<valtree::object_builder>) noexcept
      -> resource_request_result;

    /// @brief Requests camera parameters.
    auto request_camera_parameters(
      const resource_request_params&,
      orbiting_camera&) noexcept -> resource_request_result;

    /// @brief Requests user input setup.
    auto request_input_setup(
      const resource_request_params&,
      execution_context&) noexcept -> resource_request_result;

    /// @brief Requests a shape geometry generator / loader object.
    auto request_shape_generator(const resource_request_params&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<shared_holder<shapes::generator>>,
      const resource_request_params&,
      loaded_resource_context&) noexcept -> resource_request_result;

    /// @brief Requests a oglplus shape generator wrapper object.
    auto request_gl_shape(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<oglplus::shape_generator>,
      const resource_request_params&,
      loaded_resource_context& ctx) noexcept -> resource_request_result;

    /// @brief Requests a shape geometry and attrib bindings object.
    auto request_gl_geometry_and_bindings(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&,
      oglplus::vertex_attrib_bindings,
      span_size_t draw_var_idx = 0) noexcept -> resource_request_result;

    auto request(
      std::type_identity<gl_geometry_and_bindings>,
      const resource_request_params&,
      loaded_resource_context& ctx,
      oglplus::vertex_attrib_bindings bindings,
      span_size_t draw_var_idx = 0) noexcept -> resource_request_result;

    /// @brief Requests a shape geometry and attrib bindings object.
    auto request_gl_geometry_and_bindings(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&,
      span_size_t draw_var_idx = 0) noexcept -> resource_request_result;

    auto request(
      std::type_identity<gl_geometry_and_bindings>,
      const resource_request_params&,
      loaded_resource_context& ctx,
      span_size_t draw_var_idx = 0) noexcept -> resource_request_result;

    /// @brief Requests GLSL shader source code resource.
    auto request_glsl_source(const resource_request_params&) noexcept
      -> resource_request_result;

    /// @brief Requests GL shader include string resource.
    auto request_gl_shader_include(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&,
      std::string path = {}) noexcept -> resource_request_result;

    auto request(
      std::type_identity<oglplus::shader_include>,
      const resource_request_params&,
      loaded_resource_context& ctx,
      std::string path) noexcept -> resource_request_result;

    /// @brief Requests a compiled GL shader object of a specified type.
    auto request_gl_shader(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&,
      oglplus::shader_type) noexcept -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_shader_name>,
      const resource_request_params&,
      loaded_resource_context& ctx,
      oglplus::shader_type shdr_type) noexcept -> resource_request_result;

    /// @brief Requests a compiled GL shader object of a type specified in URL.
    auto request_gl_shader(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_shader_name>,
      const resource_request_params&,
      loaded_resource_context& ctx) noexcept -> resource_request_result;

    /// @brief Requests a linked GL program object.
    auto request_gl_program(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_program_name>,
      const resource_request_params&,
      loaded_resource_context& ctx) noexcept -> resource_request_result;

    auto request_gl_texture_image(
      const resource_request_params&,
      oglplus::texture_target,
      const shared_holder<resource_gl_texture_image_params>&) noexcept
      -> resource_request_result;

    auto request_gl_texture_image(
      const resource_request_params&,
      oglplus::texture_target) noexcept -> resource_request_result;

    /// @brief Requests image data update for a GL texture.
    auto request_gl_texture_update(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&,
      oglplus::texture_target,
      oglplus::texture_unit,
      oglplus::texture_name) noexcept -> resource_request_result;

    /// @brief Requests a set-up GL texture object.
    auto request_gl_texture(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&,
      oglplus::texture_target,
      oglplus::texture_unit) noexcept -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_texture_name>,
      const resource_request_params&,
      loaded_resource_context& ctx,
      oglplus::texture_target tex_target,
      oglplus::texture_unit tex_unit) noexcept -> resource_request_result;

    /// @brief Requests a set-up GL buffer object.
    auto request_gl_buffer(
      const resource_request_params&,
      const oglplus::shared_gl_api_context&,
      oglplus::buffer_target) noexcept -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_buffer_name>,
      const resource_request_params&,
      loaded_resource_context& ctx,
      oglplus::buffer_target buf_target) noexcept -> resource_request_result;

    /// @brief Requests a mapped structure
    template <mapped_struct O>
    auto request_mapped_struct(
      const resource_request_params& params,
      std::type_identity<O> tid = {}) noexcept -> resource_request_result {
        auto new_request{_new_resource(params, resource_kind::mapped_struct)};

        if(const auto src_request{request_value_tree_traversal(
             params, make_mapped_struct_builder(new_request, tid))}) {
            return new_request;
        }
        new_request.info().mark_finished();
        return _cancelled_resource(params, resource_kind::mapped_struct);
    }

    template <mapped_struct O>
    auto request(
      std::type_identity<O> tid,
      const resource_request_params& params,
      loaded_resource_context&) noexcept {
        return request_mapped_struct(params, tid);
    }

private:
    friend class pending_resource_info;

    auto _is_json_resource(const url& locator) const noexcept -> bool;

    void _init() noexcept;

    void _handle_preparation_progressed(identifier_t blob_id, float) noexcept;
    void _handle_stream_data_appended(const msgbus::blob_stream_chunk&) noexcept;
    void _handle_stream_finished(identifier_t blob_id) noexcept;
    void _handle_stream_cancelled(identifier_t blob_id) noexcept;

    auto _cancelled_resource(
      const identifier_t blob_id,
      const resource_request_params&,
      const resource_kind) noexcept -> resource_request_result;

    auto _cancelled_resource(
      const resource_request_params& params,
      const resource_kind kind) noexcept -> resource_request_result {
        return _cancelled_resource(get_request_id(), params, kind);
    }

    auto _cancelled_resource(const resource_request_params& params) noexcept
      -> resource_request_result {
        return _cancelled_resource(params, resource_kind::unknown);
    }

    auto _new_resource(
      const identifier_t blob_id,
      const resource_request_params&,
      resource_kind) noexcept -> resource_request_result;

    auto _new_resource(
      const std::pair<identifier_t, const url&>& id_and_loc,
      resource_kind kind) noexcept -> resource_request_result {
        return _new_resource(
          std::get<0>(id_and_loc), {.locator = std::get<1>(id_and_loc)}, kind);
    }

    auto _new_resource(
      const resource_request_params& params,
      resource_kind kind) noexcept -> resource_request_result {
        return _new_resource(get_request_id(), params, kind);
    }

    flat_map<identifier_t, shared_holder<pending_resource_info>> _pending;
    flat_map<identifier_t, shared_holder<pending_resource_info>> _finished;
    flat_map<identifier_t, shared_holder<pending_resource_info>> _cancelled;
};
//------------------------------------------------------------------------------
template <mapped_struct T>
void pending_resource_info::handle_mapped_struct(
  const pending_resource_info&,
  T& object) noexcept {
    if(is(resource_kind::mapped_struct)) {
        resource_loader_signals::mapped_struct_load_info info{
          _request_id, _params.locator, object};
        _parent.mapped_struct_loaded(info);
        _parent.resource_loaded(_request_id, _kind, _params.locator);
    }
    mark_finished();
}
//------------------------------------------------------------------------------
/// @brief Class tracking specific pending resource load requests.
/// @see old_resource_loader
/// @see resource_request_result
export class pending_resource_requests {
public:
    /// @brief Construction from a reference to old_resource_loader.
    pending_resource_requests(old_resource_loader& loader) noexcept;

    /// @brief Construction from a reference to execution_context.
    pending_resource_requests(execution_context&) noexcept;

    /// @brief Adds a new resource request to be tracked.
    auto add(const resource_request_result& res) noexcept
      -> pending_resource_requests& {
        _request_ids.insert(res.request_id());
        return *this;
    }

    /// @brief Returns the number of currently pending resource requests.
    [[nodiscard]] auto pending_count() const noexcept -> span_size_t {
        return span_size(_request_ids.size());
    }

    /// @brief Indicates if all the tracked requests finished loading.
    [[nodiscard]] auto all_are_loaded() const noexcept -> bool {
        return _request_ids.empty();
    }

    /// @brief Indicates if all the tracked requests finished loading.
    /// @see all_are_loaded
    explicit operator bool() const noexcept {
        return all_are_loaded();
    }

private:
    void _handle_load_status(
      resource_load_status,
      const identifier_t request_id,
      resource_kind,
      const url&) noexcept;

    signal_binding _sig_bind;
    flat_set<identifier_t> _request_ids;
};
//------------------------------------------------------------------------------
} // namespace app
//------------------------------------------------------------------------------
export template <>
struct enumerator_traits<app::resource_load_status> {
    static constexpr auto mapping() noexcept {
        using app::resource_load_status;
        return enumerator_map_type<resource_load_status, 5>{
          {{"loading", resource_load_status::loading},
           {"loaded", resource_load_status::loaded},
           {"cancelled", resource_load_status::cancelled},
           {"not_found", resource_load_status::not_found},
           {"error", resource_load_status::error}}};
    }
};
//------------------------------------------------------------------------------
} // namespace eagine
