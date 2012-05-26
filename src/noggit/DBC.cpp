// DBC.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Beket <snipbeket@mail.ru>
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>

#include <noggit/DBC.h>

#include <QObject> // QObject::tr

#include <noggit/Log.h>

AreaDB gAreaDB;
MapDB gMapDB;
LoadingScreensDB gLoadingScreensDB;
LightDB gLightDB;
LightParamsDB gLightParamsDB;
LightSkyboxDB gLightSkyboxDB;
LightIntBandDB gLightIntBandDB;
LightFloatBandDB gLightFloatBandDB;
GroundEffectDoodadDB gGroundEffectDoodadDB;
GroundEffectTextureDB gGroundEffectTextureDB;
LiquidTypeDB gLiquidTypeDB;

void OpenDBs()
{
  gAreaDB.open();
  gMapDB.open();
  gLoadingScreensDB.open();
  gLightDB.open();
  gLightParamsDB.open();
  gLightSkyboxDB.open();
  gLightIntBandDB.open();
  gLightFloatBandDB.open();
  gGroundEffectDoodadDB.open();
  gGroundEffectTextureDB.open();
  gLiquidTypeDB.open();
}


QString AreaDB::getAreaName(int pAreaID)
{
  if(pAreaID <= 0)
    return QObject::tr ("Unknown location");

  unsigned int regionID (0);
  QString areaName;
  try
  {
    AreaDB::Record rec (gAreaDB.getByID (pAreaID));
    areaName = QString::fromUtf8 (rec.getLocalizedString (AreaDB::Name));
    regionID = rec.getUInt (AreaDB::Region);
  }
  catch(AreaDB::NotFound)
  {
    areaName = QObject::tr ("Unknown location");
  }

  if(regionID != 0)
  {
    try
    {
      areaName = QString::fromUtf8 (gAreaDB.getByID (regionID).getLocalizedString (AreaDB::Name)) + ": " + areaName;
    }
    catch(AreaDB::NotFound)
    {
      areaName = QObject::tr ("Unknown location");
    }
  }

  return areaName;
}

QString MapDB::getMapName(int pMapID)
{
  if(pMapID <= 0)
    return QObject::tr ("Unknown map");

  QString mapName;
  try
  {
    mapName = QString::fromUtf8 (gMapDB.getByID (pMapID).getLocalizedString (MapDB::Name));
  }
  catch(MapDB::NotFound)
  {
    mapName = QObject::tr ("Unknown map");
  }

  return mapName;
}

const char* getGroundEffectDoodad(unsigned int effectID, int DoodadNum)
{
  try
  {
    unsigned int doodadId = gGroundEffectTextureDB.getByID (effectID).getUInt (GroundEffectTextureDB::Doodads + DoodadNum);
    return gGroundEffectDoodadDB.getByID (doodadId).getString (GroundEffectDoodadDB::Filename);
  }
  catch(DBCFile::NotFound)
  {
    LogError << "Tried to get a not existing row in GroundEffectTextureDB or GroundEffectDoodadDB ( effectID = " << effectID << ", DoodadNum = " << DoodadNum << " )!" << std::endl;
    return 0;
  }
}

int LiquidTypeDB::getLiquidType(int pID)
{
  int type = 0;
  try
  {
    LiquidTypeDB::Record rec = gLiquidTypeDB.getByID (pID);
    type = rec.getUInt(LiquidTypeDB::Type);
  }
  catch(LiquidTypeDB::NotFound)
  {
    type = 0;
  }

  return type;
}
