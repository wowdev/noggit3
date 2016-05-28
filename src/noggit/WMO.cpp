// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/WMO.h>

#include <algorithm>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include <sstream>
#include <iomanip>
#include <iostream>

#include <math/vector_2d.h>

#include <opengl/call_list.h>
#include <opengl/context.h>
#include <opengl/primitives.h>

#include <noggit/async/loader.h>
#include <noggit/application.h>
#include <noggit/blp_texture.h>
#include <noggit/Frustum.h> // Frustum
#include <noggit/Liquid.h>
#include <noggit/Log.h> // LogDebug
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Shaders.h>
#include <noggit/World.h>
#include <noggit/mpq/file.h>

const std::string& WMO::filename() const
{
  return _filename;
}

WMO::WMO (const std::string& filenameArg, World* world)
  : _filename( filenameArg )
{
  noggit::mpq::file f (QString::fromStdString (_filename));

  uint32_t fourcc;
  uint32_t size;
  float ff[3];

  char *ddnames = nullptr;
  char *groupnames = nullptr;

  char *texbuf=0;

  while (!f.is_at_end_of_file()) {
    f.read(&fourcc,4);
    f.read(&size, 4);

    size_t nextpos = f.getPos() + size;

    if ( fourcc == 'MOHD' ) {
      unsigned int col;
      // header
      f.read(&nTextures, 4);
      f.read(&nGroups, 4);
      f.read(&nP, 4);
      f.read(&nLights, 4);
      f.read(&nModels, 4);
      f.read(&nDoodads, 4);
      f.read(&nDoodadSets, 4);
      f.read(&col, 4);
      f.read(&nX, 4);
      f.read(ff,12);
      extents[0] = ::math::vector_3d(ff[0],ff[1],ff[2]);
      f.read(ff,12);
      extents[1] = ::math::vector_3d(ff[0],ff[1],ff[2]);

      groups = new WMOGroup[nGroups];
    }
    else if ( fourcc == 'MOTX' ) {
      texbuf = new char[size];
      f.read(texbuf, size);
    }
    else if ( fourcc == 'MOMT' )
    {
      const size_t num_materials (size / 0x40);

      for (size_t i (0); i < num_materials; ++i)
      {
        SMOMaterial material;
        f.read (&material, sizeof (material));

        _materials.emplace_back (material, texbuf + material.nameStart);
      }
    }
    else if ( fourcc == 'MOGN' ) {
      groupnames = reinterpret_cast<char*>(f.getPointer());
    }
    else if ( fourcc == 'MOGI' ) {
      // group info - important information! ^_^
      for (unsigned int i=0; i<nGroups; ++i) {
        groups[i].init(this, &f, i, groupnames);

      }
    }
    else if ( fourcc == 'MOLT' ) {
      // Lights?
      for (unsigned int i=0; i<nLights; ++i) {
        WMOLight l;
        l.init(&f);
        lights.push_back(l);
      }
    }
    else if ( fourcc == 'MODN' ) {
      // models ...
      // MMID would be relative offsets for MMDX filenames
      if (size) {

        ddnames = reinterpret_cast<char*>( f.getPointer() );

        f.seekRelative(size);
      }
    }
    else if ( fourcc == 'MODS' ) {
      for (unsigned int i=0; i<nDoodadSets; ++i) {
        WMODoodadSet dds;
        f.read(&dds, 32);
        doodadsets.push_back(dds);
      }
    }
    else if ( fourcc == 'MODD' ) {
      nModels = size / 0x28;
      for (unsigned int i=0; i<nModels; ++i) {
        uint32_t ofs;
        f.read (&ofs, sizeof (uint32_t));

        ::math::vector_3d position;
        f.read (position, sizeof (position));

        ::math::vector_4d rotation;
        f.read (rotation, sizeof (rotation));

        float scale;
        f.read (&scale, sizeof(float));

        uint32_t light;
        f.read (&light, sizeof (uint32_t));

        modelis.emplace_back
          ( world
          , ddnames + ofs
          , ::math::vector_3d ( position.x()
                              , position.z()
                              , -position.y()
                              )
          , ::math::quaternion ( -rotation.w()
                               , rotation.y()
                               , rotation.z()
                               , rotation.x()
                               )
          , scale
          , ::math::vector_3d ( ((light & 0xff0000) >> 16) / 255.0f
                              , ((light & 0x00ff00) >> 8) / 255.0f
                              , ((light & 0x0000ff)) / 255.0f
                              )
          );
      }

    }
    else if ( fourcc == 'MOSB' )
    {
      if (size>4)
      {
        std::string path = std::string( reinterpret_cast<char*>( f.getPointer() ) );
        if (path.length())
        {
          LogDebug << "SKYBOX:" << std::endl;

          if( noggit::mpq::file::exists( QString::fromStdString (path) ) )
          {
            skybox = noggit::scoped_model_reference (path);
          }
        }
      }
    }
    else if ( fourcc == 'MOPV' ) {
      WMOPV p;
      for (unsigned int i=0; i<nP; ++i) {
        f.read(ff,12);
        p.a = ::math::vector_3d(ff[0],ff[2],-ff[1]);
        f.read(ff,12);
        p.b = ::math::vector_3d(ff[0],ff[2],-ff[1]);
        f.read(ff,12);
        p.c = ::math::vector_3d(ff[0],ff[2],-ff[1]);
        f.read(ff,12);
        p.d = ::math::vector_3d(ff[0],ff[2],-ff[1]);
        pvs.push_back(p);
      }
    }
    else if ( fourcc == 'MOPR' ) {
      int nn = size / 8;
      WMOPR *pr = reinterpret_cast<WMOPR*>(f.getPointer());
      for (int i=0; i<nn; ++i) {
        prs.push_back(*pr++);
      }
    }
    else if ( fourcc == 'MFOG' ) {
      int nfogs = size / 0x30;
      for (int i=0; i<nfogs; ++i) {
        WMOFog fog;
        fog.init(&f);
        fogs.push_back(fog);
      }
    }

    f.seek(nextpos);
  }

  f.close();

  delete[] texbuf;
  texbuf = nullptr;

  for (unsigned int i=0; i<nGroups; ++i)
    groups[i].initDisplayList();
}

