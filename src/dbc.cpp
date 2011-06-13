#include "DBC.h"

#include <string>

#include "Misc.h"
#include "Log.h"

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



std::string AreaDB::getAreaName( int pAreaID )
{
  if( !pAreaID )
    return "Unknown location";
  unsigned int regionID = 0;
  std::string areaName = "";
  try 
  {
    AreaDB::Record rec = gAreaDB.getByID( pAreaID );
    areaName = rec.getLocalizedString( AreaDB::Name );
    regionID = rec.getUInt( AreaDB::Region );
  } 
  catch(AreaDB::NotFound)
  {
    areaName = "Unknown location";
  }
  if (regionID != 0) 
  {
    try 
    {
      AreaDB::Record rec = gAreaDB.getByID( regionID );
      areaName = std::string(rec.getLocalizedString( AreaDB::Name )) + std::string(": ") + areaName;
    } 
    catch(AreaDB::NotFound)
    {
      areaName = "Unknown location";
    }
  }
  
  areaName = misc::replaceSpecialChars( areaName );

  return areaName;
}

const char * getGroundEffectDoodad( unsigned int effectID, int DoodadNum )
{
  try 
  {
    unsigned int doodadId = gGroundEffectTextureDB.getByID( effectID ).getUInt( GroundEffectTextureDB::Doodads + DoodadNum );
    return gGroundEffectDoodadDB.getByID( doodadId ).getString( GroundEffectDoodadDB::Filename );
  }
  catch( DBCFile::NotFound )
  {
    LogError << "Tried to get a not existing row in GroundEffectTextureDB or GroundEffectDoodadDB ( effectID = " << effectID << ", DoodadNum = " << DoodadNum << " )!" << std::endl;
    return 0;
  }
}

int LiquidTypeDB::getLiquidType( int pID )
{
  int type=0;
  try 
  {
    LiquidTypeDB::Record rec = gLiquidTypeDB.getByID( pID );
    type = rec.getUInt(LiquidTypeDB::Type);
  } 
  catch(LiquidTypeDB::NotFound)
  {
    type = 0;
  }
  return type;
}
