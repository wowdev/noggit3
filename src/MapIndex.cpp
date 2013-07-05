#include "MapIndex.h"

#include "MPQ.h"
#include "MapTile.h"
#include "Project.h"
#include "Misc.h"
#include "World.h"
#include "MapChunk.h"

MapIndex::MapIndex(const std::string &pBasename)
  : mHasAGlobalWMO(false)
  , mBigAlpha(false)
  , noadt(false)
  , cx(-1)
  , cz(-1)
  , basename(pBasename)
{
  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdt";

  MPQFile theFile(filename.str());
  uint32_t fourcc;
  //uint32_t size;

  // - MVER ----------------------------------------------

  uint32_t version;

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );
  theFile.read( &version, 4 );

  //! \todo find the correct version of WDT files.
  assert( fourcc == 'MVER' && version == 18 );

  // - MHDR ----------------------------------------------

  uint32_t flags;

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );

  assert( fourcc == 'MPHD' );

  theFile.read( &flags, 4 );
  theFile.seekRelative( 4 * 7 );

  mHasAGlobalWMO = flags & 1;
  mBigAlpha = flags & 4;

  // - MAIN ----------------------------------------------

  theFile.read( &fourcc, 4 );
  theFile.seekRelative( 4 );

  assert( fourcc == 'MAIN' );

  /// this is the theory. Sadly, we are also compiling on 64 bit machines with size_t being 8 byte, not 4. Therefore, we can't do the same thing, Blizzard does in its 32bit executable.
  //theFile.read( &(mTiles[0][0]), sizeof( 8 * 64 * 64 ) );

  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      theFile.read( &mTiles[j][i].flags, 4 );
      theFile.seekRelative( 4 );
      mTiles[j][i].tile = NULL;
      std::stringstream ADTfilename;
      // exist file on disc?
      ADTfilename << Project::getInstance()->getPath() << "World\\Maps\\" << basename << "\\" << basename << "_" << i << "_" << j << ".adt";

      std::ifstream myFile(ADTfilename.str().c_str());

      if(myFile)
      {
        mTiles[j][i].onDisc = true;
        myFile.close();
      }
      else mTiles[j][i].onDisc = false;

      //LogDebug << "Look at : " << ADTfilename.str() << " is on disc:" << mTiles[j][i].onDisc << std::endl;

    }
  }

  if( !theFile.isEof() )
  {
    //! \note We actually don't load WMO only worlds, so we just stop reading here, k?
    //! \bug MODF reads wrong. The assertion fails every time. Somehow, it keeps being MWMO. Or are there two blocks?

    mHasAGlobalWMO = false;

#ifdef __ASSERTIONBUGFIXED

    // - MWMO ----------------------------------------------

    theFile.read( &fourcc, 4 );
    theFile.read( &size, 4 );

    assert( fourcc == 'MWMO' );

    char * wmoFilenameBuf = new char[size];
    theFile.read( &wmoFilenameBuf, size );

    mWmoFilename = wmoFilenameBuf;

    free(wmoFilenameBuf);

    // - MODF ----------------------------------------------

    theFile.read( &fourcc, 4 );
    theFile.seekRelative( 4 );

    assert( fourcc == 'MODF' );

    theFile.read( &mWmoEntry, sizeof( ENTRY_MODF ) );

#endif //__ASSERTIONBUGFIXED

  }

  // -----------------------------------------------------

  theFile.close();
}

MapIndex::~MapIndex()
{
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        delete mTiles[j][i].tile;
        mTiles[j][i].tile = NULL;
      }
    }
  }
}

void MapIndex::enterTile( int x, int z )
{
  if( !hasTile( z, x ) )
  {
    noadt = true;
    return;
  }

  noadt = false;

  cx = x;
  cz = z;
  for( int i = std::max(cz - 2, 0); i < std::min(cz + 2, 64); ++i )
  {
    for( int j = std::max(cx - 2, 0); j < std::min(cx + 2, 64); ++j )
    {
      mTiles[i][j].tile = loadTile( i, j );
    }
  }

  if( autoheight && tileLoaded( cz, cx ) ) //ZX STEFF HERE SWAP!
  {
    float maxHeight = mTiles[cz][cx].tile->getMaxHeight();
    maxHeight = std::max( maxHeight, 0.0f );
    gWorld->camera.y = maxHeight + 50.0f;

    autoheight = false;
  }
}

void MapIndex::setChanged(float x, float z)
{
  // change the changed flag of the map tile
  int row =  misc::FtoIround((x-(TILESIZE/2))/TILESIZE);
  int column =  misc::FtoIround((z-(TILESIZE/2))/TILESIZE);
  if( row >= 0 && row <= 64 && column >= 0 && column <= 64 )
    if( mTiles[column][row].tile )
      this->setChanged(column, row);
}

