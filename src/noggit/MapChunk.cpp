// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/MapChunk.h>

#include <algorithm>
#include <ctime>
#include <iostream>

#include <QMap>

#include <math/random.h>
#include <math/vector_3d.h>

#include <opengl/context.h>
#include <opengl/scoped.h>
#include <opengl/texture.h>

#include <noggit/application.h>
#include <noggit/Brush.h>
#include <noggit/Frustum.h> // Frustum
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/Selection.h>
#include <noggit/World.h>
#include <noggit/mpq/file.h>
#include <noggit/texture_set.hpp>

static const float texDetail = 8.0f;
static const float TEX_RANGE = 62.0f / 64.0f;

namespace
{
  int indexLoD(int x, int y)
  {
    return (x+1)*9+x*8+y;
  }

  int indexNoLoD(int x, int y)
  {
    return x * 8 + x * 9 + y;
  }
}

//! \note I am aware of this being global state not even being inside a class BUT I DONT CARE AT ALL.
// THIS WHOLE THING IS BULLSHIT AND NOT WORTH A BIT AND COSTING ME TIME OF MY LIFE FOR JUST BEING BAD AS FUCK.
// REMOVEING ENVIRONMENT TOOK ME HOURS WHILE MOST VARIABLES HAD BEEN ONLY USED IN A SINGLE FUCKING CLASS,  DID NOT HAVE MEANINGFUL NAMES OR ANYTHING.
namespace
{
  QMap<int, ::math::vector_3d> areaIDColors;
}

