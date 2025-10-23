// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>
#include <math/frustum.hpp>
#include <noggit/AsyncLoader.h>
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
#include <regex>
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

  CArgb ambient_color;
  unsigned int nTextures, nGroups, nP, nLights, nModels, nDoodads, nDoodadSets, nX;
  // header
  f.read (&nTextures, 4);
  f.read (&nGroups, 4);
  f.read (&nP, 4);
  f.read (&nLights, 4);
  f.read (&nModels, 4);
  f.read (&nDoodads, 4);
  f.read (&nDoodadSets, 4);
  f.read (&ambient_color, 4);
  f.read (&nX, 4);
  f.read (ff, 12);
  extents[0] = ::math::vector_3d (ff[0], ff[1], ff[2]);
  f.read (ff, 12);
  extents[1] = ::math::vector_3d (ff[0], ff[1], ff[2]);
  f.read(&flags, 2);

  f.seekRelative (2);

  ambient_light_color.x = static_cast<float>(ambient_color.r) / 255.f;
  ambient_light_color.y = static_cast<float>(ambient_color.g) / 255.f;
  ambient_light_color.z = static_cast<float>(ambient_color.b) / 255.f;
  ambient_light_color.w = static_cast<float>(ambient_color.a) / 255.f;

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

  std::map<std::uint32_t, std::size_t> texture_offset_to_inmem_index;

  auto load_texture
    ( [&] (std::uint32_t ofs)
      {
        char const* texture
          (texbuf[ofs] ? &texbuf[ofs] : "textures/shanecube.blp");

        auto const mapping
          (texture_offset_to_inmem_index.emplace(ofs, textures.size()));

        if (mapping.second)
        {
          textures.emplace_back(texture);
        }
        return mapping.first->second;
      }
    );

  for (size_t i(0); i < num_materials; ++i)
  {
    f.read(&materials[i], sizeof(WMOMaterial));

    uint32_t shader = materials[i].shader;
    bool use_second_texture = (shader == 6 || shader == 5 || shader == 3);

    materials[i].texture1 = load_texture(materials[i].texture_offset_1);
    if (use_second_texture)
    {
      materials[i].texture2 = load_texture(materials[i].texture_offset_2);
    }
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
    path = misc::replace(path, "mdx", "m2");

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
  {
    group.load();

    if (group.liquid)
    {
      _has_liquids = true;
    }
  }

  finished = true;
  _state_changed.notify_all();
}


