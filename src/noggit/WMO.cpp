// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/frustum.hpp>
#include <noggit/AsyncLoader.h>
#include <noggit/Log.h> // LogDebug
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMO.h>
#include <noggit/World.h>
#include <opengl/primitives.hpp>
#include <opengl/scoped.hpp>

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


WMO::WMO(const std::string& filenameArg)
  : AsyncObject(filenameArg)
  , _finished_upload(false)
{
}

void WMO::finishLoading ()
{
  MPQFile f(filename);
  if (f.isEof()) {
    LogError << "Error loading WMO \"" << filename << "\"." << std::endl;
    return;
  }

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
  materials.resize (num_materials);

  std::vector<int> texture_ofs;

  for (size_t i (0); i < num_materials; ++i)
  {
    f.read (&materials[i], sizeof(WMOMaterial));

    auto it = std::find(texture_ofs.begin(), texture_ofs.end(), materials[i].texture_offset_1);

    // texture not loaded
    if (it == texture_ofs.end())
    {
      materials[i].texture1 = textures.size();
      texture_ofs.emplace_back(materials[i].texture_offset_1);
      std::string const texpath (texbuf.data() + materials[i].texture_offset_1);
      textures.emplace_back(texpath);
    }
    else
    {
      materials[i].texture1 = std::distance(texture_ofs.begin(), it);
    }

    // todo: load and use the 2nd texture if there's one
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
    std::string path = noggit::mpq::normalized_filename(std::string (reinterpret_cast<char const*>(f.getPointer ())));
    boost::replace_all(path, "mdx", "m2");

    if (path.length())
    {
      if (MPQFile::exists(path))
      {
        skybox = scoped_model_reference(path);
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

    modelis.emplace_back(ddnames + x.name_offset, &f);
    model_nearest_light_vector.emplace_back();

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

void WMO::draw ( opengl::scoped::use_program& wmo_shader
               , math::matrix_4x4 const& model_view
               , math::matrix_4x4 const& projection
               , math::matrix_4x4 const& transform
               , int doodadset
               , const math::vector_3d &ofs
               , math::degrees const angle
               , bool boundingbox
               , math::frustum const& frustum
               , const float& cull_distance
               , const math::vector_3d& camera
               , bool draw_doodads
               , bool draw_fog
               , math::vector_4d const& ocean_color_light
               , math::vector_4d const& ocean_color_dark
               , math::vector_4d const& river_color_light
               , math::vector_4d const& river_color_dark
               , liquid_render& render
               , int animtime
               , bool world_has_skies
               , display_mode display
               )
{ 
  for (auto& group : groups)
  {
    if (!group.is_visible(ofs, angle, frustum, cull_distance, camera, display))
    {
      continue;
    }

    group.draw ( wmo_shader
               , frustum
               , cull_distance
               , camera
               , draw_fog
               , world_has_skies
               );

    group.drawLiquid ( model_view
                     , projection
                     , transform
                     , ocean_color_light
                     , ocean_color_dark
                     , river_color_light
                     , river_color_dark
                     , render
                     , draw_fog
                     , animtime
                     );
  }

  if (boundingbox)
  {
    opengl::scoped::bool_setter<GL_BLEND, TRUE> const blend;
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto& group : groups)
      opengl::primitives::wire_box (group.BoundingBoxMin, group.BoundingBoxMax)
        .draw (model_view, projection, transform, {1.0f, 1.0f, 1.0f, 1.0f}, 1.0f);

    opengl::primitives::wire_box ( math::vector_3d(extents[0].x, extents[0].z, -extents[0].y)
                                 , math::vector_3d(extents[1].x, extents[1].z, -extents[1].y)
                                 ).draw ( model_view
                                        , projection
                                        , transform
                                        , {1.0f, 0.0f, 0.0f, 1.0f}
                                        , 2.0f
                                        );

  }
}

std::vector<float> WMO::intersect (math::ray const& ray) const
{
  std::vector<float> results;

  if (!finishedLoading() || loading_failed())
  {
    return results;
  }

  for (auto& group : groups)
  {
    group.intersect (ray, &results);
  }

  std::cout << filename << " " << results.size() << "\n";

  return results;
}

bool WMO::draw_skybox ( math::matrix_4x4 const& model_view
                      , math::vector_3d const& camera_pos
                      , opengl::scoped::use_program& m2_shader
                      , math::frustum const& frustum
                      , const float& cull_distance
                      , int animtime
                      , bool draw_particles
                      , math::vector_3d aabb_min
                      , math::vector_3d aabb_max
                      , std::map<int, std::pair<math::vector_3d, math::vector_3d>> const& group_extents
                      ) const
{
  if (!skybox || !camera_pos.is_inside_of(aabb_min, aabb_max))
  {
    return false;
  }

  for (int i=0; i<groups.size(); ++i)
  {
    auto const& g = groups[i];

    if (!g.has_skybox())
    {
      continue;
    }

    auto& extent(group_extents.at(i));

    if (camera_pos.is_inside_of(extent.first, extent.second))
    {
      ModelInstance sky(skybox.get()->filename);
      sky.pos = camera_pos;
      sky.scale = 2.f;
      sky.recalcExtents();

      skybox->get()->draw(model_view, sky, m2_shader, frustum, cull_distance, camera_pos, animtime, draw_particles, false, display_mode::in_3D);

      return true;
    }
  }  

  return false;
}

std::map<uint32_t, std::vector<wmo_doodad_instance>> WMO::doodads_per_group(uint16_t doodadset) const
{
  std::map<uint32_t, std::vector<wmo_doodad_instance>> doodads;

  if (doodadset >= doodadsets.size())
  {
    LogError << "Invalid doodadset for instance of wmo " << filename << std::endl;
    return doodads;
  }

  auto const& dset = doodadsets[doodadset];
  uint32_t start = dset.start, end = start + dset.size;

  for (int i = 0; i < groups.size(); ++i)
  {
    for (uint16_t ref : groups[i].doodad_ref())
    {
      if (ref >= start && ref < end)
      {
        doodads[i].push_back(modelis[ref]);
      }
    }
  }

  return doodads;
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
}

void WMOLight::setupOnce(GLint light, math::vector_3d dir, math::vector_3d light_color)
{
  math::vector_4d position(dir, 0);
  //math::vector_4d position(0,1,0,0);

  math::vector_4d ambient = math::vector_4d(light_color * 0.3f, 1);
  //math::vector_4d ambient = math::vector_4d(0.101961f, 0.062776f, 0, 1);
  math::vector_4d diffuse = math::vector_4d(light_color, 1);
  //math::vector_4d diffuse = math::vector_4d(0.439216f, 0.266667f, 0, 1);


  gl.enable(light);
}



WMOGroup::WMOGroup(WMO *_wmo, MPQFile* f, int _num, char const* names)
  : wmo(_wmo)
  , num(_num)
{
  // extract group info from f
  std::uint32_t flags; // not used, the flags are in the group header
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

WMOGroup::WMOGroup(WMOGroup const& other)
  : BoundingBoxMin(other.BoundingBoxMin)
  , BoundingBoxMax(other.BoundingBoxMax)
  , VertexBoxMin(other.VertexBoxMin)
  , VertexBoxMax(other.VertexBoxMax)
  , use_outdoor_lights(other.use_outdoor_lights)
  , name(other.name)
  , wmo(other.wmo)
  , header(other.header)
  , center(other.center)
  , rad(other.rad)
  , num(other.num)
  , fog(other.fog)
  , _doodad_ref(other._doodad_ref)
  , _batches(other._batches)
  , _vertices(other._vertices)
  , _normals(other._normals)
  , _texcoords(other._texcoords)
  , _vertex_colors(other._vertex_colors)
  , _indices(other._indices)
{
  if (other.lq)
  {
    lq = std::make_unique<wmo_liquid>(*other.lq.get());
  }
}

namespace
{
  math::vector_4d colorFromInt(unsigned int col)
  {
    GLubyte r, g, b, a;
    a = (col & 0xFF000000) >> 24;
    r = (col & 0x00FF0000) >> 16;
    g = (col & 0x0000FF00) >> 8;
    b = (col & 0x000000FF);
    return math::vector_4d(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
  }
}

void WMOGroup::upload()
{
  _vertex_array.upload();
  _buffers.upload();

  gl.bufferData<GL_ARRAY_BUFFER> ( _vertices_buffer
                                 , _vertices.size() * sizeof (*_vertices.data())
                                 , _vertices.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.bufferData<GL_ARRAY_BUFFER> ( _normals_buffer
                                 , _normals.size() * sizeof (*_normals.data())
                                 , _normals.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.bufferData<GL_ARRAY_BUFFER> ( _texcoords_buffer
                                 , _texcoords.size() * sizeof (*_texcoords.data())
                                 , _texcoords.data()
                                 , GL_STATIC_DRAW
                                 );

  gl.bufferData<GL_ARRAY_BUFFER> ( _vertex_colors_buffer
                                 , _vertex_colors.size() * sizeof (*_vertex_colors.data())
                                 , _vertex_colors.data()
                                 , GL_STATIC_DRAW
                                 );

  _uploaded = true;
}

void WMOGroup::setup_vao(opengl::scoped::use_program& wmo_shader)
{
  opengl::scoped::vao_binder const _ (_vao);

  wmo_shader.attrib("position", _vertices_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);
  wmo_shader.attrib("texcoord", _texcoords_buffer, 2, GL_FLOAT, GL_FALSE, 0, 0);

  if (header.flags.has_vertex_color)
  {
    wmo_shader.attrib("vertex_color", _vertex_colors_buffer, 4, GL_FLOAT, GL_FALSE, 0, 0);
  }

  _vao_is_setup = true;
}

void WMOGroup::load()
{
  // open group file
  std::stringstream curNum;
  curNum << "_" << std::setw (3) << std::setfill ('0') << num;

  std::string fname = wmo->filename;
  fname.insert (fname.find (".wmo"), curNum.str ());

  MPQFile f(fname);
  if (f.isEof()) {
    LogError << "Error loading WMO \"" << fname << "\"." << std::endl;
    return;
  }

  uint32_t fourcc;
  uint32_t size;

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

  f.read (&header, sizeof (wmo_group_header));

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
  if (header.flags.has_light)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOLR');

    f.seekRelative (size);
  }
  // - MODR ----------------------------------------------
  if (header.flags.has_doodads)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MODR');

    _doodad_ref.resize (size / sizeof (int16_t));

    f.read (_doodad_ref.data (), size);
  }
  // - MOBN ----------------------------------------------
  if (header.flags.has_bsp_tree)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOBN');

    f.seekRelative (size);
  }
  // - MOBR ----------------------------------------------
  if (header.flags.has_bsp_tree)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOBR');

    f.seekRelative (size);
  }
  
  if (header.flags.flag_0x400)
  {
    // - MPBV ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBV');

    f.seekRelative (size);

    // - MPBP ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBP');

    f.seekRelative (size);

    // - MPBI ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBI');

    f.seekRelative (size);

    // - MPBG ----------------------------------------------
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MPBG');

    f.seekRelative (size);
  }
  // - MOCV ----------------------------------------------
  if (header.flags.has_vertex_color)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOCV');

    uint32_t const* colors = reinterpret_cast<uint32_t const*> (f.getPointer ());
    _vertex_colors.resize (size / sizeof (uint32_t));

    for(size_t i (0); i < size / sizeof (uint32_t); ++i)
    {
      _vertex_colors[i] = colorFromInt (colors[i]);
    }

    f.seekRelative (size);
  }
  // - MLIQ ----------------------------------------------
  if (header.flags.has_water)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MLIQ');

    WMOLiquidHeader hlq;
    f.read(&hlq, 0x1E);

    lq = std::make_unique<wmo_liquid> (&f, hlq, wmo->materials[hlq.type], header.flags.indoor != 0);
  }
  if (header.flags.has_mori_morb)
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
  if (header.flags.has_two_motv)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOTV');

    f.seekRelative (size);
  }
  // - MOCV ----------------------------------------------
  if (header.flags.has_two_mocv)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOCV');

    f.seekRelative (size);
  }

  //dl_light = 0;
  // "real" lighting?
  if (header.flags.indoor && header.flags.has_vertex_color)
  {
    ::math::vector_3d dirmin(1, 1, 1);
    float lenmin;

    for (auto doodad : _doodad_ref)
    {
      lenmin = 999999.0f * 999999.0f;
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
        }
      }
      wmo->model_nearest_light_vector[doodad] = dirmin;
    }

    use_outdoor_lights = false;
  }
  else
  {
    use_outdoor_lights = true;
  }
}

