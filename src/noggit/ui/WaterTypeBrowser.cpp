// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/WaterTypeBrowser.h>

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/ui/Water.h>

#include <iostream>
#include <sstream>
#include <string>

namespace ui
{
  water_type_browser::water_type_browser(UIWater* ui_water)
    : QListWidget(nullptr)
    , _ui_water(ui_water)
  {
    setWindowTitle("Water type selector");
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    for (DBCFile::Iterator i = gLiquidTypeDB.begin(); i != gLiquidTypeDB.end(); ++i)
    {
      int liquid_id = i->getInt(LiquidTypeDB::ID);

      std::stringstream ss;
      ss << liquid_id << "-" << LiquidTypeDB::getLiquidName(liquid_id);

      auto item (new QListWidgetItem (QString::fromUtf8 (ss.str().c_str()), this));
      item->setData (Qt::UserRole, QVariant (liquid_id));

      addItem(item);
    }

    connect ( this, &QListWidget::itemClicked
            , [&] (QListWidgetItem* item)
              {
                _ui_water->changeWaterType(item->data(Qt::UserRole).toInt());
              }
            );
  }

  void water_type_browser::toggle()
  {
    if (isVisible())
    {
      hide();
    }
    else
    {
      show();
    }
  }
}
