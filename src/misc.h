#ifndef MISC_H
#define MISC_H

// class for static helper functions.

class misc
{
public:
	misc(void);
	~misc(void);
	static void find_and_replace( std::string &source, const std::string find, std::string replace ) ;
	static int FtoIround(float d);
};

#endif