// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#if defined(__linux__) || defined(__unix__)

#include <noggit/Log.h>
#include <array>
#include "Native.hpp"

int Native::showAlertDialog(std::string title, std::string message)
{
    return 0;
}

std::string Native::getGamePath()
{
    return "";
}

std::string showFileChooser()
{
	return "";
}

std::string Native::getConfigPath()
{
	return "noggit.conf"
}

std::string Native::getArialPath()
{
	return "arial.ttf";
}

#endif
