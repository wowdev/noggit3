#include <noggit/TextureManager.h>

#include <algorithm>

#include <noggit/Video.h> // OpenGL::Texture
#include <noggit/Log.h> // LogDebug

TextureManager::mapType TextureManager::items;

void TextureManager::report()
{
  std::string output = "Still in the texture manager:\n";
  for( mapType::iterator t = items.begin(); t != items.end(); ++t )
  {
    output += "- " + t->first + "\n";
  }
  LogDebug << output;
}

void TextureManager::delbyname( std::string name )
{
  std::transform( name.begin(), name.end(), name.begin(), ::tolower );

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

OpenGL::Texture* TextureManager::newTexture( std::string name )
{
  std::transform( name.begin(), name.end(), name.begin(), ::tolower );

  if( items.find( name ) == items.end() )
  {
    items[name] = new OpenGL::Texture( );
    items[name]->loadFromBLP( name );
  }

  items[name]->addReference();

  return items[name];
}

std::vector<OpenGL::Texture*> TextureManager::getAllTexturesMatching(bool (*function)( const std::string& name ) )
{
  std::vector<OpenGL::Texture*> returnVector;
  for( mapType::iterator t = items.begin(); t != items.end(); ++t )
  {
    if( function( t->first ) )
    {
      returnVector.push_back( items[t->first] );
    }
  }
  return returnVector;
}
