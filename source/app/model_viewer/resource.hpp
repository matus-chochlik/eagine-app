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
template <typename Wrapper>
class model_viewer_resources {
public:
    signal<void() noexcept> loaded;

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
        assert(_selected < _loaded.size());
        return _loaded[_selected];
    }

    auto load(url locator, execution_context& ctx, video_context& video)
      -> model_viewer_resources& {
        _loaded.emplace_back(make_viewer_resource(
          std::type_identity<Wrapper>{}, std::move(locator), ctx, video));
        _loaded.back().signals().loaded.connect(
          make_callable_ref<&model_viewer_resources::_on_loaded>(this));
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
    void _on_loaded() noexcept {
        loaded();
    }

    std::vector<Wrapper> _loaded;
    std::size_t _selected{0U};
};
//------------------------------------------------------------------------------
} // namespace eagine::app
#endif
