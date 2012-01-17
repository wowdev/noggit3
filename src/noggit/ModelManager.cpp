// ModelManager.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/ModelManager.h> // ModelManager

#include <algorithm>

#include <noggit/async/loader.h>

#include <noggit/application.h>
#include <noggit/Model.h> // Model
#include <noggit/Log.h> // LogDebug

ModelManager::mapType ModelManager::items;

void ModelManager::report()
{
  std::string output = "Still in the model manager:\n";
  for( mapType::iterator t = items.begin(); t != items.end(); ++t )
  {
    output += "- " + t->first + "\n";
  }
  LogDebug << output;
}

Model* ModelManager::add( std::string name )
{
  std::transform( name.begin(), name.end(), name.begin(), ::tolower );

  size_t found = name.rfind( ".mdx" );
  if( found != std::string::npos )
    name.replace( found, 4, ".m2" );

  found = name.rfind( ".mdl" );
  if( found != std::string::npos )
    name.replace( found, 4, ".m2" );

  if( items.find( name ) == items.end() )
  {
    items[name] = new Model( name );
    //! \todo delete this, as it should actually be async, d'uh.
    //items[name]->finish_loading();
    noggit::app().async_loader().add_object( items[name] );
  }

  items[name]->addReference();
  return items[name];
}

void ModelManager::delbyname( std::string name )
{
  std::transform( name.begin(), name.end(), name.begin(), ::tolower );

  size_t found = name.rfind( ".mdx" );
  if( found != std::string::npos )
    name.replace( found, 4, ".m2" );

  found = name.rfind( ".mdl" );
  if( found != std::string::npos )
    name.replace( found, 4, ".m2" );

  if( items.find( name ) != items.end() )
  {
    items[name]->removeReference();

    if( items[name]->hasNoReferences() )
    {
      delete items[name];
      items.erase( items.find( name ) );
    }
  }
}

void ModelManager::resetAnim()
{
  for( ModelManager::mapType::iterator it = items.begin( ); it != items.end( ); ++it )
    it->second->animcalc = false;
}

void ModelManager::updateEmitters( float dt )
{
  for( ModelManager::mapType::iterator it = items.begin( ); it != items.end( ); ++it )
    it->second->updateEmitters( dt );
}
