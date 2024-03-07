/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
module;

#include <zip.h>

module eagine.app.resource_provider;

import eagine.core;
import eagine.msgbus;
import std;

namespace eagine::app {
//------------------------------------------------------------------------------
// zipped_file
//------------------------------------------------------------------------------
class zipped_file : public main_ctx_object {
public:
    zipped_file(
      main_ctx_parent&,
      std::shared_ptr<::zip_t>,
      string_view name) noexcept;

    auto size() noexcept -> span_size_t;

    auto read(span_size_t offs, memory::block dst) noexcept -> span_size_t;

private:
    auto _get_content_size(::zip_t*, string_view) noexcept -> span_size_t;
    auto _get_content() noexcept -> memory::const_block;

    span_size_t _size;
    main_ctx_buffer _content;
    std::shared_ptr<::zip_t> _archive;
    std::unique_ptr<::zip_file_t, int (*)(::zip_file_t*)> _file;
};
//------------------------------------------------------------------------------
auto zipped_file::_get_content_size(::zip_t* archive, string_view path) noexcept
  -> span_size_t {
    if(archive) {
        ::zip_stat_t s{};
        ::zip_stat_init(&s);
        if(::zip_stat(archive, c_str(path), ZIP_STAT_SIZE, &s) == 0) {
            return span_size(s.size);
        }
    }
    return 0;
}
//------------------------------------------------------------------------------
zipped_file::zipped_file(
  main_ctx_parent& parent,
  std::shared_ptr<::zip_t> archive,
  string_view path) noexcept
  : main_ctx_object{"ZippedFile", parent}
  , _size{_get_content_size(archive.get(), path)}
  , _content{*this}
  , _archive{std::move(archive)}
  , _file{::zip_fopen(_archive.get(), c_str(path), 0), &::zip_fclose} {}
//------------------------------------------------------------------------------
auto zipped_file::_get_content() noexcept -> memory::const_block {
    if(_content.empty()) {
        if(_size > 0) {
            _content.get(_size).resize(_size);
            const auto sz{limit_cast<::zip_uint64_t>(_content.size())};
            if(::zip_fread(_file.get(), _content.data(), sz) != sz) {
                _content.clear();
            }
        }
    }
    return memory::view(_content);
}
//------------------------------------------------------------------------------
auto zipped_file::size() noexcept -> span_size_t {
    return _get_content().size();
}
//------------------------------------------------------------------------------
auto zipped_file::read(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    const auto src{skip(_get_content(), offs)};
    dst = head(dst, src);
    return copy(head(src, dst), dst).size();
}
//------------------------------------------------------------------------------
// zip_archive
//------------------------------------------------------------------------------
class zip_archive : public main_ctx_object {
public:
    zip_archive(main_ctx_parent&, const std::filesystem::path& path, int);

    explicit operator bool() noexcept {
        return bool(_archive);
    }

    auto handle() noexcept {
        return _archive.get();
    }

    auto open_file(string_view path) noexcept -> unique_holder<zipped_file>;

    void for_each_file(
      callable_ref<void(string_view) noexcept> callback) noexcept;

private:
    std::shared_ptr<::zip_t> _archive;
};
//------------------------------------------------------------------------------
zip_archive::zip_archive(
  main_ctx_parent& parent,
  const std::filesystem::path& path,
  int ec = 0)
  : main_ctx_object{"ZipArchive", parent}
  , _archive{::zip_open(path.string().c_str(), 0, &ec), &::zip_close} {}
//------------------------------------------------------------------------------
auto zip_archive::open_file(string_view path) noexcept
  -> unique_holder<zipped_file> {
    return {default_selector, as_parent(), _archive, path};
}
//------------------------------------------------------------------------------
void zip_archive::for_each_file(
  callable_ref<void(string_view) noexcept> callback) noexcept {
    if(_archive) {
        const auto count{::zip_get_num_entries(_archive.get(), 0)};
        for(::zip_int64_t i = 0; i < count; ++i) {
            callback(string_view{
              ::zip_get_name(_archive.get(), i, ZIP_FL_ENC_STRICT)});
        }
    }
}
//------------------------------------------------------------------------------
// zip_archive_io
//------------------------------------------------------------------------------
class zip_archive_io final : public msgbus::source_blob_io {
public:
    zip_archive_io(shared_holder<zipped_file>);

    auto total_size() noexcept -> span_size_t final;

