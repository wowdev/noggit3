#include "ModelManager.h" // ModelManager

#include <algorithm>
#include <map>
#include <string>

#include "AsyncLoader.h"// AsyncLoader
#include "Noggit.h" // gAsyncLoader

int ModelManager::baseid = 0;

#ifdef WIN32
template <> std::map<std::string, MODELIDTYPE> Manager<MODELIDTYPE,Model>::names;
template <> std::map<MODELIDTYPE, Model*> Manager<MODELIDTYPE,Model>::items;
#else
template <class IDTYPE,class MANAGEDITEM> std::map<std::string, MODELIDTYPE> Manager<MODELIDTYPE,Model>::names;
template <class IDTYPE,class MANAGEDITEM> std::map<MODELIDTYPE, Model*> Manager<MODELIDTYPE,Model>::items;
#endif

MODELIDTYPE ModelManager::add( std::string name )
{
  int id;
  std::transform( name.begin(), name.end(), name.begin(), ::tolower );
  if( names.find( name ) != names.end() )
  {
    id = names[name];
    items[id]->addReference();
    return id;
  }
  
  id = nextID();
  Model *model = new Model( name );
  model->finishLoading();
  
  gAsyncLoader->addObject( model );
  
  do_add( name, id, model );
  return id;
}
void ModelManager::resetAnim()
{
  for( std::map<std::string, MODELIDTYPE>::iterator it = names.begin( ); it != names.end( ); ++it )
    static_cast<Model*>( items[it->second] )->animcalc = false;
}

void ModelManager::updateEmitters( float dt )
{
  for( std::map<std::string, MODELIDTYPE>::iterator it = names.begin( ); it != names.end( ); ++it )
    static_cast<Model*>( items[it->second] )->updateEmitters( dt );
}
