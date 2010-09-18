#ifndef __PROJECT_H
#define __PROJECT_H

#include <string>

class Project
{
public:
	static Project* getInstance();
	void setPath(const std::string& setPath);
	std::string getPath();
private:
	Project();
    ~Project() {}
    static Project* instance;
	std::string path;
};

#endif
