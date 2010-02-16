/*
 * SMem.hpp
 *
 *  Created on: 09.07.2009
 *      Author: schlumpf
 */
#include <stdio.h>
#include <stdlib.h>
#ifndef SMEM_HPP_
#define SMEM_HPP_
#define SMemNew(x) SMem::New(x, __FILE__, __LINE__)
#define SMemDelete(x) SMem::Delete(x, __FILE__, __LINE__)
#define SMemRenew(x) SMem::Delete(x, __FILE__, __LINE__)

class SMem
{
public:
	static char * New( int size, char * file, int line )
	{
		char * buffer =(char*) malloc ( size );
		if( buffer == NULL )
			printf( "\n\nCould not allocate memory %d in %s at line %i!\n\n",size, file, line );
		return buffer;
	}
	static void Delete( char * buffer, char * file, int line )
	{
		free( buffer );
	}
	static char * Renew( char * buffer, int size, char * file, int line )
	{
		buffer =(char *) realloc( buffer, size );
		if( buffer == NULL )
			printf( "\n\nCould not reallocate memory in %s at line %i!\n\n", file, line );
		return buffer;
	}
};

#endif /* SMEM_HPP_ */
