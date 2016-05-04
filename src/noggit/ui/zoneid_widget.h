#ifndef ZONEID_WIDGET_H
#define ZONEID_WIDGET_H

#include <QWidget>
#include <QTreeWidget>
#include <QString>

class World;
namespace noggit
{
	namespace ui
	{
		class zoneid_widget : public QTreeWidget
		{
			Q_OBJECT

		public:
			zoneid_widget(World *world, QWidget* Parent = NULL );
			typedef zoneid_widget* Ptr;

			void setMapID();
			void setZoneID(int id);

		private:
			World* _world;

			QString ZoneName;
			QString SubZoneName;
			QString MapName;

			unsigned int ZoneID;
			int SubZoneID;
			int MapID;
			int selectedArea;

			QTreeWidgetItem* item_1;

			void AreaList();
		};
	}
}
#endif
