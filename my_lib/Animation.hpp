#ifndef __ANIMATION_HPP__
#define __ANIMATION_HPP__

// ============================================================================
// Animation.hpp — tiny time -> value functions. No state, no callbacks: call
// them every frame with the current time and assign the result.
//
//     // breathe between 0.8x and 1.2x, twice per second
//     sphere.transform.scale = Vec3{ 1, 1, 1 } * pulse(time, 2.0f, 0.2f);
//
//     // turn 45 degrees per second about Y
//     tower.transform.rotation_deg.y = spin(time, 45.0f);
//
//     // hop 1.5 units high every 1.2 seconds (this tower started 0.3 s late)
//     tower.transform.position.y = hop(time, 1.2f, 1.5f, 0.3f);
//
//     // heartbeat: a quick "lub-dub" swell, 72 times a minute
//     ball.transform.scale = Vec3{ 1, 1, 1 } * heartbeat(time);
// ============================================================================

#include <cmath>
#include <numbers>

// center + amplitude * sin(2*pi * frequency_hz * time + phase)
// Defaults give a value that oscillates between 0.9 and 1.1 once per second.
inline float pulse(double time, float frequency_hz = 1.0f, float amplitude = 0.1f,
                   float center = 1.0f, float phase_radians = 0.0f)
{
  const float angle = 2.0f * std::numbers::pi_v<float> * frequency_hz * static_cast<float>(time) + phase_radians;
  return center + amplitude * std::sin(angle);
}

// A steadily growing angle in degrees (wrapped to [0, 360) so it never loses
// float precision after long run times).
inline float spin(double time, float degrees_per_second)
{
  return static_cast<float>(std::fmod(time * static_cast<double>(degrees_per_second), 360.0));
}

// A heartbeat scale factor: base + a "lub-dub" (two quick swells, the second
// weaker) followed by a rest, repeating `beats_per_minute` times a minute.
// Each swell is one sine arch squared, sin^2(pi * x), so it starts and ends
// smoothly at `base`; the pause between beats is what makes it non-linear
// (a plain sine would just swell and shrink at an even pace).
//   amplitude  how much the first swell adds on top of `base`
inline float heartbeat(double time, float beats_per_minute = 72.0f, float amplitude = 0.25f, float base = 1.0f)
{
  const double beats_per_second = static_cast<double>(beats_per_minute) / 60.0;
  const double phase = std::fmod(time * beats_per_second, 1.0);// 0..1 within one beat

  // sin^2 arch that is nonzero only for start <= phase < start + width
  auto swell = [&](double start, double width) {
    if (phase < start || phase >= start + width) { return 0.0; }
    const double s = std::sin(std::numbers::pi * (phase - start) / width);
    return s * s;
  };

  const double lub = swell(0.00, 0.22);
  const double dub = swell(0.28, 0.22);
  return base + amplitude * static_cast<float>(lub + 0.6 * dub);
}

// A vertical hop: the height above the ground at `time`, always >= 0.
//
//   height   how high the top of the jump is
//   period   seconds between the start of one hop and the next
//   delay    seconds this object starts late (give each object a different one
//            and they jump independently, not in unison)
//   airtime  fraction (0..1] of the period spent in the air; the rest of the
//            period it rests on the ground. 1 = continuous bouncing.
//
// While in the air the height is a parabola, 4*t*(1-t) for t from 0 to 1:
// that is exactly the path of a thrown object under constant gravity, so it
// looks natural (fast near the ground, slow at the top).
inline float hop(double time, float period, float height, float delay = 0.0f, float airtime = 1.0f)
{
  double t = std::fmod((time - static_cast<double>(delay)) / static_cast<double>(period), 1.0);
  if (t < 0.0) { t += 1.0; }// before `delay` the pattern simply continues backwards
  if (t >= static_cast<double>(airtime)) { return 0.0f; }// resting on the ground
  const double u = t / static_cast<double>(airtime);// 0..1 across the jump
  return height * static_cast<float>(4.0 * u * (1.0 - u));
}

#endif// __ANIMATION_HPP__
