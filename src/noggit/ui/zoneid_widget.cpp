#include <noggit/ui/zoneid_widget.h>

#include <QVBoxLayout>
#include <QTreeWidgetItemIterator>
#include <QTreeView>
#include <QList>
#include <QDataStream>
#include <QLabel>

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/World.h>
namespace noggit
{
	namespace ui
	{
		zoneid_widget::zoneid_widget(World *world,QWidget* Parent)
			: QTreeWidget(Parent)
			, MapName("")
			, ZoneName("")
			, SubZoneName("")
			, MapID(-1)
			, ZoneID(-1)
			, SubZoneID(-1)
			, selectedArea(-1)
			, _world(world)
		{
			int width = 640;
			int height = 480;

			setWindowTitle(tr("AreaID Browser"));
			resize(width, height);

			setColumnCount(2);
			setColumnWidth(0, width / 2);
			setColumnWidth(1, width / 2);
			setHeaderLabels(QStringList() << "   Zone Name" << "Sub-Zone(s)");

			QDataStream AreaPath;
			if (SubZoneName != "")
				AreaPath << MapName << " -> " << SubZoneName;
			else
				AreaPath << MapName << " -> " << ZoneName;

			QString areapath;
			AreaPath >> areapath;

			QTreeWidgetItem* current_area(new QTreeWidgetItem(this));
			current_area->setText(0, "Current Area: " + areapath);
			current_area->setTextColor(0, Qt::green);

			setMapID();
			setZoneID(_world->getAreaID());

			/*  FOR TEST
				QTreeWidgetItem* item_1 (new QTreeWidgetItem (this));
				item_1->setText(0, "Zone Name");

				QTreeWidgetItem* sub_item_1 (new QTreeWidgetItem (item_1));
				sub_item_1->setText(1, "Sub-Zones");

				QObject::connect(this, SIGNAL(clicked()), this, SLOT(processClick()));*/
		}

		void zoneid_widget::setMapID()
		{
			MapID = int(_world->getMapID());
			ZoneID = 0;
			SubZoneID = 0;

			for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
			{
				if (i->getInt(MapDB::MapID) == int(_world->getMapID()))
					MapName = i->getString(MapDB::InternalName);
			}

			AreaList();
		}

		void zoneid_widget::setZoneID(int id)
		{
			for (DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i)
			{
				if (i->getInt(AreaDB::AreaID) == id)
				{
					if (i->getUInt(AreaDB::Region) == 0)
					{
						ZoneName = gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
						ZoneID = id;

						SubZoneID = 0;
						SubZoneName = "";
					}
					else
					{
						SubZoneName = gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
						SubZoneID = id;
					}
				}
			}

			AreaList();
		}

		void zoneid_widget::AreaList()
		{
			for (DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.begin(); ++i)
			{
				if (i->getInt(AreaDB::Continent) == MapID)
				{
					if (ZoneID == 0)
					{
						if (i->getUInt(AreaDB::Region) == 0)
						{
							QDataStream qds;
							qds << i->getInt(AreaDB::AreaID) << " - " << gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));

							QString text;
							qds >> text;

							QTreeWidgetItem* item_1(new QTreeWidgetItem(this));
							item_1->setText(0, text);

							QObject::connect(this, SIGNAL(clicked()), this, SLOT(setZoneID(i->getInt(AreaDB::AreaID))));
						}
					}
					else if (ZoneID > 0)
					{
						if (i->getUInt(AreaDB::Region) == ZoneID)
						{
							QDataStream qds;
							qds << i->getInt(AreaDB::AreaID) << " - " << gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));

							QString text;
							qds >> text;

							QTreeWidgetItem* sub_item_1(new QTreeWidgetItem(item_1));
							sub_item_1->setText(1, text);

							QObject::connect(this, SIGNAL(itemClicked(sub_item_1, 1)), this, SLOT(setZoneID(i->getInt(AreaDB::AreaID))));
						}
					}
				}
			}
		}
	}
}