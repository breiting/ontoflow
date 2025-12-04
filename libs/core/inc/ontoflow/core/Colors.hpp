#pragma once

#include <array>
#include <glm/vec4.hpp>

namespace of::core {

/**
 * @brief Nord Theme Colors
 * @see https://www.nordtheme.com/docs/colors-and-palettes
 */
namespace nord {

constexpr glm::vec4 Nord0 = {0.180f, 0.204f, 0.251f, 1.0f};  ///< Polar Night (Darkest) #2E3440
constexpr glm::vec4 Nord1 = {0.216f, 0.243f, 0.306f, 1.0f};  ///< Polar Night (Darker) #3B4252
constexpr glm::vec4 Nord2 = {0.263f, 0.290f, 0.353f, 1.0f};  ///< Polar Night (Dark) #434C5E
constexpr glm::vec4 Nord3 = {0.302f, 0.333f, 0.396f, 1.0f};  ///< Polar Night (Base) #4C566A

constexpr glm::vec4 Nord4 = {0.824f, 0.843f, 0.871f, 1.0f};  ///< Snow Storm (Light) #D8DEE9
constexpr glm::vec4 Nord5 = {0.886f, 0.902f, 0.925f, 1.0f};  ///< Snow Storm (Lighter) #E5E9F0
constexpr glm::vec4 Nord6 = {0.925f, 0.933f, 0.945f, 1.0f};  ///< Snow Storm (Lightest) #ECEFF4

constexpr glm::vec4 Nord7 = {0.529f, 0.773f, 0.894f, 1.0f};   ///< Frost (Teal) #8FBCBB
constexpr glm::vec4 Nord8 = {0.557f, 0.820f, 0.882f, 1.0f};   ///< Frost (Cyan) #88C0D0
constexpr glm::vec4 Nord9 = {0.486f, 0.816f, 0.878f, 1.0f};   ///< Frost (Blue) #81A1C1
constexpr glm::vec4 Nord10 = {0.396f, 0.596f, 0.733f, 1.0f};  ///< Frost (Dark Blue) #5E81AC

constexpr glm::vec4 Nord11 = {0.882f, 0.463f, 0.439f, 1.0f};  ///< Aurora (Red) #BF616A
constexpr glm::vec4 Nord12 = {0.933f, 0.686f, 0.369f, 1.0f};  ///< Aurora (Orange) #D08770
constexpr glm::vec4 Nord13 = {0.949f, 0.765f, 0.408f, 1.0f};  ///< Aurora (Yellow) #EBCB8B
constexpr glm::vec4 Nord14 = {0.541f, 0.745f, 0.482f, 1.0f};  ///< Aurora (Green) #A3BE8C
constexpr glm::vec4 Nord15 = {0.741f, 0.612f, 0.894f, 1.0f};  ///< Aurora (Purple) #B48EAD

constexpr std::array<glm::vec4, 16> Palette = {Nord0, Nord1, Nord2,  Nord3,  Nord4,  Nord5,  Nord6,  Nord7,
                                               Nord8, Nord9, Nord10, Nord11, Nord12, Nord13, Nord14, Nord15};

}  // namespace nord

/**
 * @brief Ghibli-Inspired Color Palette (soft, natural, pastel)
 */
namespace ghibli {

constexpr glm::vec4 Sky = {0.702f, 0.847f, 0.914f, 1.0f};       ///< #B3D8E9
constexpr glm::vec4 Leaf = {0.478f, 0.706f, 0.478f, 1.0f};      ///< #7AB47A
constexpr glm::vec4 Earth = {0.584f, 0.447f, 0.345f, 1.0f};     ///< #95725B
constexpr glm::vec4 Flower = {0.973f, 0.702f, 0.714f, 1.0f};    ///< #F8B3B6
constexpr glm::vec4 Sunlight = {0.980f, 0.902f, 0.588f, 1.0f};  ///< #FAE696
constexpr glm::vec4 Shadow = {0.251f, 0.278f, 0.282f, 1.0f};    ///< #404748

constexpr std::array<glm::vec4, 6> Palette = {Sky, Leaf, Earth, Flower, Sunlight, Shadow};

}  // namespace ghibli

}  // namespace of::core
