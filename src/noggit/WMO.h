// WMO.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef WMO_H
#define WMO_H

#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>

#include <opengl/types.h>

#include <math/vector_3d.h>
#include <math/vector_4d.h>

#include <noggit/async/object.h>
#include <noggit/ModelInstance.h> // ModelInstance
#include <noggit/multimap_with_normalized_key.hpp>
#include <noggit/TextureManager.h>

#include <boost/optional.hpp>

namespace opengl
{
  class call_list;
  class texture;
}

class Frustum;
class WMO;
class WMOGroup;
class WMOInstance;
class Liquid;
class Model;

namespace noggit
{
  namespace mpq
  {
    class file;
  }
}

class WMOGroup {
  WMO *wmo;
  uint32_t flags;
  ::math::vector_3d v1,v2;
  uint32_t nTriangles, nVertices;
  ::math::vector_3d center;
  float rad;
  int32_t num;
  int32_t fog;
  int32_t nDoodads;
  size_t nBatches;
  int16_t *ddr;
  Liquid *lq;
  std::vector< std::pair<opengl::call_list*, bool> > _lists;
public:
  ::math::vector_3d BoundingBoxMin;
  ::math::vector_3d BoundingBoxMax;
  ::math::vector_3d VertexBoxMin;
  ::math::vector_3d VertexBoxMax;
  bool indoor, hascv;

  bool outdoorLights;
  std::string name;

  WMOGroup():nBatches(0) {}
  ~WMOGroup();
  void init(WMO *wmo, noggit::mpq::file* f, int num, char *names);
  void initDisplayList();
  void initLighting(int nLR, uint16_t *useLights);
  bool is_visible ( const ::math::vector_3d& offset
                  , const float& rotation
                  , const float& cull_distance
                  , const Frustum& frustum
                  , const ::math::vector_3d& camera
                  ) const;
  void draw ( World* world
            , bool draw_fog
            , bool hasSkies
            , const float& fog_distance
            );
  void draw_for_selection() const;
  void drawLiquid ( World* world
                  , bool draw_fog
                  , const float& fog_distance
                  );
  void drawDoodads ( World* world
                   , unsigned int doodadset
                   , const ::math::vector_3d& ofs
                   , const float rot
                   , bool draw_fog
                   , const float& fog_distance
                   , const Frustum& frustum
                   , const float& cull_distance
                   , const ::math::vector_3d& camera
                   );
  void drawDoodadsSelect ( unsigned int doodadset
                         , const ::math::vector_3d& ofs
                         , const float rot
                         , const Frustum& frustum
                         , const float& cull_distance
                         , const ::math::vector_3d& camera
                         );
  void setupFog ( World* world
                , bool draw_fog
                , const float& fog_distance
                );
};

struct SMOMaterial
{
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
};

struct WMOMaterial : public SMOMaterial
{
  noggit::scoped_blp_texture_reference _texture;

  WMOMaterial (SMOMaterial material, std::string filename)
    : SMOMaterial (std::move (material))
    , _texture (filename)
  {}
};

struct WMOLight {
  uint32_t flags, color;
  ::math::vector_3d pos;
  float intensity;
  float unk[5];
  float r;

  ::math::vector_4d fcolor;

  void init(noggit::mpq::file* f);
  void setup(opengl::light light);

  static void setupOnce(opengl::light light, ::math::vector_3d dir, ::math::vector_3d lcol);
};

struct WMOPV {
  ::math::vector_3d a,b,c,d;
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
  ::math::vector_3d pos;
  int16_t type;
};

struct WMOFog {
  unsigned int flags;
  ::math::vector_3d pos;
  float r1, r2, fogend, fogstart;
  unsigned int color1;
  float f2;
  float f3;
  unsigned int color2;
  // read to here (0x30 bytes)
  ::math::vector_4d color;
  void init(noggit::mpq::file* f);
  void setup();
};

class WMO : public noggit::async::object
{
public:
  virtual void finish_loading();

  bool draw_group_boundingboxes;

  const std::string& filename() const;

  //std::string WMOName;
  std::string _filename;
  WMOGroup *groups;
  unsigned int nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, nX;
  std::vector<WMOMaterial> _materials;
  ::math::vector_3d extents[2];
  std::vector<std::string> textures;
  std::vector<std::string> models;
  std::vector<ModelInstance> modelis;

  std::vector<WMOLight> lights;
  std::vector<WMOPV> pvs;
  std::vector<WMOPR> prs;

  std::vector<WMOFog> fogs;

  std::vector<WMODoodadSet> doodadsets;

  boost::optional<noggit::scoped_model_reference> skybox;

  //! \todo This only has World* for wmo-doodads. ._.
  explicit WMO(const std::string& name, World* world);
  ~WMO();
  void draw (World* world
            , int doodadset
            , const ::math::vector_3d& ofs
            , const float rot
            , const float culldistance
            , bool boundingbox
            , bool groupboxes
            , bool highlight
            , bool draw_doodads
            , bool draw_fog
            , bool hasSkies
            , const float& fog_distance
            , const Frustum& frustum
            , const ::math::vector_3d& camera
            ) const;
  void drawSelect (World* world
                  , int doodadset
                  , const ::math::vector_3d& ofs
                  , const float rot
                  , const float culldistance
                  , bool draw_doodads
                  , const Frustum& frustum
                  , const ::math::vector_3d& camera
                  ) const;
  //void drawPortals();
  bool drawSkybox(World* world, ::math::vector_3d pCamera, ::math::vector_3d pLower, ::math::vector_3d pUpper ) const;
};

namespace noggit
{
  struct wmo_manager : private multimap_with_normalized_key<WMO>
  {
    friend struct scoped_wmo_reference;
  };

  struct scoped_wmo_reference
  {
    scoped_wmo_reference (World*, std::string const& filename);

    scoped_wmo_reference (scoped_wmo_reference const&);
    scoped_wmo_reference (scoped_wmo_reference&&);
    scoped_wmo_reference& operator= (scoped_wmo_reference const&) = delete;
    scoped_wmo_reference& operator= (scoped_wmo_reference&&);

    ~scoped_wmo_reference();

    WMO* operator->() const
    {
      return _wmo;
    }

  private:
    bool _valid;
    std::string _filename;
    World* _world;
    WMO* _wmo;
  };
}

#endif
