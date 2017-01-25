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
  eTerrainType_Count,
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
  eFlattenType_Count
};

enum eFlattenMode
{
  eFlattenMode_Both,
  eFlattenMode_Raise,
  eFlattenMode_Lower,
  eFlattenMode_Count
};