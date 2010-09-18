#include "world.h"
#include "dbc.h"
#include <string>
#include <sstream>
#include <cassert>

#include "time.h"

#include "Settings.h"
#include "Project.h"
#include "Environment.h"

#include "video.h"
#include "Log.h"
#include "MapChunk.h"
#include "MapTile.h"

#include "WMOInstance.h" // WMOInstance
 
World *gWorld=0;

static const int BUFSIZE = 8192;
unsigned int	SelectBuffer[BUFSIZE];


bool IsEditableWorld( int pMapId )
{
	std::string lMapName;
	try 
	{
		DBCFile::Record map = gMapDB.getByID( pMapId );
		lMapName = map.getString( MapDB::InternalName );
	}
	catch( ... )
	{
		LogError << "Did not find map with id " << pMapId << ". This is NOT editable.." << std::endl;
		return false;
	}

	std::stringstream ssfilename;
	ssfilename << "World\\Maps\\" << lMapName << "\\" << lMapName << ".wdt";
	
	if( !MPQFile::exists( ssfilename.str( ) ) )
	{
		Log << "World " << pMapId << ": " << lMapName << " has no WDT file!" << std::endl;
		return false;
	}

	MPQFile mf( ssfilename.str( ).c_str( ) );

	//sometimes, wdts don't open, so ignore them...
	if(mf.isEof())
		return false;

	const char * lPointer = reinterpret_cast<const char*>( mf.getPointer( ) );

	// Not using the libWDT here doubles performance. You might want to look at your lib again and improve it.
	const int lFlags = *( reinterpret_cast<const int*>( lPointer + 8 + 4 + 8 ) );
	if( lFlags & 1 )
		return false;

	const int * lData = reinterpret_cast<const int*>( lPointer + 8 + 4 + 8 + 0x20 + 8 );
	for( int i = 0; i < 8192; i += 2 )
	{
		if( lData[i] & 1 )
			return true;
	}

	return false;
}

World::World( const char* name ) : basename( name ), mCurrentSelection( 0 )
{
	mMapId = 0xFFFFFFFF;
	for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i )
	{
		if( !strcmp( name, i->getString( MapDB::InternalName ) ) )
		{
			mMapId = i->getUInt( MapDB::MapID );
			break;
		}
	}
	if( mMapId == 0xFFFFFFFF )
		LogError << "MapId for \"" << name << "\" not found! What is wrong here?" << std::endl;

	//::gWorld = this;

	LogDebug << "Loading world \"" << name << "\"." << std::endl;

	autoheight = false;

	init();
	skies = 0;
	ol = 0;

	zoom=0.25;

	// don't load map objects while still on the menu screen
	//initDisplay();
}


