/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///

#include <eagine/app/context.hpp>
#include <eagine/extract.hpp>
#include <eagine/integer_range.hpp>
#include <eagine/logging/type/yes_no_maybe.hpp>
#include <eagine/valid_if/decl.hpp>

#include <eagine/oalplus/al.hpp>
#include <eagine/oalplus/al_api.hpp>
#include <eagine/oalplus/alc_api.hpp>
#include <eagine/oalplus/alc_api/context_attribs.hpp>

namespace eagine::app {
//------------------------------------------------------------------------------
// playback device
//------------------------------------------------------------------------------
class oalplus_openal_player
  : public main_ctx_object
  , public audio_provider {
public:
    oalplus_openal_player(main_ctx_parent parent, oalplus::alc_api& alc)
      : main_ctx_object{EAGINE_ID(OpenALC), parent}
      , _alc_api{alc} {}

    auto get_context_attribs(
      execution_context&,
      const launch_options&,
      const audio_options&) const -> oalplus::context_attributes;

    auto initialize(
      execution_context&,
      const oalplus::device_handle,
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
    oalplus::device_handle _device{};
    oalplus::context_handle _context{};
};
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
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
EAGINE_LIB_FUNC
auto oalplus_openal_player::initialize(
  execution_context& exec_ctx,
  const oalplus::device_handle device,
  const identifier id,
  const launch_options& opts,
  const audio_options& audio_opts) -> bool {
    _instance_id = id;
    _device = device;
    const auto& [alc, ALC] = _alc_api;

    if(const ok context{alc.create_context(
         _device, get_context_attribs(exec_ctx, opts, audio_opts))}) {
        _context = context;
        return true;
    } else {
        exec_ctx.log_error("failed to create AL context")
          .arg(EAGINE_ID(message), (!context).message());
    }

    return false;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_player::clean_up() {
    if(_device) {
        if(_context) {
            _alc_api.destroy_context(_device, _context);
        }
        _alc_api.close_device(_device);
    }
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto oalplus_openal_player::audio_kind() const noexcept -> audio_context_kind {
    return audio_context_kind::openal;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto oalplus_openal_player::instance_id() const noexcept -> identifier {
    return _instance_id;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_player::parent_context_changed(const audio_context&) {}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_player::audio_begin(execution_context&) {
    _alc_api.make_context_current(_device, _context);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_player::audio_end(execution_context&) {
    _alc_api.make_context_current.none(_device);
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
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
      : main_ctx_object{EAGINE_ID(EGLPPrvdr), parent} {}

    auto is_implemented() const noexcept -> bool final;
    auto implementation_name() const noexcept -> string_view final;

    auto is_initialized() -> bool final;
    auto should_initialize(execution_context&) -> bool final;
    auto initialize(execution_context&) -> bool final;
    void update(execution_context&) final;
    void clean_up(execution_context&) final;

    void input_enumerate(
      callable_ref<void(std::shared_ptr<input_provider>)>) final;
    void video_enumerate(
      callable_ref<void(std::shared_ptr<video_provider>)>) final;
    void audio_enumerate(
      callable_ref<void(std::shared_ptr<audio_provider>)>) final;

private:
    oalplus::alc_api _alc_api;

    std::map<identifier, std::shared_ptr<oalplus_openal_player>> _players;
};
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto oalplus_openal_provider::is_implemented() const noexcept -> bool {
    return _alc_api.open_device && _alc_api.close_device &&
           _alc_api.create_context && _alc_api.destroy_context &&
           _alc_api.make_context_current;
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto oalplus_openal_provider::implementation_name() const noexcept
  -> string_view {
    return {"oalplus"};
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
auto oalplus_openal_provider::is_initialized() -> bool {
    return !_players.empty();
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
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
EAGINE_LIB_FUNC
auto oalplus_openal_provider::initialize(execution_context& exec_ctx) -> bool {
    if(const ok device{_alc_api.open_device()}) {
        auto& options = exec_ctx.options();
        for(auto& [inst, audio_opts] : options.audio_requirements()) {
            const bool should_create_player =
              audio_opts.has_provider(implementation_name()) &&
              (audio_opts.audio_kind() == audio_context_kind::openal);

            if(should_create_player) {
                if(auto player{std::make_shared<oalplus_openal_player>(
                     *this, _alc_api)}) {
                    if(extract(player).initialize(
                         exec_ctx, device, inst, options, audio_opts)) {
                        _players[inst] = std::move(player);
                    } else {
                        extract(player).clean_up();
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
EAGINE_LIB_FUNC
void oalplus_openal_provider::update(execution_context&) {}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_provider::clean_up(execution_context&) {
    for(auto& entry : _players) {
        entry.second->clean_up();
    }
}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_provider::input_enumerate(
  callable_ref<void(std::shared_ptr<input_provider>)>) {}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_provider::video_enumerate(
  callable_ref<void(std::shared_ptr<video_provider>)>) {}
//------------------------------------------------------------------------------
EAGINE_LIB_FUNC
void oalplus_openal_provider::audio_enumerate(
  callable_ref<void(std::shared_ptr<audio_provider>)> handler) {
    for(auto& p : _players) {
        handler(p.second);
    }
}
//------------------------------------------------------------------------------
auto make_oalplus_openal_provider(main_ctx_parent parent)
  -> std::shared_ptr<hmi_provider> {
    return {std::make_shared<oalplus_openal_provider>(parent)};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
