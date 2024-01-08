/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
#include "providers.hpp"

namespace eagine::app {
//------------------------------------------------------------------------------
// file_io
//------------------------------------------------------------------------------
class file_io final : public msgbus::source_blob_io {
public:
    file_io(std::filesystem::path path);

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    std::ifstream _file;
    span_size_t _size{0};
};
//------------------------------------------------------------------------------
file_io::file_io(std::filesystem::path path)
  : _file{path} {
    _file.seekg(0, std::ios::end);
    _size = static_cast<span_size_t>(_file.tellg());
}
//------------------------------------------------------------------------------
auto file_io::total_size() noexcept -> span_size_t {
    return _size;
}
//------------------------------------------------------------------------------
auto file_io::fetch_fragment(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    _file.seekg(offs, std::ios::beg);
    return limit_cast<span_size_t>(
      read_from_stream(_file, head(dst, _size - offs)).gcount());
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class file_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    file_provider(const provider_parameters&);

    auto has_resource(const url& locator) noexcept -> bool final;

    auto get_resource_io(const url& locator)
      -> unique_holder<msgbus::source_blob_io> final;

    void for_each_locator(
      callable_ref<void(string_view) noexcept>) noexcept final;

private:
    auto _has_resource(
      const std::filesystem::path&,
      const std::filesystem::path&,
      const url& locator) noexcept -> bool;

    auto _get_resource_io(
      const std::filesystem::path&,
      const std::filesystem::path&,
      const url& locator) noexcept -> unique_holder<msgbus::source_blob_io>;

    void _for_each_locator(
      const std::filesystem::path&,
      const std::filesystem::path&,
      callable_ref<void(string_view) noexcept>) noexcept;

    std::string _hostname;
    std::vector<std::filesystem::path> _search_paths;
};
//------------------------------------------------------------------------------
file_provider::file_provider(const provider_parameters& params)
  : main_ctx_object{"FilePrvdr", params.parent} {
    std::vector<std::string> paths;
    main_context().config().fetch("app.resource_provider.root_path", paths);
    main_context().config().fetch("app.resource_provider.root_paths", paths);
    for(const auto& path : paths) {
        if(std::filesystem::is_directory(path)) {
            _search_paths.emplace_back(path);
        } else {
            log_warning("'${path}' is not a path to existing directory")
              .arg("path", "FsPath", path);
        }
    }
    log_info("configured resource directories: ${path}")
      .arg_func([&](logger_backend& backend) {
          for(auto& path : _search_paths) {
              backend.add_string("path", "FsPath", path.string());
          }
      });
}
//------------------------------------------------------------------------------
auto file_provider::_has_resource(
  const std::filesystem::path& prefix,
  const std::filesystem::path& path,
  const url& locator) noexcept -> bool {
    if(not std::filesystem::is_symlink(path)) {
        if(std::filesystem::is_regular_file(path)) {
            std::error_code ec{};
            const auto relative{std::filesystem::relative(path, prefix, ec)};
            if(not ec) {
                if(locator.has_path(relative.string())) {
                    return true;
                }
            }
        } else if(std::filesystem::is_directory(path)) {
            for(auto const& dir_entry :
                std::filesystem::directory_iterator{path}) {
                if(_has_resource(prefix, dir_entry, locator)) {
                    return true;
                }
            }
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto file_provider::has_resource(const url& locator) noexcept -> bool {
    for(const auto& search_path : _search_paths) {
        if(_has_resource(search_path, search_path, locator)) {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto file_provider::_get_resource_io(
  const std::filesystem::path& prefix,
  const std::filesystem::path& path,
  const url& locator) noexcept -> unique_holder<msgbus::source_blob_io> {
    if(not std::filesystem::is_symlink(path)) {
        if(std::filesystem::is_regular_file(path)) {
            std::error_code ec{};
            const auto relative{std::filesystem::relative(path, prefix, ec)};
            if(not ec) {
                if(locator.has_path(relative.string())) {
                    return {hold<file_io>, path};
                }
            }
        } else if(std::filesystem::is_directory(path)) {
            for(auto const& dir_entry :
                std::filesystem::directory_iterator{path}) {
                if(auto io{_get_resource_io(prefix, dir_entry, locator)}) {
                    return io;
                }
            }
        }
    }
    return {};
}
//------------------------------------------------------------------------------
auto file_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    for(const auto& search_path : _search_paths) {
        if(auto io{_get_resource_io(search_path, search_path, locator)}) {
            return io;
        }
    }
    return {};
}
//------------------------------------------------------------------------------
void file_provider::_for_each_locator(
  const std::filesystem::path& prefix,
  const std::filesystem::path& path,
  callable_ref<void(string_view) noexcept> callback) noexcept {
    if(not std::filesystem::is_symlink(path)) {
        if(std::filesystem::is_regular_file(path)) {
            std::error_code ec{};
            const auto relative{std::filesystem::relative(path, prefix, ec)};
            if(not ec) {
                callback(
                  std::format("file://{}/{}", _hostname, relative.string()));
            }
        } else if(std::filesystem::is_directory(path)) {
            for(auto const& dir_entry :
                std::filesystem::directory_iterator{path}) {
                _for_each_locator(prefix, dir_entry, callback);
            }
        }
    }
}
//------------------------------------------------------------------------------
void file_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    for(const auto& search_path : _search_paths) {
        _for_each_locator(search_path, search_path, callback);
    }
}
//------------------------------------------------------------------------------
auto provider_file(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<file_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
