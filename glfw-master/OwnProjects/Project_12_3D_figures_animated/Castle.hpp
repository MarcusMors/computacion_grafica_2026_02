#ifndef __CASTLE_HPP__
#define __CASTLE_HPP__


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


    keep_ = &add(Box{ { 1.8f, 3.2f, 1.8f },
                      { colors::stone, colors::stone, colors::grey, colors::grey, colors::grey, colors::grey } },
      { 0.0f, 1.6f, 0.0f });
    add(Pyramid{ 2.4f, 1.6f, 2.4f, colors::blue }, { 0.0f, 3.2f + 0.8f, 0.0f });

    // gate: a flat dark box standing slightly out of the front wall
    add(Box{ { 0.9f, 1.1f, 0.1f }, colors::brown }, { 0.0f, 0.55f, half + 0.22f });
  }

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

  const std::vector<Tower *> &towers() const { return towers_; }

  const Figure &keep() const { return *keep_; }

private:
  std::vector<Tower *> towers_;
  Figure *keep_{ nullptr };
};

#endif// __CASTLE_HPP__
