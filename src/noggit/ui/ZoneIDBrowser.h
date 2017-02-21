// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QTreeWidget> 

#include <functional>
#include <string>

namespace ui
{
  class zone_id_browser : public QWidget
  {
  public:
    zone_id_browser();
    void setMapID(int id);
    void setZoneID(int id);
    void setChangeFunc(std::function<void(int)> f);

  private:
    std::function<void(int)> _func;
    QTreeWidget* _area_tree;
    std::map<int, QTreeWidgetItem*> _items;
    int mapID;

    void buildAreaList();
  };
}

