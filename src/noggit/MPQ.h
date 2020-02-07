// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncObject.h>

#include <StormLib.h>

#include <boost/filesystem/path.hpp>

#include <set>
#include <string>
#include <unordered_set>
#include <vector>

class AsyncLoader;
class MPQArchive;
class MPQFile;

extern std::unordered_set<std::string> gListfile;

class MPQArchive : public AsyncObject
{
  HANDLE _archiveHandle;

public:
  MPQArchive(const std::string& filename, bool doListfile);

  ~MPQArchive();

  bool hasFile(const std::string& filename) const;
  bool openFile(const std::string& filename, HANDLE* fileHandle) const;

  void finishLoading();

  static bool allFinishedLoading();
  static void allFinishLoading();

  static void loadMPQ (AsyncLoader*, const std::string& filename, bool doListfile = false);
  static void unloadAllMPQs();
  static void unloadMPQ(const std::string& filename);

  friend class MPQFile;
};


class MPQFile
{
  bool eof;
  std::vector<char> buffer;
  size_t pointer;


  bool External;
  boost::filesystem::path _disk_path;
  std::string _mpq_path;

public:
  explicit MPQFile(const std::string& pFilename);  // filenames are not case sensitive, the are if u dont use a filesystem which is kinda shitty...

  MPQFile() = delete;
  ~MPQFile();
  MPQFile(MPQFile const&) = delete;
  MPQFile(MPQFile&&) = delete;
  MPQFile& operator=(MPQFile const&) = delete;
  MPQFile& operator=(MPQFile&&) = delete;

  size_t read(void* dest, size_t bytes);
  size_t getSize() const;
  size_t getPos() const;
  char const* getBuffer() const;
  char const* getPointer() const;
  bool isEof() const;
  void seek(size_t offset);
  void seekRelative(size_t offset);
  void close();
  bool isExternal() const
  {
    return External;
  }

  template<typename T>
  const T* get(size_t offset) const
  {
    return reinterpret_cast<T const*>(buffer.data() + offset);
  }

  void setBuffer (std::vector<char> const& vec)
  {
    buffer = vec;
  }

  void SaveFile();

  static bool exists (std::string const& filename);
  static bool existsOnDisk (std::string const& filename);

  friend class MPQArchive;
};

namespace noggit
{
  namespace mpq
  {
    std::string normalized_filename (std::string filename);
    std::string normalized_filename_insane (std::string filename);
  }
}
