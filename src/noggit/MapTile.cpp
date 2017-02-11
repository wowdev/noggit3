// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Environment.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Settings.h>
#include <noggit/TileWater.hpp>
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/alphamap.hpp>
#include <noggit/map_index.hpp>
#include <noggit/texture_set.hpp>
#include <opengl/matrix.hpp>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <algorithm>
#include <cassert>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

int indexMapBuf(int x, int y)
{
  return ((y + 1) / 2) * 9 + (y / 2) * 8 + x;
}

MapTile::MapTile(int pX, int pZ, const std::string& pFilename, bool pBigAlpha, bool pLoadModels)
  : index(tile_index(pX, pZ))
  , xbase(pX * TILESIZE)
  , zbase(pZ * TILESIZE)
  , changed(0)
  , mBigAlpha(pBigAlpha)
  , mFilename(pFilename)
  , Water (this, xbase, zbase)
{
  MPQFile theFile(mFilename);

  Log << "Opening tile " << index.x << ", " << index.z << " (\"" << mFilename << "\") from " << (theFile.isExternal() ? "disk" : "MPQ") << "." << std::endl;

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

  theFile.read(&fourcc, 4);
  theFile.seekRelative(4);
  theFile.read(&version, 4);

  assert(fourcc == 'MVER' && version == 18);

  // - MHDR ----------------------------------------------

  theFile.read(&fourcc, 4);
  theFile.seekRelative(4);

  assert(fourcc == 'MHDR');

  theFile.read(&Header, sizeof(MHDR));

  mFlags = Header.flags;

  // - MCIN ----------------------------------------------

  theFile.seek(Header.mcin + 0x14);
  theFile.read(&fourcc, 4);
  theFile.seekRelative(4);

  assert(fourcc == 'MCIN');

  for (int i = 0; i < 256; ++i)
  {
    theFile.read(&lMCNKOffsets[i], 4);
    theFile.seekRelative(0xC);
  }

  // - MTEX ----------------------------------------------

  theFile.seek(Header.mtex + 0x14);
  theFile.read(&fourcc, 4);
  theFile.read(&size, 4);

  assert(fourcc == 'MTEX');

  {
    char* lCurPos = reinterpret_cast<char*>(theFile.getPointer());
    char* lEnd = lCurPos + size;

    while (lCurPos < lEnd)
    {
      mTextureFilenames.push_back(std::string(lCurPos));
      lCurPos += strlen(lCurPos) + 1;
    }
  }

  if (pLoadModels)
  {
    // - MMDX ----------------------------------------------

    theFile.seek(Header.mmdx + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MMDX');

    {
      char* lCurPos = reinterpret_cast<char*>(theFile.getPointer());
      char* lEnd = lCurPos + size;

      while (lCurPos < lEnd)
      {
        mModelFilenames.push_back(std::string(lCurPos));
        lCurPos += strlen(lCurPos) + 1;
      }
    }

    // - MWMO ----------------------------------------------

    theFile.seek(Header.mwmo + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MWMO');

    {
      char* lCurPos = reinterpret_cast<char*>(theFile.getPointer());
      char* lEnd = lCurPos + size;

      while (lCurPos < lEnd)
      {
        mWMOFilenames.push_back(std::string(lCurPos));
        lCurPos += strlen(lCurPos) + 1;
      }
    }

    // - MDDF ----------------------------------------------

    theFile.seek(Header.mddf + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MDDF');

    ENTRY_MDDF* mddf_ptr = reinterpret_cast<ENTRY_MDDF*>(theFile.getPointer());
    for (unsigned int i = 0; i < size / sizeof(ENTRY_MDDF); ++i)
    {
      lModelInstances.push_back(mddf_ptr[i]);
    }

    // - MODF ----------------------------------------------

    theFile.seek(Header.modf + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MODF');

    ENTRY_MODF* modf_ptr = reinterpret_cast<ENTRY_MODF*>(theFile.getPointer());
    for (unsigned int i = 0; i < size / sizeof(ENTRY_MODF); ++i)
    {
      lWMOInstances.push_back(modf_ptr[i]);
    }
  }

  // - MISC ----------------------------------------------

  //! \todo  Parse all chunks in the new style!

  // - MH2O ----------------------------------------------
  if (Header.mh2o != 0) {
    theFile.seek(Header.mh2o + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    int ofsW = Header.mh2o + 0x14 + 0x8;
    assert(fourcc == 'MH2O');

    Water.readFromFile(theFile, ofsW);
  }

  // - MFBO ----------------------------------------------

  if (mFlags & 1)
  {
    theFile.seek(Header.mfbo + 0x14);
    theFile.read(&fourcc, 4);
    theFile.read(&size, 4);

    assert(fourcc == 'MFBO');

    int16_t mMaximum[9], mMinimum[9];
    theFile.read(mMaximum, sizeof(mMaximum));
    theFile.read(mMinimum, sizeof(mMinimum));

    const float xPositions[] = { this->xbase, this->xbase + 266.0f, this->xbase + 533.0f };
    const float yPositions[] = { this->zbase, this->zbase + 266.0f, this->zbase + 533.0f };

    for (int y = 0; y < 3; y++)
    {
      for (int x = 0; x < 3; x++)
      {
        int pos = x + y * 3;
        mMinimumValues[pos] = {xPositions[x], static_cast<float> (mMinimum[pos]), yPositions[y]};
        mMaximumValues[pos] = {xPositions[x], static_cast<float> (mMaximum[pos]), yPositions[y]};
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

  if (pLoadModels)
  {

    // - Load WMOs -----------------------------------------

    for (std::vector<ENTRY_MODF>::iterator it = lWMOInstances.begin(); it != lWMOInstances.end(); ++it)
    {
      gWorld->mWMOInstances.emplace(it->uniqueID, WMOInstance(mWMOFilenames[it->nameID], &(*it)));
    }

    // - Load M2s ------------------------------------------

    for (std::vector<ENTRY_MDDF>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it)
    {
      gWorld->mModelInstances.emplace(it->uniqueID, ModelInstance(mModelFilenames[it->nameID], &(*it)));
    }
  }

  // - Load chunks ---------------------------------------

  for (int nextChunk = 0; nextChunk < 256; ++nextChunk)
  {
    theFile.seek(lMCNKOffsets[nextChunk]);
    mChunks[nextChunk / 16][nextChunk % 16] = std::make_unique<MapChunk> (this, &theFile, mBigAlpha);
  }

  theFile.close();

  // - Really done. --------------------------------------

  LogDebug << "Done loading tile " << index.x << "," << index.z << "." << std::endl;
}

bool MapTile::isTile(int pX, int pZ)
{
  return pX == index.x && pZ == index.z;
}

float MapTile::getMaxHeight()
{
  float maxHeight = -99999.0f;
  for (int nextChunk = 0; nextChunk < 256; ++nextChunk)
  {
    maxHeight = std::max(mChunks[nextChunk / 16][nextChunk % 16]->vmax.y, maxHeight);
  }
  return maxHeight;
}

void MapTile::toBigAlpha()
{
  mBigAlpha = true;
  for (size_t i = 0; i < 16; i++)
  {
    for (size_t j = 0; j < 16; j++)
    {
      mChunks[i][j]->toBigAlpha();
    }
  }
}

void MapTile::draw ( Frustum const& frustum
                   , bool highlightPaintableChunks
                   , bool draw_contour
                   , bool draw_paintability_overlay
                   , bool draw_chunk_flag_overlay
                   , bool draw_water_overlay
                   , bool draw_areaid_overlay
                   , bool draw_wireframe_overlay
                   )
{
  gl.color4f(1, 1, 1, 1);

  for (int j = 0; j<16; ++j)
  {
    for (int i = 0; i<16; ++i)
    {
      mChunks[j][i]->draw ( frustum
                          , highlightPaintableChunks
                          , draw_contour
                          , draw_paintability_overlay
                          , draw_chunk_flag_overlay
                          , draw_water_overlay
                          , draw_areaid_overlay
                          , draw_wireframe_overlay
                          );
    }
  }
}

void MapTile::intersect (math::ray const& ray, selection_result* results) const
{
  for (size_t j (0); j < 16; ++j)
  {
    for (size_t i (0); i < 16; ++i)
    {
      mChunks[j][i]->intersect (ray, results);
    }
  }
}

void MapTile::drawLines (Frustum const& frustum)//draw red lines around the square of a chunk
{
  gl.disable(GL_COLOR_MATERIAL);

  for (int j = 0; j<16; ++j)
    for (int i = 0; i<16; ++i)
      mChunks[j][i]->drawLines (frustum);

  gl.enable(GL_COLOR_MATERIAL);
}

void MapTile::drawMFBO (opengl::scoped::use_program& mfbo_shader)
{
  static unsigned char const indices[] = { 4, 1, 2, 5, 8, 7, 6, 3, 0, 1, 0, 3, 6, 7, 8, 5, 2, 1 };

  mfbo_shader.attrib ("position", mMaximumValues);
  mfbo_shader.uniform ("color", math::vector_4d (0.0f, 1.0f, 1.0f, 0.2f));
  gl.drawElements (GL_TRIANGLE_FAN, sizeof (indices) / sizeof (*indices), GL_UNSIGNED_BYTE, indices);

  mfbo_shader.attrib ("position", mMinimumValues);
  mfbo_shader.uniform ("color", math::vector_4d (1.0f, 1.0f, 0.0f, 0.2f));
  gl.drawElements (GL_TRIANGLE_FAN, sizeof (indices) / sizeof (*indices), GL_UNSIGNED_BYTE, indices);
}

void MapTile::drawWater (opengl::scoped::use_program& water_shader)
{
  if (!Water.hasData(0))
  {
    return; //no need to draw water on tile without water =)
  }

  gl.disable(GL_COLOR_MATERIAL);
  gl.disable(GL_LIGHTING);

  Water.draw (water_shader);

  gl.enable(GL_LIGHTING);
  gl.enable(GL_COLOR_MATERIAL);
}

bool MapTile::canWaterSave() {
  return true;
}

void MapTile::getAlpha(size_t id, unsigned char *amap)
{
  int index = 0;
  int offsetIndex = 0;

  for (size_t j = 0; j < 1024; ++j)
  {
    index = (int)j / 64;

    for (int i = 0; i < 16; ++i)
    {
      if (mChunks[index][i]->_texture_set.num() > id + 1)
      {
        memcpy(amap + j * 1024 + i * 64, mChunks[index][i]->_texture_set.getAlpha(id) + offsetIndex * 64, 64);
      }
      else
      {
        memset(amap + j * 1024 + i * 64, 1, 64);
      }
    }

    if (offsetIndex == 63)
      offsetIndex = 0;
    else
      offsetIndex++;
  }
}

// This is for the 2D mode only.
void MapTile::drawTextures()
{
  float xOffset, yOffset;

  opengl::scoped::matrix_pusher const matrix;

  yOffset = zbase / CHUNKSIZE;
  xOffset = xbase / CHUNKSIZE;
  gl.translatef(xOffset, yOffset, 0);

  //gl.translatef(-8,-8,0);

  for (int j = 0; j<16; ++j) {
    for (int i = 0; i<16; ++i) {
      if (((i + 1 + xOffset)>gWorld->minX) && ((j + 1 + yOffset)>gWorld->minY) && ((i + xOffset)<gWorld->maxX) && ((j + yOffset)<gWorld->maxY))
        mChunks[j][i]->drawTextures();
    }
  }
}

MapChunk* MapTile::getChunk(unsigned int x, unsigned int z)
{
  if (x < 16 && z < 16)
  {
    return mChunks[z][x].get();
  }
  else
  {
    return nullptr;
  }
}

std::vector<MapChunk*> MapTile::chunks_in_range (math::vector_3d const& pos, float radius) const
{
  std::vector<MapChunk*> chunks;

  for (size_t ty (0); ty < 16; ++ty)
  {
    for (size_t tx (0); tx < 16; ++tx)
    {
      if (misc::getShortestDist (pos.x, pos.z, mChunks[ty][tx]->xbase, mChunks[ty][tx]->zbase, CHUNKSIZE) <= radius)
      {
        chunks.emplace_back (mChunks[ty][tx].get());
      }
    }
  }

  return chunks;
}


bool MapTile::GetVertex(float x, float z, math::vector_3d *V)
{
  int xcol = (int)((x - xbase) / CHUNKSIZE);
  int ycol = (int)((z - zbase) / CHUNKSIZE);

  return xcol >= 0 && xcol <= 15 && ycol >= 0 && ycol <= 15 && mChunks[ycol][xcol]->GetVertex(x, z, V);
}

/// --- Only saving related below this line. --------------------------

void MapTile::clearAllModels()
{
  Log << "Clear all models from ADT \"" << mFilename << "\"." << std::endl;

  // Check which doodads and WMOs are on this ADT.

  for (std::map<int, WMOInstance>::iterator it = gWorld->mWMOInstances.begin(); it != gWorld->mWMOInstances.end(); ++it)
  {
    if (tile_index(it->second.pos) == index)
    {
      gWorld->deleteWMOInstance(it->second.mUniqueID);
    }
  }

  for (std::map<int, ModelInstance>::iterator it = gWorld->mModelInstances.begin(); it != gWorld->mModelInstances.end(); ++it)
  {
    if (tile_index(it->second.pos) == index)
    {
      gWorld->deleteModelInstance(it->second.d1);
    }
  }
}

void MapTile::saveTile(bool saveAllModels)
{

  Log << "Saving ADT \"" << mFilename << "\"." << std::endl;
  LogDebug << "CHANGED FLAG " << changed << std::endl;
  int lID;  // This is a global counting variable. Do not store something in here you need later.

            // if wod output path is set creat also wod map files and save them in this alternate path.
  bool wodSave = false;
  std::string wodSavePath = "";
  if (Settings::getInstance()->wodSavePath != "")
  {
    wodSave = true;
    wodSavePath = Settings::getInstance()->wodSavePath;
    LogDebug << "WOD Save path is set to : " << wodSavePath << std::endl;
  }

  std::map<int, WMOInstance> lObjectInstances;
  std::map<int, ModelInstance> lModelInstances;

  // Collect some information we need later.

  // Check which doodads and WMOs are on this ADT.
  math::vector_3d lTileExtents[2];
  lTileExtents[0] = math::vector_3d(xbase, 0.0f, zbase);
  lTileExtents[1] = math::vector_3d(xbase + TILESIZE, 0.0f, zbase + TILESIZE);


  for (std::map<int, WMOInstance>::iterator it = gWorld->mWMOInstances.begin(); it != gWorld->mWMOInstances.end(); ++it)
  {
    if (saveAllModels || it->second.isInsideRect(lTileExtents))
    {
      lObjectInstances.emplace(it->second.mUniqueID, it->second);
    }
  }

  for (std::map<int, ModelInstance>::iterator it = gWorld->mModelInstances.begin(); it != gWorld->mModelInstances.end(); ++it)
  {
    if (saveAllModels || it->second.isInsideRect(lTileExtents))
    {
      lModelInstances.emplace(it->second.d1, it->second);
    }
  }


  filenameOffsetThing nullyThing = { 0, 0 };

  std::map<std::string, filenameOffsetThing> lModels;

  for (std::map<int, ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it)
    if (lModels.find(it->second.model->_filename) == lModels.end())
      lModels.insert(std::pair<std::string, filenameOffsetThing>(it->second.model->_filename, nullyThing));

  lID = 0;
  for (std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it)
    it->second.nameID = lID++;

  std::map<std::string, filenameOffsetThing> lObjects;

  for (std::map<int, WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it)
    if (lObjects.find(it->second.wmo->_filename) == lObjects.end())
      lObjects.insert(std::pair<std::string, filenameOffsetThing>((it->second.wmo->_filename), nullyThing));

  lID = 0;
  for (std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it)
    it->second.nameID = lID++;

  // Check which textures are on this ADT.
  std::map<std::string, int> lTextures;

  for (int i = 0; i < 16; ++i)
    for (int j = 0; j < 16; ++j)
      for (size_t tex = 0; tex < mChunks[i][j]->_texture_set.num(); tex++)
        if (lTextures.find(mChunks[i][j]->_texture_set.filename(tex)) == lTextures.end())
          lTextures.insert(std::pair<std::string, int>(mChunks[i][j]->_texture_set.filename(tex), -1));

  lID = 0;
  for (std::map<std::string, int>::iterator it = lTextures.begin(); it != lTextures.end(); ++it)
    it->second = lID++;

  // Now write the file.
  sExtendableArray *lADTFile = new sExtendableArray();


  // Create the other files for wod saving
  int lADTRootFileCurrentPosition = 0;
  sExtendableArray *lADTRootFile = new sExtendableArray();
  int lADTObjFileCurrentPosition = 0;
  sExtendableArray *lADTObjFile = new sExtendableArray();
  int lADTTexFileCurrentPosition = 0;
  sExtendableArray *lADTTexFile = new sExtendableArray();




  int lCurrentPosition = 0;

  // MVER
  lADTFile->Extend(8 + 0x4);
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MVER', 4);

  if (wodSave)
  {
    // WOD ALL
    lADTRootFile->Extend(8 + 0x4);
    SetChunkHeader(*lADTRootFile, lADTRootFileCurrentPosition, 'MVER', 4);
    lADTObjFile->Extend(8 + 0x4);
    SetChunkHeader(*lADTObjFile, lADTObjFileCurrentPosition, 'MVER', 4);
    lADTTexFile->Extend(8 + 0x4);
    SetChunkHeader(*lADTTexFile, lADTTexFileCurrentPosition, 'MVER', 4);
  }

  // MVER data
  *(lADTFile->GetPointer<int>(8)) = 18;
  lCurrentPosition += 8 + 0x4;

  if (wodSave)
  {
    // WOD ALL
    *(lADTRootFile->GetPointer<int>(8)) = 18;
    lADTRootFileCurrentPosition += 8 + 0x4;
    *(lADTObjFile->GetPointer<int>(8)) = 18;
    lADTObjFileCurrentPosition += 8 + 0x4;
    *(lADTTexFile->GetPointer<int>(8)) = 18;
    lADTTexFileCurrentPosition += 8 + 0x4;
  }


  // MHDR
  int lMHDR_Position = lCurrentPosition;
  lADTFile->Extend(8 + 0x40);
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MHDR', 0x40);

  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->flags = mFlags;

  lCurrentPosition += 8 + 0x40;


  if (wodSave)
  {
    // WOD ROOT
    int ROOT_lMHDR_Position = lADTRootFileCurrentPosition;
    lADTRootFile->Extend(8 + 0x40);
    SetChunkHeader(*lADTRootFile, lADTRootFileCurrentPosition, 'MHDR', 0x40);
    lADTRootFile->GetPointer<MHDR>(ROOT_lMHDR_Position + 8)->flags = mFlags;
    lADTRootFileCurrentPosition += 8 + 0x40;
  }


  // MCIN
  int lMCIN_Position = lCurrentPosition;

  lADTFile->Extend(8 + 256 * 0x10);
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MCIN', 256 * 0x10);
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mcin = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 256 * 0x10;

  // MCIN dont exist in wod so no save

  // MAMP TODO:need implementation for WOD here!!!!!

  // MTEX
  int lMTEX_Position = lCurrentPosition;
  lADTFile->Extend(8 + 0);  // We don't yet know how big this will be.
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MTEX');
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mtex = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  int TEX_lMTEX_Position = 0;
  if (wodSave)
  {
    // WOD TEX
    TEX_lMTEX_Position = lADTTexFileCurrentPosition;
    lADTTexFile->Extend(8 + 0);  // We don't yet know how big this will be.
    SetChunkHeader(*lADTTexFile, lADTTexFileCurrentPosition, 'MTEX');
    lADTTexFile->GetPointer<MHDR>(lMHDR_Position + 8)->mtex = lADTTexFileCurrentPosition - 0x14;

    lADTTexFileCurrentPosition += 8 + 0;
  }


  // MTEX data
  for (std::map<std::string, int>::iterator it = lTextures.begin(); it != lTextures.end(); ++it)
  {
    lADTFile->Insert(lCurrentPosition, it->first.size() + 1, it->first.c_str());

    lCurrentPosition += it->first.size() + 1;
    lADTFile->GetPointer<sChunkHeader>(lMTEX_Position)->mSize += it->first.size() + 1;
    LogDebug << "Added texture \"" << it->first << "\"." << std::endl;

    if (wodSave)
    {
      // WOD TEX
      lADTTexFile->Insert(lADTTexFileCurrentPosition, it->first.size() + 1, it->first.c_str());
      lADTTexFileCurrentPosition += it->first.size() + 1;
      lADTTexFile->GetPointer<sChunkHeader>(TEX_lMTEX_Position)->mSize += it->first.size() + 1;
    }
  }




  // MMDX
  int lMMDX_Position = lCurrentPosition;
  lADTFile->Extend(8 + 0);  // We don't yet know how big this will be.
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MMDX');
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mmdx = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;


  int OBJ_lMMDX_Position;
  if (wodSave)
  {
    // WOD OBJ
    OBJ_lMMDX_Position = lADTObjFileCurrentPosition;
    lADTObjFile->Extend(8 + 0);  // We don't yet know how big this will be.
    SetChunkHeader(*lADTObjFile, lADTObjFileCurrentPosition, 'MMDX');
    lADTRootFile->GetPointer<MHDR>(OBJ_lMMDX_Position + 8)->mmdx = lADTObjFileCurrentPosition - 0x14; //ISTHISRIGHT ???MHDR is in root so I set the mmdx value there not in obj

    lADTObjFileCurrentPosition += 8 + 0;
  }


  // MMDX data
  for (std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it)
  {
    it->second.filenamePosition = lADTFile->GetPointer<sChunkHeader>(lMMDX_Position)->mSize;
    lADTFile->Insert(lCurrentPosition, it->first.size() + 1, it->first.c_str());
    lCurrentPosition += it->first.size() + 1;
    lADTFile->GetPointer<sChunkHeader>(lMMDX_Position)->mSize += it->first.size() + 1;
    LogDebug << "Added model \"" << it->first << "\"." << std::endl;

    if (wodSave)
    {
      // WOD OBJ
      it->second.filenamePosition = lADTObjFile->GetPointer<sChunkHeader>(OBJ_lMMDX_Position)->mSize;
      lADTObjFile->Insert(lADTObjFileCurrentPosition, it->first.size() + 1, it->first.c_str());
      lADTObjFileCurrentPosition += it->first.size() + 1;
      lADTObjFile->GetPointer<sChunkHeader>(OBJ_lMMDX_Position)->mSize += it->first.size() + 1;
    }


  }

  // MMID
  // M2 model names
  int lMMID_Size = 4 * lModels.size();
  lADTFile->Extend(8 + lMMID_Size);
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MMID', lMMID_Size);
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mmid = lCurrentPosition - 0x14;

  if (wodSave)
  {
    // WOD OBJ
    lADTObjFile->Extend(8 + lMMID_Size);
    SetChunkHeader(*lADTObjFile, lADTObjFileCurrentPosition, 'MMID', lMMID_Size);
    lADTRootFile->GetPointer<MHDR>(lMHDR_Position + 8)->mmid = lADTObjFileCurrentPosition - 0x14;
  }

  // MMID data
  // WMO model names
  int * lMMID_Data = lADTFile->GetPointer<int>(lCurrentPosition + 8);
  int * OBJlMMID_Data = lADTObjFile->GetPointer<int>(lADTObjFileCurrentPosition + 8); // WOD OBJ

  lID = 0;
  for (std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it)
  {
    lMMID_Data[lID] = it->second.filenamePosition;
    if (wodSave) OBJlMMID_Data[lID] = it->second.filenamePosition; // WOD OBJ
    lID++;
  }
  lCurrentPosition += 8 + lMMID_Size;
  if (wodSave) lADTObjFileCurrentPosition += 8 + lMMID_Size; // WOD OBJ

                                                             // MWMO
  int lMWMO_Position = lCurrentPosition;
  lADTFile->Extend(8 + 0);  // We don't yet know how big this will be.
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MWMO');
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mwmo = lCurrentPosition - 0x14;

  lCurrentPosition += 8 + 0;

  // MWMO data
  for (std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it)
  {
    it->second.filenamePosition = lADTFile->GetPointer<sChunkHeader>(lMWMO_Position)->mSize;
    lADTFile->Insert(lCurrentPosition, it->first.size() + 1, it->first.c_str());
    lCurrentPosition += it->first.size() + 1;
    lADTFile->GetPointer<sChunkHeader>(lMWMO_Position)->mSize += it->first.size() + 1;
    LogDebug << "Added object \"" << it->first << "\"." << std::endl;
  }

  // MWID
  int lMWID_Size = 4 * lObjects.size();
  lADTFile->Extend(8 + lMWID_Size);
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MWID', lMWID_Size);
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mwid = lCurrentPosition - 0x14;

  // MWID data
  int * lMWID_Data = lADTFile->GetPointer<int>(lCurrentPosition + 8);

  lID = 0;
  for (std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it)
    lMWID_Data[lID++] = it->second.filenamePosition;

  lCurrentPosition += 8 + lMWID_Size;

  // MDDF
  int lMDDF_Size = 0x24 * lModelInstances.size();
  lADTFile->Extend(8 + lMDDF_Size);
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MDDF', lMDDF_Size);
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mddf = lCurrentPosition - 0x14;

  // MDDF data
  ENTRY_MDDF* lMDDF_Data = lADTFile->GetPointer<ENTRY_MDDF>(lCurrentPosition + 8);

  lID = 0;
  for (std::map<int, ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it)
  {
    std::map<std::string, filenameOffsetThing>::iterator lMyFilenameThingey = lModels.find(it->second.model->_filename);
    if (lMyFilenameThingey == lModels.end())
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
    lMDDF_Data[lID].scale = (uint16_t)(it->second.sc * 1024);
    lMDDF_Data[lID].flags = 0;
    lID++;
  }

  lCurrentPosition += 8 + lMDDF_Size;

  LogDebug << "Added " << lID << " doodads to MDDF" << std::endl;

  // MODF
  int lMODF_Size = 0x40 * lObjectInstances.size();
  lADTFile->Extend(8 + lMODF_Size);
  SetChunkHeader(*lADTFile, lCurrentPosition, 'MODF', lMODF_Size);
  lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->modf = lCurrentPosition - 0x14;

  // MODF data
  ENTRY_MODF *lMODF_Data = lADTFile->GetPointer<ENTRY_MODF>(lCurrentPosition + 8);

  lID = 0;
  for (std::map<int, WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it)
  {
    std::map<std::string, filenameOffsetThing>::iterator lMyFilenameThingey = lObjects.find(it->second.wmo->_filename);
    if (lMyFilenameThingey == lObjects.end())
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

  LogDebug << "Added " << lID << " wmos to MODF" << std::endl;

  lCurrentPosition += 8 + lMODF_Size;

  //MH2O
  Water.saveToFile(*lADTFile, lMHDR_Position, lCurrentPosition);

  // MCNK
  for (int y = 0; y < 16; ++y)
  {
    for (int x = 0; x < 16; ++x)
    {
      mChunks[y][x]->save(*lADTFile, lCurrentPosition, lMCIN_Position, lTextures, lObjectInstances, lModelInstances);
    }
  }

  // MFBO
  if (mFlags & 1)
  {
    size_t chunkSize = sizeof(int16_t) * 9 * 2;
    lADTFile->Extend(8 + chunkSize);
    SetChunkHeader(*lADTFile, lCurrentPosition, 'MFBO', chunkSize);
    lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mfbo = lCurrentPosition - 0x14;

    int16_t *lMFBO_Data = lADTFile->GetPointer<int16_t>(lCurrentPosition + 8);

    lID = 0;

    for (int i = 0; i < 9; ++i)
      lMFBO_Data[lID++] = (int16_t)mMaximumValues[i].y;

    for (int i = 0; i < 9; ++i)
      lMFBO_Data[lID++] = (int16_t)mMinimumValues[i].y;

    lCurrentPosition += 8 + chunkSize;
  }

  //! \todo Do not do bullshit here in MTFX.
#if 0
  if (!mTextureEffects.empty()) {
    //! \todo check if nTexEffects == nTextures, correct order etc.
    lADTFile->Extend(8 + 4 * mTextureEffects.size());
    SetChunkHeader(*lADTFile, lCurrentPosition, 'MTFX', 4 * mTextureEffects.size());
    lADTFile->GetPointer<MHDR>(lMHDR_Position + 8)->mtfx = lCurrentPosition - 0x14;

    uint32_t* lMTFX_Data = lADTFile->GetPointer<uint32_t>(lCurrentPosition + 8);

    lID = 0;
    //they should be in the correct order...
    for (std::vector<int>::iterator it = mTextureEffects.begin(); it != mTextureEffects.end(); ++it) {
      lMTFX_Data[lID] = *it;
      ++lID;
    }
    lCurrentPosition += 8 + sizeof(uint32_t) * mTextureEffects.size();
  }
#endif

  lADTFile->Extend(lCurrentPosition - lADTFile->data.size()); // cleaning unused nulls at the end of file


  MPQFile *f = new MPQFile(mFilename);
  f->setBuffer(lADTFile->data);
  f->SaveFile();
  f->close();

  // save wod files
  if (wodSave)
  {
    // ADT root file
    MPQFile *f1 = new MPQFile(mFilename, wodSavePath);
    f1->setBuffer(lADTRootFile->data);
    f1->SaveFile();
    f1->close();

    // both tex files
    std::stringstream texFilename1;
    texFilename1 << mFilename.substr(0, mFilename.size() - 4) << "_tex0.adt";
    std::stringstream texFilename2;
    texFilename2 << mFilename.substr(0, mFilename.size() - 4) << "_tex1.adt";


    MPQFile *f2 = new MPQFile(texFilename1.str(), wodSavePath);
    f2->setBuffer(lADTTexFile->data);
    f2->SaveFile();
    f2->close();

    MPQFile *f3 = new MPQFile(texFilename2.str(), wodSavePath);
    f3->setBuffer(lADTTexFile->data);
    f3->SaveFile();
    f3->close();

    // both obj files
    std::stringstream objFilename1;
    objFilename1 << mFilename.substr(0, mFilename.size() - 4) << "_obj0.adt";
    std::stringstream objFilename2;
    objFilename2 << mFilename.substr(0, mFilename.size() - 4) << "_obj1.adt";


    MPQFile *f4 = new MPQFile(objFilename1.str(), wodSavePath);
    f4->setBuffer(lADTObjFile->data);
    f4->SaveFile();
    f4->close();

    MPQFile *f5 = new MPQFile(objFilename2.str(), wodSavePath);
    f5->setBuffer(lADTObjFile->data);
    f5->SaveFile();
    f5->close();
  }

  gWorld->mapIndex->markOnDisc(tile_index(this->index.x, this->index.z), true);

  lObjectInstances.clear();
  lModelInstances.clear();
  lModels.clear();

  delete f;
}


void MapTile::CropWater()
{
  for (int z = 0; z < 16; ++z)
  {
    for (int x = 0; x < 16; ++x)
    {
      Water.CropMiniChunk(x, z, mChunks[z][x].get());
    }
  }
}
