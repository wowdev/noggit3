// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/CheckBox.h>

#include <string>

#include <noggit/application.h> // app.getArialn13()
#include <noggit/ui/Text.h>
#include <noggit/ui/Texture.h>
#include <noggit/ui/ToggleGroup.h>

UICheckBox::UICheckBox(float xPos, float yPos, const std::string& pText)
  : UIFrame(xPos, yPos, 30.0f, 30.0f)
  , check(new UITexture(0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp"))
  , text(new UIText(32.0f, 8.0f, pText, app.getArialn13(), eJustifyLeft))
  , checked(false)
  , clickFunc(nullptr)
  , mToggleGroup(nullptr)
{
  addChild(new UITexture(0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp"));
  check->hide();
  addChild(check);
  addChild(text);
}

UICheckBox::UICheckBox(float xPos, float yPos, const std::string& pText, ClickFunction function)
  : UIFrame(xPos, yPos, 30.0f, 30.0f)
  , check(new UITexture(0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp"))
  , text(new UIText(32.0f, 8.0f, pText, app.getArialn13(), eJustifyLeft))
  , checked(false)
  , clickFunc(function)
  , mToggleGroup(nullptr)
{
  addChild(new UITexture(0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp"));
  check->hide();
  addChild(check);
  addChild(text);
}

UICheckBox::UICheckBox(float xPos, float yPos, const std::string& pText, UIToggleGroup * pToggleGroup, int pToggleID)
  : UIFrame(xPos, yPos, 30.0f, 30.0f)
  , check(new UITexture(0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp"))
  , text(new UIText(32.0f, 8.0f, pText, app.getArialn13(), eJustifyLeft))
  , checked(false)
  , clickFunc(nullptr)
  , mToggleGroup(pToggleGroup)
{
  addChild(new UITexture(0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp"));
  check->hide();
  addChild(check);
  addChild(text);

  mToggleGroup->Add(this, pToggleID);
}

UICheckBox::UICheckBox (float x, float y, std::string const& name, bool* value)
  : UICheckBox (x, y, name, [value] (bool v) { *value = v; })
{
  setState (*value);
}

void UICheckBox::SetToggleGroup(UIToggleGroup * pToggleGroup, int pToggleID)
{
  mToggleGroup = pToggleGroup;
  if (mToggleGroup)
    mToggleGroup->Add(this, pToggleID);
  clickFunc = nullptr;
}

void UICheckBox::setText(const std::string& pText)
{
  text->setText(pText);
}

void UICheckBox::setState(bool c)
{
  checked = c;
  check->hidden(!checked);
}

bool UICheckBox::getState()
{
  return checked;
}

UIFrame* UICheckBox::processLeftClick(float /*mx*/, float /*my*/)
{
  checked = !checked;
  check->hidden(!checked);

  if (mToggleGroup)
    mToggleGroup->Activate(this);
  else if (clickFunc)
    clickFunc(checked);

  return this;
}
