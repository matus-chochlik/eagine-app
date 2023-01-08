/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:resource_loader;

import eagine.core.types;
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.container;
import eagine.core.reflection;
import eagine.core.serialization;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.logging;
import eagine.core.main_ctx;
import eagine.shapes;
import eagine.oglplus;
import eagine.msgbus;
import <bitset>;
import <memory>;
import <functional>;
import <utility>;
import <variant>;

namespace eagine::app {

export class execution_context;
export class orbiting_camera;
export class video_context;
export class audio_context;
export class gl_geometry_and_bindings;
export class resource_request_result;
export class resource_loader;
//
struct resource_gl_texture_image_params;
struct resource_gl_texture_params;
struct resource_gl_buffer_data_params;
struct resource_gl_buffer_params;
//------------------------------------------------------------------------------
/// @brief Resource kind enumeration.
/// @see resource_loader
/// @see resource_loader_signals
export enum class resource_kind {
    /// @brief Unknown resource type.
    unknown,
    /// @brief JSON text.
    json_text,
    /// @brief YAML text.
    yaml_text,
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
    /// @brief GL shader object.
    gl_shader,
    ///@brief GL program object.
    gl_program,
    ///@brief GL texture object.
    gl_texture,
    ///@brief GL texture image parameters and data.
    gl_texture_image,
    ///@brief GL texture image update.
    gl_texture_update,
    ///@brief GL buffer object.
    gl_buffer,
    ///@brief GL buffer image update.
    gl_buffer_update,
    /// @brief Marks that resource request is finished.
    finished
};
//------------------------------------------------------------------------------
class pending_resource_info
  : public std::enable_shared_from_this<pending_resource_info> {
public:
    pending_resource_info(
      resource_loader& loader,
      identifier_t req_id,
      url loc,
      resource_kind k) noexcept
      : _parent{loader}
      , _request_id{req_id}
      , _locator{std::move(loc)}
      , _kind{k} {}

    auto request_id() const noexcept -> identifier_t {
        return _request_id;
    }

    auto locator() const noexcept -> const url& {
        return _locator;
    }

    auto is(resource_kind kind) const noexcept -> bool {
        return _kind == kind;
    }

    auto is_finished() const noexcept -> bool {
        return is(resource_kind::finished);
    }

    auto continuation() const noexcept
      -> std::shared_ptr<pending_resource_info> {
        return _continuation.lock();
    }

    void set_continuation(
      const std::shared_ptr<pending_resource_info>& cont) noexcept {
        _continuation = cont;
    }

    auto loader() noexcept -> resource_loader& {
        return _parent;
    }

    void mark_loaded() noexcept;
    void mark_finished() noexcept;
    auto is_done() const noexcept -> bool;

    void add_label(const string_view) noexcept;

    void add_valtree_stream_input(
      valtree::value_tree_stream_input input) noexcept;

    void handle_float_vector(
      const pending_resource_info& source,
      std::vector<float>& values) noexcept;

    void handle_vec3_vector(
      const pending_resource_info& source,
      std::vector<math::vector<float, 3, true>>& values) noexcept;

    void add_shape_generator(std::shared_ptr<shapes::generator> gen) noexcept;
    void add_gl_shape_context(video_context&) noexcept;
    void add_gl_geometry_and_bindings_context(
      video_context&,
      oglplus::vertex_attrib_bindings,
      shapes::drawing_variant) noexcept;

    void add_gl_shader_context(video_context&, oglplus::shader_type) noexcept;
    void add_gl_program_context(video_context&) noexcept;
    auto add_gl_program_shader_request(identifier_t request_id) noexcept
      -> bool;
    auto add_gl_program_input_binding(
      std::string name,
      shapes::vertex_attrib_variant) noexcept -> bool;

    void handle_gl_texture_i_param(
      const oglplus::texture_parameter param,
      const oglplus::gl_types::int_type value) noexcept;
    void handle_gl_texture_image(
      const oglplus::texture_target,
      const resource_gl_texture_image_params&,
      const memory::const_block) noexcept;
    auto add_gl_texture_image_request(identifier_t request_id) noexcept -> bool;
    void add_gl_texture_update_context(
      video_context&,
      oglplus::texture_target,
      oglplus::texture_unit,
      oglplus::texture_name) noexcept;

    void add_gl_texture_context(
      video_context&,
      oglplus::texture_target,
      oglplus::texture_unit) noexcept;
    auto handle_gl_texture_params(const resource_gl_texture_params&) noexcept
      -> bool;

    void add_gl_buffer_context(video_context&, oglplus::buffer_target) noexcept;
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
      const msgbus::blob_info&,
      const pending_resource_info& source,
      const span_size_t offset,
      const memory::span<const memory::const_block> data) noexcept;

