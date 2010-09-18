#ifndef __DIRECTORY_H
#define __DIRECTORY_H

#include <string>
#include <map>
#include <vector>

void CreatePath( const std::string& filename );
bool FileExists( const std::string& filename );

struct File
{
	std::string mName;

	File( const std::string& pName );
};

struct Directory
{
	std::map<std::string,Directory*> mSubdirectories;
	std::vector<File*> mSubfiles;

	std::string mName;
	
	Directory( const std::string& pName );
	void PrintDirectory( const int pIndent );

	Directory * AddSubDirectory( const std::string& pName );
	Directory * AddSubDirectory( Directory * pDirectory );

	File * AddFile( const std::string& pName );
	File * AddFile( File * pFile );
	
	Directory * operator[]( const std::string& pName );
	Directory * operator[]( char * pName );
};
#endif
