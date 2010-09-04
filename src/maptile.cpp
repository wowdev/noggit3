#include <cassert>
#include <algorithm>

#include "Settings.h"
#include "Project.h"
#include "Environment.h"
#include "maptile.h"
#include "world.h"
#include "vec3d.h"
#include "shaders.h"
#include "Log.h"

using namespace std;

#define HEIGHT_TOP		1000
#define HEIGHT_MID		600
#define HEIGHT_LOW		300
#define HEIGHT_ZERO		0
#define HEIGHT_SHALLOW	-100
#define HEIGHT_DEEP		-250
#define MAPCHUNK_RADIUS	47.140452079103168293389624140323


bool DrawMapContour=true;
bool drawFlags=false;


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



/*
White	1.00	1.00	1.00
Brown	0.75	0.50	0.00
Green	0.00	1.00	0.00
Yellow	1.00	1.00	0.00
Lt Blue	0.00	1.00	1.00
Blue	0.00	0.00	1.00
Black	0.00	0.00	0.00
*/

void HeightColor(float height, Vec3D *Color)
{
	float Amount;
	
	if(height>HEIGHT_TOP)
	{
		Color->x=1.0;
		Color->y=1.0;
		Color->z=1.0;
	}
	else if(height>HEIGHT_MID)
	{
		Amount=(height-HEIGHT_MID)/(HEIGHT_TOP-HEIGHT_MID);
		Color->x=.75f+Amount*0.25f;
		Color->y=0.5f+0.5f*Amount;
		Color->z=Amount;
	}
	else if(height>HEIGHT_LOW)
	{
		Amount=(height-HEIGHT_LOW)/(HEIGHT_MID-HEIGHT_LOW);
		Color->x=Amount*0.75f;
		Color->y=1.00f-0.5f*Amount;
		Color->z=0.0f;
	}
	else if(height>HEIGHT_ZERO)
	{
		Amount=(height-HEIGHT_ZERO)/(HEIGHT_LOW-HEIGHT_ZERO);

		Color->x=1.0f-Amount;
		Color->y=1.0f;
		Color->z=0.0f;
	}
	else if(height>HEIGHT_SHALLOW)
	{
		Amount=(height-HEIGHT_SHALLOW)/(HEIGHT_ZERO-HEIGHT_SHALLOW);
		Color->x=0.0f;
		Color->y=Amount;
		Color->z=1.0f;
	}
	else if(height>HEIGHT_DEEP)
	{
		Amount=(height-HEIGHT_DEEP)/(HEIGHT_SHALLOW-HEIGHT_DEEP);
		Color->x=0.0f;
		Color->y=0.0f;
		Color->z=Amount;
	}
	else
		(*Color)*=0.0f;

}

GLuint	Contour=0;
float			CoordGen[4];
#define CONTOUR_WIDTH	128
void GenerateContourMap()
{
	unsigned char	CTexture[CONTOUR_WIDTH*4];
	
	CoordGen[0]=0.0f;
	CoordGen[1]=0.25f;
	CoordGen[2]=0.0f;
	CoordGen[3]=0.0f;

	
	for(int i=0;i<(CONTOUR_WIDTH*4);i++)
		CTexture[i]=0;
	CTexture[3+CONTOUR_WIDTH/2]=0xff;
	CTexture[7+CONTOUR_WIDTH/2]=0xff;
	CTexture[11+CONTOUR_WIDTH/2]=0xff;

	glGenTextures(1, &Contour);
	glBindTexture(GL_TEXTURE_2D, Contour);
	

	gluBuild2DMipmaps(GL_TEXTURE_2D,4,CONTOUR_WIDTH,1,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);
	/*glTexImage1D(GL_TEXTURE_1D,0,GL_RGBA,CONTOUR_WIDTH,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);
	
	for(int i=0;i<(CONTOUR_WIDTH*4)/2;i++)
		CTexture[i]=0;
	CTexture[3]=0xff;

	glTexImage1D(GL_TEXTURE_1D,1,GL_RGBA,CONTOUR_WIDTH/2,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

	for(int i=0;i<(CONTOUR_WIDTH*4)/4;i++)
		CTexture[i]=0;
	CTexture[3]=0x80;

	glTexImage1D(GL_TEXTURE_1D,2,GL_RGBA,CONTOUR_WIDTH/4,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

	for(int i=0;i<(CONTOUR_WIDTH*4)/8;i++)
		CTexture[i]=0;
	CTexture[3]=0x40;

	glTexImage1D(GL_TEXTURE_1D,3,GL_RGBA,CONTOUR_WIDTH/8,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

	for(int i=0;i<(CONTOUR_WIDTH*4)/16;i++)
		CTexture[i]=0;
	CTexture[3]=0x20;

	glTexImage1D(GL_TEXTURE_1D,4,GL_RGBA,CONTOUR_WIDTH/16,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

	for(int i=0;i<(CONTOUR_WIDTH*4)/32;i++)
		CTexture[i]=0;
	CTexture[3]=0x10;

	glTexImage1D(GL_TEXTURE_1D,5,GL_RGBA,CONTOUR_WIDTH/32,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);

	for(int i=0;i<(CONTOUR_WIDTH*4)/64;i++)
		CTexture[i]=0;
	CTexture[3]=0x08;

	glTexImage1D(GL_TEXTURE_1D,6,GL_RGBA,CONTOUR_WIDTH/64,0,GL_RGBA,GL_UNSIGNED_BYTE,CTexture);*/
	

	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_S,GL_OBJECT_PLANE,CoordGen);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

int indexMapBuf(int x, int y)
{
	return ((y+1)/2)*9 + (y/2)*8 + x;
}


void startTimer();
void stopTimer();
int stopTimer2();

struct SMAreaHeader // 03-29-2005 By ObscuR, --schlumpf_ 02:35, 8 August 2009 (CEST)
{
/*000h*/  unsigned int flags;		// &1: MFBO, &2: unknown. in some Northrend ones.
/*004h*/  unsigned int mcin;		
/*008h*/  unsigned int mtex;		
/*00Ch*/  unsigned int mmdx;		
/*010h*/  unsigned int mmid;		
/*014h*/  unsigned int mwmo;		
/*018h*/  unsigned int mwid;		
/*01Ch*/  unsigned int mddf;		
/*020h*/  unsigned int modf;	
/*024h*/  unsigned int mfbo; 		// tbc, wotlk; only when flags&1
/*028h*/  unsigned int mh2o;		// wotlk
/*02Ch*/  unsigned int mtfx;		// wotlk
/*030h*/  unsigned int pad4;		
/*034h*/  unsigned int pad5;		
/*038h*/  unsigned int pad6;		
/*03Ch*/  unsigned int pad7;	
/*040h*/
};


