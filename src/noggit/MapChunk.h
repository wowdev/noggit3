// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/quaternion.hpp> // math::vector_4d
#include <noggit/MapTile.h> // MapTile
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/TextureManager.h>
#include <noggit/WMOInstance.h>
#include <noggit/texture_set.hpp>
#include <opengl/scoped.hpp>
#include <opengl/texture.hpp>
#include <noggit/Misc.h>

#include <map>

class MPQFile;
namespace math
{
  class frustum;
  struct vector_4d;
}
class Brush;
class Alphamap;
class ChunkWater;
class sExtendableArray;

using StripType = uint16_t;
static const int mapbufsize = 9 * 9 + 8 * 8; // chunk size

class MapChunk
{
private:
  bool hasMCCV;

  int holes;

  unsigned int areaID;

  unsigned char mShadowMap[8 * 64];
  opengl::texture shadow;

  std::vector<StripType> strip_with_holes;
  std::vector<StripType> strip_without_holes;
  StripType LineStrip[32];
  StripType HoleStrip[128];

  math::vector_3d mNormals[mapbufsize];
  math::vector_3d mMinimap[mapbufsize];
  math::vector_4d mFakeShadows[mapbufsize];
  math::vector_3d mccv[mapbufsize];

  void initStrip();

  int indexNoLoD(int x, int y);
  int indexLoD(int x, int y);

public:
  MapChunk(MapTile* mt, MPQFile* f, bool bigAlpha);

  MapTile *mt;
  math::vector_3d vmin, vmax, vcenter;
  int px, py;

  MapChunkHeader header;

  float xbase, ybase, zbase;

  unsigned int Flags;
  bool use_big_alphamap;

  TextureSet _texture_set;

  opengl::scoped::buffers<4> _buffers;
  GLuint const& vertices = _buffers[0];
  GLuint const& normals = _buffers[1];
  GLuint const& indices = _buffers[2];
  GLuint const& mccvEntry = _buffers[3];

  GLuint minimap, minishadows;

  math::vector_3d mVertices[mapbufsize];

  bool is_visible ( const float& cull_distance
                  , const math::frustum& frustum
                  , const math::vector_3d& camera
                  ) const;

  void draw ( math::frustum const& frustum
            , opengl::scoped::use_program& mcnk_shader
            , const float& cull_distance
            , const math::vector_3d& camera
            , bool show_unpaintable_chunks
            , bool draw_contour
            , bool draw_paintability_overlay
            , bool draw_chunk_flag_overlay
            , bool draw_areaid_overlay
            , bool draw_wireframe_overlay
            , int cursor_type
            , std::map<int, misc::random_color>& area_id_colors
            , boost::optional<selection_type> selection
            , int animtime
            );
  //! \todo only this function should be public, all others should be called from it

  void drawContour();
  void intersect (math::ray const&, selection_result*);
  void drawLines ( opengl::scoped::use_program&
                 , math::frustum const& frustum
                 , const float& cull_distance
                 , const math::vector_3d& camera
                 , bool draw_hole_lines
                 );
  void drawTextures (int animtime);
  bool ChangeMCCV(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode);

  ChunkWater* liquid_chunk() const;

  void updateVerticesData();
  void recalcNorms (std::function<boost::optional<float> (float, float)> height);

  //! \todo implement Action stack for these
  bool changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius);
  bool flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, int flattenType, const math::vector_3d& origin, math::degrees angle, math::degrees orientation);
  bool blurTerrain ( math::vector_3d const& pos, float remain, float radius, int BrushType
                   , std::function<boost::optional<float> (float, float)> height
                   );

  void selectVertex(math::vector_3d const& pos, float radius, std::set<math::vector_3d*>& vertices);
  void fixVertices(std::set<math::vector_3d*>& selected);
  // for the vertex tool
  bool isBorderChunk(std::set<math::vector_3d*>& selected);

  //! \todo implement Action stack for these
  bool paintTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, scoped_blp_texture_reference texture);
  bool canPaintTexture(scoped_blp_texture_reference texture);
  int addTexture(scoped_blp_texture_reference texture);
  void switchTexture(scoped_blp_texture_reference oldTexture, scoped_blp_texture_reference newTexture);
  void eraseTextures();
  void change_texture_flag(scoped_blp_texture_reference tex, std::size_t flag, bool add);

  //! \todo implement Action stack for these
  bool isHole(int i, int j);
  void setHole(math::vector_3d const& pos, bool big, bool add);

  void setFlag(bool value, uint32_t);
  int getFlag();

  int getAreaID();
  void setAreaID(int ID);

  bool GetVertex(float x, float z, math::vector_3d *V);
  float getHeight(int x, int z);
  float getMinHeight();

  void clearHeight();

  //! \todo this is ugly create a build struct or sth
  void save(sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::vector<WMOInstance> &lObjectInstances, std::vector<ModelInstance>& lModelInstances);

  // fix the gaps with the chunk to the left
  bool fixGapLeft(const MapChunk* chunk);
  // fix the gaps with the chunk above
  bool fixGapAbove(const MapChunk* chunk);
};
