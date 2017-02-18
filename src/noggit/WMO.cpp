// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>
#include <noggit/Environment.h>
#include <noggit/Log.h> // LogDebug
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMO.h>
#include <noggit/World.h>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

void WMOHighlight(math::vector_4d color)
{
  gl.disable(GL_ALPHA_TEST);
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.disable(GL_CULL_FACE);
  opengl::texture::set_active_texture (0);
  opengl::texture::disable_texture();
  opengl::texture::set_active_texture (1);
  opengl::texture::disable_texture();
  gl.color4fv(color);
  gl.materialfv(GL_FRONT, GL_EMISSION, color);
  gl.depthMask(GL_FALSE);
}

void WMOUnhighlight()
{
  gl.enable(GL_ALPHA_TEST);
  gl.disable(GL_BLEND);
  gl.enable(GL_CULL_FACE);
  opengl::texture::set_active_texture (0);
  opengl::texture::enable_texture();
  gl.color4fv(math::vector_4d(1, 1, 1, 1));
  gl.depthMask(GL_TRUE);
}

const std::string& WMO::filename() const
{
  return _filename;
}

WMO::WMO(const std::string& filenameArg)
  : _filename(filenameArg)
  , _finished_upload(false)
{
  finished = false;

  finishLoading();
}

void WMO::finishLoading ()
{
  MPQFile f(_filename);
  if (f.isEof()) {
    LogError << "Error loading WMO \"" << _filename << "\"." << std::endl;
    return;
  }

  LogDebug << "Loading WMO \"" << _filename << "\"." << std::endl;

  uint32_t fourcc;
  uint32_t size;

  float ff[3];

  char const* ddnames = nullptr;
  char const* groupnames = nullptr;

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

  unsigned int col, nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, nX;
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

  // - MOTX ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOTX');

  std::vector<char> texbuf (size);
  f.read (texbuf.data(), texbuf.size());

  // - MOMT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOMT');

  std::size_t const num_materials (size / 0x40);
  mat.resize (num_materials);

  for (size_t i (0); i < num_materials; ++i)
  {
    f.read (&mat[i], 0x40);

    std::string const texpath (texbuf.data() + mat[i].nameStart);
    textures.push_back (texpath);
  }

  // - MOGN ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOGN');

  groupnames = reinterpret_cast<char const*> (f.getPointer ());

  f.seekRelative (size);

  // - MOGI ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOGI');

  for (size_t i (0); i < nGroups; ++i) {
    groups.emplace_back (this, &f, i, groupnames);
  }

  // - MOSB ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOSB');

  if (size > 4)
  {
    std::string path = std::string (reinterpret_cast<char const*>(f.getPointer ()));
    if (path.length ())
    {
      LogDebug << "SKYBOX:" << std::endl;

      if (MPQFile::exists(path))
      {
        skybox = scoped_model_reference (path);
      }
    }
  }

  f.seekRelative (size);

  // - MOPV ----------------------------------------------

  f.read (&fourcc, 4);
  f.read(&size, 4);

  assert (fourcc == 'MOPV');

  std::vector<math::vector_3d> portal_vertices;

  for (size_t i (0); i < size / 12; ++i) {
    f.read (ff, 12);
    portal_vertices.push_back(math::vector_3d(ff[0], ff[2], -ff[1]));
  }

  // - MOPT ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MOPT');

  f.seekRelative (size);

  // - MOPR ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert(fourcc == 'MOPR');

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
    ddnames = reinterpret_cast<char const*> (f.getPointer ());
    f.seekRelative (size);
  }

  // - MODD ----------------------------------------------

  f.read (&fourcc, 4);
  f.read (&size, 4);

  assert (fourcc == 'MODD');

  for (size_t i (0); i < size / 0x28; ++i) {
    struct
    {
      uint32_t name_offset : 24;
      uint32_t flag_AcceptProjTex : 1;
      uint32_t flag_0x2 : 1;
      uint32_t flag_0x4 : 1;
      uint32_t flag_0x8 : 1;
      uint32_t flags_unused : 4;
    } x;

    size_t after_entry (f.getPos() + 0x28);
    f.read (&x, sizeof (x));

    modelis.push_back(ModelInstance (ddnames + x.name_offset, &f));

    f.seek (after_entry);
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

  for (auto& group : groups)
    group.load();

  finished = true;
}

