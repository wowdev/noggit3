// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_3d.hpp> // math::vector_3d
#include <noggit/MPQ.h> // MPQFile
#include <noggit/MapHeaders.h> // ENTRY_MDDF
#include <noggit/ModelManager.h>
#include <noggit/Selection.h>
#include <noggit/tile_index.hpp>

namespace math { class frustum; }
class Model;

class ModelInstance
{
public:
  scoped_model_reference model;
  math::vector_3d extents[2];

  math::vector_3d pos;
  math::vector_3d dir;

  math::quaternion _wmo_orientation;

  //! \todo  Get this out and do somehow else.
  unsigned int d1;

  float sc;

  math::vector_3d ldir;
  math::vector_3d lcol;

  explicit ModelInstance(std::string const& filename);
  explicit ModelInstance(std::string const& filename, MPQFile* f);
  explicit ModelInstance(std::string const& filename, ENTRY_MDDF *d);

  ModelInstance(ModelInstance const& other) = default;
  ModelInstance& operator= (ModelInstance const& other) = default;

  ModelInstance (ModelInstance&& other)
    : model (std::move (other.model))
    // , extents (other.extents)
    , pos (other.pos)
    , dir (other.dir)
    , _wmo_orientation (other._wmo_orientation)
    , d1 (other.d1)
    , sc (other.sc)
    , ldir (other.ldir)
    , lcol (other.lcol)
  {
    std::swap (extents, other.extents);
  }
  ModelInstance& operator= (ModelInstance&& other)
  {
    std::swap (model, other.model);
    std::swap (extents, other.extents);
    std::swap (pos, other.pos);
    std::swap (dir, other.dir);
    std::swap (_wmo_orientation, other._wmo_orientation);
    std::swap (d1, other.d1);
    std::swap (sc, other.sc);
    std::swap (ldir, other.ldir);
    std::swap (lcol, other.lcol);
    return *this;
  }

  void draw ( math::frustum const& frustum
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool force_box
            , bool all_boxes
            , bool draw_fog
            , bool is_current_selection
            );
  void drawMapTile();
  //  void drawHighlight();
  void intersect (math::ray const&, selection_result*);
  void draw_wmo ( const math::vector_3d& ofs
                , const math::degrees
                , math::frustum const&
                , bool draw_fog
                );

  void resetDirection();

  bool isInsideRect(math::vector_3d rect[2]);

  void recalcExtents();
};
