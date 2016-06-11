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
#include <opengl/scoped.h>

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

namespace
{
  math::vector_3d convert_coords (math::vector_3d const& v)
  {
    static math::matrix_4x4 const mat { 1.0f,  0.0f, 0.0f, 0.0f
                                      , 0.0f,  0.0f, 1.0f, 0.0f
                                      , 0.0f, -1.0f, 0.0f, 0.0f
                                      , 0.0f,  0.0f, 0.0f, 1.0f
                                      };
    return mat * v;
  }
}

WMO::WMO (const std::string& filenameArg, World* world)
  : _filename (filenameArg)
  , _world(world)
  , _finished_upload(false)
{
  _finished = false;
  noggit::app ().async_loader ().add_object (this);
}

void WMO::finish_loading ()
{
  noggit::mpq::file f (QString::fromStdString (_filename));

  LogDebug << "Loading WMO \"" << _filename << "\"." << std::endl;

  uint32_t fourcc;
  uint32_t size;

  float ff[3];

  char *texbuf = nullptr;
  char *ddnames = nullptr;
  char *groupnames = nullptr;

  // - MVER ----------------------------------------------

  uint32_t version;

  f.read (&fourcc, 4);
  f.seekRelative (4);
  f.read (&version, 4);

  assert (fourcc == 'MVER' && version == 17);

  // - MOHD ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MOHD');

  unsigned int col;
  // header
  f.read (&nTextures, 4);
  f.read (&nGroups, 4);
  f.read (&nP, 4);
  f.read (&nLights, 4);
  f.read (&nModels, 4);
  f.read (&nDoodads, 4);
  f.read (&nDoodadSets, 4);
  f.read (&col, 4);
  f.read (&nX, 4);
  f.read (ff, 12);
  extents[0] = ::math::vector_3d (ff[0], ff[1], ff[2]);
  f.read (ff, 12);
  extents[1] = ::math::vector_3d (ff[0], ff[1], ff[2]);
  f.seekRelative (4);

  groups = new WMOGroup[nGroups];

  // - MOTX ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOTX');

  texbuf = new char[size];
  f.read (texbuf, size);

  // - MOMT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOMT');

  const size_t num_materials (size / 0x40);

  for (size_t i (0); i < num_materials; ++i)
  {
    SMOMaterial material;
    f.read (&material, sizeof (material));

    _materials.emplace_back (material, texbuf + material.nameStart);
  }

  // - MOGN ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOGN');

  groupnames = reinterpret_cast<char*> (f.getPointer ());

  f.seekRelative (size);

  // - MOGI ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOGI');

  for (size_t i (0); i < nGroups; ++i) {
    groups[i].init (this, &f, i, groupnames);
  }

  // - MOSB ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOSB');

  if (size > 4)
  {
    std::string path = std::string (reinterpret_cast<char*>(f.getPointer ()));
    if (path.length ())
    {
      LogDebug << "SKYBOX:" << std::endl;

      if (noggit::mpq::file::exists (QString::fromStdString (path)))
      {
        skybox = noggit::scoped_model_reference (path);
      }
    }
  }

  f.seekRelative (size);

  // - MOPV ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MOPV');

  WMOPV p;
  for (size_t i (0); i < nP; ++i) {
    f.read (ff, 12);
    p.a = ::math::vector_3d (ff[0], ff[2], -ff[1]);
    f.read (ff, 12);
    p.b = ::math::vector_3d (ff[0], ff[2], -ff[1]);
    f.read (ff, 12);
    p.c = ::math::vector_3d (ff[0], ff[2], -ff[1]);
    f.read (ff, 12);
    p.d = ::math::vector_3d (ff[0], ff[2], -ff[1]);
    pvs.push_back (p);
  }

  // - MOPT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOPT');

  f.seekRelative (size);

  // - MOPR ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOPR');

  int nn = size / 8;
  WMOPR *pr = reinterpret_cast<WMOPR*> (f.getPointer ());
  for (size_t i (0); i < nn; ++i) {
    prs.push_back (*pr++);
  }

  f.seekRelative (size);

  // - MOVV ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVV');

  f.seekRelative (size);

  // - MOVB ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVB');

  f.seekRelative (size);

  // - MOLT ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MOLT');

  for (size_t i (0); i < nLights; ++i) {
    WMOLight l;
    l.init (&f);
    lights.push_back (l);
  }

  // - MODS ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MODS');

  for (size_t i (0); i < nDoodadSets; ++i) {
    WMODoodadSet dds;
    f.read (&dds, 32);
    doodadsets.push_back (dds);
  }

  // - MODN ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MODN');

  if (size)
  {
    ddnames = reinterpret_cast<char*> (f.getPointer ());
    f.seekRelative (size);
  }

  // - MODD ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MODD');

  nModels = size / 0x28;
  for (size_t i (0); i < nModels; ++i) {
    uint32_t ofs;
    f.read (&ofs, sizeof (uint32_t));

    ::math::vector_3d position;
    f.read (position, sizeof (position));

    ::math::vector_4d rotation;
    f.read (rotation, sizeof (rotation));

    float scale;
    f.read (&scale, sizeof (float));

    uint32_t light;
    f.read (&light, sizeof (uint32_t));

    modelis.emplace_back
    (_world
      , ddnames + ofs
      , ::math::vector_3d (position.x ()
        , position.z ()
        , -position.y ()
      )
      , ::math::quaternion (-rotation.w ()
        , rotation.y ()
        , rotation.z ()
        , rotation.x ()
      )
      , scale
      , ::math::vector_3d (((light & 0xff0000) >> 16) / 255.0f
        , ((light & 0x00ff00) >> 8) / 255.0f
        , ((light & 0x0000ff)) / 255.0f
      )
    );
  }

  // - MFOG ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MFOG');

  int nfogs = size / 0x30;
  for (size_t i (0); i < nfogs; ++i) {
    WMOFog fog;
    fog.init (&f);
    fogs.push_back (fog);
  }

  for (size_t i (0); i < nGroups; ++i)
  {
    groups[i].load ();
  }

  delete[] texbuf;
  texbuf = nullptr;

  _finished = true;
}

