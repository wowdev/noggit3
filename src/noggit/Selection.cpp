// Selection.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/Selection.h>

#include <cassert>

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
  LogDebug << "added model with " << _items.size() << std::endl;
  _items.push_back (new nameEntry (mod));
  return _items.size() - 1;
}
size_t nameEntryManager::add( WMOInstance *wmo )
{
  LogDebug << "added wmo with " << _items.size() << std::endl;
  _items.push_back (new nameEntry (wmo));
  return _items.size() - 1;
}
size_t nameEntryManager::add( MapChunk *chunk )
{
  LogDebug << "added chunk with " << _items.size() << std::endl;
  _items.push_back (new nameEntry (chunk));
  return _items.size() - 1;
}

nameEntry* nameEntryManager::findEntry (size_t ref) const
{
  LogDebug << "requested " << ref << " when having " << _items.size() << "items. result is " << _items[ref] << std::endl;
  assert (ref < _items.size());
  return _items[ref];
}

nameEntryManager::nameEntryManager (World* world)
  : _world (world)
  , _items (0)
{
}

void nameEntryManager::del (size_t ref)
{
  if (_items[ref])
  {
    if (_world->GetCurrentSelection() == _items[ref])
    {
      _world->ResetSelection();
    }

    delete _items[ref];
    _items[ref] = 0;
  }
}
