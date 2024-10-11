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
import eagine.core.container;
import eagine.core.reflection;
import eagine.core.value_tree;
import eagine.core.utility;
import eagine.core.runtime;
import eagine.core.main_ctx;
import eagine.msgbus;

namespace eagine::app::exp {
//------------------------------------------------------------------------------
// resource_interface::loader
//------------------------------------------------------------------------------
void resource_interface::loader::stream_data_appended(
  const msgbus::blob_stream_chunk&) noexcept {}
//------------------------------------------------------------------------------
void resource_interface::loader::stream_finished(identifier_t) noexcept {}
//------------------------------------------------------------------------------
void resource_interface::loader::stream_cancelled(identifier_t) noexcept {}
//------------------------------------------------------------------------------
void resource_interface::loader::resource_loaded(
  const resource_interface::load_info&) noexcept {}
//------------------------------------------------------------------------------
void resource_interface::loader::resource_cancelled(
  const resource_interface::load_info&) noexcept {}
//------------------------------------------------------------------------------
auto resource_interface::loader::_set_request_id(identifier_t req_id) noexcept
  -> identifier_t {
    _request_id = req_id;
    return _request_id;
}
//------------------------------------------------------------------------------
void resource_interface::loader::_notify_loaded(
  resource_loader& res_loader) noexcept {
    res_loader._handle_resource_loaded(_request_id, _params.locator, _resource);
}
//------------------------------------------------------------------------------
void resource_interface::loader::_notify_cancelled(
  resource_loader& res_loader) noexcept {
    res_loader._handle_resource_cancelled(
      _request_id, _params.locator, _resource);
}
//------------------------------------------------------------------------------
// resource_loader
//------------------------------------------------------------------------------
resource_loader::resource_loader(msgbus::endpoint& bus)
  : base{bus} {
    connect<&resource_loader::_handle_preparation_progressed>(
      this, blob_preparation_progressed);
    connect<&resource_loader::_handle_stream_data_appended>(
      this, blob_stream_data_appended);
    connect<&resource_loader::_handle_stream_finished>(
      this, blob_stream_finished);
    connect<&resource_loader::_handle_stream_cancelled>(
      this, blob_stream_cancelled);
}
//------------------------------------------------------------------------------
auto resource_loader::update_and_process_all() noexcept -> work_done {
    some_true something_done{base::update_and_process_all()};

    // TODO
    return something_done;
}
//------------------------------------------------------------------------------
auto resource_loader::has_pending_resources() const noexcept -> bool {
    return not _pending.empty() or not _consumer.empty();
}
//------------------------------------------------------------------------------
auto resource_loader::add_consumer(
  identifier_t request_id,
  const std::shared_ptr<resource_interface::loader>& l) noexcept
  -> resource_loader& {
    _consumer[request_id] = {l};
    return *this;
}
//------------------------------------------------------------------------------
void resource_loader::_handle_preparation_progressed(
  identifier_t blob_id,
  float) noexcept {
    //
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_data_appended(
  const msgbus::blob_stream_chunk& chunk) noexcept {
    if(const auto found{find(_consumer, chunk.request_id)}) {
        if(const auto& loader{*found}) {
            loader->stream_data_appended(chunk);
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_finished(identifier_t request_id) noexcept {
    if(const auto found{find(_consumer, request_id)}) {
        if(const auto& loader{*found}) {
            loader->stream_finished(request_id);
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(
  identifier_t request_id) noexcept {
    if(const auto found{find(_consumer, request_id)}) {
        if(const auto& loader{*found}) {
            loader->stream_cancelled(request_id);
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_resource_loaded(
  identifier_t request_id,
  const url& locator,
  resource_interface& resource) noexcept {
    const resource_interface::load_info info{
      locator, request_id, resource.kind()};

    if(const auto found{find(_consumer, request_id)}) {
        if(const auto& loader{*found}) {
            loader->resource_loaded(info);
        }
    }
    resource_loaded(info);
}
//------------------------------------------------------------------------------
void resource_loader::_handle_resource_cancelled(
  identifier_t request_id,
  const url& locator,
  const resource_interface& resource) noexcept {
    const resource_interface::load_info info{
      locator, request_id, resource.kind()};

    if(const auto found{find(_consumer, request_id)}) {
        if(const auto& loader{*found}) {
            loader->resource_cancelled(info);
        }
    }
    resource_cancelled(info);
}
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
