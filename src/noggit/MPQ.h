#ifndef MPQ_H
#define MPQ_H

#include <set>
#include <StormLib.h>
#include <string>
#include <vector>

#include <noggit/AsyncObject.h>

class QString;

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
  size_t pointer;
  size_t size;

  // disable copying
  MPQFile(const MPQFile& /*f*/) { }
  void operator=(const MPQFile& /*f*/) { }

  bool External;
  std::string fname;

public:
  explicit MPQFile (const QString& filename);
  explicit MPQFile(const std::string& filename);  // filenames are not case sensitive
  ~MPQFile();
  size_t read(void* dest, size_t bytes);
  size_t getSize() const;
  size_t getPos() const;
  char* getBuffer() const;
  char* getPointer() const;
  bool isEof() const;
  void seek(size_t offset);
  void seekRelative(size_t offset);
  void close();
  void save(const char* filename);
  bool isExternal() const
  {
    return External;
  }

  template<typename T>
  const T* get( size_t offset ) const
  {
    return reinterpret_cast<T*>( buffer + offset );
  }

  void setBuffer(char *Buf, size_t Size)
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

private:
  void open_file (const std::string& filename);

  friend class MPQArchive;
};

#endif