void World::init()
{
	for (int j=0; j<64; j++)
	{
		for (int i=0; i<64; i++)
		{
			lowrestiles[j][i] = 0;
		}
	}
  
  std::stringstream filename; 
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdt";

	time = 1450;
	animtime = 0;

	ex = ez = -1;
	loading = false;

	drawfog = false;
	
	mapstrip = 0;
	mapstrip2 = 0;
	
	minimap = 0;
	
	mWmoFilename = "";

	MPQFile theFile(filename.str());
	uint32_t fourcc;
	uint32_t size;

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
	
	for( int j = 0; j < 64; j++ ) 
	{
		for( int i = 0; i < 64; i++ ) 
		{
			theFile.read( &mTiles[j][i].flags, 4 );
      theFile.seekRelative( 4 );
      mTiles[j][i].tile = NULL;
      printf("%i ", mTiles[j][i].flags);
		}
    printf("\n");
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
	
	theFile.close( );
	
	if( !mHasAGlobalWMO )
		initMinimap();
}


void World::initMinimap()
{
  std::stringstream filename; 
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdl";
  
	MPQFile f(filename.str());
	if (f.isEof()) {
		LogError << "file \"World\\Maps\\" << basename << "\\" << basename << ".wdl\" does not exist." << std::endl;
		return;
	}

	int ofsbuf[64][64];
	memset(ofsbuf, 0, 64*64*4);

	int fourcc;
	size_t size;

	while (!f.isEof()) {
		f.read(&fourcc,4);
		f.read(&size, 4);

		if (size == 0)
			continue;

		size_t nextpos = f.getPos() + size;

	/*	if( fourcc == 'MVER' ) {
		} 
		else if( fourcc == 'MWMO' ) {
			// Filenames for WMO that appear in the low resolution map. Zero terminated strings.
		} 
		else if( fourcc == 'MWID' ) {
			// List of indexes into the MWMO chunk.
		} 
		else if( fourcc == 'MODF' ) {
			// Placement information for the WMO. Appears to be the same 64 byte structure used in the WDT and ADT MODF chunks.
		} 
		else*/ if( fourcc == 'MAOF' ) {
			f.read(ofsbuf,64*64*4);
		}
		else if( fourcc == 'MARE' ) {
			glGenTextures(1, &minimap);

			// zomg, data on the stack!!1
			//int texbuf[512][512];
			unsigned int *texbuf = new unsigned int[512*512];
			memset(texbuf,0,512*512*4);

			// as alpha is unused, maybe I should try 24bpp? :(
			short tilebuf[17*17];

			for (int j=0; j<64; j++) {
				for (int i=0; i<64; i++) {
					if (ofsbuf[j][i]) {
						f.seek(ofsbuf[j][i]+8);
						// read height values ^_^

						/*
						short *sp = tilebuf;
						for (int z=0; z<33; z++) {
							f.read(sp, 2 * ( (z%2) ? 16 : 17 ));
							sp += 17;
						}*/
						/*
						fucking win. in the .adt files, height maps are stored in 9-8-9-8-... interleaved order.
						here, apparently, a 17x17 map is stored followed by a 16x16 map.
						yay for consistency.
						I'm only using the 17x17 map here.
						*/
						f.read(tilebuf,17*17*2);

						// make minimap
						// for a 512x512 minimap texture, and 64x64 tiles, one tile is 8x8 pixels
						for (int z=0; z<8; z++) {
							for (int x=0; x<8; x++) {
								short hval = tilebuf[(z*2)*17+x*2]; // for now

								// make rgb from height value
								unsigned char r,g,b;
								if (hval < 0) {
									// water = blue
									if (hval < -511) hval = -511;
									hval /= -2;
									r = g = 0;
									b = 255 - hval;
								} else {
									// above water = should apply a palette :(
									/*
									float fh = hval / 1600.0f;
									if (fh > 1.0f) fh = 1.0f;
									unsigned char c = (unsigned char) (fh * 255.0f);
									r = g = b = c;
									*/

									// green: 20,149,7		0-600
									// brown: 137, 84, 21	600-1200
									// gray: 96, 96, 96		1200-1600
									// white: 255, 255, 255
									unsigned char r1,r2,g1,g2,b1,b2;
									float t;

									if (hval < 600) {
										r1 = 20;
										r2 = 137;
										g1 = 149;
										g2 = 84;
										b1 = 7;
										b2 = 21;
										t = hval / 600.0f;
									}
									else if (hval < 1200) {
										r2 = 96;
										r1 = 137;
										g2 = 96;
										g1 = 84;
										b2 = 96;
										b1 = 21;
										t = (hval-600) / 600.0f;
									}
									else /*if (hval < 1600)*/ {
										r1 = 96;
										r2 = 255;
										g1 = 96;
										g2 = 255;
										b1 = 96;
										b2 = 255;
										if (hval >= 1600) hval = 1599;
										t = (hval-1200) / 600.0f;
									}

									//! \todo  add a regular palette here

									r = (unsigned char)(r2*t + r1*(1.0f-t));
									g = (unsigned char)(g2*t + g1*(1.0f-t));
									b = (unsigned char)(b2*t + b1*(1.0f-t));
								}

								texbuf[(j*8+z)*512 + i*8+x] = (r) | (g<<8) | (b<<16) | (255 << 24);
							}
						}
					}
				}
			}
			
			glBindTexture(GL_TEXTURE_2D, minimap);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, texbuf);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

			delete[] texbuf;
			f.close();
			return;
		} 
		else if( fourcc == 'MAHO' ) {
/*
After each MARE chunk there follows a MAHO (MapAreaHOles) chunk. It may be left out if the data is supposed to be 0 all the time.
Its an array of 16 shorts. Each short is a bitmask. If the bit is not set, there is a hole at this position.
*/
		} 
	/*	else  {
			char fcc[5];
			f.seekRelative(-8);
			f.read(fcc,4);
			fcc[4] = 0;
			gLog("minimap %s [%d].\n", fcc, size);
		} */
		f.seek((int)nextpos);
	}

	f.close();
}


void World::initLowresTerrain()
{
  std::stringstream filename; 
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdl";
  
	short tilebuf[17*17];
	short tilebuf2[16*16];
	Vec3D lowres[17][17];
	Vec3D lowsub[16][16];
	int ofsbuf[64][64];

	MPQFile f(filename.str());

	int fourcc;
	size_t size;

	while (!f.isEof()) 
	{
		f.read(&fourcc,4);
		f.read(&size, 4);

		if (size == 0)
			continue;

		size_t nextpos = f.getPos() + size;

		if( fourcc == 'MAOF' )
		{
			f.read(ofsbuf,64*64*4);

			for (int j=0; j<64; j++) 
			{
				for (int i=0; i<64; i++) 
				{
					if (ofsbuf[j][i]) 
					{
						f.seek(ofsbuf[j][i]+8);
						f.read(tilebuf,17*17*2);
						f.read(tilebuf2,16*16*2);
					
						for (int y=0; y<17; y++) 
						{
							for (int x=0; x<17; x++) 
							{
								lowres[y][x] = Vec3D(TILESIZE*(i+x/16.0f), tilebuf[y*17+x], TILESIZE*(j+y/16.0f));
							}
						}
						for (int y=0; y<16; y++) 
						{
							for (int x=0; x<16; x++) 
							{
								lowsub[y][x] = Vec3D(TILESIZE*(i+(x+0.5f)/16.0f), tilebuf2[y*16+x], TILESIZE*(j+(y+0.5f)/16.0f));
							}
						}

						GLuint dl;
						dl = glGenLists(1);
						glNewList(dl, GL_COMPILE);
						/*
						// draw tiles 16x16?
						glBegin(GL_TRIANGLE_STRIP);
						for (int y=0; y<16; y++) {
							// end jump
							if (y>0) glVertex3fv(lowres[y][0]);
							for (int x=0; x<17; x++) {
								glVertex3fv(lowres[y][x]);
								glVertex3fv(lowres[y+1][x]);
							}
							// start jump
							if (y<15) glVertex3fv(lowres[y+1][16]);
						}
						glEnd();
						*/
						// draw tiles 17*17+16*16
						glBegin(GL_TRIANGLES);
						for (int y=0; y<16; y++) {
							for (int x=0; x<16; x++) {
								glVertex3fv(lowres[y][x]);		glVertex3fv(lowsub[y][x]);	glVertex3fv(lowres[y][x+1]);
								glVertex3fv(lowres[y][x+1]);	glVertex3fv(lowsub[y][x]);	glVertex3fv(lowres[y+1][x+1]);
								glVertex3fv(lowres[y+1][x+1]);	glVertex3fv(lowsub[y][x]);	glVertex3fv(lowres[y+1][x]);
								glVertex3fv(lowres[y+1][x]);	glVertex3fv(lowsub[y][x]);	glVertex3fv(lowres[y][x]);
							}
						}
						glEnd();
						glEndList();
						lowrestiles[j][i] = dl;
					}
				}
			}
			f.close();
			return;
		}
		f.seek((int)nextpos);
	}

	LogError << "Error in reading low res terrain. MAOF not found." << std::endl;
	f.close();
}

