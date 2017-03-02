#include <noggit/camera.hpp>

namespace noggit
{

camera::camera(const math::vector_3d& position, math::degrees yaw_, math::degrees pitch_)
    : position(position)
    , move_speed(200.6f)
    , _roll(0.0f)
    , _yaw(0.0f)
    , _pitch(0.0f)
    , _fov (math::degrees (54.f))
{
  add_to_yaw(yaw_);
  add_to_pitch(pitch_);
}

const math::degrees camera::yaw() const
{
  return _yaw;
}

const math::degrees camera::yaw(math::degrees value)
{
  _yaw = value;

  while (_yaw._ > 360.0f)
    _yaw._ -= 360.0f;
  while (_yaw._ < -360.0f)
    _yaw._ += 360.0f;

  return _yaw;
}

void camera::add_to_yaw(math::degrees value)
{
  yaw(math::degrees(_yaw._ - value._));
}

const math::degrees camera::pitch() const
{
  return _pitch;
}

const math::degrees camera::pitch(math::degrees value)
{
  _pitch._ = std::max (-80.f, std::min (80.f, value._));

  return _pitch;
}

void camera::add_to_pitch(math::degrees value)
{
  pitch(math::degrees(_pitch._ - value._));
}

  math::radians camera::fov() const
  {
    return _fov;
  }

const math::vector_3d camera::look_at() const
{
  return position + direction();
}

const math::vector_3d camera::direction() const
{
  const math::vector_3d forward (1.0f, 0.0f, 0.0f);

  return ( math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                            , math::degrees::vec3 ( _roll
                                                  , _yaw
                                                  , _pitch
                                                  )
                            )
          * forward ).normalize();
}

void camera::move_forward(float sign, float dt)
{
  position += direction() * sign * move_speed * dt;
}

void camera::move_horizontal(float sign, float dt)
{
  const math::vector_3d up (0.0f, 1.0f, 0.0f);
  const math::vector_3d right ((direction() % up).normalize());

  position += right * sign * move_speed * dt;
}

void camera::move_vertical(float sign, float dt)
{
  const math::vector_3d up (0.0f, 1.0f, 0.0f);

  position += up * sign * move_speed * dt;
}

}