void WMO::draw_instanced( opengl::scoped::use_program& wmo_shader
                        , math::matrix_4x4 const& // model_view
                        , math::matrix_4x4 const& // projection
                        , std::vector<WMOInstance*>& instances
                        , bool // boundingbox
                        , math::frustum const& frustum
                        , const float& cull_distance
                        , const math::vector_3d& camera
                        , bool // draw_doodads
                        , bool draw_fog
                        , liquid_render& // render
                        , int // animtime
                        , bool world_has_skies
                        , display_mode display
                        , wmo_group_uniform_data& wmo_uniform_data
                        , std::vector<std::pair<wmo_liquid*, math::matrix_4x4>>& wmo_liquids_to_draw
                        , noggit::texture_array_handler& texture_handler
                        , bool update_transform_matrix_buffer
                        )
{
  if (!finishedLoading() || loading_failed())
  {
    return;
  }

  if (!_uploaded)
  {
    if (_textures_infos.empty())
    {
      for (std::string& tex : textures)
      {
        _textures_infos.push_back(texture_handler.get_texture_info(tex));
      }
    }

    // wait for the textures to load before rendering the model
    // texture data required for the ubo setup
    if (!check_texture_upload_status())
    {
      return;
    }

    _vertex_arrays.upload();
    _buffers.upload();

    opengl::scoped::vao_binder const _ (_vao);

    wmo_shader.attrib(_, "position", _vertices_buffer, 3, GL_FLOAT, GL_FALSE, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, position));
    wmo_shader.attrib(_, "normal",  _vertices_buffer, 3, GL_FLOAT, GL_FALSE, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, normal));
    wmo_shader.attrib(_, "color", _vertices_buffer, 4, GL_FLOAT, GL_FALSE, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, color));
    wmo_shader.attrib(_, "uv1", _vertices_buffer, 2, GL_FLOAT, GL_FALSE, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, uv1));
    wmo_shader.attrib(_, "uv2", _vertices_buffer, 2, GL_FLOAT, GL_FALSE, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, uv2));
    wmo_shader.attrib_int(_, "id", _vertices_buffer, 1, GL_INT, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, index));
    wmo_shader.attrib(_, "transform", _transform_buffer, static_cast<math::matrix_4x4*> (nullptr), 1);

    std::vector<wmo_vertex> vertices_data;
    std::vector<std::uint32_t> indices_data;

    for (auto& group : groups)
    {
      group.setup_global_buffer_data(vertices_data, indices_data);
      group.setup_ubo_data();
    }

    gl.bindBuffer(GL_ARRAY_BUFFER, _vertices_buffer);
    gl.bufferData(GL_ARRAY_BUFFER, sizeof(wmo_vertex) * vertices_data.size(), vertices_data.data(), GL_STATIC_DRAW);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices_buffer);
    gl.bufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * indices_data.size(), indices_data.data(), GL_STATIC_DRAW);

    _uploaded = true;
  }

  std::vector<math::matrix_4x4> transform_matrix;

  if (update_transform_matrix_buffer || _need_transform_buffer_update)
  {
    transform_matrix.reserve(instances.size());

    for (WMOInstance* mi : instances)
    {
      if (mi->is_visible(frustum, cull_distance, camera, display))
      {
        transform_matrix.push_back(mi->transform_matrix_transposed());
      }
    }

    _instance_visible = transform_matrix.size();
  }

  if (_instance_visible == 0)
  {
    return;
  }

  if (update_transform_matrix_buffer || _need_transform_buffer_update)
  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const transform_binder(_transform_buffer);
    gl.bufferData(GL_ARRAY_BUFFER, _instance_visible * sizeof(::math::matrix_4x4), transform_matrix.data(), GL_STATIC_DRAW);

    _need_transform_buffer_update = false;
  }

  opengl::scoped::vao_binder const _ (_vao);

  wmo_shader.uniform("ambient_color", ambient_light_color.xyz());

  for (auto& group : groups)
  {
    group.draw( wmo_shader
              , frustum
              , cull_distance
              , camera
              , draw_fog
              , world_has_skies
              , wmo_uniform_data
              , _instance_visible
              , texture_handler
              );


    if (group.liquid)
    {
      for (math::matrix_4x4 const& m : transform_matrix)
      {
        wmo_liquids_to_draw.emplace_back(group.liquid.get(), m);
      }
    }
  }
}
void WMO::draw_depth( opengl::scoped::use_program& depth_shader
                    , std::vector<WMOInstance*>& instances
                    , noggit::texture_array_handler& texture_handler
                    )
{
  if (!finishedLoading() || loading_failed() || instances.size() == 0)
  {
    return;
  }

  std::vector<math::matrix_4x4> transform_matrix;
  int instance_visible = 0;

  {
    transform_matrix.reserve(instances.size());

    for (WMOInstance* mi : instances)
    {
      transform_matrix.push_back(mi->transform_matrix_transposed());
    }

    instance_visible = transform_matrix.size();

    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const transform_binder(_depth_transform_buffer);
    gl.bufferData(GL_ARRAY_BUFFER, instance_visible * sizeof(::math::matrix_4x4), transform_matrix.data(), GL_STATIC_DRAW);
  }

  if (instance_visible == 0)
  {
    return;
  }

  {
    opengl::scoped::vao_binder const _ (_depth_vao);

    {
      opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder(_vertices_buffer);

      depth_shader.attrib(_, "position", opengl::array_buffer_is_already_bound{}, 3, GL_FLOAT, GL_FALSE, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, position));
      depth_shader.attrib(_, "uv1", opengl::array_buffer_is_already_bound{}, 2, GL_FLOAT, GL_FALSE, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, uv1));
      depth_shader.attrib_int(_, "id", opengl::array_buffer_is_already_bound{}, 1, GL_INT, sizeof(wmo_vertex), (void*)offsetof(wmo_vertex, index));
    }

    {
      opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const transform_binder(_depth_transform_buffer);
      depth_shader.attrib(_, "transform", opengl::array_buffer_is_already_bound{}, static_cast<math::matrix_4x4*> (nullptr), 1);
    }

    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices_buffer);
  }

  opengl::scoped::vao_binder const _ (_depth_vao);

  for (auto& group : groups)
  {
    group.draw_depth( instance_visible
                    , texture_handler
                    );
  }
}