void initGlobalVBOs( GLuint &pDetailTexCoords, GLuint &pAlphaTexCoords )
{
	if( !pDetailTexCoords && !pAlphaTexCoords )
	{
		Vec2D temp[mapbufsize], *vt;
		float tx,ty;
		
		// init texture coordinates for detail map:
		vt = temp;
		const float detail_half = 0.5f * detail_size / 8.0f;
		for (int j=0; j<17; j++) {
			for (int i=0; i<((j%2)?8:9); i++) {
				tx = detail_size / 8.0f * i;
				ty = detail_size / 8.0f * j * 0.5f;
				if (j%2) {
					// offset by half
					tx += detail_half;
				}
				*vt++ = Vec2D(tx, ty);
			}
		}

		glGenBuffers(1, &pDetailTexCoords);
		glBindBuffer(GL_ARRAY_BUFFER, pDetailTexCoords);
		glBufferData(GL_ARRAY_BUFFER, mapbufsize*2*sizeof(float), temp, GL_STATIC_DRAW);

		// init texture coordinates for alpha map:
		vt = temp;

		const float alpha_half = 0.5f * (62.0f/64.0f) / 8.0f;
		for (int j=0; j<17; j++) {
			for (int i=0; i<((j%2)?8:9); i++) {
				tx = (62.0f/64.0f) / 8.0f * i;
				ty = (62.0f/64.0f) / 8.0f * j * 0.5f;
				if (j%2) {
					// offset by half
					tx += alpha_half;
				}
				*vt++ = Vec2D(tx, ty);
			}
		}

		glGenBuffers(1, &pAlphaTexCoords);
		glBindBuffer(GL_ARRAY_BUFFER, pAlphaTexCoords);
		glBufferData(GL_ARRAY_BUFFER, mapbufsize*2*sizeof(float), temp, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}


void World::initDisplay()
{
	// default strip indices
	short *defstrip = new short[stripsize];
	for (int i=0; i<stripsize; i++) defstrip[i] = i; // note: this is ugly and should be handled in stripify
	mapstrip = new short[stripsize];
	stripify<short>(defstrip, mapstrip);
	delete[] defstrip;

	defstrip = new short[stripsize2];
	for (int i=0; i<stripsize2; i++) defstrip[i] = i; // note: this is ugly and should be handled in stripify
	mapstrip2 = new short[stripsize2];
	stripify2<short>(defstrip, mapstrip2);
	delete[] defstrip;

	initGlobalVBOs( detailtexcoords, alphatexcoords );

	highresdistance = 384.0f;
	mapdrawdistance = 998.0f;
	modeldrawdistance = 384.0f;
	doodaddrawdistance = 64.0f;

	noadt = false;
	
	if( mHasAGlobalWMO )
	{
		wmomanager.add( mWmoFilename );
		WMOInstance inst( reinterpret_cast<WMO*>( wmomanager.items[ wmomanager.get( mWmoFilename ) ] ), &mWmoEntry );
		
		gWorld->mWMOInstances.insert( std::pair<int,WMOInstance>( mWmoEntry.uniqueID, inst ) );
		camera = inst.pos;
	}

	skies = new Skies( mMapId );

	ol = new OutdoorLighting("World\\dnc.db");

	initLowresTerrain();
}

World::~World()
{
	for( int j = 0; j < 64; j++ ) 
  {
		for( int i = 0; i < 64; i++ ) 
    {
			if( lowrestiles[j][i] )
      {
        glDeleteLists( lowrestiles[j][i], 1 );
      }
      if( tileLoaded( j, i ) )
      {
        delete mTiles[j][i].tile;
        mTiles[j][i].tile = NULL;
      }
		}
	}

	if (minimap) glDeleteTextures(1, &minimap);

	if (skies) delete skies;
	if (ol) delete ol;

	if (mapstrip) delete[] mapstrip;
	if (mapstrip2) delete[] mapstrip2;

	LogDebug << "Unloaded world \"" << basename << "\"." << std::endl;
}

inline bool oktile( int i, int j )
{
	return !( i < 0 || j < 0 || i > 64 || j > 64 );
}

bool World::hasTile( int pZ, int pX )
{
  return oktile( pZ, pX ) && ( mTiles[pZ][pX].flags & 1 );
}

void World::enterTile( int x, int z )
{
  if( !hasTile( z, x ) )
  {
    noadt = true;
    return;
  }
	
  noadt = false;

  using std::min;
  using std::max;
  
	cx = x;
	cz = z;
	for( int i = max(cz - 2, 0); i < min(cz + 2, 64); i++ )
  {
		for( int j = max(cx - 2, 0); j < min(cx + 2, 64); j++ )
    {
			mTiles[i][j].tile = loadTile( i, j );
    }
  }

	if( autoheight && tileLoaded( cx, cz ) ) 
	{
		float maxHeight = mTiles[cz][cx].tile->getMaxHeight();
    maxHeight = std::max( maxHeight, 0.0f );
		camera.y = maxHeight + 50.0f;

		autoheight = false;
	}
}

void World::reloadTile(int x, int z)
{
  if( tileLoaded( z, x ) )
  {
    delete mTiles[z][x].tile;
    
    std::stringstream filename; 
    filename << "World\\Maps\\" << basename << "\\" << basename << "_" << x << "_" << z << ".adt";
    
    mTiles[z][x].tile = new MapTile( x, z, filename.str(), mBigAlpha );
    enterTile( cx, cz );
  }
}

void World::saveTile(int x, int z)
{
  if( tileLoaded( z, x ) )
  {
    mTiles[z][x].tile->saveTile();
	}
}

inline bool World::tileLoaded(int z, int x)
{
  return hasTile( z, x ) && mTiles[z][x].tile;
}

MapTile* World::loadTile(int z, int x)
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

	mTiles[z][x].tile = new MapTile( z, x, filename.str(), mBigAlpha );
	return mTiles[z][x].tile;
}


void lightingDefaults()
{
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0);

	glEnable(GL_LIGHT0);
	// wtf
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHT4);
	glDisable(GL_LIGHT5);
	glDisable(GL_LIGHT6);
	glDisable(GL_LIGHT7);
}

