// Selection.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/Selection.h>

#include <cassert>
#include <stdexcept>

#include <noggit/Log.h>
#include <noggit/MapChunk.h> // MapChunk
#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>

/**
 ** nameEntry
 **
 ** This is used for selectable objects.
 **
 **/

nameEntry::nameEntry( ModelInstance *model )
{
  type = eEntry_Model;
  data.model = model;
}

nameEntry::nameEntry( WMOInstance *wmo )
{
  type = eEntry_WMO;
  data.wmo = wmo;
}

nameEntry::nameEntry( MapChunk *chunk )
{
  type = eEntry_MapChunk;
  data.mapchunk = chunk;
}

nameEntry::nameEntry (const nameEntry& other)
{
  type = other.type;
  data.___DIRTY = data.___DIRTY;
}

/**
 ** nameEntryManager
 **
 ** This is used for managing those selectable objects.
 **
 **/

size_t nameEntryManager::add( ModelInstance *mod )
{
  _items.push_back (new nameEntry (mod));
  return _items.size() - 1;
}
size_t nameEntryManager::add( WMOInstance *wmo )
{
  _items.push_back (new nameEntry (wmo));
  return _items.size() - 1;
}
size_t nameEntryManager::add( MapChunk *chunk )
{
  _items.push_back (new nameEntry (chunk));
  return _items.size() - 1;
}

nameEntry* nameEntryManager::findEntry (size_t ref) const
{
  assert (ref < _items.size());
  return _items[ref];
}

void nameEntryManager::del (size_t ref)
{
  //! \todo This is no longer in that global _world thing, so can't be
  //! removed  there.  We  somehow  need a  back  reference between  a
  //! selection and  where the selection is  stored so we  are able to
  //! unselect it. (One item can  be selected in multiple views, which
  //! is why we can't rely on the view resetting his selection when it
  //! is  deleted. Also, a  view might  have selected  something which
  //! goes  out of  range and  therefore is  unloaded, ergo  no longer
  //! selected.

  // if (_world->GetCurrentSelection() == _items[ref])
  // {
  //   _world->mCurrentSelection = boost::none;
  // }

  delete _items[ref];
  _items[ref] = 0;
}

namespace noggit
{
  namespace selection
  {
    namespace visitors
    {
      class position : public boost::static_visitor< ::math::vector_3d>
      {
      public:
        ::math::vector_3d
        operator() (const selected_chunk_type& chunk) const
        {
          return chunk.first->GetSelectionPosition (chunk.second);
        }
        ::math::vector_3d
        operator() (const selected_model_type& model) const
        {
          return model->pos;
        }
        ::math::vector_3d
        operator() (const selected_wmo_type& wmo) const
        {
          return wmo->pos;
        }
      };

      class name_entry : public boost::static_visitor<nameEntry*>
      {
      public:
        nameEntry* operator() (const selected_chunk_type& chunk) const
        {
          return new nameEntry (chunk.first);
        }
        nameEntry* operator() (const selected_model_type& model) const
        {
          return new nameEntry (model);
        }
        nameEntry* operator() (const selected_wmo_type& wmo) const
        {
          return new nameEntry (wmo);
        }
      };

      class is_chunk : public boost::static_visitor<bool>
      {
      public:
        bool operator() (const selected_chunk_type&) const
        {
          return true;
        }
        template<typename T>
        bool operator() (const T&) const
        {
          return false;
        }
      };

      class is_model : public boost::static_visitor<bool>
      {
      public:
        bool operator() (const selected_model_type&) const
        {
          return true;
        }
        template<typename T>
        bool operator() (const T&) const
        {
          return false;
        }
      };

      class is_wmo : public boost::static_visitor<bool>
      {
      public:
        bool operator() (const selected_wmo_type&) const
        {
          return true;
        }
        template<typename T>
        bool operator() (const T&) const
        {
          return false;
        }
      };

