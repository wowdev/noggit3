#ifndef MPQ_H
#define MPQ_H

#include "StormLib.h"

// C++ files
#include <string>
#include <set>
#include <vector>

class MPQArchive
{
	HANDLE mpq_a;
public:
	MPQArchive(const std::string& filename, bool doListfile = false );
	~MPQArchive();

	void close();
};


class MPQFile
{
	bool eof;
	unsigned char *buffer;
	size_t pointer, size;

	// disable copying
	MPQFile(const MPQFile &f) { }
	void operator=(const MPQFile &f) { }

	bool External;
	std::string fname;

public:
	MPQFile():eof(false),buffer(0),pointer(0),size(0) {}
	MPQFile(const std::string& filename);	// filenames are not case sensitive
	~MPQFile();
	size_t read(void* dest, size_t bytes);
	size_t getSize();
	size_t getPos();
	unsigned char* getBuffer();
	unsigned char* getPointer();
	bool isEof();
	void seek(int offset);
	void seekRelative(int offset);
	void close();
	void save(const char* filename);
	bool isExternal(){return External;};

	void setBuffer(unsigned char *Buf, unsigned int Size)
	{
		if(buffer)
			delete buffer;
		buffer=Buf;
		size=Size;
	};
	
	void SaveFile();
	
	static bool exists( const std::string& filename );
	static int getSize( const std::string& filename ); // Used to do a quick check to see if a file is corrupted
};

/*inline void flipcc(char *fcc)
{
	char t;
	t=fcc[0];
	fcc[0]=fcc[3];
	fcc[3]=t;
	t=fcc[1];
	fcc[1]=fcc[2];
	fcc[2]=t;
}*/

inline bool defaultFilterFunc(const std::string&) { return true; }

#endif

