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
  std::string modmpqpath = "";//this will be the path to modders archive (with 'myworld' file inside)
}

std::unordered_set<std::string> gListfile;

void MPQArchive::loadMPQ (AsyncLoader* loader, const std::string& filename, bool doListfile)
{
  _openArchives.emplace_back (filename, std::make_unique<MPQArchive> (filename, doListfile));
  loader->addObject(_openArchives.back().second.get());
}

MPQArchive::MPQArchive(const std::string& filename, bool doListfile)
  : _archiveHandle(nullptr)
{
  if (!SFileOpenArchive(filename.c_str(), 0, MPQ_OPEN_NO_LISTFILE | STREAM_FLAG_READ_ONLY, &_archiveHandle))
  {
    LogError << "Error opening archive: " << filename << "\n";
    return;
  }
  else
  {
    LogDebug << "Opened archive " << filename << "\n";
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
  bool allFinished = true;
  for (ArchivesMap::const_iterator it = _openArchives.begin(); it != _openArchives.end(); ++it)
  {
    allFinished = allFinished && it->second->finishedLoading();
  }
  return allFinished;
}

void MPQArchive::allFinishLoading()
{
  for (ArchivesMap::iterator it = _openArchives.begin(); it != _openArchives.end(); ++it)
  {
    it->second->finishLoading();
  }
}

void MPQArchive::unloadAllMPQs()
{
  _openArchives.clear();
}

bool MPQArchive::hasFile(const std::string& filename) const
{
  return SFileHasFile(_archiveHandle, filename.c_str());
}

void MPQArchive::unloadMPQ(const std::string& filename)
{
  for (ArchivesMap::iterator it = _openArchives.begin(); it != _openArchives.end(); ++it)
  {
    if (it->first == filename)
    {
      _openArchives.erase(it);
    }
  }
}

bool MPQArchive::openFile(const std::string& filename, HANDLE* fileHandle) const
{
  assert(fileHandle);
  return SFileOpenFileEx(_archiveHandle, filename.c_str(), 0, fileHandle);
}
/*
* basic constructor to save the file to project path
*/
MPQFile::MPQFile(const std::string& pFilename)
  : eof(true)
  , pointer(0)
  , External(false)
{
  boost::mutex::scoped_lock lock(gMPQFileMutex);

  if (pFilename.empty())
    throw std::runtime_error("MPQFile: filename empty");
  if (!exists(pFilename))
    return;

  fname = getDiskPath(pFilename);

  std::ifstream input(fname.c_str(), std::ios_base::binary | std::ios_base::in);
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

  std::string filename(getMPQPath(pFilename));

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
}

MPQFile::~MPQFile()
{
  close();
}

std::string MPQFile::getDiskPath(const std::string &pFilename)
{
  std::string filename(pFilename);
  std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
  QSettings settings;
  std::string diskpath = settings.value("project/path").toString().toStdString().append(filename);

  size_t found = diskpath.find("\\");
  while (found != std::string::npos)
  {
    diskpath.replace(found, 1, "/");
    found = diskpath.find("\\");
  }

  return diskpath;
}

std::string MPQFile::getMPQPath(const std::string &pFilename)
{
  std::string filename(pFilename);
  std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

  size_t found = filename.find("/");
  while (found != std::string::npos)
  {
    filename.replace(found, 1, "\\");
    found = filename.find("/");
  }

  return filename;
}

bool MPQFile::exists(const std::string& pFilename)
{
  return (existsOnDisk(pFilename) || existsInMPQ(pFilename));
}

bool MPQFile::existsOnDisk(const std::string &pFilename)
{
  std::string filename(getDiskPath(pFilename));
  return boost::filesystem::exists(filename);
}

bool MPQFile::existsInMPQ(const std::string &pFilename)
{
  std::string filename(getMPQPath(pFilename));

  for (ArchivesMap::reverse_iterator it = _openArchives.rbegin(); it != _openArchives.rend(); ++it)
    if (it->second->hasFile(filename))
      return true;

  return false;
}

size_t MPQFile::read(void* dest, size_t bytes)
{
  if (eof)
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

  std::string lFilename = fname;
  LogDebug << "Save file to: " << lFilename << std::endl;

  size_t found = lFilename.find("\\");
  while (found != std::string::npos)
  {
    lFilename.replace(found, 1, "/");
    found = lFilename.find("\\");
  }

  std::string lDirectoryName = lFilename;

  found = lDirectoryName.find_last_of("/\\");
  if (found == std::string::npos || !boost::filesystem::create_directories(lDirectoryName.substr(0, found + 1)))
  {
    LogError << "Is \"" << lDirectoryName << "\" really a location I can write to? Saving failed." << std::endl;
  }

  std::ofstream output(lFilename.c_str(), std::ios_base::binary | std::ios_base::out);
  if (output.is_open())
  {
    Log << "Saving file \"" << lFilename << "\"." << std::endl;

    output.write(buffer.data(), buffer.size());
    output.close();

    External = true;

    //! \todo Enable again. After fixing it.
    //save(lFilename.c_str());
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
  }
}
