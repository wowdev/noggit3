#include "MapTile.h"

#include <algorithm>
#include <cassert>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "Environment.h"
#include "TileWater.h"
#include "Log.h"
#include "MapChunk.h"
#include "Misc.h"
#include "ModelInstance.h" // ModelInstance
#include "ModelManager.h" // ModelManager
#include "WMOInstance.h" // WMOInstance
#include "World.h"
#include "Alphamap.h"
#include "TextureSet.h"
#include "MapIndex.h"

int indexMapBuf(int x, int y)
{
  return ((y+1)/2)*9 + (y/2)*8 + x;
}

MapTile::MapTile( int pX, int pZ, const std::string& pFilename, bool pBigAlpha )
{
  this->modelCount = 0;
  this->mPositionX = pX;
  this->mPositionZ = pZ;

  this->changed = 0;
  this->xbase = mPositionX * TILESIZE;
  this->zbase = mPositionZ * TILESIZE;

  this->mBigAlpha = pBigAlpha;

  for( int i = 0; i < 16; ++i )
  {
    for( int j = 0; j < 16; j++ )
    {
      mChunks[i][j] = NULL;
    }
  }

  mFilename = pFilename;

  MPQFile theFile( mFilename );

  Log << "Opening tile " << mPositionX << ", " << mPositionZ << " (\"" << mFilename << "\") from " << (theFile.isExternal() ? "disk" : "MPQ") << "." << std::endl;

  // - Parsing the file itself. --------------------------

  // We store this data to load it at the end.
  uint32_t lMCNKOffsets[256];
  std::vector<ENTRY_MDDF> lModelInstances;
  std::vector<ENTRY_MODF> lWMOInstances;

  uint32_t fourcc;
  uint32_t size;

  MHDR Header;

  // - MVER ----------------------------------------------

  uint32_t version;

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );
  theFile.read( &version, 4 );

  assert( fourcc == 'MVER' && version == 18 );

  // - MHDR ----------------------------------------------

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );

  assert( fourcc == 'MHDR' );

  theFile.read( &Header, sizeof( MHDR ) );

  mFlags = Header.flags;

  // - MCIN ----------------------------------------------

  theFile.seek( Header.mcin + 0x14 );
  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );

  assert( fourcc == 'MCIN' );

  for( int i = 0; i < 256; ++i )
  {
    theFile.read( &lMCNKOffsets[i], 4 );
    theFile.seekRelative( 0xC );
  }

  // - MTEX ----------------------------------------------

  theFile.seek( Header.mtex + 0x14 );
  theFile.read( &fourcc, 4 );
  theFile.read( &size, 4 );

  assert( fourcc == 'MTEX' );

  {
    char* lCurPos = reinterpret_cast<char*>( theFile.getPointer() );
    char* lEnd = lCurPos + size;

    while( lCurPos < lEnd )
    {
      mTextureFilenames.push_back( std::string( lCurPos ) );
      lCurPos += strlen( lCurPos ) + 1;
    }
  }

  // - MMDX ----------------------------------------------

  theFile.seek( Header.mmdx + 0x14 );
  theFile.read( &fourcc, 4 );
  theFile.read( &size, 4 );

  assert( fourcc == 'MMDX' );

  {
    char* lCurPos = reinterpret_cast<char*>( theFile.getPointer() );
    char* lEnd = lCurPos + size;

    while( lCurPos < lEnd )
    {
      mModelFilenames.push_back( std::string( lCurPos ) );
      lCurPos += strlen( lCurPos ) + 1;
    }
  }

  // - MWMO ----------------------------------------------

  theFile.seek( Header.mwmo + 0x14 );
  theFile.read( &fourcc, 4 );
  theFile.read( &size, 4 );

  assert( fourcc == 'MWMO' );

  {
    char* lCurPos = reinterpret_cast<char*>( theFile.getPointer() );
    char* lEnd = lCurPos + size;

    while( lCurPos < lEnd )
    {
      mWMOFilenames.push_back( std::string( lCurPos ) );
      lCurPos += strlen( lCurPos ) + 1;
    }
  }

  // - MDDF ----------------------------------------------

  theFile.seek( Header.mddf + 0x14 );
  theFile.read( &fourcc, 4 );
  theFile.read( &size, 4 );

  assert( fourcc == 'MDDF' );

  ENTRY_MDDF* mddf_ptr = reinterpret_cast<ENTRY_MDDF*>( theFile.getPointer() );
  for( unsigned int i = 0; i < size / 36; ++i )
  {
    lModelInstances.push_back( mddf_ptr[i] );
  }

  // - MODF ----------------------------------------------

  theFile.seek( Header.modf + 0x14 );
  theFile.read( &fourcc, 4 );
  theFile.read( &size, 4 );

  assert( fourcc == 'MODF' );

  ENTRY_MODF* modf_ptr = reinterpret_cast<ENTRY_MODF*>( theFile.getPointer() );
  for( unsigned int i = 0; i < size / 64; ++i )
  {
    lWMOInstances.push_back( modf_ptr[i] );
  }

  // - MISC ----------------------------------------------

  //! \todo  Parse all chunks in the new style!

  // - MH2O ----------------------------------------------
  if(Header.mh2o != 0) {
    hasWater = true;
	theFile.seek( Header.mh2o + 0x14 );
    theFile.read( &fourcc, 4 );
    theFile.read( &size, 4 );

    mWaterSize = size;

    int ofsW = Header.mh2o + 0x14 + 0x8;
    assert( fourcc == 'MH2O' );
    
	Water = new TileWater(true); //has water
	Water->readFromFile(theFile, ofsW); //reading MH2O data at separated class...
   	Water->init(xbase,zbase);
  }else{
	hasWater = false;
	Water = new TileWater(false); //empty water tile
  }

  // - MFBO ----------------------------------------------

  if( mFlags & 1 )
  {
    theFile.seek( Header.mfbo + 0x14 );
    theFile.read( &fourcc, 4 );
    theFile.read( &size, 4 );

    assert( fourcc == 'MFBO' );

    int16_t mMaximum[9], mMinimum[9];
    theFile.read( mMaximum, sizeof( mMaximum ) );
    theFile.read( mMinimum, sizeof( mMinimum ) );

    static const float xPositions[] = { this->xbase, this->xbase + 266.0f, this->xbase + 533.0f };
    static const float yPositions[] = { this->zbase, this->zbase + 266.0f, this->zbase + 533.0f };

    for( int y = 0; y < 3; y++ )
    {
      for( int x = 0; x < 3; x++ )
      {
        int pos = x + y * 3;
        mMinimumValues[pos * 3 + 0] = xPositions[x];
        mMinimumValues[pos * 3 + 1] = mMinimum[pos];
        mMinimumValues[pos * 3 + 2] = yPositions[y];

        mMaximumValues[pos * 3 + 0] = xPositions[x];
        mMaximumValues[pos * 3 + 1] = mMaximum[pos];
        mMaximumValues[pos * 3 + 2] = yPositions[y];
      }
    }
  }

  // - MTFX ----------------------------------------------
  /*
  //! \todo Implement this or just use Terrain Cube maps?
  Log << "MTFX offs: " << Header.mtfx << std::endl;
  if(Header.mtfx != 0){
    Log << "Try to load MTFX" << std::endl;
    theFile.seek( Header.mtfx + 0x14 );

    theFile.read( &fourcc, 4 );
    theFile.read( &size, 4 );

    assert( fourcc == 'MTFX' );


    {
      char* lCurPos = reinterpret_cast<char*>( theFile.getPointer() );
      char* lEnd = lCurPos + size;
      int tCount = 0;
      while( lCurPos < lEnd ) {
        int temp = 0;
        theFile.read(&temp, 4);
        Log << "Adding to " << mTextureFilenames[tCount].first << " texture effect: " << temp << std::endl;
        mTextureFilenames[tCount++].second = temp;
        lCurPos += 4;
      }
    }

  }*/

  // - Done. ---------------------------------------------

  // - Load textures -------------------------------------

  //! \note We no longer pre load textures but the chunks themselves do.

  // - Load WMOs -----------------------------------------

  for( std::vector<ENTRY_MODF>::iterator it = lWMOInstances.begin(); it != lWMOInstances.end(); ++it )
  {
    gWorld->mWMOInstances.insert( std::pair<int,WMOInstance>( it->uniqueID, WMOInstance( WMOManager::add( mWMOFilenames[it->nameID] ), &(*it) ) ) );
  }

  // - Load M2s ------------------------------------------

  for( std::vector<ENTRY_MDDF>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
  {
    gWorld->mModelInstances.insert( std::pair<int,ModelInstance>( it->uniqueID, ModelInstance( ModelManager::add( mModelFilenames[it->nameID] ), &(*it) ) ) );
  }

  // - Load chunks ---------------------------------------

  for( int nextChunk = 0; nextChunk < 256; ++nextChunk )
  {
    theFile.seek( lMCNKOffsets[nextChunk] );
    mChunks[nextChunk / 16][nextChunk % 16] = new MapChunk( this, &theFile, mBigAlpha );
  }

  theFile.close();

  // - Really done. --------------------------------------

  LogDebug << "Done loading tile " << mPositionX << "," << mPositionZ << "." << std::endl;
}