MapChunk::MapChunk(World* world, MapTile* maintile, noggit::mpq::file* f,bool bigAlpha)
  : _world (world)
{
  CreateStrips();

  uint32_t fourcc;
  uint32_t size;

  size_t base = f->getPos();
  unsigned int MCALoffset[4];

  // - MCNK ----------------------------------------------
  {
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCNK');

    f->read(&header, 0x80);

    if (!areaIDColors.contains(header.areaid))
    {
      areaIDColors[header.areaid] = ::math::vector_3d(::math::random::floating_point(0.0f, 1.0f)
        , ::math::random::floating_point(0.0f, 1.0f)
        , ::math::random::floating_point(0.0f, 1.0f)
      );
    }

    zbase = header.zpos;
    xbase = header.xpos;
    ybase = header.ypos;

    px = header.ix;
    py = header.iy;

    holes = header.holes;
    // correct the x and z values ^_^
    zbase = zbase * -1.0f + ZEROPOINT;
    xbase = xbase * -1.0f + ZEROPOINT;

    vmin = ::math::vector_3d(9999999.0f, 9999999.0f, 9999999.0f);
    vmax = ::math::vector_3d(-9999999.0f, -9999999.0f, -9999999.0f);
  }
  // - MCVT ----------------------------------------------
  {
    f->seek(base + header.ofsHeight);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCVT');

    ::math::vector_3d *ttv = mVertices;

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
        ::math::vector_3d v(xbase + xpos, ybase + h, zbase + zpos);
        *ttv++ = v;
        vmin.y(std::min(vmin.y(), v.y()));
        vmax.y(std::max(vmax.y(), v.y()));
      }
    }

    vmin.x(xbase);
    vmin.z(zbase);
    vmax.x(xbase + 8 * UNITSIZE);
    vmax.z(zbase + 8 * UNITSIZE);
  }
  // - MCNR ----------------------------------------------
  {
    f->seek(base + header.ofsNormal);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCNR');

    char nor[3];
    ::math::vector_3d *ttn = mNormals;
    for (int j = 0; j < 17; ++j) {
      for (int i = 0; i < ((j % 2) ? 8 : 9); ++i) {
        f->read(nor, 3);
        // order X,Z,Y
        // *ttn++ = ::math::vector_3d((float)nor[0]/127.0f, (float)nor[2]/127.0f, (float)nor[1]/127.0f);
        *ttn++ = ::math::vector_3d(-nor[1] / 127.0f, nor[2] / 127.0f, -nor[0] / 127.0f);
      }
    }
  }
  // - MCLY ----------------------------------------------
  {
    f->seek(base + header.ofsLayer);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCLY');

    textures.initTextures(f, &maintile->mTextureFilenames, size);
  }
  // - MCSH ----------------------------------------------
  if(header.ofsShadow && header.sizeShadow)
  {
    f->seek(base + header.ofsShadow);
    f->read(&fourcc, 4);
    f->read(&size, 4);

    assert(fourcc == 'MCSH');

    f->read(mShadowMap, 0x200);
    f->seekRelative(-0x200);

    unsigned char sbuf[64 * 64], *p, c[8];
    p = sbuf;
    for (int j = 0; j < 64; ++j) {
      f->read(c, 8);
      for (int i = 0; i < 8; ++i) {
        for (int b = 0x01; b != 0x100; b <<= 1) {
          *p++ = (c[i] & b) ? 85 : 0;
        }
      }
    }
    gl.genTextures(1, &shadow);
    gl.bindTexture(GL_TEXTURE_2D, shadow);
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

    const mcnk_flags_type flags = mcnk_flags_type::interpret(header.flags);
    textures.initAlphamaps(f, maintile->mBigAlpha, flags.do_not_fix_alpha_map);
  }

  vcenter = (vmin + vmax) * 0.5f;

  // create vertex buffers
  gl.genBuffers(1,&vertices);
  gl.genBuffers(1,&normals);

  gl.bindBuffer(GL_ARRAY_BUFFER, vertices);
  gl.bufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);

  gl.bindBuffer(GL_ARRAY_BUFFER, normals);
  gl.bufferData(GL_ARRAY_BUFFER, sizeof(mNormals), mNormals, GL_STATIC_DRAW);

  gl.genBuffers(1, &indices);

  initStrip();

  // minimap
  ::math::vector_3d *ttv = mMinimap;

  for (int j=0; j<17; ++j) {
    for (int i=0; i < ((j % 2) ? 8 : 9); ++i) {
      float xpos,zpos;

      xpos = i * 0.125f;
      zpos = j * 0.5f * 0.125f;
      if (j % 2) {
                 xpos += 0.125f*0.5f;
      }
      ::math::vector_3d v = ::math::vector_3d(xpos+px, zpos+py, -1);
      *ttv++ = v;
    }
  }

  if( ( header.flags & 1 ) == 0 )
  {
    /** We have no shadow map (MCSH), so we got no shadows at all!  **
     ** This results in everything being black.. Yay. Lets fake it! **/
    for( size_t i = 0; i < 512; ++i )
      mShadowMap[i] = 0;

    unsigned char sbuf[64*64];
    for( size_t j = 0; j < 4096; ++j )
      sbuf[j] = 0;

    gl.genTextures( 1, &shadow );
    gl.bindTexture( GL_TEXTURE_2D, shadow );
    gl.texImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, sbuf );
    gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  }

  for (size_t j (0); j < mapbufsize; ++j)
  {
    mFakeShadows[j].x (0.0f);
    mFakeShadows[j].y (0.0f);
    mFakeShadows[j].z (0.0f);
    mFakeShadows[j].w ( qBound ( 0.0f
                               , 1.0f - (-mNormals[j].x()
                                        + mNormals[j].y()
                                        - mNormals[j].z()
                                        )
                               , 1.0f
                               )
                      * 0.5f
                      );
  }

  gl.genBuffers(1,&minimap);
  gl.genBuffers(1,&minishadows);

  gl.bindBuffer(GL_ARRAY_BUFFER, minimap);
  gl.bufferData(GL_ARRAY_BUFFER, sizeof(mMinimap), mMinimap, GL_STATIC_DRAW);

  gl.bindBuffer(GL_ARRAY_BUFFER, minishadows);
  gl.bufferData(GL_ARRAY_BUFFER, sizeof(mFakeShadows), mFakeShadows, GL_STATIC_DRAW);
}

namespace
{
  struct texture_animation_setup
  {
    mcly_flags_type _flags;
    boost::optional<opengl::scoped::matrix_pusher> _matrix_pusher;