MapTile::MapTile(int x0, int z0, char* filename, bool bigAlpha): x(x0), z(z0), topnode(0,0,16)
{
	xbase = x0 * TILESIZE;
	zbase = z0 * TILESIZE;
	mBigAlpha=bigAlpha;

	startTimer();
	fname=filename;

	theFile = new MPQFile(filename);
	ok = !theFile->isEof();
	
	if( !ok ) 
	{
		LogError << "Could not open tile " << x0 << ", " << z0 << " (\"" << fname << "\")" << std::endl;
		return;
	}

	if( theFile->isExternal( ) )
		Log << "Opening tile " << x0 << ", " << z0 << " (\"" << fname << "\") from disk." << std::endl;
	else
		Log << "Opening tile " << x0 << ", " << z0 << " (\"" << fname << "\") from MPQ." << std::endl;

	uint32_t fourcc;
	uint32_t size;

	SMAreaHeader Header;

	// - MVER ----------------------------------------------
	
	uint32_t version;
	
	theFile->read( &fourcc, 4 );
	theFile->seekRelative( 4 );
	theFile->read( &version, 4 );
	
	assert( fourcc == 'MVER' && version == 18 );
	
	// - MHDR ----------------------------------------------
	
	theFile->read( &fourcc, 4 );
	theFile->seekRelative( 4 );
	
	assert( fourcc == 'MHDR' );
	
	theFile->read( &Header, sizeof( SMAreaHeader ) );
	
	// - MCIN ----------------------------------------------
	
	theFile->seek( Header.mcin + 0x14 );
	theFile->read( &fourcc, 4 );
	theFile->seekRelative( 4 );
	
	assert( fourcc == 'MCIN' );
	
	for( int i = 0; i < 256; i++ ) 
	{
		theFile->read( &mcnk_offsets[i], 4 );
		theFile->read( &mcnk_sizes[i], 4 );
		theFile->seekRelative( 8 );
	}
	
	// - MTEX ----------------------------------------------
	
	theFile->seek( Header.mtex + 0x14 );
	theFile->read( &fourcc, 4 );
	theFile->read( &size, 4 );
	
	assert( fourcc == 'MTEX' );
	
	if( size )
	{
		mTexturesLoaded = false;
		
		char * lBuffer = new char[size];
		theFile->read( lBuffer, size );
		
		int lPosition = 0;
		while( lPosition < size )
		{
			mTextureFilenames.push_back( std::string( lBuffer + lPosition ) );
			lPosition += strlen( lBuffer + lPosition ) + 1;
		}
		
		delete[] lBuffer;
	}
	else
	{
		mTexturesLoaded = true;
	}
	
	// - MISC ----------------------------------------------
	
	/// TODO: Parse all chunks in the new style!

	while( !theFile->isEof() ) 
	{
		theFile->read( &fourcc, 4 );
		theFile->read( &size, 4 );
		
		size_t nextpos = theFile->getPos() + size;

		if ( fourcc == 'MMDX' ) 
		{
			// models ...
			// MMID would be relative offsets for MMDX filenames
			if(size!=0)
			{
				modelSize=size;
				modelBuffer=new char[size];
				theFile->read(modelBuffer,size);
				modelPos=modelBuffer;
				modelsLoaded=false;
				curModelID=0;
			}
			else
				modelsLoaded=true;

		}
		else if ( fourcc == 'MWMO' ) 
		{
			// map objects
			// MWID would be relative offsets for MWMO filenames			
			if(size!=0)
			{
				wmoSize=size;
				wmoBuffer=new char[size];
				theFile->read(wmoBuffer,size);
				wmoPos=wmoBuffer;
				wmosLoaded=false;
			}
			else
				wmosLoaded=true;
		}
		else if ( fourcc == 'MDDF' ) 
		{
			// model instance data
			modelNum = (int)size / 36;
			modelInstances=new ENTRY_MDDF[modelNum];
			theFile->read((unsigned char *)modelInstances,size);
		}
		else if ( fourcc == 'MODF' ) 
		{
			// wmo instance data
			wmoNum = (int)size / 64;
			wmoInstances=new ENTRY_MODF[wmoNum];
			theFile->read((unsigned char *)wmoInstances,size);
		}
		else if ( fourcc == 'MH2O' ) 
		{
			// water data
			uint8_t * lMH2O_Chunk = theFile->getPointer( );

			MH2O_Header * lHeader = reinterpret_cast<MH2O_Header*>( lMH2O_Chunk );
			
			for( int py = 0; py < 16; py++ )
				for( int px = 0; px < 16; px++ )
					for( int lLayer = 0; lLayer < lHeader[py * 16 + px].nLayers; lLayer++ )
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

							
							/// TODO: This is wrong?
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

								gLog( "%s\n", dbg.c_str());
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

		theFile->seek((int)nextpos);
	}
	
	// - MFBO ----------------------------------------------
	
	if( Header.flags & 1 )
	{
		theFile->seek( Header.mfbo + 0x14 );
		theFile->read( &fourcc, 4 );
		theFile->read( &size, 4 );
		
		assert( fourcc == 'MFBO' );
		
		short mMaximum[9], mMinimum[9];
		theFile->read( mMaximum, sizeof( short[9] ) );
		theFile->read( mMinimum, sizeof( short[9] ) );
			
		const float xPositions[] = { this->xbase, this->xbase + 266.0f, this->xbase + 533.0f };
		float yPositions[] = { this->zbase, this->zbase + 266.0f, this->zbase + 533.0f };
		
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

	Log << "Finished processing everything but MCNKs in " << stopTimer2( ) << "ms." << std::endl;

	//while(!texturesLoaded)
	//	loadTexture();

	// read individual map chunks
	/*for (int j=0; j<16; j++) {
		for (int i=0; i<16; i++) {
			f.seek((int)mcnk_offsets[j*16+i]);
			chunks[j][i].init(this, f);
		}
	}*/
	chunksLoaded = false;
	nextChunk = 0;
}

void MapTile::loadChunk( )
{	
	if( chunksLoaded )
		return;
	
	for( nextChunk = 0; nextChunk < 256; nextChunk++ ) 
	{
		theFile->seek( mcnk_offsets[nextChunk] );
		chunks[nextChunk / 16][nextChunk % 16] = new MapChunk( );
		chunks[nextChunk / 16][nextChunk % 16]->init( this, *theFile, mBigAlpha );	
	}
	
	theFile->close( );
	chunksLoaded = true;
	
	topnode.setup( this );
}


void MapTile::loadTexture( )
{
	if( mTexturesLoaded )
		return;

	for( std::vector<std::string>::iterator it = mTextureFilenames.begin( ); it != mTextureFilenames.end( ); it++ )
	{
		std::string lTexture = *it;
		/// TODO: Find a different way to do this.
		/*
		if( video.mSupportShaders )
		{
			std::string lTemp = lTexture;
			lTemp.insert( lTemp.length( ) - 4, "_s" );
			if( MPQFileExists( lTemp.c_str( ) ) )
					lTexture = lTemp;
		}
		*/
		video.textures.add( lTexture );
		textures.push_back( lTexture );
	}

	mTexturesLoaded = true;
}

void MapTile::finishTextureLoad( )
{
	while( !mTexturesLoaded )
		loadTexture( );
}

void MapTile::loadModel( )
{
	if(modelsLoaded)
		return;
	
	if(modelSize<=0)
	{
		modelsLoaded=true;
		return;
	}
	
	
	if(strlen(modelPos)>0)
	{
		string path(modelPos);
		gWorld->modelmanager.add(path);
		models.push_back(path);
		loadModelInstances(curModelID);
		curModelID++;
	}

	modelPos+=strlen(modelPos)+1;

	if(modelPos>=modelBuffer+modelSize)
	{
		Log << "Finished loading models for \"" << fname << "\"." << std::endl;
		modelsLoaded=true;
		//Need to load Model Instances now
		//loadModelInstances();
		delete modelInstances;
		delete modelBuffer;				
	}
}

void MapTile::loadModelInstances(int id)//adding do the map ony models with current modelID
{
	for (int i=0; i<modelNum; i++)
	{
		if(modelInstances[i].nameID!=id)
			continue;

		Model *model = (Model*)gWorld->modelmanager.items[gWorld->modelmanager.get(models[modelInstances[i].nameID])];
		ModelInstance inst(model, &modelInstances[i]);

		gWorld->mModelInstances.insert( pair<int,ModelInstance>( modelInstances[i].uniqueID, inst ) );
	}	
}

/*void MapTile::loadModelInstances()
{
	for (int i=0; i<modelNum; i++)
	{
		Model *model = (Model*)gWorld->modelmanager.items[gWorld->modelmanager.get(models[modelInstances[i].nameID])];
		ModelInstance inst(model, &modelInstances[i]);
		inst.modelID=modelInstances[i].nameID;
		//addModelToList(model,f);
		modelis.push_back(inst);
	}
	nMDX=modelNum;
	delete modelInstances;
}*/

void MapTile::loadWMO()//loading WMO
{
	if(wmosLoaded)
		return;	

	if(wmoSize<=0)
	{
		wmosLoaded=true;
		return;
	}
	
	if(strlen(wmoPos)>0)
	{
		string path(wmoPos);
		gWorld->wmomanager.add(path);
		wmos.push_back(path);
	}

	wmoPos+=strlen(wmoPos)+1;

	if(wmoPos>=wmoBuffer+wmoSize)
	{
		Log << "Finished loading WMOs for \"" << fname << "\"." << std::endl;
		wmosLoaded=true;
		//Need to load WMO Instances now
		loadWMOInstances();
		delete wmoBuffer;				
	}
}

void MapTile::loadWMOInstances()
{
	for (int i=0; i<wmoNum; i++)
	{
		WMO *wmo = (WMO*)gWorld->wmomanager.items[gWorld->wmomanager.get(wmos[wmoInstances[i].nameID])];
		WMOInstance inst(wmo, &wmoInstances[i]);
		
		/// TODO: Get this out.
//		wmois.push_back(inst);
		
		gWorld->mWMOInstances.insert( pair<int,WMOInstance>( wmoInstances[i].uniqueID, inst ) );
	}
	delete wmoInstances;
}

SDL_mutex * gLoadThreadMutex;

int MapTileWMOLoadThread( void * pMapTile )
{
	LogDebug << "Starting WMO Load thread with maptile at x" << std::hex << pMapTile << "." << std::endl;
	MapTile * lThis = reinterpret_cast<MapTile*>( pMapTile );

	while( !lThis->wmosLoaded )
	{
		SDL_LockMutex( gLoadThreadMutex );
		lThis->loadWMO( );
		SDL_UnlockMutex( gLoadThreadMutex );
	}

	LogDebug << "Finished WMO Load thread." << std::endl;
	return 0;
}

int MapTileModelLoadThread( void * pMapTile )
{
	LogDebug << "Starting model Load thread with maptile at x" << std::hex << pMapTile << "." << std::endl;
	MapTile * lThis = reinterpret_cast<MapTile*>( pMapTile );

	while( !lThis->modelsLoaded )
	{
	//	SDL_LockMutex( gLoadThreadMutex );
		lThis->loadModel( );
	//	SDL_UnlockMutex( gLoadThreadMutex );
	}

	LogDebug << "Finished model Load thread." << std::endl;
	return 0;
}

void MapTile::finishLoading()
{
	TimerStart();
	while(!mTexturesLoaded)
		loadTexture();
	LogDebug << "finishLoading(textures) took " << TimerStop( ) << " ms." << std::endl;
	TimerStart();
	while(!chunksLoaded)
		loadChunk();
	LogDebug << "finishLoading(chunks) took " << TimerStop( ) << " ms." << std::endl;

	while( !wmosLoaded )
		loadWMO( );

	while( !modelsLoaded )
		loadModel( );
	//MapTileModelLoadThread( this );

	//gLoadThreadMutex = SDL_CreateMutex( );

	//SDL_Thread * lModelThread = SDL_CreateThread( MapTileModelLoadThread, this );
	//SDL_Thread * lWMOThread = SDL_CreateThread( MapTileWMOLoadThread, this );

	//SDL_WaitThread( lModelThread, 0 );
	//SDL_WaitThread( lWMOThread, 0 );

	//SDL_DestroyMutex( gLoadThreadMutex );

}

MapTile::~MapTile()
{
	if (!ok) 
		return;

	LogDebug << "Unloading tile " << x << "," << z << "." << std::endl;

	if(chunksLoaded)
	{
		topnode.cleanup();

		for (int j=0; j<16; j++) {
			for (int i=0; i<16; i++) {
				delete chunks[j][i];
			}
		}
	}
	else
	{
		for(int i=0;i<nextChunk;i++)
			delete chunks[i/16][i%16];
	}

	for (vector<string>::iterator it = textures.begin(); it != textures.end(); ++it) {
        video.textures.delbyname(*it);
	}

	for (vector<string>::iterator it = wmos.begin(); it != wmos.end(); ++it) {
		gWorld->wmomanager.delbyname(*it);
	}

	for (vector<string>::iterator it = models.begin(); it != models.end(); ++it) {
		gWorld->modelmanager.delbyname(*it);
	}
}

extern float groundBrushRadius;
extern int terrainMode;

void MapTile::draw()
{
	if (!ok) 
		return;
	
	if(!mTexturesLoaded)
		finishTextureLoad();
	
	while(!chunksLoaded)
		loadChunk();


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
			chunks[j][i]->draw();
}

void MapTile::drawSelect()
{
	if (!ok) 
		return;
	
	/// TODO: Do we really need to load textures for selecting? ..
	if(!mTexturesLoaded)
		finishTextureLoad();
	
	while(!chunksLoaded)
		loadChunk();
	
	for (int j=0; j<16; j++)
		for (int i=0; i<16; i++)
			chunks[j][i]->drawSelect();
}

void MapTile::drawLines()//draw red lines around the square of a chunk
{
	if (!ok) 
		return;
	
	while(!chunksLoaded)
		loadChunk();

	glDisable(GL_COLOR_MATERIAL);
	
	for (int j=0; j<16; j++)
		for (int i=0; i<16; i++)
			chunks[j][i]->drawLines();
	
	glEnable(GL_COLOR_MATERIAL);
}

void MapTile::drawMFBO()
{
	GLshort lIndices[] = { 4, 1, 2, 5, 8, 7, 6, 3, 0, 1, 0, 3, 6, 7, 8, 5, 2, 1 };

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


void enableWaterShader();

void MapTile::drawWater()
{
	if (!ok) 
		return;

	/// TODO: Do we really need textures for drawing water? ..
	if(!mTexturesLoaded)
		finishTextureLoad();

	while(!chunksLoaded)
		loadChunk();

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	
	for( std::vector<Liquid*>::iterator liq = mLiquids.begin( ); liq != mLiquids.end( ); liq++ )
		( *liq )->draw( );

	glEnable(GL_COLOR_MATERIAL);
}

// This is for the 2D mode only.
void MapTile::drawTextures()
{
	if (!ok) 
		return;
	
	if(!mTexturesLoaded)
		finishTextureLoad();

	while(!chunksLoaded)
		loadChunk();

	float xOffset,yOffset;

	glPushMatrix();
	xOffset=xbase/CHUNKSIZE;
	yOffset=zbase/CHUNKSIZE;
	glTranslatef(xOffset,yOffset,0);
	
	//glTranslatef(-8,-8,0);
	
	for (int j=0; j<16; j++) {
		for (int i=0; i<16; i++) {
			if(((i+1+xOffset)>gWorld->minX)&&((j+1+yOffset)>gWorld->minY)&&((i+xOffset)<gWorld->maxX)&&((j+yOffset)<gWorld->maxY))
				chunks[j][i]->drawTextures();
		}
	}
	glPopMatrix();
}

// - MapChunk ----------------------------------------------------------------------------------------------------

/// TODO: Make this the constructor. Do it!
void MapChunk::init(MapTile* maintile, MPQFile &f,bool bigAlpha)
{	
	mt=maintile;
	mBigAlpha=bigAlpha;

	uint32_t fourcc;
	uint32_t size;
	
	f.read(&fourcc, 4);
	f.read(&size, 4);

	assert( fourcc == 'MCNK' );
	
	size_t lastpos = f.getPos() + size;

	f.read(&header, 0x80);

	Flags = header.flags;
	areaID = header.areaid;
	
    zbase = header.zpos;
    xbase = header.xpos;
    ybase = header.ypos;

	px = header.ix;
	py = header.iy;

	holes = header.holes;

	hasholes = (holes != 0);

	/*
	if (hasholes) {
		gLog("Holes: %d\n", holes);
		int k=1;
		for (int j=0; j<4; j++) {
			for (int i=0; i<4; i++) {
				gLog((holes & k)?"1":"0");
				k <<= 1;
			}
			gLog("\n");
		}
	}
	*/

	// correct the x and z values ^_^
	zbase = zbase*-1.0f + ZEROPOINT;
	xbase = xbase*-1.0f + ZEROPOINT;

	vmin = Vec3D( 9999999.0f, 9999999.0f, 9999999.0f);
	vmax = Vec3D(-9999999.0f,-9999999.0f,-9999999.0f);
	glGenTextures(3, alphamaps);
	
	while (f.getPos() < lastpos) {
		f.read(&fourcc,4);
		f.read(&size, 4);

		size_t nextpos = f.getPos() + size;

		if ( fourcc == 'MCNR' ) {
			nextpos = f.getPos() + 0x1C0; // size fix
			// normal vectors
			char nor[3];
			Vec3D *ttn = tn;
			for (int j=0; j<17; j++) {
				for (int i=0; i<((j%2)?8:9); i++) {
					f.read(nor,3);
					// order Z,X,Y ?
					//*ttn++ = Vec3D((float)nor[0]/127.0f, (float)nor[2]/127.0f, (float)nor[1]/127.0f);
					*ttn++ = Vec3D(-(float)nor[1]/127.0f, (float)nor[2]/127.0f, -(float)nor[0]/127.0f);
				}
			}
		}
		else if ( fourcc == 'MCVT' ) {
			Vec3D *ttv = tv;

			// vertices
			for (int j=0; j<17; j++) {
				for (int i=0; i<((j%2)?8:9); i++) {
					float h,xpos,zpos;
					f.read(&h,4);
					xpos = i * UNITSIZE;
					zpos = j * 0.5f * UNITSIZE;
					if (j%2) {
                        xpos += UNITSIZE*0.5f;
					}
					Vec3D v = Vec3D(xbase+xpos, ybase+h, zbase+zpos);
					*ttv++ = v;
					if (v.y < vmin.y) vmin.y = v.y;
					if (v.y > vmax.y) vmax.y = v.y;
				}
			}

			vmin.x = xbase;
			vmin.z = zbase;
			vmax.x = xbase + 8 * UNITSIZE;
			vmax.z = zbase + 8 * UNITSIZE;
			r = (vmax - vmin).length() * 0.5f;

		}
		else if ( fourcc == 'MCLY' ) {
			// texture info
			nTextures = (int)size / 16;
			//gLog("=\n");
			for (int i=0; i<nTextures; i++) {
				f.read(&tex[i],4);
				f.read(&texFlags[i], 4);
				f.read(&MCALoffset[i], 4);
				f.read(&effectID[i], 4);

				if (texFlags[i] & 0x80) {
                    animated[i] = texFlags[i];
				} else {
					animated[i] = 0;
				}
				textures[i] = video.textures.get(mt->textures[tex[i]]);
			}
		}
		else if ( fourcc == 'MCSH' ) {
			// shadow map 64 x 64

			f.read( mShadowMap, 0x200 );
			f.seekRelative( -0x200 );

			unsigned char sbuf[64*64], *p, c[8];
			p = sbuf;
			for (int j=0; j<64; j++) {
				f.read(c,8);
				for (int i=0; i<8; i++) {
					for (int b=0x01; b!=0x100; b<<=1) {
						*p++ = (c[i] & b) ? 85 : 0;
					}
				}
			}
			glGenTextures(1, &shadow);
			glBindTexture(GL_TEXTURE_2D, shadow);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, sbuf);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		}
		else if ( fourcc == 'MCAL' ) 
		{
			unsigned int MCALbase = f.getPos();
			for( int layer = 0; layer < header.nLayers; layer++ )
			{
				if( texFlags[layer] & 0x100 )
				{

					f.seek( MCALbase + MCALoffset[layer] );
		
					if( texFlags[layer] & 0x200 )
					{	// compressed
						glBindTexture(GL_TEXTURE_2D, alphamaps[layer-1]);

						// 21-10-2008 by Flow
						unsigned offI = 0; //offset IN buffer
						unsigned offO = 0; //offset OUT buffer
						uint8_t* buffIn = f.getPointer(); // pointer to data in adt file
						char buffOut[4096]; // the resulting alpha map

						while( offO < 4096 )
						{
						  // fill or copy mode
						  bool fill = buffIn[offI] & 0x80;
						  unsigned n = buffIn[offI] & 0x7F;
						  offI++;
						  for( unsigned k = 0; k < n; k++ )
						  {
							if (offO == 4096) break;
							buffOut[offO] = buffIn[offI];
							offO++;
							if( !fill )
							  offI++;
						  }
						  if( fill ) offI++;
						}

						glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, buffOut);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					}
					else if(mBigAlpha){
						// not compressed
						glBindTexture(GL_TEXTURE_2D, alphamaps[layer-1]);
						unsigned char *p;
						uint8_t *abuf = f.getPointer();
						p = amap[layer-1];
						for (int j=0; j<64; j++) {
							for (int i=0; i<64; i++) {
								*p++ = *abuf++;
							}

						}
						memcpy(amap[layer-1]+63*64,amap[layer-1]+62*64,64);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[layer-1]);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						f.seekRelative(0x1000);
					}
					else
					{	
						// not compressed
						glBindTexture(GL_TEXTURE_2D, alphamaps[layer-1]);
						unsigned char *p;
						uint8_t *abuf = f.getPointer();
						p = amap[layer-1];
						for (int j=0; j<63; j++) {
							for (int i=0; i<32; i++) {
								unsigned char c = *abuf++;
								if(i!=31)
								{
									*p++ = (unsigned char)((255*((int)(c & 0x0f)))/0x0f);
									*p++ = (unsigned char)((255*((int)(c & 0xf0)))/0xf0);
								}
								else
								{
									*p++ = (unsigned char)((255*((int)(c & 0x0f)))/0x0f);
									*p++ = (unsigned char)((255*((int)(c & 0x0f)))/0x0f);
								}
							}

						}
						memcpy(amap[layer-1]+63*64,amap[layer-1]+62*64,64);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[layer-1]);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						f.seekRelative(0x800);
					}
				}
			}
		}
		else if ( fourcc == 'MCLQ' ) {
			// liquid / water level
			f.read(&fourcc,4);
			if( fourcc != 'MCSE' ||  fourcc != 'MCNK' || header.sizeLiquid == 8 ) {
				haswater = false;
			}
			else {
				haswater = true;
				f.seekRelative(-4);
				f.read(waterlevel,8);//2 values - Lowest water Level, Highest Water Level

				if (waterlevel[1] > vmax.y) vmax.y = waterlevel[1];
				//if (waterlevel < vmin.y) haswater = false;

				//f.seekRelative(4);

				Liquid * lq = new Liquid(8, 8, Vec3D(xbase, waterlevel[1], zbase));
				//lq->init(f);
				lq->initFromTerrain(f, header.flags);

				this->mt->mLiquids.push_back( lq );

				/*
				// let's output some debug info! ( '-')b
				string lq = "";
				if (flags & 4) lq.append(" river");
				if (flags & 8) lq.append(" ocean");
				if (flags & 16) lq.append(" magma");
				if (flags & 32) lq.append(" slime?");
				gLog("LQ%s (base:%f)\n", lq.c_str(), waterlevel);
				*/

			}
			// we're done here!
			break;
		}
		else if( fourcc == 'MCCV' )
		{
			/// TODO: implement
		}
		f.seek(nextpos);
	}

	// create vertex buffers
	glGenBuffers(1,&vertices);
	glGenBuffers(1,&normals);

	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, mapbufsize*3*sizeof(float), tv, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, normals);
	glBufferData(GL_ARRAY_BUFFER, mapbufsize*3*sizeof(float), tn, GL_STATIC_DRAW);

	/*if (hasholes) */
	//	initStrip();
	
	//else {
		strip = gWorld->mapstrip;
		striplen = 16*18 + 7*2 + 8*2; //stripsize;
	//}
	

	this->mt = mt;

	vcenter = (vmin + vmax) * 0.5f;

	nameID = SelectionNames.add( this );



	Vec3D *ttv = tm;

	// vertices
	for (int j=0; j<17; j++) {
		for (int i=0; i<((j%2)?8:9); i++) {
			float xpos,zpos;
				//f.read(&h,4);
			xpos = i * 0.125f;
			zpos = j * 0.5f * 0.125f;
			if (j%2) {
                 xpos += 0.125f*0.5f;
			}
			Vec3D v = Vec3D(xpos+px, zpos+py,-1);
			*ttv++ = v;
		}
	}
	
	if( ( Flags & 1 ) == 0 )
	{
		/** We have no shadow map (MCSH), so we got no shadows at all!  **
		 ** This results in everything being black.. Yay. Lets fake it! **/
		for( size_t i = 0; i < 512; i++ )
			mShadowMap[i] = 0;

		unsigned char sbuf[64*64];
		for( size_t j = 0; j < 4096; j++ ) 
			sbuf[j] = 0;

		glGenTextures( 1, &shadow );
		glBindTexture( GL_TEXTURE_2D, shadow );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, sbuf );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	}

	float ShadowAmount;
	for (int j=0; j<mapbufsize;j++)
	{
		//tm[j].z=tv[j].y;
		ShadowAmount=1.0f-(-tn[j].x+tn[j].y-tn[j].z);
		if(ShadowAmount<0)
			ShadowAmount=0.0f;
		if(ShadowAmount>1.0)
			ShadowAmount=1.0f;
		ShadowAmount*=0.5f;
		//ShadowAmount=0.2;
		ts[j].x=0;
		ts[j].y=0;
		ts[j].z=0;
		ts[j].w=ShadowAmount;
	}

	glGenBuffers(1,&minimap);
	glGenBuffers(1,&minishadows);
	
	glBindBuffer(GL_ARRAY_BUFFER, minimap);
	glBufferData(GL_ARRAY_BUFFER, mapbufsize*3*sizeof(float), tm, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, minishadows);
	glBufferData(GL_ARRAY_BUFFER, mapbufsize*4*sizeof(float), ts, GL_STATIC_DRAW);
}