WMO::~WMO()
{
  LogDebug << "Unloading WMO \"" << filename() << "\"." << std::endl;

  delete[] groups;
  groups = nullptr;
}

void WMO::draw ( World* world
               , int doodadset
               , const ::math::vector_3d &ofs
               , const float rot
               , const float culldistance
               , bool boundingbox
               , bool groupboxes
               , bool /*highlight*/
               , bool draw_doodads
               , bool draw_fog
               , bool hasSkies
               , const float& fog_distance
               , const Frustum& frustum
               , const ::math::vector_3d& camera
               ) const
{
  if (draw_fog)
    gl.enable( GL_FOG );
  else
    gl.disable( GL_FOG );

  for (unsigned int i=0; i<nGroups; ++i)
  {
    if (groups[i].is_visible (ofs, rot, culldistance, frustum, camera))
    {
      groups[i].draw (world, draw_fog, hasSkies, fog_distance);

      if (draw_doodads)
      {
        groups[i].drawDoodads ( world
                              , doodadset
                              , ofs
                              , rot
                              , draw_fog
                              , fog_distance
                              , frustum
                              , culldistance
                              , camera
                              );
      }

      groups[i].drawLiquid (world, draw_fog, fog_distance);
    }
  }

  if( boundingbox )
  {
    gl.disable( GL_LIGHTING );

    gl.disable( GL_COLOR_MATERIAL );
    gl.activeTexture( GL_TEXTURE0 );
    gl.disable( GL_TEXTURE_2D );
    gl.activeTexture( GL_TEXTURE1 );
    gl.disable( GL_TEXTURE_2D );
    gl.enable( GL_BLEND );
    gl.blendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    static const ::math::vector_4d white (1.0f, 1.0f, 1.0f, 1.0f);

    for( unsigned int i = 0; i < nGroups; ++i )
    {
      const ::opengl::primitives::wire_box group_box
        (groups[i].BoundingBoxMin, groups[i].BoundingBoxMax);
      group_box.draw (white, 1.0f);
    }

    /*gl.color4fv( ::math::vector_4d( 1.0f, 0.0f, 0.0f, 1.0f ) );
    gl.begin( GL_LINES );
      gl.vertex3f( 0.0f, 0.0f, 0.0f );
      gl.vertex3f( header.BoundingBoxMax.x + header.BoundingBoxMax.x / 5.0f, 0.0f, 0.0f );
    gl.end();

    gl.color4fv( ::math::vector_4d( 0.0f, 1.0f, 0.0f, 1.0f ) );
    gl.begin( GL_LINES );
      gl.vertex3f( 0.0f, 0.0f, 0.0f );
      gl.vertex3f( 0.0f, header.BoundingBoxMax.z + header.BoundingBoxMax.z / 5.0f, 0.0f );
    gl.end();

    gl.color4fv( ::math::vector_4d( 0.0f, 0.0f, 1.0f, 1.0f ) );
    gl.begin( GL_LINES );
      gl.vertex3f( 0.0f, 0.0f, 0.0f );
      gl.vertex3f( 0.0f, 0.0f, header.BoundingBoxMax.y + header.BoundingBoxMax.y / 5.0f );
    gl.end();*/

    gl.activeTexture( GL_TEXTURE1 );
    gl.disable( GL_TEXTURE_2D );
    gl.activeTexture( GL_TEXTURE0 );
    gl.enable( GL_TEXTURE_2D );

    gl.enable( GL_LIGHTING );

  }

/*  {
    // draw boundingboxe and axis
    // Turn light off and highlight the following
    gl.disable(GL_LIGHTING);
    gl.disable(GL_COLOR_MATERIAL);
    gl.activeTexture(GL_TEXTURE0);
    gl.disable(GL_TEXTURE_2D);
    gl.activeTexture(GL_TEXTURE1);
    gl.disable(GL_TEXTURE_2D);
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.enable (GL_LINE_SMOOTH);
    gl.lineWidth(1.0);
    gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);

    gl.color4f( 1, 1, 1, 1 );

    gl.lineWidth(1.0);
    gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    for (int i=0; i<nGroups; ++i)
    {
      WMOGroup &header = groups[i];
      /// Bounding box
      gl.color4f( 1, 1, 1, 1 );
      gl.begin( GL_LINE_STRIP );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
      gl.end();

      gl.begin( GL_LINES );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
      gl.end();
      gl.begin( GL_LINES );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
      gl.end();
      gl.begin( GL_LINES );
        gl.vertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
        gl.vertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
      gl.end();

      // draw axis
      gl.color4f( 1.0f, 0.0f, 0.0f, 1.0f );
      gl.begin( GL_LINES );
        gl.vertex3f( 0.0f, 0.0f, 0.0f );
        gl.vertex3f( header.BoundingBoxMax.x + 6.0f, 0.0f, 0.0f );
      gl.end();


      gl.color4f( 0.0f, 1.0f, 0.0f, 1.0f );
      gl.begin( GL_LINES );
        gl.vertex3f( 0.0f, 0.0f, 0.0f );
        gl.vertex3f( 0.0f, header.BoundingBoxMax.y + 6.0f, 0.0f );
      gl.end();

      gl.color4f( 0.0f, 0.0f, 1.0f, 1.0f );
      gl.begin( GL_LINES );
        gl.vertex3f( 0.0f, 0.0f, 0.0f );
        gl.vertex3f( 0.0f, 0.0f, header.BoundingBoxMax.x + 6.0f );
      gl.end();



    }
    // Back to normal light rendering
    gl.activeTexture(GL_TEXTURE1);
    gl.disable(GL_TEXTURE_2D);
    gl.activeTexture(GL_TEXTURE0);
    gl.enable(GL_TEXTURE_2D);
    gl.enable(GL_LIGHTING);
  } // end bounding  boxes.*/

  if(false && groupboxes)
  {
    //WIP STEFF
    // draw group boundingboxes
    // Turn light off and highlight the following
    gl.disable(GL_LIGHTING);
    gl.disable(GL_COLOR_MATERIAL);
    gl.activeTexture(GL_TEXTURE0);
    gl.disable(GL_TEXTURE_2D);
    gl.activeTexture(GL_TEXTURE1);
    gl.disable(GL_TEXTURE_2D);
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.enable (GL_LINE_SMOOTH);
    gl.lineWidth(1.0);
    gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);

    gl.color4f( 1, 1, 0, 1 );

    gl.lineWidth(1.0);
    gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);
    for (unsigned int i=0; i<nGroups; ++i)
    {
      WMOGroup &header = groups[i];
      gl.begin( GL_LINE_STRIP );
      //A
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMax.y(), header.VertexBoxMin.z() );
      //C
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMin.y(), header.VertexBoxMin.z() );
      //D
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMin.y(), header.VertexBoxMin.z() );
      //G
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMin.y(), header.VertexBoxMax.z() );
      //H
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMax.y(), header.VertexBoxMax.z() );
      //B
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMax.y(), header.VertexBoxMin.z() );
      //A
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMax.y(), header.VertexBoxMin.z() );
      //E
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMax.y(), header.VertexBoxMax.z() );
      //F
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMin.y(), header.VertexBoxMax.z() );
      //C
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMin.y(), header.VertexBoxMin.z() );
      gl.end();

      gl.begin( GL_LINES );
      // F G
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMin.y(), header.VertexBoxMax.z() );
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMin.y(), header.VertexBoxMax.z() );
      gl.end();
      gl.begin( GL_LINES );
      // B D
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMax.y(), header.VertexBoxMin.z() );
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMin.y(), header.VertexBoxMin.z() );
      gl.end();
      gl.begin( GL_LINES );
      // E H
        gl.vertex3f( header.VertexBoxMin.x(), header.VertexBoxMax.y(), header.VertexBoxMax.z() );
        gl.vertex3f( header.VertexBoxMax.x(), header.VertexBoxMax.y(), header.VertexBoxMax.z() );
      gl.end();
    }
    // Back to normal light rendering
    gl.activeTexture(GL_TEXTURE1);
    gl.disable(GL_TEXTURE_2D);
    gl.activeTexture(GL_TEXTURE0);
    gl.enable(GL_TEXTURE_2D);
    gl.enable(GL_LIGHTING);
  } // end drow groupe boxes.





  /*
  // draw portal relations
  gl.begin(GL_LINES);
  for (size_t i=0; i<prs.size(); ++i) {
    WMOPR &pr = prs[i];
    WMOPV &pv = pvs[pr.portal];
    if (pr.dir>0) gl.color4f(1,0,0,1);
    else gl.color4f(0,0,1,1);
    ::math::vector_3d pc = (pv.a+pv.b+pv.c+pv.d)*0.25f;
    ::math::vector_3d gc = (groups[pr.group].b1 + groups[pr.group].b2)*0.5f;
    gl.vertex3fv(pc);
    gl.vertex3fv(gc);
  }
  gl.end();
  gl.color4f(1,1,1,1);
  // draw portals
  for (int i=0; i<nP; ++i) {
    gl.begin(GL_LINE_STRIP);
    gl.vertex3fv(pvs[i].d);
    gl.vertex3fv(pvs[i].c);
    gl.vertex3fv(pvs[i].b);
    gl.vertex3fv(pvs[i].a);
    gl.end();
  }
  gl.enable(GL_TEXTURE_2D);
  gl.enable(GL_LIGHTING);
  */
}