    texture_animation_setup (mcly_flags_type flags)
      : _flags (flags)
    {
      if (_flags.animate)
      {
        opengl::texture::set_active_texture (0);
        gl.matrixMode (GL_TEXTURE);
        _matrix_pusher = opengl::scoped::matrix_pusher();

        static float const direction_table_x[8] = {  0.0f, -1.0f, -1.0f, -1.0f
                                                  ,  0.0f,  1.0f,  1.0f,  1.0f
                                                  };
        static float const direction_table_y[8] = {  1.0f,  1.0f,  0.0f, -1.0f
                                                  , -1.0f, -1.0f,  0.0f,  1.0f
                                                  };

        //! \todo  Find  a good  factor  to slow  this  thing  down to  an
        //! appropriate speed.
        static float const animation_slowdown_factor (1.0f);

        //! \note This does not wrap back to zero! Maybe this is therefore
        //! wrong. Needs to be tested.
        float const animation_progress ( (clock() / CLOCKS_PER_SEC)
                                       * (_flags.animation_speed + 1)
                                       * animation_slowdown_factor
                                       );

        gl.translatef ( animation_progress
                     * direction_table_x[_flags.animation_rotation]
                     , animation_progress
                     * direction_table_y[_flags.animation_rotation]
                     , 0.0f
                     );
      }
    }

    ~texture_animation_setup()
    {
      if (_flags.animate)
      {
        _matrix_pusher.reset();
        gl.matrixMode (GL_MODELVIEW);
        opengl::texture::set_active_texture (1);
      }
    }
  };
}

void MapChunk::drawTextures() const
{
  gl.color4f(1.0f,1.0f,1.0f,1.0f);

  if(textures.num() > 0U)
  {
    textures.bindTexture(0, 0);

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    opengl::texture::disable_texture (1);
  }
  else
  {
    opengl::texture::disable_texture (0);
    opengl::texture::disable_texture (1);
  }

  const mcly_flags_type& flags_layer_0
    (mcly_flags_type::interpret (textures.animated(0)));

  {
    texture_animation_setup const texture_animation (flags_layer_0);

    gl.begin(GL_TRIANGLE_STRIP);
    gl.texCoord2f(0.0f,texDetail);
    gl.vertex3f(static_cast<float>(px), py+1.0f, -2.0f);
    gl.texCoord2f(0.0f, 0.0f);
    gl.vertex3f(static_cast<float>(px), static_cast<float>(py), -2.0f);
    gl.texCoord2f(texDetail, texDetail);
    gl.vertex3f(px+1.0f, py+1.0f, -2.0f);
    gl.texCoord2f(texDetail, 0.0f);
    gl.vertex3f(px+1.0f, static_cast<float>(py), -2.0f);
    gl.end();
  }

  if (textures.num() > 1U) {
    //gl.depthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    //gl.depthMask(GL_FALSE);
  }
  for(size_t i=1; i < textures.num(); ++i)
  {
    textures.bindTexture(i, 0);

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    textures.bindAlphamap(i - 1, 1);

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const mcly_flags_type& flags (mcly_flags_type::interpret (textures.animated(i)));

    texture_animation_setup const texture_animation (flags);

    gl.begin(GL_TRIANGLE_STRIP);
    gl.multiTexCoord2f(GL_TEXTURE0, texDetail, 0.0f);
    gl.multiTexCoord2f(GL_TEXTURE1, TEX_RANGE, 0.0f);
    gl.vertex3f(px+1.0f, static_cast<float>(py), -2.0f);
    gl.multiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
    gl.multiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
    gl.vertex3f(static_cast<float>(px), static_cast<float>(py), -2.0f);
    gl.multiTexCoord2f(GL_TEXTURE0, texDetail, texDetail);
    gl.multiTexCoord2f(GL_TEXTURE1, TEX_RANGE, TEX_RANGE);
    gl.vertex3f(px+1.0f, py+1.0f, -2.0f);
    gl.multiTexCoord2f(GL_TEXTURE0, 0.0f, texDetail);
    gl.multiTexCoord2f(GL_TEXTURE1, 0.0f, TEX_RANGE);
    gl.vertex3f(static_cast<float>(px), py+1.0f, -2.0f);
    gl.end();
  }

  opengl::texture::disable_texture (0);
  opengl::texture::disable_texture (1);

  gl.bindBuffer(GL_ARRAY_BUFFER, minimap);
  gl.vertexPointer(3, GL_FLOAT, 0, 0);

  gl.bindBuffer(GL_ARRAY_BUFFER, minishadows);
  gl.colorPointer(4, GL_FLOAT, 0, 0);

  gl.drawElements(GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, strip);
}