void MapChunk::loadTextures()
{
	return;
	for(int i=0;i<nTextures;i++)
		textures[i] = video.textures.get(mt->textures[tex[i]]);
}

#define texDetail 8.0f

void SetAnim(int anim)
{
	if (anim) {
		glActiveTexture(GL_TEXTURE0);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();

		// note: this is ad hoc and probably completely wrong
		int spd = (anim & 0x08) | ((anim & 0x10) >> 2) | ((anim & 0x20) >> 4) | ((anim & 0x40) >> 6);
		int dir = anim & 0x07;
		const float texanimxtab[8] = {0, 1, 1, 1, 0, -1, -1, -1};
		const float texanimytab[8] = {1, 1, 0, -1, -1, -1, 0, 1};
		float fdx = -texanimxtab[dir], fdy = texanimytab[dir];

		int animspd = (int)(200.0f * 8.0f);
		float f = ( ((int)(gWorld->animtime*(spd/15.0f))) % animspd) / (float)animspd;
		glTranslatef(f*fdx,f*fdy,0);
	}
}

void RemoveAnim(int anim)
{
	if (anim) {
        glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glActiveTexture(GL_TEXTURE1);
	}
}

#define	TEX_RANGE 62.0f/64.0f

void MapChunk::drawTextures()
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	if(nTextures>0)
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
	}

	SetAnim(animated[0]);
	glBegin(GL_TRIANGLE_STRIP);	
	glTexCoord2f(0.0f,texDetail);
	glVertex3f((float)px,py+1.0f,-2.0f);	
	glTexCoord2f(0.0f,0.0f);
	glVertex3f((float)px,(float)py,-2.0f);
	glTexCoord2f(texDetail,texDetail);
	glVertex3f((float)px+1.0f,(float)py+1.0f,-2.0f);	
	glTexCoord2f(texDetail,0.0f);
	glVertex3f((float)px+1.0f,(float)py,-2.0f);	
	glEnd();
	RemoveAnim(animated[0]);

	if (nTextures>1) {
		//glDepthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
		//glDepthMask(GL_FALSE);
	}
	for(int i=1;i<nTextures;i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, alphamaps[i-1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		SetAnim(animated[i]);

		glBegin(GL_TRIANGLE_STRIP);	
		glMultiTexCoord2f(GL_TEXTURE0,texDetail,0.0f);
		glMultiTexCoord2f(GL_TEXTURE1,TEX_RANGE,0.0f);
		glVertex3f(px+1.0f,(float)py,-2.0f);
		glMultiTexCoord2f(GL_TEXTURE0,0.0f,0.0f);
		glMultiTexCoord2f(GL_TEXTURE1,0.0f,0.0f);
		glVertex3f((float)px,(float)py,-2.0f);		
		glMultiTexCoord2f(GL_TEXTURE0,texDetail,texDetail);
		glMultiTexCoord2f(GL_TEXTURE1,TEX_RANGE,TEX_RANGE);
		glVertex3f(px+1.0f,py+1.0f,-2.0f);	
		glMultiTexCoord2f(GL_TEXTURE0,0.0f,texDetail);
		glMultiTexCoord2f(GL_TEXTURE1,0.0f,TEX_RANGE);
		glVertex3f((float)px,py+1.0f,-2.0f);	
		glEnd();

		RemoveAnim(animated[i]);

	}
	
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);


	glBindBuffer(GL_ARRAY_BUFFER, minimap);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, minishadows);
	glColorPointer(4, GL_FLOAT, 0, 0);
	
	
	glDrawElements(GL_TRIANGLE_STRIP, stripsize2, GL_UNSIGNED_SHORT, gWorld->mapstrip2);


	
}


