#include "ModelManager.h" // ModelManager

#include "AsyncLoader.h"// AsyncLoader
#include "noggit.h" // gAsyncLoader

int ModelManager::baseid = 0;

#ifdef WIN32
template <> std::map<std::string, MODELIDTYPE> Manager<MODELIDTYPE,Model>::names;
template <> std::map<MODELIDTYPE, Model*> Manager<MODELIDTYPE,Model>::items;
#else
template <class IDTYPE,class MANAGEDITEM> std::map<std::string, MODELIDTYPE> Manager<MODELIDTYPE,Model>::names;
template <class IDTYPE,class MANAGEDITEM> std::map<MODELIDTYPE, Model*> Manager<MODELIDTYPE,Model>::items;
#endif

MODELIDTYPE ModelManager::add( const std::string& name )
{
	int id;
	std::string name_ = name;
	std::transform( name_.begin(), name_.end(), name_.begin(), ::tolower );
	if( names.find( name_ ) != names.end() ) 
	{
		id = names[name_];
		items[id]->addref();
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
		reinterpret_cast<Model*>( items[it->second] )->animcalc = false;
}

void ModelManager::updateEmitters( float dt )
{
	for( std::map<std::string, MODELIDTYPE>::iterator it = names.begin( ); it != names.end( ); ++it )
		reinterpret_cast<Model*>( items[it->second] )->updateEmitters( dt );
}
