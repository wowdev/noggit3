// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>
#include <math/quaternion.hpp>
#include <math/vector_3d.hpp>
#include <noggit/Brush.h>
#include <noggit/TileWater.hpp>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>
#include <noggit/Misc.h>
#include <noggit/World.h>
#include <noggit/alphamap.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/TexturingGUI.h>
#include <opengl/scoped.hpp>
#include <opengl/matrix.hpp>

#include <algorithm>
#include <iostream>
#include <map>

static const float texDetail = 8.0f;
static const float TEX_RANGE = 1.0f;
MapChunk::MapChunk(MapTile *maintile, MPQFile *f, bool bigAlpha)
  : mt(maintile)
  , use_big_alphamap(bigAlpha)
{
  uint32_t fourcc;
  uint32_t size;

  size_t base = f->getPos();

  hasMCCV = false;

  // - MCNK ----------------------------------------------
  {
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCNK');

    f->read(&header, 0x80);

    Flags = header.flags;
    areaID = header.areaid;

    zbase = header.zpos;
    xbase = header.xpos;
    ybase = header.ypos;

    px = header.ix;
    py = header.iy;

    holes = header.holes;

    // correct the x and z values ^_^
    zbase = zbase*-1.0f + ZEROPOINT;
    xbase = xbase*-1.0f + ZEROPOINT;

    vmin = math::vector_3d(9999999.0f, 9999999.0f, 9999999.0f);
    vmax = math::vector_3d(-9999999.0f, -9999999.0f, -9999999.0f);
  }
  // - MCVT ----------------------------------------------
  {
    f->seek(base + header.ofsHeight);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCVT');

    math::vector_3d *ttv = mVertices;

    // vertices
    for (int j = 0; j < 17; ++j) {
      for (int i = 0; i < ((j % 2) ? 8 : 9); ++i) {
        float h, xpos, zpos;
        f->read(&h, 4);
        xpos = i * UNITSIZE;
        zpos = j * 0.5f * UNITSIZE;
        if (j % 2) {
          xpos += UNITSIZE*0.5f;
        }
        math::vector_3d v = math::vector_3d(xbase + xpos, ybase + h, zbase + zpos);
        *ttv++ = v;
        vmin.y = std::min(vmin.y, v.y);
        vmax.y = std::max(vmax.y, v.y);
      }
    }

    vmin.x = xbase;
    vmin.z = zbase;
    vmax.x = xbase + 8 * UNITSIZE;
    vmax.z = zbase + 8 * UNITSIZE;

    update_intersect_points();

    // use absolute y pos in vertices
    ybase = 0.0f;
    header.ypos = 0.0f;
  }
  // - MCNR ----------------------------------------------
  {
    f->seek(base + header.ofsNormal);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCNR');

    char nor[3];
    math::vector_3d *ttn = mNormals;
    for (int i = 0; i< mapbufsize; ++i) 
    {
      f->read(nor, 3);
      *ttn++ = math::vector_3d(nor[0] / 127.0f, nor[2] / 127.0f, nor[1] / 127.0f);
    }
  }
  // - MCLY ----------------------------------------------
  {
    f->seek(base + header.ofsLayer);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCLY');

    _texture_set.initTextures(f, mt, size);
  }
  // - MCSH ----------------------------------------------
  if(header.ofsShadow && header.sizeShadow)
  {
    f->seek(base + header.ofsShadow);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCSH');

    // shadow map 64 x 64
    f->read(mShadowMap, 0x200);
    f->seekRelative(-0x200);

    unsigned char sbuf[64 * 64], *p, c[8];
    p = sbuf;
    for (int j = 0; j<64; ++j) {
      f->read(c, 8);
      for (int i = 0; i<8; ++i) {
        for (int b = 0x01; b != 0x100; b <<= 1) {
          *p++ = (c[i] & b) ? 85 : 0;
        }
      }
    }
    shadow.bind();
    gl.texImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, sbuf);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  // - MCAL ----------------------------------------------
  {
    f->seek(base + header.ofsAlpha);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCAL');

    _texture_set.initAlphamaps(f, header.nLayers, use_big_alphamap, (header.flags & FLAG_do_not_fix_alpha_map) == 0);
  }
  // - MCCV ----------------------------------------------
  if(header.ofsMCCV)
  {
    f->seek(base + header.ofsMCCV);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCCV');

    if (!(Flags & FLAG_MCCV))
      Flags |= FLAG_MCCV;

    hasMCCV = true;

    unsigned char t[4];
    for (int i = 0; i < mapbufsize; ++i)
    {
      f->read(t, 4);
      mccv[i] = math::vector_3d((float)t[2] / 127.0f, (float)t[1] / 127.0f, (float)t[0] / 127.0f);
    }
  }

  // create vertex buffers
  gl.bufferData<GL_ARRAY_BUFFER> (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER> (normals, sizeof(mNormals), mNormals, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER> (mccvEntry, sizeof(mccv), mccv, GL_STATIC_DRAW);

  initStrip();

  vcenter = (vmin + vmax) * 0.5f;

  math::vector_3d *ttv = mMinimap;

  // vertices
  for (int j = 0; j < 17; ++j) {
    for (int i = 0; i < ((j % 2) ? 8 : 9); ++i) {
      float xpos, zpos;

      xpos = i * 0.125f;
      zpos = j * 0.5f * 0.125f;

      if (j % 2) {
        xpos += 0.125f * 0.5f;
      }

      math::vector_3d v = math::vector_3d(xpos + px, zpos + py, -1);
      *ttv++ = v;
    }
  }

  if ((Flags & 1) == 0)
  {
    /** We have no shadow map (MCSH), so we got no shadows at all!  **
    ** This results in everything being black.. Yay. Lets fake it! **/
    for (size_t i = 0; i < 512; ++i)
      mShadowMap[i] = 0;

    unsigned char sbuf[64 * 64];
    for (size_t j = 0; j < 4096; ++j)
      sbuf[j] = 0;

    shadow.bind();
    gl.texImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, sbuf);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  float ShadowAmount;
  for (int j = 0; j<mapbufsize; ++j)
  {
    ShadowAmount = 1.0f - (mNormals[j].x + mNormals[j].y + mNormals[j].z);
    ShadowAmount = std::min(1.0f, std::max(0.0f, ShadowAmount)) * 0.5f;

    mFakeShadows[j].w = ShadowAmount;
  }

  gl.genBuffers(1, &minimap);
  gl.genBuffers(1, &minishadows);

  gl.bufferData<GL_ARRAY_BUFFER> (minimap, sizeof(mMinimap), mMinimap, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER> (minishadows, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}


void MapChunk::drawTextures (int animtime)
{
  gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);

  if (_texture_set.num() > 0U)
  {
    _texture_set.bindTexture(0, 0);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();
  }
  else
  {
    opengl::texture::set_active_texture (0);
    opengl::texture::disable_texture();

    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();
  }

  _texture_set.startAnim(0, animtime);
  gl.begin(GL_TRIANGLE_STRIP);
  gl.texCoord2f(0.0f, texDetail);
  gl.vertex3f(static_cast<float>(px), py + 1.0f, -2.0f);
  gl.texCoord2f(0.0f, 0.0f);
  gl.vertex3f(static_cast<float>(px), static_cast<float>(py), -2.0f);
  gl.texCoord2f(texDetail, texDetail);
  gl.vertex3f(px + 1.0f, py + 1.0f, -2.0f);
  gl.texCoord2f(texDetail, 0.0f);
  gl.vertex3f(px + 1.0f, static_cast<float>(py), -2.0f);
  gl.end();
  _texture_set.stopAnim(0);

  if (_texture_set.num() > 1U)
  {
    //gl.depthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    gl.depthMask(GL_FALSE);
  }

  for (size_t i = 1; i < _texture_set.num(); ++i)
  {
    _texture_set.bindTexture(i, 0);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    _texture_set.bindAlphamap(i - 1, 1);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _texture_set.startAnim(i, animtime);

    gl.begin(GL_TRIANGLE_STRIP);
    gl.multiTexCoord2f(GL_TEXTURE0, texDetail, 0.0f);
    gl.multiTexCoord2f(GL_TEXTURE1, TEX_RANGE, 0.0f);
    gl.vertex3f(px + 1.0f, static_cast<float>(py), -2.0f);
    gl.multiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
    gl.multiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
    gl.vertex3f(static_cast<float>(px), static_cast<float>(py), -2.0f);
    gl.multiTexCoord2f(GL_TEXTURE0, texDetail, texDetail);
    gl.multiTexCoord2f(GL_TEXTURE1, TEX_RANGE, TEX_RANGE);
    gl.vertex3f(px + 1.0f, py + 1.0f, -2.0f);
    gl.multiTexCoord2f(GL_TEXTURE0, 0.0f, texDetail);
    gl.multiTexCoord2f(GL_TEXTURE1, 0.0f, TEX_RANGE);
    gl.vertex3f(static_cast<float>(px), py + 1.0f, -2.0f);
    gl.end();

    _texture_set.stopAnim(i);
  }

  opengl::texture::set_active_texture (0);
  opengl::texture::disable_texture();

  opengl::texture::set_active_texture (1);
  opengl::texture::disable_texture();

  gl.vertexPointer (minimap, 3, GL_FLOAT, 0, 0);
  gl.colorPointer (minishadows, 4, GL_FLOAT, 0, 0);

  gl.drawElements(GL_TRIANGLES, strip_without_holes.size(), GL_UNSIGNED_SHORT, strip_without_holes.data());

  if (_texture_set.num() > 1U)
  {
    //gl.depthFunc(GL_LEQUAL);
    gl.depthMask(GL_TRUE);
  }
}

int MapChunk::indexLoD(int x, int y)
{
  return (x + 1) * 9 + x * 8 + y;
}

int MapChunk::indexNoLoD(int x, int y)
{
  return x * 8 + x * 9 + y;
}

void MapChunk::update_intersect_points()
{
  _intersect_points.clear();

  _intersect_points.emplace_back (vmin.x, vmin.y, vmin.z);
  _intersect_points.emplace_back (vmin.x, vmin.y, vmax.z);
  _intersect_points.emplace_back (vmin.x, vmax.y, vmin.z);
  _intersect_points.emplace_back (vmin.x, vmax.y, vmax.z);
  _intersect_points.emplace_back (vmax.x, vmin.y, vmin.z);
  _intersect_points.emplace_back (vmax.x, vmin.y, vmax.z);
  _intersect_points.emplace_back (vmax.x, vmax.y, vmin.z);
  _intersect_points.emplace_back (vmax.x, vmax.y, vmax.z);
}

void MapChunk::initStrip()
{
  strip_with_holes.clear();
  strip_without_holes.clear();

  for (int x = 0; x<8; ++x)
  {
    for (int y = 0; y<8; ++y)
    {
      strip_without_holes.emplace_back (indexLoD(y, x)); //9
      strip_without_holes.emplace_back (indexNoLoD(y, x)); //0
      strip_without_holes.emplace_back (indexNoLoD(y + 1, x)); //17
      strip_without_holes.emplace_back (indexLoD(y, x)); //9
      strip_without_holes.emplace_back (indexNoLoD(y + 1, x)); //17
      strip_without_holes.emplace_back (indexNoLoD(y + 1, x + 1)); //18
      strip_without_holes.emplace_back (indexLoD(y, x)); //9
      strip_without_holes.emplace_back (indexNoLoD(y + 1, x + 1)); //18
      strip_without_holes.emplace_back (indexNoLoD(y, x + 1)); //1
      strip_without_holes.emplace_back (indexLoD(y, x)); //9
      strip_without_holes.emplace_back (indexNoLoD(y, x + 1)); //1
      strip_without_holes.emplace_back (indexNoLoD(y, x)); //0

      if (isHole(x / 2, y / 2))
        continue;

      strip_with_holes.emplace_back (indexLoD(y, x)); //9
      strip_with_holes.emplace_back (indexNoLoD(y, x)); //0
      strip_with_holes.emplace_back (indexNoLoD(y + 1, x)); //17
      strip_with_holes.emplace_back (indexLoD(y, x)); //9
      strip_with_holes.emplace_back (indexNoLoD(y + 1, x)); //17
      strip_with_holes.emplace_back (indexNoLoD(y + 1, x + 1)); //18
      strip_with_holes.emplace_back (indexLoD(y, x)); //9
      strip_with_holes.emplace_back (indexNoLoD(y + 1, x + 1)); //18
      strip_with_holes.emplace_back (indexNoLoD(y, x + 1)); //1
      strip_with_holes.emplace_back (indexLoD(y, x)); //9
      strip_with_holes.emplace_back (indexNoLoD(y, x + 1)); //1
      strip_with_holes.emplace_back (indexNoLoD(y, x)); //0
    }
  }

  opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (indices);
  gl.bufferData (GL_ELEMENT_ARRAY_BUFFER, strip_with_holes.size() * sizeof (StripType), strip_with_holes.data(), GL_STATIC_DRAW);
}

bool MapChunk::GetVertex(float x, float z, math::vector_3d *V)
{
  float xdiff, zdiff;

  xdiff = x - xbase;
  zdiff = z - zbase;

  const int row = static_cast<int>(zdiff / (UNITSIZE * 0.5f) + 0.5f);
  const int column = static_cast<int>((xdiff - UNITSIZE * 0.5f * (row % 2)) / UNITSIZE + 0.5f);
  if ((row < 0) || (column < 0) || (row > 16) || (column >((row % 2) ? 8 : 9)))
    return false;

  *V = mVertices[17 * (row / 2) + ((row % 2) ? 9 : 0) + column];
  return true;
}

float MapChunk::getHeight(int x, int z)
{
  if (x > 9 || z > 9 || x < 0 || z < 0) return 0.0f;
  return mVertices[indexNoLoD(x, z)].y;
}

float MapChunk::getMinHeight()
{
  float min (mVertices[0].y);

  for (auto&& vertex : mVertices)
  {
    min = std::min (min, vertex.y);
  }

  return min;
}

void MapChunk::clearHeight()
{
  for (int i = 0; i < mapbufsize; ++i)
  {
    mVertices[i].y = 0.0f;
  }

  vmin.y = 0.0f;
  vmax.y = 0.0f;

  update_intersect_points();

  gl.bufferData<GL_ARRAY_BUFFER>
    (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);

}

bool MapChunk::is_visible ( const float& cull_distance
                          , const math::frustum& frustum
                          , const math::vector_3d& camera
                          ) const
{
  static const float chunk_radius = std::sqrt (CHUNKSIZE * CHUNKSIZE / 2.0f); //was (vmax - vmin).length() * 0.5f;

  return frustum.intersects (_intersect_points)
      && (((camera - vcenter).length() - chunk_radius) < cull_distance);
}


void MapChunk::draw ( math::frustum const& frustum
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
                    )
{
  if (!is_visible (cull_distance, frustum, camera))
    return;

  bool cantPaint = noggit::ui::selected_texture::get()
                 && !canPaintTexture(*noggit::ui::selected_texture::get())
                 && show_unpaintable_chunks
                 && draw_paintability_overlay;
  
  opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const index_buffer(indices);

  if (_texture_set.num())
  {
    _texture_set.bindTexture(0, 0); // tex0
    _texture_set.bind_alpha(1);

    switch (_texture_set.num())
    {
    case 4: _texture_set.bindTexture(3, 4); // tex3
    case 3: _texture_set.bindTexture(2, 3); // tex2
    case 2: _texture_set.bindTexture(1, 2); // tex1
    }
  }

  opengl::texture::set_active_texture(5);
  shadow.bind();
  
  mcnk_shader.uniform("layer_count", (int)_texture_set.num());
  mcnk_shader.uniform("cant_paint", (int)cantPaint);
  
  bool impassible = draw_chunk_flag_overlay && (Flags & FLAG_IMPASS);
  mcnk_shader.uniform ("draw_impassible_flag", (int)impassible);

  if (draw_areaid_overlay)
  {
    mcnk_shader.uniform("areaid_color", (math::vector_4d)area_id_colors[areaID]);
  }

  mcnk_shader.attrib("position", mVertices);
  mcnk_shader.attrib("normal", mNormals);
  mcnk_shader.attrib("mccv", mccv);

  gl.drawElements(GL_TRIANGLES, strip_with_holes.size(), GL_UNSIGNED_SHORT, nullptr);
}

void MapChunk::intersect (math::ray const& ray, selection_result* results)
{
  if (!ray.intersect_bounds (vmin, vmax))
  {
    return;
  }

  for (int i (0); i < strip_without_holes.size(); i += 3)
  {
    if ( auto distance = ray.intersect_triangle ( mVertices[strip_without_holes[i + 0]]
                                                , mVertices[strip_without_holes[i + 1]]
                                                , mVertices[strip_without_holes[i + 2]]
                                                )
       )
    {
      results->emplace_back
        (*distance, selected_chunk_type (this, i, ray.position (*distance)));
    }
  }
}

void MapChunk::updateVerticesData()
{
  vmin.y = std::numeric_limits<float>::max();
  vmax.y = std::numeric_limits<float>::lowest();

  for (int i(0); i < mapbufsize; ++i)
  {
    vmin.y = std::min(vmin.y, mVertices[i].y);
    vmax.y = std::max(vmax.y, mVertices[i].y);
  }

  update_intersect_points();

  gl.bufferData<GL_ARRAY_BUFFER>(vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
}

void MapChunk::recalcNorms (std::function<boost::optional<float> (float, float)> height)
{
  auto point
  (
    [&] (math::vector_3d& v, float xdiff, float zdiff)
    {
      return math::vector_3d
             ( v.x + xdiff
             , height (v.x + xdiff, v.z + zdiff).get_value_or (v.y)
             , v.z + zdiff
             );
    }
  );

  float const half_unit = UNITSIZE / 2.f;

  for (int i = 0; i<mapbufsize; ++i)
  {
    math::vector_3d const P1 (point(mVertices[i], -half_unit, -half_unit));
    math::vector_3d const P2 (point(mVertices[i],  half_unit, -half_unit));
    math::vector_3d const P3 (point(mVertices[i],  half_unit,  half_unit));
    math::vector_3d const P4 (point(mVertices[i], -half_unit,  half_unit));

    math::vector_3d const N1 ((P2 - mVertices[i]) % (P1 - mVertices[i]));
    math::vector_3d const N2 ((P3 - mVertices[i]) % (P2 - mVertices[i]));
    math::vector_3d const N3 ((P4 - mVertices[i]) % (P3 - mVertices[i]));
    math::vector_3d const N4 ((P1 - mVertices[i]) % (P4 - mVertices[i]));

    math::vector_3d Norm (N1 + N2 + N3 + N4);
    Norm.normalize();

    Norm.x = std::floor(Norm.x * 127) / 127;
    Norm.y = std::floor(Norm.y * 127) / 127;
    Norm.z = std::floor(Norm.z * 127) / 127;

    //! \todo: find out why recalculating normals without changing the terrain result in slightly different normals
    mNormals[i] = {-Norm.z, Norm.y, -Norm.x};
  }
  gl.bufferData<GL_ARRAY_BUFFER> (normals, sizeof(mNormals), mNormals, GL_STATIC_DRAW);

  float ShadowAmount;
  for (int j = 0; j<mapbufsize; ++j)
  {
    ShadowAmount = 1.0f - (-mNormals[j].x + mNormals[j].y - mNormals[j].z);
    ShadowAmount = std::min(1.0f, std::max(0.0f, ShadowAmount)) * 0.5f;

    mFakeShadows[j].w = ShadowAmount;
  }

  gl.bufferData<GL_ARRAY_BUFFER> (minishadows, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}

bool MapChunk::changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius)
{
  float dist, xdiff, zdiff;
  bool changed = false;

  for (int i = 0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x - pos.x;
    zdiff = mVertices[i].z - pos.z;
    if (BrushType == eTerrainType_Quadra)
    {
      if ((std::abs(xdiff) < std::abs(radius / 2)) && (std::abs(zdiff) < std::abs(radius / 2)))
      {
        dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);
        mVertices[i].y += change * (1.0f - dist * inner_radius / radius);
        changed = true;
      }
    }
    else
    {
      dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);
      if (dist < radius)
      {
        changed = true;

        switch (BrushType)
        {
          case eTerrainType_Flat:
            mVertices[i].y += change;
            break;
          case eTerrainType_Linear:
            mVertices[i].y += change * (1.0f - dist * (1.0f - inner_radius) / radius);
            break;
          case eTerrainType_Smooth:
            mVertices[i].y += change / (1.0f + dist / radius);
            break;
          case eTerrainType_Polynom:
            mVertices[i].y += change*((dist / radius)*(dist / radius) + dist / radius + 1.0f);
            break;
          case eTerrainType_Trigo:
            mVertices[i].y += change*cos(dist / radius);
            break;
          case eTerrainType_Gaussian:
            mVertices[i].y += dist < radius * inner_radius ? change * std::exp(-(std::pow(radius * inner_radius / radius, 2) / (2 * std::pow(0.39f, 2)))) : change * std::exp(-(std::pow(dist / radius, 2) / (2 * std::pow(0.39f, 2))));

            break;
          default:
            LogError << "Invalid terrain edit type (" << BrushType << ")" << std::endl;
            changed = false;
            break;
        }
      }
    }
  }
  if (changed)
  {
    updateVerticesData();
  }
  return changed;
}

bool MapChunk::ChangeMCCV(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode)
{
  float dist;
  bool changed = false;

  if (!hasMCCV)
  {
    for (int i = 0; i < mapbufsize; ++i)
    {
      mccv[i].x = 1.0f; // set default shaders
      mccv[i].y = 1.0f;
      mccv[i].z = 1.0f;
    }

    changed = true;
    Flags |= FLAG_MCCV;
    hasMCCV = true;
  }

  for (int i = 0; i < mapbufsize; ++i)
  {
    dist = misc::dist(mVertices[i], pos);
    if (dist <= radius)
    {
      float edit = change * (1.0f - dist / radius);
      if (editMode)
      {
        mccv[i].x += (color.x - mccv[i].x)* edit;
        mccv[i].y += (color.y - mccv[i].y)* edit;
        mccv[i].z += (color.z - mccv[i].z)* edit;
      }
      else
      {
        mccv[i].x += (1.0f - mccv[i].x) * edit;
        mccv[i].y += (1.0f - mccv[i].y) * edit;
        mccv[i].z += (1.0f - mccv[i].z) * edit;
      }

      mccv[i].x = std::min(std::max(mccv[i].x, 0.0f), 2.0f);
      mccv[i].y = std::min(std::max(mccv[i].y, 0.0f), 2.0f);
      mccv[i].z = std::min(std::max(mccv[i].z, 0.0f), 2.0f);

      changed = true;
    }
  }
  if (changed)
  {
    gl.bufferData<GL_ARRAY_BUFFER> (mccvEntry, sizeof(mccv), mccv, GL_STATIC_DRAW);
  }
  return changed;
}

bool MapChunk::flattenTerrain ( math::vector_3d const& pos
                              , float remain
                              , float radius
                              , int BrushType
                              , int flattenType
                              , math::vector_3d const& origin
                              , math::degrees angle
                              , math::degrees orientation
                              )
{
  bool changed (false);

  for (int i(0); i < mapbufsize; ++i)
  {
	  float const dist(misc::dist(mVertices[i], pos));

	  if (dist >= radius)
	  {
		  continue;
	  }

	  float const ah(origin.y
		  + ((mVertices[i].x - origin.x) * math::cos(orientation)
			  + (mVertices[i].z - origin.z) * math::sin(orientation)
			  ) * math::tan(angle)
	  );

	  if ((flattenType == eFlattenMode_Raise && ah < mVertices[i].y)
		  || (flattenType == eFlattenMode_Lower && ah > mVertices[i].y)
		  )
	  {
		  continue;
	  }

	  if (BrushType == eFlattenType_Origin)
	  {
		  mVertices[i].y = origin.y;
		  changed = true;
		  continue;
	  }

    mVertices[i].y = math::interpolation::linear
      ( BrushType == eFlattenType_Flat ? remain
      : BrushType == eFlattenType_Linear ? remain * (1.f - dist / radius)
      : BrushType == eFlattenType_Smooth ? pow (remain, 1.f + dist / radius)
      : throw std::logic_error ("bad brush type")
      , mVertices[i].y
      , ah
      );

    changed = true;
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}

bool MapChunk::blurTerrain ( math::vector_3d const& pos
                           , float remain
                           , float radius
                           , int BrushType
                           , std::function<boost::optional<float> (float, float)> height
                           )
{
  bool changed (false);

  for (int i (0); i < mapbufsize; ++i)
  {
    float const dist(misc::dist(mVertices[i], pos));

    if (dist >= radius)
    {
      continue;
    }

    int Rad = (int)(radius / UNITSIZE);
    float TotalHeight = 0;
    float TotalWeight = 0;
    for (int j = -Rad * 2; j <= Rad * 2; ++j)
    {
      float tz = pos.z + j * UNITSIZE / 2;
      for (int k = -Rad; k <= Rad; ++k)
      {
        float tx = pos.x + k*UNITSIZE + (j % 2) * UNITSIZE / 2.0f;
        float dist2 = misc::dist (tx, tz, mVertices[i].x, mVertices[i].z);
        if (dist2 > radius)
          continue;
        auto h (height (tx, tz));
        if (h)
        {
          TotalHeight += (1.0f - dist2 / radius) * h.get();
          TotalWeight += (1.0f - dist2 / radius);
        }
      }
    }

	if (BrushType == eFlattenType_Origin)
	{
		continue;
	}

    mVertices[i].y = math::interpolation::linear
      ( BrushType == eFlattenType_Flat ? remain
      : BrushType == eFlattenType_Linear ? remain * (1.f - dist / radius)
      : BrushType == eFlattenType_Smooth ? pow (remain, 1.f + dist / radius)
      : throw std::logic_error ("bad brush type")
      , mVertices[i].y
      , TotalHeight / TotalWeight
      );

    changed = true;
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}


void MapChunk::eraseTextures()
{
  _texture_set.eraseTextures();
}

void MapChunk::change_texture_flag(scoped_blp_texture_reference tex, std::size_t flag, bool add)
{
  _texture_set.change_texture_flag(tex, flag, add);
}

int MapChunk::addTexture(scoped_blp_texture_reference texture)
{
  return _texture_set.addTexture(texture);
}

void MapChunk::switchTexture(scoped_blp_texture_reference oldTexture, scoped_blp_texture_reference newTexture)
{
  _texture_set.switchTexture(oldTexture, newTexture);
}

bool MapChunk::paintTexture(math::vector_3d const& pos, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  return _texture_set.paintTexture(xbase, zbase, pos.x, pos.z, brush, strength, pressure, texture);
}

bool MapChunk::canPaintTexture(scoped_blp_texture_reference texture)
{
  return _texture_set.canPaintTexture(texture);
}

bool MapChunk::isHole(int i, int j)
{
  return (holes & ((1 << ((j * 4) + i)))) != 0;
}

void MapChunk::setHole(math::vector_3d const& pos, bool big, bool add)
{
  if (big)
  {
    holes = add ? 0xFFFFFFFF : 0x0;
  }
  else
  {
    int v = 1 << ((int)((pos.z - zbase) / MINICHUNKSIZE) * 4 + (int)((pos.x - xbase) / MINICHUNKSIZE));
    holes = add ? (holes | v) : (holes & ~v);
  }

  initStrip();
}

void MapChunk::setAreaID(int ID)
{
  areaID = ID;
}

int MapChunk::getAreaID()
{
  return areaID;
}


void MapChunk::setFlag(bool changeto, uint32_t flag)
{
  if (changeto)
    this->Flags = this->Flags | (flag);
  else
    this->Flags = this->Flags & ~(flag);
}

int MapChunk::getFlag()
{
  return this->Flags;
}

void MapChunk::save(sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::vector<WMOInstance> &lObjectInstances, std::vector<ModelInstance>& lModelInstances)
{
  int lID;
  int lMCNK_Size = 0x80;
  int lMCNK_Position = lCurrentPosition;
  lADTFile.Extend(8 + 0x80);  // This is only the size of the header. More chunks will increase the size.
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCNK', lMCNK_Size);
  lADTFile.GetPointer<MCIN>(lMCIN_Position + 8)->mEntries[py * 16 + px].offset = lCurrentPosition; // check this

                                                                                                   // MCNK data
  lADTFile.Insert(lCurrentPosition + 8, 0x80, reinterpret_cast<char*>(&(header)));
  MapChunkHeader *lMCNK_header = lADTFile.GetPointer<MapChunkHeader>(lCurrentPosition + 8);

  lMCNK_header->flags = Flags | FLAG_do_not_fix_alpha_map;
  lMCNK_header->holes = holes;
  lMCNK_header->areaid = areaID;

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
  lMCNK_header->ofsMCCV = -1;

  //! \todo  Implement sound emitter support. Or not.
  lMCNK_header->ofsSndEmitters = 0;
  lMCNK_header->nSndEmitters = 0;

  lMCNK_header->ofsLiquid = 0;
  //! \todo Is this still 8 if no chunk is present? Or did they correct that?
  lMCNK_header->sizeLiquid = 8;

  lMCNK_header->ypos = mVertices[0].y;

  memset(lMCNK_header->low_quality_texture_map, 0, 0x10);

  static const size_t minimum_value_to_overwrite(128);

  for (size_t y(0); y < 8; ++y)
  {
    for (size_t x(0); x < 8; ++x)
    {
      size_t winning_layer(0);
      std::map<std::size_t, float> alpha_visibility;

      for (size_t j(0); j < 8; ++j)
      {
        for (size_t i(0); i < 8; ++i)
        {
          float alphas[4] = { 255.f, 0.f, 0.f, 0.f };

          for (size_t layer(1); layer < _texture_set.num(); ++layer)
          {            
            float a = static_cast<float>(_texture_set.getAlpha(layer - 1, (y * 8 + j) * 64 + (x * 8 + i)));
            alphas[layer] = a;
            a /= 255.f;

            for (size_t n = 0; n < layer; n++)
            {
              alphas[n] = (alphas[n] * (1.f - a));
            }              
          }

          for (int k = 0; k < 4; ++k)
          {
            alpha_visibility[k] += alphas[k];
          }
        }
      }

      for (std::size_t i = 1; i < 4; ++i)
      {
        if (alpha_visibility[i] > alpha_visibility[winning_layer])
        {
          winning_layer = i;
        }
      }

      const size_t array_index((y * 8 + x) / 4);
      const size_t bit_index(((y * 8 + x) % 4) * 2);

      lMCNK_header->low_quality_texture_map[array_index] |= ((winning_layer & 3) << bit_index);
    }
  }

  lCurrentPosition += 8 + 0x80;

  // MCVT
  int lMCVT_Size = mapbufsize * 4;

  lADTFile.Extend(8 + lMCVT_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCVT', lMCVT_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsHeight = lCurrentPosition - lMCNK_Position;

  float* lHeightmap = lADTFile.GetPointer<float>(lCurrentPosition + 8);

  for (int i = 0; i < mapbufsize; ++i)
    lHeightmap[i] = mVertices[i].y - mVertices[0].y;

  lCurrentPosition += 8 + lMCVT_Size;
  lMCNK_Size += 8 + lMCVT_Size;

  // MCCV
  int lMCCV_Size = 0;
  if (hasMCCV)
  {
    lMCCV_Size = mapbufsize * sizeof(unsigned int);
    lADTFile.Extend(8 + lMCCV_Size);
    SetChunkHeader(lADTFile, lCurrentPosition, 'MCCV', lMCCV_Size);
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsMCCV = lCurrentPosition - lMCNK_Position;

    unsigned int *lmccv = lADTFile.GetPointer<unsigned int>(lCurrentPosition + 8);

    for (int i = 0; i < mapbufsize; ++i)
    {
      *lmccv++ = ((unsigned char)(mccv[i].z * 127.0f) & 0xFF)
        + (((unsigned char)(mccv[i].y * 127.0f) & 0xFF) << 8)
        + (((unsigned char)(mccv[i].x * 127.0f) & 0xFF) << 16);
    }

    lCurrentPosition += 8 + lMCCV_Size;
    lMCNK_Size += 8 + lMCCV_Size;
  }
  else
  {
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsMCCV = 0;
  }

  // MCNR
  int lMCNR_Size = mapbufsize * 3;

  lADTFile.Extend(8 + lMCNR_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCNR', lMCNR_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsNormal = lCurrentPosition - lMCNK_Position;

  char * lNormals = lADTFile.GetPointer<char>(lCurrentPosition + 8);

  for (int i = 0; i < mapbufsize; ++i)
  {
    lNormals[i * 3 + 0] = static_cast<char>(mNormals[i].x * 127);
    lNormals[i * 3 + 1] = static_cast<char>(mNormals[i].z * 127);
    lNormals[i * 3 + 2] = static_cast<char>(mNormals[i].y * 127);
  }

  lCurrentPosition += 8 + lMCNR_Size;
  lMCNK_Size += 8 + lMCNR_Size;
  //        }

  // Unknown MCNR bytes
  // These are not in as we have data or something but just to make the files more blizzlike.
  //        {
  lADTFile.Extend(13);
  lCurrentPosition += 13;
  lMCNK_Size += 13;
  //        }

  // MCLY
  //        {
  size_t lMCLY_Size = _texture_set.num() * 0x10;

  lADTFile.Extend(8 + lMCLY_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCLY', lMCLY_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsLayer = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nLayers = _texture_set.num();

  std::vector<std::vector<uint8_t>> alphamaps = _texture_set.save_alpha(use_big_alphamap);
  int lMCAL_Size = 0; 

  // MCLY data
  for (size_t j = 0; j < _texture_set.num(); ++j)
  {
    ENTRY_MCLY * lLayer = lADTFile.GetPointer<ENTRY_MCLY>(lCurrentPosition + 8 + 0x10 * j);

    lLayer->textureID = lTextures.find(_texture_set.filename(j))->second;
    lLayer->flags = _texture_set.flag(j);
    lLayer->ofsAlpha = lMCAL_Size;
    lLayer->effectID = _texture_set.effect(j);

    if (j == 0)
    {
      lLayer->flags &= ~(FLAG_USE_ALPHA | FLAG_ALPHA_COMPRESSED);  
    }
    else
    {
      lLayer->flags |= FLAG_USE_ALPHA;
      // always compress big alpha
      if (use_big_alphamap)
      {
        lLayer->flags |= FLAG_ALPHA_COMPRESSED;
      }

      lMCAL_Size += alphamaps[j - 1].size();
    }
  }

  lCurrentPosition += 8 + lMCLY_Size;
  lMCNK_Size += 8 + lMCLY_Size;
  //        }

  // MCRF
  //        {
  std::list<int> lDoodadIDs;
  std::list<int> lObjectIDs;

  math::vector_3d lChunkExtents[2];
  lChunkExtents[0] = math::vector_3d(xbase, 0.0f, zbase);
  lChunkExtents[1] = math::vector_3d(xbase + CHUNKSIZE, 0.0f, zbase + CHUNKSIZE);

  // search all wmos that are inside this chunk
  lID = 0;
  for(auto const& wmo : lObjectInstances)
  {
    if (wmo.isInsideRect(lChunkExtents))
    {
      lObjectIDs.push_back(lID);
    }      

    lID++;
  }

  // search all models that are inside this chunk
  lID = 0;
  for(auto const& model : lModelInstances)
  {
    if (model.isInsideRect (lChunkExtents))
    {
      lDoodadIDs.push_back(lID);
    }
    lID++;
  }

  int lMCRF_Size = 4 * (lDoodadIDs.size() + lObjectIDs.size());
  lADTFile.Extend(8 + lMCRF_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCRF', lMCRF_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsRefs = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nDoodadRefs = lDoodadIDs.size();
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nMapObjRefs = lObjectIDs.size();

  // MCRF data
  int *lReferences = lADTFile.GetPointer<int>(lCurrentPosition + 8);

  lID = 0;
  for (std::list<int>::iterator it = lDoodadIDs.begin(); it != lDoodadIDs.end(); ++it)
  {
    lReferences[lID] = *it;
    lID++;
  }

  for (std::list<int>::iterator it = lObjectIDs.begin(); it != lObjectIDs.end(); ++it)
  {
    lReferences[lID] = *it;
    lID++;
  }

  lCurrentPosition += 8 + lMCRF_Size;
  lMCNK_Size += 8 + lMCRF_Size;
  //        }

  // MCSH
  //        {
  //! \todo  Somehow determine if we need to write this or not?
  //! \todo  This sometime gets all shadows black.
  if (Flags & 1)
  {
    int lMCSH_Size = 0x200;
    lADTFile.Extend(8 + lMCSH_Size);
    SetChunkHeader(lADTFile, lCurrentPosition, 'MCSH', lMCSH_Size);

    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsShadow = lCurrentPosition - lMCNK_Position;
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->sizeShadow = 0x200;

    char * lLayer = lADTFile.GetPointer<char>(lCurrentPosition + 8);

    memcpy(lLayer, mShadowMap, 0x200);

    lCurrentPosition += 8 + lMCSH_Size;
    lMCNK_Size += 8 + lMCSH_Size;
  }
  else
  {
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsShadow = 0;
    lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->sizeShadow = 0;
  }

  // MCAL
  size_t lMaps = _texture_set.num() ? _texture_set.num() - 1U : 0U;

  lADTFile.Extend(8 + lMCAL_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCAL', lMCAL_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsAlpha = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->sizeAlpha = 8 + lMCAL_Size;

  char * lAlphaMaps = lADTFile.GetPointer<char>(lCurrentPosition + 8);

  for (auto alpha : alphamaps)
  {
    memcpy(lAlphaMaps, alpha.data(), alpha.size());
    lAlphaMaps += alpha.size();
  }

  lCurrentPosition += 8 + lMCAL_Size;
  lMCNK_Size += 8 + lMCAL_Size;
  //        }

  //! Don't write anything MCLQ related anymore...

  // MCSE
  int lMCSE_Size = 0;
  lADTFile.Extend(8 + lMCSE_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCSE', lMCSE_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsSndEmitters = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nSndEmitters = lMCSE_Size / 0x1C;

  lCurrentPosition += 8 + lMCSE_Size;
  lMCNK_Size += 8 + lMCSE_Size;

  lADTFile.GetPointer<sChunkHeader>(lMCNK_Position)->mSize = lMCNK_Size;
  lADTFile.GetPointer<MCIN>(lMCIN_Position + 8)->mEntries[py * 16 + px].size = lMCNK_Size + sizeof (sChunkHeader);
}


bool MapChunk::fixGapLeft(const MapChunk* chunk)
{
  if (!chunk)
    return false;

  bool changed = false;

  for (size_t i = 0; i <= 136; i+= 17)
  {
    float h = chunk->mVertices[i + 8].y;
    if (mVertices[i].y != h)
    {
      mVertices[i].y = h;
      changed = true;
    }
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}

bool MapChunk::fixGapAbove(const MapChunk* chunk)
{
  if (!chunk)
    return false;

  bool changed = false;

  for (size_t i = 0; i < 9; i++)
  {
    float h = chunk->mVertices[i + 136].y;
    if (mVertices[i].y != h)
    {
      mVertices[i].y = h;
      changed = true;
    }
  }

  if (changed)
  {
    updateVerticesData();
  }

  return changed;
}


void MapChunk::selectVertex(math::vector_3d const& pos, float radius, std::set<math::vector_3d*>& vertices)
{
  if (misc::getShortestDist(pos.x, pos.z, xbase, zbase, CHUNKSIZE) > radius)
  {
    return;
  }

  for (int i = 0; i < mapbufsize; ++i)
  {
    if (misc::dist(pos.x, pos.z, mVertices[i].x, mVertices[i].z) <= radius)
    {
      vertices.emplace(&mVertices[i]);
    }
  }
}

void MapChunk::fixVertices(std::set<math::vector_3d*>& selected)
{
  std::vector<int> ids ={ 0, 1, 17, 18 };
  // iterate through each "square" of vertices
  for (int i = 0; i < 64; ++i)
  {
    int not_selected = 0, count = 0, mid_vertex = ids[0] + 9;
    float h = 0.0f;

    for (int& index : ids)
    {
      if (selected.find(&mVertices[index]) == selected.end())
      {
        not_selected = index;
      }
      else
      {
        count++;
      }
      h += mVertices[index].y;
      index += (((i+1) % 8) == 0) ? 10 : 1;
    }

    if (count == 2)
    {
      mVertices[mid_vertex].y = h * 0.25f;
    }
    else if (count == 3)
    {
      mVertices[mid_vertex].y = (h - mVertices[not_selected].y) / 3.0f;
    }
  }
}

bool MapChunk::isBorderChunk(std::set<math::vector_3d*>& selected)
{
  for (int i = 0; i < mapbufsize; ++i)
  {
    // border chunk if at least a vertex isn't selected
    if (selected.find(&mVertices[i]) == selected.end())
    {
      return true;
    }
  }

  return false;
}

ChunkWater* MapChunk::liquid_chunk() const
{
  return mt->Water.getChunk(px, py);
}
