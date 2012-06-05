// WorldController.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#ifndef WORLDCONTROLLER_H
#define WORLDCONTROLLER_H

#include <QObject>
#include <QString>

class World;
class MapView;

class WorldController : public QObject
{
    Q_OBJECT
public:

  enum terrain_editing_modes
  {
    shaping = 0,
    smoothing = 1,
    texturing = 2,
    hole_setting = 3,
    area_id_setting = 4,
    impassable_flag_setting = 5,
  };

  WorldController();

  void setModel(World *world);
  void addView(MapView *view);

private:
  QList< MapView* > _views;
  World *_model;
  terrain_editing_modes _current_terrain_editing_mode;
  terrain_editing_modes _terrain_editing_mode_before_2d;

public slots:
  void toggle_flag(bool);
  void set_terrain_editing_mode (int);

};

#endif
