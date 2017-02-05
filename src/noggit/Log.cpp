// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Log.h>

#include <cstring>
#include <ctime>
#include <fstream>

std::ostream& _LogError(const char * pFile, int pLine)
{
  return std::cerr << clock() * 1000 / CLOCKS_PER_SEC << " - (" << ((strrchr(pFile, '/') ? strrchr(pFile, '/') : (strrchr(pFile, '\\') ? strrchr(pFile, '\\') : pFile - 1)) + 1) << ":" << pLine << "): [Error] ";
}
std::ostream& _LogDebug(const char * pFile, int pLine)
{
  return std::clog << clock() * 1000 / CLOCKS_PER_SEC << " - (" << ((strrchr(pFile, '/') ? strrchr(pFile, '/') : (strrchr(pFile, '\\') ? strrchr(pFile, '\\') : pFile - 1)) + 1) << ":" << pLine << "): [Debug] ";
}
std::ostream& _Log(const char * pFile, int pLine)
{
  return std::cout << clock() * 1000 / CLOCKS_PER_SEC << " - (" << ((strrchr(pFile, '/') ? strrchr(pFile, '/') : (strrchr(pFile, '\\') ? strrchr(pFile, '\\') : pFile - 1)) + 1) << ":" << pLine << "): ";
}

#if DEBUG__LOGGINGTOCONSOLE
void InitLogging()
{
  LogDebug << "Logging to console window." << std::endl;
}
#else
namespace
{
  std::ofstream gLogStream;
}
void InitLogging()
{
  // Set up log.
  gLogStream.open("log.txt", std::ios_base::out | std::ios_base::trunc);
  if (gLogStream)
  {
    std::cout.rdbuf(gLogStream.rdbuf());
    std::clog.rdbuf(gLogStream.rdbuf());
    std::cerr.rdbuf(gLogStream.rdbuf());
  }
}
#endif
