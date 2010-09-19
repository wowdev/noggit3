#include <cassert>
#include <algorithm>
#include <string>

#include "MapTile.h"
#include "MapChunk.h"
#include "Log.h"
#include "world.h"
#include "ModelInstance.h" // ModelInstance
#include "WMOInstance.h" // WMOInstance
#include "liquid.h"
#include "noggit.h" // TimerStop, TimerStart

void renderCylinder(float x1, float y1, float z1, float x2,float y2, float z2, float radius,int subdivisions,GLUquadricObj *quadric)
{
  float vx = x2-x1;
  float vy = y2-y1;
  float vz = z2-z1;

  //handle the degenerate case of z1 == z2 with an approximation
  if( vz == 0.0f )
      vz = .0001f;

  float v = sqrt( vx*vx + vy*vy + vz*vz );
  float ax = 57.2957795f*acos( vz/v );
  if ( vz < 0.0f )
      ax = -ax;
  float rx = -vy*vz;
  float ry = vx*vz;
  glPushMatrix();

  //draw the cylinder body
  glTranslatef( x1,y1,z1 );
  glRotatef(ax, rx, ry, 0.0);
  gluQuadricOrientation(quadric,GLU_OUTSIDE);
  gluCylinder(quadric, radius, radius, v, subdivisions, 1);

  glPopMatrix();
}
void renderCylinder_convenient(float x, float y, float z, float radius,int subdivisions)
{
  //the same quadric can be re-used for drawing many cylinders
  GLUquadricObj *quadric=gluNewQuadric();
  gluQuadricNormals(quadric, GLU_SMOOTH);
  renderCylinder(x,y-10,z,x,y+10,z,radius,subdivisions,quadric);
  gluDeleteQuadric(quadric);
}

int indexMapBuf(int x, int y)
{
	return ((y+1)/2)*9 + (y/2)*8 + x;
}

void startTimer();
void stopTimer();
int stopTimer2();

