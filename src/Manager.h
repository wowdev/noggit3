#ifndef MANAGER_H
#define MANAGER_H

#include <string> // std::string
#include <map> // std::map
#include <algorithm> // std::transform()

// base class for manager objects

class ManagedItem 
{
private:
  int _referenceCount;
  std::string _name;
  
public:
  explicit ManagedItem( const std::string& n )
  : _referenceCount( 0 )
  , _name( n )
  {
    std::transform( _name.begin(), _name.end(), _name.begin(), ::tolower );
  }
  
  virtual ~ManagedItem()
  {
  }

  void addref()
  {
    ++_referenceCount;
  }

  bool delref()
  {
    return --_referenceCount == 0;
  }
  
  const std::string& name() const
  {
    return _name;
  }
  void name(const std::string& value)
  {
    _name = value;
  }
};

template <class IDTYPE,class MANAGEDITEM>
class Manager
{
  typedef std::map<IDTYPE, MANAGEDITEM*> itemsMapType;
public:
  static std::map<std::string, IDTYPE> names;

  static IDTYPE add( const std::string& name );
  
  static void doDelete( IDTYPE id )
  {
    if( items[id] )
    {
      delete items[id];
      items[id] = NULL;
    }
  }
  
  static void del( IDTYPE id )
  {
    if( items[id]->delref() )
    {
      names.erase( names.find( items[id]->name() ) );
      
      doDelete( id );
      
      items.erase( items.find( id ) );
    }
  }
  static void delbyname( const std::string& name )
  {
    std::string name_ = name;
    std::transform( name_.begin(), name_.end(), name_.begin(), ::tolower );
    
    if( has( name_ ) )
    {
      del( get( name_ ) );
    }
  }
  static bool has( const std::string& name )
  {
    std::string name_ = name;
    std::transform( name_.begin(), name_.end(), name_.begin(), ::tolower );
    
    return( names.find( name_ ) != names.end() );
  }
  static IDTYPE get( const std::string& name )
  {
    std::string name_ = name;
    std::transform( name_.begin(), name_.end(), name_.begin(), ::tolower );
    
    return names[name_];
  }
  
  static MANAGEDITEM* item( std::string name )
  {
    std::transform( name.begin(), name.end(), name.begin(), ::tolower );
    return items[names[name]];
  }
  static MANAGEDITEM* item( IDTYPE id )
  {
    return items[id];
  }
  
  static typename itemsMapType::iterator begin()
  {
    return items.begin();
  }
  static typename itemsMapType::iterator end()
  {
    return items.end();
  }
  static typename itemsMapType::iterator find( IDTYPE searchFor )
  {
    return items.find( searchFor );
  }

protected:
  static itemsMapType items;
  
  static void do_add( const std::string& name, IDTYPE id, MANAGEDITEM* item )
  {
    std::string name_ = name;
    std::transform( name_.begin(), name_.end(), name_.begin(), ::tolower );
    
    names[name_] = id;
    item->addref();
    item->name(name_);
    items[id] = item;
  }
};

#endif
