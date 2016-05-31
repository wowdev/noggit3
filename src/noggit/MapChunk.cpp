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
#include <noggit/blp_texture.h>
#include <noggit/Brush.h>
#include <noggit/Frustum.h> // Frustum
#include <noggit/Liquid.h>
#include <noggit/Log.h>
#include <noggit/MapHeaders.h>
#include <noggit/Selection.h>
#include <noggit/World.h>
#include <noggit/mpq/file.h>

static const double MAPCHUNK_RADIUS = 47.140452079103168293389624140323; //std::sqrt((533.33333/16)^2 + (533.33333/16)^2)
static const int CONTOUR_WIDTH = 128;
static const float texDetail = 8.0f;
static const float TEX_RANGE = 62.0f / 64.0f;


const int stripsize = 8*18 + 7*2;

namespace
  {
  StripType *mapstrip;
  StripType *mapstrip2;

  int indexMapBuf (int x, int y)
  {
    return ((y+1)/2)*9 + (y/2)*8 + x;
  }

  int indexLoD(int x, int y)
  {
    return (x+1)*9+x*8+y;
  }

  int indexNoLoD(int x, int y)
  {
    return x*8+x*9+y;
  }

  // 8x8x2 version with triangle strips, size = 8*18 + 7*2
  template <class V>
  void stripify(V *in, V *out)
  {
    for (int row=0; row<8; row++) {
      V *thisrow = &in[indexMapBuf(0,row*2)];
      V *nextrow = &in[indexMapBuf(0,(row+1)*2)];

      if (row>0) *out++ = thisrow[0];
      for (int col=0; col<9; col++) {
        *out++ = thisrow[col];
        *out++ = nextrow[col];
      }
      if (row<7) *out++ = nextrow[8];
    }
  }

  // high res version, size = 16*18 + 7*2 + 8*2
  const int stripsize2 = 16*18 + 7*2 + 8*2;
  template <class V>
  void stripify2(V *in, V *out)
  {
    for (int row=0; row<8; row++) {
      V *thisrow = &in[indexMapBuf(0,row*2)];
      V *nextrow = &in[indexMapBuf(0,row*2+1)];
      V *overrow = &in[indexMapBuf(0,(row+1)*2)];

      if (row>0) *out++ = thisrow[0];// jump end
      for (int col=0; col<8; col++) {
        *out++ = thisrow[col];
        *out++ = nextrow[col];
      }
      *out++ = thisrow[8];
      *out++ = overrow[8];
      *out++ = overrow[8];// jump start
      *out++ = thisrow[0];// jump end
      *out++ = thisrow[0];
      for (int col=0; col<8; col++) {
        *out++ = overrow[col];
        *out++ = nextrow[col];
      }
      if (row<8) *out++ = overrow[8];
      if (row<7) *out++ = overrow[8];// jump start
    }
  }

  void init_map_strip()
  {
    // default strip indices
    StripType *defstrip = new StripType[stripsize];
    for (int i=0; i<stripsize; ++i) defstrip[i] = i; // note: this is ugly and should be handled in stripify
    mapstrip = new StripType[stripsize];
    stripify<StripType>(defstrip, mapstrip);
    delete[] defstrip;

    defstrip = new StripType[stripsize2];
    for (int i=0; i<stripsize2; ++i) defstrip[i] = i; // note: this is ugly and should be handled in stripify
    mapstrip2 = new StripType[stripsize2];
    stripify2<StripType>(defstrip, mapstrip2);
    delete[] defstrip;
  }
}