MapTile::MapTile( int pX, int pZ, const std::string& pFilename, bool pBigAlpha )
{
  mPositionX = pX;
  mPositionZ = pZ;
  
	xbase = mPositionX * TILESIZE;
	zbase = mPositionZ * TILESIZE;

  mBigAlpha = pBigAlpha;
  
  for( int i = 0; i < 16; i++ )
  {
    for( int j = 0; j < 16; j++ )
    {
      mChunks[i][j] = NULL;
    }
  }
  
  mFilename = pFilename;

	MPQFile theFile( mFilename );

  Log << "Opening tile " << mPositionX << ", " << mPositionZ << " (\"" << mFilename << "\") from " << (theFile.isExternal( ) ? "disk" : "MPQ") << "." << std::endl;
  
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
	
	// - MCIN ----------------------------------------------
	
	theFile.seek( Header.mcin + 0x14 );
	theFile.read( &fourcc, 4 );
	theFile.seekRelative( 4 );
	
	assert( fourcc == 'MCIN' );
	
	for( int i = 0; i < 256; i++ ) 
	{
		theFile.read( &lMCNKOffsets[i], 4 );
		theFile.seekRelative( 0xC );
	}
	
	// - MTEX ----------------------------------------------
	
	theFile.seek( Header.mtex + 0x14 );
	theFile.read( &fourcc, 4 );
	theFile.read( &size, 4 );
	
	assert( fourcc == 'MTEX' );
	
	if( size )
	{
		char * lBuffer = new char[size];
		theFile.read( lBuffer, size );
		
		unsigned int lPosition = 0;
		while( lPosition < size )
		{
			mTextureFilenames.push_back( std::string( lBuffer + lPosition ) );
			lPosition += strlen( lBuffer + lPosition ) + 1;
		}
		
		delete[] lBuffer;
	}
	
	// - MMDX ----------------------------------------------
	
	theFile.seek( Header.mmdx + 0x14 );
	theFile.read( &fourcc, 4 );
	theFile.read( &size, 4 );
	
	assert( fourcc == 'MMDX' );
	
	if( size )
	{
		char * lBuffer = new char[size];
		theFile.read( lBuffer, size );
		
		unsigned int lPosition = 0;
		while( lPosition < size )
		{
			mModelFilenames.push_back( std::string( lBuffer + lPosition ) );
			lPosition += strlen( lBuffer + lPosition ) + 1;
		}
		
		delete[] lBuffer;
	}
	
	// - MWMO ----------------------------------------------
	
	theFile.seek( Header.mwmo + 0x14 );
	theFile.read( &fourcc, 4 );
	theFile.read( &size, 4 );
	
	assert( fourcc == 'MWMO' );
	
	if( size )
	{
		char * lBuffer = new char[size];
		theFile.read( lBuffer, size );
		
		unsigned int lPosition = 0;
		while( lPosition < size )
		{
			mWMOFilenames.push_back( std::string( lBuffer + lPosition ) );
			lPosition += strlen( lBuffer + lPosition ) + 1;
		}
		
		delete[] lBuffer;
	}
	
	// - MDDF ----------------------------------------------
	
	theFile.seek( Header.mddf + 0x14 );
	theFile.read( &fourcc, 4 );
	theFile.read( &size, 4 );
	
	assert( fourcc == 'MDDF' );
	
  ENTRY_MDDF* mddf_ptr = reinterpret_cast<ENTRY_MDDF*>( theFile.getPointer() );
  for( unsigned int i = 0; i < size / 36; i++ )
  {
    lModelInstances.push_back( mddf_ptr[i] );
  }
	
	// - MODF ----------------------------------------------
	
	theFile.seek( Header.modf + 0x14 );
	theFile.read( &fourcc, 4 );
	theFile.read( &size, 4 );
	
	assert( fourcc == 'MODF' );
	
  ENTRY_MODF* modf_ptr = reinterpret_cast<ENTRY_MODF*>( theFile.getPointer() );
  for( unsigned int i = 0; i < size / 64; i++ )
  {
    lWMOInstances.push_back( modf_ptr[i] );
  }
	
	// - MISC ----------------------------------------------
	
	//! \todo  Parse all chunks in the new style!

	while( !theFile.isEof() ) 
	{
		theFile.read( &fourcc, 4 );
		theFile.read( &size, 4 );
		
		size_t nextpos = theFile.getPos() + size;

		if ( fourcc == 'MH2O' && false ) // Do not even try to render water, it will be a fuckup anyway. ._. 
		{
			// water data
			uint8_t * lMH2O_Chunk = theFile.getPointer( );

			MH2O_Header * lHeader = reinterpret_cast<MH2O_Header*>( lMH2O_Chunk );
			
			for( int py = 0; py < 16; py++ )
				for( int px = 0; px < 16; px++ )
					for( unsigned int lLayer = 0; lLayer < lHeader[py * 16 + px].nLayers; lLayer++ )
					{
						MH2O_Tile lTile;
						if( lHeader[py * 16 + px].ofsInformation )
						{
							MH2O_Information * lInfoBlock = reinterpret_cast<MH2O_Information*>( lMH2O_Chunk + lHeader[py * 16 + px].ofsInformation + lLayer * 0x18 );

							lTile.mLiquidType = lInfoBlock->LiquidType;
							lTile.mFlags = lInfoBlock->Flags;
							lTile.mMinimum = lInfoBlock->minHeight;
							lTile.mMaximum = lInfoBlock->maxHeight;
							
							char * lDepthInfo;
							float * lHeightInfo = reinterpret_cast<float*>( lMH2O_Chunk + lInfoBlock->ofsHeightMap );
							if( lTile.mFlags & 2 )
								lDepthInfo = reinterpret_cast<char*>( lMH2O_Chunk + lInfoBlock->ofsHeightMap );
							else
								lDepthInfo = reinterpret_cast<char*>( lMH2O_Chunk + lInfoBlock->ofsHeightMap + sizeof( float ) * ( lInfoBlock->width + 1 ) * ( lInfoBlock->height + 1 ) );

							if( lInfoBlock->ofsHeightMap )
								for( int i = lInfoBlock->yOffset; i < lInfoBlock->yOffset + lInfoBlock->height + 1; i++ )
									for( int j = lInfoBlock->xOffset; j < lInfoBlock->xOffset + lInfoBlock->width + 1; j++ )
									{
										if( lTile.mFlags & 2 )
											lTile.mHeightmap[i][j] = lTile.mMinimum;
										else
											lTile.mHeightmap[i][j] = lHeightInfo[( i - lInfoBlock->yOffset ) * lInfoBlock->width + j];
										lTile.mDepth[i][j] = lDepthInfo[( i - lInfoBlock->yOffset ) * lInfoBlock->width + j]/255.0f;
									}
							else
								for( int i = 0; i < 9; i++ )
									for( int j = 0; j < 9; j++ ){
										lTile.mHeightmap[i][j] = lTile.mMinimum;
										lTile.mDepth[i][j] = 0.0f;
									}

							for( int i = 0; i < 9; i++ )
								for( int j = 0; j < 9; j++ )
									lTile.mHeightmap[i][j] = lTile.mHeightmap[i][j] < lTile.mMinimum ? lTile.mMinimum-10 : lTile.mHeightmap[i][j] > lTile.mMaximum ? lTile.mMaximum+10 : lTile.mHeightmap[i][j];

							
							//! \todo  This is wrong?
							if( lHeader[py * 16 + px].ofsRenderMask )
							{
								bool * lRenderBlock = reinterpret_cast<bool*>( lMH2O_Chunk + lHeader[py * 16 + px].ofsRenderMask + lLayer * 8 );

								int k = 0;
								for( int i = 0; i < 8; i++ )
									for( int j = 0; j < 8; j++ )
										lTile.mRender[i][j] = lRenderBlock[k++];
							}
							else
								// --Slartibartfast 00:41, 30 October 2008 (CEST): "If the block is omitted, the whole thing is rendered"
								for( int i = 0; i < 8; i++ )
									for( int j = 0; j < 8; j++ )
										lTile.mRender[i][j] = true;

						/*	if( lInfoBlock->ofsInfoMask )
							{
								for( int i = 0; i < 8; i++ )
									for( int j = 0; j < 8; j++ )
										lTile.mRender[i][j] = false;

								bool * lMaskInfo = reinterpret_cast<bool*>( lMH2O_Chunk + lInfoBlock->ofsInfoMask );
								unsigned char * lMaskInfo2 = reinterpret_cast<unsigned char*>( lMH2O_Chunk + lInfoBlock->ofsInfoMask );
								int k = 0;

								string dbg = "";
								for( int i = lInfoBlock->yOffset; i < lInfoBlock->yOffset + lInfoBlock->height; i++ )
								{
									gLog( "\t\t\tx%2x (%c)\n", lMaskInfo2[i], lMaskInfo2[i] );
									for( int j = lInfoBlock->xOffset; j < lInfoBlock->xOffset + lInfoBlock->width; j++ )
									{
										dbg.append( lMaskInfo[k] ? "#" : " " );
										lTile.mRender[i][j] = lMaskInfo[k++];
									}
									dbg.append( "\n" );
								}

								LogDebug << dbg << std::endl;
							}
							else*/
							{
								for( int i = 0; i < 8; i++ )
									for( int j = 0; j < 8; j++ )
										lTile.mRender[i][j] = false;

								for( int i = lInfoBlock->yOffset; i < lInfoBlock->yOffset + lInfoBlock->height; i++ )
									for( int j = lInfoBlock->xOffset; j < lInfoBlock->xOffset + lInfoBlock->width; j++ )
										lTile.mRender[i][j] = true;
							}

							Liquid * lq = new Liquid( lInfoBlock->width, lInfoBlock->height, Vec3D( xbase + CHUNKSIZE * px, lTile.mMinimum, zbase + CHUNKSIZE * py ) );
							lq->initFromMH2O( lTile );
							mLiquids.push_back( lq );
						}
					}
		}

		theFile.seek((int)nextpos);
	}
	
	// - MFBO ----------------------------------------------
	
	if( Header.flags & 1 )
	{
		theFile.seek( Header.mfbo + 0x14 );
		theFile.read( &fourcc, 4 );
		theFile.read( &size, 4 );
		
		assert( fourcc == 'MFBO' );
		
		short mMaximum[9], mMinimum[9];
		theFile.read( mMaximum, sizeof( short[9] ) );
		theFile.read( mMinimum, sizeof( short[9] ) );
			
		static const float xPositions[] = { this->xbase, this->xbase + 266.0f, this->xbase + 533.0f };
		static const float yPositions[] = { this->zbase, this->zbase + 266.0f, this->zbase + 533.0f };
		
		for( int y = 0; y < 3; y++ )
		{
			for( int x = 0; x < 3; x++ )
			{
				int pos = x + y * 3;
				mMinimumValues[pos*3 + 0] = xPositions[x];
				mMinimumValues[pos*3 + 1] = mMinimum[pos];
				mMinimumValues[pos*3 + 2] = yPositions[y];
				
				mMaximumValues[pos*3 + 0] = xPositions[x];
				mMaximumValues[pos*3 + 1] = mMaximum[pos];
				mMaximumValues[pos*3 + 2] = yPositions[y];
			}
		}
	}
	
	// - Done. ---------------------------------------------
  
  // - Load textures -------------------------------------
  
  for( std::vector<std::string>::iterator it = mTextureFilenames.begin( ); it != mTextureFilenames.end( ); it++ )
	{
		std::string lTexture = *it;
		//! \todo  Find a different way to do this.
		/*
     if( video.mSupportShaders )
     {
     std::string lTemp = lTexture;
     lTemp.insert( lTemp.length( ) - 4, "_s" );
     if( MPQFile::exists( lTemp ) )
     lTexture = lTemp;
     }
     */
		video.textures.add( lTexture );
	}
  
  // - Load WMOs -----------------------------------------
  
	for( std::vector<std::string>::iterator it = mWMOFilenames.begin( ); it != mWMOFilenames.end( ); it++ )
	{
		gWorld->wmomanager.add( *it );
	}
  
	for( std::vector<ENTRY_MODF>::iterator it = lWMOInstances.begin( ); it != lWMOInstances.end( ); it++ )
	{
		WMO *wmo = (WMO*)gWorld->wmomanager.items[gWorld->wmomanager.get(mWMOFilenames[it->nameID])];
		gWorld->mWMOInstances.insert( std::pair<int,WMOInstance>( it->uniqueID, WMOInstance( wmo, &(*it) ) ) );
	}
  
  // - Load M2s ------------------------------------------
  
	for( std::vector<std::string>::iterator it = mModelFilenames.begin( ); it != mModelFilenames.end( ); it++ )
	{
		gWorld->modelmanager.add( *it );
	}
  
	for( std::vector<ENTRY_MDDF>::iterator it = lModelInstances.begin( ); it != lModelInstances.end( ); it++ )
	{
		Model *model = (Model*)gWorld->modelmanager.items[gWorld->modelmanager.get(mModelFilenames[it->nameID])];
		gWorld->mModelInstances.insert( std::pair<int,ModelInstance>( it->uniqueID, ModelInstance( model, &(*it) ) ) );
	}
  
  // - Load chunks ---------------------------------------
  
	for( int nextChunk = 0; nextChunk < 256; nextChunk++ ) 
	{
		theFile.seek( lMCNKOffsets[nextChunk] );
		mChunks[nextChunk / 16][nextChunk % 16] = new MapChunk( this, theFile, mBigAlpha );
	}
  
	theFile.close( );
  
	// - Really done. --------------------------------------
  
	LogDebug << "Done loading tile " << mPositionX << "," << mPositionZ << "." << std::endl;
}

