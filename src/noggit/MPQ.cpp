// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncLoader.h> // AsyncLoader
#include <noggit/Log.h>
#include <noggit/MPQ.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include <QtCore/QSettings>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>

namespace
{
  typedef std::pair<std::string, std::unique_ptr<MPQArchive>> ArchiveEntry;
  typedef std::list<ArchiveEntry> ArchivesMap;
  ArchivesMap _openArchives;

  boost::mutex gListfileLoadingMutex;
  boost::mutex gMPQFileMutex;
}

std::unordered_set<std::string> gListfile;

void MPQArchive::loadMPQ (AsyncLoader* loader, std::string const& filename, bool doListfile)
{
  _openArchives.emplace_back (filename, std::make_unique<MPQArchive> (filename, doListfile));
  loader->queue_for_load(_openArchives.back().second.get());
}

MPQArchive::MPQArchive(std::string const& filename_, bool doListfile)
  : AsyncObject(filename_)
  ,_archiveHandle(nullptr)
{
  if (!SFileOpenArchive (filename.c_str(), 0, MPQ_OPEN_NO_LISTFILE | STREAM_FLAG_READ_ONLY, &_archiveHandle))
  {
    LogError << "Error opening archive: " << filename << std::endl;
    return;
  }
  else
  {
    LogDebug << "Opened archive " << filename << std::endl;
  }

  finished = !doListfile;
}

void MPQArchive::finishLoading()
{
  if (finished)
    return;

  HANDLE fh;

  boost::mutex::scoped_lock lock2(gMPQFileMutex);
  boost::mutex::scoped_lock lock(gListfileLoadingMutex);

  if (SFileOpenFileEx(_archiveHandle, "(listfile)", 0, &fh))
  {
    size_t filesize = SFileGetFileSize(fh, nullptr); //last nullptr for newer version of StormLib

    std::vector<char> readbuffer (filesize);
    SFileReadFile(fh, readbuffer.data(), filesize, nullptr, nullptr); //last nullptrs for newer version of StormLib
    SFileCloseFile(fh);

    std::string current;
    for (char c : readbuffer)
    {
      if (c == '\r')
      {
        continue;
      }
      if (c == '\n')
      {
        gListfile.emplace (noggit::mpq::normalized_filename (current));
        current.resize (0);
      }
      else
      {
        current += c;
      }
    }

    if (!current.empty())
    {
      gListfile.emplace (noggit::mpq::normalized_filename (current));
    }
  }

  finished = true;
  _state_changed.notify_all();

  if (MPQArchive::allFinishedLoading())
  {
    LogDebug << "Completed listfile loading: " << gListfile.size() << " files\n";
  }
}

MPQArchive::~MPQArchive()
{
  if (_archiveHandle)
    SFileCloseArchive(_archiveHandle);
}

bool MPQArchive::allFinishedLoading()
{
  return std::all_of ( _openArchives.begin(), _openArchives.end()
                     , [] (ArchiveEntry const& archive)
                       {
                         return archive.second->finishedLoading();
                       }
                     );
}

void MPQArchive::allFinishLoading()
{
  for (auto& archive : _openArchives)
  {
    archive.second->finishLoading();
  }
}

void MPQArchive::unloadAllMPQs()
{
  _openArchives.clear();
}

bool MPQArchive::hasFile(std::string const& file) const
{
  return SFileHasFile(_archiveHandle, noggit::mpq::normalized_filename_insane (file).c_str());
}

void MPQArchive::unloadMPQ(std::string const& filename)
{
  for (auto it = _openArchives.begin(); it != _openArchives.end(); ++it)
  {
    if (it->first == filename)
    {
      _openArchives.erase(it);
    }
  }
}

bool MPQArchive::openFile(std::string const& file, HANDLE* fileHandle) const
{
  assert(fileHandle);
  return SFileOpenFileEx(_archiveHandle, noggit::mpq::normalized_filename_insane (file).c_str(), 0, fileHandle);
}

namespace
{
  boost::filesystem::path getDiskPath (std::string const& pFilename)
  {
    QSettings settings;
    return boost::filesystem::path (settings.value ("project/path").toString().toStdString())
      / noggit::mpq::normalized_filename (pFilename);
  }