void MapChunk::GenerateContourMap()
{
  gl.enable(GL_TEXTURE_2D);

  unsigned char  CTexture[CONTOUR_WIDTH*4];

  _contour_coord_gen[0]=0.0f;
  _contour_coord_gen[1]=0.25f;
  _contour_coord_gen[2]=0.0f;
  _contour_coord_gen[3]=0.0f;


  for(int i=0;i<(CONTOUR_WIDTH*4);++i)
    CTexture[i]=0;
  CTexture[3+CONTOUR_WIDTH/2]=0xff;
  CTexture[7+CONTOUR_WIDTH/2]=0xff;
  CTexture[11+CONTOUR_WIDTH/2]=0xff;

  gl.genTextures(1, &_contour_texture);
  gl.bindTexture(GL_TEXTURE_2D, _contour_texture);

  gl.texImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, CONTOUR_WIDTH, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, CTexture);
  gl.generateMipmap (GL_TEXTURE_2D);

  gl.enable(GL_TEXTURE_GEN_S);
  gl.texGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
  gl.texGenfv(GL_S,GL_OBJECT_PLANE,_contour_coord_gen);

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

  gl.disable(GL_TEXTURE_GEN_S);
  gl.disable(GL_TEXTURE_2D);
}

//! \note I am aware of this being global state not even being inside a class BUT I DONT CARE AT ALL. THIS WHOLE THING IS BULLSHIT AND NOT WORTH A BIT AND COSTING ME TIME OF MY LIFE FOR JUST BEING BAD AS FUCK. REMOVEING ENVIRONMENT TOOK ME HOURS WHILE MOST VARIABLES HAD BEEN ONLY USED IN A SINGLE FUCKING CLASS,  DID NOT HAVE MEANINGFUL NAMES OR ANYTHING.
namespace
{
  QMap<int, ::math::vector_3d> areaIDColors;
}