MapTile::~MapTile( )
{
	LogDebug << "Unloading tile " << mPositionX << "," << mPositionZ << "." << std::endl;

  for( int j = 0; j < 16; j++ ) 
  {
    for( int i = 0; i < 16; i++ ) 
    {
      if( mChunks[j][i] )
      {
        delete mChunks[j][i];
        mChunks[j][i] = NULL;
      }
    }
  }

	for( std::vector<std::string>::iterator it = mTextureFilenames.begin( ); it != mTextureFilenames.end( ); it++ )
  {
    video.textures.delbyname( *it );
	}

	for( std::vector<std::string>::iterator it = mWMOFilenames.begin( ); it != mWMOFilenames.end( ); it++ ) 
  {
		gWorld->wmomanager.delbyname( *it );
	}

	for( std::vector<std::string>::iterator it = mModelFilenames.begin( ); it != mModelFilenames.end( ); it++ ) 
  {
		gWorld->modelmanager.delbyname( *it );
	}
  
  for( std::vector<Liquid*>::iterator it = mLiquids.begin( ); it != mLiquids.end( ); it++ )
  {
    delete &(*it);
  }
  
  mLiquids.clear();
}

bool MapTile::isTile( int pX, int pZ )
{
  return pX == mPositionX && pZ == mPositionZ;
}

float MapTile::getMaxHeight()
{
  using std::max;
  float maxHeight = -99999.0f;
  for( int nextChunk = 0; nextChunk < 256; nextChunk++ ) 
	{
    maxHeight = max( mChunks[nextChunk / 16][nextChunk % 16]->vmax.y, maxHeight );
	}
  return maxHeight;
}

//extern float groundBrushRadius;

void MapTile::draw()
{
	/* Selection circle
	if( false && gWorld->IsSelection( eEntry_MapChunk ) && terrainMode != 3 )
	{
		int poly = gWorld->GetCurrentSelectedTriangle( );

		glColor4f( 1.0f, 0.3f, 0.3f, 1.0f );

		nameEntry * Selection = gWorld->GetCurrentSelection( );

		if( !Selection->data.mapchunk->strip )
			Selection->data.mapchunk->initStrip( );
		
		float x = ( Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly]].x + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+1]].x + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+2]].x ) / 3;
		float y = ( Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly]].y + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+1]].y + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+2]].y ) / 3;
		float z = ( Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly]].z + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+1]].z + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+2]].z ) / 3;
		glDisable(GL_CULL_FACE);
		glDepthMask(false);
		//glDisable(GL_DEPTH_TEST);
		renderCylinder_convenient( x, y, z, groundBrushRadius, 100 );
		glEnable(GL_CULL_FACE);
		//glEnable(GL_DEPTH_TEST);
		glDepthMask(true);

	}*/
	glColor4f(1,1,1,1);

	for (int j=0; j<16; j++)
		for (int i=0; i<16; i++)
			mChunks[j][i]->draw();
}

