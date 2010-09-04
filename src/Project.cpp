#include "Project.h"
#include "ConfigFile.h"
#include "directory.h"

Project::Project()
{
	// Read out config and set path in project if exists.
	// will later come direct from the project file.
	if( FileExists( "NoggIt.conf" ) )
	{
		ConfigFile config( "NoggIt.conf" );
		if(	config.keyExists("ProjectPath"))
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

void Project::setPath(std::string setPath)
{
	path = setPath;
}

std::string Project::getPath()
{
	return path;
}
