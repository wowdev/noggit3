// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ZoneIDBrowser.h>

#include <noggit/application.h>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Misc.h>

#include <QtWidgets/QVBoxLayout>

#include <iostream>
#include <sstream>
#include <string>

namespace noggit
{
  namespace ui
  {
    zone_id_browser::zone_id_browser()
      : QWidget(nullptr)
      , _area_tree(new QTreeWidget())
      , mapID(-1)
    {
      new QVBoxLayout(this);
      this->layout()->addWidget(_area_tree);

      connect ( _area_tree, &QTreeWidget::itemSelectionChanged
              , [this]
                {
                  auto const& selected_items = _area_tree->selectedItems();
                  if (selected_items.size())
                  {
                    emit selected (selected_items.back()->data(0, 1).toInt());
                  }
                }
              );
    }

    void zone_id_browser::setMapID(int id)
    {
      mapID = id;

      for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
      {
        if (i->getInt(MapDB::MapID) == id)
        {
          std::stringstream ss;
          ss << id << "-" << i->getString(MapDB::InternalName);
          _area_tree->setHeaderLabel(ss.str().c_str());
        }
      }

      buildAreaList();
    }

    void zone_id_browser::setZoneID(int id)
    {
      QSignalBlocker const block_area_tree(_area_tree);

      if (_items.find(id) != _items.end())
      {
        _area_tree->selectionModel()->clear();
        auto* item = _items.at(id);

        item->setSelected(true);

        while ((item = item->parent()))
        {
          item->setExpanded(true);
        }
      }
    }

    void zone_id_browser::buildAreaList()
    {
      QSignalBlocker const block_area_tree(_area_tree);
      _area_tree->clear();
      _area_tree->setColumnCount(1);
      _items.clear();

      //  Read out Area List.
      for (DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i)
      {
        if (i->getInt(AreaDB::Continent) == mapID)
        {
          int area = i->getInt(AreaDB::AreaID);
          int parent = i->getInt(AreaDB::Region);

          std::stringstream ss;
          ss << area << "-" << gAreaDB.getAreaName(area);
          QTreeWidgetItem* item = (_items.find(area) != _items.end()) ? _items.at(area) : new QTreeWidgetItem();
          item->setData(0, 1, QVariant(area));
          item->setText(0, QString(ss.str().c_str()));
          _items.emplace(area, item);

          if (parent)
          {
            QTreeWidgetItem* parent_item(nullptr);
            parent_item = (_items.find(parent) != _items.end()) ? _items.at(parent) : new QTreeWidgetItem();
            parent_item->addChild(item);
          }
          else
          {
            _area_tree->addTopLevelItem(item);
          }
        }
      }
    }
  }
}
