// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/matrix_4x4.hpp>

namespace noggit
{
  //! \todo there should be a seperate class for tile mode
  class camera
  {
  public:
    camera ( math::vector_3d const& position
           , math::degrees yaw_
           , math::degrees pitch_
           );

    math::degrees yaw() const;
    math::degrees yaw (math::degrees);
    void add_to_yaw (math::degrees);

    math::degrees pitch() const;
    math::degrees pitch (math::degrees);
    void add_to_pitch (math::degrees);

    math::radians fov() const;

    math::vector_3d look_at() const;
    math::vector_3d direction() const;

    math::matrix_4x4 look_at_matrix() const;

    void move_forward (float sign, float dt);
    void move_horizontal (float sign, float dt);
    void move_vertical (float sign, float dt);

    math::vector_3d position;
    float move_speed;

  private:
    math::degrees _roll; // this is not used currently
    math::degrees _yaw;
    math::degrees _pitch;
    math::radians _fov;
  };
}
