#include <noggit/camera.hpp>

namespace noggit
{
  camera::camera ( math::vector_3d const& position
                 , math::degrees yaw_
                 , math::degrees pitch_
                 )
    : position (position)
    , move_speed (200.6f)
    , _roll (0.0f)
    , _yaw (0.f)
    , _pitch (0.f)
    , _fov (math::degrees (54.f))
  {
    //! \note ensure ranges
    yaw (yaw_);
    pitch (pitch_);
  }

  math::degrees camera::yaw() const
  {
    return _yaw;
  }

  math::degrees camera::yaw (math::degrees value)
  {
    _yaw = value;

    //! [-180, 180)
    while (_yaw._ >= 180.0f)
      _yaw._ -= 360.0f;
    while (_yaw._ < -180.0f)
      _yaw._ += 360.0f;

    return _yaw;
  }

  void camera::add_to_yaw (math::degrees value)
  {
    yaw (math::degrees (_yaw._ - value._));
  }

  math::degrees camera::pitch() const
  {
    return _pitch;
  }

  math::degrees camera::pitch (math::degrees value)
  {
    _pitch._ = std::max (-80.f, std::min (80.f, value._));

    return _pitch;
  }

  void camera::add_to_pitch (math::degrees value)
  {
    pitch (math::degrees (_pitch._ - value._));
  }

  math::radians camera::fov() const
  {
    return _fov;
  }

  math::vector_3d camera::look_at() const
  {
    return position + direction();
  }

  math::vector_3d camera::direction() const
  {
    math::vector_3d const forward (1.0f, 0.0f, 0.0f);

    return ( math::matrix_4x4 ( math::matrix_4x4::rotation_yzx
                              , math::degrees::vec3 (_roll, _yaw, _pitch)
                              )
           * forward
           ).normalize();
  }

  void camera::move_forward (float sign, float dt)
  {
    position += direction() * sign * move_speed * dt;
  }

  void camera::move_horizontal (float sign, float dt)
  {
    math::vector_3d const up (0.0f, 1.0f, 0.0f);
    math::vector_3d const right ( (direction() % up).normalize());

    position += right * sign * move_speed * dt;
  }

  math::matrix_4x4 camera::view_matrix() const
  {
    math::vector_3d up (0.0f, 1.0f, 0.0f);
    math::vector_3d const dir ((position - look_at()).normalized());
    math::vector_3d const right ((dir % up).normalized());
    up = dir % right;

    return math::matrix_4x4(
        right.x, up.x, dir.x, 0.f,
        right.y, up.y, dir.y, 0.f,
        right.z, up.z, dir.z, 0.f,
        -right.dot(position), -up.dot(position), -dir.dot(position), 1.f
      );
  }

  void camera::move_vertical (float sign, float dt)
  {
    math::vector_3d const up (0.0f, 1.0f, 0.0f);

    position += up * sign * move_speed * dt;
  }
}
