#ifndef MISC_H
#define MISC_H

// namespace for static helper functions.

namespace misc
{
	void find_and_replace( std::string& source, const std::string& find, const std::string& replace );
	int FtoIround(float d);
	char roundc(float a);
  float frand();
  float randfloat(float lower, float upper);
  int randint(int lower, int upper);
};


#endif