MapChunk::MapChunk(World* world, MapTile* maintile, noggit::mpq::file* f,bool bigAlpha)
  : _world (world)
  , _contour_texture (0)
{
  CreateStrips();
  init_map_strip();

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

    for (size_t i = 0; i < (size / 16U); ++i) {
      f->read(&tex[i], 4);
      f->read(&_texFlags[i], 4);
      f->read(&MCALoffset[i], 4);
      f->read(&_effectID[i], 4);

      if (texture_flags(i) & FLAG_ANIMATE)
      {
        animated[i] = texture_flags(i);
      }
      else {
        animated[i] = 0;
      }
      _textures.emplace_back(maintile->mTextureFilenames[tex[i]]);
    }
  }
  // - MCSH ----------------------------------------------
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

    gl.genTextures(3, alphamaps);
    size_t MCALbase = f->getPos();

    for (unsigned int layer = 0; layer < header.nLayers; ++layer)
    {
      if (texture_flags(layer) & 0x100)
      {
        uint8_t* const alpha_map(&amap[layer - 1][0]);
        uint8_t const* input(f->get<uint8_t>(MCALbase + MCALoffset[layer]));

        memset(alpha_map, 255, 64 * 64);

        if (texture_flags(layer) & 0x200)
        {
          for (std::size_t offset_output(0); offset_output < 4096;)
          {
            bool const fill(*input & 0x80);
            std::size_t const n(*input & 0x7F);
            ++input;

            if (fill)
            {
              memset(&alpha_map[offset_output], *input, n);
              ++input;
            }
            else
            {
              memcpy(&alpha_map[offset_output], input, n);
              input += n;
            }

            offset_output += n;
          }
        }
        else if (bigAlpha)
        {
          memcpy(alpha_map, input, 64 * 64);
        }
        else
        {
          for (std::size_t x(0); x < 64; ++x)
          {
            for (std::size_t y(0); y < 64; y += 2)
            {
              alpha_map[x * 64 + y + 0] = ((*input & 0x0f) << 4) | 0xf;
              alpha_map[x * 64 + y + 1] = ((*input & 0xf0) << 0) | 0xf;
              ++input;
            }
          }
        }

        if (!(header.flags & FLAG_do_not_fix_alpha_map))
        {
          for (std::size_t i(0); i < 64; ++i)
          {
            alpha_map[i * 64 + 63] = alpha_map[i * 64 + 62];
            alpha_map[63 * 64 + i] = alpha_map[62 * 64 + i];
          }
          alpha_map[63 * 64 + 63] = alpha_map[62 * 64 + 62];
        }

        gl.bindTexture(GL_TEXTURE_2D, alphamaps[layer - 1]);
        gl.texImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, alpha_map);
        gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }
  }

  vcenter = (vmin + vmax) * 0.5f;
  nameID = _world->selection_names().add(this);

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

  GenerateContourMap();
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

  if(_textures.size() > 0U)
  {
    opengl::texture::enable_texture (0);

    _textures[0]->bind();

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
    (mcly_flags_type::interpret (animated[0]));

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

  if (_textures.size() > 1U) {
    //gl.depthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    //gl.depthMask(GL_FALSE);
  }
  for(size_t i=1; i < _textures.size(); ++i)
  {
    opengl::texture::enable_texture (0);

    _textures[i]->bind();

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    opengl::texture::enable_texture (1);

    gl.bindTexture(GL_TEXTURE_2D, alphamaps[i-1]);

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const mcly_flags_type& flags (mcly_flags_type::interpret (animated[i]));

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

  gl.drawElements(GL_TRIANGLE_STRIP, stripsize2, GL_UNSIGNED_SHORT, mapstrip2);
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
  // unload alpha maps
  gl.deleteTextures( 3, alphamaps );
  // shadow maps, too
  gl.deleteTextures( 1, &shadow );

  // delete VBOs
  gl.deleteBuffers( 1, &vertices );
  gl.deleteBuffers( 1, &normals );

  //delete IBO
  gl.deleteBuffers( 1, &indices );

  delete strip;
  strip = nullptr;

  if( nameID != -1 )
  {
    _world->selection_names().del( nameID );
    nameID = -1;
  }
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
  float min = mVertices[indexNoLoD(0, 0)].y();

  for (int j = 0; j < 9; ++j)
  {
    for (int i = 0; i < 9; ++i)
    {
      min = std::min(mVertices[indexNoLoD(i, j)].y(), min);
    }
  }

	return min;
}

void MapChunk::drawPass (int anim) const
{
  const mcly_flags_type& flags (mcly_flags_type::interpret (anim));

  texture_animation_setup const texture_animation (flags);

  gl.drawElements(GL_TRIANGLES, striplen, GL_UNSIGNED_SHORT, nullptr);
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

void MapChunk::drawContour() const
{
  opengl::scoped::texture_setter<0, GL_TRUE> const texture_0;
  gl.bindTexture (GL_TEXTURE_2D, _contour_texture);

  opengl::scoped::bool_setter<GL_BLEND, GL_TRUE> const blend;
  gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  opengl::scoped::bool_setter<GL_ALPHA_TEST, GL_TRUE> const alpha_test;

  opengl::scoped::bool_setter<GL_TEXTURE_GEN_S, GL_TRUE> const texture_gen_s;
  gl.texGeni (GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
  gl.texGenfv (GL_S, GL_OBJECT_PLANE, _contour_coord_gen);

  gl.color4f (1.0f, 1.0f, 1.0f, 1.0f);

  drawPass (0);
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

void MapChunk::draw ( bool draw_terrain_height_contour
                    , bool mark_impassable_chunks
                    , bool draw_area_id_overlay
                    , bool dont_draw_cursor
                    , const Skies* skies
                    , const boost::optional<selection_type>& selected_item
                    )
{
  // setup vertex buffers
  gl.bindBuffer(GL_ARRAY_BUFFER, vertices);
  gl.vertexPointer(3, GL_FLOAT, 0, 0);
  gl.bindBuffer(GL_ARRAY_BUFFER, normals);
  gl.normalPointer(GL_FLOAT, 0, 0);
  gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
  // ASSUME: texture coordinates set up already


  // first pass: base texture
  if (_textures.empty())
  {
    opengl::texture::disable_texture (0);
    opengl::texture::disable_texture (1);

    gl.color3f(1.0f,1.0f,1.0f);
  }
  else
  {
    opengl::texture::enable_texture (0);

    _textures[0]->bind();

    opengl::texture::disable_texture (1);
  }

  gl.enable(GL_LIGHTING);
  drawPass(animated[0]);

  if (_textures.size() > 1U) {
    //gl.depthFunc(GL_EQUAL); // GL_LEQUAL is fine too...?
    gl.depthMask(GL_FALSE);
  }

  // additional passes: if required
  for( size_t i = 1; i < _textures.size(); ++i )
  {
    opengl::texture::enable_texture (0);

    _textures[i]->bind();

    // this time, use blending:
    opengl::texture::enable_texture (1);

    gl.bindTexture( GL_TEXTURE_2D, alphamaps[i - 1] );

    drawPass(animated[i]);
  }

  if (_textures.size() > 1U) {
    //gl.depthFunc(GL_LEQUAL);
    gl.depthMask(GL_TRUE);
  }

  // shadow map
  gl.activeTexture(GL_TEXTURE0);
  gl.disable(GL_TEXTURE_2D);
  gl.disable(GL_LIGHTING);

  ::math::vector_3d shc = skies->colorSet[WATER_COLOR_DARK] * 0.3f;
  gl.color4f(shc.x(),shc.y(),shc.z(),1);

  //gl.color4f(1,1,1,1);

  gl.activeTexture(GL_TEXTURE1);
  gl.bindTexture(GL_TEXTURE_2D, shadow);
  gl.enable(GL_TEXTURE_2D);

  drawPass (0);

  gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  gl.disable(GL_TEXTURE_2D);
  gl.disable(GL_LIGHTING);

  if (draw_terrain_height_contour)
  {
    drawContour();
  }

  if (mark_impassable_chunks && (header.flags & FLAG_IMPASS))
  {
    gl.color4f (1.0f, 1.0f, 1.0f, 0.6f);
    drawPass (0);
  }

  if (draw_area_id_overlay)
  {
    gl.color4f ( areaIDColors[header.areaid].x()
              , areaIDColors[header.areaid].y()
              , areaIDColors[header.areaid].z()
              , 0.7f
              );
    drawPass (0);
  }

  //! \todo This actually should be an enum. And should be passed into this method.
  if ( !dont_draw_cursor
    && noggit::app().setting ("cursor/type", 1).toInt() == 3
    && selected_item
    && noggit::selection::is_the_same_as (this, *selected_item)
     )
  {
    const int selected_polygon
      (noggit::selection::selected_polygon (*selected_item));

    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull_face;
    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> const depth_test;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    gl.begin( GL_TRIANGLES );
    gl.color4f( 1.0f, 1.0f, 0.0f, 1.0f );
    gl.vertex3fv( mVertices[mapstrip2[selected_polygon + 0]] );
    gl.vertex3fv( mVertices[mapstrip2[selected_polygon + 1]] );
    gl.vertex3fv( mVertices[mapstrip2[selected_polygon + 2]] );
    gl.end();
  }


  gl.color4f( 1.0f, 1.0f, 1.0f, 1.0f );

  gl.enable( GL_LIGHTING );
  gl.color4f( 1.0f, 1.0f, 1.0f, 1.0f );

  /*
  //////////////////////////////////
  // debugging tile flags:
  GLfloat tcols[8][4] = {  {1,1,1,1},
    {1,0,0,1}, {1, 0.5f, 0, 1}, {1, 1, 0, 1},
    {0,1,0,1}, {0,1,1,1}, {0,0,1,1}, {0.8f, 0, 1,1}
  };
  opengl::scoped::matrix_pusher const matrix_pusher;
  gl.disable(GL_CULL_FACE);
  gl.disable(GL_TEXTURE_2D);
  gl.translatef(xbase, ybase, zbase);
  for (int i=0; i<8; ++i) {
    int v = 1 << (7-i);
    for (int j=0; j<4; j++) {
      if (animated[j] & v) {
        gl.begin(GL_TRIANGLES);
        gl.color4fv(tcols[i]);

        gl.vertex3f(i*2.0f, 2.0f, j*2.0f);
        gl.vertex3f(i*2.0f+1.0f, 2.0f, j*2.0f);
        gl.vertex3f(i*2.0f+0.5f, 4.0f, j*2.0f);

        gl.end();
      }
    }
  }
  gl.enable(GL_TEXTURE_2D);
  gl.enable(GL_CULL_FACE);
  gl.color4f(1,1,1,1);*/
}

void MapChunk::drawSelect()
{
  if( nameID == -1 )
    nameID = _world->selection_names().add( this );

  //! \todo Use backface culling again? Maybe this adds problems. Idk.
  // opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull_face;

  opengl::scoped::name_pusher const name_pusher (nameID);

  for( int i = 0; i < stripsize2 - 2; ++i )
  {
    opengl::scoped::name_pusher const inner_name_pusher (i);
    gl.begin( GL_TRIANGLES );
    gl.vertex3fv( mVertices[mapstrip2[i]] );
    gl.vertex3fv( mVertices[mapstrip2[i + 1]] );
    gl.vertex3fv( mVertices[mapstrip2[i + 2]] );
    gl.end();
  }
}

void MapChunk::getSelectionCoord ( const int& selected_polygon
                                 , float* x
                                 , float* z
                                 ) const
{
  if (selected_polygon + 2 >= stripsize2)
  {
    LogError << "getSelectionCoord() fucked up because the selection was bad. "
             << selected_polygon
             << " with stripsize2 of "
             << stripsize2
             << ".\n";
    //! \todo Return none, instead of some weird constant.
    *x = -1000000.0f;
    *z = -1000000.0f;
    return;
  }

  *x = ( mVertices[mapstrip2[selected_polygon + 0]].x()
       + mVertices[mapstrip2[selected_polygon + 1]].x()
       + mVertices[mapstrip2[selected_polygon + 2]].x()
       )
     / 3.0f;
  *z = ( mVertices[mapstrip2[selected_polygon + 0]].z()
       + mVertices[mapstrip2[selected_polygon + 1]].z()
       + mVertices[mapstrip2[selected_polygon + 2]].z()
       )
     / 3.0f;
}

float MapChunk::getSelectionHeight (const int& selected_polygon) const
{
  if (selected_polygon + 2 >= stripsize2)
  {
    LogError << "getSelectionHeight() fucked up because the selection was bad. "
             << selected_polygon
             << " with stripsize2 of "
             << stripsize2
             << ".\n";
    //! \todo Return none, instead of some weird constant.
    return -1000000.0f;
  }

  return ( mVertices[mapstrip2[selected_polygon + 0]].y()
         + mVertices[mapstrip2[selected_polygon + 1]].y()
         + mVertices[mapstrip2[selected_polygon + 2]].y()
         )
         / 3.0f;
}

::math::vector_3d MapChunk::GetSelectionPosition (const int& selected_polygon) const
{
  if (selected_polygon + 2 >= stripsize2)
  {
    LogError << "GetSelectionPosition() fucked up because the selection was bad. "
             << selected_polygon
             << " with stripsize2 of "
             << stripsize2
             << ".\n";
    //! \todo Return none, instead of some weird constant.
    return ::math::vector_3d (-1000000.0f, -1000000.0f, -1000000.0f);
  }

  return ( ::math::vector_3d( mVertices[mapstrip2[selected_polygon + 0]] )
         + ::math::vector_3d( mVertices[mapstrip2[selected_polygon + 1]] )
         + ::math::vector_3d( mVertices[mapstrip2[selected_polygon + 2]] )
         )
         * (1.0f / 3.0f);
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

  if(dist > (radius + MAPCHUNK_RADIUS))
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

  if(dist > (radius + MAPCHUNK_RADIUS))
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

  if(dist > (radius + MAPCHUNK_RADIUS) )
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
  _textures.clear();
}

int MapChunk::addTexture( noggit::scoped_blp_texture_reference texture )
{
  int texLevel = -1;
  if( _textures.size() < 4U )
  {
    texLevel = _textures.size();
    _textures.emplace_back (texture);
    animated[texLevel] = 0;
    texture_flags (texLevel, 0);
    texture_effect_id (texLevel, 0);
    if( texLevel )
    {
      if( alphamaps[texLevel-1] < 1 )
      {
        LogError << "Alpha Map has invalid texture binding" << std::endl;
        _textures.pop_back();
        return -1;
      }
      memset( amap[texLevel - 1], 0, 64 * 64 );
      gl.bindTexture( GL_TEXTURE_2D, alphamaps[texLevel - 1] );
      gl.texImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[texLevel - 1] );
      gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      gl.texParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    }
  }
  return texLevel;
}
void MapChunk::switchTexture( noggit::scoped_blp_texture_reference oldTexture, noggit::scoped_blp_texture_reference newTexture )
{
  int texLevel = -1;
  for (size_t i = 0;i < _textures.size();++i)
  {
    // prevent texture duplication
    if (_textures[i] == newTexture)
      return;
    if (_textures[i] == oldTexture)
      texLevel = i;
  }

  if(texLevel != -1)
  {
  _textures[texLevel] = newTexture;
  }
}
bool MapChunk::paintTexture( float x, float z, const brush& Brush, float strength, float pressure, noggit::scoped_blp_texture_reference texture )
{
#if 1
  float zPos,xPos,change,xdiff,zdiff,dist, radius;

  int texLevel=-1;

  radius=Brush.getRadius();

  xdiff= xbase - x + CHUNKSIZE/2;
  zdiff= zbase - z + CHUNKSIZE/2;
  dist= std::sqrt( xdiff*xdiff + zdiff*zdiff );

  if( dist > (radius+MAPCHUNK_RADIUS) )
    return false;

  //First Lets find out do we have the texture already
  for(size_t i=0;i<_textures.size();++i)
    if(_textures[i]==texture)
      texLevel=i;


  if( (texLevel==-1) && (_textures.size()==4) )
  {
    // Implement here auto texture slot freeing :)
    LogDebug << "paintTexture: No free texture slot" << std::endl;
    return false;
  }

  //Only 1 layer and its that layer
  if( (texLevel!=-1) && (_textures.size()==1) )
    return true;


  change=CHUNKSIZE/62.0f;
  zPos=zbase;

  float target,tarAbove, tPressure;
  //int texAbove=_textures.size()-texLevel-1;


  for(int j=0; j < 63 ; j++)
  {
    xPos=xbase;
    for(int i=0; i < 63; ++i)
    {
      xdiff=xPos-x;
      zdiff=zPos-z;
      dist=std::abs(std::sqrt( xdiff*xdiff + zdiff*zdiff ));

      if(dist>radius)
      {
        xPos+=change;
        continue;
      }

      if(texLevel==-1)
      {
        texLevel=addTexture(texture);
        if(texLevel==0)
          return true;
        if(texLevel==-1)
        {
          LogDebug << "paintTexture: Unable to add texture." << std::endl;
          return false;
        }
      }

      target=strength;
      tarAbove=1-target;

      tPressure=pressure*Brush.getValue(dist);

      if(texLevel>0)
      {
        uchar test = static_cast<unsigned char>(std::max( std::min( (1.0f-tPressure)*( static_cast<float>(amap[texLevel-1][i+j*64]) ) + tPressure*target + 0.5f ,255.0f) , 0.0f));
        amap[texLevel-1][i+j*64]=test;
      }
      for(size_t k=texLevel;k<_textures.size()-1;k++)
        amap[k][i+j*64]=static_cast<unsigned char>(std::max( std::min( (1.0f-tPressure)*( static_cast<float>(amap[k][i+j*64]) ) + tPressure*tarAbove + 0.5f ,255.0f) , 0.0f));
      xPos+=change;
    }
    zPos+=change;
  }

  if( texLevel == -1 )
  {
    LogDebug << "Somehow no texture got painted." << std::endl;
    return false;
  }

  for( size_t j = texLevel; j < _textures.size() - 1; j++ )
  {
    if( j > 2 )
    {
      LogError << "WTF how did you get here??? Get a cookie." << std::endl;
      continue;
    }
    gl.bindTexture( GL_TEXTURE_2D, alphamaps[j] );
    gl.texImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j] );
  }

  if( texLevel )
  {
    gl.bindTexture( GL_TEXTURE_2D, alphamaps[texLevel - 1] );
    gl.texImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[texLevel - 1] );
  }
#else
  // new stuff from bernd.
  // need to get rework. Add old code with switch that the guys out there can use paint.
  const float radius = Brush.getRadius();

  // Are we really painting on this chunk?
  const float xdiff = xbase + CHUNKSIZE / 2 - x;
  const float zdiff = zbase + CHUNKSIZE / 2 - z;

  if( ( xdiff * xdiff + zdiff * zdiff ) > ( MAPCHUNK_RADIUS+ radius ) * ( MAPCHUNK_RADIUS + radius ) )
  return false;


  // Search for empty layer.
  int texLevel = -1;

  for( size_t i = 0; i < _textures.size(); ++i )
  {
    if( _textures[i] == texture )
    {
      texLevel = i;
    }
   }

  if( texLevel == -1 )
  {

    if( _textures.size() == 4 )
    {
      for( size_t layer = 0; layer < _textures.size(); ++layer )
      {
        unsigned char map[64*64];
        if( layer )
          memcpy( map, amap[layer-1], 64*64 );
        else
          memset( map, 255, 64*64 );

        for( size_t layerAbove = layer + 1; layerAbove < _textures.size(); ++layerAbove )
        {
          unsigned char* above = amap[layerAbove-1];
          for( size_t i = 0; i < 64 * 64; ++i )
          {
            map[i] = std::max( 0, map[i] - above[i] );
          }
        }

        size_t sum = 0;
        for( size_t i = 0; i < 64 * 64; ++i )
        {
          sum += map[i];
        }

        if( !sum )
        {
          for( size_t i = layer; i < _textures.size() - 1; ++i )
          {
            _textures[i] = _textures[i+1];
            animated[i] = animated[i+1];
            texture_flags (i, texture_flags (i + 1));
            texture_effect_id (i, texture_effect_id (i + 1));
            if( i )
              memcpy( amap[i-1], amap[i], 64*64 );
          }

          for( size_t j = layer; j < _textures.size(); j++ )
              {
                gl.bindTexture( GL_TEXTURE_2D, alphamaps[j - 1] );
                gl.texImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1] );
              }
              _textures.pop_back();
        }
      }
    }

    if( _textures.size() == 4 )
    return false;

      texLevel = addTexture( texture );

  }
  else
  {
    if( _textures.size() == 1 )
      return true;
  }
  LogDebug << "TexLevel: " << texLevel << " -  _textures.size(): " << _textures.size() << "\n";
  // We now have a texture at texLevel > 0.
  static const float change = CHUNKSIZE / 62.0f; //! \todo 64? 63? 62? Wtf?

  if( texLevel == 0 )
    return true;

  for( size_t j = 0; j < 64; ++j )
  {
    for( size_t i = 0; i < 64; ++i )
    {
      const float xdiff_ = xbase + change * i - x;
      const float zdiff_ = zbase + change * j - z;
      const float dist = std::sqrt( xdiff_ * xdiff_ + zdiff_ * zdiff_ );

      if( dist <= radius )
      {
          amap[texLevel - 1][i + j * 64] = (unsigned char)( std::max( std::min( amap[texLevel - 1][i + j * 64] + pressure * strength * Brush.getValue( dist ) + 0.5f, 255.0f ), 0.0f ) );
      }
    }
  }


  // Redraw changed layers.

  for( size_t j = texLevel; j < _textures.size(); j++ )
  {
    gl.bindTexture( GL_TEXTURE_2D, alphamaps[j - 1] );
    gl.texImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1] );
  }
#endif

  return true;
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
      for (size_t layer (1); layer < _textures.size(); ++layer)
      {
        size_t sum (0);
        for (size_t j (0); j < 8; ++j)
        {
          for (size_t i (0); i < 8; ++i)
          {
            sum += amap[layer][(y * 8 + j) * 64 + (x * 8 + i)];
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
