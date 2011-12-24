#include <noggit/Menu.h>

#include <QHBoxLayout>
#include <QMetaType>
#include <QListWidget>
#include <QSettings>
#include <QTabWidget>

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapView.h>
#include <noggit/Vec3D.h>
#include <noggit/World.h>
#include <noggit/ui/minimap_widget.h>

struct bookmark_entry
{
  int map_id;
  Vec3D position;
  float rotation;
  float tilt;
};

// for storing in QVariant
Q_DECLARE_METATYPE (bookmark_entry);

Menu::Menu (QWidget* parent)
  : QWidget (parent)
  , _minimap (NULL)
  , _world (NULL)
{
  QListWidget* continents_table (new QListWidget (NULL));
  QListWidget* dungeons_table (new QListWidget (NULL));
  QListWidget* raids_table (new QListWidget (NULL));

  connect (continents_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_map_list_item (QListWidgetItem*)));
  connect (dungeons_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_map_list_item (QListWidgetItem*)));
  connect (raids_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_map_list_item (QListWidgetItem*)));

  QListWidget* tables[3] = { continents_table, dungeons_table, raids_table };

  for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i )
  {
    const int mapID (i->getInt (MapDB::MapID));
    const int areaType (i->getUInt (MapDB::AreaType));
    if (areaType < 0 || areaType > 2 || !World::IsEditableWorld (mapID))
      continue;

    QListWidgetItem* item (new QListWidgetItem (QString::fromUtf8 (i->getLocalizedString(MapDB::Name)), tables[areaType]));
    item->setData (Qt::UserRole, QVariant (mapID));
  }

  QListWidget* bookmarks_table (new QListWidget (NULL));

  connect (bookmarks_table, SIGNAL (itemClicked (QListWidgetItem*)), SLOT (show_bookmark_list_item (QListWidgetItem*)));
  connect (bookmarks_table, SIGNAL (itemDoubleClicked (QListWidgetItem*)), SLOT (open_bookmark_list_item (QListWidgetItem*)));

  //! \todo The list needs to be refreshed upon adding a bookmark.
  QSettings settings;

  int bookmarks (settings.beginReadArray ("bookmarks"));
  for (int i (0); i < bookmarks; ++i)
  {
    settings.setArrayIndex (i);
    bookmark_entry b;
    b.map_id = settings.value ("map_id").toInt();
    b.position.x = settings.value ("camera/position/x").toFloat();
    b.position.y = settings.value ("camera/position/y").toFloat();
    b.position.z = settings.value ("camera/position/z").toFloat();
    b.rotation = settings.value ("camera/rotation").toFloat();
    b.tilt = settings.value ("camera/tilt").toFloat();

    const int area_id (settings.value ("area_id").toInt());

    QListWidgetItem* item (new QListWidgetItem (QString ("%1: %2").arg (MapDB::getMapName (b.map_id)).arg (AreaDB::getAreaName (area_id)), bookmarks_table));
    item->setData (Qt::UserRole, QVariant::fromValue (b));
  }
  settings.endArray();

  QHBoxLayout* menu_layout (new QHBoxLayout (this));

  QTabWidget* entry_points_tabs (new QTabWidget (NULL));
  entry_points_tabs->addTab (continents_table, tr ("Continents"));
  entry_points_tabs->addTab (dungeons_table, tr ("Dungeons"));
  entry_points_tabs->addTab (raids_table, tr ("Raids"));
  entry_points_tabs->addTab (bookmarks_table, tr ("Bookmarks"));

  _minimap = new ui::minimap_widget (NULL);
  _minimap->draw_boundaries (true);
  connect (_minimap, SIGNAL (map_clicked (Vec3D)), SLOT (minimap_clicked (Vec3D)));

  menu_layout->addWidget (entry_points_tabs);
  menu_layout->addWidget (_minimap);
}

Menu::~Menu()
{
  delete _world;
  _world = NULL;
}

void Menu::enter_world_at (const Vec3D& pos, bool autoHeight, float av, float ah )
{
  prepare_world (pos, ah, av, autoHeight);
  _world->enterTile (pos.x / TILESIZE, pos.y / TILESIZE);

  emit create_world_view_request (_world);
}

void Menu::load_map (int map_id)
{
  delete _world;

  _world = new World (gMapDB.getByID (map_id, MapDB::MapID).getString (MapDB::InternalName));

  _minimap->world (_world);
}

void Menu::minimap_clicked (const Vec3D& position)
{
  enter_world_at (position, true, 0.0, 0.0);
}

void Menu::prepare_world (const Vec3D& pos, float rotation, float tilt, bool auto_height)
{
  _world->autoheight = auto_height;
  _world->camera = Vec3D (pos.x, pos.y, pos.z);
  //! \todo actually set lookat!
  _world->lookat = Vec3D (pos.x + 10.0f, pos.y + 10.0f, pos.z + 10.0f); // ah = rotation

  _world->initDisplay();
}

void Menu::show_map_list_item (QListWidgetItem* item)
{
  load_map (item->data (Qt::UserRole).toInt());
  _minimap->draw_camera (false);
}

void Menu::show_bookmark_list_item (QListWidgetItem* item)
{
  const bookmark_entry e (item->data (Qt::UserRole).value<bookmark_entry>());
  load_map (e.map_id);
  prepare_world (e.position, e.rotation, e.tilt, false);
  _minimap->draw_camera (true);
}

void Menu::open_bookmark_list_item (QListWidgetItem* item)
{
  const bookmark_entry e (item->data (Qt::UserRole).value<bookmark_entry>());
  load_map (e.map_id);
  enter_world_at (e.position, false, e.tilt, e.rotation);
}