/*
void myFakeLighting()
{
	GLfloat la = 0.5f;
	GLfloat ld = 1.0f;

	GLfloat LightAmbient[] = {la, la, la, 1.0f};
	GLfloat LightDiffuse[] = {ld, ld, ld, 1.0f};
	GLfloat LightPosition[] = {-10.0f, 20.0f, -10.0f, 0.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
}
*/

void World::outdoorLighting()
{
	Vec4D black(0,0,0,0);
	Vec4D ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

	float di = outdoorLightStats.dayIntensity, ni = outdoorLightStats.nightIntensity;
	di = 1;
	ni = 0;

	//Vec3D dd = outdoorLightStats.dayDir;
	// HACK: let's just keep the light source in place for now
	Vec4D pos(-1, 1, -1, 0);
	Vec4D col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, black);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	
	/*
	Vec3D dd = outdoorLightStats.nightDir;
	Vec4D pos(-dd.x, -dd.z, dd.y, 0);
	Vec4D col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * ni, 1); 
	glLightfv(GL_LIGHT1, GL_AMBIENT, black);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, col);
	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	*/
}

/*void World::outdoorLighting2()
{
	Vec4D black(0,0,0,0);
	Vec4D ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

	float di = outdoorLightStats.dayIntensity, ni = outdoorLightStats.nightIntensity;
	di = 1;
	ni = 0;

	//Vec3D dd = outdoorLightStats.dayDir;
	// HACK: let's just keep the light source in place for now
	Vec4D pos(-1, -1, -1, 0);
	Vec4D col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * di, 1); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, black);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	*/
	/*
	Vec3D dd = outdoorLightStats.nightDir;
	Vec4D pos(-dd.x, -dd.z, dd.y, 0);
	Vec4D col(skies->colorSet[LIGHT_GLOBAL_DIFFUSE] * ni, 1); 
	glLightfv(GL_LIGHT1, GL_AMBIENT, black);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, col);
	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	*//*
}*/


void World::outdoorLights(bool on)
{
	float di = outdoorLightStats.dayIntensity, ni = outdoorLightStats.nightIntensity;
	di = 1;
	ni = 0;

	if (on) {
		Vec4D ambient(skies->colorSet[LIGHT_GLOBAL_AMBIENT], 1);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
		if (di>0) {
			glEnable(GL_LIGHT0);
		} else {
			glDisable(GL_LIGHT0);
		}
		if (ni>0) {
			glEnable(GL_LIGHT1);
		} else {
			glDisable(GL_LIGHT1);
		}
	} else {
		Vec4D ambient(0, 0, 0, 1);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
		glDisable(GL_LIGHT0);
		glDisable(GL_LIGHT1);
	}
}

void World::setupFog()
{
	if (drawfog) {

		//float fogdist = 357.0f; // minimum draw distance in wow
		//float fogdist = 777.0f; // maximum draw distance in wow
		
		float fogdist = fogdistance;
		float fogstart = 0.5f;

		culldistance = fogdist;

		//FOG_COLOR
		Vec4D fogcolor(skies->colorSet[FOG_COLOR], 1);
		glFogfv(GL_FOG_COLOR, fogcolor);
		//! \todo  retreive fogstart and fogend from lights.lit somehow
		glFogf(GL_FOG_END, fogdist);
		glFogf(GL_FOG_START, fogdist * fogstart);

		glEnable(GL_FOG);
	} else {
		glDisable(GL_FOG);
		culldistance = mapdrawdistance;
	}
	culldistance2 = culldistance * culldistance;
}

