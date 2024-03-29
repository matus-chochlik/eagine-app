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

namespace eagine::app {
//------------------------------------------------------------------------------
/// @brief 2d floating-point vector type.
/// @ingroup application
export using vec2 = oglplus::tvec<float, 2, true>;

/// @brief 3d floating-point vector type.
/// @ingroup application
export using vec3 = oglplus::tvec<float, 3, true>;

/// @brief 4d floating-point vector type.
/// @ingroup application
export using vec4 = oglplus::tvec<float, 4, true>;

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

export template <typename Selector>
constexpr auto enumerator_mapping(
  const std::type_identity<video_context_kind>,
  const Selector) noexcept {
    return enumerator_map_type<video_context_kind, 2>{
      {{"opengl", video_context_kind::opengl},
       {"openvg", video_context_kind::openvg}}};
}
//------------------------------------------------------------------------------
/// @brief Audio / sound playback and recodring context kind.
/// @ingroup application
export enum class audio_context_kind : std::uint8_t {
    /// @brief OpenAL© context.
    openal
};

export template <typename Selector>
constexpr auto enumerator_mapping(
  const std::type_identity<audio_context_kind>,
  const Selector) noexcept {
    return enumerator_map_type<audio_context_kind, 1>{
      {{"openal", audio_context_kind::openal}}};
}
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

export template <typename Selector>
constexpr auto enumerator_mapping(
  const std::type_identity<video_device_kind>,
  const Selector) noexcept {
    return enumerator_map_type<video_device_kind, 3>{
      {{"dont_care", video_device_kind::dont_care},
       {"hardware", video_device_kind::hardware},
       {"software", video_device_kind::software}}};
}
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

export template <typename Selector>
constexpr auto enumerator_mapping(
  const std::type_identity<framedump_data_type>,
  const Selector) noexcept {
    return enumerator_map_type<framedump_data_type, 3>{
      {{"none", framedump_data_type::none},
       {"float_type", framedump_data_type::float_type},
       {"byte_type", framedump_data_type::byte_type}}};
}

export constexpr auto enumerator_mapping(
  const std::type_identity<framedump_data_type>,
  const from_config_t) noexcept {
    return enumerator_map_type<framedump_data_type, 3>{
      {{"none", framedump_data_type::none},
       {"float", framedump_data_type::float_type},
       {"byte", framedump_data_type::byte_type}}};
}
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

export template <typename Selector>
constexpr auto enumerator_mapping(
  const std::type_identity<framedump_pixel_format>,
  const Selector) noexcept {
    return enumerator_map_type<framedump_pixel_format, 4>{
      {{"none", framedump_pixel_format::none},
       {"rgba", framedump_pixel_format::rgba},
       {"depth", framedump_pixel_format::depth},
       {"stencil", framedump_pixel_format::stencil}}};
}
//------------------------------------------------------------------------------
} // namespace eagine::app