MapTile::~MapTile()
{
  LogDebug << "Unloading tile " << mPositionX << "," << mPositionZ << "." << std::endl;

  for( int j = 0; j < 16; ++j )
  {
    for( int i = 0; i < 16; ++i )
    {
      if( mChunks[j][i] )
      {
        delete mChunks[j][i];
        mChunks[j][i] = NULL;
      }
    }
  }

  mTextureFilenames.clear();

  for( std::vector<std::string>::iterator it = mWMOFilenames.begin(); it != mWMOFilenames.end(); ++it )
  {
    WMOManager::delbyname( *it );
  }
  mWMOFilenames.clear();

  for( std::vector<std::string>::iterator it = mModelFilenames.begin(); it != mModelFilenames.end(); ++it )
  {
    ModelManager::delbyname( *it );
  }
  mModelFilenames.clear();

  /*for( std::vector<Liquid*>::iterator it = mLiquids.begin(); it != mLiquids.end(); ++it )
  {
    if( *it )
    {
      delete *it;
      *it  = NULL;
    }
  }

  mLiquids.clear();*/
}


bool MapTile::isTile( int pX, int pZ )
{
  return pX == mPositionX && pZ == mPositionZ;
}

float MapTile::getMaxHeight()
{
  float maxHeight = -99999.0f;
  for( int nextChunk = 0; nextChunk < 256; ++nextChunk )
  {
    maxHeight = std::max( mChunks[nextChunk / 16][nextChunk % 16]->vmax.y, maxHeight );
  }
  return maxHeight;
}

