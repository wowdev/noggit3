// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/SettingsPanel.h>

#include <algorithm>

#include <noggit/application.h> // fonts
#include <noggit/Settings.h>
#include <noggit/ui/Button.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/TextBox.h>
#include <noggit/ui/Texture.h>
#include <noggit/ui/MinimizeButton.h>
#include <noggit/Native.hpp>
#include <noggit/Video.h> // video

#include "revision.h"

UISettings::UISettings()
  : UIWindow((float)video.xres() / 2.0f - (float)winWidth / 2.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f  , (float)winWidth, (float)winHeight)
{
    addChild(new UIMinimizeButton(width()));
//  addChild(new UITexture(20.0f, 20.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Potion_83.blp"));
  addChild(new UIText(10, 5, "Settings", app.getFritz16(), eJustifyLeft));
    addChild(new UIText(30, 34, "Game Path:", app.getArial12(), eJustifyLeft));
    addChild(gamePathField = new UITextBox(99, 32, 286, 32, app.getArial12()));
    addChild(new UIButton(390, 31, 100, 32, "Browse…", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", Native::getGamePath));
    gamePathField->value(Settings::getInstance()->gamePath);
    
    addChild(new UIText(26, 62, "Project Path:", app.getArial12(), eJustifyLeft));
    addChild(projectPathField = new UITextBox(99, 60, 286, 32, app.getArial12()));
    addChild(new UIButton(390, 59, 100, 32, "Browse…", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"));
    projectPathField->value(Settings::getInstance()->projectPath);
    
    addChild(new UIText(36, 92, "WoD Path:", app.getArial12(), eJustifyLeft));
    addChild(wodPathField = new UITextBox(99, 90, 286, 32, app.getArial12()));
    addChild(new UIButton(390, 89, 100, 32, "Browse…", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"));
    
    addChild(new UIText(12, 122, "View Distance:", app.getArial12(), eJustifyLeft));
    addChild(farZField = new UITextBox(99, 120, 128, 32, app.getArial12()));
    farZField->value(std::to_string(Settings::getInstance()->FarZ));
    
    addChild(new UICheckBox(95, 145, "Enable tablet mode", &Settings::getInstance()->tabletMode));
    addChild(new UICheckBox(95, 170, "Enable auto-selecting mode", &Settings::getInstance()->AutoSelectingMode));
    addChild(new UICheckBox(95, 195, "Copy model stats", &Settings::getInstance()->copyModelStats));
    addChild(new UICheckBox(95, 220, "Render models with box", &Settings::getInstance()->renderModelsWithBox));
    addChild(new UICheckBox(95, 245, "Random rotation", &Settings::getInstance()->random_rotation));
    addChild(new UICheckBox(95, 270, "Random size", &Settings::getInstance()->random_size));
    addChild(new UICheckBox(95, 295, "Random tilt", &Settings::getInstance()->random_tilt));
}

void UISettings::resize()
{
  x(std::max((video.xres() / 2.0f) - (winWidth / 2.0f), 0.0f));
  y(std::max((video.yres() / 2.0f) - (winHeight / 2.0f), 0.0f));
}
