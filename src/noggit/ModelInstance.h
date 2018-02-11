// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_3d.hpp> // math::vector_3d
#include <noggit/MPQ.h> // MPQFile
#include <noggit/MapHeaders.h> // ENTRY_MDDF
#include <noggit/ModelManager.h>
#include <noggit/Selection.h>
#include <noggit/tile_index.hpp>
#include <opengl/shader.fwd.hpp>

namespace math { class frustum; }
class Model;
class WMOInstance;

class ModelInstance
{
public:
  scoped_model_reference model;
  math::vector_3d extents[2];

  math::vector_3d pos;
  math::vector_3d dir;
  math::vector_3d light_color = { 1.f, 1.f, 1.f };

  //! \todo  Get this out and do somehow else.
  unsigned int uid;

  float scale;

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
  {
    std::swap (extents, other.extents);
  }
  ModelInstance& operator= (ModelInstance&& other)
  {
    std::swap (model, other.model);
    std::swap (extents, other.extents);
    std::swap (pos, other.pos);
    std::swap (dir, other.dir);
    std::swap (light_color, other.light_color);
    std::swap (uid, other.uid);
    std::swap (scale, other.scale);
    std::swap (size_cat, other.size_cat);
    return *this;
  }

  void draw_box (bool is_current_selection);

  void intersect ( math::ray const&
                 , selection_result*
                 , int animtime
                 );
  

  math::matrix_4x4 const& transform_matrix_transposed() const { return _transform_mat_transposed; }

  void resetDirection();

  bool isInsideRect(math::vector_3d rect[2]) const;
  virtual bool is_visible(math::frustum const& frustum, const float& cull_distance, const math::vector_3d& camera) const;
  bool cull_by_size_category(const math::vector_3d& camera) const;

  void recalcExtents();

protected:
  void update_transform_matrix();

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
  {
    std::swap (extents, other.extents);
  }
  wmo_doodad_instance& operator= (wmo_doodad_instance&& other)
  {
    ModelInstance::operator= (other);
    std::swap (doodad_orientation, other.doodad_orientation);
    std::swap (world_pos, other.world_pos);
    return *this;
  }

  void update_transform_matrix_wmo(WMOInstance* wmo);

  virtual bool is_visible(math::frustum const& frustum, const float& cull_distance, const math::vector_3d& camera) const;
};
