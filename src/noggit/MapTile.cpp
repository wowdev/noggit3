// MapTile.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Mjollnà <mjollna.wow@gmail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/MapTile.h>

#include <QDir>

#include <algorithm>
#include <cassert>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <math/bounded_nearest.h>

#include <noggit/blp_texture.h>
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/mpq/file.h>

MapTile::MapTile (World* world, int pX, int pZ, const std::string& pFilename, bool pBigAlpha)
  : mPositionX (pX)
  , mPositionZ (pZ)
//! \todo Actually, this is defined inside the ADT.
  , xbase (mPositionX * TILESIZE)
  , zbase (mPositionZ * TILESIZE)
  , mFlags (0)
  , mBigAlpha (pBigAlpha)
  , mTextureFilenames (0)
  , mModelFilenames (0)
  , mWMOFilenames (0)
  , mFilename (pFilename)
  , mLiquids (0)
  , _world (world)
{
  for( int i = 0; i < 16; ++i )
  {
    for( int j = 0; j < 16; j++ )
    {
      mChunks[i][j] = NULL;
    }
  }

  noggit::mpq::file theFile (QString::fromStdString (mFilename), true);

  Log << "Opening tile " << mPositionX << ", " << mPositionZ << " (\"" << mFilename << "\") from " << (theFile.file_is_on_disk() ? "disk" : "MPQ") << "." << std::endl;

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
    theFile.seek( Header.mh2o + 0x14 );
    theFile.read( &fourcc, 4 );
    theFile.read( &size, 4 );

    int ofsW = Header.mh2o + 0x14 + 0x8;
    assert( fourcc == 'MH2O' );
    MH2O_Header lHeader[256];
    theFile.read(lHeader, 256*sizeof(MH2O_Header));
    for(int i=0; i < 16; ++i) {
      for(int j=0; j < 16; ++j) {
        //! \todo Implement more than just one layer...
        if(lHeader[i*16 + j].nLayers < 1){ //if it has no layers, insert a dummy liquid tile for later use
          continue;
        }
        MH2O_Tile lTile;
        MH2O_Information info;
        theFile.seek(ofsW + lHeader[i*16 + j].ofsInformation);
        theFile.read(&info, sizeof(MH2O_Information));
        lTile.mLiquidType = info.LiquidType;
        lTile.mMaximum = info.maxHeight;
        lTile.mMinimum = info.minHeight;
        lTile.mFlags = info.Flags;

        for( int x = 0; x < 9; ++x ) {
          for( int y = 0; y < 9; y++ ) {
            lTile.mHeightmap[x][y] = lTile.mMinimum;
            lTile.mDepth[x][y] = 0.0f;
          }
        }

        if(info.ofsHeightMap != 0 && !(lTile.mFlags & 2)) {
          theFile.seek(ofsW + info.ofsHeightMap);
          for (int w = info.yOffset; w < info.yOffset + info.height + 1; ++w) {
            for(int h=info.xOffset; h < info.xOffset + info.width + 1; ++h) {
              float tmp;
              theFile.read(&tmp, sizeof(tmp));
              lTile.mHeightmap[w][h] = tmp;

              //! \todo raise Max/Min instead?
              if(lTile.mHeightmap[w][h] < lTile.mMinimum)
                lTile.mHeightmap[w][h] = lTile.mMinimum;
              if(lTile.mHeightmap[w][h] > lTile.mMaximum)
                lTile.mHeightmap[w][h] = lTile.mMaximum;
            }
          }
          for (int w = info.yOffset; w < info.yOffset + info.height + 1; ++w) {
            for(int h=info.xOffset; h < info.xOffset + info.width + 1; ++h) {
              char tmp;
              theFile.read(&tmp, sizeof(tmp));
              lTile.mDepth[w][h] = tmp/255.0f; //! \todo get this correct done
            }
          }
        }
        //! \todo investigate flags...
        if(info.ofsInfoMask != 0 /*&& !(lTile.mFlags & 2)*/) {
          theFile.seek(ofsW + info.ofsInfoMask);
          int h = 0;
          int w = 0;
          int shft = 0;
          char tmp;
          theFile.read(&tmp, sizeof(tmp));
          while(h < info.height) {
            if(shft == 8){
              shft = 0;
              theFile.read(&tmp, sizeof(tmp));
            }
            if(w >= info.width) {
              ++h;
              w = 0;
            }
            lTile.mRender[info.yOffset+h][info.xOffset+w] = tmp & (1 << shft);
            ++w;
            ++shft;
          }
        }
        else
        /* if (info.Flags == 0)*/
        {
          for(int h=info.yOffset ; h < info.yOffset+info.height; ++h) {
            for(int w=info.xOffset; w < info.xOffset+info.width; ++w) {
              lTile.mRender[h][w] = true;
            }
          }
        }
        //! \todo ...and check, if we can omit this, or what this really is.
        /*else if(lHeader[i*16 + j].ofsRenderMask!=0) {
          char render[8];
          theFile.seek(ofsW + lHeader[i*16 + j].ofsRenderMask);
          theFile.read(&render, 8*sizeof(char));
          for(int k=0 ; k < 8; ++k){
            for(int m=0; m < 8; ++m){
              lTile.mRender[k][m] |= render[k] & (1 << m);
            }
          }
        }
        else {
          for(int k=0 ; k < 8; ++k) {
            for(int m=0; m < 8; ++m) {
              lTile.mRender[k][m] = true;
            }
          }
        }*/


        Liquid* lq ( new Liquid ( info.width
                                , info.height
                                , ::math::vector_3d ( xbase + CHUNKSIZE * j
                                        , lTile.mMinimum
                                        , zbase + CHUNKSIZE * i
                                        )
                                )
                   );
        lq->setMH2OData (lTile);
        //LogDebug << "Inserted Data to MH2O: "<<i*16+j << std::endl;
        mLiquids.push_back (lq);
      }
    }

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

    const float xPositions[] = { xbase, xbase + 266.0f, xbase + 533.0f };
    const float yPositions[] = { zbase, zbase + 266.0f, zbase + 533.0f };

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

  //! \todo Implement this or just use Terrain Cube maps?

  /*
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
    _world->mWMOInstances.insert( std::pair<int,WMOInstance *>( it->uniqueID, new WMOInstance( _world, mWMOFilenames[it->nameID], &(*it) ) ) );
  }

  // - Load M2s ------------------------------------------

  for( std::vector<ENTRY_MDDF>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
  {
    _world->mModelInstances.insert( std::pair<int,ModelInstance *>( it->uniqueID, new ModelInstance( _world, mModelFilenames[it->nameID], &(*it) ) ) );
  }

  // - Load chunks ---------------------------------------

  for( int nextChunk = 0; nextChunk < 256; ++nextChunk )
  {
    theFile.seek( lMCNKOffsets[nextChunk] );
    mChunks[nextChunk / 16][nextChunk % 16] = new MapChunk( _world, this, &theFile, mBigAlpha );
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
      delete mChunks[j][i];
      mChunks[j][i] = NULL;
    }
  }

  mTextureFilenames.clear();

  for( std::vector<Liquid*>::iterator it = mLiquids.begin(); it != mLiquids.end(); ++it )
  {
    delete *it;
    *it  = NULL;
  }

  mLiquids.clear();
}

bool MapTile::isTile( int pX, int pZ )
{
  return pX == mPositionX && pZ == mPositionZ;
}

void MapTile::draw ( bool draw_terrain_height_contour
                   , bool mark_impassable_chunks
                   , bool draw_area_id_overlay
                   , bool dont_draw_cursor
                   , const Skies* skies
                   , const float& cull_distance
                   , const Frustum& frustum
                   , const ::math::vector_3d& camera
                   , const boost::optional<selection_type>& selected_item
                   )
{
  glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

  for (size_t j (0); j < 16; ++j)
  {
    for (size_t i (0); i < 16; ++i)
    {
      if (mChunks[j][i]->is_visible (cull_distance, frustum, camera))
      {
        mChunks[j][i]->draw ( draw_terrain_height_contour
                            , mark_impassable_chunks
                            , draw_area_id_overlay
                            , dont_draw_cursor
                            , skies
                            , selected_item
                            );
      }
    }
  }
}

void MapTile::drawSelect ( const float& cull_distance
                         , const Frustum& frustum
                         , const ::math::vector_3d& camera
                         )
{
  for (size_t j (0); j < 16; ++j)
  {
    for (size_t i (0); i < 16; ++i)
    {
      if (mChunks[j][i]->is_visible (cull_distance, frustum, camera))
      {
        mChunks[j][i]->drawSelect();
      }
    }
  }
}

void MapTile::drawLines ( bool draw_hole_lines
                        , const float& cull_distance
                        , const Frustum& frustum
                        , const ::math::vector_3d& camera
                        )
{
  glDisable (GL_COLOR_MATERIAL);

  for (size_t j (0); j < 16; ++j)
  {
    for (size_t i (0); i < 16; ++i)
    {
      if (mChunks[j][i]->is_visible (cull_distance, frustum, camera))
      {
        mChunks[j][i]->drawLines (draw_hole_lines);
      }
    }
  }

  glEnable (GL_COLOR_MATERIAL);
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

void MapTile::drawWater (const Skies* skies)
{
  glDisable (GL_COLOR_MATERIAL);
  glDisable (GL_LIGHTING);

  foreach (const Liquid* liquid, mLiquids)
  {
    liquid->draw (skies);
  }

  glEnable (GL_LIGHTING);
  glEnable (GL_COLOR_MATERIAL);
}

// This is for the 2D mode only.
void MapTile::drawTextures (const QRectF& chunks_to_draw) const
{
  glPushMatrix();

  glTranslatef (xbase / CHUNKSIZE, zbase / CHUNKSIZE, 0.0f);

  for (size_t j (chunks_to_draw.top()); j < chunks_to_draw.bottom(); ++j)
  {
    for (size_t i (chunks_to_draw.left()); i < chunks_to_draw.right(); ++i)
    {
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

boost::optional<float> MapTile::get_height ( const float& x
                                           , const float& z
                                           ) const
{
  const int xcol ((x - xbase) / CHUNKSIZE);
  const int ycol ((z - zbase) / CHUNKSIZE);

  if (xcol < 0 || xcol > 15 || ycol < 0 || ycol > 15)
  {
    return boost::none;
  }

  return mChunks[ycol][xcol]->get_height (x, z);
}


/// --- Only saving related below this line. --------------------------

void minmax (::math::vector_3d* a, ::math::vector_3d* b)
{
  if( a->x() > b->x() )
  {
    const float t (b->x());
    b->x (a->x());
    a->x (t);
  }
  if( a->y() > b->y() )
  {
    const float t (b->y());
    b->y (a->y());
    a->y (t);
  }
  if( a->z() > b->z() )
  {
    const float t (b->z());
    b->z (a->z());
    a->z (t);
  }
}

bool checkInside( ::math::vector_3d extentA[2], ::math::vector_3d extentB[2] )
{
  minmax( &extentA[0], &extentA[1] );
  minmax( &extentB[0], &extentB[1] );

  return extentA[0].is_inside_of (extentB[0], extentB[1]) ||
         extentA[1].is_inside_of (extentB[0], extentB[1]) ||
         extentB[0].is_inside_of (extentA[0], extentA[1]) ||
         extentB[1].is_inside_of (extentA[0], extentA[1]);
}

template<typename T>
T* get_pointer (std::vector<char>& vector, size_t pPosition = 0)
{
  return reinterpret_cast<T*> (&vector[pPosition]);
}

void insert_string ( std::vector<char>& vector
                   , size_t position
                   , const std::string& str
                   )
{
  const char* const cstr (str.c_str());
  vector.insert (vector.begin() + position, cstr, cstr + str.size() + 1);
}

struct sChunkHeader
{
  int mMagic;
  int mSize;
};

void SetChunkHeader( std::vector<char>& pArray, int pPosition, int pMagix, int pSize = 0 )
{
  sChunkHeader * Header = get_pointer<sChunkHeader>( pArray, pPosition );
  Header->mMagic = pMagix;
  Header->mSize = pSize;
}

struct filenameOffsetThing
{
  int nameID;
  int filenamePosition;
};

void MapTile::clearAllModels()
{
  Log << "Clear all models from ADT \"" << mFilename << "\"." << std::endl;

  // Collect some information we need later.

  // Check which doodads and WMOs are on this ADT.
  ::math::vector_3d lTileExtents[2];
  lTileExtents[0] = ::math::vector_3d( xbase, 0.0f, zbase );
  lTileExtents[1] = ::math::vector_3d( xbase + TILESIZE, 0.0f, zbase + TILESIZE );

  std::map<int, WMOInstance> lObjectInstances;
  std::map<int, ModelInstance> lModelInstances;

  for( std::map<int, WMOInstance *>::iterator it = _world->mWMOInstances.begin(); it != _world->mWMOInstances.end(); ++it )
    if( checkInside( lTileExtents, it->second->extents ) )
          _world->deleteWMOInstance( it->second->mUniqueID );

  for( std::map<int, ModelInstance*>::iterator it = _world->mModelInstances.begin(); it != _world->mModelInstances.end(); ++it )
  {
    ::math::vector_3d lModelExtentsV1[2], lModelExtentsV2[2];
    lModelExtentsV1[0] = it->second->model->header.BoundingBoxMin + it->second->pos;
    lModelExtentsV1[1] = it->second->model->header.BoundingBoxMax + it->second->pos;
    lModelExtentsV2[0] = it->second->model->header.VertexBoxMin + it->second->pos;
    lModelExtentsV2[1] = it->second->model->header.VertexBoxMax + it->second->pos;

    if( checkInside( lTileExtents, lModelExtentsV1 ) || checkInside( lTileExtents, lModelExtentsV2 ) )
    {
      _world->deleteModelInstance( it->second->d1 );
    }
  }

}
void MapTile::saveTile ( const World::model_instances_type::const_iterator& models_begin
                       , const World::model_instances_type::const_iterator& models_end
                       , const World::wmo_instances_type::const_iterator& wmos_begin
                       , const World::wmo_instances_type::const_iterator& wmos_end
                       )
{
  Log << "Saving ADT \"" << mFilename << "\"." << std::endl;

  int lID;  // This is a global counting variable. Do not store something in here you need later.

  // Collect some information we need later.

  // Check which doodads and WMOs are on this ADT.
  ::math::vector_3d lTileExtents[2];
  lTileExtents[0] = ::math::vector_3d( xbase, 0.0f, zbase );
  lTileExtents[1] = ::math::vector_3d( xbase + TILESIZE, 0.0f, zbase + TILESIZE );

  World::wmo_instances_type lObjectInstances;
  World::model_instances_type lModelInstances;

  //! \todo std::copy_if
  for ( World::wmo_instances_type::const_iterator it (wmos_begin)
      ; it != wmos_end
      ; ++it
      )
    //! \todo checkinside seems to fuck up everything
    //if( checkInside( lTileExtents, it->second->extents ) )
      lObjectInstances.insert( std::pair<int, WMOInstance*>( it->first, it->second ) );

  for( World::model_instances_type::const_iterator it (models_begin)
     ; it != models_end
     ; ++it
     )
  {
    ::math::vector_3d lModelExtentsV1[2], lModelExtentsV2[2];
    lModelExtentsV1[0] = it->second->model->header.BoundingBoxMin + it->second->pos;
    lModelExtentsV1[1] = it->second->model->header.BoundingBoxMax + it->second->pos;
    lModelExtentsV2[0] = it->second->model->header.VertexBoxMin + it->second->pos;
    lModelExtentsV2[1] = it->second->model->header.VertexBoxMax + it->second->pos;

    //! \todo checkinside seems to fuck up everything
    //if( checkInside( lTileExtents, lModelExtentsV1 ) || checkInside( lTileExtents, lModelExtentsV2 ) )
    //{
      lModelInstances.insert( std::pair<int, ModelInstance*>( it->first, it->second ) );
    //}
  }

  filenameOffsetThing nullyThing = { 0, 0 };

  std::map<std::string, filenameOffsetThing> lModels;

  for( std::map<int,ModelInstance*>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
  {
    //! \todo  Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
    std::string lTemp = it->second->model->_filename;
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

  for( std::map<int,WMOInstance *>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
    if( lObjects.find( it->second->wmo->_filename ) == lObjects.end() )
      lObjects.insert( std::pair<std::string, filenameOffsetThing>( ( it->second->wmo->_filename ), nullyThing ) );

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
      for( size_t tex = 0; tex < mChunks[i][j]->nTextures; tex++ )
        if( lTextures.find( mChunks[i][j]->_textures[tex]->filename().toStdString() ) == lTextures.end() )
          lTextures.insert( std::pair<std::string, int>( mChunks[i][j]->_textures[tex]->filename().toStdString(), -1 ) );

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

  std::vector<char> lADTFile;

  int lCurrentPosition = 0;

  // MVER
//  {
  lADTFile.resize (lADTFile.size() + 8 + 0x4);
    SetChunkHeader( lADTFile, lCurrentPosition, 'MVER', 4 );

    // MVER data
    *( get_pointer<int>( lADTFile, 8 ) ) = 18;

    lCurrentPosition += 8 + 0x4;
//  }

  // MHDR
  int lMHDR_Position = lCurrentPosition;
//  {
    lADTFile.resize (lADTFile.size() + 8 + 0x40 );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MHDR', 0x40 );

    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->flags = mFlags;

    lCurrentPosition += 8 + 0x40;
//  }

  // MCIN
  int lMCIN_Position = lCurrentPosition;
//  {
    lADTFile.resize (lADTFile.size() + 8 + 256 * 0x10 );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MCIN', 256 * 0x10 );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mcin = lCurrentPosition - 0x14;

    // MCIN * MCIN_Data = get_pointer<MCIN>( lADTFile, lMCIN_Position + 8 );

    lCurrentPosition += 8 + 256 * 0x10;
//  }

  // MTEX
//  {
    int lMTEX_Position = lCurrentPosition;
    lADTFile.resize (lADTFile.size() + 8 + 0 );  // We don't yet know how big this will be.
    SetChunkHeader( lADTFile, lCurrentPosition, 'MTEX' );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mtex = lCurrentPosition - 0x14;

    lCurrentPosition += 8 + 0;

    // MTEX data
    for( std::map<std::string, int>::iterator it = lTextures.begin(); it != lTextures.end(); ++it )
    {
      insert_string (lADTFile, lCurrentPosition, it->first);
      lCurrentPosition += it->first.size() + 1;
      get_pointer<sChunkHeader>( lADTFile, lMTEX_Position )->mSize += it->first.size() + 1;
      LogDebug << "Added texture \"" << it->first << "\"." << std::endl;
    }
//  }

  // MMDX
//  {
    int lMMDX_Position = lCurrentPosition;
    lADTFile.resize (lADTFile.size() + 8 + 0 );  // We don't yet know how big this will be.
    SetChunkHeader( lADTFile, lCurrentPosition, 'MMDX' );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mmdx = lCurrentPosition - 0x14;

    lCurrentPosition += 8 + 0;

    // MMDX data
    for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
    {
      it->second.filenamePosition = get_pointer<sChunkHeader>( lADTFile, lMMDX_Position )->mSize;
      insert_string (lADTFile, lCurrentPosition, it->first);
      lCurrentPosition += it->first.size() + 1;
      get_pointer<sChunkHeader>( lADTFile, lMMDX_Position )->mSize += it->first.size() + 1;
      LogDebug << "Added model \"" << it->first << "\"." << std::endl;
    }
//  }

  // MMID
//  {
    int lMMID_Size = 4 * lModels.size();
    lADTFile.resize (lADTFile.size() + 8 + lMMID_Size );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MMID', lMMID_Size );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mmid = lCurrentPosition - 0x14;

    // MMID data
    int * lMMID_Data = get_pointer<int>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
      lMMID_Data[lID++] = it->second.filenamePosition;

    lCurrentPosition += 8 + lMMID_Size;
//  }

  // MWMO
//  {
    int lMWMO_Position = lCurrentPosition;
    lADTFile.resize (lADTFile.size() + 8 + 0 );  // We don't yet know how big this will be.
    SetChunkHeader( lADTFile, lCurrentPosition, 'MWMO' );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mwmo = lCurrentPosition - 0x14;

    lCurrentPosition += 8 + 0;

    // MWMO data
    for( std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it )
    {
      it->second.filenamePosition = get_pointer<sChunkHeader>( lADTFile, lMWMO_Position )->mSize;
      insert_string (lADTFile, lCurrentPosition, it->first);
      lCurrentPosition += it->first.size() + 1;
      get_pointer<sChunkHeader>( lADTFile, lMWMO_Position )->mSize += it->first.size() + 1;
      LogDebug << "Added object \"" << it->first << "\"." << std::endl;
    }
//  }

  // MWID
//  {
    int lMWID_Size = 4 * lObjects.size();
    lADTFile.resize (lADTFile.size() + 8 + lMWID_Size );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MWID', lMWID_Size );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mwid = lCurrentPosition - 0x14;

    // MWID data
    int * lMWID_Data = get_pointer<int>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it )
      lMWID_Data[lID++] = it->second.filenamePosition;

    lCurrentPosition += 8 + lMWID_Size;
//  }

  // MDDF
//  {
    int lMDDF_Size = 0x24 * lModelInstances.size();
    lADTFile.resize (lADTFile.size() + 8 + lMDDF_Size );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MDDF', lMDDF_Size );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mddf = lCurrentPosition - 0x14;

    // MDDF data
    ENTRY_MDDF * lMDDF_Data = get_pointer<ENTRY_MDDF>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<int,ModelInstance*>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
    {
      //! \todo  Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
      std::string lTemp = it->second->model->_filename;
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
      lMDDF_Data[lID].uniqueID = it->first;
      lMDDF_Data[lID].pos[0] = it->second->pos.x();
      lMDDF_Data[lID].pos[1] = it->second->pos.y();
      lMDDF_Data[lID].pos[2] = it->second->pos.z();
      lMDDF_Data[lID].rot[0] = it->second->dir.x();
      lMDDF_Data[lID].rot[1] = it->second->dir.y();
      lMDDF_Data[lID].rot[2] = it->second->dir.z();
      lMDDF_Data[lID].scale = it->second->sc * 1024;
      lMDDF_Data[lID].flags = 0;
      lID++;
    }

    lCurrentPosition += 8 + lMDDF_Size;
//  }

  // MODF
//  {
    int lMODF_Size = 0x40 * lObjectInstances.size();
    lADTFile.resize (lADTFile.size() + 8 + lMODF_Size );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MODF', lMODF_Size );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->modf = lCurrentPosition - 0x14;

    // MODF data
    ENTRY_MODF * lMODF_Data = get_pointer<ENTRY_MODF>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<int,WMOInstance *>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
    {
      std::map<std::string, filenameOffsetThing>::iterator lMyFilenameThingey = lObjects.find( it->second->wmo->_filename );
      if( lMyFilenameThingey == lObjects.end() )
      {
        LogError << "There is a problem with saving the objects. We have an object that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
        return;
      }

      //! \todo Do not fuck things up via UIDs here! Stop calculating them. Or somewhere else. Or idk. This is shit. Pure shit.

      // XXZZTNNN
      //        1
      //     1000
      //    10000
      //  1000000

      //int lNewUID = lID + mPositionX * 1000000 + mPositionZ * 10000 + 2 * 1000;

      lMODF_Data[lID].nameID = lMyFilenameThingey->second.nameID;
      lMODF_Data[lID].uniqueID = it->first;
      lMODF_Data[lID].pos[0] = it->second->pos.x();
      lMODF_Data[lID].pos[1] = it->second->pos.y();
      lMODF_Data[lID].pos[2] = it->second->pos.z();
      lMODF_Data[lID].rot[0] = it->second->dir.x();
      lMODF_Data[lID].rot[1] = it->second->dir.y();
      lMODF_Data[lID].rot[2] = it->second->dir.z();
      //! \todo  Calculate them here or when rotating / moving? What is nicer? We should at least do it somewhere..
      lMODF_Data[lID].extents[0][0] = it->second->extents[0].x();
      lMODF_Data[lID].extents[0][1] = it->second->extents[0].y();
      lMODF_Data[lID].extents[0][2] = it->second->extents[0].z();
      lMODF_Data[lID].extents[1][0] = it->second->extents[1].x();
      lMODF_Data[lID].extents[1][1] = it->second->extents[1].y();
      lMODF_Data[lID].extents[1][2] = it->second->extents[1].z();
      lMODF_Data[lID].flags = it->second->mFlags;
      lMODF_Data[lID].doodadSet = it->second->doodadset;
      lMODF_Data[lID].nameSet = it->second->mNameset;
      lMODF_Data[lID].unknown = it->second->mUnknown;
      lID++;
    }

    lCurrentPosition += 8 + lMODF_Size;
//  }

#if 0
  //! \todo Move to correct position. Actually do it correctly.
  //MH2O
  if(false){
    int lMH2O_size = 256*sizeof(MH2O_Header);
    MH2O_Header lHeader[256];
    MH2O_Information lInfo[256];
    float heightMask[256][9][9];
    char depthMask[256][9][9];
    char lRender[256][8];
    char lMask[256][8];
    Liquid* lLiquids[256];
    //! \todo implement finding the correct liquids...
    //prev work for writing MH2O, setting offsets etc.
    for(int i=0; i< 256;++i){
        Liquid* tmpLiqu = lLiquids[i];//mLiquids[i];
        if(tmpLiqu && tmpLiqu->isNotEmpty()){
          MH2O_Tile tTile = tmpLiqu->getMH2OData();
          //! \todo implement more than just one layer...
          lHeader[i].nLayers  = 1;
          lHeader[i].ofsInformation = lMH2O_size;

          lMH2O_size += sizeof(MH2O_Information);
          lInfo[i].Flags = tTile.mFlags;
          lInfo[i].LiquidType = tTile.mLiquidType;
          lInfo[i].maxHeight = tTile.mMaximum;
          lInfo[i].minHeight = tTile.mMinimum;
          lInfo[i].width = tmpLiqu->getWidth();
          lInfo[i].height = tmpLiqu->getHeight();
          lInfo[i].xOffset = tmpLiqu->getXOffset();
          lInfo[i].yOffset = tmpLiqu->getYOffset();
          //LogDebug << "TileInfo "<< i << " " << j << " Width: "<<lInfo[i*16+j].width << " Height: "<<lInfo[i*16+j].height;

          //! put the data instead after all info?

          lInfo[i].ofsHeightMap = lMH2O_size;
          //raising size for the heightmask
          lMH2O_size += (lInfo[i].height+1)*(lInfo[i].width+1)*(sizeof(float)+sizeof(char));
          for(int w = lInfo[i].yOffset; w < lInfo[i].yOffset+lInfo[i].width + 1; ++w){
            for(int h = lInfo[i].xOffset; h < lInfo[i].xOffset+lInfo[i].height + 1; ++h){
              heightMask[i][w][h] =  tTile.mHeightmap[w][h];
              depthMask[i][w][h] = char(255*tTile.mDepth[w][h]);
            }
          }

          lInfo[i].ofsInfoMask = lMH2O_size;
          //raising size for the infomask
          lMH2O_size += lInfo[i].height*sizeof(char); //this is false?
          //! \todo check for flags
          lHeader[i].ofsRenderMask = lMH2O_size;
          lMH2O_size += 8*sizeof(char); //rendermask
          for(int w = 0; w < 8; ++w) {
            char tmp = 0;
            for(int h = 0; h < 8; ++h) {
              if(tTile.mRender[w][h]) {
                tmp |= 1 << h;
              }
            }
            lRender[i][w] = tmp;
          }
          int tc = 0;
          int shft = 0;
          char tmp = 0;
          for(int w = 0; w < lInfo[i].width; ++w){
            for(int h = 0; h < lInfo[i].height; ++h){
              tmp += 1 << shft;
              ++shft;
              if(shft == 8){
                lMask[i][tc++] = tmp;
                shft = 0;
                tmp = 0;
              }
            }
          }
          if(shft != 0)
            lMask[i][tc++] = tmp;
        }
        else{
          lHeader[i].nLayers  = 0;
          lHeader[i].ofsInformation = 0;
          lHeader[i].ofsRenderMask = 0;
        }
    }

    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mh2o = lCurrentPosition - 0x14;
    lADTFile.resize (lADTFile.size() + 8 + lMH2O_size);
    SetChunkHeader( lADTFile, lCurrentPosition, 'MH2O', lMH2O_size );

    for(int i=0; i<256; ++i){
      MH2O_Header * tmpHeader = get_pointer<MH2O_Header>(lADTFile, lCurrentPosition + 8 + i*sizeof(MH2O_Header));
        memcpy(tmpHeader, &lHeader[i], sizeof(MH2O_Header));
        if(tmpHeader->nLayers != 0){
          MH2O_Information* tmpInfo = get_pointer<MH2O_Information>(lADTFile, lCurrentPosition + 8 + tmpHeader->ofsInformation);
          memcpy(tmpInfo, &lInfo[i], sizeof(MH2O_Information));

          float * tmpHeight = get_pointer<float>(lADTFile, lCurrentPosition + 8 + tmpInfo->ofsHeightMap);
          char * tmpDepth = get_pointer<char>(lADTFile, lCurrentPosition + 8 + tmpInfo->ofsHeightMap + (tmpInfo->width+1)*(tmpInfo->height+1)*sizeof(float));
          int c = 0;
          for(int w = tmpInfo->yOffset; w < tmpInfo->yOffset+tmpInfo->width + 1; ++w){
            for(int h = tmpInfo->xOffset; h < tmpInfo->xOffset+tmpInfo->height + 1; ++h){
              tmpHeight[c] = heightMask[i][w][h];
              tmpDepth[c] = depthMask[i][w][h];
              ++c;
            }
          }
          char* tmpMask = get_pointer<char>(lADTFile, lCurrentPosition + 8 + tmpInfo->ofsInfoMask);
          char * tmpRender = get_pointer<char>(lADTFile, lCurrentPosition + 8 + tmpHeader->ofsRenderMask);
          for(int w = 0; w < 8; ++w){
              tmpRender[w] = lRender[i][w];
          }
          for(int h =0; h < tmpInfo->height; ++h){
            tmpMask[h] = lMask[i][h];
          }
        }
    }
    LogDebug << "Wrote MH2O!" << std::endl;
    lCurrentPosition += 8 + lMH2O_size;
  }
#endif

  // MCNK
//  {


    for( int y = 0; y < 16; ++y )
    {
      for( int x = 0; x < 16; ++x )
      {
        int lMCNK_Size = 0x80;
        int lMCNK_Position = lCurrentPosition;
        lADTFile.resize (lADTFile.size() + 8 + 0x80 );  // This is only the size of the header. More chunks will increase the size.
        SetChunkHeader( lADTFile, lCurrentPosition, 'MCNK', lMCNK_Size );
        get_pointer<MCIN>( lADTFile, lMCIN_Position + 8 )->mEntries[y*16+x].offset = lCurrentPosition;

        // MCNK data
        lADTFile.insert ( lADTFile.begin() + lCurrentPosition + 8
                        , reinterpret_cast<char*> (&(mChunks[y][x]->header))
                        , reinterpret_cast<char*> (&(mChunks[y][x]->header)) + 0x80
                        );
        MapChunkHeader * lMCNK_header = get_pointer<MapChunkHeader>( lADTFile, lCurrentPosition + 8 );

        lMCNK_header->flags = mChunks[y][x]->header.flags;
        lMCNK_header->holes = mChunks[y][x]->holes;
        lMCNK_header->areaid = mChunks[y][x]->header.areaid;

        lMCNK_header->nLayers = -1;
        lMCNK_header->nDoodadRefs = -1;
        lMCNK_header->ofsHeight = -1;
        lMCNK_header->ofsNormal = -1;
        lMCNK_header->ofsLayer = -1;
        lMCNK_header->ofsRefs = -1;
        lMCNK_header->ofsAlpha = -1;
        lMCNK_header->sizeAlpha = -1;
        lMCNK_header->ofsShadow = -1;
        lMCNK_header->sizeShadow = -1;
        lMCNK_header->nMapObjRefs = -1;

        //! \todo  Implement sound emitter support. Or not.
        lMCNK_header->ofsSndEmitters = 0;
        lMCNK_header->nSndEmitters = 0;

        lMCNK_header->ofsLiquid = 0;
        //! \todo Is this still 8 if no chunk is present? Or did they correct that?
        lMCNK_header->sizeLiquid = 8;

        //! \todo  MCCV sub-chunk
        lMCNK_header->ofsMCCV = 0;

        mChunks[y][x]->update_low_quality_texture_map();
        memcpy ( lMCNK_header->low_quality_texture_map
               , mChunks[y][x]->low_quality_texture_map()
               , sizeof (lMCNK_header->low_quality_texture_map)
               );

        if( lMCNK_header->flags & 0x40 )
          LogError << "Problem with saving: This ADT is said to have vertex shading but we don't write them yet. This might get you really fucked up results." << std::endl;
        lMCNK_header->flags = lMCNK_header->flags & ( ~0x40 );


        lCurrentPosition += 8 + 0x80;

        // MCVT
//        {
          int lMCVT_Size = ( 9 * 9 + 8 * 8 ) * 4;

          lADTFile.resize (lADTFile.size() + 8 + lMCVT_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCVT', lMCVT_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsHeight = lCurrentPosition - lMCNK_Position;

          float * lHeightmap = get_pointer<float>( lADTFile, lCurrentPosition + 8 );

          float lMedian = 0.0f;
          for( int i = 0; i < ( 9 * 9 + 8 * 8 ); ++i )
            lMedian = lMedian + mChunks[y][x]->mVertices[i].y();

          lMedian = lMedian / ( 9 * 9 + 8 * 8 );
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ypos = lMedian;

          for( int i = 0; i < ( 9 * 9 + 8 * 8 ); ++i )
            lHeightmap[i] = mChunks[y][x]->mVertices[i].y() - lMedian;

          lCurrentPosition += 8 + lMCVT_Size;
          lMCNK_Size += 8 + lMCVT_Size;
//        }

        // MCNR
//        {
          int lMCNR_Size = ( 9 * 9 + 8 * 8 ) * 3;

          lADTFile.resize (lADTFile.size() + 8 + lMCNR_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCNR', lMCNR_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsNormal = lCurrentPosition - lMCNK_Position;

          char* lNormals (get_pointer<char> (lADTFile, lCurrentPosition + 8));

          mChunks[y][x]->update_normal_vectors();
          for (size_t i (0); i < (9 * 9 + 8 * 8); ++i)
          {
            lNormals[i*3 + 0] = ::math::bounded_nearest<char>
                                  (-mChunks[y][x]->mNormals[i].z() * 127.0f);
            lNormals[i*3 + 1] = ::math::bounded_nearest<char>
                                  (-mChunks[y][x]->mNormals[i].x() * 127.0f);
            lNormals[i*3 + 2] = ::math::bounded_nearest<char>
                                  ( mChunks[y][x]->mNormals[i].y() * 127.0f);
          }

          lCurrentPosition += 8 + lMCNR_Size;
          lMCNK_Size += 8 + lMCNR_Size;
//        }

        // Unknown MCNR bytes
        // These are not in as we have data or something but just to make the files more blizzlike.
//        {
          lADTFile.resize (lADTFile.size() + 13 );
          lCurrentPosition += 13;
          lMCNK_Size += 13;
//        }

        // MCLY
//        {
          size_t lMCLY_Size = mChunks[y][x]->nTextures * 0x10;

          lADTFile.resize (lADTFile.size() + 8 + lMCLY_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCLY', lMCLY_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsLayer = lCurrentPosition - lMCNK_Position;
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->nLayers = mChunks[y][x]->nTextures;

          // MCLY data
          for( size_t j = 0; j < mChunks[y][x]->nTextures; ++j )
          {
            ENTRY_MCLY * lLayer = get_pointer<ENTRY_MCLY>( lADTFile, lCurrentPosition + 8 + 0x10 * j );

            lLayer->textureID = lTextures.find( mChunks[y][x]->_textures[j]->filename().toStdString() )->second;

            lLayer->flags = mChunks[y][x]->texture_flags (j);

            // if not first, have alpha layer, if first, have not. never have compression.
            lLayer->flags = ( j > 0 ? lLayer->flags | FLAG_USE_ALPHA : lLayer->flags & ( ~FLAG_USE_ALPHA ) ) & ( ~FLAG_ALPHA_COMPRESSED );

            lLayer->ofsAlpha = ( j == 0 ? 0 : ( mBigAlpha ? 64 * 64 * ( j - 1 ) : 32 * 64 * ( j - 1 ) ) );
            lLayer->effectID = mChunks[y][x]->texture_effect_id (j);
          }

          lCurrentPosition += 8 + lMCLY_Size;
          lMCNK_Size += 8 + lMCLY_Size;
//        }

        // MCRF
//        {
          std::list<int> lDoodadIDs;
          std::list<int> lObjectIDs;

          ::math::vector_3d lChunkExtents[2];
          lChunkExtents[0] = ::math::vector_3d( mChunks[y][x]->xbase, 0.0f, mChunks[y][x]->zbase );
          lChunkExtents[1] = ::math::vector_3d( mChunks[y][x]->xbase + CHUNKSIZE, 0.0f, mChunks[y][x]->zbase + CHUNKSIZE );

          // search all wmos that are inside this chunk
          lID = 0;
          for( std::map<int,WMOInstance *>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
          {
            //! \todo  This requires the extents already being calculated. See above.
            if( checkInside( lChunkExtents, it->second->extents ) )
              lObjectIDs.push_back( lID );
            lID++;
          }

          // search all models that are inside this chunk
          lID = 0;
          for( std::map<int, ModelInstance *>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
          {
            // get radius and position of the m2
            float radius = it->second->model->header.BoundingBoxRadius;
            ::math::vector_3d& pos = it->second->pos;

            // Calculate the chunk zenter
            ::math::vector_3d chunkMid(mChunks[y][x]->xbase + CHUNKSIZE / 2, 0,
            mChunks[y][x]->zbase + CHUNKSIZE / 2);

            // find out if the model is inside the reach of the chunk.
            float dx = chunkMid.x() - pos.x();
            float dz = chunkMid.z() - pos.z();
            float dist = sqrtf(dx * dx + dz * dz);
            static float sqrt2 = sqrtf(2.0f);

            if(dist - radius <= ((sqrt2 / 2.0f) * CHUNKSIZE))
            {
              lDoodadIDs.push_back(lID);
            }

            lID++;
          }

          int lMCRF_Size = 4 * ( lDoodadIDs.size() + lObjectIDs.size() );
          lADTFile.resize (lADTFile.size() + 8 + lMCRF_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCRF', lMCRF_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsRefs = lCurrentPosition - lMCNK_Position;
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->nDoodadRefs = lDoodadIDs.size();
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->nMapObjRefs = lObjectIDs.size();

          // MCRF data
          int * lReferences = get_pointer<int>( lADTFile, lCurrentPosition + 8 );

          lID = 0;
          for( std::list<int>::iterator it = lDoodadIDs.begin(); it != lDoodadIDs.end(); ++it )
          {
            lReferences[lID] = *it;
            lID++;
          }

          for( std::list<int>::iterator it = lObjectIDs.begin(); it != lObjectIDs.end(); ++it )
          {
            lReferences[lID] = *it;
            lID++;
          }

          lCurrentPosition += 8 + lMCRF_Size;
          lMCNK_Size += 8 + lMCRF_Size;
//        }

        // MCSH
//        {
          //! \todo  Somehow determine if we need to write this or not?
          //! \todo  This sometime gets all shadows black.
          if( mChunks[y][x]->header.flags & 1 )
          {
            int lMCSH_Size = 0x200;
            lADTFile.resize (lADTFile.size() + 8 + lMCSH_Size );
            SetChunkHeader( lADTFile, lCurrentPosition, 'MCSH', lMCSH_Size );

            get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsShadow = lCurrentPosition - lMCNK_Position;
            get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->sizeShadow = 0x200;

            char * lLayer = get_pointer<char>( lADTFile, lCurrentPosition + 8 );

            memcpy( lLayer, mChunks[y][x]->mShadowMap, 0x200 );

            lCurrentPosition += 8 + lMCSH_Size;
            lMCNK_Size += 8 + lMCSH_Size;
          }
          else
          {
            get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsShadow = 0;
            get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->sizeShadow = 0;
          }
//        }

        // MCAL
//        {
          int lDimensions = 64 * ( mBigAlpha ? 64 : 32 );

      size_t lMaps = mChunks[y][x]->nTextures ? mChunks[y][x]->nTextures - 1U : 0U;

          int lMCAL_Size = lDimensions * lMaps;

          lADTFile.resize (lADTFile.size() + 8 + lMCAL_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCAL', lMCAL_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsAlpha = lCurrentPosition - lMCNK_Position;
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->sizeAlpha = 8 + lMCAL_Size;

          char * lAlphaMaps = get_pointer<char>( lADTFile, lCurrentPosition + 8 );

          for( size_t j = 0; j < lMaps; j++ )
          {
            //First thing we have to do is downsample the alpha maps before we can write them
            if( mBigAlpha )
              for( int k = 0; k < lDimensions; k++ )
                lAlphaMaps[lDimensions * j + k] = mChunks[y][x]->amap[j][k];
            else
            {
              unsigned char upperNibble, lowerNibble;
              for( int k = 0; k < lDimensions; k++ )
              {
                lowerNibble = static_cast<unsigned char>(std::max(std::min( ( static_cast<float>(mChunks[y][x]->amap[j][k * 2 + 0]) ) * 0.05882f + 0.5f , 15.0f),0.0f));
                upperNibble = static_cast<unsigned char>(std::max(std::min( ( static_cast<float>(mChunks[y][x]->amap[j][k * 2 + 1]) ) * 0.05882f + 0.5f , 15.0f),0.0f));
                lAlphaMaps[lDimensions * j + k] = ( upperNibble << 4 ) + lowerNibble;
              }
            }
          }

          lCurrentPosition += 8 + lMCAL_Size;
          lMCNK_Size += 8 + lMCAL_Size;
//        }

        //! Don't write anything MCLQ related anymore...

        // MCSE
//        {
          int lMCSE_Size = 0;
          lADTFile.resize (lADTFile.size() + 8 + lMCSE_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCSE', lMCSE_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsSndEmitters = lCurrentPosition - lMCNK_Position;
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->nSndEmitters = lMCSE_Size / 0x1C;

          // if ( data ) do write

          /*
          if(sound_Exist){
          memcpy(&Temp,f.getBuffer()+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters+4,sizeof(int));
          memcpy(Buffer+Change+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters+lChange,f.getBuffer()+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters,Temp+8);
          ChunkHeader[i].ofsSndEmitters+=lChange;
          }
          */

          lCurrentPosition += 8 + lMCSE_Size;
          lMCNK_Size += 8 + lMCSE_Size;
//        }



          get_pointer<sChunkHeader>( lADTFile, lMCNK_Position )->mSize = lMCNK_Size;
          get_pointer<MCIN>( lADTFile, lMCIN_Position + 8 )->mEntries[y*16+x].size = lMCNK_Size;
      }
    }

