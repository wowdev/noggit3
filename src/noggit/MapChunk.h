// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <boost/optional.hpp>

#include <math/vector_4d.h>

#include <opengl/shader.hpp>
#include <opengl/types.h>

#include <noggit/MapHeaders.h>
#include <noggit/Selection.h>
#include <noggit/TextureManager.h>
#include <noggit/texture_set.hpp>
#include <math/ray.hpp>

namespace noggit
{
  class blp_texture;
  namespace mpq
  {
    class file;
  }
}
class brush;
class Frustum;
class MapTile;
class World;
class Skies;

typedef uint16_t StripType;

static const int mapbufsize = 9*9 + 8*8;

class MapChunk
{
public:
  MapChunk(World* world, MapTile* mt, noggit::mpq::file* f,bool bigAlpha);
  ~MapChunk();

  void initStrip();

  bool is_visible ( const float& cull_distance
                  , const Frustum& frustum
                  , const ::math::vector_3d& camera
                  ) const;

  void draw ( opengl::scoped::use_program& mcnk_shader
            , const boost::optional<selection_type>& selected_item
            );
  void drawSelect();

  void intersect (math::ray ray, selection_result& results);

  // todo split into draw_lines and draw_hole_lines
  void drawLines (bool draw_hole_lines) const;

  float getHeight (int x, int z) const;
  float getMinHeight() const;

  void drawTextures() const;

  void update_normal_vectors();

  void getSelectionCoord (const int& selected_polygon, float* x, float* z) const;
  float getSelectionHeight (const int& selected_polygon) const;
  ::math::vector_3d GetSelectionPosition(const int& selected_polygon) const;

  bool changeTerrain(float x, float z, float change, float radius, int BrushType);
  bool flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType);
  bool blurTerrain(float x, float z, float remain, float radius, int BrushType);

  bool paintTexture(float x, float z, const brush& Brush, float strength, float pressure, noggit::scoped_blp_texture_reference texture);
  int addTexture(noggit::scoped_blp_texture_reference texture);
  void switchTexture( noggit::scoped_blp_texture_reference oldTexture, noggit::scoped_blp_texture_reference newTexture );
  void eraseTextures();

  bool isHole(int i,int j);
  void addHole(int i,int j);
  void removeHole(int i,int j);
  void make_all_holes();
  void remove_all_holes();

  void setFlag(bool on_or_off, int flag);

  int getAreaID();
  void setAreaID(int ID);

  boost::optional<float> get_height ( const float& x
                                    , const float& z
                                    ) const;

  void CreateStrips();

  void update_low_quality_texture_map();
  const unsigned char* low_quality_texture_map() const;


  ::math::vector_3d vmin, vmax, vcenter;
  int px, py;

  MapChunkHeader header;

  float xbase, ybase, zbase;

  int nameID, holes;

  bool haswater;

  noggit::texture_set textures;

  unsigned char mShadowMap[8*64];
  GLuint shadow;

  GLuint vertices, normals, indices, minimap, minishadows;

  StripType *strip;
  int striplen;

  ::math::vector_3d mNormals[mapbufsize];
  ::math::vector_3d mVertices[mapbufsize];
  //! \todo Is this needed? Can't we just use the real vertices?
  ::math::vector_3d mMinimap[mapbufsize];
  ::math::vector_4d mFakeShadows[mapbufsize];

private:
  World* _world;

  StripType _line_strip[32];
  StripType _hole_strip[128];
};
