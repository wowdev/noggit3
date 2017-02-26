#include <noggit/camera.hpp>

namespace noggit
{

camera::camera(const math::vector_3d& position, float yaw_, float pitch_)
    : position(position)
    , _roll(0.0f)
    , _yaw(0.0f)
    , _pitch(0.0f)
    , _fov (math::degrees (54.f))
    , move_speed(200.6f)
{
  add_to_yaw(yaw_);
  add_to_pitch(pitch_);
}

const float camera::yaw() const
{
  return _yaw;
}

const float camera::yaw(float value)
{
  _yaw = value;

  while (_yaw > 360.0f)
    _yaw -= 360.0f;
  while (_yaw < -360.0f)
    _yaw += 360.0f;

  return _yaw;
}

void camera::add_to_yaw(float value)
{
  yaw(_yaw - value);
}

const float camera::pitch() const
{
  return _pitch;
}

const float camera::pitch(float value)
{
  _pitch = std::max (-80.f, std::min (80.f, value));

  return _pitch;
}

void camera::add_to_pitch(float value)
{
  pitch(_pitch - value);
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
                            , math::degrees::vec3 ( math::degrees (_roll)
                                                  , math::degrees (_yaw)
                                                  , math::degrees (_pitch)
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