void MapTile::drawSelect()
{
	for (int j=0; j<16; j++)
		for (int i=0; i<16; i++)
			mChunks[j][i]->drawSelect();
}

void MapTile::drawLines()//draw red lines around the square of a chunk
{
	glDisable(GL_COLOR_MATERIAL);
	
	for (int j=0; j<16; j++)
		for (int i=0; i<16; i++)
			mChunks[j][i]->drawLines();
	
	glEnable(GL_COLOR_MATERIAL);
}

void MapTile::drawMFBO()
{
	static const GLshort lIndices[] = { 4, 1, 2, 5, 8, 7, 6, 3, 0, 1, 0, 3, 6, 7, 8, 5, 2, 1 };

	glColor4f(0,1,1,0.2f);
	glBegin(GL_TRIANGLE_FAN);
	for( int i = 0; i < 18; i++ )
	{
		glVertex3f( mMinimumValues[lIndices[i]*3 + 0], mMinimumValues[lIndices[i]*3 + 1], mMinimumValues[lIndices[i]*3 + 2]	);
	}
	glEnd();
	
	glColor4f(1,1,0,0.2f);
	glBegin(GL_TRIANGLE_FAN);
	for( int i = 0; i < 18; i++ )
	{
		glVertex3f( mMaximumValues[lIndices[i]*3 + 0], mMaximumValues[lIndices[i]*3 + 1], mMaximumValues[lIndices[i]*3 + 2]	);
	}
	glEnd();
}

void MapTile::drawWater()
{
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	
	for( std::vector<Liquid*>::iterator liq = mLiquids.begin( ); liq != mLiquids.end( ); liq++ )
		( *liq )->draw( );
  
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
}