void WMO::drawSelect (World* world
                     , int doodadset
                     , const ::math::vector_3d &ofs
                     , const float rot
                     , const float culldistance
                     , bool draw_doodads
                     , const Frustum& frustum
                     , const ::math::vector_3d& camera
                     ) const
{
  for (unsigned int i=0; i<nGroups; ++i)
  {
    if (groups[i].is_visible (ofs, rot, culldistance, frustum, camera))
    {
      groups[i].draw_for_selection();

      if (draw_doodads)
      {
        groups[i].drawDoodadsSelect( doodadset
                                   , ofs
                                   , rot
                                   , frustum
                                   , culldistance
                                   , camera
                                   );
      }

      groups[i].drawLiquid (world, false, 0.0f);
    }
  }
}

bool WMO::drawSkybox(World* world, ::math::vector_3d pCamera, ::math::vector_3d pLower, ::math::vector_3d pUpper ) const
{
  if(skybox && pCamera.is_inside_of (pLower, pUpper))
  {
    //! \todo  only draw sky if we are "inside" the WMO... ?

    // We need to clear the depth buffer, because the skybox model can (will?)
    // require it *. This is inefficient - is there a better way to do this?
    // * planets in front of "space" in Caverns of Time
    //gl.clear(GL_DEPTH_BUFFER_BIT);

    // update: skybox models seem to have an explicit renderop ordering!
    // that saves us the depth buffer clear and the depth testing, too

    gl.disable(GL_CULL_FACE);
    gl.disable(GL_DEPTH_TEST);
    gl.pushMatrix();
    gl.translatef(pCamera.x(), pCamera.y(), pCamera.z());
    const float sc = 2.0f;
    gl.scalef(sc,sc,sc);
    skybox.get()->draw (world, clock() / CLOCKS_PER_SEC);
    gl.popMatrix();
    gl.enable(GL_DEPTH_TEST);

    return true;
  }

  return false;
}

