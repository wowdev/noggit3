#ifndef MPQ_H
#define MPQ_H

#include <StormLib.h>

#include <string>
#include <set>
#include <vector>

#include "AsyncObject.h"

class MPQArchive;
class MPQFile;
extern std::vector<MPQArchive*> gOpenArchives;

class MPQArchive : public AsyncObject
{
	HANDLE mpq_a;
public:
	MPQArchive(const std::string& filename, bool doListfile = false );
	~MPQArchive();
	
	void finishLoading();
	
	static bool allFinishedLoading();
	
	friend class MPQFile;
};


class MPQFile
{
	bool eof;
	unsigned char *buffer;
	size_t pointer, size;

	// disable copying
	MPQFile(const MPQFile& /*f*/) { }
	void operator=(const MPQFile& /*f*/) { }

	bool External;
	std::string fname;

public:
	MPQFile():eof(false),buffer(0),pointer(0),size(0) {}
	MPQFile(const std::string& filename);	// filenames are not case sensitive
	~MPQFile();
	size_t read(void* dest, size_t bytes);
	size_t getSize() const;
	size_t getPos() const;
	unsigned char* getBuffer() const;
	unsigned char* getPointer() const;
	bool isEof() const;
	void seek(int offset);
	void seekRelative(int offset);
	void close();
	void save(const char* filename);
	bool isExternal() const
	{
		return External;
	}

	void setBuffer(unsigned char *Buf, unsigned int Size)
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
	static int getSize( const std::string& filename ); // Used to do a quick check to see if a file is corrupted
	
	friend class MPQArchive;
};

#endif