    void handle_source_finished(const pending_resource_info&) noexcept;
    void handle_source_cancelled(const pending_resource_info&) noexcept;

private:
    friend class resource_loader;
    friend class resource_request_result;

    // the pending resource state
    struct _pending_valtree_traversal_state {
        valtree::value_tree_stream_input input;
    };

    struct _pending_shape_generator_state {
        std::shared_ptr<shapes::generator> generator;
    };

    struct _pending_gl_shape_state {
        std::reference_wrapper<video_context> video;
    };

    struct _pending_gl_geometry_and_bindings_state {
        std::reference_wrapper<video_context> video;
        oglplus::vertex_attrib_bindings bindings;
        span_size_t draw_var_idx;
    };

    struct _pending_gl_shader_state {
        std::reference_wrapper<video_context> video;
        oglplus::shader_type shdr_type;
    };

    struct _pending_gl_program_state {
        std::reference_wrapper<video_context> video;
        oglplus::owned_program_name prog;
        oglplus::program_input_bindings input_bindings;
        flat_set<identifier_t> pending_requests;
        bool loaded{false};
    };

    struct _pending_gl_texture_update_state {
        std::reference_wrapper<video_context> video;
        oglplus::texture_target tex_target;
        oglplus::texture_unit tex_unit;
        oglplus::texture_name tex;
    };

    struct _pending_gl_texture_state {
        std::reference_wrapper<video_context> video;
        const resource_gl_texture_params* pparams{nullptr};
        flat_set<identifier_t> pending_requests;
        std::bitset<32> level_images_done;
        span_size_t levels{0};
        oglplus::texture_target tex_target;
        oglplus::texture_unit tex_unit;
        oglplus::owned_texture_name tex;
        bool loaded{false};
    };

    struct _pending_gl_buffer_update_state {
        std::reference_wrapper<video_context> video;
        oglplus::buffer_target buf_target;
        oglplus::buffer_name buf;
    };

    struct _pending_gl_buffer_state {
        std::reference_wrapper<video_context> video;
        oglplus::buffer_target buf_target;
        oglplus::owned_buffer_name buf;
        flat_set<identifier_t> pending_requests;
        bool loaded{false};
    };

    // resource loaded handlers
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

    void _handle_vec3_vector(
      const pending_resource_info& source,
      const std::vector<math::vector<float, 3, true>>& values) noexcept;

    auto _apply_shape_modifiers(std::shared_ptr<shapes::generator>) noexcept
      -> std::shared_ptr<shapes::generator>;
    void _handle_shape_generator(
      const pending_resource_info& source,
      const std::shared_ptr<shapes::generator>& gen) noexcept;
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
    void _handle_gl_texture_image(
      const pending_resource_info& source,
      const oglplus::texture_target,
      const resource_gl_texture_image_params&,
      const memory::const_block) noexcept;

    auto _finish_gl_buffer(_pending_gl_buffer_state&) noexcept -> bool;
    void _handle_gl_buffer_data(
      const pending_resource_info& source,
      const oglplus::buffer_target,
      const resource_gl_buffer_data_params&,
      const memory::const_block) noexcept;

    resource_loader& _parent;
    const identifier_t _request_id;
    std::weak_ptr<pending_resource_info> _continuation{};
    const url _locator;

