#ifndef WORLD_H
#define WORLD_H

#include <string>

#include "frustum.h" // Frustum
#include "wmo.h" // WMOManager
#include "model.h" // ModelManager
#include "selection.h" // nameEntryManager
#include "sky.h" // Skies, OutdoorLighting, OutdoorLightStats

class brush;
class MapTile;

static const int MAPTILECACHESIZE = 36;

extern nameEntryManager SelectionNames;

static const float detail_size = 8.0f;

bool IsEditableWorld( int pMapId );


enum eSelectionMode
{
	eSelectionMode_General,
	eSelectionMode_Triangle
};

struct MapTileEntry
{
  uint32_t flags;
  MapTile* tile;
};

class World 
{
	//MapTile *maptilecache[MAPTILECACHESIZE];
	//MapTile *current[5][5];
	int ex,ez;

	bool mBigAlpha;

  MapTileEntry mTiles[64][64];
	

public:
	float LastRaise;
	float zoom;
	bool SelectionMode;	
	
	std::string basename;

	//bool maps[64][64];
	GLuint lowrestiles[64][64];
	bool autoheight;

	bool mHasAGlobalWMO;
	std::string mWmoFilename;
	ENTRY_MODF mWmoEntry;

	float mapdrawdistance, modeldrawdistance, doodaddrawdistance, highresdistance;
	float mapdrawdistance2, modeldrawdistance2, doodaddrawdistance2, highresdistance2;

	float culldistance, culldistance2, fogdistance;

	float l_const, l_linear, l_quadratic;

	Skies *skies;
	float time,animtime;

    bool hadSky;

	bool lighting, drawmodels, drawdoodads, drawterrain, drawwmo, loading, drawfog, drawlines, drawwater, uselowlod;

	GLuint detailtexcoords, alphatexcoords;

	short *mapstrip,*mapstrip2;

	Vec3D camera, lookat;
	Frustum frustum;
	int cx,cz;
	bool noadt;

	unsigned int mMapId;

	WMOManager wmomanager;
	ModelManager modelmanager;
	//! \todo  Get these managed? ._.
	std::map<int, ModelInstance> mModelInstances;
	std::map<int, WMOInstance> mWMOInstances;

	OutdoorLighting *ol;
	OutdoorLightStats outdoorLightStats;

	GLuint minimap;

	World( const std::string& name);
	~World();
	void init();
	void initMinimap();
	void initDisplay();
	void initLowresTerrain();

	void onTheFlyLoading();

	void enterTile(int x, int z);
	bool tileLoaded(int x, int z);
	MapTile *loadTile(int x, int z);
	void reloadTile(int x, int z);
	void saveTile(int x, int z);
	void tick(float dt);
	void draw();
	

	void outdoorLighting();
	void outdoorLighting2();
	void outdoorLights(bool on);
	void setupFog();

	/// Get the tile on wich the camera currently is on
	unsigned int getAreaID();

	void drawSelection(int cursorX,int cursorY, bool pOnlyMap = false );
	void drawSelectionChunk(int cursorX,int cursorY);
	void drawTileMode(float ah);


	void getSelection( int pSelectionMode );

private:
	nameEntry * mCurrentSelection;
	GLuint mCurrentSelectedTriangle;

public:
	bool HasSelection( ) { return mCurrentSelection; }
	bool IsSelection( int pSelectionType ) { return HasSelection( ) && mCurrentSelection->type == pSelectionType; }
	nameEntry * GetCurrentSelection( ) { return mCurrentSelection; }
	void ResetSelection( ) { mCurrentSelection = 0; }
	GLuint GetCurrentSelectedTriangle( ) { return mCurrentSelectedTriangle; }

	bool GetVertex(float x,float z, Vec3D *V);
	void changeTerrain(float x, float z, float change, float radius, int BrushType);
	void flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);	
	void blurTerrain(float x, float z, float remain, float radius, int BrushType);
	bool paintTexture(float x, float z, brush *Brush, float strength, float pressure, int texture);
	void eraseTextures(float x, float z);
	void addHole( float x, float z );
	void addModel( nameEntry entry, Vec3D newPos);
	void addM2( Model *model, Vec3D newPos );
	void addWMO( WMO *wmo, Vec3D newPos );
	void removeHole( float x, float z );
	void saveMap();

	void deleteModelInstance( int pUniqueID );
	void deleteWMOInstance( int pUniqueID );
  
  bool hasTile( int pX, int pZ );

	float minX,maxX,minY,maxY;
};


extern World *gWorld;


void lightingDefaults();
void myFakeLighting();



#endif