void MapChunk::initStrip()
{
	strip = new short[256]; // TODO: figure out exact length of strip needed
	short *s = strip;
	bool first = true;
	for (int y=0; y<4; y++) {
		for (int x=0; x<4; x++) {
			if (!isHole(x, y)) {
				// draw tile here
				// this is ugly but sort of works
				int i = x*2;
				int j = y*4;
				for (int k=0; k<2; k++) {
					if (!first) {
						*s++ = indexMapBuf(i,j+k*2);
					} else first = false;
					for (int l=0; l<3; l++) {
						*s++ = indexMapBuf(i+l,j+k*2);
						*s++ = indexMapBuf(i+l,j+k*2+2);
					}
					*s++ = indexMapBuf(i+2,j+k*2+2);
				}
			}
		}
	}
	striplen = (int)(s - strip);
}


MapChunk::~MapChunk( )
{
	// unload alpha maps
	glDeleteTextures( 3, alphamaps );
	// shadow maps, too
	glDeleteTextures( 1, &shadow );

	// delete VBOs
	glDeleteBuffers( 1, &vertices );
	glDeleteBuffers( 1, &normals );

	//if( strip ) 
	//	delete strip;

	if( nameID != -1 )
	{
		SelectionNames.del( nameID );
		nameID = -1;
	}
}

bool MapChunk::GetVertex(float x,float z, Vec3D *V)
{
	float xdiff,zdiff;

	xdiff=x-xbase;
	zdiff=z-zbase;
	
	int row, column;
	row=int(zdiff/(UNITSIZE*0.5)+0.5);
	column=int((xdiff-UNITSIZE*0.5*(row%2))/UNITSIZE+0.5);
	if((row<0)||(column<0)||(row>16)||(column>((row%2)?8:9)))
		return false;

	*V=tv[17*(row/2)+((row%2)?9:0)+column];
	return true;
}

unsigned short OddStrips[8*18];
unsigned short EvenStrips[8*18];
unsigned short LineStrip[32];
unsigned short HoleStrip[128];

void CreateStrips()
{
	unsigned short Temp[18];
	int j;

	for(int i=0;i<8;i++)
	{
		OddStrips[i*18+0]=i*17+17;
		for(j=0;j<8;j++)
		{
			OddStrips[i*18+2*j+1]=i*17+j;
			OddStrips[i*18+2*j+2]=i*17+j+9;
			EvenStrips[i*18+2*j]=i*17+17+j;
			EvenStrips[i*18+2*j+1]=i*17+9+j;			
		}
		OddStrips[i*18+17]=i*17+8;
		EvenStrips[i*18+16]=i*17+17+8;
		EvenStrips[i*18+17]=i*17+8;
	}

	//Reverse the order whoops
	for(int i=0;i<8;i++)
	{
		for(j=0;j<18;j++)
			Temp[17-j]=OddStrips[i*18+j];
		memcpy(&OddStrips[i*18],Temp,sizeof(unsigned short)*18);
		for(j=0;j<18;j++)
			Temp[17-j]=EvenStrips[i*18+j];
		memcpy(&EvenStrips[i*18],Temp,sizeof(unsigned short)*18);

	}

	for(int i=0;i<32;i++)
	{
		if(i<9)
			LineStrip[i]=i;
		else if(i<17)
			LineStrip[i]=8+(i-8)*17;
		else if(i<25)
			LineStrip[i]=145-(i-15);
		else
			LineStrip[i]=(32-i)*17;
	}


	int iferget = 0;
	
	for( size_t i = 34; i < 43; i++ )
 		HoleStrip[iferget++] = i;
	
	for( size_t i = 68; i < 77; i++ )
		HoleStrip[iferget++] = i;

	for( size_t i = 102; i < 111; i++ )
 		HoleStrip[iferget++] = i;

	
	HoleStrip[iferget++]=2;
	HoleStrip[iferget++]=19;
	HoleStrip[iferget++]=36;
	HoleStrip[iferget++]=53;
	HoleStrip[iferget++]=70;
	HoleStrip[iferget++]=87;
	HoleStrip[iferget++]=104;
	HoleStrip[iferget++]=121;
	HoleStrip[iferget++]=138;

	HoleStrip[iferget++]=4;
	HoleStrip[iferget++]=21;
	HoleStrip[iferget++]=38;
	HoleStrip[iferget++]=55;
	HoleStrip[iferget++]=72;
	HoleStrip[iferget++]=89;
	HoleStrip[iferget++]=106;
	HoleStrip[iferget++]=123;
	HoleStrip[iferget++]=140;
	
	HoleStrip[iferget++]=6;
	HoleStrip[iferget++]=23;
	HoleStrip[iferget++]=40;
	HoleStrip[iferget++]=57;
	HoleStrip[iferget++]=74;
	HoleStrip[iferget++]=91;
	HoleStrip[iferget++]=108;
	HoleStrip[iferget++]=125;
	HoleStrip[iferget++]=142;

}

void MapChunk::drawColor()
{
	
	if (!gWorld->frustum.intersects(vmin,vmax)) return;
	float mydist = (gWorld->camera - vcenter).length() - r;
	//if (mydist > gWorld->mapdrawdistance2) return;
	if (mydist > gWorld->culldistance) {
		if (gWorld->uselowlod) this->drawNoDetail();
		return;
	}

	if( !hasholes )
	{
		if ( mydist < gWorld->highresdistance2 ) 
		{
			strip = gWorld->mapstrip2;
			striplen = stripsize2;
		} 
		else 
		{
			strip = gWorld->mapstrip;
			striplen = stripsize;
		}
	}

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	//glDisable(GL_LIGHTING);

	Vec3D Color;
	glBegin(GL_TRIANGLE_STRIP);
	for(int i=0;i<striplen;i++)
	{
		HeightColor(tv[strip[i]].y,&Color);
		glColor3fv(&Color.x);
		glNormal3fv(&tn[strip[i]].x);
		glVertex3fv(&tv[strip[i]].x);
	}
	glEnd();
	//glEnable(GL_LIGHTING);
}


