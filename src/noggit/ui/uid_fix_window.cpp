// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/uid_fix_window.hpp>

#include <noggit/application.h>
#include <noggit/ui/Button.h>
#include <noggit/map_index.hpp>
#include <noggit/Menu.h>
#include <noggit/World.h>


void fixAll(UIFrame* f, int)
{
  static_cast<ui::uid_fix_window*>(f->parent())->fixAllTiles();
}

void maxUID(UIFrame* f, int)
{
  static_cast<ui::uid_fix_window*>(f->parent())->getMaxUID();
}

namespace ui
{
  uid_fix_window::uid_fix_window(Menu* menu)
    : UIWindow((video.xres() - winWidth) / 2.0f, (video.yres() - winHeight) / 2.0f, winWidth, winHeight)
    , _menuLink(menu)
  {
    addChild(new UIText(10.0f, 10.0f, "Required: Models UID fix (run only once)", app.getArial14(), eJustifyLeft));

    addChild(new UIButton(5.0f, 40.0f, 66.6f, 35.0f, "Fix All", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", fixAll, 0));
    addChild(new UIText(80.0f, 40.0f, "remove duplicates / fix collisions", app.getArial14(), eJustifyLeft));
    addChild(new UIText(10.0f, 65.0f, "Warning /!\\", app.getArial14(), eJustifyLeft));
    addChild(new UIText(10.0f, 85.0f, "recommended for custom maps, edit all adts", app.getArial14(), eJustifyLeft));

    addChild(new UIText(5.0f, 110.0f, "Or", app.getArial14(), eJustifyLeft));

    addChild(new UIButton(5.0f, 135.0f, 100.0f, 35.0f, "Get Max UID", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", maxUID, 0));
    addChild(new UIText(120.0f, 135.0f, "only prevent new UID bug", app.getArial14(), eJustifyLeft));
    addChild(new UIText(10.0f, 160.0f, "recommended for blizzard's maps", app.getArial14(), eJustifyLeft));
  }

  void uid_fix_window::enterAt(math::vector_3d const& pos)
  {
    _pos = pos;
    show();
  }

  void uid_fix_window::fixAllTiles()
  {
    hide();
    gWorld->mapIndex.fixUIDs();
    _menuLink->enterMapAt(_pos);
  }

  void uid_fix_window::getMaxUID()
  {
    hide();
    gWorld->mapIndex.searchMaxUID();
    _menuLink->enterMapAt(_pos);
  }
}
