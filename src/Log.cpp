#include <fstream>

#include "Log.h"

std::ofstream gLogStream;

std::ostream& _LogError( const char * pFile, int pLine )
{
	return std::cerr << "(" << pFile << ":" << pLine << "): [Error] ";
}
std::ostream& _LogDebug( const char * pFile, int pLine )
{
	return std::clog << "(" << pFile << ":" << pLine << "): [Debug] ";
}
std::ostream& _Log( const char * pFile, int pLine )
{
	return std::cout << "(" << pFile << ":" << pLine << "): ";
}

void InitLogging( )
{
#if DEBUG__LOGGINGTOCONSOLE
	LogDebug << "Logging to console window." << std::endl;
#else
	// Set up log.
	gLogStream.open( "log.txt", std::ios_base::out | std::ios_base::trunc );
	if( gLogStream )
	{
		std::cout.rdbuf( gLogStream.rdbuf( ) );
		std::clog.rdbuf( gLogStream.rdbuf( ) );
		std::cerr.rdbuf( gLogStream.rdbuf( ) );
	}
#endif
}