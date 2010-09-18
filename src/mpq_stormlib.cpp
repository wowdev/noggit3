#include "mpq_stormlib.h"
#include "noggit.h"
#include "Log.h"
#include "directory.h"
#include "Project.h"

#include <vector>
#include <list>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>

typedef std::vector< std::pair< std::string, HANDLE* > > ArchiveSet;
ArchiveSet gOpenArchives;

std::list<std::string> gListfile;

MPQArchive::MPQArchive(const std::string& filename,bool doListfile)
{

	bool result = SFileOpenArchive( filename.c_str(), 0, MPQ_OPEN_NO_LISTFILE, &mpq_a );

	if(!result) {
		//gLog("Error opening archive %s\n", filename);
		return;
	}
	Log << "Opening archive:" << filename << std::endl;
	gOpenArchives.push_back( make_pair( filename, &mpq_a ) );

	if( doListfile )
	{
		HANDLE fh;

		if( !SFileOpenFileEx( mpq_a, "(listfile)", 0, &fh ) )
		{
			LogError << "No listfile in archive \"" << filename << "\"." << std::endl;	
			SFileCloseArchive(&mpq_a);
			return;
		}

		// Found!
		DWORD filesize = SFileGetFileSize( fh );

		// HACK: in patch.mpq some files don't want to open and give 1 for filesize
		// TODO STEFF test if this is alos in StormLib version the case.
		if (filesize<=1) {
			return;
		}

		unsigned char *readbuffer = new unsigned char[filesize];
		SFileReadFile( fh, readbuffer, filesize );
		SFileCloseFile( fh );

		char*file=strtok((char *)readbuffer,"\r\n");
		while (file) 
		{
		  std::string line = file;
		  std::transform( line.begin( ), line.end( ), line.begin( ), ::tolower );
		  gListfile.push_back( line );
	      
		  file = (char*)strtok(NULL, "\r\n");
		}
		free(readbuffer);
	}
}

MPQArchive::~MPQArchive()
{
	/*
	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		mpq_archive &mpq_a = **i;
		
		free(mpq_a.header);
	}
	*/
	//gOpenArchives.erase(gOpenArchives.begin(), gOpenArchives.end());
}

void MPQArchive::close()
{
	SFileCloseArchive(mpq_a);
	for(ArchiveSet::iterator it=gOpenArchives.begin(); it!=gOpenArchives.end();++it)
	{
		HANDLE &mpq_b = *it->second;
		if (&mpq_b == &mpq_a) {
			gOpenArchives.erase(it);
			//delete (*it);
			return;
		}
	}
	
}

void
MPQFile::openFile( const std::string& filename )
{
	eof = false;
	buffer = 0;
	pointer = 0;
	size = 0;

	std::string diskpath = Project::getInstance()->getPath().append(filename) ;
  
	size_t found = diskpath.find( "\\" );
	while( found != std::string::npos )
	{
		diskpath.replace( found, 1, "/" );
		found = diskpath.find( "\\" );
	}
  
  //LogDebug << "trying to open " << diskpath << "." << std::endl;

	FILE* fd = fopen( diskpath.c_str() , "rb" );
	if( !fd )
	{
		fd = fopen( diskpath.c_str( ), "rb" );
	}
	
	fname = diskpath;

	// if file is found on disk load binary data into buffer
	if( fd )
	{
		fseek( fd, 0, SEEK_END );
		size = ftell( fd);
		
		buffer = new uint8_t[size];
		fseek( fd, 0, SEEK_SET );
		fread( buffer, 1, size, fd );
		fclose( fd );
		External = true;
		Log << "Opening file \"" << filename << "\" from disk." << std::endl;
		std::transform( fname.begin( ), fname.end( ), fname.begin( ), ::tolower );
		return;
	}



	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end(); ++i)
	{
		HANDLE &mpq_a = *i->second;

		HANDLE fh;

		if( !SFileOpenFileEx( mpq_a, filename.c_str(), 0, &fh ) )
			continue;

		// Found!
		DWORD filesize = SFileGetFileSize( fh );
		size = filesize;

		// HACK: in patch.mpq some files don't want to open and give 1 for filesize
		if (size<=1) {
			eof = true;
			buffer = 0;
			return;
		}

		buffer = new unsigned char[size];
		SFileReadFile( fh, buffer, size );
		SFileCloseFile( fh );

		return;
	}

	eof = true;
	buffer = 0;
}

