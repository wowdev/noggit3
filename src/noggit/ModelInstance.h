// ModelInstance.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include <boost/optional.hpp>

#include <math/vector_3d.h>
#include <math/quaternion.h>

#include <noggit/ModelManager.h>
#include <noggit/Selection.h>

class Frustum;
class Model;
class World;

struct ENTRY_MDDF;

namespace noggit
{
  namespace mpq
  {
    class file;
  }
}

class ModelInstance
{
public:
  scoped_model_reference model;

private:
  unsigned int nameID;
public:

  ::math::vector_3d pos;
  ::math::vector_3d dir;

  ::math::quaternion _wmo_doodad_rotation;

  //! \todo  Get this out and do somehow else.
  unsigned int d1;

  float w, sc;

  ::math::vector_3d lcol;
  ::math::vector_3d ldir;

  ModelInstance( World*, std::string const& path );
  ModelInstance( World*, std::string const& path, noggit::mpq::file* f );
  ModelInstance( World*, std::string const& path, ENTRY_MDDF *d );

  //! \todo This constructor should actually be one of a subclass.
  ModelInstance ( World* world
                , std::string const& path
                , const ::math::vector_3d& position
                , const ::math::quaternion& rotation
                , const float& scale
                , const ::math::vector_3d& lighting_color
                );
  ~ModelInstance();

  bool is_visible ( const float& cull_distance
                  , const Frustum& frustum
                  , const ::math::vector_3d& camera
                  , const ::math::vector_3d& offset = ::math::vector_3d()
                  , const float& rotation = 0.0f
                  ) const;

  void draw ( bool draw_fog
            , const boost::optional<selection_type>& selected_item
            ) const;
  void draw_for_selection();

  void draw2() const;
  void draw2Select() const;

//  void drawMapTile();

  void resetDirection();

  std::pair<::math::vector_3d, ::math::vector_3d> extents() const;

private:
  void draw_selection_indicator() const;

  World* _world;

  size_t _spawn_timestamp;
  size_t time_since_spawn() const;
};

#endif // MODELINSTANCE_H