      class remove_from_world : public boost::static_visitor<>
      {
      public:
        remove_from_world (World* world_link)
          : _world_link (world_link)
        {}

        void operator() (const selected_chunk_type&) const
        {
          throw std::runtime_error ("Can't delete chunks as of now.");
        }
        void operator() (const selected_model_type& model) const
        {
          _world_link->deleteModelInstance (model->d1);
        }
        void operator() (const selected_wmo_type& wmo) const
        {
          _world_link->deleteWMOInstance (wmo->mUniqueID);
        }

      private:
        World* _world_link;
      };

      class reset_rotation : public boost::static_visitor<>
      {
      public:
        void operator() (const selected_chunk_type&) const
        {
          throw std::runtime_error ("Can't reset rotation chunks.");
        }
        void operator() (selected_model_type const& model) const
        {
          model->resetDirection();
        }
        void operator() (selected_wmo_type const& wmo) const
        {
          wmo->resetDirection();
          wmo->recalc_extents();
        }
      };

      class selected_polygon : public boost::static_visitor<int>
      {
      public:
        int operator() (const selected_chunk_type& chunk) const
        {
          return chunk.second;
        }
        template<typename T>
        int operator() (const T&) const
        {
          throw std::runtime_error ("Only chunks have a selected polygon.");
        }
      };

      class area_id : public boost::static_visitor<int>
      {
      public:
        int operator() (const selected_chunk_type& chunk) const
        {
          return chunk.first->header.areaid;
        }
        template<typename T>
        int operator() (const T&) const
        {
          throw std::runtime_error ("Only chunks have an area id.");
        }
      };

      //! \todo WMOs do not adjust their extents!
#define ROTATION_VISITOR(AXIS) \
      class rotate_ ## AXIS : public boost::static_visitor<>            \
      {                                                                 \
      public:                                                           \
        rotate_ ## AXIS (const float& degrees)                          \
          : _degrees (degrees)                                          \
        { }                                                             \
                                                                        \
        void operator() (const selected_chunk_type&) const              \
        {                                                               \
          throw std::runtime_error ("Can't rotate a chunk as of now."); \
        }                                                               \
        void operator() (selected_model_type const& model) const        \
        {                                                               \
          model->dir.AXIS() += _degrees;                                \
          while (model->dir.AXIS() > 360.0f)                            \
          {                                                             \
            model->dir.AXIS() -= 360.0f;                                \
          }                                                             \
          while (model->dir.AXIS() < 0.0f)                              \
          {                                                             \
            model->dir.AXIS() += 360.0f;                                \
          }                                                             \
        }                                                               \
        void operator() (selected_wmo_type const& wmo) const            \
        {                                                               \
          wmo->dir.AXIS() += _degrees;                                  \
          while (wmo->dir.AXIS() > 360.0f)                              \
          {                                                             \
            wmo->dir.AXIS() -= 360.0f;                                  \
          }                                                             \
          while (wmo->dir.AXIS() < 0.0f)                                \
          {                                                             \
            wmo->dir.AXIS() += 360.0f;                                  \
          }                                                             \
          wmo->recalc_extents();                                        \
        }                                                               \
                                                                        \
      private:                                                          \
        float _degrees;                                                 \
      }

      ROTATION_VISITOR(x);
      ROTATION_VISITOR(y);
      ROTATION_VISITOR(z);

#undef ROTATION_VISITOR

      class scale : public boost::static_visitor<>
      {
      public:
        scale (const float& factor)
          : _factor (factor)
        { }

        template<typename T>
        void operator() (const T&) const
        {
          throw std::runtime_error ("Can only scale doodads.");
        }
        void operator() (const selected_model_type& model) const
        {
          static const float minimum_scale (0.00098f);
          static const float maximum_scale (63.9f);
          model->sc = qBound ( minimum_scale
                             , model->sc * _factor
                             , maximum_scale
                             );
        }

