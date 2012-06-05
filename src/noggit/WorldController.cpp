// WorldController.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Benedikt Kleiner <benedikt.kleiner@googlemail.com>

#include <noggit/WorldController.h>

#include <noggit/World.h>
#include <noggit/MapView.h>

WorldController::WorldController() : QObject(NULL)
  , model(NULL)
  , views(NULL)
  , _current_terrain_editing_mode (shaping)
  , _terrain_editing_mode_before_2d (_current_terrain_editing_mode)
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

void WorldController::toggle_flag (bool value)
{
  QAction *action = (QAction *)sender();
  if(value)
  {
      _model->renderflags |= action->data().toInt();
  }
  else
  {
      _model->renderflags &= ~action->data().toInt();
  }
}

void WorldController::set_terrain_editing_mode (int mode)
{
  _current_terrain_editing_mode = (terrain_editing_modes)mode;

  _model->renderflags &= ~HOLELINES;
  _model->renderflags &= ~NOCURSOR;
  _model->renderflags &= ~AREAID;
  _model->renderflags &= ~MARKIMPASSABLE;


  //! todo care about these widgets
  //_shaping_settings_widget->hide();
  //_smoothing_settings_widget->hide();
  //_texturing_settings_widget->hide();

  switch (mode)
  {
  case shaping:
    //_shaping_settings_widget->show();
    break;

  case smoothing:
    //_smoothing_settings_widget->show();
    break;

  case texturing:
    //_texturing_settings_widget->show();
    break;

  case hole_setting:
    _model->renderflags |= HOLELINES | NOCURSOR;
    break;

  case area_id_setting:
    _model->renderflags |= AREAID;
    break;

  case impassable_flag_setting:
    _model->renderflags |= MARKIMPASSABLE;
    break;

  default:
    break;
  }