extern float groundBrushRadius;
extern float blurBrushRadius;
extern int terrainMode;
extern Brush textureBrush;



void MapTile::draw()
{


  glColor4f(1,1,1,1);

  for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
      mChunks[j][i]->draw();

}

void MapTile::drawSelect()
{
  for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
      mChunks[j][i]->drawSelect();
}

void MapTile::drawLines()//draw red lines around the square of a chunk
{
  glDisable(GL_COLOR_MATERIAL);

  for (int j=0; j<16; ++j)
    for (int i=0; i<16; ++i)
      mChunks[j][i]->drawLines();

  glEnable(GL_COLOR_MATERIAL);
}

void MapTile::drawMFBO()
{
  static const GLshort lIndices[] = { 4, 1, 2, 5, 8, 7, 6, 3, 0, 1, 0, 3, 6, 7, 8, 5, 2, 1 };

  glColor4f(0,1,1,0.2f);
  glBegin(GL_TRIANGLE_FAN);
  for( int i = 0; i < 18; ++i )
  {
    glVertex3f( mMinimumValues[lIndices[i]*3 + 0], mMinimumValues[lIndices[i]*3 + 1], mMinimumValues[lIndices[i]*3 + 2]  );
  }
  glEnd();

  glColor4f(1,1,0,0.2f);
  glBegin(GL_TRIANGLE_FAN);
  for( int i = 0; i < 18; ++i )
  {
    glVertex3f( mMaximumValues[lIndices[i]*3 + 0], mMaximumValues[lIndices[i]*3 + 1], mMaximumValues[lIndices[i]*3 + 2]  );
  }
  glEnd();
}

void MapTile::drawWater()
{
  if(!hasWater) return; //no need to draw water on tile without water =)

  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_LIGHTING);

  Water->draw();

  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
}

void MapTile::addChunksLiquid(TileWater *lq)
{
  //chunksLiquids.push_back( lq );
}

bool MapTile::canWaterSave(){
  return !mFlags || mWaterSize>0;
}

