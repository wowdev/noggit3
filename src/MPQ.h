#ifndef MPQ_H
#define MPQ_H

#include <set>
#include <StormLib.h>
#include <string>
#include <vector>

#include "AsyncObject.h"

class MPQArchive;
class MPQFile;

class MPQArchive : public AsyncObject
{
  HANDLE _archiveHandle;
  
  MPQArchive( const std::string& filename, bool doListfile );
  
public:
  ~MPQArchive();
  std::string mpqname;
  
  bool hasFile( const std::string& filename ) const;
  bool openFile( const std::string& filename, HANDLE* fileHandle ) const;

  void finishLoading();
  
  static bool allFinishedLoading();
  static void allFinishLoading();
  
  static void loadMPQ( const std::string& filename, bool doListfile = false );
  static void unloadAllMPQs();
  static void unloadMPQ( const std::string& filename );
  
  friend class MPQFile;
};


class MPQFile
{
  bool eof;
  char* buffer;
  size_t pointer, size;

  // disable copying
  MPQFile(const MPQFile& /*f*/) { }
  void operator=(const MPQFile& /*f*/) { }

  bool External;
  std::string fname;

public:
  explicit MPQFile(const std::string& filename);  // filenames are not case sensitive
  ~MPQFile();
  size_t read(void* dest, size_t bytes);
  size_t getSize() const;
  size_t getPos() const;
  char* getBuffer() const;
  char* getPointer() const;
  bool isEof() const;
  void seek(int offset);
  void seekRelative(int offset);
  void close();
  void save(const char* filename);
  bool isExternal() const
  {
    return External;
  }

  void setBuffer(char *Buf, unsigned int Size)
  {
    if(buffer)
    {
      delete buffer;
      buffer = NULL;
    }
    buffer=Buf;
    size=Size;
  };
  
  void SaveFile();
  
  static bool exists( const std::string& filename );
  
  friend class MPQArchive;
};

#endif