void World::draw()
{
	//Now for drawing code

	WMOInstance::reset();
	modelmanager.resetAnim();

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	highresdistance2 = highresdistance * highresdistance;
	mapdrawdistance2 = mapdrawdistance * mapdrawdistance;
	modeldrawdistance2 = modeldrawdistance * modeldrawdistance;
	doodaddrawdistance2 = doodaddrawdistance * doodaddrawdistance;

	// setup camera
	gluLookAt(camera.x,camera.y,camera.z, lookat.x,lookat.y,lookat.z, 0, 1, 0);

	// camera is set up
	frustum.retrieve();

	///glDisable(GL_LIGHTING);
	///glColor4f(1,1,1,1);

    //int tt = 1440;
	//if (modelmanager.v>0) {
	//	tt = (modelmanager.v *180 + 1440) % 2880;
	//}		

	hadSky = false;
	if( drawwmo || mHasAGlobalWMO )
		for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); !hadSky && it != mWMOInstances.end(); ++it )
			it->second.wmo->drawSkybox( this->camera, it->second.extents[0], it->second.extents[1] );


	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);

	int daytime = ((int)time)%2880;
	//outdoorLightStats = ol->getLightStats(daytime);
	skies->initSky(camera, daytime);

	if (!hadSky) 
		hadSky = skies->drawSky(camera);

	// clearing the depth buffer only - color buffer is/has been overwritten anyway
	// unless there is no sky OR skybox
	GLbitfield clearmask = GL_DEPTH_BUFFER_BIT;
	if (!hadSky) 	clearmask |= GL_COLOR_BUFFER_BIT;
	glClear(clearmask); 

	glDisable(GL_TEXTURE_2D);
	

	outdoorLighting();
	outdoorLights(true);
	
	glFogi(GL_FOG_MODE, GL_LINEAR);
	setupFog();

	// Draw verylowres heightmap
	if (drawfog && drawterrain) {
		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glColor3fv(this->skies->colorSet[FOG_COLOR]);
		//glColor3f(0,1,0);
		//glDisable(GL_FOG);
		const int lrr = 2;
		for (int i=cx-lrr; i<=cx+lrr; i++) {
			for (int j=cz-lrr; j<=cz+lrr; j++) {
				//! \todo  some annoying visual artifacts when the verylowres terrain overlaps
				// maptiles that are close (1-off) - figure out how to fix.
				// still less annoying than hoels in the horizon when only 2-off verylowres tiles are drawn
				if ( !(i==cx&&j==cz) && oktile(i,j) && lowrestiles[j][i]) {
					glCallList(lowrestiles[j][i]);
				}
			}
		}
		//glEnable(GL_FOG);
	}

	// Draw height map
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // less z-fighting artifacts this way, I think
	glEnable(GL_LIGHTING);


	glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glColor4f(1,1,1,1);
	// if we're using shaders let's give it some specular
	if (video.mSupportShaders) {
		Vec4D spec_color(0.1,0.1,0.1,0.1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_color);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 5);

		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	}

	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, detailtexcoords);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, alphatexcoords);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	glClientActiveTexture(GL_TEXTURE0);
	SaveGLSettings();


	glInitNames();
	
	uselowlod = drawfog;

	// height map w/ a zillion texture passes
	//! \todo  Do we need to push the matrix here?
	
	glPushMatrix();

	if( drawterrain )
  {
		for( int j = 0; j < 64; j++ )
    {
			for( int i = 0; i < 64; i++ )
      {
				if( tileLoaded( j, i ) )
        {
					mTiles[j][i].tile->draw( );
        }
      }
    }
  }
  
	glPopMatrix();

	
	if (drawlines)
	{
		glDisable(GL_COLOR_MATERIAL);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		

		setupFog();
		for( int j = 0; j < 64; j++ )
    {
			for( int i = 0; i < 64; i++ )
      {
				if( tileLoaded( j, i ) )
        {
					mTiles[j][i].tile->drawLines( );
					mTiles[j][i].tile->drawMFBO( );
        }
      }
    }
	}
	

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);


	glColor4f(1,1,1,1);
	glEnable(GL_BLEND);

	if (video.mSupportShaders) {
		Vec4D spec_color(0,0,0,1);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_color);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
	}
	


	// unbind hardware buffers
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glEnable(GL_CULL_FACE);

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);

	// TEMP: for fucking around with lighting
	for (int i=0; i<8; i++) {
		GLuint light = GL_LIGHT0 + i;
		glLightf(light, GL_CONSTANT_ATTENUATION, l_const);
		glLightf(light, GL_LINEAR_ATTENUATION, l_linear);
		glLightf(light, GL_QUADRATIC_ATTENUATION, l_quadratic);
	}

	// WMOs / map objects
	if( drawwmo || mHasAGlobalWMO )
		if (video.mSupportShaders) 
		{
			Vec4D spec_color( 1.0f, 1.0f, 1.0f, 1.0f );
			glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
			glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 10 );

			glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR );
		
			for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
				it->second.draw();

			spec_color = Vec4D( 0.0f, 0.0f, 0.0f, 1.0f );
			glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, spec_color );
			glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, 0 );
		}
		else
			for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
				it->second.draw();

	outdoorLights( true );
	setupFog( );

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	// M2s / models	
	if( drawmodels)
	{
		glEnable(GL_LIGHTING);	//! \todo  Is this needed? Or does this fuck something up?
		for( std::map<int, ModelInstance>::iterator it = mModelInstances.begin(); it != mModelInstances.end(); ++it )
			it->second.draw();

		//drawModelList();
	}
	glDisable(GL_CULL_FACE);

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	//glEnable(GL_LIGHTING);

	// gosh darn alpha blended evil

	LoadGLSettings();
	// TODO Steff: Liquid generates GL_OUT_OF_MEMORY
	setupFog();
	
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        mTiles[j][i].tile->drawWater( );
      }
    }
  }
	
	glColor4f(1,1,1,1);
	glEnable(GL_BLEND);

	/*
	// temp frustum code
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBegin(GL_TRIANGLES);
	glColor4f(0,1,0,0.5);
	glVertex3fv(camera);
	glVertex3fv(fp - rt * fl * 1.33f - up * fl);
	glVertex3fv(fp + rt * fl * 1.33f - up * fl);
	glColor4f(0,0,1,0.5);
	glVertex3fv(camera);
	fl *= 0.5f;
	glVertex3fv(fp - rt * fl * 1.33f + up * fl);
	glVertex3fv(fp + rt * fl * 1.33f + up * fl);
	glEnd();
	*/

	glColor4f(1,1,1,1);
	glDisable(GL_COLOR_MATERIAL);
  
	
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        mTiles[j][i].tile->drawWater( );
      }
    }
  }
  
  ex = (int)(camera.x / TILESIZE);
  ez = (int)(camera.z / TILESIZE);
}