void MapChunk::drawPass(int anim)
{
	if (anim) 
	{
		glActiveTexture(GL_TEXTURE0);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();

		// note: this is ad hoc and probably completely wrong
		int spd = (anim & 0x08) | ((anim & 0x10) >> 2) | ((anim & 0x20) >> 4) | ((anim & 0x40) >> 6);
		int dir = anim & 0x07;
		const float texanimxtab[8] = {0, 1, 1, 1, 0, -1, -1, -1};
		const float texanimytab[8] = {1, 1, 0, -1, -1, -1, 0, 1};
		float fdx = -texanimxtab[dir], fdy = texanimytab[dir];

		int animspd = (int)(200.0f * detail_size);
		float f = ( ((int)(gWorld->animtime*(spd/15.0f))) % animspd) / (float)animspd;
		glTranslatef(f*fdx,f*fdy,0);
	}
	
	if(!this->hasholes)
	/*{
		glFrontFace(GL_CW);
		for(int i=0;i<8;i++)
		{
			glDrawElements(GL_TRIANGLE_STRIP, 18, GL_UNSIGNED_SHORT, &OddStrips[i*18]);
			glDrawElements(GL_TRIANGLE_STRIP, 18, GL_UNSIGNED_SHORT, &EvenStrips[i*18]);
		}
		glFrontFace(GL_CCW);
	}
	 else*/
		 glDrawElements(GL_TRIANGLE_STRIP, striplen, GL_UNSIGNED_SHORT, strip);
	

	if (anim) 
	{
        glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glActiveTexture(GL_TEXTURE1);
	}
}

void MapChunk::drawLines()
{
	if (!gWorld->frustum.intersects(vmin,vmax))		return;
	float mydist = (gWorld->camera - vcenter).length() - r;
	if (mydist > gWorld->mapdrawdistance2) return;

	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	
	glPushMatrix();
	glColor4f(1.0,0.0,0.0f,0.5f);
	glTranslatef(0.0f,0.05f,0.0f);
	glEnable (GL_LINE_SMOOTH);	
	glLineWidth(1.5);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	if((px!=15)&&(py!=0))
	{
		glDrawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, LineStrip);
	}
	else if((px==15)&&(py==0))
	{
		glColor4f(0.0,1.0,0.0f,0.5f);
		glDrawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, LineStrip);
	}
	else if(px==15)
	{
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, LineStrip);
		glColor4f(0.0,1.0,0.0f,0.5f);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &LineStrip[8]);
	}
	else if(py==0)
	{
		glColor4f(0.0,1.0,0.0f,0.5f);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, LineStrip);
		glColor4f(1.0,0.0,0.0f,0.5f);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &LineStrip[8]);
	}
		
	if(Environment::getInstance()->view_holelines)
	{	
		// Draw hole lines if view_subchunk_lines is true
		glColor4f(0.0,0.0,1.0f,0.5f);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, HoleStrip);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[9]);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[18]);		
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[27]);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[36]);
		glDrawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[45]);
	}

	glPopMatrix();
	glEnable(GL_LIGHTING);
	glColor4f(1,1,1,1);
}

void MapChunk::drawContour()
{
	if(!DrawMapContour)
		return;
	glColor4f(1,1,1,1);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_ALPHA_TEST);
	if(Contour==0)
		GenerateContourMap();
	glBindTexture(GL_TEXTURE_2D, Contour);
	
	glEnable(GL_TEXTURE_GEN_S);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
	glTexGenfv(GL_S,GL_OBJECT_PLANE,CoordGen);
	

	drawPass(0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_GEN_S);
}

void MapChunk::draw()
{
	if (!gWorld->frustum.intersects(vmin,vmax))		return;
	float mydist = (gWorld->camera - vcenter).length() - r;
	//if (mydist > gWorld->mapdrawdistance2) return;
	/*if (mydist > gWorld->culldistance+75) {
		if (gWorld->uselowlod) this->drawNoDetail();
		return;
	}*/

	if( !hasholes )
	{
		if ( mydist < gWorld->highresdistance2 ) 
		{
			strip = gWorld->mapstrip2;
			striplen = stripsize2;
		} 
		else 
		{
			strip = gWorld->mapstrip;
			striplen = stripsize;
		}
	}
	

	// setup vertex buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, normals);
	glNormalPointer(GL_FLOAT, 0, 0);
	// ASSUME: texture coordinates set up already

	// first pass: base texture
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);

//	if( nameID == -1 )
//		nameID = SelectionNames.add( this );
//	glPushName(nameID);

	if (nTextures==0)
	{
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);

		glColor3f(1.0f,1.0f,1.0f);
	//	glDisable(GL_LIGHTING);
	}

	glEnable(GL_LIGHTING);
	drawPass(animated[0]);
//	glPopName();

	if (nTextures>1) {
		//glDepthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
		glDepthMask(GL_FALSE);
	}

	// additional passes: if required
	for (int i=0; i<nTextures-1; i++) {
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[i+1]);

		// this time, use blending:
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, alphamaps[i]);

		drawPass(animated[i+1]);
	}

	if (nTextures>1) {
		//glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
	}
	
	// shadow map
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	Vec3D shc = gWorld->skies->colorSet[WATER_COLOR_DARK] * 0.3f;
	glColor4f(shc.x,shc.y,shc.z,1);

	//glColor4f(1,1,1,1);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, shadow);
	glEnable(GL_TEXTURE_2D);

	drawPass(0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	drawContour();

	
	if(drawFlags)
	{
		if(Flags&0x02)
		{
			glColor4f(1,0,0,0.2f);
			drawPass(0);
		}

		if(Flags&0x04)
		{
			glColor4f(0,0.5f,1,0.2f);
			drawPass(0);
		}

		if(Flags&0x08)
		{
			glColor4f(0,0,0.8f,0.2f);
			drawPass(0);
		}
		if(Flags&0x10)
		{
			glColor4f(1,0.5f,0,0.2f);
			drawPass(0);
		}
	}
	if( gWorld->IsSelection( eEntry_MapChunk ) && gWorld->GetCurrentSelection( )->data.mapchunk == this && terrainMode != 3 )
	{
		int poly = gWorld->GetCurrentSelectedTriangle( );

		//glClear( /*GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT |*/ GL_STENCIL_BUFFER_BIT );
		/*glColorMask( false, false, false, false );
		glEnable(GL_STENCIL_TEST);
			glStencilFunc(GL_ALWAYS, 1, 1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glDisable(GL_DEPTH_TEST);
				glDisable(GL_TEXTURE_2D);
					glDrawElements(GL_TRIANGLE_STRIP, striplen, GL_UNSIGNED_SHORT, strip);
					glStencilFunc(GL_EQUAL, 1, 1);
					glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
					glDisable(GL_CULL_FACE);
						renderCylinder_convenient(tv[strip[poly]].x, tv[strip[poly]].y-10, tv[strip[poly]].z, tv[strip[poly]].x,tv[strip[poly]].y+10, tv[strip[poly]].z, 10.0f,100);
					glEnable(GL_CULL_FACE);
				glEnable(GL_TEXTURE_2D);
			glEnable(GL_DEPTH_TEST);
			glColorMask(true, true, true, true);
			glStencilFunc(GL_EQUAL, 1, 1);
			glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
			glDisable(GL_CULL_FACE);
				renderCylinder_convenient(tv[strip[poly]].x, tv[strip[poly]].y-10, tv[strip[poly]].z, tv[strip[poly]].x,tv[strip[poly]].y+10, tv[strip[poly]].z, 10.0f,100);
			glEnable(GL_CULL_FACE);
		glDisable(GL_STENCIL_TEST );		*/

	/*	glColor4f(1,0.3f,0.3f,0.2f);
		
		float x = ( Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly]].x + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+1]].x + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+2]].x ) / 3;
		float y = ( Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly]].y + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+1]].y + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+2]].y ) / 3;
		float z = ( Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly]].z + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+1]].z + Selection->data.mapchunk->tv[Selection->data.mapchunk->strip[poly+2]].z ) / 3;
		glDisable(GL_CULL_FACE);
		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);
		renderCylinder_convenient( x, y, z, groundBrushRadius, 100 );
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);*/
		
		//glColor4f(1.0f,0.3f,0.3f,0.2f);
		glColor4f( 1.0f, 1.0f, 0.0f, 1.0f );

		glPushMatrix( );

		glDisable( GL_CULL_FACE );
		glDepthMask( false );
		glDisable( GL_DEPTH_TEST );
		glBegin( GL_TRIANGLES );
		glVertex3fv( tv[gWorld->mapstrip2[poly + 0]] );
		glVertex3fv( tv[gWorld->mapstrip2[poly + 1]] );
		glVertex3fv( tv[gWorld->mapstrip2[poly + 2]] );
		glEnd( );		
		glEnable( GL_CULL_FACE );
		glEnable( GL_DEPTH_TEST );
		glDepthMask( true );

		glPopMatrix( );
	}
	
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	
	glEnable( GL_LIGHTING );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	/*
	//////////////////////////////////
	// debugging tile flags:
	GLfloat tcols[8][4] = {	{1,1,1,1},
		{1,0,0,1}, {1, 0.5f, 0, 1}, {1, 1, 0, 1},
		{0,1,0,1}, {0,1,1,1}, {0,0,1,1}, {0.8f, 0, 1,1}
	};
	glPushMatrix();
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	glTranslatef(xbase, ybase, zbase);
	for (int i=0; i<8; i++) {
		int v = 1 << (7-i);
		for (int j=0; j<4; j++) {
			if (animated[j] & v) {
				glBegin(GL_TRIANGLES);
				glColor4fv(tcols[i]);

				glVertex3f(i*2.0f, 2.0f, j*2.0f);
				glVertex3f(i*2.0f+1.0f, 2.0f, j*2.0f);
				glVertex3f(i*2.0f+0.5f, 4.0f, j*2.0f);

				glEnd();
			}
		}
	}
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glColor4f(1,1,1,1);
	glPopMatrix();*/
	
	
}

void MapChunk::drawNoDetail( )
{
	glActiveTexture( GL_TEXTURE1 );
	glDisable( GL_TEXTURE_2D );
	glActiveTexture(GL_TEXTURE0 );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );

	//glColor3fv(gWorld->skies->colorSet[FOG_COLOR]);
	//glColor3f(1,0,0);
	//glDisable(GL_FOG);

	// low detail version
	glBindBuffer( GL_ARRAY_BUFFER, vertices );
	glVertexPointer( 3, GL_FLOAT, 0, 0 );
	glDisableClientState( GL_NORMAL_ARRAY );
	glDrawElements( GL_TRIANGLE_STRIP, stripsize, GL_UNSIGNED_SHORT, gWorld->mapstrip );
	glEnableClientState( GL_NORMAL_ARRAY );

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	//glEnable(GL_FOG);

	glEnable( GL_LIGHTING );
	glActiveTexture( GL_TEXTURE1 );
	glEnable( GL_TEXTURE_2D );
	glActiveTexture( GL_TEXTURE0 );
	glEnable( GL_TEXTURE_2D );
}

