#ifdef _WIN32

#include <windows.h>
#include "Native.hpp"

int Native::showAlertDialog(std::string title, std::string message)
{
	int msgboxID = MessageBox(
		NULL,
		message.c_str(),
		title.c_str(),
		MB_ICONEXCLAMATION | MB_OK
	);

	return msgboxID;
}

std::string Native::getGamePath()
{
	//Log << "Will try to load the game path from you registry now:" << std::endl;
	HKEY key;
	DWORD t;
	const DWORD s(1024);
	char temp[s];
	memset(temp, 0, s);
	LONG l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\Blizzard Entertainment\\World of Warcraft", 0, KEY_QUERY_VALUE, &key);
	if (l != ERROR_SUCCESS) {
		l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Blizzard Entertainment\\World of Warcraft\\PTR", 0, KEY_QUERY_VALUE, &key);
	}
	if (l != ERROR_SUCCESS) {
		l = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Blizzard Entertainment\\World of Warcraft", 0, KEY_QUERY_VALUE, &key);
	}
	if (l == ERROR_SUCCESS && RegQueryValueEx(key, "InstallPath", 0, &t, (LPBYTE)temp, (LPDWORD)&s) == ERROR_SUCCESS) {
		return temp;
	} 

	RegCloseKey(key);

	Native::showAlertDialog("Unable to locate World of Warcraft",
		"Please select the location of your Wrath of the Lich King (3.3.5) installation.");

	OPENFILENAME opendialog = { sizeof(OPENFILENAME) };
	std::string path(MAX_PATH, '\0');

	opendialog.lStructSize = sizeof(opendialog);
	opendialog.hInstance = GetModuleHandle(NULL);
	opendialog.lpstrFile = &path[0];
	opendialog.nFilterIndex = 0;
	opendialog.nMaxFile = 256;
	opendialog.lpstrInitialDir = NULL;
	opendialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&opendialog)) {
		path.resize(strlen(path.c_str()));
	} else {
		return "";
	}

	return path;
}

std::string Native::getArialPath()
{
	//! \todo This might not work on windows 7 or something. Please fix.
	return "C:\\windows\\fonts\\arial.ttf";
}

#endif