/*
void WMO::drawPortals()
{
  // not used ;)
  gl.begin(GL_QUADS);
  for (int i=0; i<nP; ++i) {
    gl.vertex3fv(pvs[i].d);
    gl.vertex3fv(pvs[i].c);
    gl.vertex3fv(pvs[i].b);
    gl.vertex3fv(pvs[i].a);
  }
  gl.end();
}
*/

void WMOLight::init(noggit::mpq::file* f)
{
  char type[4];
  f->read(&type,4);
  f->read(&color,4);
  f->read(pos, 12);
  f->read(&intensity, 4);
  f->read(unk, 4*5);
  f->read(&r,4);

  pos = ::math::vector_3d(pos.x(), pos.z(), -pos.y());

  // rgb? bgr? hm
  float fa = ((color & 0xff000000) >> 24) / 255.0f;
  float fr = ((color & 0x00ff0000) >> 16) / 255.0f;
  float fg = ((color & 0x0000ff00) >>  8) / 255.0f;
  float fb = ((color & 0x000000ff)      ) / 255.0f;

  fcolor = ::math::vector_4d (fr,fg,fb,fa);
  fcolor *= intensity;
  fcolor.w (1.0f);

  /*
  // light logging
  gLog("Light %08x @ (%4.2f,%4.2f,%4.2f)\t %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f\t(%d,%d,%d,%d)\n",
    color, pos.x, pos.y, pos.z, intensity,
    unk[0], unk[1], unk[2], unk[3], unk[4], r,
    type[0], type[1], type[2], type[3]);
  */
}

void WMOLight::setup(opengl::light light)
{
  // not used right now -_-

  GLfloat LightAmbient[] = {0, 0, 0, 1.0f};
  GLfloat LightPosition[] = {pos.x(), pos.y(), pos.z(), 0.0f};

  gl.lightfv(light, GL_AMBIENT, LightAmbient);
  gl.lightfv(light, GL_DIFFUSE, fcolor);
  gl.lightfv(light, GL_POSITION,LightPosition);

  gl.enable(light);
}