void MapChunk::drawSelect( )
{
	if( !gWorld->frustum.intersects( vmin, vmax ) ) 
		return;

/*	float mydist = (gWorld->camera - vcenter).length() - r;
	//if (mydist > gWorld->mapdrawdistance2) return;
	if (mydist > gWorld->culldistance)
		return;*/


	if( nameID == -1 )
		nameID = SelectionNames.add( this );

	glDisable( GL_CULL_FACE );
	glPushName( nameID );
	glBegin( GL_TRIANGLE_STRIP );
	for( int i = 0; i < stripsize2; i++ )
		glVertex3fv( tv[gWorld->mapstrip2[i]] );
	glEnd( );
	glPopName( );
	glEnable( GL_CULL_FACE );	
}

void MapChunk::drawSelect2( )
{
	glDisable( GL_CULL_FACE );
	for( int i = 0; i < stripsize2 - 2; i++ )
	{
		glPushName( i );
		glBegin( GL_TRIANGLES );
		glVertex3fv( tv[gWorld->mapstrip2[i + 0]] );
		glVertex3fv( tv[gWorld->mapstrip2[i + 1]] );
		glVertex3fv( tv[gWorld->mapstrip2[i + 2]] );
		glEnd( );
		glPopName( );	
	}
	glEnable( GL_CULL_FACE );	
}

void MapChunk::getSelectionCoord( float *x, float *z )
{
	int Poly = gWorld->GetCurrentSelectedTriangle( );
	if( Poly + 2 > stripsize2 )
	{
		*x = -1000000.0f;
		*z = -1000000.0f;
		return;
	}
	*x = ( tv[gWorld->mapstrip2[Poly + 0]].x + tv[gWorld->mapstrip2[Poly + 1]].x + tv[gWorld->mapstrip2[Poly + 2]].x ) / 3;
	*z = ( tv[gWorld->mapstrip2[Poly + 0]].z + tv[gWorld->mapstrip2[Poly + 1]].z + tv[gWorld->mapstrip2[Poly + 2]].z ) / 3;
}

float MapChunk::getSelectionHeight( )
{
	int Poly = gWorld->GetCurrentSelectedTriangle( );
	if( Poly + 2 < striplen )
		return ( tv[gWorld->mapstrip2[Poly + 0]].y + tv[gWorld->mapstrip2[Poly + 1]].y + tv[gWorld->mapstrip2[Poly + 2]].y ) / 3;
	LogError << "Getting selection height fucked up because the selection was bad. " << Poly << "%i with striplen of " << stripsize2 << "." << std::endl;
	return 0.0f;
}

Vec3D MapChunk::GetSelectionPosition( )
{
	int Poly = gWorld->GetCurrentSelectedTriangle( );
	if( Poly + 2 > stripsize2 )
	{
		LogError << "Getting selection position fucked up because the selection was bad. " << Poly << "%i with striplen of " << stripsize2 << "." << std::endl;
		return Vec3D( -1000000.0f, -1000000.0f, -1000000.0f );
	}

	Vec3D lPosition;
	lPosition  = Vec3D( tv[gWorld->mapstrip2[Poly + 0]].x, tv[gWorld->mapstrip2[Poly + 0]].y, tv[gWorld->mapstrip2[Poly + 0]].z );
	lPosition += Vec3D( tv[gWorld->mapstrip2[Poly + 1]].x, tv[gWorld->mapstrip2[Poly + 1]].y, tv[gWorld->mapstrip2[Poly + 1]].z );
	lPosition += Vec3D( tv[gWorld->mapstrip2[Poly + 2]].x, tv[gWorld->mapstrip2[Poly + 2]].y, tv[gWorld->mapstrip2[Poly + 2]].z );
	lPosition *= 0.3333333f;

	return lPosition;
}

void MapChunk::recalcNorms()
{
	Vec3D P1,P2,P3,P4;
	Vec3D Norm,N1,N2,N3,N4,D;


	if(Changed==false)
		return;
	Changed=false;

	for(int i=0;i<mapbufsize;i++)
	{
		if(!gWorld->GetVertex(tv[i].x-UNITSIZE*0.5f,tv[i].z-UNITSIZE*0.5f,&P1))
		{
			P1.x=tv[i].x-UNITSIZE*0.5f;
			P1.y=tv[i].y;
			P1.z=tv[i].z-UNITSIZE*0.5f;
		}

		if(!gWorld->GetVertex(tv[i].x+UNITSIZE*0.5f,tv[i].z-UNITSIZE*0.5f,&P2))
		{
			P2.x=tv[i].x+UNITSIZE*0.5f;
			P2.y=tv[i].y;
			P2.z=tv[i].z-UNITSIZE*0.5f;
		}

		if(!gWorld->GetVertex(tv[i].x+UNITSIZE*0.5f,tv[i].z+UNITSIZE*0.5f,&P3))
		{
			P3.x=tv[i].x+UNITSIZE*0.5f;
			P3.y=tv[i].y;
			P3.z=tv[i].z+UNITSIZE*0.5f;
		}

		if(!gWorld->GetVertex(tv[i].x-UNITSIZE*0.5f,tv[i].z+UNITSIZE*0.5f,&P4))
		{
			P4.x=tv[i].x-UNITSIZE*0.5f;
			P4.y=tv[i].y;
			P4.z=tv[i].z+UNITSIZE*0.5f;
		}

		N1=(P2-tv[i])%(P1-tv[i]);
		N2=(P3-tv[i])%(P2-tv[i]);
		N3=(P4-tv[i])%(P3-tv[i]);
		N4=(P1-tv[i])%(P4-tv[i]);

		Norm=N1+N2+N3+N4;
		Norm.normalize();
		tn[i]=Norm;
	}
	glBindBuffer(GL_ARRAY_BUFFER, normals);
	glBufferData(GL_ARRAY_BUFFER, mapbufsize*3*sizeof(float), tn, GL_STATIC_DRAW);

	float ShadowAmount;
	for (int j=0; j<mapbufsize;j++)
	{
		//tm[j].z=tv[j].y;
		ShadowAmount=1.0f-(-tn[j].x+tn[j].y-tn[j].z);
		if(ShadowAmount<0)
			ShadowAmount=0;
		if(ShadowAmount>1.0)
			ShadowAmount=1.0f;
		ShadowAmount*=0.5f;
		//ShadowAmount=0.2;
		ts[j].x=0;
		ts[j].y=0;
		ts[j].z=0;
		ts[j].w=ShadowAmount;
	}

	glBindBuffer(GL_ARRAY_BUFFER, minishadows);
	glBufferData(GL_ARRAY_BUFFER, mapbufsize*4*sizeof(float), ts, GL_STATIC_DRAW);
}
//next is a manipulating of terrain
void MapChunk::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
	float dist,xdiff,zdiff;
	
	Changed=false;

	xdiff=xbase-x+CHUNKSIZE/2;
	zdiff=zbase-z+CHUNKSIZE/2;
	dist=sqrt(xdiff*xdiff+zdiff*zdiff);

	if(dist>(radius+MAPCHUNK_RADIUS))
		return;
	vmin.y =  9999999.0f;
	vmax.y = -9999999.0f;
	for(int i=0;i<mapbufsize;i++)
	{
		xdiff=tv[i].x-x;
		zdiff=tv[i].z-z;

		dist=sqrt(xdiff*xdiff+zdiff*zdiff);

		if(dist<radius)
		{
			if(BrushType==0)//Flat
				tv[i].y+=change;
			else if(BrushType==1)//Linear
				tv[i].y+=change*(1.0f-dist/radius);
			else if(BrushType==2)//Smooth
				tv[i].y+=change/(1.0f+dist/radius);
			Changed=true;
		}
		
		if (tv[i].y < vmin.y) vmin.y = tv[i].y;
		if (tv[i].y > vmax.y) vmax.y = tv[i].y;
	}
	if(Changed)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertices);
		glBufferData(GL_ARRAY_BUFFER, mapbufsize*3*sizeof(float), tv, GL_STATIC_DRAW);
	}
}

void MapChunk::flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType)
{
	float dist,xdiff,zdiff,nremain;
	Changed=false;

	xdiff=xbase-x+CHUNKSIZE/2;
	zdiff=zbase-z+CHUNKSIZE/2;
	dist=sqrt(xdiff*xdiff+zdiff*zdiff);

	if(dist>(radius+MAPCHUNK_RADIUS))
		return;

	vmin.y =  9999999.0f;
	vmax.y = -9999999.0f;

	for(int i=0;i<mapbufsize;i++)
	{
		xdiff=tv[i].x-x;
		zdiff=tv[i].z-z;

		dist=sqrt(xdiff*xdiff+zdiff*zdiff);

		if(dist<radius)
		{



			if(BrushType==0)//Flat
			{
				tv[i].y=remain*tv[i].y+(1-remain)*h;
			}
			else if(BrushType==1)//Linear
			{
				nremain=1-(1-remain)*(1-dist/radius);
				tv[i].y=nremain*tv[i].y+(1-nremain)*h;
			}
			else if(BrushType==2)//Smooth
			{
				nremain=1.0f-pow(1.0f-remain,(1.0f+dist/radius));
				tv[i].y=nremain*tv[i].y+(1-nremain)*h;
			}

			Changed=true;
		}
		
		if (tv[i].y < vmin.y) vmin.y = tv[i].y;
		if (tv[i].y > vmax.y) vmax.y = tv[i].y;
	}
	if(Changed)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertices);
		glBufferData(GL_ARRAY_BUFFER, mapbufsize*3*sizeof(float), tv, GL_STATIC_DRAW);
	}
}

