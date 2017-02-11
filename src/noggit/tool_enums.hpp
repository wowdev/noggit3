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
  eTerrainType_Vertex,
  eTerrainType_Gaussian,
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
  eFlattenType_Count,
  eFlattenType_Origin
};

enum eFlattenMode
{
  eFlattenMode_Both,
  eFlattenMode_Raise,
  eFlattenMode_Lower,
  eFlattenMode_Count
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
  light,
  mccv,
  object,
};

enum water_opacity
{
  river_opacity,
  ocean_opacity,
  custom_opacity,
};