//  }

  // MFBO
  if( mFlags & 1 )
  {
    size_t chunkSize = sizeof( int16_t ) * 9 * 2;
    lADTFile.resize (lADTFile.size() + 8 + chunkSize );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MFBO', chunkSize );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mfbo = lCurrentPosition - 0x14;

    int16_t* lMFBO_Data = get_pointer<int16_t>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    for( int i = 0; i < 9; ++i )
      lMFBO_Data[lID++] = mMinimumValues[i * 3 + 1];

    for( int i = 0; i < 9; ++i )
      lMFBO_Data[lID++] = mMaximumValues[i * 3 + 1];

    lCurrentPosition += 8 + chunkSize;
  }

  // \! todo Do not do bullshit here in MTFX.
#if 0
  if(!mTextureEffects.empty()) {
    //! \todo check if nTexEffects == nTextures, correct order etc.
    lADTFile.resize (lADTFile.size() + 8 + 4*mTextureEffects.size());
    SetChunkHeader( lADTFile, lCurrentPosition, 'MTFX', 4*mTextureEffects.size() );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mtfx = lCurrentPosition - 0x14;

    uint32_t* lMTFX_Data = get_pointer<uint32_t>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    //they should be in the correct order...
    for(std::vector<int>::iterator it = mTextureEffects.begin(); it!= mTextureEffects.end(); ++it) {
      lMTFX_Data[lID] = *it;
      ++lID;
    }
    lCurrentPosition += 8 + sizeof( uint32_t ) * mTextureEffects.size();
  }
