// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>

namespace noggit
{
//! \todo there should be a seperate class for tile mode
class camera
{
public:
  camera(const math::vector_3d& position, float yaw_, float pitch_);

  const float yaw() const;
  const float yaw(float value);
  void add_to_yaw(float value);

  const float pitch() const;
  const float pitch(float value);
  void add_to_pitch(float value);

  const math::vector_3d look_at() const;
  const math::vector_3d direction() const;

  void move_forward(float sign, float dt);
  void move_horizontal(float sign, float dt);
  void move_vertical(float sign, float dt);

  math::vector_3d position;
  float move_speed;

private:
  float _roll; // this is not used currently
  float _yaw;
  float _pitch;
    
};

}