void WMO::upload()
{
  for (unsigned int i = 0; i < mat.size(); ++i)
    mat[i]._texture = textures[i];

  for (auto& group : groups)
    group.upload ();

  _finished_upload = true;
}

// model.cpp
void DrawABox(math::vector_3d pMin, math::vector_3d pMax, math::vector_4d pColor, float pLineWidth);

void WMO::draw ( int doodadset
               , const math::vector_3d &ofs
               , math::degrees const angle
               , bool boundingbox
               , bool groupboxes
               , bool /*highlight*/
               , math::frustum const& frustum
               , bool draw_doodads
               )
{
  if (!finishedLoading ())
    return;

  if (!_finished_upload) {
    upload ();
    return;
  }

  if (gWorld && gWorld->drawfog)
    gl.enable(GL_FOG);
  else
    gl.disable(GL_FOG);

  for (auto& group : groups)
  {
    group.draw(ofs, angle, frustum);

    if (draw_doodads)
    {
      group.drawDoodads(doodadset, ofs, angle, frustum);
    }

    group.drawLiquid();
  }

  if (boundingbox)
  {
    gl.disable(GL_LIGHTING);

    gl.disable(GL_COLOR_MATERIAL);
    opengl::texture::set_active_texture (0);
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& group : groups)
      opengl::primitives::wire_box (group.BoundingBoxMin, group.BoundingBoxMax)
        .draw ({1.0f, 1.0f, 1.0f, 1.0f}, 1.0f);

    opengl::primitives::wire_box ( math::vector_3d(extents[0].x, extents[0].z, -extents[0].y)
                                 , math::vector_3d(extents[1].x, extents[1].z, -extents[1].y)
                                 ).draw ({1.0f, 0.0f, 0.0f, 1.0f}, 2.0f);

    /*gl.color4fv( math::vector_4d( 1.0f, 0.0f, 0.0f, 1.0f ) );
    gl.begin( GL_LINES );
    gl.vertex3f( 0.0f, 0.0f, 0.0f );
    gl.vertex3f( this->header.BoundingBoxMax.x + header.BoundingBoxMax.x / 5.0f, 0.0f, 0.0f );
    gl.end();

    gl.color4fv( math::vector_4d( 0.0f, 1.0f, 0.0f, 1.0f ) );
    gl.begin( GL_LINES );
    gl.vertex3f( 0.0f, 0.0f, 0.0f );
    gl.vertex3f( 0.0f, header.BoundingBoxMax.z + header.BoundingBoxMax.z / 5.0f, 0.0f );
    gl.end();

    gl.color4fv( math::vector_4d( 0.0f, 0.0f, 1.0f, 1.0f ) );
    gl.begin( GL_LINES );
    gl.vertex3f( 0.0f, 0.0f, 0.0f );
    gl.vertex3f( 0.0f, 0.0f, header.BoundingBoxMax.y + header.BoundingBoxMax.y / 5.0f );
    gl.end();*/

    opengl::texture::set_active_texture (1);
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture (0);
    opengl::texture::enable_texture();

    gl.enable(GL_LIGHTING);
  }
}

std::vector<float> WMO::intersect (math::ray const& ray) const
{
  std::vector<float> results;

  if (!finishedLoading ())
    return results;

  for (auto& group : groups)
  {
    group.intersect (ray, &results);
  }

  std::cout << _filename << " " << results.size() << "\n";

  return results;
}

