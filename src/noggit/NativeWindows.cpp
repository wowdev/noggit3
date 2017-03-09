// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>
#include <noggit/Log.h>
#include <array>
#include "Native.hpp"

const char *kRegCompatPath = "SOFTWARE\\Wow6432Node\\Blizzard Entertainment\\World of Warcraft";
const char *kRegPTRPath    = "SOFTWARE\\Blizzard Entertainment\\World of Warcraft\\PTR";
const char *kRegStdPath    = "SOFTWARE\\Blizzard Entertainment\\World of Warcraft";
const char *kFolderDialogTitle = "Select your World of Warcraft installation:";

const std::string kNotFoundTitle = "Unable to locate game";
const std::string kNotFoundMessage = "Noggit was unable to locate World of Warcraft.\n"
                                     "Click OK to select the location of your Wrath of the Lich King (3.3.5) installation.";

LONG readRegistryKey(const char* path, HKEY *key)
{
	return RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE, key);
}

std::string Native::showFileChooser()
{
	TCHAR path[MAX_PATH] = { 0 };

	BROWSEINFO bInfo = { 0 };
	bInfo.pidlRoot = NULL;
	bInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
	bInfo.lpfn = NULL;
	bInfo.lParam = 0;
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);
	if (lpItem != NULL)
	{
		SHGetPathFromIDList(lpItem, path);
	}

	std::string stdPath = std::string(path);

	return stdPath + "\\";
}

std::string Native::getGamePath()
{
	Log << "Will try to load the game path from you registry now:" << std::endl;
	HKEY key;
	DWORD t;
	const DWORD s(1024);
	char temp[s];
	memset(temp, 0, s);

	LONG result;

	for (const char *path : { kRegCompatPath, kRegPTRPath, kRegStdPath }) {
		result = readRegistryKey(path, &key);
		if (result == ERROR_SUCCESS) { break; }
	}

	LONG queryResult = RegQueryValueEx(key, "InstallPath", 0, &t, (LPBYTE)temp, (LPDWORD)&s);

	if (result == ERROR_SUCCESS && queryResult == ERROR_SUCCESS) {
		return temp;
	}

	RegCloseKey(key);

  QMessageBox::critical(nullptr, "Unable to load WoW", QString::fromStdString(kNotFoundMessage));

	std::string gamePath = showFileChooser();

	return gamePath;
}

std::string Native::getConfigPath()
{
	return "noggit.conf";
}

#endif