void MapChunk::initStrip()
{
    strip = new StripType[768]; //! \todo  figure out exact length of strip needed
    StripType* s = strip;
    for (size_t y=0; y < 8; ++y) {
      for (size_t x=0; x < 8; ++x) {
        if (!isHole(x/2, y/2)) {
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y, x); //0
            *s++ = indexNoLoD(y+1, x); //17
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y+1, x); //17
            *s++ = indexNoLoD(y+1, x+1); //18
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y+1, x+1); //18
            *s++ = indexNoLoD(y, x+1); //1
            *s++ = indexLoD(y, x); //9
            *s++ = indexNoLoD(y, x+1); //1
            *s++ = indexNoLoD(y, x); //0
        }
      }
    }
    striplen = static_cast<int>(s - strip);

    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    gl.bufferData(GL_ELEMENT_ARRAY_BUFFER, striplen * sizeof(StripType), strip, GL_STATIC_DRAW);
}


MapChunk::~MapChunk()
{
  // shadow maps, too
  gl.deleteTextures( 1, &shadow );

  // delete VBOs
  gl.deleteBuffers( 1, &vertices );
  gl.deleteBuffers( 1, &normals );

  //delete IBO
  gl.deleteBuffers( 1, &indices );

  delete strip;
  strip = nullptr;
}

boost::optional<float> MapChunk::get_height ( const float& x
                                            , const float& z
                                            ) const
{
  const float xdiff (x - xbase);
  const float zdiff (z - zbase);

  const int row = static_cast<int>( zdiff / (UNITSIZE * 0.5f ) + 0.5f );
  const int column = static_cast<int>( ( xdiff - UNITSIZE * 0.5f * (row % 2) ) / UNITSIZE + 0.5f );
  if ( (row < 0) || (column < 0)
    || (row > 16) || (column > ((row % 2) ? 8 : 9))
     )
  {
    return boost::none;
  }

  return mVertices[17*(row/2) + ((row % 2) ? 9 : 0) + column].y();
}


void MapChunk::CreateStrips()
{
  for(int i=0; i < 32; ++i)
  {
    if(i < 9)
      _line_strip[i] = i;
    else if(i < 17)
      _line_strip[i] = 8 + (i-8)*17;
    else if(i < 25)
      _line_strip[i] = 145 - (i-15);
    else
      _line_strip[i] = (32-i)*17;
  }

  int iferget = 0;

  for( size_t i = 34; i < 43; ++i )
     _hole_strip[iferget++] = i;

  for( size_t i = 68; i < 77; ++i )
    _hole_strip[iferget++] = i;

  for( size_t i = 102; i < 111; ++i )
     _hole_strip[iferget++] = i;

  for( size_t i = 2; i < 139; i += 17 )
    _hole_strip[iferget++] = i;

  for( size_t i = 4; i < 141; i += 17 )
    _hole_strip[iferget++] = i;

  for( size_t i = 6; i < 143; i += 17 )
    _hole_strip[iferget++] = i;
}

float MapChunk::getHeight (int x, int z) const
{
  return mVertices[indexNoLoD(x, z)].y();
}

float MapChunk::getMinHeight() const
{
  float min (mVertices[0].y());
  for (auto const& vertex : mVertices)
  {
    min = std::min (min, vertex.y());
  }
	return min;
}