void MPQFile::SaveFile( )
{	
	FILE* fd;
	
	std::string lFilename = fname;
	LogDebug << fname << std::endl;
	size_t found = lFilename.find( "\\" );
	while( found != std::string::npos )
	{
		lFilename.replace( found, 1, "/" );
		found = lFilename.find( "\\" );
	}
	LogDebug << lFilename << std::endl;	
	std::string lDirectoryName = lFilename;	

	found = lDirectoryName.find_last_of("/\\");
	if( found != std::string::npos )
		CreatePath( lDirectoryName.substr( 0, found + 1 ) );
	else
		LogError << "Is \"" << lDirectoryName << "\" really a location I can write to? Please report this line." << std::endl;
		LogDebug << lDirectoryName << std::endl;	
	

	fd = fopen( lFilename.c_str( ), "wb" );

	if( fd )
	{
		Log << "Saving file \"" << lFilename << "\"." << std::endl;
		fseek(fd,0,SEEK_SET);
		fwrite(buffer,1,size,fd);
		fclose(fd);
		External=true;
		return;
	}
}

MPQFile::MPQFile( const std::string& filename ):
	eof(false),
	buffer(0),
	pointer(0),
	size(0)
{
	openFile( filename );
}

MPQFile::~MPQFile()
{
	close();
}

bool MPQFile::exists( const std::string& filename )
{
	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		HANDLE &mpq_a = *i->second;

		if( SFileHasFile( mpq_a, filename.c_str() ) )
			return true;
	}

	return false;
}

void MPQFile::save(const char* filename)
{
/*
	wxFile f;
	f.Open(wxString(filename, wxConvUTF8), wxFile::write);
	f.Write(buffer, size);
	f.Close();
*/
}

size_t MPQFile::read(void* dest, size_t bytes)
{
	if (eof) 
		return 0;

	size_t rpos = pointer + bytes;
	if (rpos > size) {
		bytes = size - pointer;
		eof = true;
	}

	memcpy(dest, &(buffer[pointer]), bytes);

	pointer = rpos;

	return bytes;
}

bool MPQFile::isEof()
{
    return eof;
}

void MPQFile::seek(int offset)
{
	pointer = offset;
	eof = (pointer >= size);
}

void MPQFile::seekRelative(int offset)
{
	pointer += offset;
	eof = (pointer >= size);
}

void MPQFile::close()
{
	if (buffer) delete[] buffer;
	buffer = 0;
	eof = true;
}

size_t MPQFile::getSize()
{
	return size;
}

void FixFilePath( std::string & pFilename )
{
	//std::transform( pFilename.begin( ), pFilename.end( ), pFilename.begin( ), ::tolower );
	
	size_t found = pFilename.find( "/" );
	while( found != std::string::npos )
	{
		pFilename.replace( found, 1, "\\" );
		found = pFilename.find( "/" );
	}
}

int MPQFile::getSize( const std::string& filename )
{
	for(ArchiveSet::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
	{
		HANDLE &mpq_a = *i->second;
		HANDLE fh;
		
		if( !SFileOpenFileEx( mpq_a, filename.c_str(), 0, &fh ) )
			continue;

		DWORD filesize = SFileGetFileSize( fh );
		SFileCloseFile( fh );
		return filesize;
	}

	return 0;
}

size_t MPQFile::getPos()
{
	return pointer;
}

unsigned char* MPQFile::getBuffer()
{
	return buffer;
}

unsigned char* MPQFile::getPointer()
{
	return buffer + pointer;
}
