#include <vector>
#include <list>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <algorithm>

#include "mpq_libmpq.h"
#include "Log.h"
#include "directory.h"
#include "Project.h"

typedef std::vector<mpq_archive_s*> ArchiveSet;
ArchiveSet gOpenArchives;

std::list<std::string> gListfile;

MPQArchive::MPQArchive( const std::string filename, bool doListfile )
{
	int result = libmpq__archive_open( &mpq_a, filename.c_str( ), 0 );	
	if( result ) 
	{
		LogError << "Error opening archive \"" << filename << "\": " << result << std::endl;
		return;
	}
	else 
	{
		Log << "Opening archive \"" << filename << "\"." << std::endl;
	}

	gOpenArchives.push_back( mpq_a );

	if( doListfile )
	{
			// get filenumber and size for listfile
		uint32_t listfile_number; off_t listfile_size;
		if (libmpq__file_number(mpq_a, "(listfile)", &listfile_number) != 0) 
		{
			LogError << "No listfile in archive \"" << filename << "\"." << std::endl;
			libmpq__archive_close(mpq_a);
			return;
		}
		libmpq__file_unpacked_size(mpq_a, listfile_number, &listfile_size);
		
			// read listfile content into memory
		uint8_t *listfile = (uint8_t*)malloc(listfile_size);
		libmpq__file_read(mpq_a, listfile_number, listfile, listfile_size, NULL);
		
			// go through all entries in the listfile
		char *filename = strtok((char*)listfile, "\r\n");
		while (filename) 
		{
			std::string line = filename;
			std::transform( line.begin( ), line.end( ), line.begin( ), ::tolower );
			gListfile.push_back( line );
			
			filename = strtok(NULL, "\r\n");
		}
	}
}