void MapChunk::drawLines (bool draw_hole_lines) const
{
  gl.bindBuffer(GL_ARRAY_BUFFER, vertices);
  gl.vertexPointer(3, GL_FLOAT, 0, 0);

  gl.disable(GL_TEXTURE_2D);
  gl.disable(GL_LIGHTING);

  opengl::scoped::matrix_pusher const matrix_pusher;

  gl.color4f(1.0,0.0,0.0f,0.5f);
  gl.translatef(0.0f,0.05f,0.0f);
  gl.enable (GL_LINE_SMOOTH);
  gl.lineWidth(1.5);
  gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);

  if( (px != 15) && (py != 0))
  {
    gl.drawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, _line_strip);
  }
  else if( (px==15) && (py==0) )
  {
    gl.color4f(0.0,1.0,0.0f,0.5f);
    gl.drawElements(GL_LINE_STRIP, 17, GL_UNSIGNED_SHORT, _line_strip);
  }
  else if(px==15)
  {
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, _line_strip);
    gl.color4f(0.0,1.0,0.0f,0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_line_strip[8]);
  }
  else if(py==0)
  {
    gl.color4f(0.0,1.0,0.0f,0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, _line_strip);
    gl.color4f(1.0,0.0,0.0f,0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_line_strip[8]);
  }

  if(draw_hole_lines)
  {
    gl.color4f(0.0,0.0,1.0f,0.5f);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, _hole_strip);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[9]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[18]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[27]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[36]);
    gl.drawElements(GL_LINE_STRIP, 9, GL_UNSIGNED_SHORT, &_hole_strip[45]);
  }

  gl.enable(GL_LIGHTING);
  gl.color4f(1,1,1,1);
}

bool MapChunk::is_visible ( const float& cull_distance
                          , const Frustum& frustum
                          , const ::math::vector_3d& camera
                          ) const
{
  static const float chunk_radius = std::sqrt (CHUNKSIZE * CHUNKSIZE / 2.0f);

  return frustum.intersects (vmin, vmax)
      && (((camera - vcenter).length() - chunk_radius) < cull_distance);
}

