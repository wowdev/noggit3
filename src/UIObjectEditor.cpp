#include "UIObjectEditor.h"

#include "Noggit.h" // fonts
#include "UIText.h"
#include "Video.h" // video
#include "World.h"

UIObjectEditor::UIObjectEditor(float x, float y)
   : UIWindow(x, y, 400.0f, 100.0f)
   , selected()
{
  addChild(new UIText(190.0f, 2.0f, "Object edit", app.getArial14(), eJustifyCenter));

}

void UIObjectEditor::pasteObject(Vec3D pos)
{
  if (selected.type == eEntry_Model || selected.type == eEntry_WMO)
  {
    gWorld->addModel(selected, pos, true);
  }
}

void UIObjectEditor::copy(nameEntry entry)
{
  if (entry.type == eEntry_Model || entry.type == eEntry_WMO)
  {
    selected = entry;
  }    
}