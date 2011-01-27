#include <fstream>

#include "Log.h"



std::ofstream gLogStream;

std::ostream& _LogError( const char * pFile, int pLine )
{
	return std::cerr << "(" << ((strrchr(pFile, '/') ? strrchr(pFile, '/') : ( strrchr(pFile, '\\') ? strrchr(pFile, '\\') : pFile - 1 ) ) + 1) << ":" << pLine << "): [Error] ";
}
std::ostream& _LogDebug( const char * pFile, int pLine )
{
	return std::clog << "(" << ((strrchr(pFile, '/') ? strrchr(pFile, '/') : ( strrchr(pFile, '\\') ? strrchr(pFile, '\\') : pFile - 1 ) ) + 1) << ":" << pLine << "): [Debug] ";
}
std::ostream& _Log( const char * pFile, int pLine )
{
	return std::cout << "(" << ((strrchr(pFile, '/') ? strrchr(pFile, '/') : ( strrchr(pFile, '\\') ? strrchr(pFile, '\\') : pFile - 1 ) ) + 1) << ":" << pLine << "): ";
}

void InitLogging()
{
#if DEBUG__LOGGINGTOCONSOLE
	LogDebug << "Logging to console window." << std::endl;
#else
	// Set up log.
	gLogStream.open( "log.txt", std::ios_base::out | std::ios_base::trunc );
	if( gLogStream )
	{
		std::cout.rdbuf( gLogStream.rdbuf() );
		std::clog.rdbuf( gLogStream.rdbuf() );
		std::cerr.rdbuf( gLogStream.rdbuf() );
	}
#endif
}
