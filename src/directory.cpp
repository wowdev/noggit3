//! \todo  Use boost::filesystem.
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <sstream>
#include <fstream>
#include <string>

#include "directory.h"

bool FileExists( const std::string& filename )
{
	std::fstream foo;
	foo.open( filename.c_str() );
	if( foo.is_open( ) )
	{
		foo.close( );
		return true;
	}
	return false;
}

void CreatePath( const std::string& filename )
{
	size_t found = filename.substr( 0, filename.length( ) - 1 ).find_last_of("/\\");
	if( found != std::string::npos )
		CreatePath( filename.substr( 0, found + 1 ) );
	
#ifdef _WIN32
	mkdir( filename.c_str( ) );
#else
	mkdir( filename.c_str( ), 0777 );
#endif
}


File::File( const std::string& pName )
{
	mName = pName;
}

Directory::Directory( const std::string& pName )
{
	mName = pName;
}

Directory * Directory::AddSubDirectory( const std::string& pName )
{
  std::string name = pName;
  
	Directory * lCurrentDir = this;
	size_t found;
	found = name.find_last_of( "/\\" );
	while( found != std::string::npos )
	{
		lCurrentDir = lCurrentDir->AddSubDirectory( name.substr( 0, found ) );
		name = name.substr( found + 1 );
		found = name.find_last_of( "/\\" );
	}
	if( ! lCurrentDir->mSubdirectories[name] )
		lCurrentDir->mSubdirectories[name] = new Directory( name );
	return lCurrentDir->mSubdirectories[name];
}
Directory * Directory::AddSubDirectory( Directory * pDirectory )
{
	mSubdirectories[pDirectory->mName] = pDirectory;
	return mSubdirectories[pDirectory->mName];
}

File * Directory::AddFile( const std::string& pName )
{
	File * lNewFile = new File( pName );
	mSubfiles.push_back( lNewFile );
	return lNewFile;
}
File * Directory::AddFile( File * pFile )
{
	mSubfiles.push_back( pFile );
	return pFile;
}

Directory * Directory::operator[]( const std::string& pName )
{
	return mSubdirectories[pName];
}
Directory * Directory::operator[]( char * pName )
{
	return mSubdirectories[std::string( pName )];
}
