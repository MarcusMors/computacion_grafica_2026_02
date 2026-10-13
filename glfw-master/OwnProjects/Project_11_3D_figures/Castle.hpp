#ifndef __CASTLE_HPP__
#define __CASTLE_HPP__

// ============================================================================
// Castle.hpp — a very naive castle made of boxes and pyramids.
// This file is the worked EXAMPLE of a composite object (see Figure.hpp):
// to change the castle, edit the constructors below and recompile.
//
// Units: the castle stands on the ground plane y = 0, centered on x = z = 0.
//
// A Tower is a Box (body) with a Pyramid (roof) on top.
// The Castle is 4 corner Towers + 4 wall Boxes + the keep (Box + Pyramid)
// + a gate Box.
// ============================================================================

#include <vector>

#include "Figure.hpp"

// ---- Tower: a box with a pyramid roof (origin at the foot of the tower) ----
class Tower : public Composite
{
public:
  explicit Tower(float height = 3.0f, const Vec3 &wall = colors::stone, const Vec3 &roof = colors::crimson)
  {
    const float roof_height = 1.2f;
    add(Box{ { 1.0f, height, 1.0f }, wall }, { 0.0f, height * 0.5f, 0.0f });
    add(Pyramid{ 1.4f, roof_height, 1.4f, roof }, { 0.0f, height + roof_height * 0.5f, 0.0f });
  }
};

// ---- Castle ------------------------------------------------------------------
class Castle : public Composite
{
public:
  Castle()
  {
    constexpr float half = 2.5f;// the walls form a square of side 5

    // 4 corner towers
    for (const float x : { -half, half }) {
      for (const float z : { -half, half }) { add_tower({ x, 0.0f, z }); }
    }

    // 4 walls between the towers
    const Vec3 wall_color = colors::stone;
    add(Box{ { 2 * half, 1.6f, 0.4f }, wall_color }, { 0.0f, 0.8f, half });   // front
    add(Box{ { 2 * half, 1.6f, 0.4f }, wall_color }, { 0.0f, 0.8f, -half });  // back
    add(Box{ { 0.4f, 1.6f, 2 * half }, wall_color }, { half, 0.8f, 0.0f });   // right
    add(Box{ { 0.4f, 1.6f, 2 * half }, wall_color }, { -half, 0.8f, 0.0f });  // left

    // keep in the middle: a Box with one color per face, plus a pyramid roof
    //                      front         back          right         left          top           bottom
    keep_ = &add(Box{ { 1.8f, 3.2f, 1.8f },
                      { colors::stone, colors::stone, colors::grey, colors::grey, colors::grey, colors::grey } },
      { 0.0f, 1.6f, 0.0f });
    add(Pyramid{ 2.4f, 1.6f, 2.4f, colors::blue }, { 0.0f, 3.2f + 0.8f, 0.0f });

    // gate: a flat dark box standing slightly out of the front wall
    add(Box{ { 0.9f, 1.1f, 0.1f }, colors::brown }, { 0.0f, 0.55f, half + 0.22f });
  }

  // Add one more tower (returned so it can be edited). Towers added this way
  // are tracked so remove_last_tower() can take them away again.
  Tower &add_tower(const Vec3 &position)
  {
    Tower &tower = add(Tower{}, position);
    towers_.push_back(&tower);
    return tower;
  }

  // Take the most recently added tower away. False if there is none left.
  bool remove_last_tower()
  {
    if (towers_.empty()) { return false; }
    const bool removed = remove(*towers_.back());
    towers_.pop_back();
    return removed;
  }

  std::size_t tower_count() const { return towers_.size(); }

  // All the towers (corner towers first, then added ones), so each can be
  // animated on its own:  for (Tower *t : castle.towers()) { t->transform... }
  const std::vector<Tower *> &towers() const { return towers_; }

  // The keep: handy as the frame of reference (e.g. use_frame_of(castle.keep())).
  const Figure &keep() const { return *keep_; }

private:
  std::vector<Tower *> towers_;
  Figure *keep_{ nullptr };
};

#endif// __CASTLE_HPP__
