/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import eagine.shapes;
import eagine.eglplus;
import eagine.oglplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// Renderer
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_renderer_base : public eagitexi_cubemap_renderer {
protected:
    eagitexi_cubemap_blur_renderer_base(
      gl_rendered_source_blob_io& parent,
      const gl_rendered_blob_params& params,
      shared_holder<gl_rendered_blob_context> context,
      int size) noexcept
      : eagitexi_cubemap_renderer{
          parent,
          "blurring cube-map",
          params,
          std::move(context),
          size,
          _tile_size(parent)} {}

private:
    static auto _tile_size(gl_rendered_source_blob_io& parent) noexcept -> int;
};
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_renderer_base::_tile_size(
  gl_rendered_source_blob_io& p) noexcept -> int {
    if(const auto size{p.app_config().get<int>(
         "application.resource_provider.cubemap_blur.tile_size")}) {
        return *size;
    }
    return 16;
}
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_renderer final
  : public eagitexi_cubemap_blur_renderer_base {
public:
    eagitexi_cubemap_blur_renderer(
      gl_rendered_source_blob_io& parent,
      const gl_rendered_blob_params& params,
      shared_holder<gl_rendered_blob_context> context,
      url source,
      int size,
      int sharpness) noexcept;

    ~eagitexi_cubemap_blur_renderer() noexcept final;

    auto prepare_render() noexcept -> msgbus::blob_preparation_result final;

private:
    static auto _vs_source() noexcept -> string_view;
    static auto _fs_source() noexcept -> string_view;
    auto _build_program(const gl_rendered_blob_params&, int) noexcept
      -> oglplus::program_object;
    void _on_tex_loaded(const gl_texture_resource::load_info&) noexcept;

    gl_texture_resource _cubemap;

    const signal_binding _sig_binding{
      _cubemap.loaded.bind_to<&eagitexi_cubemap_blur_renderer::_on_tex_loaded>(
        this)};
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_renderer::eagitexi_cubemap_blur_renderer(
  gl_rendered_source_blob_io& parent,
  const gl_rendered_blob_params& params,
  shared_holder<gl_rendered_blob_context> context,
  url source,
  int size,
  int sharpness) noexcept
  : eagitexi_cubemap_blur_renderer_base{parent, params, std::move(context), size}
  , _cubemap{std::move(source), resource_context()} {
    _init_program(_build_program(params, sharpness));
}
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_renderer::~eagitexi_cubemap_blur_renderer() noexcept {
    _cubemap.clean_up(resource_context());
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_renderer::_build_program(
  const gl_rendered_blob_params& params,
  int sharpness) noexcept -> oglplus::program_object {
    const auto& glapi{gl_api()};
    const auto& [gl, GL]{glapi};

    // program
    auto prog{glapi.create_program_object()};
    glapi.add_shader(prog, GL.vertex_shader, embedded<"iCmBlurVS">());
    glapi.add_shader(prog, GL.fragment_shader, embedded<"iCmBlurFS">());
    gl.link_program(prog);
    gl.use_program(prog);

    gl.bind_attrib_location(prog, _screen_position_loc(), "Position");
    glapi.try_set_uniform(
      prog,
      "cubeSide",
      std::min(
        std::max(params.surface_width.value(), params.surface_height.value()),
        128));

    glapi.try_set_uniform(prog, "sharpness", sharpness);
    glapi.try_set_uniform(prog, "cubeMap", 0);

    return prog;
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_renderer::_on_tex_loaded(
  const gl_texture_resource::load_info& loaded) noexcept {
    const auto& GL{gl_api().constants()};
    loaded.parameter_i(GL.texture_min_filter, GL.linear);
    loaded.parameter_i(GL.texture_mag_filter, GL.linear);
    loaded.parameter_i(GL.texture_wrap_s, GL.clamp_to_edge);
    loaded.parameter_i(GL.texture_wrap_t, GL.clamp_to_edge);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_renderer::prepare_render() noexcept
  -> msgbus::blob_preparation_result {
    const auto& GL{gl_api().constants()};
    if(_cubemap.load_if_needed(
         resource_context(), GL.texture_cube_map, GL.texture0)) {
        return {msgbus::blob_preparation_status::working};
    }
    return msgbus::blob_preparation_result::finished();
}
//------------------------------------------------------------------------------
// I/O
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_io final : public gl_rendered_source_blob_io {
public:
    eagitexi_cubemap_blur_io(
      main_ctx_parent,
      const shared_provider_objects& shared,
      const gl_rendered_blob_params& params,
      url source,
      int size,
      int sharpness,
      int level) noexcept;

protected:
    auto make_renderer(shared_holder<gl_rendered_blob_context>) noexcept
      -> unique_holder<gl_blob_renderer> final;

private:
    void _make_header_bgn(int size, int level) noexcept;
    void _make_header_end(eagitexi_cubemap_blur_renderer&) noexcept;

    const url _source;
    const int _size;
    const int _sharpness;
};
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_io::_make_header_bgn(int size, int level) noexcept {
    std::stringstream hdr;
    hdr << R"({"level":)" << level;
    hdr << R"(,"width":)" << size;
    hdr << R"(,"height":)" << size;
    hdr << R"(,"depth":)" << 6;
    hdr << R"(,"channels":4)";
    hdr << R"(,"data_type":"unsigned_byte")";
    hdr << R"(,"format":"rgba")";
    hdr << R"(,"iformat":"rgba8")";
    hdr << R"(,"tag":["blur","cubemap"])";
    hdr << R"(,"data_filter":"zlib")";
    append(hdr.str());
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_io::_make_header_end(
  eagitexi_cubemap_blur_renderer& renderer) noexcept {
    std::stringstream hdr;

    if(const auto vendor_name{renderer.renderer_name()}) {
        hdr << R"(,"metadata":{"renderer":{)";
        hdr << R"("vendor":")" << *vendor_name << R"(")";
        if(const auto renderer_name{renderer.renderer_name()}) {
            hdr << R"(,"name":")" << *renderer_name << R"(")";
        }
        if(const auto version{renderer.version()}) {
            hdr << R"(,"version":")" << *version << R"(")";
        }
        if(const auto driver_name{renderer.driver_name()}) {
            hdr << R"(,"driver":")" << *driver_name << R"(")";
        }
        hdr << R"(}})";
    }
    hdr << "}";
    append(hdr.str());
}
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_io::eagitexi_cubemap_blur_io(
  main_ctx_parent parent,
  const shared_provider_objects& shared,
  const gl_rendered_blob_params& params,
  url source,
  int size,
  int sharpness,
  int level) noexcept
  : gl_rendered_source_blob_io{"ITxCubBlur", parent, shared, params, size * size * 6}
  , _source{std::move(source)}
  , _size{size}
  , _sharpness{sharpness} {
    _make_header_bgn(size, level);
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_io::make_renderer(
  shared_holder<gl_rendered_blob_context> context) noexcept
  -> unique_holder<gl_blob_renderer> {
    unique_holder<eagitexi_cubemap_blur_renderer> renderer{
      default_selector,
      *this,
      params(),
      std::move(context),
      _source,
      _size,
      _sharpness};

    assert(renderer);
    _make_header_end(*renderer);

    return renderer;
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class eagitexi_cubemap_blur_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagitexi_cubemap_blur_provider(const provider_parameters&) noexcept;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> shared_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t size) noexcept
      -> std::chrono::seconds final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    const shared_provider_objects& _shared;
    application_config_value<int> _device_index{
      main_context().config(),
      "application.resource_provider.cubemap_blur.device_index",
      -1};
    application_config_value<std::chrono::seconds> _blob_timeout{
      main_context().config(),
      "application.resource_provider.cubemap_blur.blob_timeout",
      std::chrono::seconds{900}};
};
//------------------------------------------------------------------------------
eagitexi_cubemap_blur_provider::eagitexi_cubemap_blur_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PTxCubBlur", params.parent}
  , _shared{params.shared} {}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::has_resource(const url& locator) noexcept
  -> bool {
    if(
      is_valid_eagitexi_resource_url(locator) and
      locator.has_path("cube_map_blur")) {
        return is_valid_eagitex_resource_url(locator.query().arg_url("source"));
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::get_resource_io(const url& locator)
  -> shared_holder<msgbus::source_blob_io> {
    if(has_resource(locator)) {
        const auto& q{locator.query()};
        const auto size{q.arg_value_as<int>("size").value_or(1024)};
        const auto sharpness{q.arg_value_as<int>("sharpness").value_or(8)};
        const auto level{q.arg_value_as<int>("level").value_or(0)};

        gl_rendered_blob_params params{
          .device_index = _device_index.value(),
          .surface_width = size,
          .surface_height = size};

        q.arg_value_as<int>("device_index")
          .and_then(_1.assign_to(params.device_index));

        return {
          hold<eagitexi_cubemap_blur_io>,
          as_parent(),
          _shared,
          params,
          q.arg_url("source"),
          size,
          sharpness,
          level};
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitexi_cubemap_blur_provider::get_blob_timeout(const span_size_t) noexcept
  -> std::chrono::seconds {
    return adjusted_duration(_blob_timeout.value());
}
//------------------------------------------------------------------------------
void eagitexi_cubemap_blur_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitexi:///cube_map_blur");
}
//------------------------------------------------------------------------------
auto provider_eagitexi_cubemap_blur(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitexi_cubemap_blur_provider>, params};
}
//------------------------------------------------------------------------------
// level blur texture I/O
//------------------------------------------------------------------------------
class eagitex_cubemap_levels_blur_io
  : public std::enable_shared_from_this<eagitex_cubemap_levels_blur_io>
  , public main_ctx_object
  , public msgbus::source_blob_io
  , public valtree::object_builder_impl<eagitex_cubemap_levels_blur_io> {

public:
    eagitex_cubemap_levels_blur_io(
      main_ctx_parent parent,
      shared_provider_objects&,
      url locator) noexcept;

    static auto valid_dim(int d) noexcept -> bool {
        return (d >= 256) and math::is_positive_power_of_2(d);
    }

    auto prepare() noexcept -> msgbus::blob_preparation_result final;

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(const span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

public:
    auto max_token_size() noexcept -> span_size_t final {
        return 256;
    }

    void do_add(
      const basic_string_path& path,
      span<const string_view> data) noexcept;

    template <std::integral T>
    void do_add(
      const basic_string_path& path,
      const span<const T> data) noexcept;

    template <typename T>
    void do_add(const basic_string_path&, const span<const T>) noexcept {}

    void unparsed_data(span<const memory::const_block> data) noexcept final;

    auto finish() noexcept -> bool final;

    void failed() noexcept final;

private:
    auto _content() const noexcept -> string_view;
    auto _make_content() const noexcept -> std::string;

    shared_provider_objects& _shared;
    const url _locator;
    const url _source_loc;
    std::string _image_loc;
    std::string _text_content;
    flat_set<std::string> _tags;
    int _levels{8};
    valid_if_positive<int> _width;
    valid_if_positive<int> _height;
    valid_if_positive<int> _channels;
    valid_if_not_empty<std::string> _data_type;
    valid_if_not_empty<std::string> _format;
    valid_if_not_empty<std::string> _iformat;
    bool _started_loading{false};
    bool _finished_loading{false};
    bool _success{true};
};
//------------------------------------------------------------------------------
// eagitex_cubemap_levels_blur_io
//------------------------------------------------------------------------------
eagitex_cubemap_levels_blur_io::eagitex_cubemap_levels_blur_io(
  main_ctx_parent parent,
  shared_provider_objects& shared,
  url locator) noexcept
  : main_ctx_object{"ITxCbLvlBl", parent}
  , _shared{shared}
  , _locator{std::move(locator)}
  , _source_loc{_locator.query().arg_url("source")} {
    _tags.insert("generated");
    _tags.insert("cubemap");
    _tags.insert("blur");
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_io::prepare() noexcept
  -> msgbus::blob_preparation_result {
    if(not _started_loading) {
        shared_holder<valtree::object_builder> self{shared_from_this()};
        const auto max_token_size{self->max_token_size()};
        if(_shared.old_loader.request_json_traversal(
             {.locator = _source_loc, .max_time = std::chrono::minutes{5}},
             valtree::make_building_value_tree_visitor(std::move(self)),
             max_token_size)) {
            _started_loading = true;
            return {msgbus::blob_preparation_status::working};
        }
        return {msgbus::blob_preparation_status::failed};
    }
    if(not _finished_loading) {
        return {msgbus::blob_preparation_status::working};
    }
    if(_success) {
        if(_text_content.empty()) {
            if(_width and _height) {
                if(valid_dim(_width.value()) and _width == _height) {
                    if(_channels and _data_type and _format and _iformat) {
                        _text_content = _make_content();
                        return {msgbus::blob_preparation_status::working};
                    }
                }
            }
            return {msgbus::blob_preparation_status::failed};
        }
    }
    return msgbus::blob_preparation_result::finished();
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_io::_make_content() const noexcept
  -> std::string {
    std::stringstream hdr;
    hdr << R"({"levels":)" << _levels;
    hdr << R"(,"width":)" << _width.value();
    hdr << R"(,"height":)" << _width.value();
    hdr << R"(,"channels":)" << _channels.value();
    hdr << R"(,"data_type":")" << _data_type.value() << R"(")";
    hdr << R"(,"format":")" << _format.value() << R"(")";
    hdr << R"(,"iformat":")" << _iformat.value() << R"(")";
    hdr << R"(,"min_filter":"linear_mipmap_linear")";
    hdr << R"(,"mag_filter":"linear")";
    hdr << R"(,"wrap_s":"clamp_to_edge")";
    hdr << R"(,"wrap_t":"clamp_to_edge")";
    hdr << R"(,"tag":[)";
    bool first_tag = true;
    for(const auto& tag : _tags) {
        if(first_tag) {
            first_tag = false;
        } else {
            hdr << ',';
        }
        hdr << '"' << tag << '"';
    }
    hdr << "]";
    hdr << R"(,"images":)";
    hdr << R"([{"url":")" << _image_loc << R"("})";

    const auto sharpness{[](int level, int) {
        switch(level) {
            case 1:
                return 20;
            case 2:
                return 16;
            case 3:
                return 8;
            case 4:
                return 4;
            case 5:
                return 2;
            case 6:
                return 1;
            default:
                break;
        }
        return 0;
    }};

    const std::string enc_img_loc{
      url::encode_component(_source_loc.get_string())};
    int size{_width.value()};
    for(int level = 1; level < _levels; ++level) {
        size /= 2;
        hdr << R"(,{"url":"eagitexi:///cube_map_blur?source=)" << enc_img_loc
            << R"(&level=)" << level << R"(&size=)" << size << R"(&sharpness=)"
            << sharpness(level, _levels) << R"("})";
    }

    hdr << "]}";
    return hdr.str();
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_io::_content() const noexcept -> string_view {
    if(_text_content.empty()) {
        return {"{}"};
    }
    return {_text_content};
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_io::total_size() noexcept -> span_size_t {
    return span_size(_content().size());
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_io::fetch_fragment(
  const span_size_t offs,
  memory::block dst) noexcept -> span_size_t {
    return copy(head(skip(view(_content()), offs), dst.size()), dst).size();
}
//------------------------------------------------------------------------------
void eagitex_cubemap_levels_blur_io::do_add(
  const basic_string_path& path,
  span<const string_view> data) noexcept {
    if(data.has_single_value()) {
        if(path.has_size(1)) {
            if(path.starts_with("data_type")) {
                _data_type = to_string(*data);
            } else if(path.starts_with("format")) {
                _format = to_string(*data);
            } else if(path.starts_with("iformat")) {
                _iformat = to_string(*data);
            }
        } else if(path.has_size(2)) {
            if(path.starts_with("tag") and path.ends_with("_")) {
                _tags.insert(to_string(*data));
            }
        } else if(path.has_size(3)) {
            if(path.starts_with("images") and path.ends_with("url")) {
                if(_image_loc.empty()) {
                    _image_loc = to_string(*data);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
template <std::integral T>
void eagitex_cubemap_levels_blur_io::do_add(
  const basic_string_path& path,
  const span<const T> data) noexcept {
    if(data.has_single_value()) {
        if(path.has_size(1)) {
            if(path.starts_with("width")) {
                _success = assign_if_fits(data, _width) and _success;
            } else if(path.starts_with("height")) {
                _success = assign_if_fits(data, _height) and _success;
            } else if(path.starts_with("channels")) {
                _success = assign_if_fits(data, _channels) and _success;
            }
        }
    }
}
//------------------------------------------------------------------------------
void eagitex_cubemap_levels_blur_io::unparsed_data(
  span<const memory::const_block> data) noexcept {}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_io::finish() noexcept -> bool {
    _finished_loading = true;
    return true;
}
//------------------------------------------------------------------------------
void eagitex_cubemap_levels_blur_io::failed() noexcept {
    _finished_loading = true;
}
//------------------------------------------------------------------------------
// levels blur texture provider
//------------------------------------------------------------------------------
class eagitex_cubemap_levels_blur_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    eagitex_cubemap_levels_blur_provider(const provider_parameters&) noexcept;

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> shared_holder<msgbus::source_blob_io> final;

    auto get_blob_timeout(const span_size_t size) noexcept
      -> std::chrono::seconds final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    shared_provider_objects& _shared;

    static auto _valid_dim(int d) noexcept -> bool {
        return eagitex_cubemap_levels_blur_io::valid_dim(d);
    }
    static auto _valid_lvls(int l) noexcept -> bool {
        return (l >= 1) and (l <= 8);
    }
};
//------------------------------------------------------------------------------
eagitex_cubemap_levels_blur_provider::eagitex_cubemap_levels_blur_provider(
  const provider_parameters& params) noexcept
  : main_ctx_object{"PTxCbLvlBl", params.parent}
  , _shared{params.shared} {}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_provider::has_resource(
  const url& locator) noexcept -> bool {
    if(
      is_valid_eagitex_resource_url(locator) and
      locator.has_path("cube_map_levels_blur")) {
        const auto& q{locator.query()};
        const bool args_ok =
          is_valid_eagitex_resource_url(q.arg_url("source")) and
          _valid_lvls(q.arg_value_as<int>("levels").value_or(1));
        return args_ok;
    }
    return false;
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_provider::get_resource_io(const url& locator)
  -> shared_holder<msgbus::source_blob_io> {
    if(has_resource(locator)) {
        return {
          hold<eagitex_cubemap_levels_blur_io>, as_parent(), _shared, locator};
    }
    return {};
}
//------------------------------------------------------------------------------
auto eagitex_cubemap_levels_blur_provider::get_blob_timeout(
  const span_size_t) noexcept -> std::chrono::seconds {
    return adjusted_duration(std::chrono::minutes{1});
}
//------------------------------------------------------------------------------
void eagitex_cubemap_levels_blur_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    callback("eagitex:///cube_map_levels_blur");
}
//------------------------------------------------------------------------------
// provider factory functions
//------------------------------------------------------------------------------
auto provider_eagitex_cubemap_levels_blur(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<eagitex_cubemap_levels_blur_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app

