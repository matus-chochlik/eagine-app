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
import eagine.core.math;
import eagine.core.memory;
import eagine.core.string;
import eagine.core.value_tree;
import eagine.core.units;

namespace eagine::app {
//------------------------------------------------------------------------------
// valtree_float_vector_builder
//------------------------------------------------------------------------------
class valtree_float_vector_builder
  : public valtree_builder_base<valtree_float_vector_builder> {
    using base = valtree_builder_base<valtree_float_vector_builder>;

public:
    using base::base;
    using base::do_add;

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        if(path.size() == 2) {
            if(!data.empty()) {
                if((path.front() == "values") && (path.front() == "data")) {
                    for(const auto v : data) {
                        _values.push_back(float(v));
                    }
                }
            }
        } else if(path.size() == 1) {
            if(data.has_single_value()) {
                if((path.front() == "count") || (path.front() == "size")) {
                    _values.reserve(std_size(extract(data)));
                }
            }
        }
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        if(path.size() == 2) {
            if(!data.empty()) {
                if((path.front() == "values") && (path.front() == "data")) {
                    for(const auto v : data) {
                        _values.push_back(float(v));
                    }
                }
            }
        }
    }

    void do_add(
      const basic_string_path& path,
      span<const float>& data) noexcept {
        if(path.size() == 2) {
            if(!data.empty()) {
                if((path.front() == "values") && (path.front() == "data")) {
                    _values.insert(_values.end(), data.begin(), data.end());
                }
            }
        }
    }

    void finish() noexcept final {
        if(auto parent{_parent.lock()}) {
            if(const auto cont{extract(parent).continuation()}) {
                extract(cont).handle_float_vector(extract(parent), _values);
            }
        }
    }

private:
    std::vector<float> _values;
};
//------------------------------------------------------------------------------
auto make_valtree_float_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_float_vector_builder>(parent);
}
//------------------------------------------------------------------------------
// valtree_vec3_vector_builder
//------------------------------------------------------------------------------
class valtree_vec3_vector_builder
  : public valtree_builder_base<valtree_vec3_vector_builder> {
    using base = valtree_builder_base<valtree_vec3_vector_builder>;

public:
    using base::base;
    using base::do_add;

    template <typename T>
    auto _do_add(const basic_string_path& path, span<const T>& data) noexcept
      -> bool {
        if(path.size() == 3) {
            if((path.front() == "values") || (path.front() == "data")) {
                if(data.has_single_value()) {
                    if((path.back() == "x") || (path.back() == "r")) {
                        _temp._v[0] = extract(data);
                        return true;
                    } else if((path.back() == "y") || (path.back() == "g")) {
                        _temp._v[1] = extract(data);
                        return true;
                    } else if((path.back() == "z") || (path.back() == "b")) {
                        _temp._v[2] = extract(data);
                        return true;
                    }
                }
                if(path.back() == "_") {
                    for(const auto i : integer_range(data.size())) {
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

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        if(!_do_add(path, data)) {
            if((path.size() == 1) && data.has_single_value()) {
                if((path.front() == "count") || (path.front() == "size")) {
                    _values.reserve(std_size(extract(data)));
                }
            }
        }
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        _do_add(path, data);
    }

    void finish_object(const basic_string_path& path) noexcept final {
        if(path.size() == 2) {
            if((path.front() == "values") || (path.front() == "data")) {
                _values.push_back(_temp);
                _temp = {0.F, 0.F, 0.F};
            }
        }
    }

    void finish() noexcept final {
        if(auto parent{_parent.lock()}) {
            extract(parent).handle_vec3_vector(extract(parent), _values);
        }
    }

private:
    math::vector<float, 3, true> _temp{0.F, 0.F, 0.F};
    std::vector<math::vector<float, 3, true>> _values;
    std::size_t _offs{0U};
};
//------------------------------------------------------------------------------
auto make_valtree_vec3_vector_builder(
  const std::shared_ptr<pending_resource_info>& parent) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_vec3_vector_builder>(parent);
}
//------------------------------------------------------------------------------
// camera parameters
//------------------------------------------------------------------------------
class valtree_orbiting_camera_parameters_builder
  : public valtree_builder_base<valtree_orbiting_camera_parameters_builder> {
    using base =
      valtree_builder_base<valtree_orbiting_camera_parameters_builder>;

public:
    valtree_orbiting_camera_parameters_builder(
      const std::shared_ptr<pending_resource_info>& parent,
      orbiting_camera& camera) noexcept
      : base{parent}
      , _camera{camera} {}

    using base::do_add;

    template <typename T>
    void parse_param(
      const basic_string_path& path,
      span<const T>& data) noexcept {
        if((path.size() == 1) && data.has_single_value()) {
            const auto value{extract(data)};
            if(value > 0.F) {
                if(path.front() == "near") {
                    _camera.set_near(value);
                } else if(path.front() == "far") {
                    _camera.set_far(value);
                } else if(path.front() == "orbit_min") {
                    _camera.set_orbit_min(value);
                } else if(path.front() == "orbit_max") {
                    _camera.set_orbit_max(value);
                } else if(path.front() == "fov_deg") {
                    _camera.set_fov(degrees_(value));
                } else if(path.front() == "fov_rad") {
                    _camera.set_fov(radians_(value));
                }
            }
            if(path.front() == "azimuth_deg") {
                _camera.set_azimuth(degrees_(value));
            } else if(path.front() == "azimuth_rad") {
                _camera.set_azimuth(radians_(value));
            } else if(path.front() == "elevation_deg") {
                _camera.set_elevation(degrees_(value));
            } else if(path.front() == "elevation_rad") {
                _camera.set_elevation(radians_(value));
            }
        }
    }

    template <std::integral T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        parse_param(path, data);
    }

    template <std::floating_point T>
    void do_add(const basic_string_path& path, span<const T>& data) noexcept {
        parse_param(path, data);
    }

private:
    orbiting_camera& _camera;
};
//------------------------------------------------------------------------------
auto make_valtree_camera_parameters_builder(
  const std::shared_ptr<pending_resource_info>& parent,
  orbiting_camera& camera) noexcept
  -> std::unique_ptr<valtree::object_builder> {
    return std::make_unique<valtree_orbiting_camera_parameters_builder>(
      parent, camera);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
