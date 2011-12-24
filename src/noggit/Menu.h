#ifndef __MENU_H
#define __MENU_H

#include <QWidget>

class QKeyEvent;
class QMouseEvent;
class QListWidgetItem;

namespace ui
{
  class minimap_widget;
}
class Vec3D;
class World;

class Menu : public QWidget
{
Q_OBJECT

public:
  Menu (QWidget* parent = NULL);
  virtual ~Menu();

private slots:
  void show_map_list_item (QListWidgetItem* item);
  void show_bookmark_list_item (QListWidgetItem* item);
  void open_bookmark_list_item (QListWidgetItem* item);

  void minimap_clicked (const Vec3D&);

signals:
  void create_world_view_request (World*);

private:
  void load_map (int mapID);
  void prepare_world (const Vec3D& pos, float rotation, float tilt, bool auto_height);
  void enter_world_at (const Vec3D& pos, bool autoHeight = true, float av = -30.0f, float ah = -90.0f);

  ui::minimap_widget* _minimap;
  World* _world;
};

#endif
