// Menu.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Beket <snipbeket@mail.ru>
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Glararan <glararan@glararan.eu>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/Menu.h>

#include <QHBoxLayout>
#include <QGridLayout>
#include <QMetaType>
#include <QListWidget>
#include <QPushButton>
#include <QSettings>
#include <QTabWidget>

#include <math/vector_3d.h>

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapView.h>
#include <noggit/World.h>
#include <noggit/ui/minimap_widget.h>
#include <noggit/ui/settingsDialog.h>

struct bookmark_entry
{
  int map_id;
  ::math::vector_3d position;
  float rotation;
  float tilt;
};

// for storing in QVariant
Q_DECLARE_METATYPE (bookmark_entry);

Menu::Menu (QWidget* parent)
  : QWidget (parent)
  , _minimap (new noggit::ui::minimap_widget (NULL))
  //, _world (NULL)
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
    b.position.x (settings.value ("camera/position/x").toFloat());
    b.position.y (settings.value ("camera/position/y").toFloat());
    b.position.z (settings.value ("camera/position/z").toFloat());
    b.rotation = settings.value ("camera/rotation").toFloat();
    b.tilt = settings.value ("camera/tilt").toFloat();

    const int area_id (settings.value ("area_id").toInt());

    QListWidgetItem* item (new QListWidgetItem (QString ("%1: %2").arg (MapDB::getMapName (b.map_id)).arg (AreaDB::getAreaName (area_id)), bookmarks_table));
    item->setData (Qt::UserRole, QVariant::fromValue (b));
  }
  settings.endArray();

  QGridLayout* menu_layout (new QGridLayout (this));

  QTabWidget* entry_points_tabs (new QTabWidget (NULL));
  entry_points_tabs->addTab (continents_table, tr ("Continents"));
  entry_points_tabs->addTab (dungeons_table, tr ("Dungeons"));
  entry_points_tabs->addTab (raids_table, tr ("Raids"));
  entry_points_tabs->addTab (bookmarks_table, tr ("Bookmarks"));

  _minimap->draw_boundaries (true);
  connect (_minimap, SIGNAL (map_clicked (const World *, const ::math::vector_3d&)), SLOT (minimap_clicked (const World *, const ::math::vector_3d&)));

  menu_layout->addWidget (entry_points_tabs, 0, 0);
  menu_layout->addWidget (_minimap, 0, 1);
}

Menu::~Menu()
{
  //delete _world;
  //_world = NULL;
}

void Menu::enter_world_at (World *world, const ::math::vector_3d& pos, bool auto_height, float av, float ah )
{
  prepare_world (world,pos, ah, av);
  world->enterTile (pos.x() / TILESIZE, pos.y() / TILESIZE);

  emit create_world_view_request (world);

  if(auto_height)
    world->set_camera_above_terrain();
}

World *Menu::load_map (unsigned int map_id)
{
  if (_minimap->world() && _minimap->world()->getMapID() == map_id)
  {
    return const_cast<World *>(_minimap->world());
  }

  return new World (gMapDB.getByID (map_id, MapDB::MapID).getString (MapDB::InternalName));
}

void Menu::minimap_clicked (const World *world, const ::math::vector_3d& position)
{
  enter_world_at (const_cast<World *>(world), position, true, 0.0, 0.0);
}

void Menu::prepare_world (World *world, const ::math::vector_3d& pos, float rotation, float tilt)
{
  world->camera = ::math::vector_3d (pos.x(), pos.y(), pos.z());
  //! \todo actually set lookat!
  world->lookat = ::math::vector_3d (pos.x() + 10.0f, pos.y() + 10.0f, pos.z() + 10.0f); // ah = rotation

  world->initDisplay();
}

void Menu::show_map_list_item (QListWidgetItem* item)
{
  _minimap->draw_camera (false);
  _minimap->world(load_map (item->data (Qt::UserRole).toInt()));
}

void Menu::show_bookmark_list_item (QListWidgetItem* item)
{
  const bookmark_entry e (item->data (Qt::UserRole).value<bookmark_entry>());
  World *world = load_map (e.map_id);
  _minimap->world(world);
  prepare_world (world,e.position, e.rotation, e.tilt);
  _minimap->draw_camera (true);
}

void Menu::open_bookmark_list_item (QListWidgetItem* item)
{
  const bookmark_entry e (item->data (Qt::UserRole).value<bookmark_entry>());
  enter_world_at (load_map (e.map_id),e.position, false, e.tilt, e.rotation);
}
