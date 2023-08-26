/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_RESOURCE_HPP
#define EAGINE_APP_MODEL_VIEWER_RESOURCE_HPP
#include <cassert>

import eagine.core;
import eagine.guiplus;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_resource_intf;
struct model_viewer_resource_signals {
    signal<void() noexcept> loaded;
};
//------------------------------------------------------------------------------
struct model_viewer_resource_intf
  : abstract<model_viewer_resource_intf>
  , model_viewer_resource_signals {

    virtual auto is_loaded() noexcept -> bool = 0;
    virtual void load_if_needed(execution_context&, video_context&) = 0;
    virtual void use(video_context&) = 0;
    virtual void clean_up(execution_context&, video_context&) = 0;

protected:
    void signal_loaded();
};
//------------------------------------------------------------------------------
template <typename Intf>
class model_viewer_resource_wrapper {
public:
    model_viewer_resource_wrapper(unique_holder<Intf> impl) noexcept
      : _impl{std::move(impl)} {}

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

    void settings(const guiplus::imgui_api& gui) noexcept;
    void update() noexcept;

protected:
    auto _load_handler() noexcept {
        return make_callable_ref<&model_viewer_resources_base::_on_loaded>(
          this);
    }

    auto _add_name(std::string name) {
        _names.emplace_back(std::move(name));
    }

    auto _selected(auto& items) const noexcept -> auto& {
        assert(_selected_index < items.size());
        return items[_selected_index];
    }

private:
    void _on_loaded() noexcept {
        loaded();
    }
    std::size_t _previous_index{0U};
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

    auto selected() noexcept -> Wrapper& {
        return _selected(_loaded);
    }

    auto load(
      std::string name,
      url locator,
      execution_context& ctx,
      video_context& video) -> model_viewer_resources& {
        this->_add_name(std::move(name));
        _loaded.emplace_back(make_viewer_resource(
          std::type_identity<Wrapper>{}, std::move(locator), ctx, video));
        _loaded.back().signals().loaded.connect(this->_load_handler());
        return *this;
    }

    auto use(video_context& video) noexcept -> model_viewer_resources& {
        selected().use(video);
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

private:
    std::vector<Wrapper> _loaded;
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