void MapIndex::setChanged(int x, int z)
{
  // change the changed flag of the map tile
  if( !mTiles[x][z].tile ) return;
  if( mTiles[x][z].tile->changed == 1) return;

  mTiles[x][z].tile->changed = 1;

  for (int posaddx=-1; posaddx<2; posaddx++)
  {
    for (int posaddz=-1; posaddz<2; posaddz++)
    {
      if(!(posaddx==0 && posaddz==0))// exclude center ADT
      {
        if(hasTile( x+posaddx,z+posaddz ))
        {
          if( !mTiles[x+posaddx][z+posaddz].tile  )
          {
            mTiles[x+posaddx][z+posaddz].tile = loadTile( x+posaddx, z+posaddz );
            mTiles[x+posaddx][z+posaddz].tile->changed = 2;
          }
          else if( mTiles[x+posaddx][z+posaddz].tile->changed != 1 )
          {
            mTiles[x+posaddx][z+posaddz].tile->changed = 2;
          }
        }
      }
    }
  }

  int px;
  int pz;
  // mark surrounding as 2
  for(px=0;px==2;px++)
  {
    LogDebug << "X: " << px << std::endl;
    for(pz=0;pz==2;pz++)
    {
      LogDebug << "Z: " << pz << std::endl;
      if(px!=1 && pz!=1)// exclude center ADT
      {
        LogDebug << "Tile not Center: " << x+px << "_" << z+pz << std::endl;
        if( mTiles[x+px-1][z+pz-1].tile )
        {
          LogDebug << "Tile exist: " << x+px << "_" << z+pz << std::endl;
          if(  mTiles[x+px-1][z+pz-1].tile->changed!=1 )
          {
            LogDebug << "MarkSave 2: " << x+px << "_" << z+pz << std::endl;
            mTiles[x+px-1][z+pz-1].tile->changed = 2;
          }
        }
      }
    }
  }


}

void MapIndex::unsetChanged(int x, int z)
{
  // change the changed flag of the map tile
  if( mTiles[x][z].tile )
    mTiles[x][z].tile->changed = 0;
}

int MapIndex::getChanged(int x, int z)
{
  if(mTiles[x][z].tile && mTiles[x][z].tile->changed==1) // why do we need to save tile with changed=2? What "2" means?
    return mTiles[x][z].tile->changed;
  else return 0;
}

void MapIndex::setFlag(bool to, float x, float z)
{
  // set the inpass flag to selected chunk
  this->setChanged(x, z);
  const int newX = x / TILESIZE;
  const int newZ = z / TILESIZE;

  for( int j = newZ - 1; j < newZ + 1; ++j )
  {
    for( int i = newX - 1; i < newX + 1; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ++ty )
        {
          for( int tx = 0; tx < 16; ++tx )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              chunk->setFlag(to);
            }
          }
        }
      }
    }
  }
}

MapTile* MapIndex::loadTile(int z, int x)
{
  if( !hasTile( z, x ) )
  {
    return NULL;
  }

  if( tileLoaded( z, x ) )
  {
    return mTiles[z][x].tile;
  }

  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";

  if( !MPQFile::exists( filename.str() ) )
  {
    LogError << "The requested tile \"" << filename.str() << "\" does not exist! Oo" << std::endl;
    return NULL;
  }

  mTiles[z][x].tile = new MapTile( x, z, filename.str(), mBigAlpha );// XZ STEFF Swap MapTile( z, x, file
  return mTiles[z][x].tile;
}

void MapIndex::reloadTile(int x, int z)
{
  if( tileLoaded( z, x ) )
  {
    delete mTiles[z][x].tile;
    mTiles[z][x].tile = NULL;

    std::stringstream filename;
    filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";

    mTiles[z][x].tile = new MapTile( x, z, filename.str(), mBigAlpha );
    enterTile( cx, cz );
  }
}

void MapIndex::markOnDisc(int x, int z, bool mto)
{
  mTiles[z][x].onDisc = mto;
}

bool MapIndex::isTileExternal(int x, int z)
{
  // is onDisc
  return mTiles[z][x].onDisc;
}

void MapIndex::saveTile(int x, int z)
{
  // save goven tile
  if( tileLoaded( z, x ) )
  {
    mTiles[z][x].tile->saveTile();
  }
}

void MapIndex::saveChanged()
{

  // First recalculated UIDs.
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        if(this->getChanged(j,i) == 1)
        {
          mTiles[j][i].tile->uidTile();
        }
      }
    }
  }

  // Now save all marked as 1 and 2 because UIDs now fits.
  for( int j = 0; j < 64; ++j )
  {
    for( int i = 0; i < 64; ++i )
    {
      if( tileLoaded( j, i ) )
      {
        if(this->getChanged(j,i) > 0)
        {
          mTiles[j][i].tile->saveTile();
          this->unsetChanged(j,i);
        }
      }
    }
  }

}

bool MapIndex::hasAGlobalWMO()
{
  return mHasAGlobalWMO;
}

bool MapIndex::oktile(int z, int x)
{
  return !(z < 0 || x < 0 || z > 64 || x > 64);
}

bool MapIndex::hasTile(int pZ, int pX)
{
  return oktile(pZ, pX) && (mTiles[pZ][pX].flags & 1);
}

bool MapIndex::tileLoaded(int z, int x)
{
  return hasTile(z, x) && mTiles[z][x].tile;
}

bool MapIndex::hasAdt()
{
  return noadt;
}

void MapIndex::setAdt(bool value)
{
  noadt = value;
}

MapTile* MapIndex::getTile(size_t z, size_t x)
{
  return mTiles[z][x].tile;
}

uint32_t MapIndex::getFlag(size_t z, size_t x)
{
  return mTiles[z][x].flags;
}
