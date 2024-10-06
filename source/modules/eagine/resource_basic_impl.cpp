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
import eagine.core.value_tree;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.msgbus;

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
struct plain_text_resource::_loader final : loader_of<plain_text_resource> {
    using loader_of<plain_text_resource>::loader_of;

    auto request_dependencies(
      const std::shared_ptr<resource_loader>& loader,
      resource_request_params&& params) noexcept -> bool final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;

    void stream_finished() noexcept final;

    void stream_cancelled() noexcept final;

    identifier_t _chunk_req_id{0};
};
//------------------------------------------------------------------------------
auto plain_text_resource::_loader::request_dependencies(
  const std::shared_ptr<resource_loader>& res_loader,
  resource_request_params&& params) noexcept -> bool {
    _chunk_req_id = res_loader->fetch_resource_chunks(params, 1024).first;
    if(_chunk_req_id > 0) {
        resource()._status = resource_status::loading;
        return true;
    }
    resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
void plain_text_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    auto& text{resource()._text};
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
void plain_text_resource::_loader::stream_finished() noexcept {
    resource()._status = resource_status::loaded;
}
//------------------------------------------------------------------------------
void plain_text_resource::_loader::stream_cancelled() noexcept {
    resource()._status = resource_status::cancelled;
}
//------------------------------------------------------------------------------
auto plain_text_resource::make_loader(resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(
      params.locator.has_path_suffix(".txt") or
      params.locator.has_scheme("txt") or params.locator.has_scheme("text")) {
        return {hold<plain_text_resource::_loader>, *this, std::move(params)};
    }
    return {};
}
//------------------------------------------------------------------------------
// string_list_resource
//------------------------------------------------------------------------------
struct string_list_resource::_loader final : loader_of<string_list_resource> {
    using loader_of<string_list_resource>::loader_of;

    auto request_dependencies(
      const std::shared_ptr<resource_loader>& loader,
      resource_request_params&& params) noexcept -> bool final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;

    void stream_finished() noexcept final;

    void stream_cancelled() noexcept final;

    std::string _chunk_line;
    identifier_t _chunk_req_id{0};
};
//------------------------------------------------------------------------------
auto string_list_resource::_loader::request_dependencies(
  const std::shared_ptr<resource_loader>& res_loader,
  resource_request_params&& params) noexcept -> bool {
    _chunk_req_id = res_loader->fetch_resource_chunks(params, 1024).first;
    if(_chunk_req_id > 0) {
        resource()._status = resource_status::loading;
        return true;
    }
    resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
void string_list_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    for_each_chunk_line(_chunk_line, chunks, [&, this](auto line) {
        resource()._strings.emplace_back(std::move(line));
    });
}
//------------------------------------------------------------------------------
void string_list_resource::_loader::stream_finished() noexcept {
    if(not _chunk_line.empty()) {
        resource()._strings.emplace_back(std::move(_chunk_line));
    }
    resource()._status = resource_status::loaded;
}
//------------------------------------------------------------------------------
void string_list_resource::_loader::stream_cancelled() noexcept {
    resource()._status = resource_status::cancelled;
}
//------------------------------------------------------------------------------
auto string_list_resource::make_loader(resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(
      params.locator.has_path_suffix(".txt") or
      params.locator.has_path_suffix(".text") or
      params.locator.has_scheme("txt") or params.locator.has_scheme("text")) {
        return {hold<string_list_resource::_loader>, *this, std::move(params)};
    }
    return {};
}
//------------------------------------------------------------------------------
// url_list_resource
//------------------------------------------------------------------------------
struct url_list_resource::_loader final : loader_of<url_list_resource> {
    using loader_of<url_list_resource>::loader_of;

    auto request_dependencies(
      const std::shared_ptr<resource_loader>& loader,
      resource_request_params&& params) noexcept -> bool final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;

    void stream_finished() noexcept final;

    void stream_cancelled() noexcept final;

    std::string _chunk_line;
    identifier_t _chunk_req_id{0};
};
//------------------------------------------------------------------------------
auto url_list_resource::_loader::request_dependencies(
  const std::shared_ptr<resource_loader>& res_loader,
  resource_request_params&& params) noexcept -> bool {
    _chunk_req_id = res_loader->fetch_resource_chunks(params, 1024).first;
    if(_chunk_req_id > 0) {
        resource()._status = resource_status::loading;
        return true;
    }
    resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
void url_list_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    for_each_chunk_line(_chunk_line, chunks, [&, this](auto line) {
        if(url locator{std::move(line)}) {
            resource()._urls.emplace_back(std::move(locator));
        }
    });
}
//------------------------------------------------------------------------------
void url_list_resource::_loader::stream_finished() noexcept {
    if(not _chunk_line.empty()) {
        if(url locator{std::move(_chunk_line)}) {
            resource()._urls.emplace_back(std::move(locator));
        }
    }
    resource()._status = resource_status::loaded;
}
//------------------------------------------------------------------------------
void url_list_resource::_loader::stream_cancelled() noexcept {
    resource()._status = resource_status::cancelled;
}
//------------------------------------------------------------------------------
auto url_list_resource::make_loader(resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(
      params.locator.has_path_suffix(".txt") or
      params.locator.has_path_suffix(".text") or
      params.locator.has_path_suffix(".urls") or
      params.locator.has_scheme("txt") or params.locator.has_scheme("text")) {
        return {hold<url_list_resource::_loader>, *this, std::move(params)};
    }
    return {};
}
//------------------------------------------------------------------------------
// visited_valtree_resource
//------------------------------------------------------------------------------
struct visited_valtree_resource::_loader final
  : loader_of<visited_valtree_resource> {
    _loader(
      resource_interface& resource,
      resource_request_params params,
      valtree::value_tree_stream_input input) noexcept
      : loader_of<visited_valtree_resource>(resource, std::move(params))
      , _input{std::move(input)} {}

    auto request_dependencies(
      const std::shared_ptr<resource_loader>& loader,
      resource_request_params&& params) noexcept -> bool final;

    void stream_data_appended(const msgbus::blob_stream_chunk&) noexcept final;

    void stream_finished() noexcept final;

    void stream_cancelled() noexcept final;

    valtree::value_tree_stream_input _input;
    identifier_t _chunk_req_id{0};
    bool _finished{false};
};
//------------------------------------------------------------------------------
auto visited_valtree_resource::_loader::request_dependencies(
  const std::shared_ptr<resource_loader>& res_loader,
  resource_request_params&& params) noexcept -> bool {
    _chunk_req_id = res_loader->fetch_resource_chunks(params, 1024).first;
    if(_chunk_req_id > 0) {
        resource()._status = resource_status::loading;
        return true;
    }
    resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
void visited_valtree_resource::_loader::stream_data_appended(
  const msgbus::blob_stream_chunk& chunks) noexcept {
    for(const auto chunk : chunks.data) {
        if(not _finished) {
            if(not _input.consume_data(chunk)) {
                _input.finish();
                _finished = true;
            }
        }
    }
}
//------------------------------------------------------------------------------
void visited_valtree_resource::_loader::stream_finished() noexcept {
    resource()._status = resource_status::loaded;
}
//------------------------------------------------------------------------------
void visited_valtree_resource::_loader::stream_cancelled() noexcept {
    resource()._status = resource_status::cancelled;
}
//------------------------------------------------------------------------------
auto visited_valtree_resource::make_loader(
  resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    if(
      params.locator.has_path_suffix(".json") or
      params.locator.has_scheme("json")) {
        return {
          hold<visited_valtree_resource::_loader>,
          *this,
          std::move(params),
          traverse_json_stream(
            _visitor,
            _max_token_size,
            main_ctx::get().buffers(),
            main_ctx::get().log())};
    }
    if(
      params.locator.has_path_suffix(".yaml") or
      params.locator.has_scheme("yaml")) {
        return {
          hold<visited_valtree_resource::_loader>,
          *this,
          std::move(params),
          traverse_yaml_stream(
            _visitor,
            _max_token_size,
            main_ctx::get().buffers(),
            main_ctx::get().log())};
    }
    return {};
}
//------------------------------------------------------------------------------
// valtree_float_vector_builder
//------------------------------------------------------------------------------
class valtree_float_vector_builder final
  : public valtree::object_builder_impl<valtree_float_vector_builder> {
    using base = valtree::object_builder_impl<valtree_float_vector_builder>;

public:
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

    auto finish() noexcept -> bool final;

    void failed() noexcept final;

private:
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
auto valtree_float_vector_builder::finish() noexcept -> bool {
    // TODO: notify continuation
    // resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
void valtree_float_vector_builder::failed() noexcept {
    // resource()._status = resource_status::error;
}
//------------------------------------------------------------------------------
// float_list_resource
//------------------------------------------------------------------------------
struct float_list_resource::_loader final : loader_of<float_list_resource> {
    _loader(
      resource_interface& resource,
      resource_request_params params,
      shared_holder<valtree::value_tree_visitor> visitor) noexcept
      : loader_of<float_list_resource>{resource, std::move(params)}
      , _visit{std::move(visitor), 64} {}

    auto request_dependencies(
      const std::shared_ptr<resource_loader>& loader,
      resource_request_params&& params) noexcept -> bool final;

    visited_valtree_resource _visit;
    identifier_t _visit_req_id{0};
};
//------------------------------------------------------------------------------
auto float_list_resource::_loader::request_dependencies(
  const std::shared_ptr<resource_loader>& res_loader,
  resource_request_params&& params) noexcept -> bool {
    _visit_req_id = res_loader->load(_visit, std::move(params));
    if(_visit_req_id > 0) {
        resource()._status = resource_status::loading;
        return true;
    }
    resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
auto float_list_resource::make_loader(resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {
      hold<float_list_resource::_loader>,
      *this,
      std::move(params),
      valtree::make_building_value_tree_visitor(
        {hold<valtree_float_vector_builder>})};
}
//------------------------------------------------------------------------------
// valtree_vec3_vector_builder
//------------------------------------------------------------------------------
class valtree_vec3_vector_builder
  : public valtree::object_builder_impl<valtree_vec3_vector_builder> {
    using base = valtree::object_builder_impl<valtree_vec3_vector_builder>;

public:
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

    auto finish() noexcept -> bool final;

    void failed() noexcept final;

private:
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
auto valtree_vec3_vector_builder::finish() noexcept -> bool {
    // resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
void valtree_vec3_vector_builder::failed() noexcept {
    // resource()._status = resource_status::error;
}
//------------------------------------------------------------------------------
// vec3_list_resource
//------------------------------------------------------------------------------
struct vec3_list_resource::_loader final : loader_of<vec3_list_resource> {
    _loader(
      resource_interface& resource,
      resource_request_params params,
      shared_holder<valtree::value_tree_visitor> visitor) noexcept
      : loader_of<vec3_list_resource>{resource, std::move(params)}
      , _visit{std::move(visitor), 64} {}

    auto request_dependencies(
      const std::shared_ptr<resource_loader>& loader,
      resource_request_params&& params) noexcept -> bool final;

    visited_valtree_resource _visit;
    identifier_t _visit_req_id{0};
};
//------------------------------------------------------------------------------
auto vec3_list_resource::_loader::request_dependencies(
  const std::shared_ptr<resource_loader>& res_loader,
  resource_request_params&& params) noexcept -> bool {
    _visit_req_id = res_loader->load(_visit, std::move(params));
    if(_visit_req_id > 0) {
        resource()._status = resource_status::loading;
        return true;
    }
    resource()._status = resource_status::error;
    return false;
}
//------------------------------------------------------------------------------
auto vec3_list_resource::make_loader(resource_request_params params) noexcept
  -> shared_holder<resource_interface::loader> {
    return {
      hold<vec3_list_resource::_loader>,
      *this,
      std::move(params),
      valtree::make_building_value_tree_visitor(
        {hold<valtree_vec3_vector_builder>})};
}
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