bool WMOGroup::is_visible( math::vector_3d const& ofs
                         , math::degrees const& angle
                         , math::frustum const& frustum
                         , float const& cull_distance
                         , math::vector_3d const& camera
                         , display_mode display
                         ) const
{
  math::vector_3d pos = center + ofs;

  math::rotate(ofs.x, ofs.z, &pos.x, &pos.z, angle);

  if (!frustum.intersectsSphere(pos, rad))
  {
    return false;
  }

  float dist = display == display_mode::in_3D 
             ? (pos - camera).length() - rad
             : std::abs(pos.y - camera.y) - rad;

  return (dist < cull_distance);
}

void WMOGroup::draw( opengl::scoped::use_program& wmo_shader
                   , math::frustum const& frustum
                   , const float& cull_distance
                   , const math::vector_3d& camera
                   , bool draw_fog
                   , bool world_has_skies
                   )
{
  if (!_uploaded)
  {
    upload();
  }

  if (!_vao_is_setup)
  {
    setup_vao(wmo_shader);
  }

  wmo_shader.uniform("use_vertex_color", (int)header.flags.has_vertex_color);

  opengl::scoped::vao_binder const _ (_vao);

  for (wmo_batch& batch : _batches)
  {
    WMOMaterial const& mat (wmo->materials.at (batch.texture));
    
    float alpha_test = 0.003921568f; // 1/255

    switch (mat.blend_mode)
    {
      case 1:
        gl.disable(GL_BLEND);
        alpha_test = 0.878431372f; // 224/255
        break;
      case 2:
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
      case 3:
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
      case 4:
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_DST_COLOR, GL_ZERO);
        break;
      case 5:
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        break;
      case 6:
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_DST_COLOR, GL_ONE);
        break;
      case 0:
      default:
        alpha_test = -1.f;
        gl.disable(GL_BLEND);
        break;
    }    

    wmo_shader.uniform("alpha_test", alpha_test);
    wmo_shader.uniform("unfogged", (int)mat.flags.unfogged);

    if (mat.flags.unculled)
    {
      gl.disable(GL_CULL_FACE);
    }
    else
    {
      gl.enable(GL_CULL_FACE);
    }

    opengl::texture::set_active_texture(0);
    wmo->textures[mat.texture1]->bind();

    gl.drawRangeElements (GL_TRIANGLES, batch.vertex_start, batch.vertex_end, batch.index_count, GL_UNSIGNED_SHORT, _indices.data () + batch.index_start);
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

void WMOGroup::drawLiquid ( math::matrix_4x4 const& model_view
                          , math::matrix_4x4 const& projection
                          , math::matrix_4x4 const& transform
                          , math::vector_4d const& ocean_color_light
                          , math::vector_4d const& ocean_color_dark
                          , math::vector_4d const& river_color_light
                          , math::vector_4d const& river_color_dark
                          , liquid_render& render
                          , bool draw_fog
                          , int animtime
                          )
{
  // draw liquid
  //! \todo  culling for liquid boundingbox or something
  if (lq) 
  { 
    gl.disable(GL_BLEND);
    gl.depthMask(GL_TRUE);

    lq->draw ( model_view
             , projection
             , transform
             , ocean_color_light
             , ocean_color_dark
             , river_color_light
             , river_color_dark
             , render
             , animtime
             );
  }
}

void WMOGroup::setupFog (bool draw_fog, std::function<void (bool)> setup_fog)
{
  if (use_outdoor_lights || fog == -1) {
    setup_fog (draw_fog);
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

void WMOManager::clear_hidden_wmos()
{
  _.apply ( [&] (std::string const& key, WMO& wmo)
            {
              wmo.show();
            }
          );
}
