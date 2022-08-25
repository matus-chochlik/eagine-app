/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module eagine.app;

import eagine.core.types;
import eagine.core.memory;
import eagine.core.reflection;
import eagine.core.runtime;
import eagine.core.main_ctx;
import <fstream>;
import <iomanip>;
import <iostream>;
import <sstream>;

namespace eagine::app {
//------------------------------------------------------------------------------
class raw_framedump
  : public main_ctx_object
  , public framedump {
public:
    raw_framedump(main_ctx_parent parent)
      : main_ctx_object("RawFrmDump", parent) {}

    auto initialize(execution_context&, const video_options&) -> bool final;

    auto get_buffer(const span_size_t size) -> memory::block final;

    auto dump_frame(
      const long frame_number,
      const int width,
      const int height,
      const int elements,
      const span_size_t element_size,
      const framedump_pixel_format,
      const framedump_data_type,
      memory::block data) -> bool final;

private:
    auto _write_to_file(const std::string&, const memory::const_block) -> bool;

    string_view _prefix;
    std::string _feedback;
    memory::buffer _pixeldata;
    memory::buffer _compressed;
};
//------------------------------------------------------------------------------
auto raw_framedump::initialize(execution_context&, const video_options& opts)
  -> bool {
    _prefix = opts.framedump_prefix();
    log_info("frame dump prefix: ${prefix}").arg("prefix", "FsPath", _prefix);
    return true;
}
//------------------------------------------------------------------------------
auto raw_framedump::get_buffer(const span_size_t size) -> memory::block {
    return head(cover(_pixeldata.ensure(size)), size);
}
//------------------------------------------------------------------------------
inline auto raw_framedump::_write_to_file(
  const std::string& path,
  const memory::const_block data) -> bool {
    std::ofstream out{path};
    if(!write_to_stream(out, data).flush().good()) {
        return false;
    }
    std::cout << path << std::endl << std::flush;
    return std::getline(std::cin, _feedback).good() && (_feedback == path);
}
//------------------------------------------------------------------------------
auto raw_framedump::dump_frame(
  const long frame_number,
  const int width,
  const int height,
  const int elements,
  [[maybe_unused]] const span_size_t element_size,
  const framedump_pixel_format format,
  const framedump_data_type type,
  [[maybe_unused]] memory::block data) -> bool {

    std::stringstream path;
    path << _prefix << '-' << width << 'x' << height << 'x' << elements << '-'
         << enumerator_name(
              format,
              std::type_identity<framedump_pixel_format>(),
              from_value_tree)
         << '-'
         << enumerator_name(
              type, std::type_identity<framedump_data_type>(), from_value_tree)
         << '-' << std::setfill('0') << std::setw(6) << frame_number;

    if(const auto packed{main_context().compressor().compress(
         data, _compressed, data_compression_level::highest)}) {
        path << ".zlib";
        return _write_to_file(path.str(), skip(packed, 1));
    }
    return _write_to_file(path.str(), data);
}
//------------------------------------------------------------------------------
auto make_raw_framedump(main_ctx_parent parent) -> std::shared_ptr<framedump> {
    return std::make_shared<raw_framedump>(parent);
}
//------------------------------------------------------------------------------
} // namespace eagine::app