void MapChunk::draw ( opengl::scoped::use_program& shader
                    , const boost::optional<selection_type>& selected_item
                    )
{
  shader.sampler ("shadow_map", GL_TEXTURE_2D, GL_TEXTURE0, shadow);

  shader.uniform ("area_id_color", areaIDColors[header.areaid]);

  shader.uniform ("is_impassable_chunk", !!(header.flags & FLAG_IMPASS));

  std::vector<int> texture_indices;
  std::vector<int> alphamap_indices;

  for (int i = 0; i < textures.num(); ++i)
  {
    texture_indices.push_back(i + 1);
    textures.bindTexture(i, i + 1);

    if (i == 0) continue;

    alphamap_indices.push_back(i + 5);
    textures.bindAlphamap(i - 1, i + 5);
  }

  shader.uniform("textures", texture_indices);
  shader.uniform("alphamaps", alphamap_indices);
  shader.uniform("layer_count", int (textures.num()));

  shader.attrib ("position", mVertices);
  shader.attrib ("normal", mNormals);

  if (selected_item && noggit::selection::is_the_same_as (this, *selected_item))
  {
    shader.uniform ("selected_triangle_id", noggit::selection::selected_polygon (*selected_item));
  }
  else
  {
    shader.uniform ("selected_triangle_id", -1);
  }

  gl.bindBuffer (GL_ELEMENT_ARRAY_BUFFER, indices);
  gl.drawElements (GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
}

void MapChunk::intersect(math::ray ray, selection_result& results)
{
  auto distance = math::intersect_bounds (ray, vmin, vmax);

  if (!distance)
    return;

  for (int i = 0; i < striplen; i += 3)
  {
    math::vector_3d const v0 (mVertices[strip[i    ]]);
    math::vector_3d const v1 (mVertices[strip[i + 1]]);
    math::vector_3d const v2 (mVertices[strip[i + 2]]);

    if ((distance = math::intersect_triangle (ray, v0, v1, v2)))
    {
      results.emplace_back (*distance, selected_chunk_type (this, i / 3, ray.position (*distance)));
    }
  }
}

void MapChunk::update_normal_vectors()
{
  static const float point_offset (UNITSIZE * 0.5f);

  for (size_t i (0); i < mapbufsize; ++i)
  {
    const ::math::vector_3d point_1 ( -point_offset
                                    , _world->get_height ( mVertices[i].x()
                                                         , mVertices[i].z()
                                                         )
                                    .get_value_or (mVertices[i].y())
                                    , -point_offset
                                    );

    const ::math::vector_3d point_2 ( point_offset
                                    , _world->get_height ( mVertices[i].x()
                                                         , mVertices[i].z()
                                                         )
                                    .get_value_or (mVertices[i].y())
                                    , -point_offset
                                    );

    const ::math::vector_3d point_3 ( point_offset
                                    , _world->get_height ( mVertices[i].x()
                                                         , mVertices[i].z()
                                                         )
                                    .get_value_or (mVertices[i].y())
                                    , point_offset
                                    );

    const ::math::vector_3d point_4 ( -point_offset
                                    , _world->get_height ( mVertices[i].x()
                                                         , mVertices[i].z()
                                                         )
                                    .get_value_or (mVertices[i].y())
                                    , point_offset
                                    );

    mNormals[i] = ( (point_2 % point_1)
                  + (point_3 % point_2)
                  + (point_4 % point_3)
                  + (point_1 % point_4)
                  ).normalized();
  }

  gl.bindBuffer (GL_ARRAY_BUFFER, normals);
  gl.bufferData ( GL_ARRAY_BUFFER
               , sizeof(mNormals)
               , mNormals
               , GL_STATIC_DRAW
               );

  for (size_t j (0); j < mapbufsize; ++j)
  {
    mFakeShadows[j].x (0.0f);
    mFakeShadows[j].y (0.0f);
    mFakeShadows[j].z (0.0f);
    mFakeShadows[j].w ( qBound ( 0.0f
                               , 1.0f - (-mNormals[j].x()
                                        + mNormals[j].y()
                                        - mNormals[j].z()
                                        )
                               , 1.0f
                               )
                      * 0.5f
                      );
  }

  gl.bindBuffer (GL_ARRAY_BUFFER, minishadows);
  gl.bufferData ( GL_ARRAY_BUFFER
               , sizeof(mFakeShadows)
               , mFakeShadows
               , GL_STATIC_DRAW
               );
}

bool MapChunk::changeTerrain(float x, float z, float change, float radius, int BrushType)
{
  float dist,xdiff,zdiff;

  bool Changed=false;

  xdiff = xbase - x + CHUNKSIZE/2;
  zdiff = zbase - z + CHUNKSIZE/2;
  dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);

  if(dist > (radius + noggit::MAPCHUNK_RADIUS))
    return false;

  vmin.y (9999999.0f);
  vmax.y (-9999999.0f);
  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x() - x;
    zdiff = mVertices[i].z() - z;
    if(BrushType == 5){
      if((std::abs(xdiff) < std::abs(radius/2)) && (std::abs(zdiff) < std::abs(radius/2))){
        mVertices[i].y (mVertices[i].y() + change);
        Changed=true;
      }
    }
    else
    {
      dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);
      if(dist < radius)
      {

        if(BrushType==0)//Flat
          mVertices[i].y (mVertices[i].y() + change);

        else if(BrushType==1)//Linear
          mVertices[i].y (mVertices[i].y() + change*(1.0f - dist/radius));

        else if(BrushType==2)//Smooth
          mVertices[i].y (mVertices[i].y() + change/(1.0f + dist/radius));

        else if (BrushType == 3) //x^2
          mVertices[i].y (mVertices[i].y() + change*( (dist/radius)*(dist/radius) + dist/radius + 1.0f));

        else if (BrushType == 4) //cos
          mVertices[i].y (mVertices[i].y() + change*cos(dist/radius));

        Changed=true;
      }
    }

    vmin.y (std::min(vmin.y(), mVertices[i].y()));
    vmax.y (std::max(vmax.y(), mVertices[i].y()));
  }
  if(Changed)
  {
    gl.bindBuffer(GL_ARRAY_BUFFER, vertices);
    gl.bufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return Changed;
}