bool WMO::drawSkybox(math::vector_3d pCamera, math::vector_3d pLower, math::vector_3d pUpper) const
{
  if (skybox && pCamera.is_inside_of(pLower, pUpper))
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
    opengl::scoped::matrix_pusher const matrix;
    math::vector_3d o = gWorld->camera;
    gl.translatef(o.x, o.y, o.z);
    const float sc = 2.0f;
    gl.scalef(sc, sc, sc);
    skybox.get()->draw();
    gl.enable(GL_DEPTH_TEST);

    return true;
  }

  return false;
}

void WMOLight::init(MPQFile* f)
{
  char type[4];
  f->read(&type, 4);
  f->read(&color, 4);
  f->read(pos, 12);
  f->read(&intensity, 4);
  f->read(unk, 4 * 5);
  f->read(&r, 4);

  pos = math::vector_3d(pos.x, pos.z, -pos.y);

  // rgb? bgr? hm
  float fa = ((color & 0xff000000) >> 24) / 255.0f;
  float fr = ((color & 0x00ff0000) >> 16) / 255.0f;
  float fg = ((color & 0x0000ff00) >> 8) / 255.0f;
  float fb = ((color & 0x000000ff)) / 255.0f;

  fcolor = math::vector_4d(fr, fg, fb, fa);
  fcolor *= intensity;
  fcolor.w = 1.0f;

  /*
  // light logging
  gLog("Light %08x @ (%4.2f,%4.2f,%4.2f)\t %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f\t(%d,%d,%d,%d)\n",
  color, pos.x, pos.y, pos.z, intensity,
  unk[0], unk[1], unk[2], unk[3], unk[4], r,
  type[0], type[1], type[2], type[3]);
  */
}

void WMOLight::setup(GLint light)
{
  // not used right now -_-

  GLfloat LightAmbient[] = { 0, 0, 0, 1.0f };
  GLfloat LightPosition[] = { pos.x, pos.y, pos.z, 0.0f };

  gl.lightfv(light, GL_AMBIENT, LightAmbient);
  gl.lightfv(light, GL_DIFFUSE, fcolor);
  gl.lightfv(light, GL_POSITION, LightPosition);

  gl.enable(light);
}

void WMOLight::setupOnce(GLint light, math::vector_3d dir, math::vector_3d lcol)
{
  math::vector_4d position(dir, 0);
  //math::vector_4d position(0,1,0,0);

  math::vector_4d ambient = math::vector_4d(lcol * 0.3f, 1);
  //math::vector_4d ambient = math::vector_4d(0.101961f, 0.062776f, 0, 1);
  math::vector_4d diffuse = math::vector_4d(lcol, 1);
  //math::vector_4d diffuse = math::vector_4d(0.439216f, 0.266667f, 0, 1);

  gl.lightfv(light, GL_AMBIENT, ambient);
  gl.lightfv(light, GL_DIFFUSE, diffuse);
  gl.lightfv(light, GL_POSITION, position);

  gl.enable(light);
}



WMOGroup::WMOGroup(WMO *_wmo, MPQFile* f, int _num, char const* names)
  : wmo(_wmo)
  , num(_num)
{
  // extract group info from f
  f->read(&flags, 4);
  float ff[3];
  f->read(ff, 12);
  VertexBoxMax = math::vector_3d(ff[0], ff[1], ff[2]);
  f->read(ff, 12);
  VertexBoxMin = math::vector_3d(ff[0], ff[1], ff[2]);
  int nameOfs;
  f->read(&nameOfs, 4);

  //! \todo  get proper name from group header and/or dbc?
  if (nameOfs > 0) {
    name = std::string(names + nameOfs);
  }
  else name = "(no name)";
}

void setGLColor(unsigned int col)
{
  //gl.color4ubv((GLubyte*)(&col));
  GLubyte r, g, b, a;
  a = (col & 0xFF000000) >> 24;
  r = (col & 0x00FF0000) >> 16;
  g = (col & 0x0000FF00) >> 8;
  b = (col & 0x000000FF);
  gl.color4ub(r, g, b, 1);
}

