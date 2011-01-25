#include <string>
#include "misc.h"

misc::misc(void)
{
}


misc::~misc(void)
{
}


void misc::find_and_replace( std::string &source, const std::string find, std::string replace ) 
{
	int found = source.rfind( find );
	if( found != std::string::npos )
		source.replace( found, find.length(), replace );
}


//dirty hack
int misc::FtoIround(float d)
{
	return d<0 ? d-.5f : d+.5f;
}

char misc::roundc( float a )
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