      private:
        float _factor;
      };

      class move : public boost::static_visitor<>
      {
      public:
        move (const ::math::vector_3d& offset)
          : _offset (offset)
        { }

        void operator() (const selected_chunk_type&) const
        {
          throw std::runtime_error ("Can't move chunks as of now.");
        }
        void operator() (const selected_model_type& model) const
        {
          model->pos += _offset;
        }
        void operator() (const selected_wmo_type& wmo) const
        {
          wmo->pos += _offset;
          wmo->recalc_extents();
        }

      private:
        ::math::vector_3d _offset;
      };

      class set_doodad_set : public boost::static_visitor<void>
      {
      public:
        set_doodad_set (const int& id)
          : _id (id)
        { }

        void operator() (const selected_wmo_type& wmo) const
        {
          wmo->doodadset = _id;
        }
        template<typename T>
        void operator() (const T&) const
        {
          throw std::runtime_error ("Can only set doodad set ids of WMOs");
        }

      private:
        int _id;
      };

      class is_the_same_as : public boost::static_visitor<bool>
      {
      public:
        is_the_same_as (const void* other)
          : _other (other)
        { }

        bool operator() (const selected_chunk_type& chunk) const
        {
          return chunk.first == _other;
        }
        bool operator() (const selected_model_type& model) const
        {
          return model == _other;
        }
        bool operator() (const selected_wmo_type& wmo) const
        {
          return wmo == _other;
        }

      private:
        const void* _other;
      };
    }

    ::math::vector_3d position (const selection_type& selection)
    {
      return boost::apply_visitor (visitors::position(), selection);
    }

    //! \note This is bullshit, as a name entry does not exist outside World.
    nameEntry* name_entry (const selection_type& selection)
    {
      return boost::apply_visitor (visitors::name_entry(), selection);
    }

    bool is_chunk (const selection_type& selection)
    {
      return boost::apply_visitor (visitors::is_chunk(), selection);
    }
    bool is_model (const selection_type& selection)
    {
      return boost::apply_visitor (visitors::is_model(), selection);
    }
    bool is_wmo (const selection_type& selection)
    {
      return boost::apply_visitor (visitors::is_wmo(), selection);
    }

    void remove_from_world ( World* world_link
                           , const selection_type& selection
                           )
    {
      boost::apply_visitor ( visitors::remove_from_world (world_link)
                           , selection
                           );
    }

    void reset_rotation (const selection_type& selection)
    {
      boost::apply_visitor (visitors::reset_rotation(), selection);
    }

    int selected_polygon (const selection_type& selection)
    {
      return boost::apply_visitor (visitors::selected_polygon(), selection);
    }

    int area_id (const selection_type& selection)
    {
      return boost::apply_visitor (visitors::area_id(), selection);
    }

#define ROTATE_FUNCTION(AXIS)                                             \
    void rotate_ ## AXIS ( const float& degrees                           \
                         , const selection_type& selection                \
                         )                                                \
    {                                                                     \
      boost::apply_visitor                                                \
        (visitors::rotate_ ## AXIS (degrees), selection);                 \
    }

    ROTATE_FUNCTION(x)
    ROTATE_FUNCTION(y)
    ROTATE_FUNCTION(z)

#undef ROTATE_FUNCTION

    void scale (const float& factor, const selection_type& selection)
    {
      boost::apply_visitor (visitors::scale (factor), selection);
    }

    void move ( const ::math::vector_3d& offset
              , const selection_type& selection
              )
    {
      boost::apply_visitor (visitors::move (offset), selection);
    }

    void set_doodad_set (const int& id, const selection_type& selection)
    {
      boost::apply_visitor (visitors::set_doodad_set (id), selection);
    }

    bool is_the_same_as (const void* other, const selection_type& selection)
    {
      return boost::apply_visitor ( visitors::is_the_same_as (other)
                                  , selection
                                  );

    }
  }
}