    auto fetch_fragment(span_size_t offs, memory::block dst) noexcept
      -> span_size_t final;

private:
    shared_holder<zipped_file> _file;
};
//------------------------------------------------------------------------------
zip_archive_io::zip_archive_io(shared_holder<zipped_file> file)
  : _file{std::move(file)} {}
//------------------------------------------------------------------------------
auto zip_archive_io::total_size() noexcept -> span_size_t {
    return _file.member(&zipped_file::size).value_or(0);
}
//------------------------------------------------------------------------------
auto zip_archive_io::fetch_fragment(span_size_t offs, memory::block dst) noexcept
  -> span_size_t {
    if(_file) {
        return _file->read(offs, dst);
    }
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
    auto _get_archive(const std::filesystem::path& path) noexcept
      -> optional_reference<zip_archive>;

    auto _search_archive(std::filesystem::path, const url&) noexcept
      -> std::tuple<optional_reference<zip_archive>, std::string>;
    auto _search_archive_file(std::filesystem::path, const url&) noexcept
      -> shared_holder<zipped_file>;

    auto _is_zip_archive(const std::filesystem::path& path) noexcept -> bool;

    auto _has_resource(std::filesystem::path, const url& locator) noexcept
      -> bool;

    void _for_each_locator(
      const std::filesystem::path&,
      const std::filesystem::path&,
      callable_ref<void(string_view) noexcept>) noexcept;

    std::string _hostname;
    filesystem_search_paths _search_paths;
    flat_map<std::filesystem::path, zip_archive> _open_archives;
};
//------------------------------------------------------------------------------
zip_archive_provider::zip_archive_provider(const provider_parameters& params)
  : main_ctx_object{"FilePrvdr", params.parent}
  , _search_paths{"ZipSrchPth", as_parent()} {}
//------------------------------------------------------------------------------
auto zip_archive_provider::_get_archive(
  const std::filesystem::path& path) noexcept
  -> optional_reference<zip_archive> {
    if(auto found{find(_open_archives, path)}) {
        return found.ref();
    }
    if(auto open{zip_archive(as_parent(), path)}) {
        auto pos{_open_archives.emplace(path, std::move(open)).first};
        return {pos->second};
    }
    return {};
}
//------------------------------------------------------------------------------
auto zip_archive_provider::_is_zip_archive(
  const std::filesystem::path& path) noexcept -> bool {
    static const std::array<std::string_view, 2> exts{{".eagizip", ".zip"}};
    for(const auto ext : exts) {
        if(path.extension() == ext) {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto zip_archive_provider::_search_archive(
  std::filesystem::path archive_path,
  const url& locator) noexcept
  -> std::tuple<optional_reference<zip_archive>, std::string> {
    std::string file_path;
    bool found_archive{false};
    for(const auto entry : locator.path()) {
        if(found_archive) {
            if(not file_path.empty()) {
                file_path.append("/");
            }
            append_to(entry, file_path);
        } else {
            archive_path /= to_string(entry);
            if(not std::filesystem::exists(archive_path)) {
                return {};
            }
            if(std::filesystem::is_regular_file(archive_path)) {
                if(not _is_zip_archive(archive_path)) {
                    return {};
                }
                found_archive = true;
            }
        }
    }
    if(found_archive) {
        return {_get_archive(archive_path), std::move(file_path)};
    }
    return {};
}
//------------------------------------------------------------------------------
auto zip_archive_provider::_search_archive_file(
  std::filesystem::path archive_path,
  const url& locator) noexcept -> shared_holder<zipped_file> {
    if(auto [zip, file_path]{_search_archive(archive_path, locator)}; zip) {
        return zip->open_file(file_path);
    }
    return {};
}
//------------------------------------------------------------------------------
auto zip_archive_provider::_has_resource(
  std::filesystem::path archive_path,
  const url& locator) noexcept -> bool {
    if(auto [zip, file_path]{_search_archive(archive_path, locator)}; zip) {
        ::zip_stat_t s{};
        ::zip_stat_init(&s);
        if(::zip_stat(zip->handle(), file_path.c_str(), ZIP_STAT_SIZE, &s) == 0) {
            return s.size > 0;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto zip_archive_provider::has_resource(const url& locator) noexcept -> bool {
    for(const auto& search_path : _search_paths) {
        if(_has_resource(search_path, locator)) {
            return true;
        }
    }
    return false;
}
//------------------------------------------------------------------------------
auto zip_archive_provider::get_resource_io(const url& locator)
  -> unique_holder<msgbus::source_blob_io> {
    for(const auto& search_path : _search_paths) {
        if(auto zip{_search_archive_file(std::move(search_path), locator)}) {
            return {hold<zip_archive_io>, std::move(zip)};
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
        if(std::filesystem::is_regular_file(path) and _is_zip_archive(path)) {
            std::error_code error{};
            const auto relative{std::filesystem::relative(path, prefix, error)};
            if(not error) {
                if(auto zip{_get_archive(path)}) {
                    const auto wrapped_callback{[&](const string_view name) {
                        callback(std::format(
                          "zip://{}/{}/{}",
                          _hostname,
                          relative.string(),
                          std::string_view{name}));
                    }};
                    zip->for_each_file({construct_from, wrapped_callback});
                }
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