void MapTile::getAlpha(size_t id, unsigned char *amap)
{
  int index = 0;
  int offsetIndex = 0;

  for(size_t j = 0; j < 1024; ++j)
  {
    index = (int)j/64;

    for(int i = 0; i < 16; ++i)
    {
      if(mChunks[index][i]->textureSet->num() > id+1)
      {
        memcpy(amap + j*1024 + i*64, mChunks[index][i]->textureSet->getAlpha(id) + offsetIndex*64, 64);
      }
      else
      {
        memset(amap + j*1024 + i*64, 1, 64);
      }
    }

    if(offsetIndex == 63)
      offsetIndex = 0;
    else
      offsetIndex++;
  }
}

// This is for the 2D mode only.
void MapTile::drawTextures()
{
  float xOffset,yOffset;

  glPushMatrix();
  yOffset=zbase/CHUNKSIZE;
  xOffset=xbase/CHUNKSIZE;
  glTranslatef(xOffset,yOffset,0);

  //glTranslatef(-8,-8,0);

  for (int j=0; j<16; ++j) {
    for (int i=0; i<16; ++i) {
      if(((i+1+xOffset)>gWorld->minX)&&((j+1+yOffset)>gWorld->minY)&&((i+xOffset)<gWorld->maxX)&&((j+yOffset)<gWorld->maxY))
        mChunks[j][i]->drawTextures();
    }
  }
  glPopMatrix();


}

MapChunk* MapTile::getChunk( unsigned int x, unsigned int z )
{
  if( x < 16 && z < 16 )
  {
    return mChunks[z][x];
  }
  else
  {
    return NULL;
  }
}

bool MapTile::GetVertex( float x, float z, Vec3D *V )
{
  int xcol = ( x - xbase ) / CHUNKSIZE;
  int ycol = ( z - zbase ) / CHUNKSIZE;

  return xcol >= 0 && xcol <= 15 && ycol >= 0 && ycol <= 15 && mChunks[ycol][xcol]->GetVertex( x, z, V );
}

/// --- Only saving related below this line. --------------------------

void MapTile::clearAllModels()
{
  Log << "Clear all models from ADT \"" << mFilename << "\"." << std::endl;

  int lID;  // This is a global counting variable. Do not store something in here you need later.

  // Collect some information we need later.

  // Check which doodads and WMOs are on this ADT.
  Vec3D lTileExtents[2];
  lTileExtents[0] = Vec3D( this->xbase, 0.0f, this->zbase );
  lTileExtents[1] = Vec3D( this->xbase + TILESIZE, 0.0f, this->zbase + TILESIZE );

  std::map<int, WMOInstance> lObjectInstances;
  std::map<int, ModelInstance> lModelInstances;

  for( std::map<int, WMOInstance>::iterator it = gWorld->mWMOInstances.begin(); it != gWorld->mWMOInstances.end(); ++it )
    if( checkInside( lTileExtents, it->second.extents ) )
      gWorld->deleteWMOInstance( it->second.mUniqueID );

  for( std::map<int, ModelInstance>::iterator it = gWorld->mModelInstances.begin(); it != gWorld->mModelInstances.end(); ++it )
  {
    Vec3D lModelExtentsV1[2], lModelExtentsV2[2];
    lModelExtentsV1[0] = it->second.model->header.BoundingBoxMin + it->second.pos;
    lModelExtentsV1[1] = it->second.model->header.BoundingBoxMax + it->second.pos;
    lModelExtentsV2[0] = it->second.model->header.VertexBoxMin + it->second.pos;
    lModelExtentsV2[1] = it->second.model->header.VertexBoxMax + it->second.pos;

    if( checkInside( lTileExtents, lModelExtentsV1 ) || checkInside( lTileExtents, lModelExtentsV2 ) )
    {
      gWorld->deleteModelInstance( it->second.d1 );
    }
  }

}