void MapChunk::blurTerrain(float x, float z, float remain, float radius, int BrushType)
{
	float dist,dist2,xdiff,zdiff,nremain;
	Changed=false;

	xdiff=xbase-x+CHUNKSIZE/2;
	zdiff=zbase-z+CHUNKSIZE/2;
	dist=sqrt(xdiff*xdiff+zdiff*zdiff);

	if(dist>(radius+MAPCHUNK_RADIUS))
		return;

	vmin.y =  9999999.0f;
	vmax.y = -9999999.0f;

	for(int i=0;i<mapbufsize;i++)
	{
		xdiff=tv[i].x-x;
		zdiff=tv[i].z-z;

		dist=sqrt(xdiff*xdiff+zdiff*zdiff);

		if(dist<radius)
		{
			float TotalHeight;
			float TotalWeight;
			float tx,tz, h;
			Vec3D TempVec;
			int Rad;
			//Calculate Average
			Rad=(int)(radius/UNITSIZE);

			TotalHeight=0;
			TotalWeight=0;
			for(int j=-Rad*2;j<=Rad*2;j++)
			{
				tz=z+j*UNITSIZE/2;
				for(int k=-Rad;k<=Rad;k++)
				{
					tx=x+k*UNITSIZE+(j%2)*UNITSIZE/2.0f;
					xdiff=tx-tv[i].x;
					zdiff=tz-tv[i].z;
					dist2=sqrt(xdiff*xdiff+zdiff*zdiff);
					if(dist2>radius)
						continue;
					gWorld->GetVertex(tx,tz,&TempVec);
					TotalHeight+=(1.0f-dist2/radius)*TempVec.y;
					TotalWeight+=(1.0f-dist2/radius);
				}
			}

			h=TotalHeight/TotalWeight;

			if(BrushType==0)//Flat
			{
				tv[i].y=remain*tv[i].y+(1-remain)*h;
			}
			else if(BrushType==1)//Linear
			{
				nremain=1-(1-remain)*(1-dist/radius);
				tv[i].y=nremain*tv[i].y+(1-nremain)*h;
			}
			else if(BrushType==2)//Smooth
			{
				nremain=1.0f-pow(1.0f-remain,(1.0f+dist/radius));
				tv[i].y=nremain*tv[i].y+(1-nremain)*h;
			}

			Changed=true;
		}
		
		if (tv[i].y < vmin.y) vmin.y = tv[i].y;
		if (tv[i].y > vmax.y) vmax.y = tv[i].y;
	}
	if(Changed)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertices);
		glBufferData(GL_ARRAY_BUFFER, mapbufsize*3*sizeof(float), tv, GL_STATIC_DRAW);
	}
}

/* The correct way to do everything
Visibility = (1-Alpha above)*Alpha

Objective is Visibility = level

if (not bottom texture)
	New Alpha = Pressure*Level+(1-Pressure)*Alpha;
	New Alpha Above = (1-Pressure)*Alpha Above;
else Bottom Texture 
	New Alpha Above = Pressure*(1-Level)+(1-Pressure)*Alpha Above

For bottom texture with multiple above textures

For 2 textures above
x,y = current alphas
u,v = target alphas
v=sqrt((1-level)*y/x)
u=(1-level)/v

For 3 textures above
x,y,z = current alphas
u,v,w = target alphas
L=(1-Level)
u=pow(L*x*x/(y*y),1.0f/3.0f)
w=sqrt(L*z/(u*y))
*/
void MapChunk::eraseTextures( )
{
	nTextures = 0;
}

int MapChunk::addTexture( GLuint texture )
{
	int texLevel = -1;
	if( nTextures < 4 )
	{
		texLevel = nTextures;
		nTextures++;
		textures[texLevel] = texture;
		animated[texLevel] = 0;
		texFlags[texLevel] = 0;
		effectID[texLevel] = 0;
		if( texLevel )
		{
			if( alphamaps[texLevel-1] < 1 )
			{
				LogError << "Alpha Map has invalid texture binding" << std::endl;
				nTextures--;
				return -1;
			}
			memset( amap[texLevel - 1], 0, 64 * 64 );
			glBindTexture( GL_TEXTURE_2D, alphamaps[texLevel - 1] );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[texLevel - 1] );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
	}
	return texLevel;
}

bool MapChunk::paintTexture(float x, float z, brush *Brush, float strength, float pressure, int texture)//paint with texture
{
	float zPos,xPos,change,xdiff,zdiff,dist, radius;

	int texLevel=-1,i,j;

	radius=Brush->getRadius();

	xdiff=xbase-x+CHUNKSIZE/2;
	zdiff=zbase-z+CHUNKSIZE/2;
	dist=sqrt(xdiff*xdiff+zdiff*zdiff);

	if(dist>(radius+MAPCHUNK_RADIUS))
		return true;

	//First Lets find out do we have the texture already
	for(i=0;i<nTextures;i++)
		if(textures[i]==texture)
			texLevel=i;


	if((texLevel==-1)&&(nTextures==4))
		return false;
	
	//Only 1 layer and its that layer
	if((texLevel!=-1)&&(nTextures==1))
		return true;

	
	change=CHUNKSIZE/62.0f;
	zPos=zbase;

	int texAbove;
	float target,tarAbove, tPressure;
	texAbove=nTextures-texLevel-1;


	for(j=0;j<63;j++)
	{
		xPos=xbase;
		for(i=0;i<63;i++)
		{
			xdiff=xPos-x;
			zdiff=zPos-z;
			dist=abs(sqrt(xdiff*xdiff+zdiff*zdiff));
			
			if(dist>radius)
			{
				xPos+=change;
				continue;
			}

			if(texLevel==-1)
			{
				texLevel=addTexture(texture);
				if(texLevel==0)
					return true;
				if(texLevel==-1)
					return false;
			}
			
			target=strength;
			tarAbove=1-target;
			
			tPressure=pressure*Brush->getValue(dist);
			if(texLevel>0)
				amap[texLevel-1][i+j*64]=(unsigned char)max( min( (1-tPressure)*( (float)amap[texLevel-1][i+j*64] ) + tPressure*target + 0.5f ,255.0f) , 0.0f);
			for(int k=texLevel;k<nTextures-1;k++)
				amap[k][i+j*64]=(unsigned char)max( min( (1-tPressure)*( (float)amap[k][i+j*64] ) + tPressure*tarAbove + 0.5f ,255.0f) , 0.0f);
			xPos+=change;
		}
		zPos+=change;
	}

	if( texLevel == -1 )
		return false;
	
	for( int j = texLevel; j < nTextures - 1; j++ )
	{
		if( j > 2 )
		{
			LogError << "WTF how did you get here??? Get a cookie." << std::endl;
			continue;
		}
		glBindTexture( GL_TEXTURE_2D, alphamaps[j] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j] );
	}
	
	if( texLevel )
	{
		glBindTexture( GL_TEXTURE_2D, alphamaps[texLevel - 1] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[texLevel - 1] );
	}
	
	return true;
}

//--Holes ;P
bool MapChunk::isHole( int i, int j )
{
	return( holes & ( ( 1 << ((j*4)+i) ) ));
}

void MapChunk::addHole( int i, int j )
{
	holes = holes | ( ( 1 << ((j*4)+i)) );
	hasholes = holes;
	initStrip( );
}

void MapChunk::removeHole( int i, int j )
{
	holes = holes & ~( ( 1 << ((j*4)+i)) );
	hasholes = holes;
	initStrip( );
}

//-AreaID
void MapChunk::setAreaID( int ID )
{
	areaID = ID;
}

void MapNode::draw()
{
	/*if (!gWorld->frustum.intersects(vmin,vmax)) return;
	for (int i=0; i<4; i++) children[i]->draw();*/
}

void MapNode::drawSelect()
{
	//if (!gWorld->frustum.intersects(vmin,vmax)) return;
	//for (int i=0; i<4; i++) children[i]->drawSelect();
}

void MapNode::drawColor()
{
	if( !gWorld->frustum.intersects( vmin, vmax ) )
		return;
	
	for( int i = 0; i < 4; i++ )
		children[i]->drawColor( );
}

void MapNode::setup( MapTile *t )
{
	vmin = Vec3D( 9999999.0f, 9999999.0f, 9999999.0f );
	vmax = Vec3D( -9999999.0f, -9999999.0f, -9999999.0f );
	mt = t;
	if( size == 2 )
	{
		// children will be mapchunks
		children[0] = mt->chunks[py][px];
		children[1] = mt->chunks[py][px+1];
		children[2] = mt->chunks[py+1][px];
		children[3] = mt->chunks[py+1][px+1];
	} 
	else 
	{
		int half = size / 2;
		children[0] = new MapNode( px, py, half );
		children[1] = new MapNode( px + half, py, half );
		children[2] = new MapNode( px, py + half, half );
		children[3] = new MapNode( px + half, py + half, half );
		for( int i = 0; i < 4; i++ )
			children[i]->setup(mt);
	}
	for( int i = 0; i < 4; i++ )
	{
		if( children[i]->vmin.x < vmin.x ) 
			vmin.x = children[i]->vmin.x;
		if( children[i]->vmin.y < vmin.y ) 
			vmin.y = children[i]->vmin.y;
		if( children[i]->vmin.z < vmin.z ) 
			vmin.z = children[i]->vmin.z;
		if( children[i]->vmax.x > vmax.x ) 
			vmax.x = children[i]->vmax.x;
		if( children[i]->vmax.y > vmax.y ) 
			vmax.y = children[i]->vmax.y;
		if( children[i]->vmax.z > vmax.z ) 
			vmax.z = children[i]->vmax.z;
	}
}

void MapNode::cleanup( )
{
	if( size > 2 )
	{
		for( int i = 0; i < 4; i++ )
		{
			children[i]->cleanup( );
			delete children[i];
		}
	}
}

MapChunk* MapTile::getChunk( unsigned int x, unsigned int z )
{
	assert( x < 16 && z < 16 );
	return chunks[z][x];
}

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
	Log << "Saving ADT \"" << fname << "\"." << std::endl;

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
		/// TODO: Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
		std::string lTemp = it->second.model->filename;
		transform( lTemp.begin(), lTemp.end(), lTemp.begin(), ::tolower );
		size_t found = lTemp.rfind( ".m2" );
		if( found != string::npos )
		{
			lTemp.replace( found, 3, ".md" );
			lTemp.append( "x" );
		}

		if( lModels.find( lTemp ) == lModels.end( ) )
			lModels.insert( pair<std::string, int*>( lTemp, new int[2] ) ); 
	}
	
	lID = 0;
	for( std::map<std::string, int*>::iterator it = lModels.begin( ); it != lModels.end( ); it++ )
		it->second[0] = lID++;

	std::map<std::string, int*> lObjects;

	for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
		if( lObjects.find( it->second.wmo->filename ) == lObjects.end( ) )
			lObjects.insert( pair<std::string, int*>( ( it->second.wmo->filename ), new int[2] ) ); 
	
	lID = 0;
	for( std::map<std::string, int*>::iterator it = lObjects.begin( ); it != lObjects.end( ); it++ )
		it->second[0] = lID++;

	// Check which textures are on this ADT.
	std::map<std::string, int> lTextures;

	for( int i = 0; i < 16; i++ )
		for( int j = 0; j < 16; j++ )
			for( int tex = 0; tex < chunks[i][j]->nTextures; tex++ )
				if( lTextures.find( video.textures.items[chunks[i][j]->textures[tex]]->name ) == lTextures.end( ) )
					lTextures.insert( pair<std::string, int>( video.textures.items[chunks[i][j]->textures[tex]]->name, -1 ) ); 
	
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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MCIN_Offset = lCurrentPosition - 0x14;

		// MCIN * MCIN_Data = lADTFile.GetPointer<MCIN>( lMCIN_Position + 8 );

		lCurrentPosition += 8 + 256 * 0x10;
