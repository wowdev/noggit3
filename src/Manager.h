#ifndef MANAGER_H
#define MANAGER_H

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
