#ifndef WORLD_H
#define WORLD_H

#include <string>

#include "frustum.h" // Frustum
#include "wmo.h" // WMOManager
#include "model.h" // ModelManager
#include "selection.h" // nameEntryManager
#include "sky.h" // Skies, OutdoorLighting, OutdoorLightStats

class OpenGL::CallList;
class brush;
class MapTile;

extern nameEntryManager SelectionNames;

static const float detail_size = 8.0f;
static const float highresdistance = 384.0f;
static const float mapdrawdistance = 998.0f;
static const float modeldrawdistance = 384.0f;
static const float doodaddrawdistance = 64.0f;

enum eSelectionMode
{
	eSelectionMode_General,
	eSelectionMode_Triangle
};

/*!
 \brief This class is only a holder to have easier access to MapTiles and their flags for easier WDT parsing. This is private and for the class World only.
 */
class MapTileEntry
{
private:
	uint32_t flags;
	MapTile* tile;
	
	MapTileEntry() : flags( 0 ), tile( NULL ) {}
	
	friend class World;
};

//! \todo Split this. There should be a seperate class for WDTs.
class World 
{
	// Which tile are we over / entering?
	int cx;
	int cz;
	int ex;
	int ez;

	// Holding all MapTiles there can be in a World.
	MapTileEntry mTiles[64][64];
	
	// Information about the currently selected model / WMO / triangle.
	nameEntry* mCurrentSelection;
	int mCurrentSelectedTriangle;
	bool SelectionMode;
	
	// Is the WDT telling us to use a different alphamap structure.
	bool mBigAlpha;
	
	// Call lists for the low resolution heightmaps.
	OpenGL::CallList* lowrestiles[64][64];

	// Temporary variables for loading a WMO, if we have a global WMO.
	std::string mWmoFilename;
	ENTRY_MODF mWmoEntry;
	
	// Vertex Buffer Objects for coordinates used for drawing.
	GLuint detailtexcoords;
	GLuint alphatexcoords;

	// Map ID of this World.
	unsigned int mMapId;
	
	// The lighting used.
	OutdoorLighting *ol;
	
	// Light attenuation related parameters.
	float l_const;
	float l_linear;
	float l_quadratic;
	

	void initMinimap();
	void initLowresTerrain();
	
	bool tileLoaded(int x, int z);
	MapTile *loadTile(int x, int z);
	
	void outdoorLighting();
	void outdoorLighting2();
	
public:
	unsigned int getMapID();
	// Do we draw *? Should be moved somewhere else, these are not World related.
	bool drawdoodads;
	bool drawfog;
	bool drawlines;
	bool drawmodels;
	bool drawterrain;
	bool drawwater;
	bool drawwmo;
	bool lighting;
	bool uselowlod;
	
	// Time of the day.
	float animtime;
	float time;
	
	//! \brief Name of this map.
	std::string basename;
	
	// Dynamic distances for rendering. Actually, these should be the same..
	float fogdistance;
	float culldistance;
	
	bool autoheight;
	
	float minX;
	float maxX;
	float minY;
	float maxY;
	float zoom;
	
	Skies *skies;
	bool mHasAGlobalWMO;
	bool loading;
	bool noadt;
	bool hadSky;
	
	//! \todo	Get these managed? ._.
	std::map<int, ModelInstance> mModelInstances;
	std::map<int, WMOInstance> mWMOInstances;
	
	OutdoorLightStats outdoorLightStats;
	
	GLuint minimap;
	
	short *mapstrip;
	short *mapstrip2;
	
	Vec3D camera;
	Vec3D lookat;
	Frustum frustum;

	World( const std::string& name);
	~World();

	void initDisplay();
	void enterTile(int x, int z);
	void reloadTile(int x, int z);
	void saveTile(int x, int z);
	void saveChanged();
	void tick(float dt);
	void draw();
	
	void outdoorLights(bool on);
	void setupFog();
	
	//! \brief Get the area ID of the tile on which the camera currently is on.
	unsigned int getAreaID();
	void setAreaID(int id, int x, int z);
	void setAreaID(int id, int x, int z , int cx, int cz);
	void setFlag(bool to, float x, float z);

	void moveADT();

	void drawSelection(int cursorX,int cursorY, bool pOnlyMap = false );
	void drawSelectionChunk(int cursorX,int cursorY);
	void drawTileMode(float ah);
	
	// Selection related methods.
	void getSelection( int pSelectionMode );
	bool HasSelection() { return mCurrentSelection; }
	bool IsSelection( int pSelectionType ) { return HasSelection() && mCurrentSelection->type == pSelectionType; }
	nameEntry * GetCurrentSelection() { return mCurrentSelection; }
	void ResetSelection() { mCurrentSelection = NULL; }
	GLuint GetCurrentSelectedTriangle() { return mCurrentSelectedTriangle; }
	
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

	void setChanged(float x, float z);
	void setChanged(int x, int z);
	void unsetChanged(int x, int z);
	bool getChanged(int x, int z);

	void deleteModelInstance( int pUniqueID );
	void deleteWMOInstance( int pUniqueID );
	
	bool hasTile( int pX, int pZ );
	
	static bool IsEditableWorld( int pMapId );
	void clearHeight(int id, int x, int z);
	void clearHeight(int id, int x, int z , int _cx, int _cz);
	void moveHeight(int id, int x, int z);
	void moveHeight(int id, int x, int z , int _cx, int _cz);
};

extern World *gWorld;

void lightingDefaults();
void myFakeLighting();

#endif
