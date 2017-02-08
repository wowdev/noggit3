#ifdef _WIN32

#include <windows.h>
#include "Native.hpp"

int Native::showAlertDialog(std::string title, std::string message)
{
	int msgboxID = MessageBox(
		NULL,
		title.c_str(),
		message.c_str(),
		MB_ICONEXCLAMATION | MB_YESNO
	);

	if (msgboxID == IDYES)
	{
		// TODO: add code
	}

	return msgboxID;
}

#endif