void WMO::upload()
{
  for (auto mat : _materials)
    _textures.emplace_back (mat._texture);

  for (unsigned int i = 0; i<nGroups; ++i)
    groups[i].initDisplayList ();

  _finished_upload = true;
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
               )
{
  if (!finished_loading ())
    return;

  if (!_finished_upload) {
    upload ();
    return;
  }

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

  if (groupboxes || boundingbox)
  {
    opengl::scoped::bool_setter<GL_LIGHTING, GL_FALSE> const lighting;
    opengl::scoped::bool_setter<GL_FOG, GL_FALSE> const fog;
    opengl::scoped::texture_setter<0, GL_FALSE> const texture0;
    opengl::scoped::texture_setter<1, GL_FALSE> const texture1;
    opengl::scoped::bool_setter<GL_COLOR_MATERIAL, GL_FALSE> const color_material;
    opengl::scoped::bool_setter<GL_BLEND, GL_TRUE> const blend;

    gl.blendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    {
      static ::math::vector_4d const white (1.0f, 1.0f, 1.0f, 1.0f);

      for (std::size_t i (0); i < nGroups; ++i)
      {
        opengl::primitives::wire_box
          (groups[i].BoundingBoxMin, groups[i].BoundingBoxMax).draw (white, 1.0f);
      }
    }

    if (boundingbox)
    {
      static ::math::vector_4d const yellow (1.0f, 1.0f, 0.0f, 1.0f);

      opengl::primitives::wire_box ( {extents[0][0], extents[0][2], -extents[0][1]}
                                   , {extents[1][0], extents[1][2], -extents[1][1]}
                                   ).draw (yellow, 1.0f);
    }
  }
}

void WMO::drawSelect (World* world
                     , int doodadset
                     , const ::math::vector_3d &ofs
                     , const float rot
                     , const float culldistance
                     , bool draw_doodads
                     , const Frustum& frustum
                     , const ::math::vector_3d& camera
                     )
{
  if (!finished_loading ())
    return;

  if (!_finished_upload) {
    upload ();
    return;
  }

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

    opengl::scoped::matrix_pusher const matrix_pusher;
    gl.translatef(pCamera.x(), pCamera.y(), pCamera.z());
    const float sc = 2.0f;
    gl.scalef(sc,sc,sc);
    skybox.get()->draw (world, clock() / CLOCKS_PER_SEC);
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

  pos = convert_coords (pos);

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

  lq = 0;
}

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

