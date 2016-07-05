#ifndef WORLD_H
#define WORLD_H

#include <map>
#include <string>

#include "Frustum.h" // Frustum
#include "Model.h" // ModelManager
#include "Selection.h" // nameEntryManager
#include "Sky.h" // Skies, OutdoorLighting, OutdoorLightStats
#include "WMO.h" // WMOManager

namespace OpenGL
{
	class CallList;
	class Texture;
};

class Brush;
class MapTile;
class MapIndex;

extern nameEntryManager SelectionNames;

static const float detail_size = 8.0f;
static const float highresdistance = 384.0f;
static const float mapdrawdistance = 998.0f;
static const float modeldrawdistance = 384.0f;
static const float doodaddrawdistance = 64.0f;

typedef unsigned short StripType;

class World
{
public:
	// Which tile are we over / entering?
	int ex;
	int ez;
	int cx;
	int cz;

	MapIndex *mapIndex;

	// Information about the currently selected model / WMO / triangle.
	nameEntry *mCurrentSelection;
	int mCurrentSelectedTriangle;
	bool SelectionMode;

	// Call lists for the low resolution heightmaps.
	OpenGL::CallList *lowrestiles[64][64];

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

	void outdoorLighting();
	void outdoorLighting2();

	unsigned int getMapID();
	// Do we draw *? Should be moved somewhere else, these are not World related.
	bool drawdoodads;
	bool drawfog;
	bool drawlines;
	bool drawmodels;
	bool drawterrain;
	bool drawwater;
	bool drawwmo;
	bool drawwireframe;
	bool lighting;
	bool renderAnimations;
	// Time of the day.
	float animtime;
	float time;

	//! \brief Name of this map.
	std::string basename;

	// Dynamic distances for rendering. Actually, these should be the same..
	float fogdistance;
	float culldistance;

	float minX;
	float maxX;
	float minY;
	float maxY;
	float zoom;

	Skies *skies;

	bool loading;

	bool autoheight;

	//! \todo  Get these managed? ._.
	std::map<int, ModelInstance> mModelInstances;
	std::map<int, WMOInstance> mWMOInstances;

	OutdoorLightStats outdoorLightStats;

	GLuint minimap;

	StripType *mapstrip;
	StripType *mapstrip2;

	Vec3D camera;
	Vec3D lookat;
	Frustum frustum;

	explicit World(const std::string& name);
	~World();

	void initDisplay();

	void tick(float dt);
	void draw();

	void outdoorLights(bool on);
	void setupFog();

	//! \brief Get the area ID of the tile on which the camera currently is on.
	unsigned int getAreaID();
	void setAreaID(int id, int x, int z);
	void setAreaID(int id, int x, int z, int cx, int cz);
	void setBaseTexture(int x, int z);

	//void moveADT(); does not exist
	//void drawSelectionChunk(int cursorX,int cursorY); does not exist
	//bool hasAdt(); does not exist

	void drawSelection(int cursorX, int cursorY, bool pOnlyMap = false);
	void drawTileMode(float ah);

	void initGlobalVBOs(GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords);

	bool HasSelection();

	// Selection related methods.
	bool IsSelection(int pSelectionType);
	nameEntry * GetCurrentSelection() { return mCurrentSelection; }
	void ResetSelection() { mCurrentSelection = NULL; }
	GLuint GetCurrentSelectedTriangle() { return (unsigned int)mCurrentSelectedTriangle; }

	bool GetVertex(float x, float z, Vec3D *V);

	void changeTerrain(float x, float z, float change, float radius, int BrushType);
	void changeShader(float x, float z, float change, float radius, bool editMode);
  void flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType, float angle = 0.0f, float orientation = 0.0f);
	void blurTerrain(float x, float z, float remain, float radius, int BrushType);
	bool paintTexture(float x, float z, Brush *brush, float strength, float pressure, OpenGL::Texture* texture);
	void eraseTextures(float x, float z);
	void overwriteTextureAtCurrentChunk(float x, float z, OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture);

	void addHole(float x, float y, float z, bool big);
	void removeHole(float x, float z, bool big);

	void addModel(nameEntry entry, Vec3D newPos, bool copyit);
	void addM2(std::string const& filename, Vec3D newPos, bool copyit);
	void addWMO(std::string const& filename, Vec3D newPos, bool copyit);

	void jumpToCords(Vec3D pos);
	void saveMap();

	//void unsetChanged(int x, int z); does not exist
	//int getChanged(int x, int z); does not exist

	void deleteModelInstance(int pUniqueID);
	void deleteWMOInstance(int pUniqueID);

	void delete_duplicate_model_and_wmo_instances();

  void ensure_instance_maps_having_correct_keys_and_unlock_uids();

	static bool IsEditableWorld(int pMapId);
	void clearHeight(int id, int x, int z);
	void clearHeight(int id, int x, int z, int _cx, int _cz);
	void moveHeight(int id, int x, int z);
	void moveHeight(int id, int x, int z, int _cx, int _cz);

	void saveWDT();
	void clearAllModelsOnADT(int x, int z);
	void swapTexture(int x, int z, OpenGL::Texture *tex);
  void removeTexDuplicateOnADT(int x, int z);

	bool canWaterSave(int x, int y);

	void setWaterHeight(int x, int y, float h);
	float getWaterHeight(int x, int y);
	float HaveSelectWater(int x, int y);
	void CropWaterADT(int x, int z);
	void setWaterTrans(int x, int y, unsigned char value);
	unsigned char getWaterTrans(int x, int y);

	void setWaterType(int x, int y, int type);
	int getWaterType(int x, int y);

	void deleteWaterLayer(int x, int z);
	void ClearShader(int x, int z);

	void addWaterLayer(int x, int z);
	void addWaterLayer(int x, int z, float height, unsigned char trans);
	void addWaterLayerChunk(int x, int z, int i, int j);
	void delWaterLayerChunk(int x, int z, int i, int j);

	void autoGenWaterTrans(int x, int y, int factor);
	void AddWaters(int x, int y);

  void fixAllGaps();

  void convertMapToBigAlpha();
private:
	void getSelection();
};

extern World *gWorld;

//void lightingDefaults(); not used
//void myFakeLighting(); not used

#endif
