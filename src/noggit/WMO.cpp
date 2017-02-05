// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Environment.h>
#include <noggit/Frustum.h>
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
{
  MPQFile f(_filename);
  if (f.isEof()) {
    LogError << "Error loading WMO \"" << _filename << "\"." << std::endl;
    return;
  }

  uint32_t fourcc;
  uint32_t size;
  float ff[3];

  char *ddnames = nullptr;
  char *groupnames = nullptr;

  char *texbuf = 0;

  while (!f.isEof()) {
    f.read(&fourcc, 4);
    f.read(&size, 4);

    size_t nextpos = f.getPos() + size;

    if (fourcc == 'MOHD') {
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
      f.read(ff, 12);
      extents[0] = math::vector_3d(ff[0], ff[1], ff[2]);
      f.read(ff, 12);
      extents[1] = math::vector_3d(ff[0], ff[1], ff[2]);

      groups.resize (nGroups);
    }
    else if (fourcc == 'MOTX') {
      // textures
      texbuf = new char[size];
      f.read(texbuf, size);
    }
    else if (fourcc == 'MOMT')
    {
      std::size_t const num_materials (size / 0x40);
      mat.resize (num_materials);

      for (std::size_t i (0); i < num_materials; ++i)
      {
        f.read (&mat[i], 0x40);

        std::string const texpath (texbuf + mat[i].nameStart);

        mat[i]._texture = texpath;
        textures.push_back (texpath);
      }
    }
    else if (fourcc == 'MOGN') {
      groupnames = reinterpret_cast<char*>(f.getPointer());
    }
    else if (fourcc == 'MOGI') {
      // group info - important information! ^_^
      for (unsigned int i = 0; i<nGroups; ++i) {
        groups[i].init(this, &f, i, groupnames);

      }
    }
    else if (fourcc == 'MOLT') {
      // Lights?
      for (unsigned int i = 0; i<nLights; ++i) {
        WMOLight l;
        l.init(&f);
        lights.push_back(l);
      }
    }
    else if (fourcc == 'MODN') {
      // models ...
      // MMID would be relative offsets for MMDX filenames
      if (size) {

        ddnames = reinterpret_cast<char*>(f.getPointer());

        f.seekRelative(size);
      }
    }
    else if (fourcc == 'MODS') {
      for (unsigned int i = 0; i<nDoodadSets; ++i) {
        WMODoodadSet dds;
        f.read(&dds, 32);
        doodadsets.push_back(dds);
      }
    }
    else if (fourcc == 'MODD') {
      nModels = size / 0x28;
      for (unsigned int i = 0; i<nModels; ++i) {
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

    }
    else if (fourcc == 'MOSB')
    {
      if (size>4)
      {
        std::string path = std::string(reinterpret_cast<char*>(f.getPointer()));
        if (path.length())
        {
          LogDebug << "SKYBOX:" << std::endl;

          if (MPQFile::exists(path))
          {
            skybox = scoped_model_reference (path);
          }
        }
      }
    }
    else if (fourcc == 'MOPV') {
      WMOPV p;
      for (unsigned int i = 0; i<nP; ++i) {
        f.read(ff, 12);
        p.a = math::vector_3d(ff[0], ff[2], -ff[1]);
        f.read(ff, 12);
        p.b = math::vector_3d(ff[0], ff[2], -ff[1]);
        f.read(ff, 12);
        p.c = math::vector_3d(ff[0], ff[2], -ff[1]);
        f.read(ff, 12);
        p.d = math::vector_3d(ff[0], ff[2], -ff[1]);
        pvs.push_back(p);
      }
    }
    else if (fourcc == 'MOPR') {
      int nn = size / 8;
      WMOPR *pr = reinterpret_cast<WMOPR*>(f.getPointer());
      for (int i = 0; i<nn; ++i) {
        prs.push_back(*pr++);
      }
    }
    else if (fourcc == 'MFOG') {
      int nfogs = size / 0x30;
      for (int i = 0; i<nfogs; ++i) {
        WMOFog fog;
        fog.init(&f);
        fogs.push_back(fog);
      }
    }

    f.seek(nextpos);
  }

  f.close();
  if (texbuf)
  {
    delete[] texbuf;
    texbuf = nullptr;
  }

  for (unsigned int i = 0; i<nGroups; ++i)
    groups[i].initDisplayList();
}

// model.cpp
void DrawABox(math::vector_3d pMin, math::vector_3d pMax, math::vector_4d pColor, float pLineWidth);

void WMO::draw(int doodadset, const math::vector_3d &ofs, math::degrees const angle, bool boundingbox, bool groupboxes, bool /*highlight*/, Frustum const& frustum)
{
  if (gWorld && gWorld->drawfog)
    gl.enable(GL_FOG);
  else
    gl.disable(GL_FOG);

  for (unsigned int i = 0; i<nGroups; ++i)
  {
    groups[i].draw(ofs, angle, frustum);

    if (gWorld->drawdoodads)
    {
      groups[i].drawDoodads(doodadset, ofs, angle, frustum);
    }

    groups[i].drawLiquid();
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

    for (unsigned int i = 0; i < nGroups; ++i)
      opengl::primitives::wire_box (groups[i].BoundingBoxMin, groups[i].BoundingBoxMax)
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

  for (size_t i (0); i < nGroups; ++i)
  {
    groups[i].intersect (ray, &results);
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



void WMOGroup::init(WMO *_wmo, MPQFile* f, int _num, char *names)
{
  this->wmo = _wmo;
  this->num = _num;

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

  nDoodads = 0;
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

void WMOGroup::initDisplayList()
{
  math::vector_3d *normals = nullptr;
  math::vector_2d *texcoords = nullptr;
  struct SMOPoly *materials = nullptr;

  WMOGroupHeader gh;

  uint16_t *useLights = 0;
  int nLR = 0;


  // open group file

  std::stringstream curNum;
  curNum << "_" << std::setw(3) << std::setfill('0') << num;

  std::string fname = wmo->filename();
  fname.insert(fname.find(".wmo"), curNum.str());

  MPQFile gf(fname);
  if (gf.isEof()) {
    LogError << "Error loading WMO \"" << fname << "\"." << std::endl;
    return;
  }

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

  BoundingBoxMin = math::vector_3d(gh.box1[0], gh.box1[2], -gh.box1[1]);
  BoundingBoxMax = math::vector_3d(gh.box2[0], gh.box2[2], -gh.box2[1]);

  gf.seek(0x58); // first chunk

  uint32_t fourcc;
  uint32_t size;

  unsigned int *cv = nullptr;
  hascv = false;

  while (!gf.isEof()) {
    gf.read(&fourcc, 4);
    gf.read(&size, 4);

    size_t nextpos = gf.getPos() + size;

    // why copy stuff when I can just map it from memory ^_^

    if (fourcc == 'MOPY') {
      // materials per triangle
      nTriangles = size / 2;
      materials = reinterpret_cast<SMOPoly*>(gf.getPointer());
    }
    else if (fourcc == 'MOVI') {
      // indices
      auto nIndices (size / 2);
      auto indices_raw = reinterpret_cast<uint16_t*>(gf.getPointer());
      indices.insert (indices.end(), indices_raw, indices_raw + nIndices);
    }
    else if (fourcc == 'MOVT') {
      nVertices = size / 12;
      // let's hope it's padded to 12 bytes, not 16...
      auto vertices_raw = reinterpret_cast<math::vector_3d*>(gf.getPointer());
      vertices.insert (vertices.end(), vertices_raw, vertices_raw + nVertices);
      VertexBoxMin = math::vector_3d(9999999.0f, 9999999.0f, 9999999.0f);
      VertexBoxMax = math::vector_3d(-9999999.0f, -9999999.0f, -9999999.0f);
      rad = 0;
      for (size_t i = 0; i<nVertices; ++i) {
        math::vector_3d v(vertices[i].x, vertices[i].z, -vertices[i].y);
        if (v.x < VertexBoxMin.x) VertexBoxMin.x = v.x;
        if (v.y < VertexBoxMin.y) VertexBoxMin.y = v.y;
        if (v.z < VertexBoxMin.z) VertexBoxMin.z = v.z;
        if (v.x > VertexBoxMax.x) VertexBoxMax.x = v.x;
        if (v.y > VertexBoxMax.y) VertexBoxMax.y = v.y;
        if (v.z > VertexBoxMax.z) VertexBoxMax.z = v.z;
      }
      center = (VertexBoxMax + VertexBoxMin) * 0.5f;
      rad = (VertexBoxMax - center).length() + 300.0f;
    }
    else if (fourcc == 'MONR') {
      normals = reinterpret_cast<math::vector_3d*>(gf.getPointer());
    }
    else if (fourcc == 'MOTV') {
      texcoords = reinterpret_cast<math::vector_2d*>(gf.getPointer());
    }
    else if (fourcc == 'MOLR') {
      nLR = size / 2;
      useLights = reinterpret_cast<uint16_t*>(gf.getPointer());
    }
    else if (fourcc == 'MODR') {
      nDoodads = size / 2;
      ddr.resize (nDoodads);
      gf.read(ddr.data(), size);
    }
    else if (fourcc == 'MOBA') {
      nBatches = size / 24;
      auto batches_raw = reinterpret_cast<WMOBatch*>(gf.getPointer());
      batches.insert (batches.end(), batches_raw, batches_raw + nBatches);
    }
    else if (fourcc == 'MOCV') {
      //gLog("CV: %d\n", size);
      hascv = true;
      cv = reinterpret_cast<uint32_t*>(gf.getPointer());
    }
    else if (fourcc == 'MLIQ') {
      // liquids
      WMOLiquidHeader hlq;
      gf.read(&hlq, 0x1E);

      lq = std::make_unique<wmo_liquid> (&gf, hlq, wmo->mat[hlq.type], (flags & 0x2000) != 0);
    }

    //! \todo  figure out/use MFOG ?

    gf.seek(nextpos);
  }

  // ok, make a display list

  indoor = (flags & 8192) != 0;
  //gLog("Lighting: %s %X\n\n", indoor?"Indoor":"Outdoor", flags);

  initLighting(nLR, useLights);

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
  _lists.resize(nBatches);

  // assume that texturing is on, for unit 1

  for (int b = 0; b<nBatches; b++)
  {
    WMOBatch *batch = &batches[b];
    WMOMaterial *mat = &wmo->mat[batch->texture];

    bool overbright = ((mat->flags & 0x10) && !hascv);
    bool spec_shader = (mat->specular && !hascv && !overbright);

    _lists[b].first = std::make_unique<opengl::call_list>();
    _lists[b].second = spec_shader;

    _lists[b].first->start_recording(GL_COMPILE);

    mat->_texture.get()->bind();

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
    }
    else {
      math::vector_4d nospec(0, 0, 0, 1);
      gl.materialfv(GL_FRONT_AND_BACK, GL_SPECULAR, nospec);
    }

    if (overbright) {
      //! \todo  use emissive color from the WMO Material instead of 1,1,1,1
      GLfloat em[4] = { 1, 1, 1, 1 };
      gl.materialfv(GL_FRONT, GL_EMISSION, em);
    }

    // render
    gl.begin(GL_TRIANGLES);
    for (int t = 0, i = batch->indexStart; t<batch->indexCount; t++, ++i) {
      int a = indices[i];
      if (indoor && hascv) {
        setGLColor(cv[a]);
      }
      gl.normal3f(normals[a].x, normals[a].z, -normals[a].y);
      gl.texCoord2fv(texcoords[a]);
      gl.vertex3f(vertices[a].x, vertices[a].z, -vertices[a].y);
    }
    gl.end();

    if (overbright) {
      GLfloat em[4] = { 0, 0, 0, 1 };
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

    math::vector_3d dirmin(1, 1, 1);
    float lenmin;
    int lmin;

    for (int i = 0; i<nDoodads; ++i) {
      lenmin = 999999.0f*999999.0f;
      lmin = 0;
      ModelInstance &mi = wmo->modelis[ddr[i]];
      for (unsigned int j = 0; j<wmo->nLights; j++) {
        WMOLight &l = wmo->lights[j];
        math::vector_3d dir = l.pos - mi.pos;
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
  }
  else {
    outdoorLights = true;
  }
}

void WMOGroup::draw(const math::vector_3d& ofs, const math::degrees angle, Frustum const& frustum)
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

  if (hascv) {
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


  //gl.callList(dl);
  gl.disable(GL_BLEND);
  gl.color4f(1, 1, 1, 1);
  for (int i = 0; i<nBatches; ++i)
  {
    _lists[i].first->render();
  }

  gl.color4f(1, 1, 1, 1);
  gl.enable(GL_CULL_FACE);

  if (hascv && gWorld->lighting)
    gl.enable(GL_LIGHTING);
}

void WMOGroup::intersect (math::ray const& ray, std::vector<float>* results) const
{
  if (!ray.intersect_bounds (VertexBoxMin, VertexBoxMax))
  {
    return;
  }

  //! \todo Also allow clicking on doodads and liquids.
  for (auto&& batch : batches)
  {
    for (size_t i (batch.indexStart); i < batch.indexStart + batch.indexCount; i += 3)
    {
      if ( auto&& distance
         = ray.intersect_triangle ( vertices[indices[i + 0]]
                                  , vertices[indices[i + 1]]
                                  , vertices[indices[i + 2]]
                                  )
         )
      {
        results->emplace_back (*distance);
      }
    }
  }
}

void WMOGroup::drawDoodads(unsigned int doodadset, const math::vector_3d& ofs, math::degrees const angle, Frustum const& frustum)
{

  if (!visible) return;
  if (nDoodads == 0) return;
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
  for (int i = 0; i<nDoodads; ++i) {
    int16_t dd = ddr[i];
    if ( dd >= wmo->doodadsets[doodadset].start
      && dd < (wmo->doodadsets[doodadset].start + wmo->doodadsets[doodadset].size)
      && dd < wmo->modelis.size()
      )
    {
      ModelInstance &mi = wmo->modelis[dd];

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