bool MapChunk::flattenTerrain(float x, float z, float h, float remain, float radius, int BrushType)
{
  float dist,xdiff,zdiff,nremain;
  bool Changed=false;

  xdiff= xbase - x + CHUNKSIZE/2;
  zdiff= zbase - z + CHUNKSIZE/2;
  dist= std::sqrt(xdiff*xdiff + zdiff*zdiff);

  if(dist > (radius + noggit::MAPCHUNK_RADIUS))
    return false;

  vmin.y (9999999.0f);
  vmax.y (-9999999.0f);

  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff = mVertices[i].x() - x;
    zdiff = mVertices[i].z() - z;

    dist=std::sqrt(xdiff*xdiff + zdiff*zdiff);

    if(dist < radius)
    {
      if(BrushType==0)//Flat
      {
        mVertices[i].y (remain*mVertices[i].y() + (1 - remain)*h);
      }
      else if(BrushType==1)//Linear
      {
        nremain = 1 - (1 - remain) * (1 - dist/radius);
        mVertices[i].y (nremain*mVertices[i].y() + (1-nremain)*h);
      }
      else if(BrushType==2)//Smooth
      {
        nremain = 1.0f - pow(1.0f - remain, (1.0f + dist/radius));
        mVertices[i].y (nremain*mVertices[i].y() + (1 - nremain)*h);
      }

      Changed=true;
    }

    vmin.y (std::min(vmin.y(), mVertices[i].y()));
    vmax.y (std::max(vmax.y(), mVertices[i].y()));
  }
  if(Changed)
  {
    gl.bindBuffer(GL_ARRAY_BUFFER, vertices);
    gl.bufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return Changed;
}

bool MapChunk::blurTerrain(float x, float z, float remain, float radius, int BrushType)
{
  float dist,dist2,xdiff,zdiff,nremain;
  bool Changed = false;

  xdiff = xbase - x + CHUNKSIZE/2;
  zdiff = zbase - z + CHUNKSIZE/2;
  dist = std::sqrt(xdiff*xdiff + zdiff*zdiff);

  if(dist > (radius + noggit::MAPCHUNK_RADIUS) )
    return false;

  vmin.y (9999999.0f);
  vmax.y (-9999999.0f);

  for(int i=0; i < mapbufsize; ++i)
  {
    xdiff= mVertices[i].x() - x;
    zdiff= mVertices[i].z() - z;

    dist= std::sqrt(xdiff*xdiff + zdiff*zdiff);

    if(dist < radius)
    {
      float TotalHeight;
      float TotalWeight;
      float tx,tz, h;
      int Rad=(radius/UNITSIZE);

      TotalHeight=0;
      TotalWeight=0;
      for(int j= -Rad*2; j <= Rad*2; ++j)
      {
        tz= z + j * UNITSIZE/2;
        for(int k=-Rad; k <= Rad; ++k)
        {
          tx= x + k*UNITSIZE + (j%2) * UNITSIZE/2.0f;
          xdiff= tx - mVertices[i].x();
          zdiff= tz - mVertices[i].z();
          dist2= std::sqrt(xdiff*xdiff + zdiff*zdiff);
          if(dist2 > radius)
            continue;

          const boost::optional<float> height
            (_world->get_height (tx, tz));
          if (height)
          {
            TotalHeight += (1.0f - dist2/radius) * *height;
            TotalWeight += (1.0f - dist2/radius);
          }
        }
      }

      h=TotalHeight/TotalWeight;

      if(BrushType==0)//Flat
      {
        mVertices[i].y (remain * mVertices[i].y() + (1 - remain) * h);
      }
      else if(BrushType==1)//Linear
      {
        nremain= 1 - (1 - remain) * (1 - dist/radius);
        mVertices[i].y (nremain * mVertices[i].y() + ( 1 - nremain) * h);
      }
      else if(BrushType==2)//Smooth
      {
        nremain= 1.0f - pow( 1.0f - remain , (1.0f + dist/radius) );
        mVertices[i].y (nremain*mVertices[i].y() + (1-nremain)*h);
      }

      Changed=true;
    }

    vmin.y (std::min(vmin.y(), mVertices[i].y()));
    vmax.y (std::max(vmax.y(), mVertices[i].y()));
  }
  if(Changed)
  {
    gl.bindBuffer(GL_ARRAY_BUFFER, vertices);
    gl.bufferData(GL_ARRAY_BUFFER, sizeof(mVertices), mVertices, GL_STATIC_DRAW);
  }
  return Changed;
}

