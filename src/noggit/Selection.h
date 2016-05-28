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

/**
 ** nameEntry
 **
 ** This is used for selectable objects.
 **
 **/

class nameEntry
{
public:
  eSelectionEntryTypes type;
  union
  {
    ModelInstance* model;
    WMOInstance* wmo;
    MapChunk* mapchunk;
    void* ___DIRTY;
  } data;

  explicit nameEntry (ModelInstance* model);
  explicit nameEntry (WMOInstance* wmo);
  explicit nameEntry (MapChunk* chunk);
  explicit nameEntry (const nameEntry& other);
};

/**
 ** nameEntryManager
 **
 ** This is used for managing those selectable objects.
 **
 **/

//! \todo This just is a vector, so inherit it or be one.
class nameEntryManager
{
public:
  size_t add (ModelInstance* mod);
  size_t add (WMOInstance* wmo);
  size_t add (MapChunk* chunk);

  void del (size_t ref);

  nameEntry* findEntry (size_t ref) const;

private:
  std::vector<nameEntry*> _items;
};

namespace math
{
  class vector_3d;
}

#include <boost/variant.hpp>

typedef std::pair<MapChunk*, int> selected_chunk_type;
typedef ModelInstance* selected_model_type;
typedef WMOInstance* selected_wmo_type;

typedef boost::variant< selected_chunk_type
                      , selected_model_type
                      , selected_wmo_type
                      > selection_type;

namespace noggit
{
  namespace selection
  {
    ::math::vector_3d position (const selection_type& selection);

    //! \note This is bullshit, as a name entry does not exist outside World.
    nameEntry* name_entry (const selection_type& selection);

    bool is_chunk (const selection_type& selection);
    bool is_model (const selection_type& selection);
    bool is_wmo (const selection_type& selection);
    void remove_from_world ( World* world_link
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