math::vector_4d colorFromInt(unsigned int col) {
  GLubyte r, g, b, a;
  a = (col & 0xFF000000) >> 24;
  r = (col & 0x00FF0000) >> 16;
  g = (col & 0x0000FF00) >> 8;
  b = (col & 0x000000FF);
  return math::vector_4d(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
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

void WMOGroup::upload()
{
  gl.genBuffers (1, &_vertices_buffer);
  gl.bufferData<GL_ARRAY_BUFFER> ( _vertices_buffer
                                 , _vertices.size() * sizeof (*_vertices.data())
                                 , _vertices.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.genBuffers (1, &_normals_buffer);
  gl.bufferData<GL_ARRAY_BUFFER> ( _normals_buffer
                                 , _normals.size() * sizeof (*_normals.data())
                                 , _normals.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.genBuffers (1, &_texcoords_buffer);
  gl.bufferData<GL_ARRAY_BUFFER> ( _texcoords_buffer
                                 , _texcoords.size() * sizeof (*_texcoords.data())
                                 , _texcoords.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.genBuffers (1, &_vertex_colors_buffer);
  gl.bufferData<GL_ARRAY_BUFFER> ( _vertex_colors_buffer
                                 , _vertex_colors.size() * sizeof (*_vertex_colors.data())
                                 , _vertex_colors.data()
                                 , GL_STATIC_DRAW
                                 );
}

void WMOGroup::load()
{
  // open group file
  std::stringstream curNum;
  curNum << "_" << std::setw (3) << std::setfill ('0') << num;

  std::string fname = wmo->filename ();
  fname.insert (fname.find (".wmo"), curNum.str ());

  MPQFile f(fname);
  if (f.isEof()) {
    LogError << "Error loading WMO \"" << fname << "\"." << std::endl;
    return;
  }

  uint32_t fourcc;
  uint32_t size;

  hascv = false;

  int nLR = 0;
  uint16_t const* useLights = nullptr;

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
  ::math::vector_3d const* vertices = reinterpret_cast< ::math::vector_3d const*>(f.getPointer ());

  VertexBoxMin = ::math::vector_3d (9999999.0f, 9999999.0f, 9999999.0f);
  VertexBoxMax = ::math::vector_3d (-9999999.0f, -9999999.0f, -9999999.0f);

  rad = 0;

  for (size_t i = 0; i < size / sizeof (::math::vector_3d); ++i) {
    ::math::vector_3d v (vertices[i].x, vertices[i].z, -vertices[i].y);

    if (v.x < VertexBoxMin.x) VertexBoxMin.x = v.x;
    if (v.y < VertexBoxMin.y) VertexBoxMin.y = v.y;
    if (v.z < VertexBoxMin.z) VertexBoxMin.z = v.z;
    if (v.x > VertexBoxMax.x) VertexBoxMax.x = v.x;
    if (v.y > VertexBoxMax.y) VertexBoxMax.y = v.y;
    if (v.z > VertexBoxMax.z) VertexBoxMax.z = v.z;

    _vertices.push_back (v);
  }

  center = (VertexBoxMax + VertexBoxMin) * 0.5f;
  rad = (VertexBoxMax - center).length () + 300.0f;;

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
    useLights = reinterpret_cast<uint16_t const*>(f.getPointer ());

    f.seekRelative (size);
  }
  // - MODR ----------------------------------------------
  if (header.flags & 0x800)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MODR');

    ddr.resize (size / sizeof (int16_t));

    f.read (ddr.data (), size);
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

    hascv = true;

    uint32_t const* colors = reinterpret_cast<uint32_t const*> (f.getPointer ());
    _vertex_colors.resize (size / sizeof (uint32_t));

    for(size_t i (0); i < size / sizeof (uint32_t); ++i)
    {
      _vertex_colors[i] = colorFromInt (colors[i]);
    }

    f.seekRelative (size);
  }
  // - MLIQ ----------------------------------------------
  if (header.flags & 0x1000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MLIQ');

    WMOLiquidHeader hlq;
    f.read(&hlq, 0x1E);

    lq = std::make_unique<wmo_liquid> (&f, hlq, wmo->mat[hlq.type], (flags & 0x2000) != 0);
  }
  if (header.flags & 0x20000)
  {
    // - MORI ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MORI');

    f.seekRelative (size);

    // - MORB ----------------------------------------------
    f.read(&fourcc, 4);
    f.read(&size, 4);

    assert(fourcc == 'MORB');

    f.seekRelative(size);
  }

  // - MOTV ----------------------------------------------
  if (header.flags & 0x2000000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOTV');

    f.seekRelative (size);
  }
  // - MOCV ----------------------------------------------
  if (header.flags & 0x1000000)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOCV');

    f.seekRelative (size);
  }

  indoor = flags & 8192;

  //dl_light = 0;
  // "real" lighting?
  if ((flags & 0x2000) && hascv)
  {
    ::math::vector_3d dirmin(1, 1, 1);
    float lenmin;
    int lmin;

    for (auto doodad : ddr)
    {
      lenmin = 999999.0f * 999999.0f;
      lmin = 0;
      ModelInstance& mi = wmo->modelis[doodad];
      for (unsigned int j = 0; j < wmo->lights.size(); j++)
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

void WMOGroup::draw(const math::vector_3d& ofs, const math::degrees angle, math::frustum const& frustum)
{
  visible = false;
  // view frustum culling

  math::vector_3d pos = center + ofs;

  math::rotate(ofs.x, ofs.z, &pos.x, &pos.z, angle);

  if (!frustum.intersectsSphere(pos, rad)) return;

  float dist = (pos - gWorld->camera).length() - rad;
  if (dist >= gWorld->culldistance) return;
  visible = true;
  setupFog();

  gl.vertexPointer (_vertices_buffer, 3, GL_FLOAT, 0, nullptr);
  gl.normalPointer (_normals_buffer, GL_FLOAT, 0, nullptr);
  gl.texCoordPointer (_texcoords_buffer, 2, GL_FLOAT, 0, nullptr);

  if (hascv)
  {
    if(indoor)
    {
      gl.enableClientState (GL_COLOR_ARRAY);
      gl.colorPointer (_vertex_colors_buffer, 4, GL_FLOAT, 0, 0);
    }

    gl.disable(GL_LIGHTING);
    gWorld->outdoorLights(false);
  }
  else
  {

    if (gWorld->skies->hasSkies())
    {
      gWorld->outdoorLights(true);
    }
    else gl.disable(GL_LIGHTING);
  }

  gl.disable(GL_BLEND);
  gl.color4f(1,1,1,1);

  for (wmo_batch& batch : _batches)
  {
    WMOMaterial* mat (&wmo->mat.at (batch.texture));
    scoped_material_setter const material_setter (mat, _vertex_colors.size ());

    mat->_texture.get()->bind();

    gl.drawRangeElements (GL_TRIANGLES, batch.vertex_start, batch.vertex_end, batch.index_count, GL_UNSIGNED_SHORT, _indices.data () + batch.index_start);
  }

  gl.color4f(1, 1, 1, 1);
  gl.enable(GL_CULL_FACE);

  if (hascv)
  {
    gl.disableClientState (GL_COLOR_ARRAY);
    gl.enable (GL_LIGHTING);
  }
}

void WMOGroup::intersect (math::ray const& ray, std::vector<float>* results) const
{
  if (!ray.intersect_bounds (VertexBoxMin, VertexBoxMax))
  {
    return;
  }

  //! \todo Also allow clicking on doodads and liquids.
  for (auto&& batch : _batches)
  {
    for (size_t i (batch.index_start); i < batch.index_start + batch.index_count; i += 3)
    {
      if ( auto&& distance
         = ray.intersect_triangle ( _vertices[_indices[i + 0]]
                                  , _vertices[_indices[i + 1]]
                                  , _vertices[_indices[i + 2]]
                                  )
         )
      {
        results->emplace_back (*distance);
      }
    }
  }
}

void WMOGroup::drawDoodads(unsigned int doodadset, const math::vector_3d& ofs, math::degrees const angle, math::frustum const& frustum)
{
  if (!visible) return;
  if (ddr.empty()) return;
  if (doodadset >= wmo->doodadsets.size()) return;

  gWorld->outdoorLights(outdoorLights);
  setupFog();

  /*
  float xr=0,xg=0,xb=0;
  if (flags & 0x0040) xr = 1;
  //if (flags & 0x0008) xg = 1;
  if (flags & 0x8000) xb = 1;
  gl.color4f(xr,xg,xb,1);
  */

  // draw doodads
  gl.color4f(1, 1, 1, 1);
  for (const auto& dd : ddr) {
    if ( dd >= wmo->doodadsets[doodadset].start
      && dd < (wmo->doodadsets[doodadset].start + wmo->doodadsets[doodadset].size)
      && dd < wmo->modelis.size()
      )
    {
      ModelInstance& mi = wmo->modelis[dd];

      if (!outdoorLights) {
        WMOLight::setupOnce(GL_LIGHT2, mi.ldir, mi.lcol);
      }
      setupFog();
      wmo->modelis[dd].draw_wmo(ofs, angle, frustum);
    }
  }

  gl.disable(GL_LIGHT2);

  gl.color4f(1, 1, 1, 1);

}

void WMOGroup::drawLiquid()
{
  if (!visible) return;

  // draw liquid
  //! \todo  culling for liquid boundingbox or something
  if (lq) {
    setupFog();
    if (outdoorLights) {
      gWorld->outdoorLights(true);
    }
    else {
      //! \todo  setup some kind of indoor lighting... ?
      gWorld->outdoorLights(false);
      gl.enable(GL_LIGHT2);
      gl.lightfv(GL_LIGHT2, GL_AMBIENT, math::vector_4d(0.1f, 0.1f, 0.1f, 1));
      gl.lightfv(GL_LIGHT2, GL_DIFFUSE, math::vector_4d(0.8f, 0.8f, 0.8f, 1));
      gl.lightfv(GL_LIGHT2, GL_POSITION, math::vector_4d(0, 1, 0, 0));
    }
    gl.disable(GL_BLEND);
    gl.disable(GL_ALPHA_TEST);
    gl.depthMask(GL_TRUE);
    gl.color4f(1, 1, 1, 1);
    lq->draw();
    gl.disable(GL_LIGHT2);
  }
}

void WMOGroup::setupFog()
{
  if (outdoorLights || fog == -1) {
    gWorld->setupFog();
  }
  else {
    wmo->fogs[fog].setup();
  }
}

void WMOFog::init(MPQFile* f)
{
  f->read(this, 0x30);
  color = math::vector_4d(((color1 & 0x00FF0000) >> 16) / 255.0f, ((color1 & 0x0000FF00) >> 8) / 255.0f,
    (color1 & 0x000000FF) / 255.0f, ((color1 & 0xFF000000) >> 24) / 255.0f);
  float temp;
  temp = pos.y;
  pos.y = pos.z;
  pos.z = -temp;
  fogstart = fogstart * fogend * 1.5f;
  fogend *= 1.5;
}

void WMOFog::setup()
{
  /*if (gWorld->drawfog) {
  gl.fogfv(GL_FOG_COLOR, color);
  gl.fogf(GL_FOG_START, fogstart);
  gl.fogf(GL_FOG_END, fogend);

  gl.enable(GL_FOG);
  } else {
  gl.disable(GL_FOG);
  }*/
  gl.disable(GL_FOG);
}

decltype (WMOManager::_) WMOManager::_;

void WMOManager::report()
{
  std::string output = "Still in the WMO manager:\n";
  _.apply ( [&] (std::string const& key, WMO const&)
            {
              output += " - " + key + "\n";
            }
          );
  LogDebug << output;
}
