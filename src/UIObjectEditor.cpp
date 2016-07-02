#include "UIObjectEditor.h"

#include "Noggit.h" // fonts
#include "UIText.h"
#include "Video.h" // video

UIObjectEditor::UIObjectEditor(float x, float y)
   : UIWindow(x, y, 400.0f, 100.0f)
{
  addChild(new UIText(190.0f, 2.0f, "Object edit", app.getArial14(), eJustifyCenter));

}