int	numTimers;
int startTime[25];
void reportModelTimes();

void startTimer()
{
	startTime[numTimers]=SDL_GetTicks();
	numTimers++;
}

void stopTimer()
{
	int endTime=SDL_GetTicks();
	numTimers--;
	Log << endTime-startTime[numTimers] << "ms" << std::endl;
}

int stopTimer2()
{
	int endTime=SDL_GetTicks();
	numTimers--;
	return endTime-startTime[numTimers];
}


void World::drawSelection(int cursorX,int cursorY, bool pOnlyMap )
{
	GLint viewport[4];
	glSelectBuffer(BUFSIZE,SelectBuffer);
	glRenderMode(GL_SELECT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glGetIntegerv(GL_VIEWPORT,viewport);
	gluPickMatrix(cursorX,viewport[3]-cursorY, 7,7,viewport);
	video.set3D_select();

	if( !pOnlyMap )
	{
		if( drawwmo )
			WMOInstance::reset();

		if( drawmodels )
			modelmanager.resetAnim();
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	highresdistance2 = highresdistance * highresdistance;
	mapdrawdistance2 = mapdrawdistance * mapdrawdistance;
	modeldrawdistance2 = modeldrawdistance * modeldrawdistance;
	doodaddrawdistance2 = doodaddrawdistance * doodaddrawdistance;

	gluLookAt(camera.x,camera.y,camera.z, lookat.x,lookat.y,lookat.z, 0, 1, 0);

	// camera is set up
	frustum.retrieve();

	glClear(GL_DEPTH_BUFFER_BIT); 
	glInitNames();

	// height map w/ a zillion texture passes
	uselowlod = drawfog;
	if (drawterrain) 
	{
    for( int j = 0; j < 64; j++ )
    {
      for( int i = 0; i < 64; i++ )
      {
        if( tileLoaded( j, i ) )
        {
          mTiles[j][i].tile->drawSelect( );
        }
      }
    }
	}
	
	if( pOnlyMap )
		return;

	if( drawwmo )
		// WMOs / map objects
		for( std::map<int, WMOInstance>::iterator it = mWMOInstances.begin(); it != mWMOInstances.end(); ++it )
			it->second.drawSelect();

	if( drawmodels )
		// M2s / models
		for( std::map<int, ModelInstance>::iterator it = mModelInstances.begin(); it != mModelInstances.end(); ++it )
			it->second.drawSelect();
}

void World::drawSelectionChunk(int cursorX,int cursorY)
{
	if( !IsSelection( eEntry_MapChunk ) )
		return;

	GLint viewport[4];
	glSelectBuffer(BUFSIZE,SelectBuffer);
	glRenderMode(GL_SELECT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glGetIntegerv(GL_VIEWPORT,viewport);
	gluPickMatrix(cursorX,viewport[3]-cursorY,12,12,viewport);
	video.set3D_select();

	WMOInstance::reset();
	modelmanager.resetAnim();

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	highresdistance2 = highresdistance * highresdistance;
	mapdrawdistance2 = mapdrawdistance * mapdrawdistance;
	modeldrawdistance2 = modeldrawdistance * modeldrawdistance;
	doodaddrawdistance2 = doodaddrawdistance * doodaddrawdistance;

	gluLookAt(camera.x,camera.y,camera.z, lookat.x,lookat.y,lookat.z, 0, 1, 0);

	// camera is set up
	frustum.retrieve();

	glClear(GL_DEPTH_BUFFER_BIT); 
	glInitNames();
	gWorld->GetCurrentSelection( )->data.mapchunk->drawSelect2();
}

void World::getSelection( int pSelectionMode )
{
	unsigned int lMinDist = 0xFFFFFFFF, lOffset = 0, lMinimumName = 0, lHits = glRenderMode( GL_RENDER );

	while( lHits-- > 0 )
	{
		unsigned int lEntries = SelectBuffer[lOffset];

		if( SelectBuffer[lOffset + 1] < lMinDist )
		{			
			lMinDist = SelectBuffer[lOffset + 1];
			lMinimumName = SelectBuffer[lOffset + 3];
		}

		lOffset = lOffset + 3 + lEntries;
	}
	
	if( pSelectionMode == eSelectionMode_General )
		mCurrentSelection = SelectionNames.findEntry( lMinimumName );
	else if( pSelectionMode == eSelectionMode_Triangle )
		mCurrentSelectedTriangle = lMinimumName;
}

void World::tick(float dt)
{
  enterTile(ex,ez);
  
	while (dt > 0.1f) {
		modelmanager.updateEmitters(0.1f);
		dt -= 0.1f;
	}
	modelmanager.updateEmitters(dt);
}

unsigned int World::getAreaID()
{
	MapTile *curTile;

	int mtx,mtz,mcx,mcz;
	mtx = (int) (camera.x / TILESIZE);
	mtz = (int) (camera.z / TILESIZE);
	mcx = (int) (fmod(camera.x, TILESIZE) / CHUNKSIZE);
	mcz = (int) (fmod(camera.z, TILESIZE) / CHUNKSIZE);

	if ((mtx<cx-1) || (mtx>cx+1) || (mtz<cz-1) || (mtz>cz+1)) return 0;
	
	curTile = mTiles[mtz][mtx].tile;
	if(curTile == 0) return 0;

	MapChunk *curChunk = curTile->getChunk(mcx, mcz);

	if(curChunk == 0) return 0;

	return curChunk->areaID;
}


void World::drawTileMode(float ah)
{
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); 
	glEnable(GL_BLEND);
	
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glScalef(zoom,zoom,1.0f);

	glPushMatrix();
	glTranslatef(-camera.x/CHUNKSIZE,-camera.z/CHUNKSIZE,0);

	minX=camera.x/CHUNKSIZE-2.0f*video.ratio/zoom;
	maxX=camera.x/CHUNKSIZE+2.0f*video.ratio/zoom;
	minY=camera.z/CHUNKSIZE-2.0f/zoom;
	maxY=camera.z/CHUNKSIZE+2.0f/zoom;
	
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
  
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        mTiles[j][i].tile->drawTextures( );
      }
    }
  }
  
	glDisableClientState(GL_COLOR_ARRAY);
	
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	

	glPopMatrix();
	if (drawlines) {
		glTranslatef(fmod(-camera.x/CHUNKSIZE,16),fmod(-camera.z/CHUNKSIZE,16),0);
	/*	for(int x=-32;x<=48;x++)
		{
			if(x%16==0)
				glColor4f(0.0f,1.0f,0.0f,0.5f);
			else
				glColor4f(1.0f,0.0f,0.0f,0.5f);
			glBegin(GL_LINES);
			glVertex3f(-32.0f,(float)x,-1);
			glVertex3f(48.0f,(float)x,-1);
			glVertex3f((float)x,-32.0f,-1);
			glVertex3f((float)x,48.0f,-1);
			glEnd();
		}*/
		
		for(float x=-32.0f;x<=48.0f;x+=1.0f)
		{
			if(int(x)%16==0)
				glColor4f(0.0f,1.0f,0.0f,0.5f);
			else
				glColor4f(1.0f,0.0f,0.0f,0.5f);
			glBegin(GL_LINES);
			glVertex3f(-32.0f,x,-1);
			glVertex3f(48.0f,x,-1);
			glVertex3f(x,-32.0f,-1);
			glVertex3f(x,48.0f,-1);
			glEnd();
		}
	}
	
	glPopMatrix();
  
  ex = (int)(camera.x / TILESIZE);
  ez = (int)(camera.z / TILESIZE);
}

