#ifndef WMO_H
#define WMO_H

#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>

#include "Manager.h"
#include "ModelInstance.h" // ModelInstance
#include "MPQ.h"
#include "Quaternion.h"
#include "Vec3D.h"
#include "Video.h"

class WMO;
class WMOGroup;
class WMOInstance;
class WMOManager;
class Liquid;
class Model;

class WMOGroup {
  WMO *wmo;
  uint32_t flags;
  Vec3D v1,v2;
  uint32_t nTriangles, nVertices;
  Vec3D center;
  float rad;
  int32_t num;
  int32_t fog;
  int32_t nDoodads, nBatches;
  int16_t *ddr;
  Liquid *lq;
  std::vector< std::pair<OpenGL::CallList*, bool> > _lists;
public:
  Vec3D BoundingBoxMin;
  Vec3D BoundingBoxMax;
  Vec3D VertexBoxMin;
  Vec3D VertexBoxMax;
  bool indoor, hascv;
  bool visible;

  bool outdoorLights;
  std::string name;

  WMOGroup():nBatches(0) {}
  ~WMOGroup();
  void init(WMO *wmo, MPQFile* f, int num, char *names);
  void initDisplayList();
  void initLighting(int nLR, uint16_t *useLights);
  void draw(World* world, const Vec3D& ofs, const float rot,bool selection);
  void drawLiquid(World* world);
  void drawDoodads(World* world, unsigned int doodadset, const Vec3D& ofs, const float rot);
  void drawDoodadsSelect(World* world, unsigned int doodadset, const Vec3D& ofs, const float rot);
  void setupFog(World* world);
};

struct WMOMaterial {
  int32_t flags;
  int32_t specular;
  int32_t transparent; // Blending: 0 for opaque, 1 for transparent
  int32_t nameStart; // Start position for the first texture filename in the MOTX data block
  uint32_t col1; // color
  int32_t d3; // flag
  int32_t nameEnd; // Start position for the second texture filename in the MOTX data block
  uint32_t col2; // color
  int32_t d4; // flag
  uint32_t col3;
  float f2;
  float diffColor[3];
  uint32_t texture1; // this is the first texture object. of course only in RAM. leave this alone. :D
  uint32_t texture2; // this is the second texture object.
  // read up to here -_-
  OpenGL::Texture* _texture;
};

struct WMOLight {
  uint32_t flags, color;
  Vec3D pos;
  float intensity;
  float unk[5];
  float r;

  Vec4D fcolor;

  void init(MPQFile* f);
  void setup(GLint light);

  static void setupOnce(GLint light, Vec3D dir, Vec3D lcol);
};

struct WMOPV {
  Vec3D a,b,c,d;
};

struct WMOPR {
  int16_t portal, group, dir, reserved;
};

struct WMODoodadSet {
  char name[0x14];
  int32_t start;
  int32_t size;
  int32_t unused;
};

struct WMOLiquidHeader {
  int32_t X, Y, A, B;
  Vec3D pos;
  int16_t type;
};

struct WMOFog {
  unsigned int flags;
  Vec3D pos;
  float r1, r2, fogend, fogstart;
  unsigned int color1;
  float f2;
  float f3;
  unsigned int color2;
  // read to here (0x30 bytes)
  Vec4D color;
  void init(MPQFile* f);
  void setup();
};

class WMO : public ManagedItem
{
public:
  bool draw_group_boundingboxes;

  const std::string& filename() const;

  //std::string WMOName;
  std::string _filename;
  WMOGroup *groups;
  unsigned int nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, nX;
  WMOMaterial *mat;
  Vec3D extents[2];
  std::vector<std::string> textures;
  std::vector<std::string> models;
  std::vector<ModelInstance> modelis;

  std::vector<WMOLight> lights;
  std::vector<WMOPV> pvs;
  std::vector<WMOPR> prs;

  std::vector<WMOFog> fogs;

  std::vector<WMODoodadSet> doodadsets;

  Model *skybox;
  std::string skyboxFilename;

  //! \todo This only has World* for wmo-doodads. ._.
  explicit WMO(World* world, const std::string& name);
  ~WMO();
  void draw(World* world, int doodadset, const Vec3D& ofs, const float rot, bool boundingbox, bool groupboxes, bool highlight) const;
  void drawSelect(World* world, int doodadset, const Vec3D& ofs, const float rot) const;
  //void drawPortals();
  bool drawSkybox(World* world, Vec3D pCamera, Vec3D pLower, Vec3D pUpper ) const;
};

class WMOManager
{
public:
  static void delbyname( std::string name );
  //! \todo This only has World* for wmo-doodads. ._.
  static WMO* add(World* world, std::string name);

  static void report();

private:
  typedef std::map<std::string, WMO*> mapType;
  static mapType items;
};


#endif
