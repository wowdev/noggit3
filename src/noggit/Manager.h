// Manager.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef MANAGER_H
#define MANAGER_H

#include <cstdlib> // size_t

// base class for manager objects

//! \todo Proxy objects / handles holding the name and the object.

class ManagedItem
{
private:
  size_t _referenceCount;

public:
  explicit ManagedItem( )
  : _referenceCount( 0 )
  {
  }

  virtual ~ManagedItem()
  {
  }

  inline void addReference()
  {
    ++_referenceCount;
  }

  inline void removeReference()
  {
    --_referenceCount;
  }

  inline bool hasNoReferences()
  {
    return _referenceCount == 0;
  }
};

#endif
