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

	float frand()
	{
			return rand()/(float)RAND_MAX;
	}

	float randfloat(float lower, float upper)
	{
		return lower + (upper-lower)*(rand()/(float)RAND_MAX);
	}

	int randint(int lower, int upper)
	{
			return lower + (int)((upper+1-lower)*frand());
	}

	std::string replaceSpezialChars(std::string text)
	{
		find_and_replace(text,"ä","ae");
		find_and_replace(text,"ö","oe");
		find_and_replace(text,"ü","ue");
		find_and_replace(text,"ß","ss");
		find_and_replace(text,"Ä","Ae");
		find_and_replace(text,"Ö","Oe");
		find_and_replace(text,"�oe","Ue");
		return text;
	}

	int getADTCord(float cord)
	{
		return (int)cord/533.33333;
	}

}