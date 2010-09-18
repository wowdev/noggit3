#ifndef __ENVIRONMENT_H
#define __ENVIRONMENT_H

#include <string>

#include "selection.h"

class Environment
{
public:
	static Environment* getInstance();
	nameEntry get_clipboard();
	void set_clipboard(nameEntry* entry);

	bool view_holelines;
	std::vector<int> areaIDs; // List of all area IDs to draw them with different colors
	// hold keys
	bool ShiftDown;
	bool AltDown;
	bool CtrlDown;
	
	bool AutoSelecting;		// If true the auto selection is active

private:
	Environment();
	static Environment* instance;

	nameEntry clipboard;
};

#endif