  bool existsInMPQ (std::string const& filename)
  {
    return std::any_of ( _openArchives.begin(), _openArchives.end()
                       , [&] (ArchiveEntry const& archive)
                         {
                           return archive.second->hasFile (filename);
                         }
                       );
  }
}

/*
* basic constructor to save the file to project path
*/
MPQFile::MPQFile(std::string const& filename)
  : eof(true)
  , pointer(0)
  , External(false)
  , _disk_path (getDiskPath (filename))
{
  if (filename.empty())
    throw std::runtime_error("MPQFile: filename empty");

  boost::mutex::scoped_lock lock(gMPQFileMutex);

  std::ifstream input(_disk_path.string(), std::ios_base::binary | std::ios_base::in);
  if (input.is_open())
  {
    External = true;
    eof = false;

    input.seekg(0, std::ios::end);
    buffer.resize (input.tellg());
    input.seekg(0, std::ios::beg);

    input.read(buffer.data(), buffer.size());

    input.close();
    return;
  }

  for (ArchivesMap::reverse_iterator i = _openArchives.rbegin(); i != _openArchives.rend(); ++i)
  {
    HANDLE fileHandle;

    if (!i->second->openFile(filename, &fileHandle))
      continue;

    eof = false;
    buffer.resize (SFileGetFileSize(fileHandle, nullptr));
    SFileReadFile(fileHandle, buffer.data(), buffer.size(), nullptr, nullptr); //last nullptrs for newer version of StormLib
    SFileCloseFile(fileHandle);

    return;
  }

  throw std::invalid_argument ("File '" + filename + "' does not exist.");
}

MPQFile::~MPQFile()
{
  close();
}

bool MPQFile::exists (std::string const& filename)
{
  return existsOnDisk (filename) || existsInMPQ (filename);
}
bool MPQFile::existsOnDisk (std::string const& filename)
{
  return boost::filesystem::exists (getDiskPath (filename));
}

size_t MPQFile::read(void* dest, size_t bytes)
{
  if (eof || !bytes)
    return 0;

  size_t rpos = pointer + bytes;
  if (rpos > buffer.size()) {
    bytes = buffer.size() - pointer;
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

void MPQFile::seek(size_t offset)
{
  pointer = offset;
  eof = (pointer >= buffer.size());
}

void MPQFile::seekRelative(size_t offset)
{
  pointer += offset;
  eof = (pointer >= buffer.size());
}

void MPQFile::close()
{
  eof = true;
}

size_t MPQFile::getSize() const
{
  return buffer.size();
}

size_t MPQFile::getPos() const
{
  return pointer;
}

char const* MPQFile::getBuffer() const
{
  return buffer.data();
}

char const* MPQFile::getPointer() const
{
  return buffer.data() + pointer;
}

void MPQFile::SaveFile()
{
  LogDebug << "Save file to: " << _disk_path << std::endl;

  auto const directory_name (_disk_path.parent_path());
  boost::system::error_code ec;
  boost::filesystem::create_directories (directory_name, ec);
  if (ec)
  {
    LogError << "Creating directory \"" << directory_name << "\" failed: " << ec << ". Saving is highly likely to fail." << std::endl;
  }

  std::ofstream output(_disk_path.string(), std::ios_base::binary | std::ios_base::out);
  if (output.is_open())
  {
    NOGGIT_LOG << "Saving file \"" << _disk_path << "\"." << std::endl;

    output.write(buffer.data(), buffer.size());
    output.close();

    External = true;
  }
}

namespace noggit
{
  namespace mpq
  {
    std::string normalized_filename (std::string filename)
    {
      std::transform (filename.begin(), filename.end(), filename.begin(), ::tolower);
      std::transform ( filename.begin(), filename.end(), filename.begin()
                     , [] (char c)
                       {
                         return c == '\\' ? '/' : c;
                       }
                     );
      return filename;
    }
    std::string normalized_filename_insane (std::string filename)
    {
      std::transform (filename.begin(), filename.end(), filename.begin(), ::toupper);
      std::transform ( filename.begin(), filename.end(), filename.begin()
                     , [] (char c)
                       {
                         return c == '/' ? '\\' : c;
                       }
                     );
      return filename;
    }
  }
}
