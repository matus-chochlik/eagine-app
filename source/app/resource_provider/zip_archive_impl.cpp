/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt
///
module;

#include <zip.h>

module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// zip_archive
//------------------------------------------------------------------------------
class zip_archive {
public:
    zip_archive(const std::filesystem::path& path, int);

private:
    std::unique_ptr<::zip_t, int (*)(::zip_t*)> _zip;
};
//------------------------------------------------------------------------------
zip_archive::zip_archive(const std::filesystem::path& path, int ec = 0)
  : _zip{::zip_open(path.string().c_str(), 0, &ec), &::zip_close} {}
//------------------------------------------------------------------------------
// zip_archive_io
//------------------------------------------------------------------------------
class zip_archive_io final : public msgbus::source_blob_io {
public:
    zip_archive_io(std::filesystem::path, const url&);

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    span_size_t _size{0};
};
//------------------------------------------------------------------------------
zip_archive_io::zip_archive_io(std::filesystem::path path, const url& locator) {
    (void)path;
    (void)locator;
}
//------------------------------------------------------------------------------
auto zip_archive_io::total_size() noexcept -> span_size_t {
    return _size;
}
//------------------------------------------------------------------------------
auto zip_archive_io::fetch_fragment(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    (void)offs;
    (void)dst;
    return 0;
}
//------------------------------------------------------------------------------
// provider
//------------------------------------------------------------------------------
class zip_archive_provider final
  : public main_ctx_object
  , public resource_provider_interface {
public:
    zip_archive_provider(const provider_parameters&);

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
zip_archive_provider::zip_archive_provider(const provider_parameters& params)
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
auto zip_archive_provider::_has_resource(
  const std::filesystem::path& prefix,
  const std::filesystem::path& path,
  const url& locator) noexcept -> bool {
    if(not std::filesystem::is_symlink(path)) {
        if(std::filesystem::is_regular_file(path)) {
            std::error_code error{};
            const auto relative{std::filesystem::relative(path, prefix, error)};
            if(not error) {
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
auto zip_archive_provider::has_resource(const url& locator) noexcept -> bool {
    for(const auto& search_path : _search_paths) {
        if(_has_resource(search_path, search_path, locator)) {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto zip_archive_provider::_get_resource_io(
  const std::filesystem::path& prefix,
  const std::filesystem::path& path,
  const url& locator) noexcept -> unique_holder<msgbus::source_blob_io> {
    if(not std::filesystem::is_symlink(path)) {
        if(std::filesystem::is_regular_file(path)) {
            std::error_code error{};
            const auto relative{std::filesystem::relative(path, prefix, error)};
            if(not error) {
                if(locator.has_path(relative.string())) {
                    return {hold<zip_archive_io>, path, locator};
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
auto zip_archive_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    for(const auto& search_path : _search_paths) {
        if(auto io{_get_resource_io(search_path, search_path, locator)}) {
            return io;
        }
    }
    return {};
}
//------------------------------------------------------------------------------
void zip_archive_provider::_for_each_locator(
  const std::filesystem::path& prefix,
  const std::filesystem::path& path,
  callable_ref<void(string_view) noexcept> callback) noexcept {
    if(not std::filesystem::is_symlink(path)) {
        if(std::filesystem::is_regular_file(path)) {
            std::error_code error{};
            const auto relative{std::filesystem::relative(path, prefix, error)};
            if(not error) {
                callback(
                  std::format("zip://{}/{}", _hostname, relative.string()));
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
void zip_archive_provider::for_each_locator(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    for(const auto& search_path : _search_paths) {
        _for_each_locator(search_path, search_path, callback);
    }
}
//------------------------------------------------------------------------------
auto provider_zip_archive(const provider_parameters& params)
  -> unique_holder<resource_provider_interface> {
    return {hold<zip_archive_provider>, params};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