/* The correct way to do everything
Visibility = (1-Alpha above)*Alpha

Objective is Visibility = level

if (not bottom texture)
  New Alpha = Pressure*Level+(1-Pressure)*Alpha;
  New Alpha Above = (1-Pressure)*Alpha Above;
else Bottom Texture
  New Alpha Above = Pressure*(1-Level)+(1-Pressure)*Alpha Above

For bottom texture with multiple above textures

For 2 textures above
x,y = current alphas
u,v = target alphas
v=std::sqrt((1-level)*y/x)
u=(1-level)/v

For 3 textures above
x,y,z = current alphas
u,v,w = target alphas
L=(1-Level)
u=pow(L*x*x/(y*y),1.0f/3.0f)
w=std::sqrt(L*z/(u*y))
*/
void MapChunk::eraseTextures()
{
  textures.eraseTextures();
}

int MapChunk::addTexture( noggit::scoped_blp_texture_reference texture )
{
  return textures.addTexture(texture);
}
void MapChunk::switchTexture( noggit::scoped_blp_texture_reference oldTexture, noggit::scoped_blp_texture_reference newTexture )
{
  textures.switchTexture(oldTexture, newTexture);
}
bool MapChunk::paintTexture( float x, float z, const brush& Brush, float strength, float pressure, noggit::scoped_blp_texture_reference texture )
{
  return textures.paintTexture(xbase, zbase, x, z, Brush, strength, pressure, texture);
}

bool MapChunk::isHole( int i, int j )
{
  return( holes & ( ( 1 << ((j*4)+i) ) ));
}

void MapChunk::addHole( int i, int j )
{
  holes = holes | ( ( 1 << ((j*4)+i)) );
  initStrip();
}

void MapChunk::removeHole( int i, int j )
{
  holes = holes & ~( ( 1 << ((j*4)+i)) );
  initStrip();
}
void MapChunk::make_all_holes()
{
  holes = 0xffff;
  initStrip();
}
void MapChunk::remove_all_holes()
{
  holes = 0;
  initStrip();
}

void MapChunk::setAreaID( int ID )
{
  header.areaid = ID;
}

int MapChunk::getAreaID(){
  return header.areaid;
}


void MapChunk::setFlag( bool on_or_off, int flag)
{
  if(on_or_off)
    header.flags |= flag;
  else
    header.flags &= ~(flag);
}

void MapChunk::update_low_quality_texture_map()
{
  for (size_t y (0); y < 8; ++y)
  {
    for (size_t x (0); x < 8; ++x)
    {
      size_t winning_layer (0);
      for (size_t layer (1); layer < textures.num(); ++layer)
      {
        size_t sum (0);
        for (size_t j (0); j < 8; ++j)
        {
          for (size_t i (0); i < 8; ++i)
          {
            sum += textures.getAlpha(layer, (y * 8 + j) * 64 + (x * 8 + i));
          }
        }
        sum /= 8 * 8;

        const size_t minimum_value_to_overwrite (128 / layer);

        if (sum > minimum_value_to_overwrite)
        {
          winning_layer = layer;
        }
      }

      const size_t array_index ((y * 8 + x) / 4);
      const size_t bit_index (((y * 8 + x) % 4) * 2);

      header.low_quality_texture_map[array_index]
        |= ((winning_layer & 3) << bit_index);
    }
  }
}

const unsigned char* MapChunk::low_quality_texture_map() const
{
  return header.low_quality_texture_map;
}
