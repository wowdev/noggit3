// WorldController.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#ifndef WORLD_H
#define WORLD_H

#include <QObject>
#include <QString>

class World;
class MapView;

class WorldController : public QObject
{
    Q_OBJECT

public:
  WorldController();

  void setModel(World *world);
  void addView(MapView *view);

private:
  QList< MapView* > _views;
  World *_model;

};


#endif