    std::variant<
      std::monostate,
      _pending_valtree_traversal_state,
      _pending_shape_generator_state,
      _pending_gl_shape_state,
      _pending_gl_geometry_and_bindings_state,
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
class valtree_builder_common {
public:
    valtree_builder_common(
      const std::shared_ptr<pending_resource_info>& info) noexcept
      : _parent{info} {}

    auto log(log_event_severity severity, const string_view format) noexcept
      -> log_entry;

    auto log_error(const string_view format) noexcept {
        return log(log_event_severity::error, format);
    }

    auto log_info(const string_view format) noexcept {
        return log(log_event_severity::info, format);
    }

    auto log_debug(const string_view format) noexcept {
        return log(log_event_severity::debug, format);
    }

protected:
    std::weak_ptr<pending_resource_info> _parent;
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
            extract(parent).mark_loaded();
            return true;
        }
        return false;
    }

    void failed() noexcept override {
        if(auto parent{_parent.lock()}) {
            extract(parent).mark_loaded();
        }
    }
};
//------------------------------------------------------------------------------
auto make_valtree_float_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> std::unique_ptr<valtree::object_builder>;
auto make_valtree_vec3_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> std::unique_ptr<valtree::object_builder>;
auto make_valtree_camera_parameters_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  orbiting_camera&) noexcept -> std::unique_ptr<valtree::object_builder>;
auto make_valtree_input_setup_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  execution_context&) noexcept -> std::unique_ptr<valtree::object_builder>;
//------------------------------------------------------------------------------
auto make_valtree_gl_program_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  video_context& video) noexcept -> std::unique_ptr<valtree::object_builder>;
auto make_valtree_gl_texture_image_loader(
  const std::shared_ptr<pending_resource_info>& parent,
  oglplus::texture_target,
  const resource_gl_texture_image_params& params) noexcept
  -> std::unique_ptr<valtree::object_builder>;
auto make_valtree_gl_texture_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  video_context& video,
  oglplus::texture_target,
  oglplus::texture_unit) noexcept -> std::unique_ptr<valtree::object_builder>;
auto make_valtree_gl_buffer_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  video_context& video,
  oglplus::buffer_target) noexcept -> std::unique_ptr<valtree::object_builder>;
//------------------------------------------------------------------------------
/// @brief Result of resource request operation.
/// @see resource_kind
export class resource_request_result {
public:
    resource_request_result(
      std::shared_ptr<pending_resource_info> info,
      bool cancelled) noexcept
      : _info{std::move(info)}
      , _was_cancelled{cancelled} {}

    resource_request_result(
      const std::pair<identifier_t, std::shared_ptr<pending_resource_info>>& p,
      bool cancelled) noexcept
      : resource_request_result{std::get<1>(p), cancelled} {}

    /// @brief Indicates if the request is valid.
    explicit operator bool() const noexcept {
        return not _was_cancelled;
    }

    auto info() const noexcept -> pending_resource_info&;

    operator std::shared_ptr<pending_resource_info>() const noexcept {
        return _info;
    }

    /// @brief Returns a reference to the parent loader.
    auto loader() const noexcept -> resource_loader& {
        return info().loader();
    }

    /// @brief Returns the unique id of the request.
    auto request_id() const noexcept -> identifier_t {
        return extract(_info).request_id();
    }

    /// @brief Returns the locator of the requested resource.
    auto locator() const noexcept -> const url& {
        return info()._locator;
    }

    /// @brief Sets the reference to the continuation request of this request.
    auto set_continuation(const std::shared_ptr<pending_resource_info>& cont)
      const noexcept -> const resource_request_result& {
        info().set_continuation(cont);
        return *this;
    }

    /// @brief Sets the reference to the continuation request of this request.
    auto set_continuation(resource_request_result& cont) const noexcept
      -> const resource_request_result& {
        info().set_continuation(cont._info);
        return *this;
    }

private:
    std::shared_ptr<pending_resource_info> _info;
    bool _was_cancelled;
};
//------------------------------------------------------------------------------
/// @brief Collection of signals emitted by the resource loader.
/// @see resource_loader
export struct resource_loader_signals {
    /// @brief Emitted then the loading of a resource is cancelled.
    signal<void(identifier_t, resource_kind, const url&) noexcept>
      resource_cancelled;