#endif

  noggit::mpq::file f (QString::fromStdString (mFilename), true);
  f.setBuffer( get_pointer<char>(lADTFile), lADTFile.size() );
  f.save_to_disk();
  f.close();
}

void MapTile::saveTileCata ( const World::model_instances_type::const_iterator& models_begin
                           , const World::model_instances_type::const_iterator& models_end
                           , const World::wmo_instances_type::const_iterator& wmos_begin
                           , const World::wmo_instances_type::const_iterator& wmos_end
                           )
{
  Log << "Saving ADT (Cata) \"" << mFilename << "\"." << std::endl;

  int lID;  // This is a global counting variable. Do not store something in here you need later.

  // Collect some information we need later.

  // Check which doodads and WMOs are on this ADT.
  ::math::vector_3d lTileExtents[2];
  lTileExtents[0] = ::math::vector_3d( xbase, 0.0f, zbase );
  lTileExtents[1] = ::math::vector_3d( xbase + TILESIZE, 0.0f, zbase + TILESIZE );

  World::wmo_instances_type lObjectInstances;
  World::model_instances_type lModelInstances;

  //! \todo std::copy_if
  for ( World::wmo_instances_type::const_iterator it (wmos_begin)
      ; it != wmos_end
      ; ++it
      )
    //! \todo checkinside seems to fuck up everything
    //if( checkInside( lTileExtents, it->second->extents ) )
      lObjectInstances.insert( std::pair<int, WMOInstance*>( it->first, it->second ) );

  for( World::model_instances_type::const_iterator it (models_begin)
     ; it != models_end
     ; ++it
     )
  {
    ::math::vector_3d lModelExtentsV1[2], lModelExtentsV2[2];
    lModelExtentsV1[0] = it->second->model->header.BoundingBoxMin + it->second->pos;
    lModelExtentsV1[1] = it->second->model->header.BoundingBoxMax + it->second->pos;
    lModelExtentsV2[0] = it->second->model->header.VertexBoxMin + it->second->pos;
    lModelExtentsV2[1] = it->second->model->header.VertexBoxMax + it->second->pos;

    //! \todo checkinside seems to fuck up everything
    //if( checkInside( lTileExtents, lModelExtentsV1 ) || checkInside( lTileExtents, lModelExtentsV2 ) )
    //{
      lModelInstances.insert( std::pair<int, ModelInstance*>( it->first, it->second ) );
    //}
  }

  filenameOffsetThing nullyThing = { 0, 0 };

  std::map<std::string, filenameOffsetThing> lModels;

  for( std::map<int,ModelInstance*>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
  {
    std::string lTemp = it->second->model->_filename;

    if( lModels.find( lTemp ) == lModels.end() )
    {
      lModels.insert( std::pair<std::string, filenameOffsetThing>( lTemp, nullyThing ) );
    }
  }

  lID = 0;
  for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
    it->second.nameID = lID++;

  std::map<std::string, filenameOffsetThing> lObjects;

  for( std::map<int,WMOInstance *>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
    if( lObjects.find( it->second->wmo->_filename ) == lObjects.end() )
      lObjects.insert( std::pair<std::string, filenameOffsetThing>( ( it->second->wmo->_filename ), nullyThing ) );

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
      for( size_t tex = 0; tex < mChunks[i][j]->nTextures; tex++ )
        if( lTextures.find( mChunks[i][j]->_textures[tex]->filename().toStdString() ) == lTextures.end() )
          lTextures.insert( std::pair<std::string, int>( mChunks[i][j]->_textures[tex]->filename().toStdString(), -1 ) );

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

  // terrain

  std::vector<char> lADTFile;

  int lCurrentPosition = 0;

  // MVER
//  {
    lADTFile.resize (lADTFile.size() + 8 + 0x4 );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MVER', 4 );

    // MVER data
    *( get_pointer<int>( lADTFile, 8 ) ) = 18;

    lCurrentPosition += 8 + 0x4;
//  }

  // MHDR
  int lMHDR_Position = lCurrentPosition;
//  {
    lADTFile.resize (lADTFile.size() + 8 + 0x40 );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MHDR', 0x40 );

    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->flags = mFlags;

    lCurrentPosition += 8 + 0x40;
//  }

  // MH2O : not yet.

  // MCNK
//  {


    for( int y = 0; y < 16; ++y )
    {
      for( int x = 0; x < 16; ++x )
      {
        int lMCNK_Size = 0x80;
        int lMCNK_Position = lCurrentPosition;
        lADTFile.resize (lADTFile.size() + 8 );
        SetChunkHeader( lADTFile, lCurrentPosition, 'MCNK', lMCNK_Size );

        // MCNK data
        lADTFile.insert ( lADTFile.begin() + lCurrentPosition + 8
                        , reinterpret_cast<char*> (&(mChunks[y][x]->header))
                        , reinterpret_cast<char*> (&(mChunks[y][x]->header)) + 0x80
                        );
        // This is only the size of the header. More chunks will increase the size.
        MapChunkHeader * lMCNK_header = get_pointer<MapChunkHeader>( lADTFile, lCurrentPosition + 8 );

        lMCNK_header->flags = mChunks[y][x]->header.flags;
        lMCNK_header->holes = mChunks[y][x]->holes;
        lMCNK_header->areaid = mChunks[y][x]->header.areaid;

        lMCNK_header->nLayers = 0;
        lMCNK_header->nDoodadRefs = 0;
        lMCNK_header->ofsHeight = -1;
        lMCNK_header->ofsNormal = -1;
        lMCNK_header->ofsLayer = 0;
        lMCNK_header->ofsRefs = 0;
        lMCNK_header->ofsAlpha = 0;
        lMCNK_header->sizeAlpha = 0;
        lMCNK_header->ofsShadow = 0;
        lMCNK_header->sizeShadow = 0;
        lMCNK_header->nMapObjRefs = 0;

        //! \todo  Implement sound emitter support. Or not.
        lMCNK_header->ofsSndEmitters = 0;
        lMCNK_header->nSndEmitters = 0;

        lMCNK_header->ofsLiquid = 0;
        lMCNK_header->sizeLiquid = 0;

        //! \todo  MCCV sub-chunk
        lMCNK_header->ofsMCCV = 0;

        mChunks[y][x]->update_low_quality_texture_map();
        memcpy ( lMCNK_header->low_quality_texture_map
               , mChunks[y][x]->low_quality_texture_map()
               , sizeof (lMCNK_header->low_quality_texture_map)
               );

        if( lMCNK_header->flags & 0x40 )
          LogError << "Problem with saving: This ADT is said to have vertex shading but we don't write them yet. This might get you really fucked up results." << std::endl;
        lMCNK_header->flags = lMCNK_header->flags & ( ~0x40 );


        lCurrentPosition += 8 + 0x80;

        // MCVT
//        {
          int lMCVT_Size = ( 9 * 9 + 8 * 8 ) * 4;

          lADTFile.resize (lADTFile.size() + 8 + lMCVT_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCVT', lMCVT_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsHeight = lCurrentPosition - lMCNK_Position;

          float * lHeightmap = get_pointer<float>( lADTFile, lCurrentPosition + 8 );

          float lMedian = 0.0f;
          for( int i = 0; i < ( 9 * 9 + 8 * 8 ); ++i )
            lMedian = lMedian + mChunks[y][x]->mVertices[i].y();

          lMedian = lMedian / ( 9 * 9 + 8 * 8 );
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ypos = lMedian;

          for( int i = 0; i < ( 9 * 9 + 8 * 8 ); ++i )
            lHeightmap[i] = mChunks[y][x]->mVertices[i].y() - lMedian;

          lCurrentPosition += 8 + lMCVT_Size;
          lMCNK_Size += 8 + lMCVT_Size;
//        }

        // MCNR
//        {
          int lMCNR_Size = ( 9 * 9 + 8 * 8 ) * 3;

          lADTFile.resize (lADTFile.size() + 8 + lMCNR_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCNR', lMCNR_Size + 13 ); // For Cata we do add the 13 bytes too in chunk size

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsNormal = lCurrentPosition - lMCNK_Position;

          char* lNormals (get_pointer<char> (lADTFile, lCurrentPosition + 8));

          mChunks[y][x]->update_normal_vectors();
          for (size_t i (0); i < (9 * 9 + 8 * 8); ++i)
          {
            lNormals[i*3 + 0] = ::math::bounded_nearest<char>
                                  (-mChunks[y][x]->mNormals[i].z() * 127.0f);
            lNormals[i*3 + 1] = ::math::bounded_nearest<char>
                                  (-mChunks[y][x]->mNormals[i].x() * 127.0f);
            lNormals[i*3 + 2] = ::math::bounded_nearest<char>
                                  ( mChunks[y][x]->mNormals[i].y() * 127.0f);
          }

          lCurrentPosition += 8 + lMCNR_Size;
          lMCNK_Size += 8 + lMCNR_Size;
//        }

        // Unknown MCNR bytes
        // These are not in as we have data or something but just to make the files more blizzlike.
//        {
          lADTFile.resize (lADTFile.size() + 13 );
          lCurrentPosition += 13;
          lMCNK_Size += 13;
//        }

        // MCSE
//        {
          int lMCSE_Size = 0;
          lADTFile.resize (lADTFile.size() + 8 + lMCSE_Size );
          SetChunkHeader( lADTFile, lCurrentPosition, 'MCSE', lMCSE_Size );

          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsSndEmitters = lCurrentPosition - lMCNK_Position;
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->nSndEmitters = lMCSE_Size / 0x1C;

          // if ( data ) do write

          /*
          if(sound_Exist){
          memcpy(&Temp,f.getBuffer()+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters+4,sizeof(int));
          memcpy(Buffer+Change+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters+lChange,f.getBuffer()+MCINs[i].offset+ChunkHeader[i].ofsSndEmitters,Temp+8);
          ChunkHeader[i].ofsSndEmitters+=lChange;
          }
          */

          lCurrentPosition += 8 + lMCSE_Size;
          lMCNK_Size += 8 + lMCSE_Size;
//        }



          get_pointer<sChunkHeader>( lADTFile, lMCNK_Position )->mSize = lMCNK_Size;
          get_pointer<MapChunkHeader>( lADTFile, lMCNK_Position + 8 )->ofsLiquid = lMCNK_Size;
      }
    }

//  }

  // MFBO
  if( mFlags & 1 )
  {
    size_t chunkSize = sizeof( int16_t ) * 9 * 2;
    lADTFile.resize (lADTFile.size() + 8 + chunkSize );
    SetChunkHeader( lADTFile, lCurrentPosition, 'MFBO', chunkSize );
    get_pointer<MHDR>( lADTFile, lMHDR_Position + 8 )->mfbo = lCurrentPosition - 0x14;

    int16_t* lMFBO_Data = get_pointer<int16_t>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    for( int i = 0; i < 9; ++i )
      lMFBO_Data[lID++] = mMinimumValues[i * 3 + 1];

    for( int i = 0; i < 9; ++i )
      lMFBO_Data[lID++] = mMaximumValues[i * 3 + 1];

    lCurrentPosition += 8 + chunkSize;
  }

  noggit::mpq::file f (QString::fromStdString (mFilename), true);
  f.setBuffer( get_pointer<char>(lADTFile), lADTFile.size() );
  f.save_to_disk();
  f.close();

  // tex0

  std::vector<char> lADTTexFile;
  lCurrentPosition = 0;

  // MVER
//  {
    lADTTexFile.resize (lADTTexFile.size() + 8 + 0x4 );
    SetChunkHeader( lADTTexFile, lCurrentPosition, 'MVER', 4 );

    // MVER data
    *( get_pointer<int>( lADTTexFile, 8 ) ) = 18;

    lCurrentPosition += 8 + 0x4;
//  }

  // MAMP

    lADTTexFile.resize (lADTTexFile.size() + 8 + 0x4 );
    SetChunkHeader( lADTTexFile, lCurrentPosition, 'MAMP', 4 );

    // MAMP data (seems to be always 0 in tex0)
    *( get_pointer<int>( lADTTexFile, lCurrentPosition + 8 ) ) = 0;

    lCurrentPosition += 8 + 0x4;

  // MTEX
//  {
    int lMTEX_Position = lCurrentPosition;
    lADTTexFile.resize (lADTTexFile.size() + 8 + 0 );  // We don't yet know how big this will be.
    SetChunkHeader( lADTTexFile, lCurrentPosition, 'MTEX' );

    lCurrentPosition += 8 + 0;

    // MTEX data
    for( std::map<std::string, int>::iterator it = lTextures.begin(); it != lTextures.end(); ++it )
    {
      insert_string (lADTTexFile, lCurrentPosition, it->first);
      lCurrentPosition += it->first.size() + 1;
      get_pointer<sChunkHeader>( lADTTexFile, lMTEX_Position )->mSize += it->first.size() + 1;
      LogDebug << "Added texture \"" << it->first << "\"." << std::endl;
    }
//  }

  // MCNK
//  {

    for( int y = 0; y < 16; ++y )
    {
      for( int x = 0; x < 16; ++x )
      {
        int lMCNK_Size = 0x0;
        int lMCNK_Position = lCurrentPosition;

        lADTTexFile.resize (lADTTexFile.size() + 8 );  // No header in tex0
        SetChunkHeader( lADTTexFile, lCurrentPosition, 'MCNK', lMCNK_Size );
        lCurrentPosition += 8;

        // MCLY
//        {
          size_t lMCLY_Size = mChunks[y][x]->nTextures * 0x10;

          lADTTexFile.resize (lADTTexFile.size() + 8 + lMCLY_Size );
          SetChunkHeader( lADTTexFile, lCurrentPosition, 'MCLY', lMCLY_Size );

          // MCLY data
          for( size_t j = 0; j < mChunks[y][x]->nTextures; ++j )
          {
            ENTRY_MCLY * lLayer = get_pointer<ENTRY_MCLY>( lADTTexFile, lCurrentPosition + 8 + 0x10 * j );

            lLayer->textureID = lTextures.find( mChunks[y][x]->_textures[j]->filename().toStdString() )->second;

            lLayer->flags = mChunks[y][x]->texture_flags (j);

            // if not first, have alpha layer, if first, have not. never have compression.
            lLayer->flags = ( j > 0 ? lLayer->flags | FLAG_USE_ALPHA : lLayer->flags & ( ~FLAG_USE_ALPHA ) ) & ( ~FLAG_ALPHA_COMPRESSED );

            lLayer->ofsAlpha = ( j == 0 ? 0 : ( mBigAlpha ? 64 * 64 * ( j - 1 ) : 32 * 64 * ( j - 1 ) ) );
            lLayer->effectID = mChunks[y][x]->texture_effect_id (j);
          }

          lCurrentPosition += 8 + lMCLY_Size;
          lMCNK_Size += 8 + lMCLY_Size;
//        }

        // MCSH <-- untested, I (Mjo) need to test with an adt that has shadows.
//        {
          //! \todo  Somehow determine if we need to write this or not?
          //! \todo  This sometime gets all shadows black.
          if( mChunks[y][x]->header.flags & 1 )
          {
            int lMCSH_Size = 0x200;
            lADTTexFile.resize (lADTTexFile.size() + 8 + lMCSH_Size );
            SetChunkHeader( lADTFile, lCurrentPosition, 'MCSH', lMCSH_Size );

            char * lLayer = get_pointer<char>( lADTTexFile, lCurrentPosition + 8 );

            memcpy( lLayer, mChunks[y][x]->mShadowMap, 0x200 );

            lCurrentPosition += 8 + lMCSH_Size;
            lMCNK_Size += 8 + lMCSH_Size;
          }
//        }

        // MCAL
//        {
          int lDimensions = 64 * ( mBigAlpha ? 64 : 32 );

          size_t lMaps = mChunks[y][x]->nTextures ? mChunks[y][x]->nTextures - 1U : 0U;

          int lMCAL_Size = lDimensions * lMaps;

          lADTTexFile.resize (lADTTexFile.size() + 8 + lMCAL_Size );
          SetChunkHeader( lADTTexFile, lCurrentPosition, 'MCAL', lMCAL_Size );

          char * lAlphaMaps = get_pointer<char>( lADTTexFile, lCurrentPosition + 8 );

          for( size_t j = 0; j < lMaps; j++ )
          {
            //First thing we have to do is downsample the alpha maps before we can write them
            if( mBigAlpha )
              for( int k = 0; k < lDimensions; k++ )
                lAlphaMaps[lDimensions * j + k] = mChunks[y][x]->amap[j][k];
            else
            {
              unsigned char upperNibble, lowerNibble;
              for( int k = 0; k < lDimensions; k++ )
              {
                lowerNibble = static_cast<unsigned char>(std::max(std::min( ( static_cast<float>(mChunks[y][x]->amap[j][k * 2 + 0]) ) * 0.05882f + 0.5f , 15.0f),0.0f));
                upperNibble = static_cast<unsigned char>(std::max(std::min( ( static_cast<float>(mChunks[y][x]->amap[j][k * 2 + 1]) ) * 0.05882f + 0.5f , 15.0f),0.0f));
                lAlphaMaps[lDimensions * j + k] = ( upperNibble << 4 ) + lowerNibble;
              }
            }
          }

          lCurrentPosition += 8 + lMCAL_Size;
          lMCNK_Size += 8 + lMCAL_Size;
//        }

          get_pointer<sChunkHeader>( lADTTexFile, lMCNK_Position )->mSize = lMCNK_Size;
      }
    }

  // \! todo Do not do bullshit here in MTFX.
#if 0
  if(!mTextureEffects.empty()) {
    //! \todo check if nTexEffects == nTextures, correct order etc.
    lADTTexFile.resize (lADTTexFile.size() + 8 + 4*mTextureEffects.size());
    SetChunkHeader( lADTTexFile, lCurrentPosition, 'MTFX', 4*mTextureEffects.size() );

    uint32_t* lMTFX_Data = get_pointer<uint32_t>( lADTFile, lCurrentPosition + 8 );

    lID = 0;
    //they should be in the correct order...
    for(std::vector<int>::iterator it = mTextureEffects.begin(); it!= mTextureEffects.end(); ++it) {
      lMTFX_Data[lID] = *it;
      ++lID;
    }
    lCurrentPosition += 8 + sizeof( uint32_t ) * mTextureEffects.size();
  }
#endif

  std::string texFilename (mFilename);

  size_t found = texFilename.rfind( ".adt" );
  if( found != std::string::npos )
  {
    texFilename.replace( found, 4, "_tex" );
    texFilename.append( "0.adt" );
  }

  found = texFilename.find_last_of("\\");
  texFilename = texFilename.substr(found + 1);
  texFilename = "/" + texFilename;

  noggit::mpq::file fTex (QString::fromStdString (texFilename), true);
  fTex.setBuffer( get_pointer<char>(lADTTexFile), lADTTexFile.size() );
  fTex.save_to_disk();
  fTex.close();

  // obj0

  std::vector<char> lADTObjFile;
  lCurrentPosition = 0;

  // MVER
//  {
    lADTObjFile.resize (lADTObjFile.size() + 8 + 0x4 );
    SetChunkHeader( lADTObjFile, lCurrentPosition, 'MVER', 4 );

    // MVER data
    *( get_pointer<int>( lADTObjFile, 8 ) ) = 18;

    lCurrentPosition += 8 + 0x4;
//  }

  // MMDX
//  {
    int lMMDX_Position = lCurrentPosition;
    lADTObjFile.resize (lADTObjFile.size() + 8 + 0 );  // We don't yet know how big this will be.
    SetChunkHeader( lADTObjFile, lCurrentPosition, 'MMDX' );

    lCurrentPosition += 8 + 0;

    // MMDX data
    for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
    {
      it->second.filenamePosition = get_pointer<sChunkHeader>( lADTObjFile, lMMDX_Position )->mSize;
      insert_string (lADTObjFile, lCurrentPosition, it->first);
      lCurrentPosition += it->first.size() + 1;
      get_pointer<sChunkHeader>( lADTObjFile, lMMDX_Position )->mSize += it->first.size() + 1;
      LogDebug << "Added model \"" << it->first << "\"." << std::endl;
    }
//  }

  // MMID
//  {
    int lMMID_Size = 4 * lModels.size();
    lADTObjFile.resize (lADTObjFile.size() + 8 + lMMID_Size );
    SetChunkHeader( lADTObjFile, lCurrentPosition, 'MMID', lMMID_Size );

    // MMID data
    int * lMMID_Data = get_pointer<int>( lADTObjFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<std::string, filenameOffsetThing>::iterator it = lModels.begin(); it != lModels.end(); ++it )
      lMMID_Data[lID++] = it->second.filenamePosition;

    lCurrentPosition += 8 + lMMID_Size;
//  }

  // MWMO
//  {
    int lMWMO_Position = lCurrentPosition;
    lADTObjFile.resize (lADTObjFile.size() + 8 + 0 );  // We don't yet know how big this will be.
    SetChunkHeader( lADTObjFile, lCurrentPosition, 'MWMO' );

    lCurrentPosition += 8 + 0;

    // MWMO data
    for( std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it )
    {
      it->second.filenamePosition = get_pointer<sChunkHeader>( lADTObjFile, lMWMO_Position )->mSize;
      insert_string (lADTObjFile, lCurrentPosition, it->first);
      lCurrentPosition += it->first.size() + 1;
      get_pointer<sChunkHeader>( lADTObjFile, lMWMO_Position )->mSize += it->first.size() + 1;
      LogDebug << "Added object \"" << it->first << "\"." << std::endl;
    }
//  }

  // MWID
//  {
    int lMWID_Size = 4 * lObjects.size();
    lADTObjFile.resize (lADTObjFile.size() + 8 + lMWID_Size );
    SetChunkHeader( lADTObjFile, lCurrentPosition, 'MWID', lMWID_Size );

    // MWID data
    int * lMWID_Data = get_pointer<int>( lADTObjFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<std::string, filenameOffsetThing>::iterator it = lObjects.begin(); it != lObjects.end(); ++it )
      lMWID_Data[lID++] = it->second.filenamePosition;

    lCurrentPosition += 8 + lMWID_Size;
//  }

  // MDDF
//  {
    int lMDDF_Size = 0x24 * lModelInstances.size();
    lADTObjFile.resize (lADTObjFile.size() + 8 + lMDDF_Size );
    SetChunkHeader( lADTObjFile, lCurrentPosition, 'MDDF', lMDDF_Size );

    // MDDF data
    ENTRY_MDDF * lMDDF_Data = get_pointer<ENTRY_MDDF>( lADTObjFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<int,ModelInstance*>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
    {
      std::string lTemp = it->second->model->_filename;
      std::map<std::string, filenameOffsetThing>::iterator lMyFilenameThingey = lModels.find( lTemp );
      if( lMyFilenameThingey == lModels.end() )
      {
        LogError << "There is a problem with saving the doodads. We have a doodad that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
        return;
      }

      lMDDF_Data[lID].nameID = lMyFilenameThingey->second.nameID;
      lMDDF_Data[lID].uniqueID = it->first;
      lMDDF_Data[lID].pos[0] = it->second->pos.x();
      lMDDF_Data[lID].pos[1] = it->second->pos.y();
      lMDDF_Data[lID].pos[2] = it->second->pos.z();
      lMDDF_Data[lID].rot[0] = it->second->dir.x();
      lMDDF_Data[lID].rot[1] = it->second->dir.y();
      lMDDF_Data[lID].rot[2] = it->second->dir.z();
      lMDDF_Data[lID].scale = it->second->sc * 1024;
      lMDDF_Data[lID].flags = 0;
      lID++;
    }

    lCurrentPosition += 8 + lMDDF_Size;
//  }

  // MODF
//  {
    int lMODF_Size = 0x40 * lObjectInstances.size();
    lADTObjFile.resize (lADTObjFile.size() + 8 + lMODF_Size );
    SetChunkHeader( lADTObjFile, lCurrentPosition, 'MODF', lMODF_Size );
    get_pointer<MHDR>( lADTObjFile, lMHDR_Position + 8 )->modf = lCurrentPosition - 0x14;

    // MODF data
    ENTRY_MODF * lMODF_Data = get_pointer<ENTRY_MODF>( lADTObjFile, lCurrentPosition + 8 );

    lID = 0;
    for( std::map<int,WMOInstance *>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
    {
      std::map<std::string, filenameOffsetThing>::iterator lMyFilenameThingey = lObjects.find( it->second->wmo->_filename );
      if( lMyFilenameThingey == lObjects.end() )
      {
        LogError << "There is a problem with saving the objects. We have an object that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
        return;
      }

      lMODF_Data[lID].nameID = lMyFilenameThingey->second.nameID;
      lMODF_Data[lID].uniqueID = it->first;
      lMODF_Data[lID].pos[0] = it->second->pos.x();
      lMODF_Data[lID].pos[1] = it->second->pos.y();
      lMODF_Data[lID].pos[2] = it->second->pos.z();
      lMODF_Data[lID].rot[0] = it->second->dir.x();
      lMODF_Data[lID].rot[1] = it->second->dir.y();
      lMODF_Data[lID].rot[2] = it->second->dir.z();
      //! \todo  Calculate them here or when rotating / moving? What is nicer? We should at least do it somewhere..
      lMODF_Data[lID].extents[0][0] = it->second->extents[0].x();
      lMODF_Data[lID].extents[0][1] = it->second->extents[0].y();
      lMODF_Data[lID].extents[0][2] = it->second->extents[0].z();
      lMODF_Data[lID].extents[1][0] = it->second->extents[1].x();
      lMODF_Data[lID].extents[1][1] = it->second->extents[1].y();
      lMODF_Data[lID].extents[1][2] = it->second->extents[1].z();
      lMODF_Data[lID].flags = it->second->mFlags;
      lMODF_Data[lID].doodadSet = it->second->doodadset;
      lMODF_Data[lID].nameSet = it->second->mNameset;
      lMODF_Data[lID].unknown = it->second->mUnknown;
      lID++;
    }

    lCurrentPosition += 8 + lMODF_Size;
//  }

  // MCNK
//  {

    for( int y = 0; y < 16; ++y )
    {
      for( int x = 0; x < 16; ++x )
      {

        int lMCNK_Size = 0x0;
        int lMCNK_Position = lCurrentPosition;

        lADTObjFile.resize (lADTObjFile.size() + 8 );  // No header in tex0
        SetChunkHeader( lADTObjFile, lCurrentPosition, 'MCNK', lMCNK_Size );
        lCurrentPosition += 8;

        // MCRD & MCRW
//        {
          std::list<int> lDoodadIDs;
          std::list<int> lObjectIDs;

          ::math::vector_3d lChunkExtents[2];
          lChunkExtents[0] = ::math::vector_3d( mChunks[y][x]->xbase, 0.0f, mChunks[y][x]->zbase );
          lChunkExtents[1] = ::math::vector_3d( mChunks[y][x]->xbase + CHUNKSIZE, 0.0f, mChunks[y][x]->zbase + CHUNKSIZE );

          // search all wmos that are inside this chunk
          lID = 0;
          for( std::map<int,WMOInstance *>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
          {
            //! \todo  This requires the extents already being calculated. See above.
            if( checkInside( lChunkExtents, it->second->extents ) )
              lObjectIDs.push_back( lID );
            lID++;
          }

          // search all models that are inside this chunk
          lID = 0;
          for( std::map<int, ModelInstance *>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
          {
            // get radius and position of the m2
            float radius = it->second->model->header.BoundingBoxRadius;
            ::math::vector_3d& pos = it->second->pos;

            // Calculate the chunk center
            ::math::vector_3d chunkMid(mChunks[y][x]->xbase + CHUNKSIZE / 2, 0,
            mChunks[y][x]->zbase + CHUNKSIZE / 2);

            // find out if the model is inside the reach of the chunk.
            float dx = chunkMid.x() - pos.x();
            float dz = chunkMid.z() - pos.z();
            float dist = sqrtf(dx * dx + dz * dz);
            static float sqrt2 = sqrtf(2.0f);

            if(dist - radius <= ((sqrt2 / 2.0f) * CHUNKSIZE))
            {
              lDoodadIDs.push_back(lID);
            }

            lID++;
          }

      // MCRD data

          if ( lDoodadIDs.size() > 0 )
          {
            int lMCRD_Size = 4 * ( lDoodadIDs.size() );

            lADTObjFile.resize (lADTObjFile.size() + 8 + lMCRD_Size );
            SetChunkHeader( lADTObjFile, lCurrentPosition, 'MCRD', lMCRD_Size );

            int * lReferencesDoodads = get_pointer<int>( lADTObjFile, lCurrentPosition + 8 );

            lID = 0;
            for( std::list<int>::iterator it = lDoodadIDs.begin(); it != lDoodadIDs.end(); ++it )
            {
              lReferencesDoodads[lID] = *it;
              lID++;
            }

            lCurrentPosition += 8 + lMCRD_Size;
            lMCNK_Size += 8 + lMCRD_Size;
          }

          // MCRW data

          if ( lObjectIDs.size() > 0 )
          {
            int lMCRW_Size = 4 * ( lObjectIDs.size() );

            lADTObjFile.resize (lADTObjFile.size() + 8 + lMCRW_Size );
            SetChunkHeader( lADTObjFile, lCurrentPosition, 'MCRW', lMCRW_Size );

            int * lReferencesWmo = get_pointer<int>( lADTObjFile, lCurrentPosition + 8 );

            lID = 0;
            for( std::list<int>::iterator it = lObjectIDs.begin(); it != lObjectIDs.end(); ++it )
            {
              lReferencesWmo[lID] = *it;
              lID++;
            }

            lCurrentPosition += 8 + lMCRW_Size;
            lMCNK_Size += 8 + lMCRW_Size;
          }
//        }

          get_pointer<sChunkHeader>( lADTObjFile, lMCNK_Position )->mSize = lMCNK_Size;
      }
    }

  std::string objFilename (mFilename);

  found = objFilename.rfind( ".adt" );
  if( found != std::string::npos )
  {
    objFilename.replace( found, 4, "_obj" );
    objFilename.append( "0.adt" );
  }

  found = objFilename.find_last_of("\\");
  objFilename = objFilename.substr(found + 1);
  objFilename = "/" + objFilename;

  noggit::mpq::file fObj (QString::fromStdString (objFilename), true);
  fObj.setBuffer( get_pointer<char>(lADTObjFile), lADTObjFile.size() );
  fObj.save_to_disk();
  fObj.close();

}