namespace
{
  struct scoped_material_setter
  {
    scoped_material_setter (WMOMaterial* material, bool vertex_colors)
      : _over_bright ((material->flags & 0x10) && !vertex_colors)
      , _specular (material->specular && !vertex_colors && !_over_bright)
      , _alpha_test ((material->transparent) != 0)
      , _back_face_cull (material->flags & 0x04)
    {
      if (_alpha_test)
      {
        gl.enable (GL_ALPHA_TEST);

        float aval = 0;

        if (material->flags & 0x80) aval = 0.3f;
        if (material->flags & 0x01) aval = 0.0f;

        gl.alphaFunc (GL_GREATER, aval);
      }

      if (_back_face_cull)
        gl.disable (GL_CULL_FACE);
      else
        gl.enable (GL_CULL_FACE);

      if (_specular)
      {
        gl.materialfv (GL_FRONT_AND_BACK, GL_SPECULAR, colorFromInt(material->col2));
      }
      else
      {
        ::math::vector_4d nospec(0, 0, 0, 1);
        gl.materialfv (GL_FRONT_AND_BACK, GL_SPECULAR, nospec);
      }

      if (_over_bright)
      {
        //! \todo  use emissive color from the WMO Material instead of 1,1,1,1
        GLfloat em[4] = {1,1,1,1};
        gl.materialfv (GL_FRONT, GL_EMISSION, em);
      }
    }

    ~scoped_material_setter()
    {
      if (_over_bright)
      {
        GLfloat em[4] = {0,0,0,1};
        gl.materialfv (GL_FRONT, GL_EMISSION, em);
      }

      if (_alpha_test)
      {
        gl.disable (GL_ALPHA_TEST);
      }
    }

    const bool _over_bright;
    const bool _specular;
    const bool _alpha_test;
    const bool _back_face_cull;
  };
}

void WMOGroup::initDisplayList()
{
  // generate lists for each batch individually instead
  _lists.resize( _batches.size() );

  // assume that texturing is on, for unit 1

  for (size_t b = 0; b < _batches.size (); b++)
  {
    _lists[b] = new opengl::call_list();

    _lists[b]->start_recording (GL_COMPILE);
    {
      wmo_batch* batch = &_batches[b];
      noggit::scoped_blp_texture_reference const& texture (wmo->_textures.at (batch->texture));
      scoped_material_setter const material_setter (&wmo->_materials.at (batch->texture), _vertex_colors.size ());

      texture->bind ();

      // render
      gl.begin (GL_TRIANGLES);
      for (int t = 0, i = batch->index_start; t < batch->index_count; t++, ++i) {
        int a = _indices[i];
        if (indoor && _vertex_colors.size ()) {
          setGLColor (_vertex_colors[a]);
        }
        gl.normal3f (_normals[a].x (), _normals[a].z (), -_normals[a].y ());
        gl.texCoord2fv (_texcoords[a]);
        gl.vertex3fv (_vertices[a]);
      }
      gl.end ();
    }
    _lists[b]->end_recording ();
  }

  _vertices.clear ();
  _normals.clear ();
  _texcoords.clear ();
  _vertex_colors.clear ();
  _indices.clear ();

  indoor = false;
}

