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
// resource_loader
//------------------------------------------------------------------------------
resource_loader::resource_loader(msgbus::endpoint& bus)
  : resource_data_consumer_node{bus} {
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
            loader->stream_finished();
        }
    }
}
//------------------------------------------------------------------------------
void resource_loader::_handle_stream_cancelled(
  identifier_t request_id) noexcept {
    if(const auto found{find(_consumer, request_id)}) {
        if(const auto& loader{*found}) {
            loader->stream_cancelled();
        }
    }
}
//------------------------------------------------------------------------------
} // namespace eagine::app::exp
