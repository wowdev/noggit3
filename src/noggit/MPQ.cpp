#include "MPQ.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <list>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>

#include "Log.h"
#include "Project.h"
#include "AsyncLoader.h" // AsyncLoader
#include "Noggit.h" // app.loader()

typedef std::pair<std::string, MPQArchive*> ArchiveEntry;
typedef std::list<ArchiveEntry> ArchivesMap;
ArchivesMap _openArchives;

std::list<std::string> gListfile;
boost::mutex gListfileLoadingMutex;
boost::mutex gMPQFileMutex;
std::string modmpqpath = "";//this will be the path to modders archive (with 'myworld' file inside)

void MPQArchive::loadMPQ(const std::string& filename, bool doListfile)
{
	_openArchives.push_back(ArchiveEntry(filename, new MPQArchive(filename, doListfile)));
	app.loader()->addObject(_openArchives.back().second);
}

MPQArchive::MPQArchive(const std::string& filename, bool doListfile)
	: _archiveHandle(NULL)
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
		size_t filesize = SFileGetFileSize(fh, NULL); //last NULL for newer version of StormLib

        char* readbuffer = new char[filesize];
		SFileReadFile(fh, readbuffer, filesize, NULL, NULL); //last NULLs for newer version of StormLib
		SFileCloseFile(fh);

        std::string list(readbuffer);
        boost::algorithm::to_lower(list);
        boost::algorithm::replace_all(list, "\r\n", "\n");

        std::vector<std::string> temp;
        boost::algorithm::split(temp, list, boost::algorithm::is_any_of("\n"));
        gListfile.insert(gListfile.end(), temp.begin(), temp.end());
        delete[] readbuffer;
	}

	finished = true;

	if (MPQArchive::allFinishedLoading())
	{
		gListfile.sort();
		gListfile.unique();
		LogDebug << "Completed listfile loading.\n";
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
	for (ArchivesMap::iterator it = _openArchives.begin(); it != _openArchives.end(); ++it)
	{
		delete it->second;
	}
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
			delete it->second;
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
	, buffer(NULL)
	, pointer(0)
	, size(0)
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
		size = (size_t)input.tellg();
		input.seekg(0, std::ios::beg);

		buffer = new char[size];
		input.read(buffer, size);

		input.close();
		return;
	}

	std::string filename(getMPQPath(pFilename));

	for (ArchivesMap::reverse_iterator i = _openArchives.rbegin(); i != _openArchives.rend(); ++i)
	{
		HANDLE fileHandle;

		if (!i->second->openFile(filename, &fileHandle))
			continue;

		size = SFileGetFileSize(fileHandle, NULL); //last NULL for newer version of StormLib

		eof = false;
		buffer = new char[size];
		SFileReadFile(fileHandle, buffer, size, NULL, NULL); //last NULLs for newer version of StormLib
		SFileCloseFile(fileHandle);

		return;
	}
}
/*
* Alternate constructor to save the file to an outside path
*/
MPQFile::MPQFile(const std::string& pFilename, const std::string& alternateSavePath)
	: eof(true)
	, buffer(NULL)
	, pointer(0)
	, size(0)
	, External(false)
{
	LogDebug << "MPGFILE 1 alternateSavePath: " << alternateSavePath << std::endl;
	boost::mutex::scoped_lock lock(gMPQFileMutex);

	if (pFilename.empty())
		throw std::runtime_error("MPQFile: filename empty");

	if(alternateSavePath.empty())if (!exists(pFilename))
		return;

	LogDebug << "WEITER!!! " << std::endl;

	fname = getAlternateDiskPath(pFilename, alternateSavePath);
	

	std::ifstream input(fname.c_str(), std::ios_base::binary | std::ios_base::in);
	if (input.is_open())
	{
		External = true;
		eof = false;

		input.seekg(0, std::ios::end);
		size = (size_t)input.tellg();
		input.seekg(0, std::ios::beg);

		buffer = new char[size];
		input.read(buffer, size);

		input.close();
		return;
	}

	std::string filename(getMPQPath(pFilename));

	for (ArchivesMap::reverse_iterator i = _openArchives.rbegin(); i != _openArchives.rend(); ++i)
	{
		HANDLE fileHandle;

		if (!i->second->openFile(filename, &fileHandle))
			continue;

		size = SFileGetFileSize(fileHandle, NULL); //last NULL for newer version of StormLib

		eof = false;
		buffer = new char[size];
		SFileReadFile(fileHandle, buffer, size, NULL, NULL); //last NULLs for newer version of StormLib
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
	std::string diskpath = Project::getInstance()->getPath().append(filename);

	size_t found = diskpath.find("\\");
	while (found != std::string::npos)
	{
		diskpath.replace(found, 1, "/");
		found = diskpath.find("\\");
	}

	return diskpath;
}

std::string MPQFile::getAlternateDiskPath(const std::string &pFilename, const std::string &pDiscpath)
{
	std::string filename(pFilename);
	std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
	std::string diskpath = pDiscpath;
	diskpath.append(filename);

	size_t found = diskpath.find("\\");
	while (found != std::string::npos)
	{
		diskpath.replace(found, 1, "/");
		found = diskpath.find("\\");
	}

	LogDebug << "Alternate disc patch: " << diskpath << std::endl;
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

void MPQFile::save(const char* filename)  //save to MPQ
{
	//! \todo Use const std::string& as parameter.
	//! \todo Get MPQ to save to via dialog or use development.MPQ.
	//! \todo Create MPQ nicer, if not existing.
	//! \todo Use a pointer to the archive instead of searching it by filename.
	//! \todo Format this code properly.
	HANDLE mpq_a;
	if (modmpqpath == "")//create new user's mods MPQ
	{
		std::string newmodmpq = Project::getInstance()->getPath().append("Data\\patch-9.MPQ");
		SFileCreateArchive(newmodmpq.c_str(), MPQ_CREATE_ARCHIVE_V2 | MPQ_CREATE_ATTRIBUTES, 0x40, &mpq_a);
		//! \note Is locale setting needed? LOCALE_NEUTRAL is windows only.
		SFileSetFileLocale(mpq_a, 0); // 0 = LOCALE_NEUTRAL.
		SFileAddFileEx(mpq_a, "shaders\\terrain1.fs", "myworld", MPQ_FILE_COMPRESS, MPQ_COMPRESSION_ZLIB, 0);//I must to add any file with name "myworld" so I decided to add terrain shader as "myworld". Last NULLs for newer version of StormLib
		SFileCompactArchive(mpq_a, nullptr, false); //last NULLs for newer version of StormLib
		SFileCloseArchive(mpq_a);
		modmpqpath = newmodmpq;
	}
	else
		MPQArchive::unloadMPQ(modmpqpath);

	SFileOpenArchive(modmpqpath.c_str(), 0, 0, &mpq_a);
	SFileSetLocale(0);
	std::string nameInMPQ = filename;
	nameInMPQ.erase(0, strlen(Project::getInstance()->getPath().c_str()));
	size_t found = nameInMPQ.find("/");
	while (found != std::string::npos)//fixing path to file
	{
		nameInMPQ.replace(found, 1, "\\");
		found = nameInMPQ.find("/");
	}
	if (SFileAddFileEx(mpq_a, filename, nameInMPQ.c_str(), MPQ_FILE_COMPRESS | MPQ_FILE_ENCRYPTED | MPQ_FILE_REPLACEEXISTING, MPQ_COMPRESSION_ZLIB, 0)) //last NULL for newer version of StormLib
	{
		LogDebug << "Added file " << fname.c_str() << " to archive \n";
	}
	else LogDebug << "Error " << GetLastError() << " on adding file to archive! Report this message \n";
	SFileCompactArchive(mpq_a, nullptr, false);//recompact our archive to avoid fragmentation. Last NULLs for newer version of StormLib
	SFileCloseArchive(mpq_a);
	new MPQArchive(modmpqpath, true);//now load edited archive to memory again
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

void MPQFile::seek(size_t offset)
{
	pointer = offset;
	eof = (pointer >= size);
}

void MPQFile::seekRelative(size_t offset)
{
	pointer += offset;
	eof = (pointer >= size);
}

void MPQFile::close()
{
	delete[] buffer;
	buffer = NULL;

	eof = true;
}

size_t MPQFile::getSize() const
{
	return size;
}

void FixFilePath(std::string& pFilename)
{
	//std::transform( pFilename.begin(), pFilename.end(), pFilename.begin(), ::tolower );

	size_t found = pFilename.find("/");
	while (found != std::string::npos)
	{
		pFilename.replace(found, 1, "\\");
		found = pFilename.find("/");
	}
}

size_t MPQFile::getPos() const
{
	return pointer;
}

char* MPQFile::getBuffer() const
{
	return buffer;
}

char* MPQFile::getPointer() const
{
	return buffer + pointer;
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

		output.write(buffer, size);
		output.close();

		External = true;

		//! \todo Enable again. After fixing it.
		//save(lFilename.c_str());
	}
}


