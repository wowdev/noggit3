// Selection.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#ifndef __SELECTION_H__
#define __SELECTION_H__

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

class nameEntryManager
{
public:
  nameEntryManager (World*);

  size_t add (ModelInstance* mod);
  size_t add (WMOInstance* wmo);
  size_t add (MapChunk* chunk);

  void del (size_t ref);

  nameEntry* findEntry (size_t ref) const;

private:
  std::vector<nameEntry*> _items;

  World* _world;
};

#endif