void MapTile::uidTile()
{
  // Check which doodads and WMOs are on this ADT.
  Vec3D lTileExtents[2];
  lTileExtents[0] = Vec3D( this->xbase, 0.0f, this->zbase );
  lTileExtents[1] = Vec3D( this->xbase + TILESIZE, 0.0f, this->zbase + TILESIZE );

  std::map<int, WMOInstance> lObjectInstances;
  std::map<int, ModelInstance> lModelInstances;

  unsigned int UID_counter = 1;
  for( std::map<int, WMOInstance>::iterator it = gWorld->mWMOInstances.begin(); it != gWorld->mWMOInstances.end(); ++it )
  {
    if( checkInside( lTileExtents, it->second.extents ) )
    {
      // If save mode == 1 and models origin is located on the adt then recalc UID
      if(this->changed==1 && checkOriginInside( lTileExtents, it->second.pos ))
      {
        it->second.mUniqueID =  mPositionX * 10000000 + mPositionZ * 100000 + UID_counter++;
      }
    }
  }

  for( std::map<int, ModelInstance>::iterator it = gWorld->mModelInstances.begin(); it != gWorld->mModelInstances.end(); ++it )
  {
    Vec3D lModelExtentsV1[2], lModelExtentsV2[2];
    lModelExtentsV1[0] = it->second.model->header.BoundingBoxMin + it->second.pos;
    lModelExtentsV1[1] = it->second.model->header.BoundingBoxMax + it->second.pos;
    lModelExtentsV2[0] = it->second.model->header.VertexBoxMin + it->second.pos;
    lModelExtentsV2[1] = it->second.model->header.VertexBoxMax + it->second.pos;

    if( checkInside( lTileExtents, lModelExtentsV1 ) || checkInside( lTileExtents, lModelExtentsV2 ) )
    {
      // If save mode == 1 recalc UID
      if(this->changed==1)
      {
        it->second.d1 = mPositionX * 10000000 + mPositionZ * 100000 + UID_counter++;
      }
    }
  }
}