void WMOLight::setupOnce(opengl::light light, ::math::vector_3d dir, ::math::vector_3d lcol)
{
  ::math::vector_4d position(dir, 0);
  //::math::vector_4d position(0,1,0,0);

  ::math::vector_4d ambient (lcol * 0.3f, 1);
  //::math::vector_4d ambient (0.101961f, 0.062776f, 0, 1);
  ::math::vector_4d diffuse (lcol, 1);
  //::math::vector_4d diffuse (0.439216f, 0.266667f, 0, 1);

  gl.lightfv(light, GL_AMBIENT, ambient);
  gl.lightfv(light, GL_DIFFUSE, diffuse);
  gl.lightfv(light, GL_POSITION,position);

  gl.enable(light);
}



void WMOGroup::init(WMO *_wmo, noggit::mpq::file* f, int _num, char *names)
{
  wmo = _wmo;
  num = _num;

  // extract group info from f
  f->read(&flags,4);
  f->read(VertexBoxMax,12);
  f->read(VertexBoxMin,12);
  int nameOfs;
  f->read(&nameOfs,4);

  //! \todo  get proper name from group header and/or dbc?
  if (nameOfs > 0) {
        name = std::string(names + nameOfs);
  } else name = "(no name)";

  ddr = 0;
  nDoodads = 0;

  lq = 0;
}


struct WMOBatch {
  signed char bytes[12];
  uint32_t indexStart;
  uint16_t indexCount, vertexStart, vertexEnd;
  unsigned char flags, texture;
};

namespace
{
  void setGLColor(unsigned int col)
  {
    //gl.color4ubv((GLubyte*)(&col));
    GLubyte r,g,b,a;
    a = (col & 0xFF000000) >> 24;
    r = (col & 0x00FF0000) >> 16;
    g = (col & 0x0000FF00) >> 8;
    b = (col & 0x000000FF);
      gl.color4ub(r,g,b,1);
  }

  ::math::vector_4d colorFromInt(unsigned int col) {
    GLubyte r,g,b,a;
    a = (col & 0xFF000000) >> 24;
    r = (col & 0x00FF0000) >> 16;
    g = (col & 0x0000FF00) >> 8;
    b = (col & 0x000000FF);
    return ::math::vector_4d(r/255.0f, g/255.0f, b/255.0f, a/255.0f);
  }
}

struct WMOGroupHeader {
  uint32_t nameStart, nameStart2, flags;
  float box1[3], box2[3];
  uint16_t portalStart, portalCount;
  uint16_t batches[4];
  uint8_t fogs[4];
  int32_t unk1, id, unk2, unk3;
};

