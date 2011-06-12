#ifndef AREADB_H
#define AREADB_H

#include <string>

#include "dbcfile.h"

class AreaDB: public DBCFile
{
public:
  AreaDB():
    DBCFile("DBFilesClient\\AreaTable.dbc")
  { }

  /// Fields
  static const size_t AreaID = 0;    // uint
  static const size_t Continent = 1;  // uint
  static const size_t Region = 2;    // uint [AreaID]
  static const size_t Flags = 4;    // bit field
  static const size_t Name = 11;    // localisation string

  static std::string getAreaName( int pAreaID );
};

class MapDB: public DBCFile
{
public:
  MapDB():
    DBCFile("DBFilesClient\\Map.dbc")
  { }

  /// Fields
  static const size_t MapID = 0;        // uint
  static const size_t InternalName = 1;    // string
  static const size_t AreaType = 2;      // uint
  static const size_t IsBattleground = 3;    // uint
  static const size_t Name = 4;        // loc

  static const size_t LoadingScreen = 57;    // uint [LoadingScreen]
};

class LoadingScreensDB: public DBCFile
{
public:
  LoadingScreensDB():
    DBCFile("DBFilesClient\\LoadingScreens.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Name = 1;      // string
  static const size_t Path = 2;      // string
};

class LightDB: public DBCFile
{
public:
  LightDB():
    DBCFile("DBFilesClient\\Light.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Map = 1;      // uint
  static const size_t PositionX = 2;    // float
  static const size_t PositionY = 3;    // float
  static const size_t PositionZ = 4;    // float
  static const size_t RadiusInner = 5;  // float
  static const size_t RadiusOuter = 6;  // float
  static const size_t DataIDs = 7;    // uint[8]
};

class LightParamsDB : public DBCFile{
public:
  LightParamsDB():
    DBCFile("DBFilesClient\\LightParams.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t skybox = 2;      // uint ref to LightSkyBox
};

class LightSkyboxDB: public DBCFile
{
public:
  LightSkyboxDB():
    DBCFile("DBFilesClient\\LightSkybox.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t filename = 1;    // string
  static const size_t flags = 2;      // uint
};

class LightIntBandDB: public DBCFile
{
public:
  LightIntBandDB():
    DBCFile("DBFilesClient\\LightIntBand.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Entries = 1;    // uint
  static const size_t Times = 2;      // uint
  static const size_t Values = 18;    // uint
};

class LightFloatBandDB: public DBCFile
{
public:
  LightFloatBandDB():
    DBCFile("DBFilesClient\\LightFloatBand.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Entries = 1;    // uint
  static const size_t Times = 2;      // uint
  static const size_t Values = 18;    // float
};

class GroundEffectTextureDB: public DBCFile
{
public:
  GroundEffectTextureDB():
    DBCFile("DBFilesClient\\GroundEffectTexture.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Doodads = 1;    // uint[4]
  static const size_t Weights = 5;    // uint[4]
  static const size_t Amount = 9;      // uint
  static const size_t TerrainType = 10;  // uint
};

class GroundEffectDoodadDB: public DBCFile
{
public:
  GroundEffectDoodadDB():
    DBCFile("DBFilesClient\\GroundEffectDoodad.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t InternalID = 1;    // uint
  static const size_t Filename = 2;    // string
};

class LiquidTypeDB: public DBCFile
{
public:
  LiquidTypeDB():
    DBCFile("DBFilesClient\\LiquidType.dbc")
  { }

  /// Fields
  static const size_t ID = 0;        // uint
  static const size_t Name = 1;      // string
  static const size_t Type = 3;      // uint
  static const size_t ShaderType = 15;  // uint
  static const size_t TextureFilenames = 16;    // string[8]

  static int getLiquidType( int pID );
};

void OpenDBs();

const char * getGroundEffectDoodad( unsigned int effectID, int DoodadNum );

extern AreaDB gAreaDB;
extern MapDB gMapDB;
extern LoadingScreensDB gLoadingScreensDB;
extern LightDB gLightDB;
extern LightParamsDB gLightParamsDB;
extern LightSkyboxDB gLightSkyboxDB;
extern LightIntBandDB gLightIntBandDB;
extern LightFloatBandDB gLightFloatBandDB;
extern GroundEffectDoodadDB gGroundEffectDoodadDB;
extern GroundEffectTextureDB gGroundEffectTextureDB;
extern LiquidTypeDB gLiquidTypeDB;

#endif
