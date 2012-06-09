// WorldController.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#include <noggit/WorldController.h>

#include <noggit/World.h>
#include <noggit/MapView.h>

WorldController::WorldController() : QObject()
  , _model(NULL)
{
}

void WorldController::addView(MapView *view)
{
  if(view)
  {
    _views.append(view);
  }
}

void WorldController::setModel(World *world)
{
  if(world)
  {
      _model = world;
  }
}