void WMOGroup::initDisplayList()
{
  ::math::vector_3d *vertices = nullptr;
  ::math::vector_3d *normals = nullptr;
  ::math::vector_2d *texcoords = nullptr;
  uint16_t *indices = nullptr;
  struct SMOPoly *materials = nullptr;
  WMOBatch *batches = nullptr;

  WMOGroupHeader gh;

  uint16_t *useLights = 0;
  int nLR = 0;


  // open group file

  std::stringstream curNum;
  curNum << "_" << std::setw(3) << std::setfill('0') << num;

  std::string fname = wmo->filename();
  fname.insert( fname.find( ".wmo" ), curNum.str() );

  noggit::mpq::file gf(QString::fromStdString (fname));

  /*if(!gf.isExternal())
    gLog("    Loading WMO from MPQ %s\n", fname);
  else
    gLog("    Loading WMO from File %s\n", fname);
  */
  gf.seek(0x14);
  // read header
  gf.read(&gh, sizeof(WMOGroupHeader));
  WMOFog &wf = wmo->fogs[gh.fogs[0]];
  if (wf.r2 <= 0) fog = -1; // default outdoor fog..?
  else fog = gh.fogs[0];

  BoundingBoxMin = ::math::vector_3d(gh.box1[0], gh.box1[2], -gh.box1[1]);
  BoundingBoxMax = ::math::vector_3d(gh.box2[0], gh.box2[2], -gh.box2[1]);

  gf.seek(0x58); // first chunk

  uint32_t fourcc;
  uint32_t size;

  unsigned int *cv = nullptr;
  hascv = false;

  while (!gf.is_at_end_of_file()) {
    gf.read(&fourcc,4);
    gf.read(&size, 4);

    size_t nextpos = gf.getPos() + size;

    // why copy stuff when I can just map it from memory ^_^

    if ( fourcc == 'MOPY' ) {
      // materials per triangle
      nTriangles = size / 2;
      materials = reinterpret_cast<SMOPoly*>(gf.getPointer());
    }
    else if ( fourcc == 'MOVI' ) {
      // indices
      indices =  reinterpret_cast<uint16_t*>(gf.getPointer());
    }
    else if ( fourcc == 'MOVT' ) {
      nVertices = size / 12;
      // let's hope it's padded to 12 bytes, not 16...
      vertices = reinterpret_cast< ::math::vector_3d*>(gf.getPointer());
      VertexBoxMin = ::math::vector_3d( 9999999.0f, 9999999.0f, 9999999.0f);
      VertexBoxMax = ::math::vector_3d(-9999999.0f,-9999999.0f,-9999999.0f);
      rad = 0;
      for (size_t i=0; i<nVertices; ++i) {
        ::math::vector_3d v(vertices[i].x(), vertices[i].z(), -vertices[i].y());
        if (v.x() < VertexBoxMin.x()) VertexBoxMin.x (v.x());
        if (v.y() < VertexBoxMin.y()) VertexBoxMin.y (v.y());
        if (v.z() < VertexBoxMin.z()) VertexBoxMin.z (v.z());
        if (v.x() > VertexBoxMax.x()) VertexBoxMax.x (v.x());
        if (v.y() > VertexBoxMax.y()) VertexBoxMax.y (v.y());
        if (v.z() > VertexBoxMax.z()) VertexBoxMax.z (v.z());
      }
      center = (VertexBoxMax + VertexBoxMin) * 0.5f;
      rad = (VertexBoxMax-center).length();
    }
    else if ( fourcc == 'MONR' ) {
      normals =  reinterpret_cast< ::math::vector_3d*>(gf.getPointer());
    }
    else if ( fourcc == 'MOTV' ) {
      texcoords =  reinterpret_cast< ::math::vector_2d*>(gf.getPointer());
    }
    else if ( fourcc == 'MOLR' ) {
      nLR = size / 2;
      useLights =  reinterpret_cast<uint16_t*>(gf.getPointer());
    }
    else if ( fourcc == 'MODR' ) {
      nDoodads = size / 2;
      ddr = new int16_t[nDoodads];
      gf.read(ddr,size);
    }
    else if ( fourcc == 'MOBA' ) {
      nBatches = size / 24;
      batches = reinterpret_cast<WMOBatch*>(gf.getPointer());
    }
    else if ( fourcc == 'MOCV' ) {
      //gLog("CV: %d\n", size);
      hascv = true;
      cv = reinterpret_cast<uint32_t*>(gf.getPointer());
    }
    else if ( fourcc == 'MLIQ' ) {
      WMOLiquidHeader hlq;
      gf.read (&hlq, sizeof (WMOLiquidHeader));

      lq = new Liquid (hlq.A, hlq.B, ::math::vector_3d (hlq.pos.x(), hlq.pos.z(), -hlq.pos.y()));
      lq->initFromWMO (&gf, wmo->_materials[hlq.type], flags & 0x2000);
    }

    //! \todo  figure out/use MFOG ?

     gf.seek(nextpos);
  }

  // ok, make a display list

  indoor = flags & 8192;
  //gLog("Lighting: %s %X\n\n", indoor?"Indoor":"Outdoor", flags);

  initLighting(nLR,useLights);

  //dl = gl.genLists(1);
  //gl.newList(dl, GL_COMPILE);
  //gl.disable(GL_BLEND);
  //gl.color4f(1,1,1,1);

  /*
  float xr=0,xg=0,xb=0;
  if (flags & 0x0040) xr = 1;
  if (flags & 0x2000) xg = 1;
  if (flags & 0x8000) xb = 1;
  gl.color4f(xr,xg,xb,1);
  */

  // generate lists for each batch individually instead
  _lists.resize( nBatches );

  // assume that texturing is on, for unit 1

  for (size_t b=0; b<nBatches; b++)
  {
    WMOBatch *batch = &batches[b];
    WMOMaterial* mat (&wmo->_materials.at (batch->texture));

    bool overbright = ((mat->flags & 0x10) && !hascv);
    bool spec_shader = (mat->specular && !hascv && !overbright);

    _lists[b].first = new opengl::call_list();
    _lists[b].second = spec_shader;

    _lists[b].first->start_recording( GL_COMPILE );

    mat->_texture->bind();

    bool atest = (mat->transparent) != 0;

    if (atest) {
      gl.enable(GL_ALPHA_TEST);
      float aval = 0;
            if (mat->flags & 0x80) aval = 0.3f;
      if (mat->flags & 0x01) aval = 0.0f;
      gl.alphaFunc(GL_GREATER, aval);
    }

    if (mat->flags & 0x04) gl.disable(GL_CULL_FACE);
    else gl.enable(GL_CULL_FACE);

    if (spec_shader) {
      gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, colorFromInt(mat->col2));
    } else {
      ::math::vector_4d nospec(0,0,0,1);
      gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, nospec);
    }

    if (overbright) {
      //! \todo  use emissive color from the WMO Material instead of 1,1,1,1
      GLfloat em[4] = {1,1,1,1};
      gl.materialfv(GL_FRONT, GL_EMISSION, em);
    }

    // render
    gl.begin(GL_TRIANGLES);
    for (int t=0, i=batch->indexStart; t<batch->indexCount; t++,++i) {
      int a = indices[i];
      if (indoor && hascv) {
              setGLColor(cv[a]);
      }
      gl.normal3f(normals[a].x(), normals[a].z(), -normals[a].y());
      gl.texCoord2fv(texcoords[a]);
      gl.vertex3f(vertices[a].x(), vertices[a].z(), -vertices[a].y());
    }
    gl.end();

    if (overbright) {
      GLfloat em[4] = {0,0,0,1};
      gl.materialfv(GL_FRONT, GL_EMISSION, em);
    }

    if (atest) {
      gl.disable(GL_ALPHA_TEST);
    }

    _lists[b].first->end_recording();
  }

  indoor = false;
}


