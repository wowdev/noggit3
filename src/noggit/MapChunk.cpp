// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/quaternion.hpp>
#include <math/vector_3d.hpp>
#include <noggit/Brush.h>
#include <noggit/Environment.h>
#include <noggit/Frustum.h>
#include <noggit/Liquid.h>
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

#include <algorithm>
#include <iostream>
#include <map>

extern int terrainMode;

bool DrawMapContour = false;

GLuint Contour = 0;
float CoordGen[4];
static const int CONTOUR_WIDTH = 128;

static const float texDetail = 8.0f;

static const float TEX_RANGE = 1.0f;

StripType LineStrip[32];
StripType HoleStrip[128];


void GenerateContourMap()
{
  unsigned char CTexture[CONTOUR_WIDTH * 4];

  CoordGen[0] = 0.0f;
  CoordGen[1] = 0.25f;
  CoordGen[2] = 0.0f;
  CoordGen[3] = 0.0f;

  for (int i = 0; i<(CONTOUR_WIDTH * 4); ++i)
    CTexture[i] = 0;
  CTexture[3 + CONTOUR_WIDTH / 2] = 0xff;
  CTexture[7 + CONTOUR_WIDTH / 2] = 0xff;
  CTexture[11 + CONTOUR_WIDTH / 2] = 0xff;

  opengl::scoped::bool_setter<GL_TEXTURE_2D, GL_TRUE> const texture_2d;
  gl.genTextures(1, &Contour);
  gl.bindTexture(GL_TEXTURE_2D, Contour);

  gl.texImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, CONTOUR_WIDTH, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, CTexture);
  gl.generateMipmap (GL_TEXTURE_2D);

  opengl::scoped::bool_setter<GL_TEXTURE_GEN_S, GL_TRUE> const texture_gen_s;
  gl.texGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  gl.texGenfv(GL_S, GL_OBJECT_PLANE, CoordGen);

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
}

void CreateStrips()
{
  for (int i = 0; i < 32; ++i)
  {
    if (i < 9)
      LineStrip[i] = i;
    else if (i < 17)
      LineStrip[i] = 8 + (i - 8) * 17;
    else if (i < 25)
      LineStrip[i] = 145 - (i - 15);
    else
      LineStrip[i] = (32 - i) * 17;
  }

  LineStrip[31] = 0;

  int iferget = 0;

  for (size_t i = 34; i < 43; ++i)
    HoleStrip[iferget++] = i;

  for (size_t i = 68; i < 77; ++i)
    HoleStrip[iferget++] = i;

  for (size_t i = 102; i < 111; ++i)
    HoleStrip[iferget++] = i;

  for (size_t i = 2; i < 139; i += 17)
    HoleStrip[iferget++] = i;

  for (size_t i = 4; i < 141; i += 17)
    HoleStrip[iferget++] = i;

  for (size_t i = 6; i < 143; i += 17)
    HoleStrip[iferget++] = i;
}

