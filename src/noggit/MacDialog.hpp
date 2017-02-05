//
//  MacDialog.h
//  Noggit
//
//  Created by John Wells on 1/28/17.
//
//

#include <iostream>
#include <string>

int showAlertDialog(std::string title, std::string message);
bool checkWoWVersionAtPath(std::string);
std::string requestWoWPath();
std::string applicationSupportPath();