bool World::GetVertex(float x,float z, Vec3D *V)
{
	const int newX = (int)(x / TILESIZE);
	const int newZ = (int)(z / TILESIZE);
  
	if( !tileLoaded( newZ, newX ) ) 
	{
		return false;
	}
  
  return mTiles[newZ][newX].tile->GetVertex(x,z,V);
}

void World::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->changeTerrain(x,z,change,radius,BrushType);
          }
        }
      }
    }
  }
  
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->recalcNorms();
          }
        }
      }
    }
  }
}
void World::flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType)
{
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->flattenTerrain(x,z,h,remain,radius,BrushType);
          }
        }
      }
    }
  }
  
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->recalcNorms();
          }
        }
      }
    }
  }
}

void World::blurTerrain(float x, float z, float remain, float radius, int BrushType)
{
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->blurTerrain(x, z, remain, radius, BrushType);
          }
        }
      }
    }
  }
  
  for( int j = 0; j < 64; j++ )
  {
    for( int i = 0; i < 64; i++ )
    {
      if( tileLoaded( j, i ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            mTiles[j][i].tile->getChunk(ty,tx)->recalcNorms();
          }
        }
      }
    }
  }
}

bool World::paintTexture(float x, float z, brush *Brush, float strength, float pressure, int texture)
{
	const int newX = (int)(x / TILESIZE);
	const int newZ = (int)(z / TILESIZE);

	bool succ = false;
  
  for( int j = newZ - 1; j < newZ + 1; j++ )
  {
    for( int i = newX - 1; i < newX + 1; i++ )
    {
      if( tileLoaded( i, j ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            succ = succ || mTiles[j][i].tile->getChunk(ty,tx)->paintTexture(x, z, Brush, strength, pressure, texture);
          }
        }
      }
    }
  }
  return succ;
}

void World::eraseTextures(float x, float z)
{
	const int newX = (int)(x / TILESIZE);
	const int newZ = (int)(z / TILESIZE);
  
  for( int j = newZ - 1; j < newZ + 1; j++ )
  {
    for( int i = newX - 1; i < newX + 1; i++ )
    {
      if( tileLoaded( i, j ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              chunk->eraseTextures();
            }
          }
        }
      }
    }
  }
}

void World::addHole( float x, float z )
{
	const int newX = (int)(x / TILESIZE);
	const int newZ = (int)(z / TILESIZE);
  
  for( int j = newZ - 1; j < newZ + 1; j++ )
  {
    for( int i = newX - 1; i < newX + 1; i++ )
    {
      if( tileLoaded( i, j ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
              int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
              chunk->addHole( k, l );
            }
          }
        }
      }
    }
  }
}

void World::removeHole( float x, float z )
{
	const int newX = (int)(x / TILESIZE);
	const int newZ = (int)(z / TILESIZE);
  
  for( int j = newZ - 1; j < newZ + 1; j++ )
  {
    for( int i = newX - 1; i < newX + 1; i++ )
    {
      if( tileLoaded( i, j ) )
      {
        for( int ty = 0; ty < 16; ty++ )
        {
          for( int tx = 0; tx < 16; tx++ )
          {
            MapChunk* chunk = mTiles[j][i].tile->getChunk( ty, tx );
            if( chunk->xbase < x && chunk->xbase + CHUNKSIZE > x && chunk->zbase < z && chunk->zbase + CHUNKSIZE > z )
            {
              int k = ( x - chunk->xbase ) / MINICHUNKSIZE;
              int l = ( z - chunk->zbase ) / MINICHUNKSIZE;
              chunk->removeHole( k, l );
            }
          }
        }
      }
    }
  }
}

