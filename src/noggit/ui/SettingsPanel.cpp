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

void saveSettings(UIFrame* f, int i)
{
    f->parent()->hide();
    Settings::getInstance()->saveToDisk();
}

void discardSettingsChanges(UIFrame* f, int i)
{
    f->parent()->hide();
    Settings::getInstance()->readFromDisk();
}

void chooseGamePath(UIFrame* f, int i)
{
    std::string path = Native::showFileChooser();
    Settings::getInstance()->gamePath = path;
    ((UISettings*)f->parent())->gamePathField->value(path);
}

void chooseProjectPath(UIFrame* f, int i)
{
    std::string path = Native::showFileChooser();
    Settings::getInstance()->projectPath = path;
    ((UISettings*)f->parent())->projectPathField->value(path);
}

void chooseWoDPath(UIFrame* f, int i)
{
    std::string path = Native::showFileChooser();
    Settings::getInstance()->wodSavePath = path;
    ((UISettings*)f->parent())->wodPathField->value(path);
}

void adjustFarZ(UIFrame*f, int i)
{
    int farZ = Settings::getInstance()->FarZ;
    farZ += i;
    farZ = std::max(farZ, 0);
    Settings::getInstance()->FarZ = farZ;
    ((UISettings*)f->parent())->farZField->value(std::to_string(farZ));
}

UISettings::UISettings()
  : UIWindow((float)video.xres() / 2.0f - (float)winWidth / 2.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f  , (float)winWidth, (float)winHeight)
{
    addChild(new UIText(winWidth/2, 5, "Settings", app.getFritz16(), eJustifyCenter));
    
    addChild(new UIText(30, 34, "Game Path:", app.getArial12(), eJustifyLeft));
    addChild(gamePathField = new UITextBox(99, 32, 286, 32, app.getArial12()));
    addChild(new UIButton(390, 31, 100, 32, "Browse…", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", chooseGamePath, 0));
    
    addChild(new UIText(26, 62, "Project Path:", app.getArial12(), eJustifyLeft));
    addChild(projectPathField = new UITextBox(99, 60, 286, 32, app.getArial12()));
    addChild(new UIButton(390, 59, 100, 32, "Browse…", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", chooseProjectPath, 0));
    
    addChild(new UIText(36, 90, "WoD Path:", app.getArial12(), eJustifyLeft));
    addChild(wodPathField = new UITextBox(99, 89, 286, 32, app.getArial12()));
    addChild(new UIButton(390, 88, 100, 32, "Browse…", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", chooseWoDPath, 0));
    
    addChild(new UIText(13, 122, "View Distance:", app.getArial12(), eJustifyLeft));
    addChild(farZField = new UITextBox(99, 120, 128, 32, app.getArial12()));
    addChild(new UIButton(195, 122, 16, 16, "", "Interface\\BUTTONS\\UI-MinusButton-Up.blp", "Interface\\BUTTONS\\UI-MinusButton-Down.blp", adjustFarZ, -256));
    addChild(new UIButton(210, 122, 16, 16, "", "Interface\\BUTTONS\\UI-AttributeButton-Encourage-Up.blp", "Interface\\BUTTONS\\UI-AttributeButton-Encourage-Down.blp", adjustFarZ, 256));
    
    addChild(new UIText(14, 154, "Editor Options:", app.getArial12(), eJustifyLeft));
    addChild(tabletModeCheck = new UICheckBox(95, 145, "Drawing tablet support", &Settings::getInstance()->tabletMode));
    addChild(autoselectCheck = new UICheckBox(95, 170, "Auto select mode", &Settings::getInstance()->AutoSelectingMode));
    addChild(modelsBoxCheck = new UICheckBox(95, 195, "Render models with box", &Settings::getInstance()->renderModelsWithBox));
    
    addChild(new UIText(285, 154, "Model Tool:", app.getArial12(), eJustifyLeft));
    addChild(randRotCheck = new UICheckBox(350, 145, "Random rotation", &Settings::getInstance()->random_rotation));
    addChild(randSizeCheck = new UICheckBox(350, 170, "Random size", &Settings::getInstance()->random_size));
    addChild(randTiltCheck = new UICheckBox(350, 195, "Random tilt", &Settings::getInstance()->random_tilt));
    addChild(modelStatsCheck = new UICheckBox(350, 220, "Copy model stats", &Settings::getInstance()->copyModelStats));
    
    addChild(new UITexture(5, height()-29, 16, 16, "Interface\\GossipFrame\\AvailableQuestIcon.blp"));
    addChild(new UIText(22, height()-29, "Changes may not take effect until next launch.", app.getArial12(), eJustifyLeft));
    
    addChild(new UIButton(width()-110, height()-32, 100, 32, "Save", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", saveSettings, 0));
    addChild(new UIButton(width()-215, height()-32, 100, 32, "Cancel", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", discardSettingsChanges, 0));
}

void UISettings::readInValues()
{
    gamePathField->value(Settings::getInstance()->gamePath);
    projectPathField->value(Settings::getInstance()->projectPath);
    farZField->value(std::to_string(Settings::getInstance()->FarZ));
    tabletModeCheck->setState(Settings::getInstance()->tabletMode);
    autoselectCheck->setState(Settings::getInstance()->AutoSelectingMode);
    modelStatsCheck->setState(Settings::getInstance()->copyModelStats);
    modelsBoxCheck->setState(Settings::getInstance()->renderModelsWithBox);
    randRotCheck->setState(Settings::getInstance()->random_rotation);
    randSizeCheck->setState(Settings::getInstance()->random_size);
    randTiltCheck->setState(Settings::getInstance()->random_tilt);
}

void UISettings::resize()
{
  x(std::max((video.xres() / 2.0f) - (winWidth / 2.0f), 0.0f));
  y(std::max((video.yres() / 2.0f) - (winHeight / 2.0f), 0.0f));
}
