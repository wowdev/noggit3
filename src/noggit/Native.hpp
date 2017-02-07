// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <string>

class Native {
public:
    enum Platform {
        Mac,
        Windows,
        Unixlike,
    };
    
    static std::string getGamePath();
    static std::string showFileChooser();
    static int showAlertDialog(std::string title, std::string message);
};
