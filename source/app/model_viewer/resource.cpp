/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <cassert>

export module eagine.app.model_viewer:resource;

import eagine.core;
import eagine.guiplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_resource_intf;
struct model_viewer_resource_signals {
    signal<void() noexcept> loaded;
    signal<void() noexcept> selected;
};
//------------------------------------------------------------------------------
struct model_viewer_resource_intf
  : abstract<model_viewer_resource_intf>
  , model_viewer_resource_signals {

    virtual auto is_loaded() noexcept -> bool = 0;
    virtual void load_if_needed(execution_context&, video_context&) = 0;
    virtual void use(video_context&) = 0;
    virtual void clean_up(execution_context&, video_context&) = 0;
    virtual auto settings_height() -> float {
        return 0.F;
    }
    virtual void settings(const guiplus::imgui_api&) noexcept {}

protected:
    void signal_loaded();
};
//------------------------------------------------------------------------------
template <typename Intf>
class model_viewer_resource_wrapper {
public:
    model_viewer_resource_wrapper(unique_holder<Intf> impl) noexcept
      : _impl{std::move(impl)} {}

    auto implementation() noexcept -> optional_reference<Intf> {
        return {_impl};
    }

    explicit operator bool() const noexcept {
        return is_loaded();
    }

    auto signals() const noexcept -> model_viewer_resource_signals& {
        return *_impl;
    }

    auto is_loaded() const noexcept -> bool {
        return _impl and _impl->is_loaded();
    }

    auto load_if_needed(execution_context& ctx, video_context& video)
      -> model_viewer_resource_wrapper& {
        if(_impl) [[likely]] {
            _impl->load_if_needed(ctx, video);
        }
        return *this;
    }

    auto use(video_context& video) -> model_viewer_resource_wrapper& {
        if(_impl) [[likely]] {
            _impl->use(video);
        }
        return *this;
    }

    auto clean_up(execution_context& ctx, video_context& video)
      -> model_viewer_resource_wrapper& {
        if(_impl) {
            _impl->clean_up(ctx, video);
        }
        return *this;
    }

protected:
    unique_holder<Intf> _impl;
};
//------------------------------------------------------------------------------
class model_viewer_resources_base {
public:
    signal<void() noexcept> loaded;
    signal<void() noexcept> selected;

    void update() noexcept;
    auto all_resource_count() noexcept -> span_size_t;

protected:
    auto _settings_height(
      optional_reference<model_viewer_resource_intf>) noexcept -> float;

    void _settings(
      const string_view head,
      optional_reference<model_viewer_resource_intf>,
      const guiplus::imgui_api& gui) noexcept;

    auto _load_handler() noexcept {
        return make_callable_ref<&model_viewer_resources_base::_on_loaded>(
          this);
    }
    auto _select_handler() noexcept {
        return make_callable_ref<&model_viewer_resources_base::_on_selected>(
          this);
    }

    void _add_name(std::string name);

    auto _at(std::size_t index, auto& items) const noexcept -> auto& {
        assert(index < items.size());
        return items[index];
    }

    auto _current(auto& items) const noexcept -> auto& {
        return _at(_selected_index, items);
    }

protected:
    auto _resource_name(const program_arg& arg) -> std::string;
    auto _resource_url(const program_arg& arg) -> url;

private:
    void _on_loaded() noexcept;
    void _on_selected() noexcept;

    std::size_t _next_index{0U};
    std::size_t _selected_index{0U};
    std::vector<std::string> _names;
};
//------------------------------------------------------------------------------
template <typename Wrapper>
class model_viewer_resources : public model_viewer_resources_base {
public:
    explicit operator bool() const noexcept {
        return are_all_loaded();
    }

    auto are_all_loaded() const noexcept -> bool {
        for(auto& resource : _loaded) {
            if(not resource.is_loaded()) {
                return false;
            }
        }
        return true;
    }

    auto loaded_resource_count() noexcept -> span_size_t {
        span_size_t result{0};
        for(auto& resource : _loaded) {
            if(resource.is_loaded()) {
                ++result;
            }
        }
        return result;
    }

    auto at(std::size_t index) noexcept -> Wrapper& {
        return _at(index, _loaded);
    }

    auto current() noexcept -> Wrapper& {
        return _current(_loaded);
    }

    template <typename... Args>
    auto load(
      std::string name,
      url locator,
      execution_context& ctx,
      video_context& video,
      Args&&... args) -> model_viewer_resources& {
        this->_add_name(std::move(name));
        _loaded.emplace_back(make_viewer_resource(
          std::type_identity<Wrapper>{},
          std::move(locator),
          ctx,
          video,
          std::forward<Args>(args)...));
        _loaded.back().signals().loaded.connect(this->_load_handler());
        _loaded.back().signals().selected.connect(this->_select_handler());
        return *this;
    }

    template <typename... Args>
    auto load(
      const program_arg& arg,
      execution_context& ctx,
      video_context& video,
      Args&&... args) -> model_viewer_resources& {
        return load(
          this->_resource_name(arg),
          this->_resource_url(arg),
          ctx,
          video,
          std::forward<Args>(args)...);
    }

    auto use(video_context& video) noexcept -> model_viewer_resources& {
        current().use(video);
        return *this;
    }

    auto load_if_needed(execution_context& ctx, video_context& video) noexcept
      -> model_viewer_resources& {
        for(auto& resource : _loaded) {
            resource.load_if_needed(ctx, video);
        }
        return *this;
    }

    auto clean_up(execution_context& ctx, video_context& video)
      -> model_viewer_resources& {
        for(auto& resource : _loaded) {
            resource.clean_up(ctx, video);
        }
        return *this;
    }

    auto settings_height() noexcept -> float {
        return this->_settings_height(current().implementation());
    }

    void settings(
      const string_view head,
      const guiplus::imgui_api& gui) noexcept {
        return this->_settings(head, current().implementation(), gui);
    }

private:
    std::vector<Wrapper> _loaded;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
