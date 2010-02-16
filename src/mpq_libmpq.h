#ifndef MPQ_H
#define MPQ_H

#include <string>
//#include "SFmpqapi.h"
#include <libmpq/mpq.h>

bool MPQFileExists( const std::string filename );

class MPQArchive
{
private:
	mpq_archive_s * mpq_a;
	
public:
	MPQArchive( const std::string filename, bool doListfile = false );
	void close( );
};


class MPQFile
{
	std::string fname;
	//MPQHANDLE handle;
	bool eof;
	uint8_t *buffer;
	off_t pointer,size;
	
	bool External;
	
	// disable copying
	MPQFile(const MPQFile &f) {}
	void operator=(const MPQFile &f) {}

public:
	MPQFile( std::string filename);	// filenames are not case sensitive
	MPQFile( std::string filename, bool Fake);
	~MPQFile();
	size_t read(void* dest, size_t bytes);
	size_t getSize();
	size_t getPos();
	uint8_t* getBuffer();
	uint8_t* getPointer();
	bool isEof();
	void seek(int offset);
	void seekRelative(int offset);
	void close();
	bool isExternal(){return External;};	

	void setBuffer(uint8_t *Buf, unsigned int Size)
	{
		if(buffer)
			delete buffer;
		buffer=Buf;
		size=Size;
	};
	void SaveFile();
};

#endif