void MPQArchive::close( )
{
	libmpq__archive_close( mpq_a );
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

MPQFile::MPQFile( std::string filename ) : eof( false ), buffer( 0 ), pointer( 0 ), size( 0 )
{
	FixFilePath( filename );

	// First try to read from disk
	std::string diskpath = Project::getInstance()->getPath().append(filename) ;

	FILE* fd = fopen( diskpath.c_str() , "rb" );
	if( !fd )
	{
		fd = fopen( diskpath.c_str( ), "rb" );
	}

	// if file is found on disk load binary data into buffer
	if( fd )
	{
		fname = diskpath;

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

	fname = filename;


	// if not on disk search in MPQ archives.
	for( ArchiveSet::reverse_iterator i = gOpenArchives.rbegin(); i != gOpenArchives.rend(); ++i )
	{
		mpq_archive_s * mpq_a = *i;
		uint32_t fileno;
		int32_t error = 0;
		
		
		if( libmpq__file_number( mpq_a, filename.c_str( ), &fileno ) == LIBMPQ_ERROR_EXIST )
			continue;
		else if ( libmpq__file_number( mpq_a, filename.c_str( ), &fileno ) == LIBMPQ_ERROR_EXIST )
			continue;
			

		if( ( error = libmpq__file_unpacked_size( mpq_a, fileno, &size ) ) < 0 )
			LogError << "When opening \"" << filename << "\", libMPQ gave error #" << error << "!" << std::endl;

		/// HACK: in patch.mpq some files don't want to open and give 1 for filesize
		if( size <= 1 ) 
		{
			eof = true;
			buffer = 0;
			return;
		}

		buffer = new uint8_t[size];

		/// TODO: This sometimes is pretty slow. May be a HDD issue? Is this slow for others too?
		
		if( ( error = libmpq__file_read( mpq_a, fileno, buffer, size, 0 ) ) < 0 ){
			LogError << "When opening \"" << filename << "\", libMPQ gave error #" << error << "!" << std::endl;
			eof = true;
			buffer = 0;	
		}

		External = false;
	std::transform( fname.begin( ), fname.end( ), fname.begin( ), ::tolower );
		return;
	}

	LogError << "Unable to find file \"" << filename << "\"! Check MPQs and the file that has been requesting this!" << std::endl;
	eof = true;
	buffer = 0;
}


MPQFile::MPQFile( std::string filename, bool Fake ) : eof( false ), buffer( 0 ), pointer( 0 ), size( 0 )
{
	FixFilePath( filename );
	
	std::string filepath;
	filepath = Project::getInstance( )->getPath( );
	filepath.append( filename );

	fname = filepath;

	for( ArchiveSet::reverse_iterator i = gOpenArchives.rbegin(); i != gOpenArchives.rend(); ++i )
	{
		mpq_archive_s * mpq_a = *i;
		uint32_t fileno;
		libmpq__file_number( mpq_a, filename.c_str( ), &fileno );
		if( fileno == LIBMPQ_ERROR_EXIST )
			continue;
		
		libmpq__file_unpacked_size( mpq_a, fileno, &size );
	
		/// HACK: in patch.mpq some files don't want to open and give 1 for filesize
		if( size <= 1 )
		{
			eof = true;
			buffer = 0;
			return;
		}
	
		buffer = new uint8_t[size];
		libmpq__file_read( mpq_a, fileno, (uint8_t*)buffer, size, 0 );
		External = false;
		return;
	}
		
	LogError << "Unable to find file \"" << filename << "\"! Check MPQs and the file that has been requesting this!" << std::endl;
	eof = true;
	buffer = 0;
}

MPQFile::~MPQFile( )
{
	close( );
}


size_t MPQFile::read( void* dest, size_t bytes )
{
	if( eof )
		return 0;

	size_t rpos = pointer + bytes;
	if( rpos > size )
	{
		bytes = size - pointer;
		eof = true;
	}

	memcpy( dest, &(buffer[pointer]), bytes );

	pointer = rpos;

	return bytes;
}

bool MPQFile::isEof( )
{
    return eof;
}

void MPQFile::seek( int offset )
{
	pointer = offset;
	eof = ( pointer >= size );
}

void MPQFile::seekRelative( int offset )
{
	pointer += offset;
	eof = ( pointer >= size );
}

void MPQFile::close( )
{
	if( buffer )
		delete[] buffer;
	buffer = 0;
	eof = true;
}

size_t MPQFile::getSize( )
{
	return size;
}

size_t MPQFile::getPos( )
{
	return pointer;
}

uint8_t* MPQFile::getBuffer( )
{
	return buffer;
}

uint8_t* MPQFile::getPointer( )
{
	return buffer + pointer;
}

void MPQFile::SaveFile( )
{	
	FILE* fd;
	
	std::string lFilename = fname;
	
	size_t found = lFilename.find( "\\" );
	while( found != std::string::npos )
	{
		lFilename.replace( found, 1, "/" );
		found = lFilename.find( "\\" );
	}
	
	std::string lDirectoryName = lFilename;	

	found = lDirectoryName.find_last_of("/\\");
	if( found != std::string::npos )
		CreatePath( lDirectoryName.substr( 0, found + 1 ) );
	else
		LogError << "Is \"" << lDirectoryName << "\" really a location I can write to? Please report this line." << std::endl;
	
	

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

bool MPQFileExists( std::string filename )
{
	FixFilePath( filename );
	
	uint32_t filenum;
	
	for( ArchiveSet::reverse_iterator i = gOpenArchives.rbegin( ); i != gOpenArchives.rend( ); ++i )
		if( libmpq__file_number( *i, filename.c_str( ), &filenum ) != LIBMPQ_ERROR_EXIST )
			return true;

	std::string lTemp = Project::getInstance( )->getPath( ), lTemp2 = lTemp;
	lTemp.append( filename );
	lTemp2.append( "Data/" ).append( filename );
	
	if( FileExists( lTemp ) || FileExists( lTemp2 ) )
	{
		Log << "The file \"" << filename << "\" was found on the disk." << std::endl;
		return true;
	}
	
	return false;
}

#if 0
int _id=1;

MPQArchive::MPQArchive(const char* filename)
{
	BOOL succ = SFileOpenArchive(filename, _id++, 0, &handle);
	if (succ) {
		MPQARCHIVE *ar = (MPQARCHIVE*)(handle);
        succ = true;
	} else {
		gLog("Error opening archive %s\n", filename);
	}
}

void MPQArchive::close()
{
	SFileCloseArchive(handle);
}

MPQFile::MPQFile(const char* filename)
{
	BOOL succ = SFileOpenFile(filename, &handle);
	pointer = 0;
	if (succ) {
		DWORD s = SFileGetFileSize(handle, 0);
		buffer = new char[s];
		SFileReadFile(handle, buffer, s, 0, 0);
		SFileCloseFile(handle);
		size = (size_t)s;
		eof = false;
	} else {
		eof = true;
		buffer = 0;
	}
}

size_t MPQFile::read(void* dest, size_t bytes)
{
	if (eof) return 0;

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

size_t MPQFile::getPos()
{
	return pointer;
}

char* MPQFile::getBuffer()
{
	return buffer;
}

char* MPQFile::getPointer()
{
	return buffer + pointer;
}


#endif
