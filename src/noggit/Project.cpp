// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ConfigFile.h>
#include <noggit/Project.h>
#include <noggit/Settings.h>

#include <boost/filesystem.hpp>

#include <string>

Project::Project()
{
  // Read out config and set path in project if exists.
  // will later come direct from the project file.
  path = Settings::getInstance()->projectPath;
  // else set the project path to the wow std path
}

Project* Project::instance = 0;


Project* Project::getInstance()
{
  if (!instance)
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
