// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

enum eTerrainType
{
  eTerrainType_Flat,
  eTerrainType_Linear,
  eTerrainType_Smooth,
  eTerrainType_Polynom,
  eTerrainType_Trigo,
  eTerrainType_Quadra,
  eTerrainType_Gaussian,
  eTerrainType_Vertex,
  eTerrainType_Count,
};

enum eVertexMode
{
  eVertexMode_Mouse,
  eVertexMode_Center,
  eVertexMode_Count
};

enum eTabletControl
{
  eTabletControl_Off,
  eTabletControl_On
};

enum eTerrainTabletActiveGroup
{
  eTerrainTabletActiveGroup_Radius,
  eTerrainTabletActiveGroup_Speed,
};

enum eFlattenType
{
  eFlattenType_Flat,
  eFlattenType_Linear,
  eFlattenType_Smooth,
  eFlattenType_Origin,
  eFlattenType_Count,
};

struct flatten_mode
{
  flatten_mode(bool a, bool b) : raise(a), lower(b) {}

  bool raise : 1;
  bool lower : 1;

  flatten_mode next()
  {
    lower = lower == raise;
    raise = !raise;

    return *this;
  }
};

enum class editing_mode
{
  ground,
  flatten_blur,
  paint,
  holes,
  areaid,
  flags,
  water,
  mccv,
  object,
#ifdef NOGGIT_HAS_SCRIPTING
  scripting
#endif
};

enum water_opacity
{
  river_opacity,
  ocean_opacity,
  custom_opacity,
};

enum class cursor_mode : unsigned int
{
  none,
  disk,
  sphere,
  unused, // left it there to avoid issues
  terrain,
  mode_count
};

enum display_mode
{
  in_2D,
  in_3D
};