void MapTile::saveTile()
{
  Log << "Saving ADT \"" << mFilename << "\"." << std::endl;
  LogDebug << "CHANGED FLAG "<< changed << std::endl;
  int lID;  // This is a global counting variable. Do not store something in here you need later.

  // Collect some information we need later.

  // Check which doodads and WMOs are on this ADT.
  Vec3D lTileExtents[2];
  lTileExtents[0] = Vec3D( this->xbase, 0.0f, this->zbase );
  lTileExtents[1] = Vec3D( this->xbase + TILESIZE, 0.0f, this->zbase + TILESIZE );

  std::map<int, WMOInstance> lObjectInstances;
  std::map<int, ModelInstance> lModelInstances;

  unsigned int UID_counter = 1;
  for( std::map<int, WMOInstance>::iterator it = gWorld->mWMOInstances.begin(); it != gWorld->mWMOInstances.end(); ++it )
  {
    if( checkInside( lTileExtents, it->second.extents ) )
    {
      lObjectInstances.insert( std::pair<int, WMOInstance>( it->first, it->second ) );
    }
  }
  

  for( std::map<int, ModelInstance>::iterator it = gWorld->mModelInstances.begin(); it != gWorld->mModelInstances.end(); ++it )
  {
    Vec3D lModelExtentsV1[2], lModelExtentsV2[2];
    lModelExtentsV1[0] = it->second.model->header.BoundingBoxMin + it->second.pos;
    lModelExtentsV1[1] = it->second.model->header.BoundingBoxMax + it->second.pos;
    lModelExtentsV2[0] = it->second.model->header.VertexBoxMin + it->second.pos;
    lModelExtentsV2[1] = it->second.model->header.VertexBoxMax + it->second.pos;

    if( checkInside( lTileExtents, lModelExtentsV1 ) || checkInside( lTileExtents, lModelExtentsV2 ) )
    {
      lModelInstances.insert( std::pair<int, ModelInstance>( it->first, it->second ) );
    }
  }

  filenameOffsetThing nullyThing = { 0, 0 };

  std::map<std::string, filenameOffsetThing> lModels;

  for( std::map<int,ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
  {
    //! \todo  Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
    std::string lTemp = it->second.model->_filename;
    transform( lTemp.begin(), lTemp.end(), lTemp.begin(), ::tolower );
    size_t found = lTemp.rfind( ".m2" );
    if( found != std::string::npos )
    {
      lTemp.replace( found, 3, ".md" );
      lTemp.append( "x" );
    }

    if( lModels.find( lTemp ) == lModels.end() )
    {
      lModels.insert( std::pair<std::string, filenameOffsetThing>( lTemp, nullyThing ) );
    }
  }

  lID = 0;
  for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
    it->second.nameID = lID++;

  std::map<std::string, filenameOffsetThing> lObjects;

  for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
    if( lObjects.find( it->second.wmo->_filename ) == lObjects.end() )
      lObjects.insert( std::pair<std::string, filenameOffsetThing>( ( it->second.wmo->_filename ), nullyThing ) );

  lID = 0;
  for( std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it )
    it->second.nameID = lID++;

  // Check which textures are on this ADT.
  std::map<std::string, int> lTextures;
#if 0
  //used to store texteffectinfo
  std::vector<int> mTextureEffects;
#endif

  for( int i = 0; i < 16; ++i )
    for( int j = 0; j < 16; ++j )
      for( size_t tex = 0; tex < mChunks[i][j]->textureSet->num(); tex++ )
        if( lTextures.find( mChunks[i][j]->textureSet->filename(tex) ) == lTextures.end() )
          lTextures.insert( std::pair<std::string, int>( mChunks[i][j]->textureSet->filename(tex), -1 ) );

  lID = 0;
  for( std::map<std::string, int>::iterator it = lTextures.begin(); it != lTextures.end(); ++it )
    it->second = lID++;

#if 0
  //! \todo actually, the folder is completely independant of this. Handle this differently. Bullshit here.
  std::string cmpCubeMaps = std::string("terrain cube maps");
  for( std::map<std::string, int>::iterator it = lTextures.begin(); it != lTextures.end(); ++it ){
    //if texture is in folder terrain cube maps, it needs to get handled different by wow
    if(it->first.compare(8, 17, cmpCubeMaps) == 0){
      Log<<it->second <<": "<< it->first << std::endl;
      mTextureEffects.push_back(1);
    }
    else
      mTextureEffects.push_back(0);
  }
#endif

  // Now write the file.

  sExtendableArray lADTFile = sExtendableArray();

  int lCurrentPosition = 0;

  // MVER
  //  {
  lADTFile.Extend( 8 + 0x4 );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MVER', 4 );

  // MVER data
  *( lADTFile.GetPointer<int>( 8 ) ) = 18;

  lCurrentPosition += 8 + 0x4;
  //  }

  // MHDR
  int lMHDR_Position = lCurrentPosition;
  //  {
  lADTFile.Extend( 8 + 0x40 );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MHDR', 0x40 );

  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->flags = mFlags;

  lCurrentPosition += 8 + 0x40;
  //  }

  // MCIN
  int lMCIN_Position = lCurrentPosition;
  //  {
  lADTFile.Extend( 8 + 256 * 0x10 );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MCIN', 256 * 0x10 );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mcin = lCurrentPosition - 0x14;

  // MCIN * MCIN_Data = lADTFile.GetPointer<MCIN>( lMCIN_Position + 8 );

  lCurrentPosition += 8 + 256 * 0x10;
  //  }

  // MTEX
  //  {
  int lMTEX_Position = lCurrentPosition;
  lADTFile.Extend( 8 + 0 );  // We don't yet know how big this will be.
  SetChunkHeader( lADTFile, lCurrentPosition, 'MTEX' );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mtex = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  // MTEX data
  for( std::map<std::string, int>::iterator it = lTextures.begin(); it != lTextures.end(); ++it )
  {
    lADTFile.Insert( lCurrentPosition, it->first.size() + 1, it->first.c_str() );
    lCurrentPosition += it->first.size() + 1;
    lADTFile.GetPointer<sChunkHeader>( lMTEX_Position )->mSize += it->first.size() + 1;
    LogDebug << "Added texture \"" << it->first << "\"." << std::endl;
  }
  //  }

  // MMDX
  //  {
  int lMMDX_Position = lCurrentPosition;
  lADTFile.Extend( 8 + 0 );  // We don't yet know how big this will be.
  SetChunkHeader( lADTFile, lCurrentPosition, 'MMDX' );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mmdx = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  // MMDX data
  for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
  {
    it->second.filenamePosition = lADTFile.GetPointer<sChunkHeader>( lMMDX_Position )->mSize;
    lADTFile.Insert( lCurrentPosition, it->first.size() + 1, it->first.c_str() );
    lCurrentPosition += it->first.size() + 1;
    lADTFile.GetPointer<sChunkHeader>( lMMDX_Position )->mSize += it->first.size() + 1;
    LogDebug << "Added model \"" << it->first << "\"." << std::endl;
  }
  //  }

  // MMID
  //  {
  int lMMID_Size = 4 * lModels.size();
  lADTFile.Extend( 8 + lMMID_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MMID', lMMID_Size );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mmid = lCurrentPosition - 0x14;

  // MMID data
  int * lMMID_Data = lADTFile.GetPointer<int>( lCurrentPosition + 8 );

  lID = 0;
  for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
    lMMID_Data[lID++] = it->second.filenamePosition;

  lCurrentPosition += 8 + lMMID_Size;
  //  }

  // MWMO
  //  {
  int lMWMO_Position = lCurrentPosition;
  lADTFile.Extend( 8 + 0 );  // We don't yet know how big this will be.
  SetChunkHeader( lADTFile, lCurrentPosition, 'MWMO' );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mwmo = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  // MWMO data
  for( std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it )
  {
    it->second.filenamePosition = lADTFile.GetPointer<sChunkHeader>( lMWMO_Position )->mSize;
    lADTFile.Insert( lCurrentPosition, it->first.size() + 1, it->first.c_str() );
    lCurrentPosition += it->first.size() + 1;
    lADTFile.GetPointer<sChunkHeader>( lMWMO_Position )->mSize += it->first.size() + 1;
    LogDebug << "Added object \"" << it->first << "\"." << std::endl;
  }
  //  }

  // MWID
  //  {
  int lMWID_Size = 4 * lObjects.size();
  lADTFile.Extend( 8 + lMWID_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MWID', lMWID_Size );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mwid = lCurrentPosition - 0x14;

  // MWID data
  int * lMWID_Data = lADTFile.GetPointer<int>( lCurrentPosition + 8 );

  lID = 0;
  for( std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it )
    lMWID_Data[lID++] = it->second.filenamePosition;

  lCurrentPosition += 8 + lMWID_Size;
  //  }

  // MDDF
  //  {
  int lMDDF_Size = 0x24 * lModelInstances.size();
  lADTFile.Extend( 8 + lMDDF_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MDDF', lMDDF_Size );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mddf = lCurrentPosition - 0x14;

  // MDDF data
  ENTRY_MDDF * lMDDF_Data = lADTFile.GetPointer<ENTRY_MDDF>( lCurrentPosition + 8 );

  lID = 0;
  for( std::map<int,ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
  {
    //! \todo  Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
    std::string lTemp = it->second.model->_filename;
    transform( lTemp.begin(), lTemp.end(), lTemp.begin(), ::tolower );
    size_t found = lTemp.rfind( ".m2" );
    if( found != std::string::npos )
    {
      lTemp.replace( found, 3, ".md" );
      lTemp.append( "x" );
    }
    std::map<std::string, filenameOffsetThing>::iterator lMyFilenameThingey = lModels.find( lTemp );
    if( lMyFilenameThingey == lModels.end() )
    {
      LogError << "There is a problem with saving the doodads. We have a doodad that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
      return;
    }

    lMDDF_Data[lID].nameID = lMyFilenameThingey->second.nameID;
    lMDDF_Data[lID].uniqueID = it->second.d1;
    lMDDF_Data[lID].pos[0] = it->second.pos.x;
    lMDDF_Data[lID].pos[1] = it->second.pos.y;
    lMDDF_Data[lID].pos[2] = it->second.pos.z;
    lMDDF_Data[lID].rot[0] = it->second.dir.x;
    lMDDF_Data[lID].rot[1] = it->second.dir.y;
    lMDDF_Data[lID].rot[2] = it->second.dir.z;
    lMDDF_Data[lID].scale = it->second.sc * 1024;
    lMDDF_Data[lID].flags = 0;
    lID++;
  }

  lCurrentPosition += 8 + lMDDF_Size;
  //  }

  // MODF
  //  {
  int lMODF_Size = 0x40 * lObjectInstances.size();
  lADTFile.Extend( 8 + lMODF_Size );
  SetChunkHeader( lADTFile, lCurrentPosition, 'MODF', lMODF_Size );
  lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->modf = lCurrentPosition - 0x14;

  // MODF data
  ENTRY_MODF * lMODF_Data = lADTFile.GetPointer<ENTRY_MODF>( lCurrentPosition + 8 );

  lID = 0;
  for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
  {
    std::map<std::string, filenameOffsetThing>::iterator lMyFilenameThingey = lObjects.find( it->second.wmo->_filename );
    if( lMyFilenameThingey == lObjects.end() )
    {
      LogError << "There is a problem with saving the objects. We have an object that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
      return;
    }



    lMODF_Data[lID].nameID = lMyFilenameThingey->second.nameID;
    lMODF_Data[lID].uniqueID = it->second.mUniqueID;
    lMODF_Data[lID].pos[0] = it->second.pos.x;
    lMODF_Data[lID].pos[1] = it->second.pos.y;
    lMODF_Data[lID].pos[2] = it->second.pos.z;
    lMODF_Data[lID].rot[0] = it->second.dir.x;
    lMODF_Data[lID].rot[1] = it->second.dir.y;
    lMODF_Data[lID].rot[2] = it->second.dir.z;
    //! \todo  Calculate them here or when rotating / moving? What is nicer? We should at least do it somewhere..
    lMODF_Data[lID].extents[0][0] = it->second.extents[0].x;
    lMODF_Data[lID].extents[0][1] = it->second.extents[0].y;
    lMODF_Data[lID].extents[0][2] = it->second.extents[0].z;
    lMODF_Data[lID].extents[1][0] = it->second.extents[1].x;
    lMODF_Data[lID].extents[1][1] = it->second.extents[1].y;
    lMODF_Data[lID].extents[1][2] = it->second.extents[1].z;
    lMODF_Data[lID].flags = it->second.mFlags;
    lMODF_Data[lID].doodadSet = it->second.doodadset;
    lMODF_Data[lID].nameSet = it->second.mNameset;
    lMODF_Data[lID].unknown = it->second.mUnknown;
    lID++;
  }

  lCurrentPosition += 8 + lMODF_Size;
  //  }

  //MH2O just data saving

  // Beket's temporary way to save a water
  // Just insert full MH2O data from original .adt to generated one...
  // Still need to fix Bernds way...
  /*if(mWaterSize>0){					//if has water... had a stupid crashes because of not checking this =))
    lADTFile.Extend(8+mWaterSize);
    lADTFile.GetPointer<MHDR>(lMHDR_Position + 8)->mh2o = lCurrentPosition - 0x14;
    LogDebug << "Water size "<< mWaterSize << std::endl;
    lADTFile.Insert( lCurrentPosition, mWaterSize, MH2O_Buffer );
    lCurrentPosition += 8+mWaterSize;
  }*/

  Water->saveToFile(lADTFile, lMHDR_Position, lCurrentPosition);
  
  // MCNK
  //  {
  for( int y = 0; y < 16; ++y )
  {
    for( int x = 0; x < 16; ++x )
    {
      mChunks[y][x]->save(lADTFile, lCurrentPosition, lMCIN_Position, lTextures, lObjectInstances, lModelInstances);
    }
  }
  //  }

  // MFBO
  if( mFlags & 1 )
  {
    size_t chunkSize = sizeof( int16_t ) * 9 * 2;
    lADTFile.Extend( 8 + chunkSize );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MFBO', chunkSize );
    lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mfbo = lCurrentPosition - 0x14;

    int16_t* lMFBO_Data = lADTFile.GetPointer<int16_t>( lCurrentPosition + 8 );

    lID = 0;
    for( int i = 0; i < 9; ++i )
      lMFBO_Data[lID++] = mMinimumValues[i * 3 + 1];

    for( int i = 0; i < 9; ++i )
      lMFBO_Data[lID++] = mMaximumValues[i * 3 + 1];

    lCurrentPosition += 8 + chunkSize;
  }

  //! \todo Do not do bullshit here in MTFX.
#if 0
  if(!mTextureEffects.empty()) {
    //! \todo check if nTexEffects == nTextures, correct order etc.
    lADTFile.Extend( 8 + 4*mTextureEffects.size());
    SetChunkHeader( lADTFile, lCurrentPosition, 'MTFX', 4*mTextureEffects.size() );
    lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mtfx = lCurrentPosition - 0x14;

    uint32_t* lMTFX_Data = lADTFile.GetPointer<uint32_t>( lCurrentPosition + 8 );

    lID = 0;
    //they should be in the correct order...
    for(std::vector<int>::iterator it = mTextureEffects.begin(); it!= mTextureEffects.end(); ++it) {
      lMTFX_Data[lID] = *it;
      ++lID;
    }
    lCurrentPosition += 8 + sizeof( uint32_t ) * mTextureEffects.size();
  }
#endif

  lADTFile.Extend(lCurrentPosition-lADTFile.mSize); // cleaning unused nulls at the end of file
  MPQFile f( mFilename );
  f.setBuffer( lADTFile.GetPointer<char>(), lADTFile.mSize );
  f.SaveFile();
  f.close();
  
  gWorld->mapIndex->markOnDisc(this->mPositionX,this->mPositionZ,true);
}