void WMOGroup::load()
{
  // open group file
  std::stringstream curNum;
  curNum << "_" << std::setw (3) << std::setfill ('0') << num;

  std::string fname = wmo->filename ();
  fname.insert (fname.find (".wmo"), curNum.str ());

  noggit::mpq::file f (QString::fromStdString (fname));

  uint32_t fourcc;
  uint32_t size;

  int nLR = 0;
  uint16_t *useLights = nullptr;

  // - MVER ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  uint32_t version;

  f.read (&version, 4);

  assert (fourcc == 'MVER' && version == 17);

  // - MOGP ----------------------------------------------

  f.read (&fourcc, 4);
  f.seekRelative (4);

  assert (fourcc == 'MOGP');

  WMOGroupHeader header;

  f.read (&header, sizeof (WMOGroupHeader));

  WMOFog &wf = wmo->fogs[header.fogs[0]];
  if (wf.r2 <= 0) fog = -1; // default outdoor fog..?
  else fog = header.fogs[0];

  BoundingBoxMin = ::math::vector_3d (header.box1[0], header.box1[2], -header.box1[1]);
  BoundingBoxMax = ::math::vector_3d (header.box2[0], header.box2[2], -header.box2[1]);

  // - MOPY ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOPY');

  f.seekRelative (size);

  // - MOVI ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVI');

  _indices.resize (size / sizeof (uint16_t));

  f.read (_indices.data (), size);

  // - MOVT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOVT');

  // let's hope it's padded to 12 bytes, not 16...
  ::math::vector_3d *vertices = reinterpret_cast< ::math::vector_3d*>(f.getPointer ());

  VertexBoxMin = ::math::vector_3d (9999999.0f, 9999999.0f, 9999999.0f);
  VertexBoxMax = ::math::vector_3d (-9999999.0f, -9999999.0f, -9999999.0f);

  rad = 0;

  for (size_t i = 0; i < size / sizeof (::math::vector_3d); ++i) {
    ::math::vector_3d v (vertices[i].x (), vertices[i].z (), -vertices[i].y ());

    if (v.x () < VertexBoxMin.x ()) VertexBoxMin.x (v.x ());
    if (v.y () < VertexBoxMin.y ()) VertexBoxMin.y (v.y ());
    if (v.z () < VertexBoxMin.z ()) VertexBoxMin.z (v.z ());
    if (v.x () > VertexBoxMax.x ()) VertexBoxMax.x (v.x ());
    if (v.y () > VertexBoxMax.y ()) VertexBoxMax.y (v.y ());
    if (v.z () > VertexBoxMax.z ()) VertexBoxMax.z (v.z ());

    _vertices.push_back (v);
  }

  center = (VertexBoxMax + VertexBoxMin) * 0.5f;
  rad = (VertexBoxMax - center).length ();

  f.seekRelative (size);

  // - MONR ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MONR');

  _normals.resize (size / sizeof (::math::vector_3d));

  f.read (_normals.data (), size);

  // - MOTV ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOTV');

  _texcoords.resize (size / sizeof (::math::vector_2d));

  f.read (_texcoords.data (), size);

  // - MOBA ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOBA');

  _batches.resize (size / sizeof (wmo_batch));
  f.read (_batches.data (), size);

  // - MOLR ----------------------------------------------
  if (header.flags & 0x200)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOLR');

    nLR = size / 2;
    useLights = reinterpret_cast<uint16_t*>(f.getPointer ());

    f.seekRelative (size);
  }
  // - MODR ----------------------------------------------
  if (header.flags & 0x800)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MODR');

    _doodads.resize (size / sizeof (int16_t));

    f.read (_doodads.data (), size);
  }
  // - MOBN ----------------------------------------------
  if (header.flags & 0x1)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOBN');

    f.seekRelative (size);
  }
  // - MOBR ----------------------------------------------
  if (header.flags & 0x1)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOBR');

    f.seekRelative (size);
  }
  // - MPBV ----------------------------------------------
  if (header.flags & 0x400)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBV');

    f.seekRelative (size);
  }
  // - MPBP ----------------------------------------------
  if (header.flags & 0x400)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBP');

    f.seekRelative (size);
  }
  // - MPBI ----------------------------------------------
  if (header.flags & 0x400)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBI');

    f.seekRelative (size);
  }
  // - MPBG ----------------------------------------------
  if (header.flags & 0x400)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBG');

    f.seekRelative (size);
  }
  // - MOCV ----------------------------------------------
  if (header.flags & 0x4)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOCV');

    _vertex_colors.resize (size / sizeof (uint32_t));
    f.read (_vertex_colors.data (), size);
  }
  // - MLIQ ----------------------------------------------
  if (header.flags & 0x1000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MLIQ');

    //! \todo actually use this, the _right_ way

    f.seekRelative (size);
  }
  // - MORI ----------------------------------------------
  if (header.flags & 0x20000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MORI');

    f.seekRelative (size);
  }
  // - MORB ----------------------------------------------
  if (header.flags & 0x20000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MORB');

    f.seekRelative (size);
  }
  // - MOTV ----------------------------------------------
  if (header.flags & 0x2000000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MORI');

    f.seekRelative (size);
  }
  // - MOCV ----------------------------------------------
  if (header.flags & 0x1000000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MORI');

    f.seekRelative (size);
  }

  indoor = flags & 8192;
  initLighting (nLR, useLights);
}

