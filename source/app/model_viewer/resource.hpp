/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#ifndef EAGINE_APP_MODEL_VIEWER_RESOURCE_HPP
#define EAGINE_APP_MODEL_VIEWER_RESOURCE_HPP

import eagine.core;
import eagine.app;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
struct model_viewer_resource_intf;
struct model_viewer_resource_signals {
    signal<void(model_viewer_resource_intf&) noexcept> loaded;
};
//------------------------------------------------------------------------------
struct model_viewer_resource_intf : abstract<model_viewer_resource_intf> {
    void setup(model_viewer_resource_signals&) noexcept;
    virtual auto is_loaded() noexcept -> bool = 0;
    virtual void load_if_needed(execution_context&) = 0;
    virtual void use(video_context&) = 0;
    virtual void clean_up(execution_context&, video_context&) = 0;

protected:
    void signal_loaded();

private:
    optional_reference<model_viewer_resource_signals> _signals;
};
//------------------------------------------------------------------------------
template <typename Intf>
class model_viewer_resource_wrapper : public model_viewer_resource_signals {
public:
    model_viewer_resource_wrapper(unique_holder<Intf> impl) noexcept
      : _impl{std::move(impl)} {
        _impl->setup(*this);
    }

    explicit operator bool() const noexcept {
        return _impl and _impl->is_loaded();
    }

    auto load_if_needed(execution_context& ctx)
      -> model_viewer_resource_wrapper& {
        if(_impl) [[likely]] {
            _impl->load_if_needed(ctx);
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
} // namespace eagine::app
#endif
