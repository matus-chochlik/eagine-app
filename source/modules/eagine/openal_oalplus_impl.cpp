/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import std;
import eagine.core.types;
import eagine.core.memory;
import eagine.core.identifier;
import eagine.core.utility;
import eagine.core.logging;
import eagine.core.c_api;
import eagine.core.main_ctx;
import eagine.oalplus;

namespace eagine::app {
//------------------------------------------------------------------------------
// playback device
//------------------------------------------------------------------------------
class oalplus_openal_player
  : public main_ctx_object
  , public audio_provider {
public:
    oalplus_openal_player(main_ctx_parent parent, oalplus::alc_api& alc)
      : main_ctx_object{"OpenALC", parent}
      , _alc_api{alc} {}

    auto get_context_attribs(
      execution_context&,
      const launch_options&,
      const audio_options&) const -> oalplus::context_attributes;

    auto initialize(
      execution_context&,
      oalplus::owned_device_handle,
      const identifier instance,
      const launch_options&,
      const audio_options&) -> bool;

    void clean_up();

    auto audio_kind() const noexcept -> audio_context_kind final;
    auto instance_id() const noexcept -> identifier final;

    void parent_context_changed(const audio_context&) final;
    void audio_begin(execution_context&) final;
    void audio_end(execution_context&) final;
    void audio_commit(execution_context&) final;

private:
    oalplus::alc_api& _alc_api;
    identifier _instance_id;
    oalplus::owned_device_handle _device{};
    oalplus::owned_context_handle _context{};
};
//------------------------------------------------------------------------------
auto oalplus_openal_player::get_context_attribs(
  execution_context&,
  const launch_options&,
  const audio_options& audio_opts) const -> oalplus::context_attributes {
    const auto& ALC = _alc_api.constants();

    // TODO
    (void)(audio_opts);

    const auto add_sync = [&](auto attribs) {
        return attribs + (ALC.sync | false);
    };

    return add_sync(oalplus::context_attributes());
}
//------------------------------------------------------------------------------
auto oalplus_openal_player::initialize(
  execution_context& exec_ctx,
  oalplus::owned_device_handle device,
  const identifier id,
  const launch_options& opts,
  const audio_options& audio_opts) -> bool {
    _instance_id = id;
    _device = std::move(device);
    const auto& [alc, ALC] = _alc_api;

    if(ok context{alc.create_context(
         _device, get_context_attribs(exec_ctx, opts, audio_opts))}) {
        _context = std::move(context.get());
        return true;
    } else {
        exec_ctx.log_error("failed to create AL context")
          .arg("message", (not context).message());
    }

    return false;
}
//------------------------------------------------------------------------------
void oalplus_openal_player::clean_up() {
    if(_device) {
        if(_context) {
            _alc_api.destroy_context(_device, std::move(_context));
        }
        _alc_api.close_device(std::move(_device));
    }
}
//------------------------------------------------------------------------------
auto oalplus_openal_player::audio_kind() const noexcept -> audio_context_kind {
    return audio_context_kind::openal;
}
//------------------------------------------------------------------------------
auto oalplus_openal_player::instance_id() const noexcept -> identifier {
    return _instance_id;
}
//------------------------------------------------------------------------------
void oalplus_openal_player::parent_context_changed(const audio_context&) {}
//------------------------------------------------------------------------------
void oalplus_openal_player::audio_begin(execution_context&) {
    _alc_api.make_context_current(_device, _context);
}
//------------------------------------------------------------------------------
void oalplus_openal_player::audio_end(execution_context&) {
    _alc_api.make_context_current.none(_device);
}
//------------------------------------------------------------------------------
void oalplus_openal_player::audio_commit(execution_context&) {
    //
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class oalplus_openal_provider
  : public main_ctx_object
  , public hmi_provider {
public:
    oalplus_openal_provider(main_ctx_parent parent)
      : main_ctx_object{"EGLPPrvdr", parent} {}

    auto is_implemented() const noexcept -> bool final;
    auto implementation_name() const noexcept -> string_view final;

    auto is_initialized() -> bool final;
    auto should_initialize(execution_context&) -> bool final;
    auto initialize(execution_context&) -> bool final;
    void update(execution_context&, application&) final;
    void clean_up(execution_context&) final;

    void input_enumerate(
      callable_ref<void(shared_holder<input_provider>)>) final;
    void video_enumerate(
      callable_ref<void(shared_holder<video_provider>)>) final;
    void audio_enumerate(
      callable_ref<void(shared_holder<audio_provider>)>) final;

private:
    oalplus::alc_api _alc_api;

    std::map<identifier, shared_holder<oalplus_openal_player>> _players;
};
//------------------------------------------------------------------------------
auto oalplus_openal_provider::is_implemented() const noexcept -> bool {
    return _alc_api.open_device and _alc_api.close_device and
           _alc_api.create_context and _alc_api.destroy_context and
           _alc_api.make_context_current;
}
//------------------------------------------------------------------------------
auto oalplus_openal_provider::implementation_name() const noexcept
  -> string_view {
    return {"oalplus"};
}
//------------------------------------------------------------------------------
auto oalplus_openal_provider::is_initialized() -> bool {
    return not _players.empty();
}
//------------------------------------------------------------------------------
auto oalplus_openal_provider::should_initialize(execution_context& exec_ctx)
  -> bool {
    for(auto& entry : exec_ctx.options().audio_requirements()) {
        auto& audio_opts = std::get<1>(entry);
        if(audio_opts.has_provider(implementation_name())) {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto oalplus_openal_provider::initialize(execution_context& exec_ctx) -> bool {
    if(ok device{_alc_api.open_device()}) {
        auto& options = exec_ctx.options();
        for(auto& [inst, audio_opts] : options.audio_requirements()) {
            const bool should_create_player =
              audio_opts.has_provider(implementation_name()) and
              (audio_opts.audio_kind() == audio_context_kind::openal);

            if(should_create_player) {
                if(auto player{std::make_shared<oalplus_openal_player>(
                     *this, _alc_api)}) {
                    if(player->initialize(
                         exec_ctx,
                         std::move(device),
                         inst,
                         options,
                         audio_opts)) {
                        _players[inst] = std::move(player);
                    } else {
                        player->clean_up();
                    }
                }
            }
        }
        return true;
    }
    exec_ctx.log_error("ALC is context is not supported");
    return false;
}
//------------------------------------------------------------------------------
void oalplus_openal_provider::update(execution_context&, application&) {}
//------------------------------------------------------------------------------
void oalplus_openal_provider::clean_up(execution_context&) {
    for(auto& entry : _players) {
        entry.second->clean_up();
    }
}
//------------------------------------------------------------------------------
void oalplus_openal_provider::input_enumerate(
  callable_ref<void(shared_holder<input_provider>)>) {}
//------------------------------------------------------------------------------
void oalplus_openal_provider::video_enumerate(
  callable_ref<void(shared_holder<video_provider>)>) {}
//------------------------------------------------------------------------------
void oalplus_openal_provider::audio_enumerate(
  callable_ref<void(shared_holder<audio_provider>)> handler) {
    for(auto& p : _players) {
        handler(p.second);
    }
}
//------------------------------------------------------------------------------
auto make_oalplus_openal_provider(main_ctx_parent parent)
  -> shared_holder<hmi_provider> {
    return {std::make_shared<oalplus_openal_provider>(parent)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
