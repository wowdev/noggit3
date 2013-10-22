// Log.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef __LOG_H
#define __LOG_H

#include <iostream>

std::ostream& _LogError( const char * pFile, int pLine );
std::ostream& _LogDebug( const char * pFile, int pLine );
std::ostream& _Log( const char * pFile, int pLine );

#define LogError _LogError( __FILE__, __LINE__ )
#define LogDebug _LogDebug( __FILE__, __LINE__ )
#define Log _Log( __FILE__, __LINE__ )

void InitLogging();

#endif
