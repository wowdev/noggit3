#include <string>
#include "misc.h"

namespace misc
{
void find_and_replace( std::string& source, const std::string& find, const std::string& replace ) 
{
	size_t found = source.rfind( find );
	if( found != std::string::npos )
		source.replace( found, find.length(), replace );
}

//dirty hack
int FtoIround(float d)
{
	return d<0 ? d-.5f : d+.5f;
}

char roundc( float a )
{
	if( a < 0 )
		a -= 0.5f;
	if( a > 0 )
		a += 0.5f;
	if( a < -127 )
		a = -127;
	else if( a > 127 )
		a = 127;
	return char( a );
}
}