//	}

	// MTEX
//	{
		int lMTEX_Position = lCurrentPosition;
		lADTFile.Extend( 8 + 0 );	// We don't yet know how big this will be.
		SetChunkHeader( lADTFile, lCurrentPosition, 'MTEX' );
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MTEX_Offset = lCurrentPosition - 0x14;

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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MMDX_Offset = lCurrentPosition - 0x14;

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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MMID_Offset = lCurrentPosition - 0x14;

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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MWMO_Offset = lCurrentPosition - 0x14;

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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MWID_Offset = lCurrentPosition - 0x14;

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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MDDF_Offset = lCurrentPosition - 0x14;

		// MDDF data
		ENTRY_MDDF * lMDDF_Data = lADTFile.GetPointer<ENTRY_MDDF>( lCurrentPosition + 8 );

		lID = 0;
		for( std::map<int,ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it )
		{
			/// TODO: Is it still needed, that they are ending in .mdx? As far as I know it isn't. So maybe remove renaming them.
			std::string lTemp = it->second.model->filename;
			transform( lTemp.begin(), lTemp.end(), lTemp.begin(), ::tolower );
			size_t found = lTemp.rfind( ".m2" );
			if( found != string::npos )
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
			lMDDF_Data[lID].nameID = lMyFilenameThingey->second[0];
			lMDDF_Data[lID].uniqueID = it->first;
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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MODF_Offset = lCurrentPosition - 0x14;

		// MODF data
		ENTRY_MODF * lMODF_Data = lADTFile.GetPointer<ENTRY_MODF>( lCurrentPosition + 8 );

		lID = 0;
		for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
		{
			std::map<std::string, int*>::iterator lMyFilenameThingey = lObjects.find( it->second.wmo->filename );
			if( lMyFilenameThingey == lObjects.end( ) )
			{
				LogError << "There is a problem with saving the objects. We have an object that somehow changed the name during the saving function. However this got produced, you can get a reward from schlumpf by pasting him this line." << std::endl;
				return;
			}
			lMODF_Data[lID].nameID = lMyFilenameThingey->second[0];
			lMODF_Data[lID].uniqueID = it->first;
			lMODF_Data[lID].pos[0] = it->second.pos.x;
			lMODF_Data[lID].pos[1] = it->second.pos.y;
			lMODF_Data[lID].pos[2] = it->second.pos.z;
			lMODF_Data[lID].rot[0] = it->second.dir.x;
			lMODF_Data[lID].rot[1] = it->second.dir.y;
			lMODF_Data[lID].rot[2] = it->second.dir.z;
			/// TODO: Calculate them here or when rotating / moving? What is nicer? We should at least do it somewhere..
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
	/// TODO: MCNK
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
				lADTFile.Insert( lCurrentPosition + 8, 0x80, reinterpret_cast<char*>( &( this->chunks[y][x]->header ) ) );
				MapChunkHeader * lMCNK_header = lADTFile.GetPointer<MapChunkHeader>( lCurrentPosition + 8 );

				lMCNK_header->flags = chunks[y][x]->Flags;
				lMCNK_header->holes = chunks[y][x]->holes;
				lMCNK_header->areaid = chunks[y][x]->areaID;
				
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

				/// TODO: Implement sound emitter support. Or not.
				lMCNK_header->ofsSndEmitters = 0;
				lMCNK_header->nSndEmitters = 0;

				lMCNK_header->ofsLiquid = 0;
				lMCNK_header->sizeLiquid = 8;

				/// TODO: MCCV sub-chunk
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
						lMedian = lMedian + chunks[y][x]->tv[i].y;

					lMedian = lMedian / ( 9 * 9 + 8 * 8 );
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ypos = lMedian;

					for( int i = 0; i < ( 9 * 9 + 8 * 8 ); i++ )
						lHeightmap[i] = chunks[y][x]->tv[i].y - lMedian;
					
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

					for( int i = 0; i < ( 9 * 9 + 8 * 8 ); i++ )
					{
						lNormals[i*3+0] = roundc( -chunks[y][x]->tn[i].z * 127 );
						lNormals[i*3+1] = roundc( -chunks[y][x]->tn[i].x * 127 );
						lNormals[i*3+2] = roundc( -chunks[y][x]->tn[i].y * 127 );
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
					int lMCLY_Size = chunks[y][x]->nTextures * 0x10;

					lADTFile.Extend( 8 + lMCLY_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCLY', lMCLY_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsLayer = lCurrentPosition - lMCNK_Position;
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nLayers = chunks[y][x]->nTextures;
			
					// MCLY data
					for( int j = 0; j < chunks[y][x]->nTextures; j++ )
					{
						ENTRY_MCLY * lLayer = lADTFile.GetPointer<ENTRY_MCLY>( lCurrentPosition + 8 + 0x10 * j );

						lLayer->textureID = lTextures.find( video.textures.items[chunks[y][x]->textures[j]]->name )->second;

						lLayer->flags = chunks[y][x]->texFlags[j];
						
						// if not first, have alpha layer, if first, have not. never have compression.
						lLayer->flags = ( j > 0 ? lLayer->flags | 0x100 : lLayer->flags & ( ~0x100 ) ) & ( ~0x200 );

						lLayer->ofsAlpha = ( j == 0 ? 0 : ( mBigAlpha ? 64 * 64 * ( j - 1 ) : 32 * 64 * ( j - 1 ) ) );
						lLayer->effectID = chunks[y][x]->effectID[j];
					}

					lCurrentPosition += 8 + lMCLY_Size;
					lMCNK_Size += 8 + lMCLY_Size;
//				}

				// MCRF
//				{
					std::vector<int> lDoodads;
					std::vector<int> lObjects;
					
					Vec3D lChunkExtents[2];
					lChunkExtents[0] = Vec3D( chunks[y][x]->xbase, 0.0f, chunks[y][x]->zbase );
					lChunkExtents[1] = Vec3D( chunks[y][x]->xbase + CHUNKSIZE, 0.0f, chunks[y][x]->zbase + CHUNKSIZE );

					// search all wmos that are inside this chunk
					lID = 0;
					for( std::map<int,WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it )
					{
						/// TODO: This requires the extents already being calculated. See above.
						if( checkInside( lChunkExtents, it->second.extents ) )
							lObjects.push_back( lID );
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
						Vec3D chunkMid(chunks[y][x]->xbase + CHUNKSIZE / 2, 0, 
						chunks[y][x]->zbase + CHUNKSIZE / 2);
	
						// find out if the model is inside the reach of the chunk.
						float dx = chunkMid.x - pos.x;
						float dz = chunkMid.z - pos.z;
						float dist = sqrtf(dx * dx + dz * dz);
						static float sqrt2 = sqrtf(2.0f);

						if(dist - radius <= ((sqrt2 / 2.0f) * CHUNKSIZE))
						{
							lDoodads.push_back(lID);
						}
	
						lID++;
					}

					int lMCRF_Size = 4 * ( lDoodads.size( ) + lObjects.size( ) );
					lADTFile.Extend( 8 + lMCRF_Size );
					SetChunkHeader( lADTFile, lCurrentPosition, 'MCRF', lMCRF_Size );

					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsRefs = lCurrentPosition - lMCNK_Position;
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nDoodadRefs = lDoodads.size( );
					lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->nMapObjRefs = lObjects.size( );

					// MCRF data
					int * lReferences = lADTFile.GetPointer<int>( lCurrentPosition + 8 );

					lID = 0;
					for( std::vector<int>::iterator it = lDoodads.begin( ); it != lDoodads.end( ); it++ )
					{
						lReferences[lID] = *it;
						lID++;
					}

					for( std::vector<int>::iterator it = lObjects.begin( ); it != lObjects.end( ); it++ )
					{
						lReferences[lID] = *it;
						lID++;
					}

					lCurrentPosition += 8 + lMCRF_Size;
					lMCNK_Size += 8 + lMCRF_Size;
//				}

				// MCSH
//				{
					/// TODO: Somehow determine if we need to write this or not?
					/// TODO: This sometime gets all shadows black.
					if( chunks[y][x]->Flags & 1 )
					{
						int lMCSH_Size = 0x200;
						lADTFile.Extend( 8 + lMCSH_Size );
						SetChunkHeader( lADTFile, lCurrentPosition, 'MCSH', lMCSH_Size );

						lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->ofsShadow = lCurrentPosition - lMCNK_Position;
						lADTFile.GetPointer<MapChunkHeader>( lMCNK_Position + 8 )->sizeShadow = 0x200;

						char * lLayer = lADTFile.GetPointer<char>( lCurrentPosition + 8 );
						
						memcpy( lLayer, chunks[y][x]->mShadowMap, 0x200 );

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

					int lMaps = chunks[y][x]->nTextures - 1;
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
								lAlphaMaps[lDimensions * j + k] = chunks[y][x]->amap[j][k];
						else
						{
							unsigned char upperNibble, lowerNibble;
							for( int k = 0; k < lDimensions; k++ )
							{
								lowerNibble = (unsigned char)max(min( ( (float)chunks[y][x]->amap[j][k * 2 + 0] ) * 0.05882f + 0.5f , 15.0f),0.0f);
								upperNibble = (unsigned char)max(min( ( (float)chunks[y][x]->amap[j][k * 2 + 1] ) * 0.05882f + 0.5f , 15.0f),0.0f);
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
		lADTFile.GetPointer<MHDR>( lMHDR_Position + 8 )->MFBO_Offset = lCurrentPosition - 0x14;

		short * lMFBO_Data = lADTFile.GetPointer<short>( lCurrentPosition + 8 );
		
		lID = 0;
		for( int i = 0; i < 9; i++ )
			lMFBO_Data[lID++] = mMinimumValues[i * 3 + 1];

		for( int i = 0; i < 9; i++ )
			lMFBO_Data[lID++] = mMaximumValues[i * 3 + 1];

		lCurrentPosition += 8 + 36;
	}

	/// TODO: MH2O
	/// TODO: MTFX
	
	MPQFile f( fname );
	f.setBuffer( lADTFile.GetPointer<uint8_t>( ), lADTFile.mSize );
	f.SaveFile( );
	f.close( );
}

bool MapTile::GetVertex( float x, float z, Vec3D *V )
{
	int xcol = int( ( x - xbase ) / CHUNKSIZE );
	int ycol = int( ( z - zbase ) / CHUNKSIZE );
	
	return xcol >= 0 && xcol <= 15 && ycol >= 0 && ycol <= 15 && chunks[ycol][xcol]->GetVertex( x, z, V );
}