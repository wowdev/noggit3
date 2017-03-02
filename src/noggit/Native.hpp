// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <string>

#include <QtWidgets/QMessageBox>

class Native {
public:
	static std::string getArialPath();
    static std::string getGamePath();
    static std::string getConfigPath();
    static std::string showFileChooser();
};
