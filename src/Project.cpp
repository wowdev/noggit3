#include "Project.h"

#include <string>
#include <boost/filesystem.hpp>

#include "ConfigFile.h"

Project::Project()
{
  // Read out config and set path in project if exists.
  // will later come direct from the project file.
  if( boost::filesystem::exists( "NoggIt.conf" ) )
  {
    ConfigFile config( "NoggIt.conf" );
    if(  config.keyExists("ProjectPath"))
    config.readInto( path, "ProjectPath" );
  }
  // else set the project path to the wow std path


}

Project* Project::instance = 0;


Project* Project::getInstance()
{
  if( !instance)
    instance = new Project();
  return instance;
}

void Project::setPath(const std::string& _setPath)
{
  path = _setPath;
}

std::string Project::getPath()
{
  return path;
}