void WMO::draw_boxes_instanced(opengl::scoped::use_program& wmo_box_shader)
{
  if (!_instance_visible)
  {
    return;
  }

  opengl::scoped::vao_binder const _ (_bbox_vao);

  if (!_bbox_uploaded)
  {
    static std::array<std::uint16_t, 16> const indices
        {{5, 7, 3, 2, 0, 1, 3, 1, 5, 4, 0, 4, 6, 2, 6, 7}};

    std::vector<math::vector_3d> bbox_vertices;
    std::vector<std::uint16_t> bbox_indices;

    bbox_vertices.reserve(groups.size() * 8);
    bbox_indices.reserve(groups.size() * 16);

    for (int i=0; i<groups.size(); ++i)
    {
      auto& group = groups[i];

      auto group_bbox_vertices(math::box_points(group.BoundingBoxMin, group.BoundingBoxMax));
      bbox_vertices.insert(bbox_vertices.begin() + i * 8, group_bbox_vertices.begin(), group_bbox_vertices.end());

      std::uint16_t offset = i * 16;
      for (std::uint16_t indice : indices)
      {
        bbox_indices.push_back(indice + offset);
      }
    }

    gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>(_bbox_vertices, bbox_vertices, GL_STATIC_DRAW);
    gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_bbox_indices, bbox_indices, GL_STATIC_DRAW);

    wmo_box_shader.attrib(_, "position", _bbox_vertices, 3, GL_FLOAT, GL_FALSE, 0, 0);
    wmo_box_shader.attrib(_, "transform", _transform_buffer, static_cast<math::matrix_4x4*> (nullptr), 1);

    gl.bindBuffer(GL_ARRAY_BUFFER, _bbox_vertices);
    gl.bindBuffer(GL_ARRAY_BUFFER, _transform_buffer);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bbox_indices);

    _bbox_uploaded = true;
  }

  gl.drawElementsInstanced(GL_LINE_STRIP, 16 * groups.size(), _instance_visible, GL_UNSIGNED_SHORT, opengl::index_buffer_is_already_bound{}, 0);
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
                      , noggit::texture_array_handler& texture_handler
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
      ModelInstance sky(skybox.value()->filename);
      sky.set_position(camera_pos);
      sky.set_scale(2.f);
      sky.recalcExtents();

      opengl_model_state_changer model_state_changer;
      skybox->get()->draw(model_view, sky, m2_shader, frustum, cull_distance, camera_pos, animtime, draw_particles, false, display_mode::in_3D, texture_handler, model_state_changer);

      return true;
    }
  }

  return false;
}

