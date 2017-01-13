// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

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
