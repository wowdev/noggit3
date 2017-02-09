// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <string>

class Native {
public:
	static std::string getArialPath();
    static std::string getGamePath();
    static std::string showFileChooser();
    static int showAlertDialog(std::string title, std::string message);
};
