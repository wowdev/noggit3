// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

enum eSelectionEntryTypes
{
  eEntry_Model,
  eEntry_WMO,
  eEntry_MapChunk
};

#include <string>
#include <vector>

// Instead of includes.
class ModelInstance;
class WMOInstance;
class MapChunk;
class World;

#include <math/vector_3d.h>
#include <boost/variant.hpp>


struct selected_chunk_type
{
  selected_chunk_type(MapChunk* _chunk, int _triangle, math::vector_3d _position)
    : chunk(_chunk)
    , triangle(_triangle)
    , position(_position)
  {}

  MapChunk* chunk;
  int triangle;
  math::vector_3d position;
};
typedef ModelInstance* selected_model_type;
typedef WMOInstance* selected_wmo_type;

typedef boost::variant< selected_chunk_type
                      , selected_model_type
                      , selected_wmo_type
                      > selection_type;

typedef std::pair<float, selection_type> selection_entry;
typedef std::vector<selection_entry> selection_result;

namespace noggit
{
  namespace selection
  {
    ::math::vector_3d position (const selection_type& selection);

    bool is_chunk (const selection_type& selection);
    bool is_model (const selection_type& selection);
    bool is_wmo (const selection_type& selection);
    void remove_from_world ( World* world_link
                           , const selection_type& selection
                           );
    void add_to_world ( World* world_link
                      , const math::vector_3d& position
                      , bool size_randomization
                      , bool position_randomization
                      , bool rotation_randomization
                      , const selection_type& selection
                      );
    void reset_rotation (const selection_type& selection);
    int selected_polygon (const selection_type& selection);
    int area_id (const selection_type& selection);

#define ROTATE_FUNCTION(AXIS)                                             \
    void rotate_ ## AXIS ( const float& degrees                           \
                         , const selection_type& selection                \
                         )

    ROTATE_FUNCTION(x);
    ROTATE_FUNCTION(y);
    ROTATE_FUNCTION(z);

#undef ROTATE_FUNCTION

    void scale (const float& factor, const selection_type& selection);
    void move ( const ::math::vector_3d& offset
              , const selection_type& selection
              );
    void set_doodad_set (const int& id, const selection_type& selection);
    bool is_the_same_as (const void* other, const selection_type& selection);
  }
}