void WMOGroup::initLighting(int /*nLR*/, uint16_t* /*useLights*/)
{
  //dl_light = 0;
  // "real" lighting?
  if ((flags & 0x2000) && hascv) {

    ::math::vector_3d dirmin(1,1,1);
    float lenmin;
    int lmin;

    for (int i=0; i<nDoodads; ++i) {
      lenmin = 999999.0f*999999.0f;
      lmin = 0;
      ModelInstance &mi = wmo->modelis[ddr[i]];
      for (unsigned int j=0; j<wmo->nLights; j++) {
        WMOLight &l = wmo->lights[j];
        ::math::vector_3d dir = l.pos - mi.pos;
        float ll = dir.length_squared();
        if (ll < lenmin) {
          lenmin = ll;
          dirmin = dir;
          lmin = j;
        }
      }
      mi.ldir = dirmin;
    }

    outdoorLights = false;
  } else {
    outdoorLights = true;
  }
}

bool WMOGroup::is_visible ( const ::math::vector_3d& offset
                          , const float& rotation
                          , const float& cull_distance
                          , const Frustum& frustum
                          , const ::math::vector_3d& camera
                          ) const
{
  const ::math::vector_3d& base_position (center);
  const float& radius (rad);

  ::math::vector_3d position (base_position + offset);
  ::math::rotate ( offset.x(), offset.z()
                 , &position.x(), &position.z()
                 , rotation
                 );

  return frustum.intersectsSphere (position, radius)
      && ((position - camera).length() < (cull_distance + radius));
}

void WMOGroup::draw ( World* world
                    , bool draw_fog
                    , bool hasSkies
                    , const float& fog_distance
                    )
{
  setupFog(world, draw_fog, fog_distance);

  if (hascv)
  {
    gl.disable(GL_LIGHTING);
    world->outdoorLights(false);
  }
  else if (hasSkies)
  {
    world->outdoorLights(true);
  }
  else
  {
    gl.disable(GL_LIGHTING);
  }


  //gl.callList(dl);
  gl.disable(GL_BLEND);
  gl.color4f(1,1,1,1);
  for (size_t i=0; i<nBatches; ++i)
  {
    if (wmoShader)
    {
      wmoShader->bind();
      _lists[i].first->render();
      wmoShader->unbind();
    }
    else
    {
      _lists[i].first->render();
    }
  }

  gl.color4f(1,1,1,1);
  gl.enable(GL_CULL_FACE);

  if (hascv)
      gl.enable(GL_LIGHTING);


}

void WMOGroup::draw_for_selection() const
{
  for (size_t i (0); i < nBatches; ++i)
  {
    _lists[i].first->render();
  }
}

void WMOGroup::drawDoodads ( World* world
                           , unsigned int doodadset
                           , const ::math::vector_3d& ofs
                           , const float rot
                           , bool draw_fog
                           , const float& fog_distance
                           , const Frustum& frustum
                           , const float& cull_distance
                           , const ::math::vector_3d& camera
                           )
{
  if (nDoodads==0) return;

  world->outdoorLights(outdoorLights);
  setupFog(world, draw_fog, fog_distance);

  /*
  float xr=0,xg=0,xb=0;
  if (flags & 0x0040) xr = 1;
  //if (flags & 0x0008) xg = 1;
  if (flags & 0x8000) xb = 1;
  gl.color4f(xr,xg,xb,1);
  */

  // draw doodads
  gl.color4f(1,1,1,1);
  for (int i=0; i<nDoodads; ++i) {
    int16_t dd = ddr[i];
    if( ! ( wmo->doodadsets.size() < doodadset ) )
      if ((dd >= wmo->doodadsets[doodadset].start) && (dd < (wmo->doodadsets[doodadset].start+wmo->doodadsets[doodadset].size))) {

        ModelInstance &mi = wmo->modelis[dd];

        if (!outdoorLights) {
          WMOLight::setupOnce(GL_LIGHT2, mi.ldir, mi.lcol);
        }
        setupFog(world, draw_fog, fog_distance);
        if (mi.is_visible (cull_distance, frustum, camera, ofs, rot))
        {
          mi.draw2();
        }
      }
  }

  gl.disable(GL_LIGHT2);

  gl.color4f(1,1,1,1);

}


