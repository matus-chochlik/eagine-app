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
import eagine.core.value_tree;
import eagine.core.valid_if;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.msgbus;
import eagine.shapes;
import eagine.oglplus;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// utilities
//------------------------------------------------------------------------------
void for_each_chunk_line(
  std::string& line,
  const msgbus::blob_stream_chunk& chunks,
  auto function) {
    const string_view sep{"\n"};
    for(const auto chunk : chunks.data) {
        auto text{as_chars(chunk)};
        while(not text.empty()) {
            if(const auto pos{memory::find_position(text, sep)}) {
                append_to(head(text, *pos), line);
                text = skip(text, *pos + sep.size());
                function(std::move(line));
            } else {
                append_to(text, line);
                text = {};
            }
        }
    }
}
//------------------------------------------------------------------------------
// plain_text_resource
//------------------------------------------------------------------------------
auto plain_text_resource::kind() const noexcept -> identifier {
    return "PlainText";
}
//------------------------------------------------------------------------------
struct plain_text_resource::_loader final
  : simple_loader_of<plain_text_resource> {
    using simple_loader_of<plain_text_resource>::simple_loader_of;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;
};
//------------------------------------------------------------------------------
auto plain_text_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.fetch_resource_chunks(parameters(), 1024).first, res_loader);
}
//------------------------------------------------------------------------------
void plain_text_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    auto& text{resource()._private_ref()};
    span_size_t total_size{span_size(text.size())};
    for(const auto chunk : chunks.data) {
        total_size = safe_add(total_size, chunk.size());
    }
    text.reserve(std_size(total_size));
    for(const auto chunk : chunks.data) {
        append_to(as_chars(chunk), text);
    }
}
//------------------------------------------------------------------------------
auto plain_text_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {hold<plain_text_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// string_list_resource
//------------------------------------------------------------------------------
auto string_list_resource::kind() const noexcept -> identifier {
    return "StringList";
}
//------------------------------------------------------------------------------
struct string_list_resource::_loader final
  : simple_loader_of<string_list_resource> {
    using base = simple_loader_of<string_list_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;

    void stream_finished(identifier_t) noexcept final;

    std::string _chunk_line;
};
//------------------------------------------------------------------------------
auto string_list_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.fetch_resource_chunks(parameters(), 1024).first, res_loader);
}
//------------------------------------------------------------------------------
void string_list_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    for_each_chunk_line(_chunk_line, chunks, [&, this](auto line) {
        resource()._private_ref().emplace_back(std::move(line));
    });
}
//------------------------------------------------------------------------------
void string_list_resource::_loader::stream_finished(
  identifier_t request_id) noexcept {
    if(not _chunk_line.empty()) {
        resource()._private_ref().emplace_back(std::move(_chunk_line));
    }
    base::stream_finished(request_id);
}
//------------------------------------------------------------------------------
auto string_list_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {hold<string_list_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// url_list_resource
//------------------------------------------------------------------------------
auto url_list_resource::kind() const noexcept -> identifier {
    return "URLList";
}
//------------------------------------------------------------------------------
struct url_list_resource::_loader final : simple_loader_of<url_list_resource> {
    using base = simple_loader_of<url_list_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;

    void stream_finished(identifier_t) noexcept final;

    std::string _chunk_line;
};
//------------------------------------------------------------------------------
auto url_list_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.fetch_resource_chunks(parameters(), 1024).first, res_loader);
}
//------------------------------------------------------------------------------
void url_list_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    for_each_chunk_line(_chunk_line, chunks, [&, this](auto line) {
        if(url locator{std::move(line)}) {
            resource()._private_ref().emplace_back(std::move(locator));
        }
    });
}
//------------------------------------------------------------------------------
void url_list_resource::_loader::stream_finished(
  identifier_t request_id) noexcept {
    if(not _chunk_line.empty()) {
        if(url locator{std::move(_chunk_line)}) {
            resource()._private_ref().emplace_back(std::move(locator));
        }
    }
    base::stream_finished(request_id);
}
//------------------------------------------------------------------------------
auto url_list_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {hold<url_list_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// valtree_float_vector_builder
//------------------------------------------------------------------------------
struct valtree_float_vector_builder final
  : valtree::object_builder_impl<valtree_float_vector_builder> {
    using base = valtree::object_builder_impl<valtree_float_vector_builder>;

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    template <typename T>
    void do_add(const basic_string_path&, span<const T>) noexcept {}

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    void do_add(const basic_string_path& path, span<const float> data) noexcept;

    std::vector<float> _values;
};
//------------------------------------------------------------------------------
template <std::integral T>
void valtree_float_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    if(path.has_size(2)) {
        if(not data.empty()) {
            if((path.starts_with("values")) or (path.starts_with("data"))) {
                for(const auto v : data) {
                    _values.push_back(float(v));
                }
            }
        }
    } else if(path.has_size(1)) {
        if(data.has_single_value()) {
            if((path.starts_with("count")) or (path.starts_with("size"))) {
                _values.reserve(std_size(*data));
            }
        }
    }
}
//------------------------------------------------------------------------------
template <std::floating_point T>
void valtree_float_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    if(path.has_size(2)) {
        if(not data.empty()) {
            if((path.starts_with("values")) or (path.starts_with("data"))) {
                for(const auto v : data) {
                    _values.push_back(float(v));
                }
            }
        }
    }
}
//------------------------------------------------------------------------------
void valtree_float_vector_builder::do_add(
  const basic_string_path& path,
  span<const float> data) noexcept {
    if(path.has_size(2)) {
        if(not data.empty()) {
            if((path.starts_with("values")) or (path.starts_with("data"))) {
                _values.insert(_values.end(), data.begin(), data.end());
            }
        }
    }
}
//------------------------------------------------------------------------------
// float_list_resource
//------------------------------------------------------------------------------
auto float_list_resource::kind() const noexcept -> identifier {
    return "FloatList";
}
//------------------------------------------------------------------------------
struct float_list_resource::_loader final
  : simple_loader_of<float_list_resource> {
    using base = simple_loader_of<float_list_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    visited_valtree_resource _visit{
      valtree::make_building_value_tree_visitor(
        {hold<valtree_float_vector_builder>}),
      64};
};
//------------------------------------------------------------------------------
auto float_list_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_visit, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void float_list_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(auto builder{_visit.builder_as<valtree_float_vector_builder>()}) {
        using std::swap;
        swap(builder->_values, resource()._private_ref());
    }
    base::resource_loaded(info);
}
//------------------------------------------------------------------------------
auto float_list_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {hold<float_list_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// valtree_vec3_vector_builder
//------------------------------------------------------------------------------
struct valtree_vec3_vector_builder
  : valtree::object_builder_impl<valtree_vec3_vector_builder> {
    using base = valtree::object_builder_impl<valtree_vec3_vector_builder>;

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    template <typename T>
    void do_add(const basic_string_path&, span<const T>) noexcept {}

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    void finish_object(const basic_string_path& path) noexcept final;

    template <typename T>
    auto _do_add(const basic_string_path& path, span<const T> data) noexcept
      -> bool;

    math::vector<float, 3> _temp{0.F, 0.F, 0.F};
    std::vector<math::vector<float, 3>> _values;
    std::size_t _offs{0U};
};
//------------------------------------------------------------------------------
template <typename T>
auto valtree_vec3_vector_builder::_do_add(
  const basic_string_path& path,
  span<const T> data) noexcept -> bool {
    if(path.has_size(3)) {
        if((path.starts_with("values")) or (path.starts_with("data"))) {
            if(data.has_single_value()) {
                if((path.ends_with("x")) or (path.ends_with("r"))) {
                    _temp._v[0] = *data;
                    return true;
                } else if((path.ends_with("y")) or (path.ends_with("g"))) {
                    _temp._v[1] = *data;
                    return true;
                } else if((path.ends_with("z")) or (path.ends_with("b"))) {
                    _temp._v[2] = *data;
                    return true;
                }
            }
            if(path.ends_with("_")) {
                for(const auto i : index_range(data)) {
                    _temp._v[_offs] = data[i];
                    if(++_offs == 3) {
                        _offs = 0;
                        _values.push_back(_temp);
                        _temp = {0.F, 0.F, 0.F};
                    }
                }
                return true;
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
template <std::integral T>
void valtree_vec3_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    if(not _do_add(path, data)) {
        if((path.has_size(1)) and data.has_single_value()) {
            if((path.starts_with("count")) or (path.starts_with("size"))) {
                _values.reserve(std_size(*data));
            }
        }
    }
}
//------------------------------------------------------------------------------
template <std::floating_point T>
void valtree_vec3_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    _do_add(path, data);
}
//------------------------------------------------------------------------------
void valtree_vec3_vector_builder::finish_object(
  const basic_string_path& path) noexcept {
    if(path.has_size(2)) {
        if((path.starts_with("values")) or (path.starts_with("data"))) {
            _values.push_back(_temp);
            _temp = {0.F, 0.F, 0.F};
        }
    }
}
//------------------------------------------------------------------------------
// vec3_list_resource
//------------------------------------------------------------------------------
auto vec3_list_resource::kind() const noexcept -> identifier {
    return "Vec3List";
}
//------------------------------------------------------------------------------
struct vec3_list_resource::_loader final
  : simple_loader_of<vec3_list_resource> {
    using base = simple_loader_of<vec3_list_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    visited_valtree_resource _visit{
      valtree::make_building_value_tree_visitor(
        {hold<valtree_vec3_vector_builder>}),
      64};
};
//------------------------------------------------------------------------------
auto vec3_list_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_visit, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void vec3_list_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(auto builder{_visit.builder_as<valtree_vec3_vector_builder>()}) {
        using std::swap;
        swap(builder->_values, resource()._private_ref());
    }
    base::resource_loaded(info);
}
//------------------------------------------------------------------------------
auto vec3_list_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {hold<vec3_list_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// valtree_mat4_vector_builder
//------------------------------------------------------------------------------
struct valtree_mat4_vector_builder
  : valtree::object_builder_impl<valtree_mat4_vector_builder> {
    using base = valtree::object_builder_impl<valtree_mat4_vector_builder>;

    using base::base;

    auto max_token_size() noexcept -> span_size_t final {
        return 64;
    }

    template <typename T>
    void do_add(const basic_string_path&, span<const T>) noexcept {}

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T> data) noexcept;

    void finish_object(const basic_string_path& path) noexcept final;

    template <typename T>
    auto _do_add(const basic_string_path& path, span<const T> data) noexcept
      -> bool;

    static constexpr auto _default() noexcept
      -> math::matrix<float, 4, 4, true> {
        return {};
    }

    math::matrix<float, 4, 4, true> _temp{_default()};
    std::vector<math::matrix<float, 4, 4, true>> _values;
    std::size_t _offs{0U};
    std::size_t _roffs{0U};
    std::size_t _coffs{0U};
};
//------------------------------------------------------------------------------
template <typename T>
auto valtree_mat4_vector_builder::_do_add(
  const basic_string_path& path,
  span<const T> data) noexcept -> bool {
    if(path.has_size(3)) {
        if((path.starts_with("values")) or (path.starts_with("data"))) {
            if(data.has_single_value()) {
                if(path.ends_with("ii")) {
                    _temp.set_rm(0, 0, float(*data));
                    return true;
                } else if(path.ends_with("ij")) {
                    _temp.set_rm(0, 1, float(*data));
                    return true;
                } else if(path.ends_with("ik")) {
                    _temp.set_rm(0, 2, float(*data));
                    return true;
                } else if(path.ends_with("il")) {
                    _temp.set_rm(0, 3, float(*data));
                    return true;
                } else if(path.ends_with("ji")) {
                    _temp.set_rm(1, 0, float(*data));
                    return true;
                } else if(path.ends_with("jj")) {
                    _temp.set_rm(1, 1, float(*data));
                    return true;
                } else if(path.ends_with("jk")) {
                    _temp.set_rm(1, 2, float(*data));
                    return true;
                } else if(path.ends_with("jl")) {
                    _temp.set_rm(1, 3, float(*data));
                    return true;
                } else if(path.ends_with("ki")) {
                    _temp.set_rm(2, 0, float(*data));
                    return true;
                } else if(path.ends_with("kj")) {
                    _temp.set_rm(2, 1, float(*data));
                    return true;
                } else if(path.ends_with("kk")) {
                    _temp.set_rm(2, 2, float(*data));
                    return true;
                } else if(path.ends_with("kl")) {
                    _temp.set_rm(2, 3, float(*data));
                    return true;
                } else if(path.ends_with("li")) {
                    _temp.set_rm(3, 0, float(*data));
                    return true;
                } else if(path.ends_with("lj")) {
                    _temp.set_rm(3, 1, float(*data));
                    return true;
                } else if(path.ends_with("lk")) {
                    _temp.set_rm(3, 2, float(*data));
                    return true;
                } else if(path.ends_with("ll")) {
                    _temp.set_rm(3, 3, float(*data));
                    return true;
                }
            }
            if(path.ends_with("_")) {
                for(const auto i : index_range(data)) {
                    _temp.set_rm(_offs / 4, _offs % 4, float(data[i]));
                    if(++_offs == 16) {
                        _offs = 0;
                        _roffs = 0;
                        _coffs = 0;
                        _values.push_back(_temp);
                        _temp = _default();
                    }
                }
                return true;
            }
        }
    } else if(path.has_size(4)) {
        if(path.ends_with("_")) {
            for(const auto i : index_range(data)) {
                _temp.set_rm(_roffs, _coffs, float(data[i]));
                if(++_coffs == 4) {
                    _coffs = 0;
                    if(++_roffs == 4) {
                        _roffs = 0;
                        _offs = 0;
                        _values.push_back(_temp);
                        _temp = _default();
                    }
                }
            }
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
template <std::integral T>
void valtree_mat4_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    if(not _do_add(path, data)) {
        if((path.has_size(1)) and data.has_single_value()) {
            if((path.starts_with("count")) or (path.starts_with("size"))) {
                _values.reserve(std_size(*data));
            }
        }
    }
}
//------------------------------------------------------------------------------
template <std::floating_point T>
void valtree_mat4_vector_builder::do_add(
  const basic_string_path& path,
  span<const T> data) noexcept {
    _do_add(path, data);
}
//------------------------------------------------------------------------------
void valtree_mat4_vector_builder::finish_object(
  const basic_string_path& path) noexcept {
    if(path.has_size(2)) {
        if((path.starts_with("values")) or (path.starts_with("data"))) {
            _values.push_back(_temp);
            _temp = {};
        }
    }
}
//------------------------------------------------------------------------------
// mat4_list_resource
//------------------------------------------------------------------------------
auto mat4_list_resource::kind() const noexcept -> identifier {
    return "Mat4List";
}
//------------------------------------------------------------------------------
struct mat4_list_resource::_loader final
  : simple_loader_of<mat4_list_resource> {
    using base = simple_loader_of<mat4_list_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    visited_valtree_resource _visit{
      valtree::make_building_value_tree_visitor(
        {hold<valtree_mat4_vector_builder>}),
      64};
};
//------------------------------------------------------------------------------
auto mat4_list_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_visit, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void mat4_list_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(auto builder{_visit.builder_as<valtree_mat4_vector_builder>()}) {
        using std::swap;
        swap(builder->_values, resource()._private_ref());
    }
    base::resource_loaded(info);
}
//------------------------------------------------------------------------------
auto mat4_list_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {hold<mat4_list_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// smooth_float_curve_resource
//------------------------------------------------------------------------------
auto smooth_float_curve_resource::kind() const noexcept -> identifier {
    return "FloatCurve";
}
//------------------------------------------------------------------------------
struct smooth_float_curve_resource::_loader final
  : simple_loader_of<smooth_float_curve_resource> {
    using base = simple_loader_of<smooth_float_curve_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    float_list_resource _cps;
};
//------------------------------------------------------------------------------
auto smooth_float_curve_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_cps, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void smooth_float_curve_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    resource()._private_ref() = {std::move(_cps.release_resource())};
    base::resource_loaded(info);
}
//------------------------------------------------------------------------------
auto smooth_float_curve_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {
      hold<smooth_float_curve_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// smooth_vec3_curve_resource
//------------------------------------------------------------------------------
auto smooth_vec3_curve_resource::kind() const noexcept -> identifier {
    return "Vec3Curve";
}
//------------------------------------------------------------------------------
struct smooth_vec3_curve_resource::_loader final
  : simple_loader_of<smooth_vec3_curve_resource> {
    using base = simple_loader_of<smooth_vec3_curve_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    vec3_list_resource _cps;
};
//------------------------------------------------------------------------------
auto smooth_vec3_curve_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_cps, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void smooth_vec3_curve_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    resource()._private_ref() = {std::move(_cps.release_resource())};
    base::resource_loaded(info);
}
//------------------------------------------------------------------------------
auto smooth_vec3_curve_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {
      hold<smooth_vec3_curve_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// glsl_string_resource
//------------------------------------------------------------------------------
auto glsl_string_resource::kind() const noexcept -> identifier {
    return "GLSLString";
}
//------------------------------------------------------------------------------
struct glsl_string_resource::_loader final
  : simple_loader_of<glsl_string_resource> {
    using base = simple_loader_of<glsl_string_resource>;
    using base::base;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    plain_text_resource _text;
};
//------------------------------------------------------------------------------
auto glsl_string_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_text, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void glsl_string_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    resource()._private_ref() = {std::move(_text.release_resource())};
    base::resource_loaded(info);
}
//------------------------------------------------------------------------------
auto glsl_string_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {hold<glsl_string_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
// shape_generator_resource
//------------------------------------------------------------------------------
auto shape_generator_resource::kind() const noexcept -> identifier {
    return "ShapeGnrtr";
}
//------------------------------------------------------------------------------
struct shape_generator_resource::_loader final
  : simple_loader_of<shape_generator_resource> {
    using base = simple_loader_of<shape_generator_resource>;

    _loader(
      resource_interface& resource,
      const shared_holder<loaded_resource_context>& context,
      resource_request_params params) noexcept;

    auto request_dependencies(resource_loader& loader) noexcept
      -> valid_if_not_zero<identifier_t> final;

    void resource_loaded(const load_info&) noexcept final;

    valtree_resource _tree;
};
//------------------------------------------------------------------------------
shape_generator_resource::_loader::_loader(
  resource_interface& resource,
  const shared_holder<loaded_resource_context>& context,
  resource_request_params params) noexcept
  : base{resource, context, std::move(params)} {
    if(auto gen{shapes::shape_from(params.locator, main_ctx::get())}) {
        this->resource()._private_ref() = std::move(gen);
        this->resource()._private_set_status(resource_status::loaded);
        if(_res_loader) [[likely]] {
            this->_notify_loaded(*_res_loader);
        }
    }
}
//------------------------------------------------------------------------------
auto shape_generator_resource::_loader::request_dependencies(
  resource_loader& res_loader) noexcept -> valid_if_not_zero<identifier_t> {
    return _add_single_dependency(
      res_loader.load(_tree, parameters()), res_loader);
}
//------------------------------------------------------------------------------
void shape_generator_resource::_loader::resource_loaded(
  const load_info& info) noexcept {
    if(auto gen{
         shapes::from_value_tree(_tree.release_resource(), main_ctx::get())}) {
        resource()._private_ref() = std::move(gen);
        base::resource_loaded(info);
        return;
    }
    base::resource_cancelled(info);
}
//------------------------------------------------------------------------------
auto shape_generator_resource::make_loader(
  const shared_holder<loaded_resource_context>& ctx,
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {
      hold<shape_generator_resource::_loader>, *this, ctx, std::move(params)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