void World::saveMap()
{
	//! \todo  Output as BLP.
	unsigned char image[256*256*3];
	char tfname[255];
	MapTile *ATile;
	FILE *fid;
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glReadBuffer(GL_BACK);

	minX=-64*16;
	maxX=64*16;
	minY=-64*16;
	maxY=64*16;

	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	for(int y=0;y<64;y++)
	{
		for(int x=0;x<64;x++)
		{
      if( !( mTiles[y][x].flags & 1 ) ) 
      {
        continue;
      }
      
			ATile=loadTile(x,y);
			glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); 

			glPushMatrix();
			glScalef(0.08333333f,0.08333333f,1.0f);

			//glTranslatef(-camera.x/CHUNKSIZE,-camera.z/CHUNKSIZE,0);
			glTranslatef((float)-x*16-8,(float)-y*16-8,0);
	
			ATile->drawTextures();
			glPopMatrix();
			glReadPixels(video.xres/2-128,video.yres/2-128,256,256,GL_RGB,GL_UNSIGNED_BYTE,image);
			SDL_GL_SwapBuffers();
			sprintf(tfname,"%s_map_%d_%d.raw",basename.c_str(),x,y);
			fid=fopen(tfname,"wb");
			fwrite(image,256*3,256,fid);
			fclose(fid);
		}
	}

	glDisableClientState(GL_COLOR_ARRAY);
	
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}

void World::deleteModelInstance( int pUniqueID )
{
	mModelInstances.erase( mModelInstances.find( pUniqueID ) );
	ResetSelection( );
}

void World::deleteWMOInstance( int pUniqueID )
{
	mWMOInstances.erase( mWMOInstances.find( pUniqueID ) );
	ResetSelection( );
}

void World::addModel( nameEntry entry, Vec3D newPos )
{
	int lModelMax = 0;
	if (mModelInstances.empty() == false)
	lModelMax = mModelInstances.rbegin( )->first;
	int lObjectMax = 0;
	if (mWMOInstances.empty() == false)
	lObjectMax = mWMOInstances.rbegin( )->first;
	int lMaxUID = ( lModelMax > lObjectMax ? lModelMax : lObjectMax ) + 1;

	if( entry.type == eEntry_Model )
	{
		ModelInstance newModelis;
		newModelis = *entry.data.model;
		newModelis.nameID = -1;
		newModelis.d1 = lMaxUID;
		newModelis.pos = newPos;

		if(Settings::getInstance()->copy_rot)
		{
			newModelis.dir.y += (rand() % 360 + 1);
		}
		
		if(Settings::getInstance()->copy_tile)
		{
			newModelis.dir.x += (rand() % 5 + 1);
			newModelis.dir.z += (rand() % 5 + 1);
		}
		
		if(Settings::getInstance()->copy_size)
		{

			newModelis.sc = newModelis.sc * (( float( rand( ) ) / float( RAND_MAX ) * 0.2 ) + 0.90);
		}
		mModelInstances.insert( std::pair<int,ModelInstance>( lMaxUID, newModelis ));
	}
	else if( entry.type == eEntry_WMO )
	{
		WMOInstance newWMOis(*entry.data.wmo);
		newWMOis.pos = newPos;
		newWMOis.id = lMaxUID;
		newWMOis.wmoID = lMaxUID;
		newWMOis.nameID = -1;
		mWMOInstances.insert( std::pair<int,WMOInstance>( lMaxUID, newWMOis ));
	}
}

void World::addM2( Model *model, Vec3D newPos )
{
	int lModelMax = 0;
	if (mModelInstances.empty() == false)
	lModelMax = mModelInstances.rbegin( )->first;
	int lObjectMax = 0;
	if (mWMOInstances.empty() == false)
	lObjectMax = mWMOInstances.rbegin( )->first;
	int lMaxUID = ( lModelMax > lObjectMax ? lModelMax : lObjectMax ) + 1;
	
		ModelInstance newModelis;
		newModelis.model = model;
		newModelis.nameID = -1;
		newModelis.d1 = lMaxUID;
		newModelis.pos = newPos;
		newModelis.sc = 1;

		if(Settings::getInstance()->copy_rot)
		{
			newModelis.dir.y += (rand() % 360 + 1);
		}
		
		if(Settings::getInstance()->copy_tile)
		{
			newModelis.dir.x += (rand() % 5 + 1);
			newModelis.dir.z += (rand() % 5 + 1);
		}
		
		if(Settings::getInstance()->copy_size)
		{

			newModelis.sc = newModelis.sc * (( float( rand( ) ) / float( RAND_MAX ) * 0.2 ) + 0.90);
		}
		mModelInstances.insert( std::pair<int,ModelInstance>( lMaxUID, newModelis ));
	
}

void World::addWMO( WMO *wmo, Vec3D newPos )
{
	int lModelMax = 0;
	if (mModelInstances.empty() == false)
	lModelMax = mModelInstances.rbegin( )->first;
	int lObjectMax = 0;
	if (mWMOInstances.empty() == false)
	lObjectMax = mWMOInstances.rbegin( )->first;
	int lMaxUID = ( lModelMax > lObjectMax ? lModelMax : lObjectMax ) + 1;
	
	WMOInstance newWMOis(wmo);
	newWMOis.pos = newPos;
	newWMOis.id = lMaxUID;
	newWMOis.wmoID = lMaxUID;
	newWMOis.nameID = -1;
	mWMOInstances.insert( std::pair<int,WMOInstance>( lMaxUID, newWMOis ));
	
}