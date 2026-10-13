#ifndef __COLORS_HPP__
#define __COLORS_HPP__

// ============================================================================
// Colors.hpp — a palette of ready-made colors, so you never have to remember
// an rgb triple.
//
//     constexpr Vec3 OUTLINE_COLOR = colors::white;
//     Box wall{ { 1, 2, 1 }, colors::dark_brown };
//     Vec3 mine = colors::rgb(255, 105, 180);        // any color, 0..255 like CSS / paint programs
//
// A color is a Vec3 (r, g, b), each in 0..1 — what OpenGL expects. The values
// follow the standard CSS color names, plus a few extras at the end.
// Add your own: copy a line and change the name and the three numbers.
// ============================================================================

#include "Buffers.hpp"// Vec3

namespace colors {

// 0..255 channels -> 0..1 floats (like picking a color in any paint program).
constexpr Vec3 rgb(int r, int g, int b)
{
  return { static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f };
}

// ---- neutrals ---------------------------------------------------------------
inline constexpr Vec3 white = rgb(255, 255, 255);
inline constexpr Vec3 light_grey = rgb(192, 192, 192);
inline constexpr Vec3 grey = rgb(128, 128, 128);
inline constexpr Vec3 dark_grey = rgb(64, 64, 64);
inline constexpr Vec3 black = rgb(0, 0, 0);
inline constexpr Vec3 gray = grey;// US spelling

// ---- reds / pinks -----------------------------------------------------------
inline constexpr Vec3 red = rgb(255, 0, 0);
inline constexpr Vec3 crimson = rgb(220, 20, 60);
inline constexpr Vec3 dark_red = rgb(139, 0, 0);
inline constexpr Vec3 maroon = rgb(128, 0, 0);
inline constexpr Vec3 salmon = rgb(250, 128, 114);
inline constexpr Vec3 pink = rgb(255, 192, 203);
inline constexpr Vec3 hot_pink = rgb(255, 105, 180);
inline constexpr Vec3 magenta = rgb(255, 0, 255);

// ---- oranges / yellows ------------------------------------------------------
inline constexpr Vec3 orange = rgb(255, 165, 0);
inline constexpr Vec3 dark_orange = rgb(255, 140, 0);
inline constexpr Vec3 gold = rgb(255, 215, 0);
inline constexpr Vec3 yellow = rgb(255, 255, 0);

// ---- greens -----------------------------------------------------------------
inline constexpr Vec3 lime = rgb(0, 255, 0);
inline constexpr Vec3 green = rgb(0, 128, 0);
inline constexpr Vec3 forest_green = rgb(34, 139, 34);
inline constexpr Vec3 dark_green = rgb(0, 100, 0);
inline constexpr Vec3 olive = rgb(128, 128, 0);

// ---- blues / cyans ----------------------------------------------------------
inline constexpr Vec3 cyan = rgb(0, 255, 255);
inline constexpr Vec3 turquoise = rgb(64, 224, 208);
inline constexpr Vec3 teal = rgb(0, 128, 128);
inline constexpr Vec3 light_blue = rgb(173, 216, 230);
inline constexpr Vec3 sky_blue = rgb(135, 206, 235);
inline constexpr Vec3 blue = rgb(0, 0, 255);
inline constexpr Vec3 royal_blue = rgb(65, 105, 225);
inline constexpr Vec3 navy = rgb(0, 0, 128);

// ---- purples ----------------------------------------------------------------
inline constexpr Vec3 violet = rgb(238, 130, 238);
inline constexpr Vec3 purple = rgb(128, 0, 128);
inline constexpr Vec3 indigo = rgb(75, 0, 130);

// ---- browns / earth tones ---------------------------------------------------
inline constexpr Vec3 beige = rgb(245, 245, 220);
inline constexpr Vec3 sand = rgb(194, 178, 128);
inline constexpr Vec3 tan = rgb(210, 180, 140);
inline constexpr Vec3 chocolate = rgb(210, 105, 30);
inline constexpr Vec3 brown = rgb(139, 69, 19);
inline constexpr Vec3 dark_brown = rgb(92, 51, 23);
inline constexpr Vec3 stone = rgb(158, 153, 143);// castle walls

}// namespace colors

#endif// __COLORS_HPP__