    /// @brief Emitted when any resource is successfully loaded
    signal<void(identifier_t, resource_kind, const url&) noexcept>
      resource_loaded;

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
        const std::shared_ptr<shapes::generator>& generator;
    };

    template <>
    struct get_load_info<std::shared_ptr<shapes::generator>>
      : std::type_identity<shape_generator_load_info> {};

    /// @brief Emitted when a shape generator is loaded.
    signal<void(const shape_generator_load_info&) noexcept>
      shape_generator_loaded;

    auto load_signal(
      std::type_identity<std::shared_ptr<shapes::generator>>) noexcept
      -> auto& {
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

    auto load_signal(std::type_identity<oglplus::shape_generator>) noexcept
      -> auto& {
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

    auto load_signal(std::type_identity<gl_geometry_and_bindings>) noexcept
      -> auto& {
        return gl_geometry_and_bindings_loaded;
    }

    /// @brief Type of parameter of the value_tree_loaded signal.
    /// @see value_tree_loaded
    struct value_tree_load_info {
        const identifier_t request_id;
        const url& locator;
        const valtree::compound& tree;

        /// @brief Loads the data from the value tree resource into an object.
        auto load(default_mapped_struct auto& object) const noexcept -> bool {
            valtree_deserializer_backend backend{tree};
            const auto errors{deserialize(object, backend)};
            return not errors;
        }

        /// @brief Loads the data from the value tree resource into an object.
        auto load(
          default_mapped_struct auto& object,
          const basic_string_path& path) const noexcept -> bool {
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

    auto load_signal(std::type_identity<valtree::compound>) noexcept -> auto& {
        return value_tree_loaded;
    }

    /// @brief Type of parameter of the float_vector_loaded signal.
    /// @see float_vector_loaded
    struct float_vector_load_info {
        const identifier_t request_id;
        const url& locator;
        std::vector<float> values;
    };

    template <>
    struct get_load_info<std::vector<float>>
      : std::type_identity<float_vector_load_info> {};

    /// @brief Emitted when a vector of floating-point values is loaded.
    signal<void(const float_vector_load_info&) noexcept> float_vector_loaded;

    auto load_signal(std::type_identity<std::vector<float>>) noexcept -> auto& {
        return float_vector_loaded;
    }

    /// @brief Type of parameter of the vec3_vector_loaded signal.
    /// @see vec3_vector_loaded
    struct vec3_vector_load_info {
        const identifier_t request_id;
        const url& locator;
        std::vector<math::vector<float, 3, true>>& values;
    };

    template <>
    struct get_load_info<std::vector<math::vector<float, 3, true>>>
      : std::type_identity<vec3_vector_load_info> {};

    /// @brief Emitted when a vector of vec3 values is loaded.
    signal<void(const vec3_vector_load_info&) noexcept> vec3_vector_loaded;

    auto load_signal(
      std::type_identity<std::vector<math::vector<float, 3, true>>>) noexcept
      -> auto& {
        return vec3_vector_loaded;
    }

    /// @brief Type of parameter of the smooth_vec3_curve_loaded signal.
    /// @see smooth_vec3_curve_loaded
    struct smooth_vec3_curve_load_info {
        const identifier_t request_id;
        const url& locator;
        math::cubic_bezier_curves<math::vector<float, 3, true>, float>& curve;
    };

    template <>
    struct get_load_info<
      math::cubic_bezier_curves<math::vector<float, 3, true>, float>>
      : std::type_identity<smooth_vec3_curve_load_info> {};

    /// @brief Emitted when a smooth closed loop of vec3 values is loaded.
    signal<void(const smooth_vec3_curve_load_info&) noexcept>
      smooth_vec3_curve_loaded;

    auto load_signal(
      std::type_identity<
        math::cubic_bezier_curves<math::vector<float, 3, true>, float>>) noexcept
      -> auto& {
        return smooth_vec3_curve_loaded;
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

    /// @brief Type of parameter of the gl_shader_loaded signal.
    /// @see gl_shader_loaded
    struct gl_shader_load_info {
        const identifier_t request_id;
        const url& locator;
        video_context& video;
        const oglplus::shader_type type;
        const oglplus::shader_name name;
        oglplus::owned_shader_name& ref;

        auto gl_api() const noexcept -> const oglplus::gl_api&;
    };

    template <>
    struct get_load_info<oglplus::owned_shader_name>
      : std::type_identity<gl_shader_load_info> {};

    /// @brief Emitted when a GL shader is successfully created and compiled.
    signal<void(const gl_shader_load_info&) noexcept> gl_shader_loaded;

    auto load_signal(std::type_identity<oglplus::owned_shader_name>) noexcept
      -> auto& {
        return gl_shader_loaded;
    }

    /// @brief Type of parameter of the gl_program_loaded signal.
    /// @see gl_program_loaded
    struct gl_program_load_info {
        const identifier_t request_id;
        const url& locator;
        video_context& video;
        const oglplus::program_name name;
        oglplus::owned_program_name& ref;
        oglplus::program_input_bindings& input_bindings;

        auto gl_api() const noexcept -> const oglplus::gl_api&;

        auto apply_input_bindings(
          const oglplus::vertex_attrib_bindings& attrib_bindings) const noexcept
          -> bool {
            return input_bindings.apply(this->gl_api(), name, attrib_bindings);
        }

        auto use_program() const noexcept {
            return this->gl_api().use_program(name);
        }

        auto get_uniform_location(const string_view var_name) const noexcept {
            return this->gl_api().get_uniform_location(name, var_name);
        }

        template <typename T>
        auto set_uniform(oglplus::uniform_location loc, const T& value)
          const noexcept {
            return this->gl_api().set_uniform(name, loc, value);
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
            return this->gl_api().get_shader_storage_block_index(
              name, var_name);
        }

        auto shader_storage_block_binding(
          oglplus::shader_storage_block_index index,
          oglplus::gl_types::uint_type binding) const noexcept {
            return this->gl_api().shader_storage_block_binding(
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

    auto load_signal(std::type_identity<oglplus::owned_program_name>) noexcept
      -> auto& {
        return gl_program_loaded;
    }

    /// @brief Type of parameter of the gl_texture_loaded signal.
    /// @see gl_texture_loaded
    struct gl_texture_load_info {
        const identifier_t request_id;
        const url& locator;
        video_context& video;
        const oglplus::texture_target target;
        const oglplus::texture_name name;
        oglplus::owned_texture_name& ref;

        auto gl_api() const noexcept -> const oglplus::gl_api&;

        template <typename Param, typename Value>
        auto parameter_f(Param param, Value value) const noexcept {
            return gl_api().tex_parameter_f(target, param, value);
        }

        template <typename Param, typename Value>
        auto parameter_i(Param param, Value value) const noexcept {
            return gl_api().tex_parameter_i(target, param, value);
        }

        auto generate_mipmap() const noexcept {
            return gl_api().operations().generate_mipmap(target);
        }
    };

    template <>
    struct get_load_info<oglplus::owned_texture_name>
      : std::type_identity<gl_texture_load_info> {};

    /// @brief Emitted when a GL texture is successfully created and its parameters set-up.
    signal<void(const gl_texture_load_info&) noexcept> gl_texture_loaded;

    auto load_signal(std::type_identity<oglplus::owned_texture_name>) noexcept
      -> auto& {
        return gl_texture_loaded;
    }

    /// @brief Type of parameter of the gl_texture_images_loaded signal.
    /// @see gl_texture_image_loaded
    struct gl_texture_images_load_info {
        const identifier_t request_id;
        const url& locator;
        video_context& video;
        const oglplus::texture_name name;

        auto gl_api() const noexcept -> const oglplus::gl_api&;
    };

    /// @brief Emitted when all GL texture images are successfully loaded.
    signal<void(const gl_texture_images_load_info&) noexcept>
      gl_texture_images_loaded;

    /// @brief Type of parameter of the gl_buffer_loaded signal.
    /// @see gl_buffer_loaded
    struct gl_buffer_load_info {
        const identifier_t request_id;
        const url& locator;
        video_context& video;
        const oglplus::buffer_name name;
        oglplus::owned_buffer_name& ref;

        auto gl_api() const noexcept -> const oglplus::gl_api&;
    };

    template <>
    struct get_load_info<oglplus::owned_buffer_name>
      : std::type_identity<gl_buffer_load_info> {};

    /// @brief Emitted when a GL buffer is successfully created and set-up.
    signal<void(const gl_buffer_load_info&) noexcept> gl_buffer_loaded;

    auto load_signal(std::type_identity<oglplus::owned_buffer_name>) noexcept
      -> auto& {
        return gl_buffer_loaded;
    }
};
//------------------------------------------------------------------------------
template <typename T>
concept resource_cancelled_observer = requires(
  T v,
  identifier_t request_id,
  resource_kind kind,
  const url& locator) {
    v.handle_resource_cancelled(request_id, kind, locator);
};

template <typename T>
concept resource_blob_stream_data_appended_observer = requires(
  T v,
  identifier_t request_id,
  const span_size_t offset,
  const memory::span<const memory::const_block> data,
  const msgbus::blob_info& binfo) {
    v.handle_blob_stream_data_appended(request_id, offset, data, binfo);
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
export class resource_loader
  : public msgbus::resource_data_consumer_node
  , public resource_loader_signals {
    using base = msgbus::resource_data_consumer_node;

public:
    /// @brief Initializing constructor
    resource_loader(msgbus::endpoint& bus)
      : resource_data_consumer_node{bus} {
        _init();
    }

    template <typename O>
    void connect_observer(O& observer) noexcept {
        if constexpr(resource_blob_stream_data_appended_observer<O>) {
            connect<&O::handle_blob_stream_data_appended>(
              &observer, this->blob_stream_data_appended);
        }
        if constexpr(resource_cancelled_observer<O>) {
            connect<&O::handle_resource_cancelled>(
              &observer, this->resource_cancelled);
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
    auto has_pending_requests() const noexcept -> bool {
        return not _pending.empty();
    }

    void forget_resource(identifier_t request_id) noexcept;

    /// @brief Does some work and updates internal state (should be called periodically).
    auto update() noexcept -> work_done;

    /// @brief Requests a float vector resource.
    auto request_float_vector(url locator) noexcept -> resource_request_result;

    auto request(std::type_identity<std::vector<float>>, url locator) noexcept {
        return request_float_vector(std::move(locator));
    }

    /// @brief Requests a float vector resource.
    auto request_vec3_vector(url locator) noexcept -> resource_request_result;

    auto request(
      std::type_identity<std::vector<math::vector<float, 3, true>>>,
      url locator) noexcept {
        return request_vec3_vector(std::move(locator));
    }

    /// @brief Requests a smoothe vec3 curve resource.
    auto request_smooth_vec3_curve(url locator) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<
        math::cubic_bezier_curves<math::vector<float, 3, true>, float>>,
      url locator) noexcept {
        return request_smooth_vec3_curve(std::move(locator));
    }

    /// @brief Requests a value tree object resource.
    auto request_value_tree(url locator) noexcept -> resource_request_result;

    auto request(std::type_identity<valtree::compound>, url locator) noexcept {
        return request_value_tree(std::move(locator));
    }

    auto request_json_traversal(
      url locator,
      std::shared_ptr<valtree::value_tree_visitor>,
      span_size_t max_token_size) noexcept -> resource_request_result;

    auto request_json_traversal(
      url locator,
      std::shared_ptr<valtree::object_builder>) noexcept
      -> resource_request_result;

    /// @brief Requests a value tree object traversal by the specified visitor.
    auto request_value_tree_traversal(
      url locator,
      std::shared_ptr<valtree::value_tree_visitor>,
      span_size_t max_token_size) noexcept -> resource_request_result;

    /// @brief Requests a value tree object traversal by the specified builder.
    auto request_value_tree_traversal(
      url locator,
      std::shared_ptr<valtree::object_builder>) noexcept
      -> resource_request_result;

    /// @brief Requests camera parameters.
    auto request_camera_parameters(url locator, orbiting_camera&) noexcept
      -> resource_request_result;

    /// @brief Requests user input setup.
    auto request_input_setup(url locator, execution_context&) noexcept
      -> resource_request_result;

    /// @brief Requests a shape geometry generator / loader object.
    auto request_shape_generator(url locator) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<std::shared_ptr<shapes::generator>>,
      url locator) noexcept {
        return request_shape_generator(std::move(locator));
    }

    /// @brief Requests a oglplus shape generator wrapper object.
    auto request_gl_shape(url locator, video_context&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<oglplus::shape_generator>,
      url locator,
      video_context& video) noexcept {
        return request_gl_shape(std::move(locator), video);
    }

    /// @brief Requests a shape geometry and attrib bindings object.
    auto request_gl_geometry_and_bindings(
      url locator,
      video_context&,
      oglplus::vertex_attrib_bindings,
      span_size_t draw_var_idx = 0) noexcept -> resource_request_result;

    auto request(
      std::type_identity<gl_geometry_and_bindings>,
      url locator,
      video_context& video,
      oglplus::vertex_attrib_bindings bindings,
      span_size_t draw_var_idx = 0) noexcept {
        return request_gl_geometry_and_bindings(
          std::move(locator), video, bindings, draw_var_idx);
    }

    /// @brief Requests a shape geometry and attrib bindings object.
    auto request_gl_geometry_and_bindings(
      url locator,
      video_context&,
      span_size_t draw_var_idx = 0) noexcept -> resource_request_result;

    auto request(
      std::type_identity<gl_geometry_and_bindings>,
      url locator,
      video_context& video,
      span_size_t draw_var_idx = 0) noexcept {
        return request_gl_geometry_and_bindings(
          std::move(locator), video, draw_var_idx);
    }

    /// @brief Requests GLSL shader source code resource.
    auto request_glsl_source(url locator) noexcept -> resource_request_result;

    /// @brief Requests a compiled GL shader object of a specified type.
    auto request_gl_shader(
      url locator,
      video_context&,
      oglplus::shader_type) noexcept -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_shader_name>,
      url locator,
      video_context& video,
      oglplus::shader_type shdr_type) noexcept {
        return request_gl_shader(std::move(locator), video, shdr_type);
    }

    /// @brief Requests a compiled GL shader object of a type specified in URL.
    auto request_gl_shader(url locator, video_context&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_shader_name>,
      url locator,
      video_context& video) noexcept {
        return request_gl_shader(std::move(locator), video);
    }

    /// @brief Requests a linked GL program object.
    auto request_gl_program(url locator, video_context&) noexcept
      -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_program_name>,
      url locator,
      video_context& video) noexcept {
        return request_gl_program(std::move(locator), video);
    }

    auto request_gl_texture_image(
      url locator,
      oglplus::texture_target,
      const resource_gl_texture_image_params&) noexcept
      -> resource_request_result;

    auto request_gl_texture_image(url locator, oglplus::texture_target) noexcept
      -> resource_request_result;

    /// @brief Requests image data update for a GL texture.
    auto request_gl_texture_update(
      url locator,
      video_context&,
      oglplus::texture_target,
      oglplus::texture_unit,
      oglplus::texture_name) noexcept -> resource_request_result;

    /// @brief Requests a set-up GL texture object.
    auto request_gl_texture(
      url locator,
      video_context&,
      oglplus::texture_target,
      oglplus::texture_unit) noexcept -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_texture_name>,
      url locator,
      video_context& video,
      oglplus::texture_target tex_target,
      oglplus::texture_unit tex_unit) noexcept {
        return request_gl_texture(
          std::move(locator), video, tex_target, tex_unit);
    }

    /// @brief Requests a set-up GL buffer object.
    auto request_gl_buffer(
      url locator,
      video_context&,
      oglplus::buffer_target) noexcept -> resource_request_result;

    auto request(
      std::type_identity<oglplus::owned_buffer_name>,
      url locator,
      video_context& video,
      oglplus::buffer_target buf_target) noexcept {
        return request_gl_buffer(std::move(locator), video, buf_target);
    }

private:
    friend class pending_resource_info;

    auto _is_json_resource(const url& locator) const noexcept -> bool;

    void _init() noexcept;

    void _handle_stream_data_appended(
      const identifier_t blob_id,
      const span_size_t offset,
      const memory::span<const memory::const_block>,
      const msgbus::blob_info& info) noexcept;

    void _handle_stream_finished(identifier_t blob_id) noexcept;
    void _handle_stream_cancelled(identifier_t blob_id) noexcept;

    auto _cancelled_resource(
      const identifier_t blob_id,
      url& locator,
      const resource_kind) noexcept -> resource_request_result;

    auto _cancelled_resource(url& locator, const resource_kind kind) noexcept
      -> resource_request_result {
        return _cancelled_resource(get_request_id(), locator, kind);
    }

    auto _cancelled_resource(url& locator) noexcept -> resource_request_result {
        return _cancelled_resource(locator, resource_kind::unknown);
    }

    auto _new_resource(
      const identifier_t blob_id,
      url locator,
      resource_kind) noexcept -> resource_request_result;

    auto _new_resource(
      const std::pair<identifier_t, const url&>& id_and_loc,
      resource_kind kind) noexcept -> resource_request_result {
        return _new_resource(
          std::get<0>(id_and_loc), std::get<1>(id_and_loc), kind);
    }

    auto _new_resource(url locator, resource_kind kind) noexcept
      -> resource_request_result {
        return _new_resource(get_request_id(), std::move(locator), kind);
    }

    flat_map<identifier_t, std::shared_ptr<pending_resource_info>> _pending;
    flat_map<identifier_t, std::shared_ptr<pending_resource_info>> _finished;
    flat_map<identifier_t, std::shared_ptr<pending_resource_info>> _cancelled;
};
//------------------------------------------------------------------------------
inline auto valtree_builder_common::log(
  log_event_severity severity,
  const string_view format) noexcept -> log_entry {
    if(auto parent{_parent.lock()}) [[likely]] {
        return extract(parent).loader().log(severity, format);
    }
    return {{}, {}, severity, {}, nullptr};
}
//------------------------------------------------------------------------------
/// @brief Class tracking specific pending resource load requests.
/// @see resource_loader
/// @see resource_request_result
export class pending_resource_requests {
public:
    /// @brief Construction from a reference to resource_loader.
    pending_resource_requests(resource_loader& loader) noexcept
      : _sig_bind{loader.resource_loaded.bind(
          make_callable_ref<&pending_resource_requests::_handle_loaded>(
            this))} {}

    /// @brief Construction from a reference to execution_context.
    pending_resource_requests(execution_context&) noexcept;

    /// @brief Adds a new resource request to be tracked.
    auto add(const resource_request_result& res) noexcept
      -> pending_resource_requests& {
        _request_ids.insert(res.request_id());
        return *this;
    }

    /// @brief Returns the number of currently pending resource requests.
    auto pending_count() const noexcept -> span_size_t {
        return span_size(_request_ids.size());
    }

    /// @brief Indicates if all the tracked requests finished loading.
    auto all_are_loaded() const noexcept -> bool {
        return _request_ids.empty();
    }

    /// @brief Indicates if all the tracked requests finished loading.
    /// @see all_are_loaded
    explicit operator bool() const noexcept {
        return all_are_loaded();
    }

private:
    void _handle_loaded(
      const identifier_t request_id,
      resource_kind,
      const url&) noexcept {
        _request_ids.erase(request_id);
    }

    signal_binding _sig_bind;
    flat_set<identifier_t> _request_ids;
};
//------------------------------------------------------------------------------
} // namespace eagine::app