// This is for the 2D mode only.
void MapTile::drawTextures()
{
	float xOffset,yOffset;

	glPushMatrix();
	xOffset=xbase/CHUNKSIZE;
	yOffset=zbase/CHUNKSIZE;
	glTranslatef(xOffset,yOffset,0);
	
	//glTranslatef(-8,-8,0);
	
	for (int j=0; j<16; j++) {
		for (int i=0; i<16; i++) {
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
	int xcol = int( ( x - xbase ) / CHUNKSIZE );
	int ycol = int( ( z - zbase ) / CHUNKSIZE );
	
	return xcol >= 0 && xcol <= 15 && ycol >= 0 && ycol <= 15 && mChunks[ycol][xcol]->GetVertex( x, z, V );
}

/// --- Only saving related below this line. --------------------------

char roundc( float a )
{
	if( a < 0 )
		a -= 0.5f;
	if( a > 0 )
		a += 0.5f;
	if( a < -127 )
		a = -127;
	else if( a > 127 )
		a = 127;
	return char( a );
}

bool pointInside( Vec3D point, Vec3D extents[2] )
{
	return point.x >= extents[0].x && point.z >= extents[0].z && point.x <= extents[1].x && point.z <= extents[1].z;
}

void minmax( Vec3D &a, Vec3D &b )
{
	if( a.x > b.x )
	{
		float t = b.x;
		b.x = a.x;
		a.x = t;
	}
	if( a.y > b.y )
	{
		float t = b.y;
		b.y = a.y;
		a.y = t;
	}
	if( a.z > b.z )
	{
		float t = b.z;
		b.z = a.z;
		a.z = t;
	}
}

bool checkInside( Vec3D extentA[2], Vec3D extentB[2] )
{
	minmax( extentA[0], extentA[1] );
	minmax( extentB[0], extentB[1] );

	return pointInside( extentA[0], extentB ) || 
	       pointInside( extentA[1], extentB ) || 
	       pointInside( extentB[0], extentA ) || 
	       pointInside( extentB[1], extentA );
}

class sExtendableArray
{
public:
	int mSize;
	char * mData;

	bool Allocate( int pSize )
	{
		mSize = pSize;
		mData = reinterpret_cast<char*>( realloc( mData, mSize ) );
		memset( mData, 0, mSize );
		return( mData != NULL );
	}
	bool Extend( int pAddition )
	{
		mSize = mSize + pAddition;
		mData = reinterpret_cast<char*>( realloc( mData, mSize ) );
		memset( mData + mSize - pAddition, 0, pAddition );
		return( mData != NULL );
	}
	bool Insert( int pPosition, int pAddition )
	{
		const int lPostSize = mSize - pPosition;

		char * lPost = reinterpret_cast<char*>( malloc( lPostSize ) );
		memcpy( lPost, mData + pPosition, lPostSize );

		if( !Extend( pAddition ) )
			return false;

		memcpy( mData + pPosition + pAddition, lPost, lPostSize );
		memset( mData + pPosition, 0, pAddition );
		return true;
	}
	bool Insert( int pPosition, int pAddition, char * pAdditionalData )
	{
		const int lPostSize = mSize - pPosition;

		char * lPost = reinterpret_cast<char*>( malloc( lPostSize ) );
		memcpy( lPost, mData + pPosition, lPostSize );

		if( !Extend( pAddition ) )
			return false;

		memcpy( mData + pPosition + pAddition, lPost, lPostSize );
		memcpy( mData + pPosition, pAdditionalData, pAddition );
		return true;
	}

	template<typename To>
	To * GetPointer( )
	{
		return( reinterpret_cast<To*>( mData ) );
	}
	template<typename To>
	To * GetPointer( unsigned int pPosition )
	{
		return( reinterpret_cast<To*>( mData + pPosition ) );
	}

	sExtendableArray( )
	{
		mSize = 0;
		mData = NULL;
	}
	sExtendableArray( int pSize, char * pData )
	{
		if( Allocate( pSize ) )
			memcpy( mData, pData, pSize );
		else
			LogError << "Allocating " << pSize << " bytes failed. This may crash soon." << std::endl;
	}

	void Destroy( )
	{
		free( mData );
	}
};

struct sChunkHeader
{
	int mMagic;
	int mSize;
};

void SetChunkHeader( sExtendableArray pArray, int pPosition, int pMagix, int pSize = 0 )
{
	sChunkHeader * Header = pArray.GetPointer<sChunkHeader>( pPosition );
	Header->mMagic = pMagix;
	Header->mSize = pSize;
}

void MapTile::saveTile( )
{
  using std::min;
  using std::max;
  
	Log << "Saving ADT \"" << mFilename << "\"." << std::endl;

	int lID;	// This is a global counting variable. Do not store something in here you need later.

	// Collect some information we need later.

	// Check which doodads and WMOs are on this ADT.
	Vec3D lTileExtents[2];
	lTileExtents[0] = Vec3D( this->xbase, 0.0f, this->zbase );
	lTileExtents[1] = Vec3D( this->xbase + TILESIZE, 0.0f, this->zbase + TILESIZE );

	std::map<int, WMOInstance> lObjectInstances;
	std::map<int, ModelInstance> lModelInstances;

	for( std::map<int, WMOInstance>::iterator it = gWorld->mWMOInstances.begin(); it != gWorld->mWMOInstances.end(); ++it )
		if( checkInside( lTileExtents, it->second.extents ) )
			lObjectInstances.insert( std::pair<int, WMOInstance>( it->first, it->second ) );
	
	for( std::map<int, ModelInstance>::iterator it = gWorld->mModelInstances.begin(); it != gWorld->mModelInstances.end(); ++it )
	{
		Vec3D lModelExtentsV1[2], lModelExtentsV2[2];
		lModelExtentsV1[0] = it->second.model->header.BoundingBoxMin + it->second.pos;
		lModelExtentsV1[1] = it->second.model->header.BoundingBoxMax + it->second.pos;
		lModelExtentsV2[0] = it->second.model->header.VertexBoxMin + it->second.pos;
		lModelExtentsV2[1] = it->second.model->header.VertexBoxMax + it->second.pos;
		
		if( checkInside( lTileExtents, lModelExtentsV1 ) || checkInside( lTileExtents, lModelExtentsV2 ) )
			lModelInstances.insert( std::pair<int, ModelInstance>( it->first, it->second ) );
	}

	std::map<std::string, int*> lModels;

	for( std::map<int,ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
	{
		//! \todo  Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
		std::string lTemp = it->second.model->filename;
		transform( lTemp.begin(), lTemp.end(), lTemp.begin(), ::tolower );
		size_t found = lTemp.rfind( ".m2" );
		if( found != std::string::npos )
		{
			lTemp.replace( found, 3, ".md" );
			lTemp.append( "x" );
		}

		if( lModels.find( lTemp ) == lModels.end( ) )
			lModels.insert( std::pair<std::string, int*>( lTemp, new int[2] ) ); 
	}
	
	lID = 0;
	for( std::map<std::string, int*>::iterator it = lModels.begin( ); it != lModels.end( ); it++ )
		it->second[0] = lID++;

	std::map<std::string, int*> lObjects;

	for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
		if( lObjects.find( it->second.wmo->filename ) == lObjects.end( ) )
			lObjects.insert( std::pair<std::string, int*>( ( it->second.wmo->filename ), new int[2] ) ); 
	
	lID = 0;
	for( std::map<std::string, int*>::iterator it = lObjects.begin( ); it != lObjects.end( ); it++ )
		it->second[0] = lID++;

	// Check which textures are on this ADT.
	std::map<std::string, int> lTextures;

	for( int i = 0; i < 16; i++ )
		for( int j = 0; j < 16; j++ )
			for( int tex = 0; tex < mChunks[i][j]->nTextures; tex++ )
				if( lTextures.find( video.textures.items[mChunks[i][j]->textures[tex]]->name ) == lTextures.end( ) )
					lTextures.insert( std::pair<std::string, int>( video.textures.items[mChunks[i][j]->textures[tex]]->name, -1 ) ); 
	
	lID = 0;
	for( std::map<std::string, int>::iterator it = lTextures.begin( ); it != lTextures.end( ); it++ )
		it->second = lID++;

	// Now write the file.
	
	sExtendableArray lADTFile = sExtendableArray( );
	
	int lCurrentPosition = 0;

	// MVER
//	{
		lADTFile.Extend( 8 + 0x4 );
		SetChunkHeader( lADTFile, lCurrentPosition, 'MVER', 4 );

		// MVER data
		*( lADTFile.GetPointer<int>( 8 ) ) = 18;

		lCurrentPosition += 8 + 0x4;
//	}

	// MHDR
	int lMHDR_Position = lCurrentPosition;
//	{
		lADTFile.Extend( 8 + 0x40 );
		SetChunkHeader( lADTFile, lCurrentPosition, 'MHDR', 0x40 );

		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->flags = mFlags;

		lCurrentPosition += 8 + 0x40;
//	}

	// MCIN
	int lMCIN_Position = lCurrentPosition;
//	{
		lADTFile.Extend( 8 + 256 * 0x10 );
		SetChunkHeader( lADTFile, lCurrentPosition, 'MCIN', 256 * 0x10 );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mcin = lCurrentPosition - 0x14;

		// MCIN * MCIN_Data = lADTFile.GetPointer<MCIN>( lMCIN_Position + 8 );

		lCurrentPosition += 8 + 256 * 0x10;
//	}

	// MTEX
//	{
		int lMTEX_Position = lCurrentPosition;
		lADTFile.Extend( 8 + 0 );	// We don't yet know how big this will be.
		SetChunkHeader( lADTFile, lCurrentPosition, 'MTEX' );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mtex = lCurrentPosition - 0x14;

		lCurrentPosition += 8 + 0;

		// MTEX data
		for( std::map<std::string, int>::iterator it = lTextures.begin( ); it != lTextures.end( ); it++ )
		{
			lADTFile.Insert( lCurrentPosition, it->first.size( ) + 1, const_cast<char*>( it->first.c_str( ) ) );
			lCurrentPosition += it->first.size( ) + 1;
			lADTFile.GetPointer<sChunkHeader>( lMTEX_Position )->mSize += it->first.size( ) + 1;
			LogDebug << "Added texture \"" << it->first << "\"." << std::endl;
		}
//	}

	// MMDX
//	{
		int lMMDX_Position = lCurrentPosition;
		lADTFile.Extend( 8 + 0 );	// We don't yet know how big this will be.
		SetChunkHeader( lADTFile, lCurrentPosition, 'MMDX' );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mmdx = lCurrentPosition - 0x14;

		lCurrentPosition += 8 + 0;

		// MMDX data
		for( std::map<std::string, int*>::iterator it = lModels.begin( ); it != lModels.end( ); it++ )
		{
			it->second[1] = lADTFile.GetPointer<sChunkHeader>( lMMDX_Position )->mSize;
			lADTFile.Insert( lCurrentPosition, it->first.size( ) + 1, const_cast<char*>( it->first.c_str( ) ) );
			lCurrentPosition += it->first.size( ) + 1;
			lADTFile.GetPointer<sChunkHeader>( lMMDX_Position )->mSize += it->first.size( ) + 1;
			LogDebug << "Added model \"" << it->first << "\"." << std::endl;
		}
//	}

	// MMID
//	{
		int lMMID_Size = 4 * lModels.size( );
		lADTFile.Extend( 8 + lMMID_Size );
		SetChunkHeader( lADTFile, lCurrentPosition, 'MMID', lMMID_Size );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mmid = lCurrentPosition - 0x14;

		// MMID data
		int * lMMID_Data = lADTFile.GetPointer<int>( lCurrentPosition + 8 );
		
		lID = 0;
		for( std::map<std::string, int*>::iterator it = lModels.begin( ); it != lModels.end( ); it++ )
			lMMID_Data[lID++] = it->second[1];

		lCurrentPosition += 8 + lMMID_Size;
//	}
	
	// MWMO
//	{
		int lMWMO_Position = lCurrentPosition;
		lADTFile.Extend( 8 + 0 );	// We don't yet know how big this will be.
		SetChunkHeader( lADTFile, lCurrentPosition, 'MWMO' );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mwmo = lCurrentPosition - 0x14;

		lCurrentPosition += 8 + 0;

		// MWMO data
		for( std::map<std::string, int*>::iterator it = lObjects.begin( ); it != lObjects.end( ); it++ )
		{
			it->second[1] = lADTFile.GetPointer<sChunkHeader>( lMWMO_Position )->mSize;
			lADTFile.Insert( lCurrentPosition, it->first.size( ) + 1, const_cast<char*>( it->first.c_str( ) ) );
			lCurrentPosition += it->first.size( ) + 1;
			lADTFile.GetPointer<sChunkHeader>( lMWMO_Position )->mSize += it->first.size( ) + 1;
			LogDebug << "Added object \"" << it->first << "\"." << std::endl;
		}
//	}

	// MWID
//	{
		int lMWID_Size = 4 * lObjects.size( );
		lADTFile.Extend( 8 + lMWID_Size );
		SetChunkHeader( lADTFile, lCurrentPosition, 'MWID', lMWID_Size );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mwid = lCurrentPosition - 0x14;

		// MWID data
		int * lMWID_Data = lADTFile.GetPointer<int>( lCurrentPosition + 8 );
		
		lID = 0;
		for( std::map<std::string, int*>::iterator it = lObjects.begin( ); it != lObjects.end( ); it++ )
			lMWID_Data[lID++] = it->second[1];

		lCurrentPosition += 8 + lMWID_Size;
//	}

	// MDDF
//	{
		int lMDDF_Size = 0x24 * lModelInstances.size( );
		lADTFile.Extend( 8 + lMDDF_Size );
		SetChunkHeader( lADTFile, lCurrentPosition, 'MDDF', lMDDF_Size );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mddf = lCurrentPosition - 0x14;

		// MDDF data
		ENTRY_MDDF * lMDDF_Data = lADTFile.GetPointer<ENTRY_MDDF>( lCurrentPosition + 8 );
	
		int model_counter = 1;
		lID = 0;
		for( std::map<int,ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
		{
			//! \todo  Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
			std::string lTemp = it->second.model->filename;
			transform( lTemp.begin(), lTemp.end(), lTemp.begin(), ::tolower );
			size_t found = lTemp.rfind( ".m2" );
			if( found != std::string::npos )
			{
				lTemp.replace( found, 3, ".md" );
				lTemp.append( "x" );
			}
			std::map<std::string, int*>::iterator lMyFilenameThingey = lModels.find( lTemp );
			if( lMyFilenameThingey == lModels.end( ) )
			{
				LogError << "There is a problem with saving the doodads. We have a doodad that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
				return;
			}
      
      // XXZZTNNN
      //        1
      //     1000
      //    10000
      //  1000000
      
      int lNewUID = lID + mPositionX * 1000000 + mPositionZ * 10000 + 1 * 1000;

			lMDDF_Data[lID].nameID = lMyFilenameThingey->second[0];
			lMDDF_Data[lID].uniqueID = lNewUID;
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
//	}

	// MODF
//	{
		int lMODF_Size = 0x40 * lObjectInstances.size( );
		lADTFile.Extend( 8 + lMODF_Size );
		SetChunkHeader( lADTFile, lCurrentPosition, 'MODF', lMODF_Size );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->modf = lCurrentPosition - 0x14;

		// MODF data
		ENTRY_MODF * lMODF_Data = lADTFile.GetPointer<ENTRY_MODF>( lCurrentPosition + 8 );

		// reset model counter
		model_counter = 1;
		lID = 0;
		for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
		{
			std::map<std::string, int*>::iterator lMyFilenameThingey = lObjects.find( it->second.wmo->filename );
			if( lMyFilenameThingey == lObjects.end( ) )
			{
				LogError << "There is a problem with saving the objects. We have an object that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
				return;
			}
      
      // XXZZTNNN
      //        1
      //     1000
      //    10000
      //  1000000
      
      int lNewUID = lID + mPositionX * 1000000 + mPositionZ * 10000 + 2 * 1000;

			lMODF_Data[lID].nameID = lMyFilenameThingey->second[0];
			lMODF_Data[lID].uniqueID = lNewUID;
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
//	}

	// MCNK
	//! \todo  MCNK
//	{
		for( int y = 0; y < 16; y++ )
		{
			for( int x = 0; x < 16; x++ )
			{
				int lMCNK_Size = 0x80;
				int lMCNK_Position = lCurrentPosition;
				lADTFile.Extend( 8 + 0x80 );	// This is only the size of the header. More chunks will increase the size.
				SetChunkHeader( lADTFile, lCurrentPosition, 'MCNK', lMCNK_Size );
				lADTFile.GetPointer<MCIN>( lMCIN_Position + 8 )->mEntries[y*16+x].offset = lCurrentPosition;

				// MCNK data
				lADTFile.Insert( lCurrentPosition + 8, 0x80, reinterpret_cast<char*>( &( this->mChunks[y][x]->header ) ) );
				MapChunkHeader * lMCNK_header = lADTFile.GetPointer<MapChunkHeader>( lCurrentPosition + 8 );

				lMCNK_header->flags = mChunks[y][x]->Flags;
				lMCNK_header->holes = mChunks[y][x]->holes;
				lMCNK_header->areaid = mChunks[y][x]->areaID;
				
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
				lMCNK_header->sizeLiquid = 8;

				//! \todo  MCCV sub-chunk
				lMCNK_header->ofsMCCV = 0;

				if( lMCNK_header->flags & 0x40 )
					LogError << "Problem with saving: This ADT is said to have vertex shading but we don't write them yet. This might get you really fucked up results." << std::endl;
				lMCNK_header->flags = lMCNK_header->flags & ( ~0x40 );


				lCurrentPosition += 8 + 0x80;
				
				// MCVT
//				{
					int lMCVT_Size = ( 9 * 9 + 8 * 8 ) * 4;

					lADTFile.Extend( 8 + lMCVT_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCVT', lMCVT_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsHeight = lCurrentPosition - lMCNK_Position;

					float * lHeightmap = lADTFile.GetPointer<float>( lCurrentPosition + 8 );

					float lMedian = 0.0f;
					for( int i = 0; i < ( 9 * 9 + 8 * 8 ); i++ )
						lMedian = lMedian + mChunks[y][x]->mVertices[i].y;

					lMedian = lMedian / ( 9 * 9 + 8 * 8 );
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ypos = lMedian;

					for( int i = 0; i < ( 9 * 9 + 8 * 8 ); i++ )
						lHeightmap[i] = mChunks[y][x]->mVertices[i].y - lMedian;
					
					lCurrentPosition += 8 + lMCVT_Size;
					lMCNK_Size += 8 + lMCVT_Size;
//				}

				
				// MCNR
//				{
					int lMCNR_Size = ( 9 * 9 + 8 * 8 ) * 3;

					lADTFile.Extend( 8 + lMCNR_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCNR', lMCNR_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsNormal = lCurrentPosition - lMCNK_Position;

					char * lNormals = lADTFile.GetPointer<char>( lCurrentPosition + 8 );

					// recalculate the normals
					mChunks[y][x]->recalcNorms();
					for( int i = 0; i < ( 9 * 9 + 8 * 8 ); i++ )
					{
						lNormals[i*3+0] = roundc( -mChunks[y][x]->mNormals[i].z * 127 );
						lNormals[i*3+1] = roundc( -mChunks[y][x]->mNormals[i].x * 127 );
						lNormals[i*3+2] = roundc(  mChunks[y][x]->mNormals[i].y * 127 );
					}
					
					lCurrentPosition += 8 + lMCNR_Size;
					lMCNK_Size += 8 + lMCNR_Size;
//				}

				// Unknown MCNR bytes
				// These are not in as we have data or something but just to make the files more blizzlike.
//				{
					lADTFile.Extend( 13 );
					lCurrentPosition += 13;
					lMCNK_Size += 13;
//				}

				// MCLY
//				{
					int lMCLY_Size = mChunks[y][x]->nTextures * 0x10;

					lADTFile.Extend( 8 + lMCLY_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCLY', lMCLY_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsLayer = lCurrentPosition - lMCNK_Position;
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nLayers = mChunks[y][x]->nTextures;
			
					// MCLY data
					for( int j = 0; j < mChunks[y][x]->nTextures; j++ )
					{
						ENTRY_MCLY * lLayer = lADTFile.GetPointer<ENTRY_MCLY>( lCurrentPosition + 8 + 0x10 * j );

						lLayer->textureID = lTextures.find( video.textures.items[mChunks[y][x]->textures[j]]->name )->second;

						lLayer->flags = mChunks[y][x]->texFlags[j];
						
						// if not first, have alpha layer, if first, have not. never have compression.
						lLayer->flags = ( j > 0 ? lLayer->flags | 0x100 : lLayer->flags & ( ~0x100 ) ) & ( ~0x200 );

						lLayer->ofsAlpha = ( j == 0 ? 0 : ( mBigAlpha ? 64 * 64 * ( j - 1 ) : 32 * 64 * ( j - 1 ) ) );
						lLayer->effectID = mChunks[y][x]->effectID[j];
					}

					lCurrentPosition += 8 + lMCLY_Size;
					lMCNK_Size += 8 + lMCLY_Size;
//				}

				// MCRF
//				{
					std::list<int> lDoodadIDs;
					std::list<int> lObjectIDs;
					
					Vec3D lChunkExtents[2];
					lChunkExtents[0] = Vec3D( mChunks[y][x]->xbase, 0.0f, mChunks[y][x]->zbase );
					lChunkExtents[1] = Vec3D( mChunks[y][x]->xbase + CHUNKSIZE, 0.0f, mChunks[y][x]->zbase + CHUNKSIZE );

					// search all wmos that are inside this chunk
					lID = 0;
					for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
					{
						//! \todo  This requires the extents already being calculated. See above.
						if( checkInside( lChunkExtents, it->second.extents ) )
							lObjectIDs.push_back( lID );
						lID++;
					}

					// search all models that are inside this chunk
					lID = 0;
					for( std::map<int, ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
					{						
						// get radius and position of the m2
						float radius = it->second.model->header.BoundingBoxRadius;
						Vec3D& pos = it->second.pos;

						// Calculate the chunk zenter
						Vec3D chunkMid(mChunks[y][x]->xbase + CHUNKSIZE / 2, 0, 
						mChunks[y][x]->zbase + CHUNKSIZE / 2);
	
						// find out if the model is inside the reach of the chunk.
						float dx = chunkMid.x - pos.x;
						float dz = chunkMid.z - pos.z;
						float dist = sqrtf(dx * dx + dz * dz);
						static float sqrt2 = sqrtf(2.0f);

						if(dist - radius <= ((sqrt2 / 2.0f) * CHUNKSIZE))
						{
							lDoodadIDs.push_back(lID);
						}
	
						lID++;
					}

					int lMCRF_Size = 4 * ( lDoodadIDs.size( ) + lObjectIDs.size( ) );
					lADTFile.Extend( 8 + lMCRF_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCRF', lMCRF_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsRefs = lCurrentPosition - lMCNK_Position;
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nDoodadRefs = lDoodadIDs.size( );
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nMapObjRefs = lObjectIDs.size( );

					// MCRF data
					int * lReferences = lADTFile.GetPointer<int>( lCurrentPosition + 8 );

					lID = 0;
					for( std::list<int>::iterator it = lDoodadIDs.begin( ); it != lDoodadIDs.end( ); it++ )
					{
						lReferences[lID] = *it;
						lID++;
					}

					for( std::list<int>::iterator it = lObjectIDs.begin( ); it != lObjectIDs.end( ); it++ )
					{
						lReferences[lID] = *it;
						lID++;
					}

					lCurrentPosition += 8 + lMCRF_Size;
					lMCNK_Size += 8 + lMCRF_Size;
//				}

				// MCSH
//				{
					//! \todo  Somehow determine if we need to write this or not?
					//! \todo  This sometime gets all shadows black.
					if( mChunks[y][x]->Flags & 1 )
					{
						int lMCSH_Size = 0x200;
						lADTFile.Extend( 8 + lMCSH_Size );
						SetChunkHeader( lADTFile, lCurrentPosition, 'MCSH', lMCSH_Size );

						lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsShadow = lCurrentPosition - lMCNK_Position;
						lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeShadow = 0x200;

						char * lLayer = lADTFile.GetPointer<char>( lCurrentPosition + 8 );
						
						memcpy( lLayer, mChunks[y][x]->mShadowMap, 0x200 );

						lCurrentPosition += 8 + lMCSH_Size;
						lMCNK_Size += 8 + lMCSH_Size;
					}
					else
					{
						lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsShadow = 0;
						lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeShadow = 0;
					}
//				}

				// MCAL
//				{
					int lDimensions = 64 * ( mBigAlpha ? 64 : 32 );

					int lMaps = mChunks[y][x]->nTextures - 1;
					lMaps = lMaps >= 0 ? lMaps : 0;

					int lMCAL_Size = lDimensions * lMaps;
					
					lADTFile.Extend( 8 + lMCAL_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCAL', lMCAL_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsAlpha = lCurrentPosition - lMCNK_Position;
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeAlpha = 8 + lMCAL_Size;

					char * lAlphaMaps = lADTFile.GetPointer<char>( lCurrentPosition + 8 );

					for( int j = 0; j < lMaps; j++ )
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
								lowerNibble = (unsigned char)max(min( ( (float)mChunks[y][x]->amap[j][k * 2 + 0] ) * 0.05882f + 0.5f , 15.0f),0.0f);
								upperNibble = (unsigned char)max(min( ( (float)mChunks[y][x]->amap[j][k * 2 + 1] ) * 0.05882f + 0.5f , 15.0f),0.0f);
								lAlphaMaps[lDimensions * j + k] = ( upperNibble << 4 ) + lowerNibble;
							}
						}
					}

					lCurrentPosition += 8 + lMCAL_Size;
					lMCNK_Size += 8 + lMCAL_Size;
//				}

				// MCLQ
//				{
					int lMCLQ_Size = 0;
					lADTFile.Extend( 8 + lMCLQ_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCLQ', lMCLQ_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsLiquid = lCurrentPosition - lMCNK_Position;
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeLiquid = ( lMCLQ_Size == 0 ? 8 : lMCLQ_Size );

					// if ( data ) do write
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
					/*
					if(ChunkHeader[i].ofsLiquid!=0&&ChunkHeader[i].sizeLiquid!=0&&ChunkHeader[i].sizeLiquid!=8){
					Temp=ChunkHeader[i].sizeLiquid;
					memcpy(Buffer+Change+MCINs[i].offset+ChunkHeader[i].ofsLiquid+lChange,f.getBuffer()+MCINs[i].offset+ChunkHeader[i].ofsLiquid,Temp);
					ChunkHeader[i].ofsLiquid+=lChange;
					}
					else{
						ChunkHeader[i].ofsLiquid=0;
						ChunkHeader[i].sizeLiquid=0;
					}
					*/

					lCurrentPosition += 8 + lMCLQ_Size;
					lMCNK_Size += 8 + lMCLQ_Size;
//				}

				// MCSE
//				{
					int lMCSE_Size = 0;
					lADTFile.Extend( 8 + lMCSE_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCSE', lMCSE_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsSndEmitters = lCurrentPosition - lMCNK_Position;
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nSndEmitters = lMCSE_Size / 0x1C;

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
//				}


			
				lADTFile.GetPointer<sChunkHeader>( lMCNK_Position )->mSize = lMCNK_Size;
				lADTFile.GetPointer<MCIN>( lMCIN_Position + 8 )->mEntries[y*16+x].size = lMCNK_Size;
			}
		}
//	}

	// MFBO
	if( this->mFlags & 1 )
	{
		lADTFile.Extend( 8 + 36 );	// We don't yet know how big this will be.
		SetChunkHeader( lADTFile, lCurrentPosition, 'MFBO', 36 );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->mfbo = lCurrentPosition - 0x14;

		short * lMFBO_Data = lADTFile.GetPointer<short>( lCurrentPosition + 8 );
		
		lID = 0;
		for( int i = 0; i < 9; i++ )
			lMFBO_Data[lID++] = mMinimumValues[i * 3 + 1];

		for( int i = 0; i < 9; i++ )
			lMFBO_Data[lID++] = mMaximumValues[i * 3 + 1];

		lCurrentPosition += 8 + 36;
	}

	//! \todo  MH2O
	//! \todo  MTFX
	
	MPQFile f( mFilename );
	f.setBuffer( lADTFile.GetPointer<uint8_t>( ), lADTFile.mSize );
	f.SaveFile( );
	f.close( );
}
