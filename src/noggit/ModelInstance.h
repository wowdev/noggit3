// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_3d.hpp> // math::vector_3d
#include <noggit/MPQ.h> // MPQFile
#include <noggit/MapHeaders.h> // ENTRY_MDDF
#include <noggit/ModelManager.h>
#include <noggit/Selection.h>
#include <noggit/tile_index.hpp>
#include <noggit/tool_enums.hpp>
#include <opengl/shader.fwd.hpp>

namespace math { class frustum; }
class Model;
class WMOInstance;

class ModelInstance
{
public:
  constexpr static float min_scale() { return 1.f / 1024.f; };
  constexpr static float max_scale() { return static_cast<float>((1 << 16) - 1) / 1024.f; };

  scoped_model_reference model;

  math::vector_3d pos;
  math::degrees::vec3 dir;
  math::vector_3d light_color = { 1.f, 1.f, 1.f };

  //! \todo  Get this out and do somehow else.
  unsigned int uid;

  float scale = 1.f;

  // used when flag 0x8 is set in wdt
  // longest side of an AABB transformed model's bounding box from the M2 header
  float size_cat;

  explicit ModelInstance(std::string const& filename);
  explicit ModelInstance(std::string const& filename, ENTRY_MDDF const*d);

  ModelInstance(ModelInstance const& other) = default;
  ModelInstance& operator= (ModelInstance const& other) = default;

  ModelInstance (ModelInstance&& other)
    : model (std::move (other.model))
    , pos (other.pos)
    , dir (other.dir)
    , light_color (other.light_color)
    , uid (other.uid)
    , scale (other.scale)
    , size_cat (other.size_cat)
    , _need_recalc_extents(other._need_recalc_extents)
    , _extents(other._extents)
    , _transform_mat_transposed(other._transform_mat_transposed)
    , _transform_mat_inverted(other._transform_mat_inverted)
  {
  }
  ModelInstance& operator= (ModelInstance&& other)
  {
    std::swap (model, other.model);
    std::swap (pos, other.pos);
    std::swap (dir, other.dir);
    std::swap (light_color, other.light_color);
    std::swap (uid, other.uid);
    std::swap (scale, other.scale);
    std::swap (size_cat, other.size_cat);
    std::swap (_need_recalc_extents, other._need_recalc_extents);
    std::swap (_extents, other._extents);
    std::swap(_transform_mat_transposed, other._transform_mat_transposed);
    std::swap(_transform_mat_inverted, other._transform_mat_inverted);
    return *this;
  }

  bool is_a_duplicate_of(ModelInstance const& other);

  void draw_box ( math::matrix_4x4 const& model_view
                , math::matrix_4x4 const& projection
                , bool is_current_selection
                );

  void intersect ( math::matrix_4x4 const& model_view
                 , math::ray const&
                 , selection_result*
                 , int animtime
                 );


  math::matrix_4x4 const& transform_matrix_transposed() const { return _transform_mat_transposed; }

  void resetDirection();

  bool isInsideRect(math::vector_3d rect[2]) const;
  bool is_visible(math::frustum const& frustum, const float& cull_distance, const math::vector_3d& camera, display_mode display);

  virtual math::vector_3d get_pos() const { return pos; }

  void recalcExtents();
  std::vector<math::vector_3d> const& extents();

protected:
  bool _need_recalc_extents = true;
  std::vector<math::vector_3d> _extents = std::vector<math::vector_3d>(2);

  virtual void update_transform_matrix();

  math::matrix_4x4 _transform_mat_transposed = math::matrix_4x4::uninitialized;
  math::matrix_4x4 _transform_mat_inverted = math::matrix_4x4::uninitialized;
};

class wmo_doodad_instance : public ModelInstance
{
public:
  math::quaternion doodad_orientation;
  math::vector_3d world_pos;

  explicit wmo_doodad_instance(std::string const& filename, MPQFile* f);

  wmo_doodad_instance(wmo_doodad_instance const& other) = default;
  wmo_doodad_instance& operator= (wmo_doodad_instance const& other) = default;

  wmo_doodad_instance (wmo_doodad_instance&& other)
    : ModelInstance (other)
    , doodad_orientation (other.doodad_orientation)
    , world_pos (other.world_pos)
    , _need_matrix_update(other._need_matrix_update)
  {
  }

  wmo_doodad_instance& operator= (wmo_doodad_instance&& other)
  {
    ModelInstance::operator= (other);
    std::swap (doodad_orientation, other.doodad_orientation);
    std::swap (world_pos, other.world_pos);
    std::swap (_need_matrix_update, other._need_matrix_update);
    return *this;
  }

  bool need_matrix_update() const { return _need_matrix_update; }
  void update_transform_matrix_wmo(WMOInstance* wmo);

  virtual math::vector_3d get_pos() const { return world_pos; }

protected:
  // to avoid redefining recalcExtents
  virtual void update_transform_matrix() { }

private:
  bool _need_matrix_update = true;
};