bool WMO::check_texture_upload_status()
{
  if (_textures_finished_upload)
  {
    return true;
  }

  for (auto& tex_info : _textures_infos)
  {
    if (!tex_info->ready())
    {
      return false;
    }
  }

  return _textures_finished_upload = true;
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

void WMOLight::setup(GLint)
{
  // not used right now -_-
}

void WMOLight::setupOnce(GLint, math::vector_3d, math::vector_3d)
{
  //math::vector_4d position(dir, 0);
  //math::vector_4d position(0,1,0,0);

  //math::vector_4d ambient = math::vector_4d(light_color * 0.3f, 1);
  //math::vector_4d diffuse = math::vector_4d(light_color, 1);


  //gl.enable(light);
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
  , _texcoords_2(other._texcoords_2)
  , _vertex_colors(other._vertex_colors)
  , _indices(other._indices)
  , _ubo(other._ubo)
{
  if (other.liquid)
  {
    liquid = std::make_unique<wmo_liquid>(*other.liquid.get());
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

  for (auto& n : _normals)
  {
    n = {n.x, n.z, -n.y};
  }

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

    load_mocv(f, size);
  }
  // - MLIQ ----------------------------------------------
  if (header.flags.has_water)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MLIQ');

    WMOLiquidHeader hlq;
    f.read(&hlq, 0x1E);

    liquid = std::make_unique<wmo_liquid> ( &f
                                      , hlq
                                      , wmo->materials[hlq.material_id]
                                      , header.group_liquid
                                      , (bool)wmo->flags.use_liquid_type_dbc_id
                                      , (bool)header.flags.ocean
                                      );

    // creating the wmo liquid doesn't move the position
    f.seekRelative(size - 0x1E);
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

    _texcoords_2.resize(size / sizeof(::math::vector_2d));
    f.read(_texcoords_2.data(), size);
  }
  // - MOCV ----------------------------------------------
  if (header.flags.use_mocv2_for_texture_blending)
  {
    f.read (&fourcc, 4);
    f.read (&size, 4);

    assert (fourcc == 'MOCV');

    std::vector<CImVector> mocv_2(size / sizeof(CImVector));
    f.read(mocv_2.data(), size);

    for (int i = 0; i < mocv_2.size(); ++i)
    {
      float alpha = static_cast<float>(mocv_2[i].a) / 255.f;

      // the second mocv is used for texture blending only
      if (header.flags.has_vertex_color)
      {
        _vertex_colors[i].w = alpha;
      }
      else // no vertex coloring, only texture blending with the alpha
      {
        _vertex_colors.emplace_back(0.f, 0.f, 0.f, alpha);
      }
    }
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
        ::math::vector_3d dir = l.pos - mi.position();
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

void WMOGroup::load_mocv(MPQFile& f, uint32_t size)
{
  uint32_t const* colors = reinterpret_cast<uint32_t const*> (f.getPointer());
  _vertex_colors.resize(size / sizeof(uint32_t));

  for (size_t i(0); i < size / sizeof(uint32_t); ++i)
  {
    _vertex_colors[i] = colorFromInt(colors[i]);
  }

  if (wmo->flags.do_not_fix_vertex_color_alpha)
  {
    int interior_batchs_start = 0;

    if (header.transparency_batches_count > 0)
    {
      interior_batchs_start = _batches[header.transparency_batches_count - 1].vertex_end + 1;
    }

    for (int n = interior_batchs_start; n < _vertex_colors.size(); ++n)
    {
      _vertex_colors[n].w = header.flags.exterior ? 1.f : 0.f;
    }
  }
  else
  {
    fix_vertex_color_alpha();
  }

  // there's no read so this is required
  f.seekRelative(size);
}

void WMOGroup::fix_vertex_color_alpha()
{
  int interior_batchs_start = 0;

  if (header.transparency_batches_count > 0)
  {
    interior_batchs_start = _batches[header.transparency_batches_count - 1].vertex_end + 1;
  }

  math::vector_4d wmo_ambient_color;

  if (wmo->flags.use_unified_render_path)
  {
    wmo_ambient_color = {0.f, 0.f, 0.f, 0.f};
  }
  else
  {
    wmo_ambient_color = wmo->ambient_light_color;
    // w is not used, set it to 0 to avoid changing the vertex color alpha
    wmo_ambient_color.w = 0.f;
  }

  for (int i = 0; i < _vertex_colors.size(); ++i)
  {
    auto& color = _vertex_colors[i];
    float r = color.x;
    float g = color.y;
    float b = color.z;
    float a = color.w;

    // I removed the color = color/2 because it's just multiplied by 2 in the shader afterward in blizzard's code
    if (i >= interior_batchs_start)
    {
      r += ((r * a / 64.f) - wmo_ambient_color.x);
      g += ((g * a / 64.f) - wmo_ambient_color.y);
      b += ((b * a / 64.f) - wmo_ambient_color.z);
    }
    else
    {
      r -= wmo_ambient_color.x;
      g -= wmo_ambient_color.y;
      b -= wmo_ambient_color.z;

      r = (r * (1.f - a));
      g = (g * (1.f - a));
      b = (b * (1.f - a));
    }

    color.x = std::min(255.f, std::max(0.f, r));
    color.y = std::min(255.f, std::max(0.f, g));
    color.z = std::min(255.f, std::max(0.f, b));
    color.w = 1.f; // default value used in the shader so I simplified it here,
                   // it can be overriden by the 2nd mocv chunk
  }
}

bool WMOGroup::is_visible( math::matrix_4x4 const& transform
                         , math::frustum const& frustum
                         , float const& cull_distance
                         , math::vector_3d const& camera
                         , display_mode display
                         ) const
{
  math::vector_3d pos = transform * center;

  if (!frustum.intersectsSphere(pos, rad))
  {
    return false;
  }

  float dist = display == display_mode::in_3D
    ? (pos - camera).length() - rad
    : std::abs(pos.y - camera.y) - rad;

  return (dist < cull_distance);
}

void WMOGroup::setup_global_buffer_data(std::vector<wmo_vertex>& vertices, std::vector<std::uint32_t>& indices)
{
  if (_batches.empty())
  {
    return;
  }

  _vertex_offset = vertices.size();
  _index_offset = indices.size();

  bool mocv = _vertex_colors.size() > 0;
  bool uv2 = _texcoords_2.size() > 0;

  vertices.reserve(vertices.size() + _vertices.size());
  indices.reserve(indices.size()+ _indices.size());

  for (int i = 0; i < _vertices.size(); ++i)
  {
    wmo_vertex v;

    v.position = _vertices[i];
    v.normal = _normals[i];
    v.color = mocv ? _vertex_colors[i] : math::vector_4d();
    v.uv1 = _texcoords[i];
    v.uv2 = uv2 ? _texcoords_2[i] : math::vector_2d();
    v.index = -1;

    vertices.push_back(v);
  }

  int next_vertex_index = vertices.size();

  for (int i = 0; i < _batches.size(); ++i)
  {
    wmo_batch& batch = _batches[i];
    std::map<std::uint16_t, std::uint32_t> shifted_indices;

    for (int j = batch.index_start; j < batch.index_start + batch.index_count; ++j)
    {
      std::uint16_t id = _indices[j];
      wmo_vertex& v = vertices[id + _vertex_offset];

      // not used or used for this batch
      if (v.index == -1 || v.index == i)
      {
        v.index = i;
        indices.push_back(id+_vertex_offset);
      }
      // duplicate vertex if necessary, or use the new index
      // doesn't affect batch indices start/count as it's just shifting the indices
      else
      {
        auto const& it = shifted_indices.find(id);

        if (it != shifted_indices.end())
        {
          indices.push_back(it->second);
        }
        else
        {
          indices.push_back(next_vertex_index);
          vertices.push_back(v);
          vertices[next_vertex_index].index = i;

          shifted_indices[id] = next_vertex_index;
          next_vertex_index++;
        }
      }
    }
  }

  // data no longer need, only keep the vertices/indices for collisions
  _normals.clear();
  _normals.shrink_to_fit();
  _vertex_colors.clear();
  _vertex_colors.shrink_to_fit();
  _texcoords.clear();
  _texcoords.shrink_to_fit();
  _texcoords_2.clear();
  _texcoords_2.shrink_to_fit();
}


void WMOGroup::setup_ubo_data()
{
  int batch_count = _batches.size();
  int exterior_lit = header.flags.exterior_lit | header.flags.exterior;
  int has_mocv = header.flags.has_vertex_color | header.flags.use_mocv2_for_texture_blending;

  std::vector<wmo_ubo_data> data(batch_count);
#ifdef USE_BINDLESS_TEXTURES
  std::vector<wmo_render_batch> render_batches;
#endif

  for (int i = 0; i < batch_count; ++i)
  {
    wmo_batch& batch = _batches[i];
    WMOMaterial const& mat(wmo->materials.at(batch.texture));
    wmo_ubo_data ubo_data;

    ubo_data.exterior_lit = exterior_lit;
    ubo_data.use_vertex_color = has_mocv;
    ubo_data.shader_id = mat.shader;
    ubo_data.unfogged = mat.flags.unfogged;
    ubo_data.unlit = mat.flags.unlit;

    wmo_render_batch rbg;
    rbg.blend_mode = mat.blend_mode;
    rbg.cull = !mat.flags.unculled;
    rbg.index_start = batch.index_start;
    rbg.index_count = batch.index_count;

    if (mat.blend_mode == 1)
    {
      rbg.blend_mode = 0; // only alpha test change, and it's in the ubo
      ubo_data.alpha_test = 0.878431372f; // 224/255
    }
    else if (mat.blend_mode > 6 || mat.blend_mode == 0)
    {
      ubo_data.alpha_test = -1.f;
    }
    else
    {
      ubo_data.alpha_test = 0.003921568f; // 1/255
    }

#ifdef USE_BINDLESS_TEXTURES
    render_batches.push_back(rbg);
#else
    _render_batches.push_back(rbg);
#endif

    auto& t1_param = wmo->_textures_infos.at(mat.texture1);
#ifdef USE_BINDLESS_TEXTURES
    ubo_data.texture_1 = t1_param->array_handle.value();
#else
    ubo_data.texture_1 = mat.texture1;
#endif
    ubo_data.index_1 = t1_param->pos_in_array->second;

    // only shaders using 2 textures in wotlk
    if (mat.shader == 6 || mat.shader == 5 || mat.shader == 3)
    {
      auto& t2_param = wmo->_textures_infos.at(mat.texture2);
#ifdef USE_BINDLESS_TEXTURES
      ubo_data.texture_2 = t2_param->array_handle.value();
#else
      ubo_data.texture_2 = mat.texture2;
#endif
      ubo_data.index_2 = t2_param->pos_in_array->second;
    }
    else
    {
      ubo_data.texture_2 = 0;
      ubo_data.index_2 = 0;
    }

    data[i] = ubo_data;
  }

#ifdef USE_BINDLESS_TEXTURES
  if (!render_batches.empty())
  {
    wmo_render_batch batch = render_batches[0];

    for (int i = 1; i < render_batches.size(); ++i)
    {
      wmo_render_batch b = render_batches[i];

      if (b.blend_mode == batch.blend_mode && b.cull == batch.cull && b.index_start == batch.index_start + batch.index_count)
      {
        batch.index_count += b.index_count;
      }
      else
      {
        _render_batches.push_back(batch);
        batch = b;
      }
    }
    _render_batches.push_back(batch);
  }
#endif

  gl.genBuffers(1, &_ubo);
  gl.bindBuffer(GL_UNIFORM_BUFFER, _ubo);
  gl.bufferData(GL_UNIFORM_BUFFER, sizeof(wmo_ubo_data) * batch_count, data.data(), GL_STATIC_DRAW);
  gl.bindBuffer(GL_UNIFORM_BUFFER, 0);
}

void WMOGroup::draw( opengl::scoped::use_program&
                   , math::frustum const& // frustum
                   , const float& //cull_distance
                   , const math::vector_3d& //camera
                   , bool // draw_fog
                   , bool // world_has_skies
                   , wmo_group_uniform_data& wmo_uniform_data
                   , int instance_count
                   , [[maybe_unused]] noggit::texture_array_handler& texture_handler
                   )
{
  gl.bindBufferBase(GL_UNIFORM_BUFFER, 0, _ubo);

#ifndef USE_BINDLESS_TEXTURES
  int batch_index = 0;
#endif

  for(wmo_render_batch& batch : _render_batches)
  {
    if (batch.blend_mode != wmo_uniform_data.blend_mode)
    {
      switch (batch.blend_mode)
      {
      default:
      case 0:
      case 1:
        gl.disable(GL_BLEND);
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
      }

      wmo_uniform_data.blend_mode = batch.blend_mode;
    }

    if (batch.cull != wmo_uniform_data.cull)
    {
      if (!batch.cull)
      {
        gl.disable(GL_CULL_FACE);
      }
      else
      {
        gl.enable(GL_CULL_FACE);
      }

      wmo_uniform_data.cull = batch.cull;
    }

#ifndef USE_BINDLESS_TEXTURES
    WMOMaterial const& mat(wmo->materials.at(_batches[batch_index++].texture));

    texture_handler.bind_layer(wmo->_textures_infos[mat.texture1]->pos_in_array->first, 0);

    if (mat.shader == 3 || mat.shader == 5 || mat.shader == 6)
    {
      texture_handler.bind_layer(wmo->_textures_infos[mat.texture2]->pos_in_array->first, 1);
    }
#endif

    gl.drawElementsInstanced(GL_TRIANGLES, batch.index_count, instance_count, GL_UNSIGNED_INT, opengl::index_buffer_is_already_bound{}, sizeof(std::uint32_t) * (batch.index_start + _index_offset));
  }
}


void WMOGroup::draw_depth( int instance_count
                         , [[maybe_unused]] noggit::texture_array_handler& texture_handler
                         )
{
  gl.bindBufferBase(GL_UNIFORM_BUFFER, 0, _ubo);

#ifndef USE_BINDLESS_TEXTURES
  int batch_index = 0;
#endif

  for(wmo_render_batch& batch : _render_batches)
  {
    // skip transparent batches
    if (batch.blend_mode != 0 && batch.blend_mode != 1)
    {
#ifndef USE_BINDLESS_TEXTURES
      batch_index++;
#endif
      continue;
    }


#ifndef USE_BINDLESS_TEXTURES
    WMOMaterial const& mat(wmo->materials.at(_batches[batch_index++].texture));

    texture_handler.bind_layer(wmo->_textures_infos[mat.texture1]->pos_in_array->first, 0);
#endif

    gl.drawElementsInstanced(GL_TRIANGLES, batch.index_count, instance_count, GL_UNSIGNED_INT, opengl::index_buffer_is_already_bound{}, sizeof(std::uint32_t) * (batch.index_start + _index_offset));
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
  _.apply ( [&] (std::string const&, WMO& wmo)
            {
              wmo.show();
            }
          );
}