void WMOGroup::initLighting(int /*nLR*/, uint16_t* /*useLights*/)
{
  //dl_light = 0;
  // "real" lighting?
  if ((flags & 0x2000) && _vertex_colors.size ())
  {
    ::math::vector_3d dirmin(1, 1, 1);
    float lenmin;
    int lmin;

    for (auto doodad : _doodads)
    {
      lenmin = 999999.0f * 999999.0f;
      lmin = 0;
      ModelInstance& mi = wmo->modelis[doodad];
      for (unsigned int j = 0; j < wmo->nLights; j++)
      {
        WMOLight& l = wmo->lights[j];
        ::math::vector_3d dir = l.pos - mi.pos;
        float ll = dir.length_squared ();
        if (ll < lenmin)
        {
          lenmin = ll;
          dirmin = dir;
          lmin = j;
        }
      }
      mi.ldir = dirmin;
    }

    outdoorLights = false;
  }
  else
  {
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

  if (_vertex_colors.size ())
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
  for (size_t i = 0; i < _batches.size (); ++i)
  {
    if (wmoShader)
    {
      wmoShader->bind();
      _lists[i]->render ();
      wmoShader->unbind();
    }
    else
    {
      _lists[i]->render ();
    }
  }

  gl.color4f(1,1,1,1);
  gl.enable(GL_CULL_FACE);

  if (_vertex_colors.size ())
      gl.enable(GL_LIGHTING);


}

void WMOGroup::draw_for_selection() const
{
  for (size_t i (0); i < _batches.size (); ++i)
  {
    _lists[i]->render ();
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
  if (_doodads.empty ())
    return;

  world->outdoorLights(outdoorLights);
  setupFog(world, draw_fog, fog_distance);

  // draw doodads
  gl.color4f(1,1,1,1);
  for (auto doodad : _doodads)
  {
    if (! (wmo->doodadsets.size() < doodadset))
    {
      if ((doodad >= wmo->doodadsets[doodadset].start) && (doodad < (wmo->doodadsets[doodadset].start + wmo->doodadsets[doodadset].size)))
      {
        ModelInstance& mi = wmo->modelis[doodad];

        if (!outdoorLights)
        {
          WMOLight::setupOnce (GL_LIGHT2, mi.ldir, mi.lcol);
        }

        setupFog (world, draw_fog, fog_distance);

        if (mi.is_visible (cull_distance, frustum, camera, ofs, rot))
        {
          mi.draw2();
        }
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
  if (_doodads.empty ())
    return;

  // draw doodads
  gl.color4f(1,1,1,1);
  for (auto doodad : _doodads)
  {
    if (! (wmo->doodadsets.size() < doodadset))
    {
      if ((doodad >= wmo->doodadsets[doodadset].start) && (doodad < (wmo->doodadsets[doodadset].start + wmo->doodadsets[doodadset].size)))
      {
        ModelInstance& mi = wmo->modelis[doodad];

        if (!outdoorLights)
        {
          WMOLight::setupOnce (GL_LIGHT2, mi.ldir, mi.lcol);
        }

        if (mi.is_visible (cull_distance, frustum, camera, ofs, rot))
        {
          mi.draw2Select ();
        }
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
  for(auto it = _lists.begin (); it != _lists.end (); ++it)
  {
    delete *it;
  }
  _lists.clear();

  if (lq)
  {
    delete lq;
    lq = nullptr;
  }
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
