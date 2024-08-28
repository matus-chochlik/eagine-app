/// @file
///
/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
export module eagine.app:types;

import std;
import eagine.core.types;
import eagine.core.reflection;
import eagine.oglplus;

namespace eagine {
namespace app {
//------------------------------------------------------------------------------
/// @brief 2d floating-point vector type.
/// @ingroup application
export using vec2 = oglplus::vector<float, 2>;

/// @brief 3d floating-point vector type.
/// @ingroup application
export using vec3 = oglplus::vector<float, 3>;

/// @brief 4d floating-point vector type.
/// @ingroup application
export using vec4 = oglplus::vector<float, 4>;

/// @brief 3x3 floating-point matrix type.
/// @ingroup application
export using mat2 = oglplus::tmat<float, 2, 2, true>;

/// @brief 3x3 floating-point matrix type.
/// @ingroup application
export using mat3 = oglplus::tmat<float, 3, 3, true>;

/// @brief 4x4 floating-point matrix type.
/// @ingroup application
export using mat4 = oglplus::tmat<float, 4, 4, true>;
//------------------------------------------------------------------------------
/// @brief Video / graphics rendering context kind.
/// @ingroup application
export enum class video_context_kind : std::uint8_t {
    /// @brief OpenGL© (or OpenGL|ES) context.
    opengl,
    /// @brief OpenVG© context.
    openvg
};
//------------------------------------------------------------------------------
/// @brief Audio / sound playback and recording context kind.
/// @ingroup application
export enum class audio_context_kind : std::uint8_t {
    /// @brief OpenAL© context.
    openal
};
//------------------------------------------------------------------------------
/// @brief Video rendering device kind.
/// @ingroup application
export enum class video_device_kind : std::uint8_t {
    /// @brief No preferrence.
    dont_care,
    /// @brief Hardware rendering device.
    hardware,
    /// @brief Software rendering device.
    software
};
//------------------------------------------------------------------------------
/// @brief Pixel data type used to store frame dump image data.
/// @ingroup application
/// @see framedump_pixel_format
export enum class framedump_data_type : std::uint8_t {
    /// @brief None, not doing frame dump render run.
    none,
    /// @brief Floating-point pixel data.
    float_type,
    /// @brief Byte pixel data.
    byte_type
};
//------------------------------------------------------------------------------
/// @brief Pixel data format of frame dump image data.
/// @ingroup application
/// @see framedump_data_type
export enum class framedump_pixel_format : std::uint8_t {
    /// @brief None.
    none,
    /// @brief Red, green, blue, alpha color channel data.
    rgba,
    /// @brief Depth buffer data.
    depth,
    /// @brief Stencil buffer data.
    stencil
};
} // namespace app
//------------------------------------------------------------------------------
export template <>
struct enumerator_traits<app::video_context_kind> {
    static constexpr auto mapping() noexcept {
        return enumerator_map_type<app::video_context_kind, 2>{
          {{"opengl", app::video_context_kind::opengl},
           {"openvg", app::video_context_kind::openvg}}};
    }
};

export template <>
struct enumerator_traits<app::audio_context_kind> {
    static constexpr auto mapping() noexcept {
        return enumerator_map_type<app::audio_context_kind, 1>{
          {{"openal", app::audio_context_kind::openal}}};
    }
};

export template <>
struct enumerator_traits<app::video_device_kind> {
    static constexpr auto mapping() noexcept {
        return enumerator_map_type<app::video_device_kind, 3>{
          {{"dont_care", app::video_device_kind::dont_care},
           {"hardware", app::video_device_kind::hardware},
           {"software", app::video_device_kind::software}}};
    }
};

export template <>
struct enumerator_traits<app::framedump_data_type> {
    static constexpr auto mapping() noexcept {
        return enumerator_map_type<app::framedump_data_type, 3>{
          {{"none", app::framedump_data_type::none},
           {"float", app::framedump_data_type::float_type},
           {"byte", app::framedump_data_type::byte_type}}};
    }
};

export template <>
struct enumerator_traits<app::framedump_pixel_format> {
    static constexpr auto mapping() noexcept {
        return enumerator_map_type<app::framedump_pixel_format, 4>{
          {{"none", app::framedump_pixel_format::none},
           {"rgba", app::framedump_pixel_format::rgba},
           {"depth", app::framedump_pixel_format::depth},
           {"stencil", app::framedump_pixel_format::stencil}}};
    }
};
//------------------------------------------------------------------------------
} // namespace eagine