MapChunk::MapChunk(MapTile *maintile, MPQFile *f, bool bigAlpha)
  : mBigAlpha(bigAlpha)
  , water(false)
  , mt(maintile)
  , textureSet(new TextureSet)
{
  uint32_t fourcc;
  uint32_t size;
  hasMCCV = false;

  f->read(&fourcc, 4);
  f->read(&size, 4);

  assert(fourcc == 'MCNK');

  size_t lastpos = f->getPos() + size;

  f->read(&header, 0x80);

  Flags = header.flags;
  areaID = header.areaid;

  if (Environment::getInstance()->areaIDColors.find(areaID) == Environment::getInstance()->areaIDColors.end())
  {
    math::vector_3d newColor = math::vector_3d(misc::randfloat(0.0f, 1.0f), misc::randfloat(0.0f, 1.0f), misc::randfloat(0.0f, 1.0f));
    Environment::getInstance()->areaIDColors.insert(std::pair<int, math::vector_3d>(areaID, newColor));
  }

  Environment::getInstance()->selectedAreaID = areaID; //The last loaded is selected on start.

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

  while (f->getPos() < lastpos)
  {
    f->read(&fourcc, 4);
    f->read(&size, 4);

    size_t nextpos = f->getPos() + size;

    if (fourcc == 'MCNR') {
      nextpos = f->getPos() + 0x1C0; // size fix
                                     // normal vectors
      char nor[3];
      math::vector_3d *ttn = mNormals;
      for (int j = 0; j<17; ++j) {
        for (int i = 0; i<((j % 2) ? 8 : 9); ++i) {
          f->read(nor, 3);
          // order X,Z,Y
          // *ttn++ = math::vector_3d((float)nor[0]/127.0f, (float)nor[2]/127.0f, (float)nor[1]/127.0f);
          *ttn++ = math::vector_3d(-nor[1] / 127.0f, nor[2] / 127.0f, -nor[0] / 127.0f);
        }
      }
    }
    else if (fourcc == 'MCVT')
    {
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
      r = (vmax - vmin).length() * 0.5f;
      // use absolute y pos in vertices
      ybase = 0.0f;
      header.ypos = 0.0f;
    }
    else if (fourcc == 'MCLY')
    {
      textureSet->initTextures(f, mt, size);
    }
    else if (fourcc == 'MCSH')
    {
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
    else if (fourcc == 'MCAL')
    {
      textureSet->initAlphamaps(f, header.nLayers, mBigAlpha, (header.flags & FLAG_do_not_fix_alpha_map) == 0);
    }
    else if (fourcc == 'MCCV')
    {
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
    f->seek(nextpos);
  }

  // create vertex buffers
  gl.genBuffers(1, &vertices);
  gl.genBuffers(1, &normals);
  gl.genBuffers(1, &mccvEntry);
  gl.genBuffers (1, &indices);

  gl.bufferData<GL_ARRAY_BUFFER> (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER> (normals, sizeof(mNormals), mNormals, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER> (mccvEntry, sizeof(mccv), mccv, GL_STATIC_DRAW);

  initStrip();

  vcenter = (vmin + vmax) * 0.5f;

  math::vector_3d *ttv = mMinimap;

  // vertices
  for (int j = 0; j<17; ++j) {
    for (int i = 0; i < ((j % 2) ? 8 : 9); ++i) {
      float xpos, zpos;
      //f->read(&h,4);
      xpos = i * 0.125f;
      zpos = j * 0.5f * 0.125f;
      if (j % 2) {
        xpos += 0.125f*0.5f;
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
    //tm[j].z=tv[j].y;
    ShadowAmount = 1.0f - (-mNormals[j].x + mNormals[j].y - mNormals[j].z);
    if (ShadowAmount<0)
      ShadowAmount = 0.0f;
    if (ShadowAmount>1.0)
      ShadowAmount = 1.0f;
    ShadowAmount *= 0.5f;
    //ShadowAmount=0.2;
    mFakeShadows[j].x = 0;
    mFakeShadows[j].y = 0;
    mFakeShadows[j].z = 0;
    mFakeShadows[j].w = ShadowAmount;
  }

  gl.genBuffers(1, &minimap);
  gl.genBuffers(1, &minishadows);

  gl.bufferData<GL_ARRAY_BUFFER> (minimap, sizeof(mMinimap), mMinimap, GL_STATIC_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER> (minishadows, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}

void MapChunk::ClearShader()
{
  for (int i = 0; i < mapbufsize; ++i)
  {
    mccv[i] = math::vector_3d(1.0f, 1.0f, 1.0f);
  }

  gl.bufferData<GL_ARRAY_BUFFER> (mccvEntry, sizeof(mccv), mccv, GL_STATIC_DRAW);
}

void MapChunk::drawTextures()
{
  gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);

  if (textureSet->num() > 0U)
  {
    textureSet->bindTexture(0, 0);
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

  textureSet->start2DAnim(0);
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
  textureSet->stop2DAnim(0);

  if (textureSet->num() > 1U)
  {
    //gl.depthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    //gl.depthMask(GL_FALSE);
  }

  for (size_t i = 1; i < textureSet->num(); ++i)
  {
    textureSet->bindTexture(i, 0);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    textureSet->bindAlphamap(i - 1, 1);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    textureSet->start2DAnim(i);

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

    textureSet->start2DAnim(i);
  }

  opengl::texture::set_active_texture (0);
  opengl::texture::disable_texture();

  opengl::texture::set_active_texture (1);
  opengl::texture::disable_texture();

  gl.vertexPointer (minimap, 3, GL_FLOAT, 0, 0);
  gl.colorPointer (minishadows, 4, GL_FLOAT, 0, 0);

  gl.drawElements(GL_TRIANGLE_STRIP, stripsize2, GL_UNSIGNED_SHORT, gWorld->mapstrip2);
}

int MapChunk::indexLoD(int x, int y)
{
  return (x + 1) * 9 + x * 8 + y;
}

int MapChunk::indexNoLoD(int x, int y)
{
  return x * 8 + x * 9 + y;
}

void MapChunk::initStrip()
{
  strip = new StripType[768]; //! \todo  figure out exact length of strip needed
  StripType* s = strip;

  for (int x = 0; x<8; ++x)
  {
    for (int y = 0; y<8; ++y)
    {
      if (isHole(x / 2, y / 2))
        continue;

      *s++ = indexLoD(y, x); //9
      *s++ = indexNoLoD(y, x); //0
      *s++ = indexNoLoD(y + 1, x); //17
      *s++ = indexLoD(y, x); //9
      *s++ = indexNoLoD(y + 1, x); //17
      *s++ = indexNoLoD(y + 1, x + 1); //18
      *s++ = indexLoD(y, x); //9
      *s++ = indexNoLoD(y + 1, x + 1); //18
      *s++ = indexNoLoD(y, x + 1); //1
      *s++ = indexLoD(y, x); //9
      *s++ = indexNoLoD(y, x + 1); //1
      *s++ = indexNoLoD(y, x); //0
    }
  }
  striplen = static_cast<int>(s - strip);

  opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const _ (indices);
  gl.bufferData (GL_ELEMENT_ARRAY_BUFFER, striplen * sizeof (StripType), strip, GL_STATIC_DRAW);
}

MapChunk::~MapChunk()
{
  delete textureSet;

  gl.deleteBuffers(1, &vertices);
  gl.deleteBuffers(1, &normals);
  gl.deleteBuffers(1, &mccvEntry);
  gl.deleteBuffers (1, &indices);

  if (strip)
  {
    delete strip;
    strip = nullptr;
  }
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

  gl.bufferData<GL_ARRAY_BUFFER>
    (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);

}

void MapChunk::drawLines (Frustum const& frustum)
{
  if (!frustum.intersects(vmin, vmax))
    return;

  if (!gWorld->drawlines)
    return;

  float mydist = (gWorld->camera - vcenter).length() - r;

  if (mydist > (mapdrawdistance * mapdrawdistance))
    return;

  gl.vertexPointer (vertices, 3, GL_FLOAT, 0, 0);

  opengl::texture::disable_texture();

  opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> const lighting;
  opengl::scoped::matrix_pusher const matrix;

  opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
  gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  gl.translatef(0.0f, 0.05f, 0.0f);
  gl.lineWidth(1.5);
  gl.color4f(1.0, 0.0, 0.0f, 0.5f);

  if ((px != 15) && (py != 0))
  {
    gl.drawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, LineStrip);
  }
  else if ((px == 15) && (py == 0))
  {
    gl.color4f(0.0, 1.0, 0.0f, 0.5f);
    gl.drawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, LineStrip);
  }
  else if (px == 15)
  {
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, LineStrip);
    gl.color4f(0.0, 1.0, 0.0f, 0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &LineStrip[8]);
  }
  else if (py == 0)
  {
    gl.color4f(0.0, 1.0, 0.0f, 0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, LineStrip);
    gl.color4f(1.0, 0.0, 0.0f, 0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &LineStrip[8]);
  }

  if (Environment::getInstance()->view_holelines)
  {
    // Draw hole lines if view_subchunk_lines is true
    gl.color4f(0.0, 0.0, 1.0f, 0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, HoleStrip);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[9]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[18]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[27]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[36]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &HoleStrip[45]);
  }

  gl.color4f(1, 1, 1, 1);
}

void MapChunk::drawContour()
{
  if (!DrawMapContour)
    return;
  gl.color4f(1, 1, 1, 1);
  opengl::scoped::texture_setter<0, GL_TRUE> const texture;
  opengl::scoped::bool_setter<GL_BLEND, GL_TRUE> const blend;
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  opengl::scoped::bool_setter<GL_ALPHA_TEST, GL_FALSE> const alpha_test;
  if (Contour == 0)
    GenerateContourMap();
  gl.bindTexture(GL_TEXTURE_2D, Contour);

  opengl::scoped::bool_setter<GL_TEXTURE_GEN_S, GL_TRUE> const texture_gen_s;
  gl.texGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  gl.texGenfv(GL_S, GL_OBJECT_PLANE, CoordGen);

  gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
}

void MapChunk::draw (Frustum const& frustum)
{
  if (!frustum.intersects(vmin, vmax))
    return;

  float mydist = (gWorld->camera - vcenter).length() - r;

  if (mydist > (mapdrawdistance * mapdrawdistance))
    return;

  bool cantPaint = !canPaintTexture(UITexturingGUI::getSelectedTexture())
                 && Environment::getInstance()->highlightPaintableChunks
                 && terrainMode == 2;

  if (cantPaint)
  {
    gl.color4f(1, 0, 0, 1);
  }

  // setup vertex buffers
  gl.vertexPointer (vertices, 3, GL_FLOAT, 0, 0);
  gl.normalPointer (normals, GL_FLOAT, 0, 0);

  opengl::scoped::buffer_binder<GL_ELEMENT_ARRAY_BUFFER> const index_buffer (indices);

  if (hasMCCV)
  {
    gl.colorPointer (mccvEntry, 3, GL_FLOAT, 0, 0);
    gl.enableClientState(GL_COLOR_ARRAY);
  }

  // ASSUME: texture coordinates set up already

  // first pass: base texture

  if (textureSet->num() == 0U)
  {
    opengl::texture::set_active_texture (0);
    opengl::texture::disable_texture();

    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();

    gl.color3f(1.0f, 1.0f, 1.0f);
  }
  else
  {
    textureSet->bindTexture(0, 0);

    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();
  }

  gl.enable(GL_LIGHTING);
  gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);

  if (textureSet->num() > 1U) {
    //gl.depthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    gl.depthMask(GL_FALSE);
  }

  // additional passes: if required
  for (size_t i = 1; i < textureSet->num(); ++i)
  {
    // this time, use blending:
    textureSet->bindTexture(i, 0);
    textureSet->bindAlphamap(i - 1, 1);

    textureSet->startAnim (i);
    gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
    textureSet->stopAnim (i);
  }

  if (textureSet->num() > 1U)
  {
    //gl.depthFunc(GL_LEQUAL);
    gl.depthMask(GL_TRUE);
  }

  if (hasMCCV)
    gl.disableClientState(GL_COLOR_ARRAY);

  if (cantPaint)
  {
    gl.color4f(1, 1, 1, 1);
  }

  // shadow map
  opengl::texture::set_active_texture (0);
  opengl::texture::disable_texture();
  gl.disable(GL_LIGHTING);

  math::vector_3d shc = gWorld->skies->colorSet[WATER_COLOR_DARK] * 0.3f;
  gl.color4f(shc.x, shc.y, shc.z, 1);

  //gl.color4f(1,1,1,1);

  opengl::texture::enable_texture (1);
  shadow.bind();

  gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);

  opengl::texture::disable_texture();
  gl.disable(GL_LIGHTING);

  drawContour();

  if (terrainMode == 5)
  {
    // draw chunk white if impassible flag is set
    if (Flags & FLAG_IMPASS)
    {
      gl.color4f(1, 1, 1, 0.6f);
      gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
    }
  }
  if (terrainMode == 6)
  {
    if (water)
    {
      gl.color4f(0.2f, 0.2f, 0.8f, 0.6f);
      gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
    }
  }

  if (terrainMode == 4)
  {
    // draw chunks in color depending on AreaID and list color from environment
    if (Environment::getInstance()->areaIDColors.find(areaID) != Environment::getInstance()->areaIDColors.end())
    {
      math::vector_3d colorValues = Environment::getInstance()->areaIDColors.find(areaID)->second;
      gl.color4f(colorValues.x, colorValues.y, colorValues.z, 0.7f);
      gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
    }
  }

  if (Environment::getInstance()->cursorType == 3)
  {
    if (gWorld->IsSelection(eEntry_MapChunk) && boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).chunk == this && terrainMode != 3)
    {
      int poly = boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).triangle;

      gl.color4f(1.0f, 1.0f, 0.0f, 1.0f);

      opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull_face;
      opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;
      opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> const depth_test;

      gl.begin(GL_TRIANGLES);
      gl.vertex3fv(mVertices[strip[poly + 0]]);
      gl.vertex3fv(mVertices[strip[poly + 1]]);
      gl.vertex3fv(mVertices[strip[poly + 2]]);
      gl.end();
    }
  }

  if (gWorld->drawwireframe)
  {
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> const lighting;
    opengl::texture::disable_texture();

    {
      opengl::scoped::bool_setter<GL_POLYGON_OFFSET_LINE, GL_TRUE> const polygon_offset_line;
      gl.polygonMode(GL_FRONT_AND_BACK, GL_LINE);
      gl.lineWidth(1);
      gl.polygonOffset(-1, -1);
      gl.color4f(1, 1, 1, 0.2f);
      gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
    }
    {
      opengl::scoped::bool_setter<GL_POLYGON_OFFSET_POINT, GL_TRUE> const polygon_offset_point;
      gl.polygonMode(GL_FRONT_AND_BACK, GL_POINT);
      gl.pointSize(2);
      gl.polygonOffset(-1, -1);
      gl.color4f(1, 1, 1, 0.5f);
      gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
    }

    gl.polygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  gl.enable(GL_LIGHTING);
  gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void MapChunk::SetWater(bool w)
{
  this->water = w;
}

bool MapChunk::GetWater()
{
  return this->water;
}

void MapChunk::intersect (math::ray const& ray, selection_result* results)
{
  if (!ray.intersect_bounds (vmin, vmax))
  {
    return;
  }

  for (int i (0); i < striplen; i += 3)
  {
    if ( auto&& distance = ray.intersect_triangle ( mVertices[strip[i + 0]]
                                                  , mVertices[strip[i + 1]]
                                                  , mVertices[strip[i + 2]]
                                                  )
       )
    {
      results->emplace_back
        (*distance, selected_chunk_type (this, i, ray.position (*distance)));
    }
  }
}

void MapChunk::recalcNorms()
{
  math::vector_3d P1, P2, P3, P4;
  math::vector_3d Norm, N1, N2, N3, N4, D;

  for (int i = 0; i<mapbufsize; ++i)
  {
    if (!gWorld->GetVertex(mVertices[i].x - UNITSIZE*0.5f, mVertices[i].z - UNITSIZE*0.5f, &P1))
    {
      P1.x = mVertices[i].x - UNITSIZE*0.5f;
      P1.y = mVertices[i].y;
      P1.z = mVertices[i].z - UNITSIZE*0.5f;
    }

    if (!gWorld->GetVertex(mVertices[i].x + UNITSIZE*0.5f, mVertices[i].z - UNITSIZE*0.5f, &P2))
    {
      P2.x = mVertices[i].x + UNITSIZE*0.5f;
      P2.y = mVertices[i].y;
      P2.z = mVertices[i].z - UNITSIZE*0.5f;
    }

    if (!gWorld->GetVertex(mVertices[i].x + UNITSIZE*0.5f, mVertices[i].z + UNITSIZE*0.5f, &P3))
    {
      P3.x = mVertices[i].x + UNITSIZE*0.5f;
      P3.y = mVertices[i].y;
      P3.z = mVertices[i].z + UNITSIZE*0.5f;
    }

    if (!gWorld->GetVertex(mVertices[i].x - UNITSIZE*0.5f, mVertices[i].z + UNITSIZE*0.5f, &P4))
    {
      P4.x = mVertices[i].x - UNITSIZE*0.5f;
      P4.y = mVertices[i].y;
      P4.z = mVertices[i].z + UNITSIZE*0.5f;
    }

    N1 = (P2 - mVertices[i]) % (P1 - mVertices[i]);
    N2 = (P3 - mVertices[i]) % (P2 - mVertices[i]);
    N3 = (P4 - mVertices[i]) % (P3 - mVertices[i]);
    N4 = (P1 - mVertices[i]) % (P4 - mVertices[i]);

    Norm = N1 + N2 + N3 + N4;
    Norm.normalize();
    mNormals[i] = Norm;
  }
  gl.bufferData<GL_ARRAY_BUFFER> (normals, sizeof(mNormals), mNormals, GL_STATIC_DRAW);

  float ShadowAmount;
  for (int j = 0; j<mapbufsize; ++j)
  {
    //tm[j].z=tv[j].y;
    ShadowAmount = 1.0f - (-mNormals[j].x + mNormals[j].y - mNormals[j].z);
    if (ShadowAmount<0)
      ShadowAmount = 0;
    if (ShadowAmount>1.0)
      ShadowAmount = 1.0f;
    ShadowAmount *= 0.5f;
    //ShadowAmount=0.2;
    mFakeShadows[j].x = 0;
    mFakeShadows[j].y = 0;
    mFakeShadows[j].z = 0;
    mFakeShadows[j].w = ShadowAmount;
  }

  gl.bufferData<GL_ARRAY_BUFFER> (minishadows, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}

bool MapChunk::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
  float dist, xdiff, zdiff;
  bool changed = false;

  vmin.y = 9999999.0f;
  vmax.y = -9999999.0f;
  for (int i = 0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x - x;
    zdiff = mVertices[i].z - z;
    if (BrushType == eTerrainType_Quadra)
    {
      if ((std::abs(xdiff) < std::abs(radius / 2)) && (std::abs(zdiff) < std::abs(radius / 2))) 
      {
        mVertices[i].y += change;
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
            mVertices[i].y += change*(1.0f - dist / radius);
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
          default:
            LogError << "Invalid terrain edit type (" << BrushType << ")" << std::endl;
            changed = false;
            break;
        }
      }
    }

    vmin.y = std::min(vmin.y, mVertices[i].y);
    vmax.y = std::max(vmax.y, mVertices[i].y);
  }
  if (changed)
  {
    gl.bufferData<GL_ARRAY_BUFFER> (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return changed;
}

bool MapChunk::ChangeMCCV(float x, float z, float change, float radius, bool editMode)
{
  float dist, xdiff, zdiff;
  bool changed = false;

  if (!hasMCCV)
  {
    ClearShader(); // create default shaders
    changed = true;
    Flags |= FLAG_MCCV;
    hasMCCV = true;
  }

  for (int i = 0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x - x;
    zdiff = mVertices[i].z - z;
    dist = sqrt(xdiff*xdiff + zdiff*zdiff);
    if (dist <= radius)
    {
      float edit = change * (1.0f - dist / radius);
      if (editMode)
      {
        mccv[i].x += (Environment::getInstance()->cursorColorR - mccv[i].x)* edit;
        mccv[i].y += (Environment::getInstance()->cursorColorG - mccv[i].y)* edit;
        mccv[i].z += (Environment::getInstance()->cursorColorB - mccv[i].z)* edit;
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

bool MapChunk::flattenTerrain ( float x
                              , float z
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

  for (int i (0); i < mapbufsize; ++i)
  {
    float const dist (misc::dist (mVertices[i].x, mVertices[i].z, x, z));

    if (dist >= radius)
    {
      continue;
    }

    float const ah ( origin.y
                   + ( (mVertices[i].x - origin.x) * math::cos (orientation)
                     + (mVertices[i].z - origin.z) * math::sin (orientation)
                     ) * math::tan (angle)
                   );

    if ( (flattenType == eFlattenMode_Raise && ah < mVertices[i].y)
       || (flattenType == eFlattenMode_Lower && ah > mVertices[i].y)
       )
    {
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
    vmin.y = std::numeric_limits<float>::max();
    vmax.y = std::numeric_limits<float>::lowest();

    for (int i (0); i < mapbufsize; ++i)
    {
      vmin.y = std::min (vmin.y, mVertices[i].y);
      vmax.y = std::max (vmax.y, mVertices[i].y);
    }

    gl.bufferData<GL_ARRAY_BUFFER> (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }

  return changed;
}

bool MapChunk::blurTerrain(float x, float z, float remain, float radius, int BrushType)
{
  bool changed (false);

  for (int i (0); i < mapbufsize; ++i)
  {
    float const dist (misc::dist (mVertices[i].x, mVertices[i].z, x, z));

    if (dist >= radius)
    {
      continue;
    }

    int Rad = (int)(radius / UNITSIZE);
    float TotalHeight = 0;
    float TotalWeight = 0;
    for (int j = -Rad * 2; j <= Rad * 2; ++j)
    {
      float tz = z + j * UNITSIZE / 2;
      for (int k = -Rad; k <= Rad; ++k)
      {
        float tx = x + k*UNITSIZE + (j % 2) * UNITSIZE / 2.0f;
        float dist2 = misc::dist (tx, tz, mVertices[i].x, mVertices[i].z);
        if (dist2 > radius)
          continue;
        math::vector_3d TempVec;
        gWorld->GetVertex(tx, tz, &TempVec);
        TotalHeight += (1.0f - dist2 / radius) * TempVec.y;
        TotalWeight += (1.0f - dist2 / radius);
      }
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
    vmin.y = std::numeric_limits<float>::max();
    vmax.y = std::numeric_limits<float>::lowest();

    for (int i (0); i < mapbufsize; ++i)
    {
      vmin.y = std::min (vmin.y, mVertices[i].y);
      vmax.y = std::max (vmax.y, mVertices[i].y);
    }

    gl.bufferData<GL_ARRAY_BUFFER> (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }

  return changed;
}


void MapChunk::eraseTextures()
{
  textureSet->eraseTextures();
}

int MapChunk::addTexture(OpenGL::Texture* texture)
{
  return textureSet->addTexture(texture);
}

void MapChunk::switchTexture(OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture)
{
  textureSet->switchTexture(oldTexture, newTexture);
}

bool MapChunk::paintTexture(float x, float z, Brush* brush, float strength, float pressure, OpenGL::Texture* texture)
{
  return textureSet->paintTexture(xbase, zbase, x, z, brush, strength, pressure, texture);
}

bool MapChunk::canPaintTexture(OpenGL::Texture* texture)
{
  return textureSet->canPaintTexture(texture);
}

bool MapChunk::isHole(int i, int j)
{
  return (holes & ((1 << ((j * 4) + i)))) != 0;
}

void MapChunk::addHole(int i, int j)
{
  holes = holes | ((1 << ((j * 4) + i)));
  initStrip();
}

void MapChunk::addHoleBig(int i, int j)
{
  for (int x = -3; x < 4; ++x)
  {
    for (int y = -3; y < 4; ++y)
    {
      addHole(i + x, j + y);
    }
  }
}

void MapChunk::addHoleEverywhere()
{
  holes = 0x7FFFFFFF;
  initStrip();
}

void MapChunk::removeHole(int i, int j)
{
  holes = holes & ~((1 << ((j * 4) + i)));
  initStrip();
}

void MapChunk::removeHoleBig(int i, int j)
{
  for (int x = -3; x<4; x++)
  {
    for (int y = -3; y<4; y++)
    {
      removeHole(i + x, j + y);
    }
  }
}

void MapChunk::removeAllHoles()
{
  holes = 0x0;
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


void MapChunk::setFlag(bool changeto)
{
  if (changeto)
    this->Flags = this->Flags | (Environment::getInstance()->flagPaintMode);
  else
    this->Flags = this->Flags & ~(Environment::getInstance()->flagPaintMode);
}

int MapChunk::getFlag()
{
  return this->Flags;
}

void MapChunk::save(sExtendableArray &lADTFile, int &lCurrentPosition, int &lMCIN_Position, std::map<std::string, int> &lTextures, std::map<int, WMOInstance> &lObjectInstances, std::map<int, ModelInstance> &lModelInstances)
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

  memset(lMCNK_header->low_quality_texture_map, 0, 0x10);

  static const size_t minimum_value_to_overwrite(128);

  for (size_t y(0); y < 8; ++y)
  {
    for (size_t x(0); x < 8; ++x)
    {
      size_t winning_layer(0);
      for (size_t layer(1); layer < textureSet->num(); ++layer)
      {
        size_t sum(0);

        for (size_t j(0); j < 8; ++j)
        {
          for (size_t i(0); i < 8; ++i)
          {
            sum += textureSet->getAlpha(layer - 1, (y * 8 + j) * 64 + (x * 8 + i));
          }
        }

        if (sum  > minimum_value_to_overwrite * 8 * 8)
        {
          winning_layer = layer;
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

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ypos = 0.0f;

  for (int i = 0; i < mapbufsize; ++i)
    lHeightmap[i] = mVertices[i].y;

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

  // recalculate the normals
  recalcNorms();
  for (int i = 0; i < mapbufsize; ++i)
  {
    lNormals[i * 3 + 0] = misc::roundc(-mNormals[i].z * 127);
    lNormals[i * 3 + 1] = misc::roundc(-mNormals[i].x * 127);
    lNormals[i * 3 + 2] = misc::roundc(mNormals[i].y * 127);
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
  size_t lMCLY_Size = textureSet->num() * 0x10;

  lADTFile.Extend(8 + lMCLY_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCLY', lMCLY_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsLayer = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->nLayers = textureSet->num();

  // MCLY data
  for (size_t j = 0; j < textureSet->num(); ++j)
  {
    ENTRY_MCLY * lLayer = lADTFile.GetPointer<ENTRY_MCLY>(lCurrentPosition + 8 + 0x10 * j);

    lLayer->textureID = lTextures.find(textureSet->filename(j))->second;

    lLayer->flags = textureSet->flag(j);

    // if not first, have alpha layer, if first, have not. never have compression.
    lLayer->flags = (j > 0 ? lLayer->flags | FLAG_USE_ALPHA : lLayer->flags & (~FLAG_USE_ALPHA)) & (~FLAG_ALPHA_COMPRESSED);

    lLayer->ofsAlpha = (j == 0 ? 0 : (mBigAlpha ? 64 * 64 * (j - 1) : 32 * 64 * (j - 1)));
    lLayer->effectID = textureSet->effect(j);
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
  for (std::map<int, WMOInstance>::iterator it = lObjectInstances.begin(); it != lObjectInstances.end(); ++it)
  {
    if (it->second.isInsideRect(lChunkExtents))
      lObjectIDs.push_back(lID);

    lID++;
  }

  // search all models that are inside this chunk
  lID = 0;
  for (std::map<int, ModelInstance>::iterator it = lModelInstances.begin(); it != lModelInstances.end(); ++it)
  {
    if (it->second.isInsideRect(lChunkExtents))
      lDoodadIDs.push_back(lID);

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
  int lDimensions = 64 * (mBigAlpha ? 64 : 32);

  size_t lMaps = textureSet->num() ? textureSet->num() - 1U : 0U;

  int lMCAL_Size = lDimensions * lMaps;

  lADTFile.Extend(8 + lMCAL_Size);
  SetChunkHeader(lADTFile, lCurrentPosition, 'MCAL', lMCAL_Size);

  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->ofsAlpha = lCurrentPosition - lMCNK_Position;
  lADTFile.GetPointer<MapChunkHeader>(lMCNK_Position + 8)->sizeAlpha = 8 + lMCAL_Size;

  char * lAlphaMaps = lADTFile.GetPointer<char>(lCurrentPosition + 8);

  // convert bigAlpha to the correct format for saving
  if (mBigAlpha)
    textureSet->convertToBigAlpha();

  for (size_t j = 0; j < lMaps; j++)
  {
    //First thing we have to do is downsample the alpha maps before we can write them
    if (mBigAlpha)
      for (int k = 0; k < lDimensions; k++)
        lAlphaMaps[lDimensions * j + k] = textureSet->getAlpha(j, k);
    else
    {
      unsigned char upperNibble, lowerNibble;
      for (int k = 0; k < lDimensions; k++)
      {
        lowerNibble = (textureSet->getAlpha(j, k * 2 + 0) & 0xF0);
        upperNibble = (textureSet->getAlpha(j, k * 2 + 1) & 0xF0);
        lAlphaMaps[lDimensions * j + k] = (upperNibble)+(lowerNibble >> 4);
      }
    }
  }

  // convert bigAlpha to the correct old format to be able to edit them correctly
  if (mBigAlpha)
    textureSet->convertToOldAlpha();

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
    gl.bufferData<GL_ARRAY_BUFFER> (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
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
    gl.bufferData<GL_ARRAY_BUFFER> (vertices, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return changed;
}

//! ------ unused functions -----

/*
void MapChunk::drawNoDetail()
{
opengl::texture::set_active_texture (1);
opengl::texture::disable_texture();
opengl::texture::set_active_texture (0);
opengl::texture::disable_texture();
gl.disable( GL_LIGHTING );

//gl.color3fv(gWorld->skies->colorSet[FOG_COLOR]);
//gl.color3f(1,0,0);
//gl.disable(GL_FOG);

// low detail version
gl.vertexPointer (vertices, 3, GL_FLOAT, 0, 0 );
gl.disableClientState( GL_NORMAL_ARRAY );
gl.drawElements( GL_TRIANGLE_STRIP, stripsize, GL_UNSIGNED_SHORT, gWorld->mapstrip );
gl.enableClientState( GL_NORMAL_ARRAY );

gl.color4f( 1.0f, 1.0f, 1.0f, 1.0f );
//gl.enable(GL_FOG);

gl.enable( GL_LIGHTING );
opengl::texture::set_active_texture (1);
opengl::texture::enable_texture();
opengl::texture::set_active_texture (0);
opengl::texture::enable_texture();
}
*/

/*
void MapChunk::drawColor()
{

if (!gWorld->frustum.intersects(vmin,vmax))
return;

float mydist = (gWorld->camera - vcenter).length() - r;

if (mydist > (mapdrawdistance * mapdrawdistance))
return;

if (mydist > gWorld->culldistance) {
if (gWorld->drawfog) this->drawNoDetail();
return;
}

opengl::texture::set_active_texture (1);
opengl::texture::disable_texture();

opengl::texture::set_active_texture (0);
opengl::texture::disable_texture();
//gl.disable(GL_LIGHTING);

math::vector_3d Color;
gl.begin(GL_TRIANGLE_STRIP);
for(int i=0; i < striplen; ++i)
{
HeightColor( mVertices[strip[i]].y, &Color);
gl.color3fv(&Color.x);
gl.normal3fv(&mNormals[strip[i]].x);
gl.vertex3fv(&mVertices[strip[i]].x);
}
gl.end();
//gl.enable(GL_LIGHTING);
}
*/

/*
void MapChunk::loadTextures()
{
//! \todo Use this kind of preloading again?
return;
for(int i=0; i < nTextures; ++i)
_textures[i] = TextureManager::get(mt->mTextureFilenames[tex[i]]);
}
*/



/*
static const int HEIGHT_TOP = 1000;
static const int HEIGHT_MID = 600;
static const int HEIGHT_LOW = 300;
static const int HEIGHT_ZERO = 0;
static const int HEIGHT_SHALLOW = -100;
static const int HEIGHT_DEEP = -250;

void HeightColor(float height, math::vector_3d *Color)
{
White  1.00  1.00  1.00
Brown  0.75  0.50  0.00
Green  0.00  1.00  0.00
Yellow  1.00  1.00  0.00
Lt Blue  0.00  1.00  1.00
Blue  0.00  0.00  1.00
Black  0.00  0.00  0.00

float Amount;

if(height>HEIGHT_TOP)
{
Color->x=1.0;
Color->y=1.0;
Color->z=1.0;
}
else if(height>HEIGHT_MID)
{
Amount=(height-HEIGHT_MID)/(HEIGHT_TOP-HEIGHT_MID);
Color->x=.75f+Amount*0.25f;
Color->y=0.5f+0.5f*Amount;
Color->z=Amount;
}
else if(height>HEIGHT_LOW)
{
Amount=(height-HEIGHT_LOW)/(HEIGHT_MID-HEIGHT_LOW);
Color->x=Amount*0.75f;
Color->y=1.00f-0.5f*Amount;
Color->z=0.0f;
}
else if(height>HEIGHT_ZERO)
{
Amount=(height-HEIGHT_ZERO)/(HEIGHT_LOW-HEIGHT_ZERO);

Color->x=1.0f-Amount;
Color->y=1.0f;
Color->z=0.0f;
}
else if(height>HEIGHT_SHALLOW)
{
Amount=(height-HEIGHT_SHALLOW)/(HEIGHT_ZERO-HEIGHT_SHALLOW);
Color->x=0.0f;
Color->y=Amount;
Color->z=1.0f;
}
else if(height>HEIGHT_DEEP)
{
Amount=(height-HEIGHT_DEEP)/(HEIGHT_SHALLOW-HEIGHT_DEEP);
Color->x=0.0f;
Color->y=0.0f;
Color->z=Amount;
}
else
(*Color)*=0.0f;

}*/
