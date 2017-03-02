// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>

namespace noggit
{
//! \todo there should be a seperate class for tile mode
class camera
{
public:
  camera(const math::vector_3d& position, math::degrees yaw_, math::degrees pitch_);

  const math::degrees yaw() const;
  const math::degrees yaw(math::degrees value);
  void add_to_yaw(math::degrees value);

  const math::degrees pitch() const;
  const math::degrees pitch(math::degrees value);
  void add_to_pitch(math::degrees value);

  math::radians fov() const;

  const math::vector_3d look_at() const;
  const math::vector_3d direction() const;

  void move_forward(float sign, float dt);
  void move_horizontal(float sign, float dt);
  void move_vertical(float sign, float dt);

  math::vector_3d position;
  float move_speed;

private:
  math::degrees _roll; // this is not used currently
  math::degrees _yaw;
  math::degrees _pitch;
  math::radians _fov;
};

}
