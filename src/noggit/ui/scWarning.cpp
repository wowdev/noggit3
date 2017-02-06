// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/scWarning.h>

#include <noggit/application.h> // fonts
#include <noggit/ui/Text.h>
#include <noggit/ui/Button.h>
#include <noggit/ui/Texture.h>
#include <noggit/Video.h> // video
#include <noggit/World.h>
#include <noggit/map_index.hpp>

void save(UIFrame *f, int /*set*/)
{
	gWorld->mapIndex->savecurrent();
	(static_cast<UISaveCurrentWarning *>(f->parent()))->hide();
}

void abort(UIFrame *f, int /*set*/)
{
  (static_cast<UISaveCurrentWarning *>(f->parent()))->hide();
}

UISaveCurrentWarning::UISaveCurrentWarning(MapView *mapview)
  : UICloseWindow((float)video.xres() / 2.0f - (float)winWidth / 2.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f - (float)(video.yres() / 4), (float)winWidth, (float)winHeight, "")
{
  addChild(new UITexture(10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Misc_QuestionMark.blp"));
  addChild(new UIText(95.0f, 20.0f, "That can cause a collision Bug when placing Objects between two ADT Borders!\n", app.getArial14(), eJustifyLeft));
  addChild(new UIText(95.0f, 40.0f, "If you often use this function, we recommend you to use the 'Save all'\nfunction as often as possible to get the collisions right.", app.getArial14(), eJustifyLeft));
  addChild(new UIButton(this->width() - 420.0f, 90.0f, 100.0f, 30.0f, "OK", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", save, 1));
  addChild(new UIButton(this->width() - 300.0f, 90.0f, 100.0f, 30.0f, "Cancel", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", abort, 2));

}

void UISaveCurrentWarning::resize()
{
  x(std::max((video.xres() / 2.0f) - (winWidth / 2.0f), 0.0f));
  y(std::max((video.yres() / 2.0f) - (winHeight / 2.0f), 0.0f) - (video.yres() / 4));
}
