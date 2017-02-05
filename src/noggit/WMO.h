// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.hpp>
#include <math/ray.hpp>
#include <math/vector_3d.hpp>
#include <noggit/MPQ.h>
#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/ModelManager.h>
#include <noggit/TextureManager.h>
#include <noggit/Video.h>
#include <noggit/multimap_with_normalized_key.hpp>
#include <noggit/wmo_liquid.hpp>
#include <opengl/call_list.hpp>

#include <boost/optional.hpp>

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class WMO;
class WMOGroup;
class WMOInstance;
class WMOManager;
class wmo_liquid;
class Model;

struct WMOBatch {
  signed char bytes[12];
  uint32_t indexStart;
  uint16_t indexCount, vertexStart, vertexEnd;
  unsigned char flags, texture;
};

class WMOGroup {
  WMO *wmo;
  uint32_t flags;
  math::vector_3d v1, v2;
  uint32_t nTriangles, nVertices;
  math::vector_3d center;
  float rad;
  int32_t num;
  int32_t fog;
  int32_t nDoodads, nBatches;
  std::vector<int16_t> ddr;
  std::unique_ptr<wmo_liquid> lq;
  std::vector< std::pair<std::unique_ptr<opengl::call_list>, bool> > _lists;

  std::vector<math::vector_3d> vertices;
  std::vector<WMOBatch> batches;
  std::vector<uint16_t> indices;

public:
  math::vector_3d BoundingBoxMin;
  math::vector_3d BoundingBoxMax;
  math::vector_3d VertexBoxMin;
  math::vector_3d VertexBoxMax;
  bool indoor, hascv;
  bool visible;

  bool outdoorLights;
  std::string name;

  WMOGroup() :nBatches(0) {}
  void init(WMO *wmo, MPQFile* f, int num, char *names);
  void initDisplayList();
  void initLighting(int nLR, uint16_t *useLights);
  void draw(const math::vector_3d& ofs, math::degrees const, Frustum const&);
  void drawLiquid();
  void drawDoodads(unsigned int doodadset, const math::vector_3d& ofs, math::degrees const, Frustum const&);
  void setupFog();
  void intersect (math::ray const&, std::vector<float>* results) const;
};

struct WMOLight {
  uint32_t flags, color;
  math::vector_3d pos;
  float intensity;
  float unk[5];
  float r;

  math::vector_4d fcolor;

  void init(MPQFile* f);
  void setup(GLint light);

  static void setupOnce(GLint light, math::vector_3d dir, math::vector_3d lcol);
};

struct WMOPV {
  math::vector_3d a, b, c, d;
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

struct WMOFog {
  unsigned int flags;
  math::vector_3d pos;
  float r1, r2, fogend, fogstart;
  unsigned int color1;
  float f2;
  float f3;
  unsigned int color2;
  // read to here (0x30 bytes)
  math::vector_4d color;
  void init(MPQFile* f);
  void setup();
};

class WMO
{
public:
  bool draw_group_boundingboxes;

  const std::string& filename() const;

  //std::string WMOName;
  std::string _filename;
  std::vector<WMOGroup> groups;
  unsigned int nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, nX;
  std::vector<WMOMaterial> mat;
  math::vector_3d extents[2];
  std::vector<std::string> textures;
  std::vector<std::string> models;
  std::vector<ModelInstance> modelis;

  std::vector<WMOLight> lights;
  std::vector<WMOPV> pvs;
  std::vector<WMOPR> prs;

  std::vector<WMOFog> fogs;

  std::vector<WMODoodadSet> doodadsets;

  boost::optional<scoped_model_reference> skybox;

  explicit WMO(const std::string& name);
  void draw(int doodadset, const math::vector_3d& ofs, math::degrees const, bool boundingbox, bool groupboxes, bool highlight, Frustum const&);
  std::vector<float> intersect (math::ray const&) const;
  //void drawPortals();
  bool drawSkybox(math::vector_3d pCamera, math::vector_3d pLower, math::vector_3d pUpper) const;
};

class WMOManager
{
public:
  static void report();

private:
  friend struct scoped_wmo_reference;
  static noggit::multimap_with_normalized_key<WMO> _;
};

struct scoped_wmo_reference
{
  scoped_wmo_reference (std::string const& filename)
    : _valid (true)
    , _filename (filename)
    , _wmo (WMOManager::_.emplace (_filename))
  {}

  scoped_wmo_reference (scoped_wmo_reference const& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _wmo (WMOManager::_.emplace (_filename))
  {}
  scoped_wmo_reference& operator= (scoped_wmo_reference const& other)
  {
    _valid = other._valid;
    _filename = other._filename;
    _wmo = WMOManager::_.emplace (_filename);
    return *this;
  }

  scoped_wmo_reference (scoped_wmo_reference&& other)
    : _valid (other._valid)
    , _filename (other._filename)
    , _wmo (other._wmo)
  {
    other._valid = false;
  }
  scoped_wmo_reference& operator= (scoped_wmo_reference&& other)
  {
    std::swap(_valid, other._valid);
    std::swap(_filename, other._filename);
    std::swap(_wmo, other._wmo);
    other._valid = false;
    return *this;
  }

  ~scoped_wmo_reference()
  {
    if (_valid)
    {
      WMOManager::_.erase (_filename);
    }
  }

  WMO* operator->() const
  {
    return _wmo;
  }
  WMO* get() const
  {
    return _wmo;
  }

private:
  bool _valid;

  std::string _filename;
  WMO* _wmo;
};
