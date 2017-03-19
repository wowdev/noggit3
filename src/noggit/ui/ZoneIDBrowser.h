// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget>

#include <functional>
#include <string>

namespace noggit
{
  namespace ui
  {
    class zone_id_browser : public QWidget
    {
      Q_OBJECT

    public:
      zone_id_browser();
      void setMapID(int id);
      void setZoneID(int id);

    signals:
      void selected (int area_id);

    private:
      QTreeWidget* _area_tree;
      std::map<int, QTreeWidgetItem*> _items;
      int mapID;

      void buildAreaList();
    };
  }
}
