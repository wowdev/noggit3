#include "MPQ.h"

#include <algorithm>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp> 
#include <cstdio>
#include <cstring>
#include <list>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

#include "Directory.h"
#include "Log.h"
#include "Project.h"

std::vector<MPQArchive*> gOpenArchives;

std::list<std::string> gListfile;
boost::mutex gListfileLoadingMutex;
boost::mutex gMPQFileMutex;

MPQArchive::MPQArchive(const std::string& filename, bool doListfile)
{
  if(!SFileOpenArchive( filename.c_str(), 0, MPQ_OPEN_NO_LISTFILE, &mpq_a ))
  {
    LogError << "Error opening archive: " << filename << "\n";
    return;
  }
  
  LogDebug << "Opened archive: " << filename << "\n";
  
  gOpenArchives.push_back( this );

  finished = !doListfile;
}

void MPQArchive::finishLoading()
{
  boost::mutex::scoped_lock lock(gListfileLoadingMutex);
  boost::mutex::scoped_lock lock2(gMPQFileMutex);
  
  if(finished)
    return;
    
  HANDLE fh;

  if( SFileOpenFileEx( mpq_a, "(listfile)", 0, &fh ) )
  {
    size_t filesize = SFileGetFileSize( fh );
  
    unsigned char *readbuffer = new unsigned char[filesize];
    SFileReadFile( fh, readbuffer, filesize );
    SFileCloseFile( fh );
  
   
	std::string list = std::string(reinterpret_cast<const char*>(readbuffer));
	
	boost::algorithm::to_lower( list ); 
	boost::algorithm::replace_all( list, "\r\n", "\n" ); 
	
	std::vector<std::string> temp; 
	boost::algorithm::split( temp, list, boost::algorithm::is_any_of("\n") ); 
	gListfile.insert(gListfile.end(), temp.begin(), temp.end());
	

    free(readbuffer);
  }
  
  finished = true;
  
  if(MPQArchive::allFinishedLoading())
  {
    gListfile.sort();
    gListfile.unique();
    LogDebug << "Completed listfile loading.\n";
  }
}

MPQArchive::~MPQArchive()
{
  SFileCloseArchive(mpq_a);
}

bool MPQArchive::allFinishedLoading()
{
  bool allFinished = true;
  for(std::vector<MPQArchive*>::const_iterator it = gOpenArchives.begin(); it != gOpenArchives.end(); ++it)
  {
    allFinished = allFinished && (*it)->finishedLoading();
  }
  return allFinished;
}

void MPQArchive::allFinishLoading()
{
  for(std::vector<MPQArchive*>::const_iterator it = gOpenArchives.begin(); it != gOpenArchives.end(); ++it)
  {
    (*it)->finishLoading();
  }
}

MPQFile::MPQFile( const std::string& filename ):
  eof(false),
  buffer(NULL),
  pointer(0),
  size(0),
  External(false)
{
  boost::mutex::scoped_lock lock(gMPQFileMutex);
  std::string diskpath = Project::getInstance()->getPath().append(filename);
  
  size_t found = diskpath.find( "\\" );
  while( found != std::string::npos )
  {
    diskpath.replace( found, 1, "/" );
    found = diskpath.find( "\\" );
  }
  
  FILE* fd = fopen( diskpath.c_str(), "rb" );
  
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
    //Log << "Opening file \"" << filename << "\" from disk." << std::endl;
    std::transform( fname.begin(), fname.end(), fname.begin(), ::tolower );
    return;
  }
  
  for(std::vector<MPQArchive*>::reverse_iterator i = gOpenArchives.rbegin(); i!=gOpenArchives.rend(); ++i)
  {
    HANDLE fh;
    
    if( !SFileOpenFileEx( (*i)->mpq_a, filename.c_str(), 0, &fh ) )
      continue;
    
    // Found!
    DWORD filesize = SFileGetFileSize( fh );
    size = filesize;
    
    // HACK: in patch.mpq some files don't want to open and give 1 for filesize
    if (size<=1) {
      eof = true;
      buffer = NULL;
      
      LogError << "size <= 1" << std::endl;
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

MPQFile::~MPQFile()
{
  close();
}

bool MPQFile::exists( const std::string& filename )
{
  boost::mutex::scoped_lock lock(gMPQFileMutex);
  for(std::vector<MPQArchive*>::iterator it=gOpenArchives.begin(); it!=gOpenArchives.end();++it)
    if( SFileHasFile( (*it)->mpq_a, filename.c_str() ) )
      return true;

  std::string diskpath = Project::getInstance()->getPath().append(filename);
  
  size_t found = diskpath.find( "\\" );
  while( found != std::string::npos )
  {
    diskpath.replace( found, 1, "/" );
    found = diskpath.find( "\\" );
  }
  
  FILE* fd = fopen( diskpath.c_str(), "rb" );

  if(fd!=NULL)
  {
    fclose(fd);
    return true;
  }

  return false;
}

void MPQFile::save(const char* /*filename*/)
{
  //! \todo Implement save to MPQ.
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

bool MPQFile::isEof() const
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
  if (buffer)
  {
    delete[] buffer;
    buffer = NULL;
  }
  eof = true;
}

size_t MPQFile::getSize() const
{
  return size;
}

void FixFilePath( std::string & pFilename )
{
  //std::transform( pFilename.begin(), pFilename.end(), pFilename.begin(), ::tolower );
  
  size_t found = pFilename.find( "/" );
  while( found != std::string::npos )
  {
    pFilename.replace( found, 1, "\\" );
    found = pFilename.find( "/" );
  }
}

int MPQFile::getSize( const std::string& filename )
{
  for(std::vector<MPQArchive*>::iterator i=gOpenArchives.begin(); i!=gOpenArchives.end();++i)
  {
    HANDLE fh;
    
    if( !SFileOpenFileEx( (*i)->mpq_a, filename.c_str(), 0, &fh ) )
      continue;

    size_t filesize = SFileGetFileSize( fh );
    SFileCloseFile( fh );
    return filesize;
  }

  return 0;
}

size_t MPQFile::getPos() const
{
  return pointer;
}

unsigned char* MPQFile::getBuffer() const
{
  return buffer;
}

unsigned char* MPQFile::getPointer() const
{
  return buffer + pointer;
}

void MPQFile::SaveFile()
{  
  FILE* fd;
  
  std::string lFilename = fname;
  //LogDebug << fname << std::endl;
  size_t found = lFilename.find( "\\" );
  while( found != std::string::npos )
  {
    lFilename.replace( found, 1, "/" );
    found = lFilename.find( "\\" );
  }
  //LogDebug << lFilename << std::endl;  
  std::string lDirectoryName = lFilename;  

  found = lDirectoryName.find_last_of("/\\");
  if( found != std::string::npos )
    CreatePath( lDirectoryName.substr( 0, found + 1 ) );
  else
    LogError << "Is \"" << lDirectoryName << "\" really a location I can write to? Please report this line." << std::endl;
    //LogDebug << lDirectoryName << std::endl;  
  

  fd = fopen( lFilename.c_str(), "wb" );

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