void WMOGroup::drawDoodadsSelect ( unsigned int doodadset
                                 , const ::math::vector_3d& ofs
                                 , const float rot
                                 , const Frustum& frustum
                                 , const float& cull_distance
                                 , const ::math::vector_3d& camera
                                 )
{
  if (nDoodads==0) return;

  /*
  float xr=0,xg=0,xb=0;
  if (flags & 0x0040) xr = 1;
  //if (flags & 0x0008) xg = 1;
  if (flags & 0x8000) xb = 1;
  gl.color4f(xr,xg,xb,1);
  */

  // draw doodads
  gl.color4f(1,1,1,1);
  for (int i=0; i<nDoodads; ++i) {
    int16_t dd = ddr[i];
    if( ! ( wmo->doodadsets.size() < doodadset ) )
      if ((dd >= wmo->doodadsets[doodadset].start) && (dd < (wmo->doodadsets[doodadset].start+wmo->doodadsets[doodadset].size))) {

        ModelInstance &mi = wmo->modelis[dd];

        if (!outdoorLights) {
          WMOLight::setupOnce(GL_LIGHT2, mi.ldir, mi.lcol);
        }
        if (mi.is_visible (cull_distance, frustum, camera, ofs, rot))
        {
          mi.draw2Select();
        }
      }
  }

  gl.disable(GL_LIGHT2);

  gl.color4f(1,1,1,1);

}

void WMOGroup::drawLiquid ( World* world
                          , bool draw_fog
                          , const float& fog_distance
                          )
{
  // draw liquid
  //! \todo  culling for liquid boundingbox or something
  if (lq) {
    setupFog (world, draw_fog, fog_distance);
    if (outdoorLights) {
      world->outdoorLights(true);
    } else {
      //! \todo  setup some kind of indoor lighting... ?
      world->outdoorLights(false);
      gl.enable(GL_LIGHT2);
      gl.lightfv(GL_LIGHT2, GL_AMBIENT, ::math::vector_4d(0.1f,0.1f,0.1f,1));
      gl.lightfv(GL_LIGHT2, GL_DIFFUSE, ::math::vector_4d(0.8f,0.8f,0.8f,1));
      gl.lightfv(GL_LIGHT2, GL_POSITION, ::math::vector_4d(0,1,0,0));
    }
    gl.disable(GL_BLEND);
    gl.disable(GL_ALPHA_TEST);
    gl.depthMask(GL_TRUE);
    gl.color4f(1,1,1,1);
    lq->draw (world->skies.get());
    gl.disable(GL_LIGHT2);
  }
}

void WMOGroup::setupFog ( World* world
                        , bool draw_fog
                        , const float& fog_distance
                        )
{
  if (outdoorLights || fog == -1)
  {
    world->setupFog (draw_fog, fog_distance);
  }
  else
  {
    wmo->fogs[fog].setup();
  }
}



WMOGroup::~WMOGroup()
{
  //if (dl) gl.deleteLists(dl, 1);
  //if (dl_light) gl.deleteLists(dl_light, 1);
  for( std::vector< std::pair<opengl::call_list*, bool> >::iterator it = _lists.begin(); it != _lists.end(); ++it )
  {
    delete it->first;
  }
  _lists.clear();

  delete[] ddr;
  ddr = nullptr;
  delete lq;
  lq = nullptr;
}


void WMOFog::init(noggit::mpq::file* f)
{
  f->read(this, 0x30);
  color = ::math::vector_4d( ((color1 & 0x00FF0000) >> 16)/255.0f, ((color1 & 0x0000FF00) >> 8)/255.0f,
          (color1 & 0x000000FF)/255.0f, ((color1 & 0xFF000000) >> 24)/255.0f);
  const float temp (pos.y());
  pos.y (pos.z());
  pos.z (-temp);
  fogstart = fogstart * fogend * 1.5;
  fogend *= 1.5;
}

void WMOFog::setup()
{
  /*if (world->drawfog) {
    gl.fogfv(GL_FOG_COLOR, color);
    gl.fogf(GL_FOG_START, fogstart);
    gl.fogf(GL_FOG_END, fogend);

    gl.enable(GL_FOG);
  } else {
    gl.disable(GL_FOG);
  }*/
    gl.disable(GL_FOG);
}

void WMO::finish_loading()
{
  _finished = true;
}

namespace noggit
{
  scoped_wmo_reference::scoped_wmo_reference (World* world, std::string const& filename)
    : _valid (true)
    , _filename (filename)
    , _world (world)
    , _wmo (noggit::app().wmo_manager().emplace (_filename, _world))
  {}

  scoped_wmo_reference::scoped_wmo_reference (scoped_wmo_reference const& other)
    : scoped_wmo_reference (other._world, other._filename)
  {}
  scoped_wmo_reference::scoped_wmo_reference (scoped_wmo_reference&& other)
    : _valid (std::move (other._valid))
    , _filename (std::move (other._filename))
    , _world (std::move (other._world))
    , _wmo (std::move (other._wmo))
  {
    other._valid = false;
  }
  scoped_wmo_reference& scoped_wmo_reference::operator= (scoped_wmo_reference&& other)
  {
    std::swap (_valid, other._valid);
    std::swap (_filename, other._filename);
    std::swap (_world, other._world);
    std::swap (_wmo, other._wmo);
    return *this;
  }

  scoped_wmo_reference::~scoped_wmo_reference()
  {
    if (_valid)
    {
      noggit::app().wmo_manager().erase (_filename);
    }
  }
}
