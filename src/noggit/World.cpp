// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/World.h>

#include <math/frustum.hpp>
#include <noggit/Brush.h> // brush
#include <noggit/chunk_mover.hpp>
#include <noggit/liquid_chunk.hpp>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/settings.hpp>
#include <noggit/TextureManager.h>
#include <noggit/liquid_tile.hpp>// tile water
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/map_index.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/TexturingGUI.h>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <QtWidgets/QMessageBox>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFramebufferObjectFormat>
#include <QtGui/QPixmap>
#include <QtOpenGL/QGLPixelBuffer>

#include <algorithm>
#include <cassert>
#include <ctime>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>


bool World::IsEditableWorld(int pMapId)
{
  std::string lMapName;
  try
  {
    DBCFile::Record map = gMapDB.getByID((unsigned int)pMapId);
    lMapName = map.getString(MapDB::InternalName);
  }
  catch (int)
  {
    LogError << "Did not find map with id " << pMapId << ". This is NOT editable.." << std::endl;
    return false;
  }

  std::stringstream ssfilename;
  ssfilename << noggit::mpq::uni_path("World/Maps/" + lMapName + "/" + lMapName + ".wdt");

  if (!MPQFile::exists(ssfilename.str()))
  {
    NOGGIT_LOG << "World " << pMapId << ": " << lMapName << " has no WDT file!" << std::endl;
    return false;
  }

  MPQFile mf(ssfilename.str());

  //sometimes, wdts don't open, so ignore them...
  if (mf.isEof())
    return false;

  const char * lPointer = reinterpret_cast<const char*>(mf.getPointer());

  // Not using the libWDT here doubles performance. You might want to look at your lib again and improve it.
  const int lFlags = *(reinterpret_cast<const int*>(lPointer + 8 + 4 + 8));
  if (lFlags & 1)
    return false;

  const int * lData = reinterpret_cast<const int*>(lPointer + 8 + 4 + 8 + 0x20 + 8);
  for (int i = 0; i < 8192; i += 2)
  {
    if (lData[i] & 1)
      return true;
  }

  return false;
}

World::World(const std::string& name, int map_id)
  : _model_instance_storage(this)
  , _tile_update_queue(this)
  , _tileset_handler(1)
  , _model_texture_handler(0)
  , mapIndex (name, map_id, this)
  , horizon(name, &mapIndex)
  , mWmoFilename("")
  , mWmoEntry(ENTRY_MODF())
  , detailtexcoords(0)
  , ol(nullptr)
  , animtime(0)
  , time(1450)
  , basename(name)
  , fogdistance(777.0f)
  , culldistance(fogdistance)
  , skies(nullptr)
  , outdoorLightStats(OutdoorLightStats())
  , _current_selection()
  , _grouped_models()
  , _view_distance(NoggitSettings.value ("view_distance", 1000.f).toFloat() + TILE_RADIUS) // add adt radius to make sure tiles aren't culled too soon, todo: improve adt culling to prevent that from happening
{
  LogDebug << "Loading world \"" << name << "\"." << std::endl;
}

bool World::is_selected(selection_type selection) const
{
  if (selection.index() == eEntry_Model)
  {
    uint uid = std::get<selected_model_type>(selection)->uid;
    auto const& it = std::find_if(_current_selection.begin()
                                  , _current_selection.end()
                                  , [uid] (selection_type type)
    {
      return type.index() == eEntry_Model
        && std::get<selected_model_type>(type)->uid == uid;
    }
    );

    if (it != _current_selection.end())
    {
      return true;
    }
  }
  else if (selection.index() == eEntry_WMO)
  {
    uint uid = std::get<selected_wmo_type>(selection)->mUniqueID;
    auto const& it = std::find_if(_current_selection.begin()
                            , _current_selection.end()
                            , [uid] (selection_type type)
    {
      return type.index() == eEntry_WMO
        && std::get<selected_wmo_type>(type)->mUniqueID == uid;
    }
    );
    if (it != _current_selection.end())
    {
      return true;
    }
  }

  return false;
}

bool World::is_selected(std::uint32_t uid) const
{
  for (selection_type const& entry : _current_selection)
  {
    if (entry.index() == eEntry_WMO)
    {
      if (std::get<selected_wmo_type>(entry)->mUniqueID == uid)
      {
        return true;
      }
    }
    else if (entry.index() == eEntry_Model)
    {
      if (std::get<selected_model_type>(entry)->uid == uid)
      {
        return true;
      }
    }
  }

  return false;
}

std::optional<selection_type> World::get_last_selected_model() const
{
  auto const it
    ( std::find_if ( _current_selection.rbegin()
                   , _current_selection.rend()
                   , [&] (selection_type const& entry)
                     {
                       return entry.index() != eEntry_MapChunk && entry.index() != eEntry_LiquidLayer;
                     }
                   )
    );

  return it == _current_selection.rend()
    ? std::optional<selection_type>() : std::optional<selection_type> (*it);
}

void World::set_current_selection(selection_type entry)
{
  _grouped_models.reset();
  _current_selection.clear();
  _current_selection.push_back(entry);
  gizmo.unlink();

  _selected_model_count = entry.index() == eEntry_MapChunk || entry.index() == eEntry_LiquidLayer ? 0 : 1;
}

void World::add_to_selection(selection_type entry)
{
  if (entry.index() != eEntry_MapChunk && entry.index() != eEntry_LiquidLayer)
  {
    _selected_model_count++;

    gizmo.link_to(&_grouped_models);

    if (entry.index() == eEntry_Model)
    {
      _grouped_models.add_object(std::get<selected_model_type>(entry));
    }
    else if (entry.index() == eEntry_WMO)
    {
      _grouped_models.add_object(std::get<selected_wmo_type>(entry));
    }
  }

  _current_selection.push_back(entry);
}

void World::remove_from_selection(selection_type entry)
{
  std::vector<selection_type>::iterator position = std::find(_current_selection.begin(), _current_selection.end(), entry);
  if (position != _current_selection.end())
  {
    if (entry.index() == eEntry_Model)
    {
      _selected_model_count--;
      _grouped_models.remove_object(std::get<selected_model_type>(*position));
    }
    else if (entry.index() == eEntry_WMO)
    {
      _selected_model_count--;
      _grouped_models.remove_object(std::get<selected_wmo_type>(*position));
    }


    _current_selection.erase(position);
  }
}

void World::remove_from_selection(std::uint32_t uid)
{
  for (auto it = _current_selection.begin(); it != _current_selection.end(); ++it)
  {
    if (it->index() == eEntry_Model && std::get<selected_model_type>(*it)->uid == uid)
    {
      _selected_model_count--;
      _current_selection.erase(it);

      _grouped_models.remove_object(std::get<selected_model_type>(*it));

      return;
    }
    else if (it->index() == eEntry_WMO && std::get<selected_wmo_type>(*it)->mUniqueID == uid)
    {
      _selected_model_count--;
      _current_selection.erase(it);

      _grouped_models.remove_object(std::get<selected_wmo_type>(*it));
      return;
    }
  }
}

void World::reset_selection()
{
  _current_selection.clear();
  _grouped_models.reset();
  _selected_model_count = 0;
  gizmo.unlink();
}

void World::delete_selected_models()
{
  _model_instance_storage.delete_instances(_current_selection);
  need_model_updates = true;
  reset_selection();
}

void World::raise_models_terrain_brush(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius, bool follow_normals)
{
  std::vector<noggit::moveable_object*> models;

  need_model_updates = true;

  _model_instance_storage.for_each_m2_instance([&](ModelInstance& model_instance)
  {
    if((model_instance.position() - pos).length() < radius)
    {
      models.push_back(&model_instance);
    }
  });

  _model_instance_storage.for_each_wmo_instance([&](WMOInstance& wmo_instance)
  {
    if ((wmo_instance.position() - pos).length() < radius)
    {
      models.push_back(&wmo_instance);
    }
  });

  for (noggit::moveable_object* model : models)
  {
    math::vector_3d model_pos = model->position();

    float dist = (model_pos - pos).length();

    if (follow_normals)
    {
      math::degrees::vec3 rot = model->rotation();

      // todo: add smooth selection
      auto normal = get_terrain_normal(model_pos, true);

      if (normal)
      {
        rot.x = normal->x;
        //rot.y = normal->y;
        rot.z = normal->z;

        model->update_rotation(rot, this);
      }
    }

    if (BrushType == eTerrainType_Quadra)
    {
      if ((std::abs(model_pos.x - pos.x) < std::abs(radius / 2)) && (std::abs(model_pos.z - pos.z) < std::abs(radius / 2)))
      {
        model_pos.y += change * (1.0f - dist * inner_radius / radius);
      }
    }
    else
    {
      switch (BrushType)
      {
      case eTerrainType_Flat:
        model_pos.y += change;
        break;
      case eTerrainType_Linear:
        model_pos.y += change * (1.0f - dist * (1.0f - inner_radius) / radius);
        break;
      case eTerrainType_Smooth:
        model_pos.y += change / (1.0f + dist / radius);
        break;
      case eTerrainType_Polynom:
        model_pos.y += change * ((dist / radius) * (dist / radius) + dist / radius + 1.0f);
        break;
      case eTerrainType_Trigo:
        model_pos.y += change * cos(dist / radius);
        break;
      case eTerrainType_Gaussian:
        model_pos.y += dist < radius * inner_radius ? change * std::exp(-(std::pow(radius * inner_radius / radius, 2) / (2 * std::pow(0.39f, 2)))) : change * std::exp(-(std::pow(dist / radius, 2) / (2 * std::pow(0.39f, 2))));
        break;
      default:
        LogError << "Invalid terrain edit type (" << BrushType << ")" << std::endl;
        break;
      }
    }

    // todo: only trigger extents recalcs for this
    // as changing the height won't change which chunks the model spans
    model->update_position(model_pos, this);
  }
}

void World::snap_selected_models_to_the_ground()
{
  for (auto& entry : _current_selection)
  {
    auto type = entry.index();

    if (type != eEntry_Model && type != eEntry_WMO)
    {
      continue;
    }

    noggit::moveable_object* model;
    if (type == eEntry_Model)
    {
      model = std::get<selected_model_type>(entry);
    }
    else
    {
      model = std::get<selected_wmo_type>(entry);
    }

    math::vector_3d pos = model->position();
    std::optional<float> height = get_exact_height_at(pos);

    // this should never happen
    if (!height)
    {
      LogError << "Snap to ground ray intersection failed" << std::endl;
      continue;
    }

    pos.y = height.value();

    model->update_position(pos, this);
  }

  _grouped_models.update_center();
}

void World::scale_selected_models(float v, m2_scaling_type type)
{
  for (auto& entry : _current_selection)
  {
    if (entry.index() == eEntry_Model)
    {
      ModelInstance* mi = std::get<selected_model_type>(entry);

      float scale = mi->scale();

      switch (type)
      {
        case World::m2_scaling_type::set:
          scale = v;
          break;
        case World::m2_scaling_type::add:
          scale += v;
          break;
        case World::m2_scaling_type::mult:
          scale *= v;
          break;
      }

      // if the change is too small, do nothing
      if (std::abs(scale - mi->scale()) < ModelInstance::min_scale())
      {
        continue;
      }

      mi->update_scale(std::min(ModelInstance::max_scale(), std::max(ModelInstance::min_scale(), scale)), this);
    }
  }
}

void World::move_selected_models(float dx, float dy, float dz)
{
  _grouped_models.move(dx, dy, dz, this);
}

void World::set_selected_models_pos(math::vector_3d const& pos, bool change_height)
{
  // move models relative to the pivot when several are selected
  if (has_multiple_model_selected())
  {
    math::vector_3d diff = pos - _grouped_models.pivot().value();

    if (change_height)
    {
      move_selected_models(diff);
    }
    else
    {
      move_selected_models(diff.x, 0.f, diff.z);
    }

    return;
  }

  for (auto& entry : _current_selection)
  {
    auto type = entry.index();

    if (type == eEntry_Model)
    {
      std::get<selected_model_type>(entry)->update_position(pos, this);
    }
    else if (type == eEntry_WMO)
    {
      std::get<selected_wmo_type>(entry)->update_position(pos, this);
    }
  }

  _grouped_models.update_center();
}

void World::rotate_selected_models(math::degrees rx, math::degrees ry, math::degrees rz, bool use_pivot)
{
  math::degrees::vec3 dir_change(rx, ry, rz);
  bool has_multi_select = has_multiple_model_selected();

  for (auto& entry : _current_selection)
  {
    auto type = entry.index();

    if (type != eEntry_Model && type != eEntry_WMO)
    {
      continue;
    }

    noggit::moveable_object* model;
    if (type == eEntry_Model)
    {
      model = std::get<selected_model_type>(entry);
    }
    else
    {
      model = std::get<selected_wmo_type>(entry);
    }

    math::vector_3d pos = model->position();
    math::degrees::vec3 dir = model->rotation();

    if (use_pivot && has_multi_select)
    {
      math::vector_3d diff_pos = pos - _grouped_models.pivot().value();
      math::vector_3d rot_result = math::matrix_4x4(math::matrix_4x4::rotation_xyz, {rx, ry, rz}) * diff_pos;

      model->move(rot_result - diff_pos, this);
    }

    dir += dir_change;

    model->update_rotation(dir, this);
  }
}

void World::rotate_selected_models_randomly(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
  for (auto& entry : _current_selection)
  {
    auto type = entry.index();

    if (type != eEntry_Model && type != eEntry_WMO)
    {
      continue;
    }

    noggit::moveable_object* model;
    if (type == eEntry_Model)
    {
      model = std::get<selected_model_type>(entry);
    }
    else
    {
      model = std::get<selected_wmo_type>(entry);
    }

    math::degrees::vec3 dir = model->rotation();

    float rx = misc::randfloat(minX, maxX);
    float ry = misc::randfloat(minY, maxY);
    float rz = misc::randfloat(minZ, maxZ);

    math::quaternion baseRotation = math::quaternion(math::degrees::from_model_rotation(dir));
    math::quaternion newRotation = math::quaternion(math::radians(math::degrees(rx)), math::radians(math::degrees(ry)), math::radians(math::degrees(rz)));

    math::quaternion finalRotation = baseRotation % newRotation;
    finalRotation.normalize();

    dir = finalRotation.ToEulerAngles();

    model->update_rotation(dir, this);
  }
}

void World::set_selected_models_rotation(math::degrees rx, math::degrees ry, math::degrees rz)
{
  math::degrees::vec3 new_dir(rx, ry, rz);

  for (auto& entry : _current_selection)
  {
    auto type = entry.index();

    if (type == eEntry_Model)
    {
      std::get<selected_model_type>(entry)->update_rotation(new_dir, this);
    }
    else if (type == eEntry_WMO)
    {
      std::get<selected_wmo_type>(entry)->update_rotation(new_dir, this);
    }
  }
}

namespace
{
  math::vector_3d getBarycentricCoordinatesAt
    ( const math::vector_3d& a
    , const math::vector_3d& b
    , const math::vector_3d& c
    , const math::vector_3d& point
    , const math::vector_3d& normal
    )
  {
    // The area of a triangle is
    double areaABC = normal * ((b - a) % (c - a));
    double areaPBC = normal * ((b - point) % (c - point));
    double areaPCA = normal * ((c - point) % (a - point));

    math::vector_3d bary;
    bary.x = areaPBC / areaABC; // alpha
    bary.y = areaPCA / areaABC; // beta
    bary.z = 1.0f - bary.x - bary.y; // gamma
    return bary;
  }
}

void World::rotate_selected_models_to_ground_normal(bool smoothNormals)
{
  for (auto& entry : _current_selection)
  {
    auto type = entry.index();

    if (type != eEntry_Model && type != eEntry_WMO)
    {
      continue;
    }

    noggit::moveable_object* model;
    if (type == eEntry_Model)
    {
      model = std::get<selected_model_type>(entry);
    }
    else
    {
      model = std::get<selected_wmo_type>(entry);
    }

    math::vector_3d pos = model->position();
    math::degrees::vec3 dir = model->rotation();


    auto normal_vector = get_terrain_normal(pos, smoothNormals);

    if (normal_vector)
    {
      dir.x = normal_vector->x;
      dir.z = normal_vector->z;

      model->update_rotation(dir, this);
    }
  }
}

void World::initGlobalVBOs(GLuint* pDetailTexCoords)
{
  if (!*pDetailTexCoords)
  {
    math::vector_2d temp[mapbufsize], *vt;
    float tx, ty;

    // init texture coordinates for detail map:
    vt = temp;
    const float detail_half = 0.5f * detail_size / 8.0f;

    for (int j = 0; j < 17; ++j)
    {
      for (int i = 0; i < ((j % 2) ? 8 : 9); ++i)
      {
        tx = detail_size / 8.0f * i;
        ty = detail_size / 8.0f * j * 0.5f;
        if (j % 2) {
          // offset by half
          tx += detail_half;
        }
        *vt++ = math::vector_2d(tx, ty);
      }
    }

    gl.genBuffers(1, pDetailTexCoords);
    gl.bufferData<GL_ARRAY_BUFFER> (*pDetailTexCoords, sizeof(temp) * 256, NULL, GL_STATIC_DRAW);

    gl.bindBuffer(GL_ARRAY_BUFFER, *pDetailTexCoords);

    // duplicate the buffer as we render the whole adt at once now
    for (int chunk = 0; chunk < 256; ++chunk)
    {
      gl.bufferSubData(GL_ARRAY_BUFFER, sizeof(temp) * chunk, sizeof(temp), temp);
    }
  }
}

void World::initDisplay()
{
  initGlobalVBOs(&detailtexcoords);

  if (mapIndex.hasAGlobalWMO())
  {
    WMOInstance inst(mWmoFilename, &mWmoEntry);

    _model_instance_storage.add_wmo_instance(std::move(inst), false);
  }
  else
  {
    _horizon_render = std::make_unique<noggit::map_horizon::render>(horizon);
  }

  skies = std::make_unique<Skies> (mapIndex._map_id);

  ol = std::make_unique<OutdoorLighting> ("World\\dnc.db");
}

void World::draw ( math::matrix_4x4 const& model_view
                 , math::matrix_4x4 const& projection
                 , math::frustum const& frustum
                 , math::vector_3d const& cursor_pos
                 , math::vector_4d const& cursor_color
                 , int cursor_type
                 , bool square_brush
                 , float brush_radius
                 , bool show_liquid_cursor
                 , bool show_unpaintable_chunks
                 , std::string const& current_texture
                 , bool draw_contour
                 , float inner_radius_ratio
                 , math::vector_3d const& ref_pos
                 , float angle
                 , float orientation
                 , bool use_ref_pos
                 , bool angled_mode
                 , bool draw_chunk_flag_overlay
                 , bool draw_areaid_overlay
                 , editing_mode terrainMode
                 , math::vector_3d const& camera_pos
                 , bool camera_moved
                 , bool draw_mfbo
                 , bool draw_wireframe
                 , bool draw_lines
                 , bool draw_terrain
                 , bool draw_wmo
                 , bool draw_water
                 , bool draw_wmo_doodads
                 , bool draw_models
                 , bool draw_model_animations
                 , bool draw_hole_lines
                 , bool draw_models_with_box
                 , bool draw_hidden_models
                 , bool draw_sky
                 , bool draw_skybox
                 , bool draw_shadows
                 , bool draw_vertex_colors
                 , bool use_dbc_lighting_data
                 , std::map<int, misc::random_color>& area_id_colors
                 , bool draw_fog
                 , eTerrainType ground_editing_brush
                 , int water_layer
                 , display_mode display
                 )
{
  // opengl 3.x garanties 16 texture unit per shader stage
  // but should be 32 for most computers for the fragment shader
  static int fragment_shader_max_texture_unit = 16;

  if (!_display_initialized)
  {
    initDisplay();
    _display_initialized = true;

    // valid only for the fragment shader, 16 for other stages
    gl.getIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &fragment_shader_max_texture_unit);

    GLint max_layers;
    gl.getIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);

    _model_texture_handler.set_max_array_size(max_layers);

    LogDebug << "Max fragment shader sampler: " << fragment_shader_max_texture_unit << std::endl;
    LogDebug << "Max texture array size: " << max_layers << std::endl;

    _only_one_model_box = NoggitSettings.value("only_one_model_box", true).toBool();
  }

  math::matrix_4x4 const mvp(model_view * projection);

  cursor_mode cursor = static_cast<cursor_mode>(cursor_type);

  if (!_gizmo_program)
  {
    _gizmo_program.reset
      ( new opengl::program
        { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("gizmo_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("gizmo_fs") }
        }
      );
  }

  if (!_m2_program)
  {
    _m2_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs") }
#ifdef USE_BINDLESS_TEXTURES
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs", {"use_bindless"}) }
#else
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
#endif
          }
      );
  }
  if (!_m2_instanced_program)
  {
    _m2_instanced_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs", {"instanced"}) }
#ifdef USE_BINDLESS_TEXTURES
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs", {"use_bindless"}) }
#else
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
#endif
          }
      );

    opengl::scoped::use_program m2_shader{ *_m2_instanced_program.get() };
  }
  if (!_m2_box_program)
  {
    _m2_box_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_box_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_box_fs") }
          }
      );
  }
  if (!_m2_ribbons_program)
  {
    _m2_ribbons_program.reset
      ( new opengl::program
#ifdef USE_BINDLESS_TEXTURES
        { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("ribbon_vs", {"use_bindless"}) }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("ribbon_fs", {"use_bindless"}) }
#else
        { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("ribbon_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("ribbon_fs") }
#endif
        }
      );
  }
  if (!_m2_particles_program)
  {
    _m2_particles_program.reset
      ( new opengl::program
#ifdef USE_BINDLESS_TEXTURES
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("particle_vs", {"use_bindless"}) }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("particle_fs", {"use_bindless"}) }
#else
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("particle_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("particle_fs") }
#endif
          }
      );
  }
  if (!_mcnk_program)
  {
    _mcnk_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("terrain_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("terrain_fs") }
          }
      );

    opengl::scoped::use_program mcnk_shader{ *_mcnk_program.get() };

    mcnk_shader.uniform("alphamap", 0);

    for (int i = 0; i < fragment_shader_max_texture_unit-1; ++i)
    {
      mcnk_shader.uniform("texture_arrays[" + std::to_string(i) + "]", i + 1);
    }

    mcnk_shader.uniform("wireframe_type", NoggitSettings.value("wireframe/type", 0).toInt());
    mcnk_shader.uniform("wireframe_radius", NoggitSettings.value("wireframe/radius", 1.5f).toFloat());
    mcnk_shader.uniform("wireframe_width", NoggitSettings.value("wireframe/width", 1.f).toFloat());
    // !\ todo store the color somewhere ?
    QColor c = NoggitSettings.value("wireframe/color").value<QColor>();
    math::vector_4d wireframe_color(c.redF(), c.greenF(), c.blueF(), c.alphaF());
    mcnk_shader.uniform("wireframe_color", wireframe_color);
  }
  if (!_mfbo_program)
  {
    _mfbo_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("mfbo_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("mfbo_fs") }
          }
      );
  }
  if (!_liquid_render)
  {
    _liquid_render.emplace();
  }
  if (!_wmo_program)
  {
    _wmo_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("wmo_vs") }
#ifdef USE_BINDLESS_TEXTURES
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("wmo_fs", {"use_bindless"}) }
#else
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("wmo_fs") }
#endif
          }
      );
  }

  gl.disable(GL_DEPTH_TEST);

  int daytime = static_cast<int>(time) % 2880;

  skies->update_sky_colors(camera_pos, daytime);
  outdoorLightStats = ol->getLightStats(daytime);

  math::vector_3d light_dir = outdoorLightStats.dayDir;
  light_dir = {-light_dir.y, -light_dir.z, -light_dir.x};
  // todo: figure out why I need to use a different light vector for the terrain
  math::vector_3d terrain_light_dir = {-light_dir.z, light_dir.y, -light_dir.x};

  math::vector_3d diffuse_color((use_dbc_lighting_data ? skies->color_set[LIGHT_GLOBAL_DIFFUSE] : math::vector_3d(1.f, 1.f, 1.f)) * outdoorLightStats.dayIntensity);
  math::vector_3d ambient_color((use_dbc_lighting_data ? skies->color_set[LIGHT_GLOBAL_AMBIENT] : math::vector_3d(1.f, 1.f, 1.f)) * outdoorLightStats.ambientIntensity);

  // if m2/wmo are rendered, check if there are textures ready to be uploaded
  if (draw_models || draw_wmo || draw_skybox)
  {
    _model_texture_handler.upload_ready_textures();
  }

  // only draw the sky in 3D
  if(display == display_mode::in_3D && draw_sky)
  {
    opengl::scoped::use_program m2_shader {*_m2_program.get()};

    m2_shader.uniform("model_view", model_view);
    m2_shader.uniform("projection", projection);

    m2_shader.uniform("draw_fog", 0);

    m2_shader.uniform("light_dir", light_dir);
    m2_shader.uniform("diffuse_color", diffuse_color);
    m2_shader.uniform("ambient_color", ambient_color);

    bool hadSky = false;

    if (draw_skybox && (draw_wmo || mapIndex.hasAGlobalWMO()))
    {
      for(WMOInstance* wmo : _wmos_with_skybox)
      {
        if (wmo->wmo->finishedLoading() && wmo->wmo->skybox)
        {
          if (wmo->group_extents.empty())
          {
            wmo->recalcExtents();
          }

          hadSky = wmo->wmo->draw_skybox( model_view
                                        , camera_pos
                                        , m2_shader
                                        , frustum
                                        , culldistance
                                        , animtime
                                        , draw_model_animations
                                        , wmo->extents[0]
                                        , wmo->extents[1]
                                        , wmo->group_extents
                                        , _model_texture_handler
                                        );
        }
      }
    }

    if (!hadSky)
    {
      skies->draw( model_view
                 , projection
                 , camera_pos
                 , m2_shader
                 , frustum
                 , culldistance
                 , animtime
                 , draw_model_animations
                 , draw_skybox
                 , outdoorLightStats
                 , _model_texture_handler
                 );
    }
  }

  culldistance = draw_fog ? fogdistance : _view_distance;

  // Draw verylowres heightmap
  if (draw_fog && draw_terrain)
  {
    _horizon_render->draw (model_view, projection, &mapIndex, skies->color_set[FOG_COLOR], culldistance, frustum, camera_pos, display);
  }

  gl.enable(GL_DEPTH_TEST);
  gl.depthFunc(GL_LEQUAL); // less z-fighting artifacts this way, I think
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // height map w/ a zillion texture passes
  if (draw_terrain)
  {
    opengl::scoped::use_program mcnk_shader{ *_mcnk_program.get() };

    bool selected_texture_changed = false;
    if (_last_selected_texture != current_texture)
    {
      _last_selected_texture = current_texture;
      selected_texture_changed = true;
    }

    mcnk_shader.uniform("mvp", mvp);

    mcnk_shader.uniform ("draw_lines", (int)draw_lines);
    mcnk_shader.uniform ("draw_hole_lines", (int)draw_hole_lines);
    mcnk_shader.uniform ("draw_areaid_overlay", (int)draw_areaid_overlay);
    mcnk_shader.uniform ("draw_terrain_height_contour", (int)draw_contour);
    mcnk_shader.uniform ("draw_impassible_flag", (int)draw_chunk_flag_overlay);
    mcnk_shader.uniform ("show_unpaintable_chunks", (int)show_unpaintable_chunks);
    mcnk_shader.uniform ("show_selection_data", terrainMode == editing_mode::chunk_mover ? 1 : 0);

    mcnk_shader.uniform ("draw_wireframe", (int)draw_wireframe);

    mcnk_shader.uniform("draw_shadows", (int)draw_shadows);
    mcnk_shader.uniform("draw_vertex_colors", (int)draw_vertex_colors);

    mcnk_shader.uniform ("draw_fog", (int)draw_fog);
    mcnk_shader.uniform ("fog_color", math::vector_4d(skies->color_set[FOG_COLOR], 1));
    // !\ todo use light dbcs values
    mcnk_shader.uniform ("fog_end", fogdistance);
    mcnk_shader.uniform ("fog_start", 0.5f);
    mcnk_shader.uniform ("camera", camera_pos);

    mcnk_shader.uniform("light_dir", terrain_light_dir);
    mcnk_shader.uniform("diffuse_color", diffuse_color);
    mcnk_shader.uniform("ambient_color", ambient_color);

    mcnk_shader.uniform("anim_time", animtime / 1600.f);


    if (cursor == cursor_mode::terrain)
    {
      if (square_brush)
      {
        mcnk_shader.uniform("draw_cursor_circle", 0);
        mcnk_shader.uniform("draw_cursor_square", 1);
      }
      else
      {
        mcnk_shader.uniform("draw_cursor_circle", 1);
        mcnk_shader.uniform("draw_cursor_square", 0);
      }

      mcnk_shader.uniform ("cursor_position", cursor_pos);
      mcnk_shader.uniform ("outer_cursor_radius", brush_radius);
      mcnk_shader.uniform ("inner_cursor_ratio", inner_radius_ratio);
      mcnk_shader.uniform ("cursor_color", cursor_color);
    }
    else
    {
      mcnk_shader.uniform ("draw_cursor_circle", 0);
    }

    _tileset_handler.bind();

    // used for the alphamap/shadowmap combo
    opengl::texture::set_active_texture(0);

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->draw ( frustum
                 , mcnk_shader
                 , detailtexcoords
                 , culldistance
                 , camera_pos
                 , camera_moved
                 , selected_texture_changed
                 , _last_selected_texture
                 , area_id_colors
                 , display
                 , _tileset_handler
                 );
    }

    gl.bindVertexArray(0);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  if(cursor != cursor_mode::terrain && cursor != cursor_mode::none)
  {
    opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
    gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    noggit::cursor_render::mode mode;

    if (terrainMode == editing_mode::ground && ground_editing_brush == eTerrainType_Quadra)
    {
      mode = cursor == cursor_mode::sphere
        ? noggit::cursor_render::mode::square
        : noggit::cursor_render::mode::cube;
    }
    else if (cursor == cursor_mode::sphere)
    {
      mode = noggit::cursor_render::mode::sphere;
    }
    else
    {
      mode = noggit::cursor_render::mode::circle;
    }

    _cursor_render.draw(mode, mvp, cursor_color, cursor_pos, brush_radius, inner_radius_ratio);
  }

  if (use_ref_pos)
  {
    _sphere_render.draw(mvp, ref_pos, cursor_color, 2.f);
  }

  if (terrainMode == editing_mode::ground && ground_editing_brush == eTerrainType_Vertex)
  {
    float size = (vertexCenter() - camera_pos).length();
    gl.pointSize(std::max(0.001f, 10.0f - (1.25f * size / CHUNKSIZE)));

    for (math::vector_3d const* pos : _vertices_selected)
    {
      _sphere_render.draw(mvp, *pos, math::vector_4d(1.f, 0.f, 0.f, 1.f), 0.5f);
    }

    _sphere_render.draw(mvp, vertexCenter(), cursor_color, 2.f);
  }

  bool draw_doodads_wmo = draw_wmo && draw_wmo_doodads;

  std::unordered_map<Model*, std::size_t> model_with_particles;
  bool update_transform_buffers = camera_moved;

  // M2s / models
  if (draw_models || draw_doodads_wmo)
  {
    if (draw_model_animations)
    {
      ModelManager::resetAnim();
    }

    int models_display_mode = (draw_models ? 1 : 0) + (draw_doodads_wmo ? 2 : 0);

    if (models_display_mode != _model_display_mode)
    {
      update_transform_buffers = true;
      _model_display_mode = models_display_mode;
    }

    std::unordered_map<std::string, std::vector<ModelInstance*>>* models_to_draw =
      (draw_models && draw_doodads_wmo) ? &_models_by_filename_with_wmo_doodads :
      (draw_models ? &_models_by_filename : &_wmo_doodads_by_filename);

    // don't check every frame when models are loading to avoid big performance drop
    if (need_model_updates || (_models_still_loading && _last_unloaded_doodad_check++ > 60))
    {
      update_models_by_filename();
      update_transform_buffers = true;
    }

    std::unordered_map<Model*, std::size_t> model_boxes_to_draw;

    {
      opengl_model_state_changer ogl_state;

      opengl::scoped::use_program m2_shader {*_m2_instanced_program.get()};

      m2_shader.uniform("model_view", model_view);
      m2_shader.uniform("projection", projection);

      m2_shader.uniform("fog_color", math::vector_4d(skies->color_set[FOG_COLOR], 1));
      // !\ todo use light dbcs values
      m2_shader.uniform("fog_end", fogdistance);
      m2_shader.uniform("fog_start", 0.5f);
      m2_shader.uniform("draw_fog", (int)draw_fog);

      m2_shader.uniform("light_dir", light_dir);
      m2_shader.uniform("diffuse_color", diffuse_color);
      m2_shader.uniform("ambient_color", ambient_color);


      for (auto& it : *models_to_draw)
      {
        if (draw_hidden_models || !it.second[0]->model->is_hidden())
        {
          it.second[0]->model->draw( model_view
                                   , it.second
                                   , m2_shader
                                   , frustum
                                   , culldistance
                                   , camera_pos
                                   , false
                                   , animtime
                                   , draw_model_animations
                                   , draw_models_with_box
                                   , model_with_particles
                                   , model_boxes_to_draw
                                   , display
                                   , update_transform_buffers
                                   , _model_texture_handler
                                   , ogl_state
                                   );
        }
      }
    }

    if(draw_models_with_box || (draw_hidden_models && !model_boxes_to_draw.empty()))
    {
      opengl::scoped::use_program m2_box_shader{ *_m2_box_program.get() };

      m2_box_shader.uniform ("model_view", model_view);
      m2_box_shader.uniform ("projection", projection);

      opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint (GL_LINE_SMOOTH_HINT, GL_NICEST);

      for (auto& it : model_boxes_to_draw)
      {
        math::vector_4d color = it.first->is_hidden()
                              ? math::vector_4d(0.f, 0.f, 1.f, 1.f)
                              : ( it.first->use_fake_geometry()
                                ? math::vector_4d(1.f, 0.f, 0.f, 1.f)
                                : math::vector_4d(0.75f, 0.75f, 0.75f, 1.f)
                                )
                              ;

        m2_box_shader.uniform("color", color);
        it.first->draw_box(m2_box_shader, it.second);
      }
    }

    for (auto& selection : current_selection())
    {
      if (selection.index() == eEntry_Model)
      {
        auto model = std::get<selected_model_type>(selection);
        if (model->is_visible(frustum, culldistance, camera_pos, display))
        {
          model->draw_box(model_view, projection, true, _only_one_model_box);
        }
      }
    }
  }



  // WMOs / map objects
  if (draw_wmo || mapIndex.hasAGlobalWMO())
  {
    // updating liquids require a full transform buffer update
    // with the current implemetation or it duplicate the liquids
    // visually until the camera moves when moving a wmo with liquids
    if (need_model_updates || _need_wmo_liquid_update)
    {
      update_models_by_filename();
      update_transform_buffers = true;
      _need_wmo_liquid_update = false;
    }

    opengl::scoped::use_program wmo_program {*_wmo_program.get()};

    wmo_program.uniform("model_view", model_view);
    wmo_program.uniform("projection", projection);

    wmo_program.uniform("draw_fog", (int)draw_fog);

    if (draw_fog)
    {
      wmo_program.uniform("fog_end", fogdistance);
      wmo_program.uniform("fog_start", 0.5f);
      wmo_program.uniform("fog_color", skies->color_set[FOG_COLOR]);
      wmo_program.uniform("camera", camera_pos);
    }

    wmo_program.uniform("exterior_light_dir", light_dir);
    wmo_program.uniform("exterior_diffuse_color", diffuse_color);
    wmo_program.uniform("exterior_ambient_color", ambient_color);

    wmo_group_uniform_data wmo_uniform_data;


    // liquid list updated only when the transform buffers are updated
    if (update_transform_buffers)
    {
      _wmo_liquids_to_draw.clear();
    }

    for (auto& it : _wmos_by_filename)
    {
      WMO* wmo = it.second[0]->wmo.get();

      if (draw_hidden_models || !wmo->is_hidden())
      {
        wmo->draw_instanced ( wmo_program
                            , model_view
                            , projection
                            , it.second
                            , false
                            , frustum
                            , culldistance
                            , camera_pos
                            , draw_wmo_doodads
                            , draw_fog
                            , _liquid_render.value()
                            , animtime
                            , skies->hasSkies()
                            , display
                            , wmo_uniform_data
                            , _wmo_liquids_to_draw
                            , _model_texture_handler
                            , update_transform_buffers
                            );
      }
    }

    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl.enable(GL_CULL_FACE);

    if (draw_models_with_box || draw_hidden_models)
    {
      // use the same shader for m2s and wmos
      opengl::scoped::use_program wmo_box_shader{ *_m2_box_program.get() };

      wmo_box_shader.uniform("model_view", model_view);
      wmo_box_shader.uniform("projection", projection);

      opengl::scoped::bool_setter<GL_LINE_SMOOTH, GL_TRUE> const line_smooth;
      gl.hint(GL_LINE_SMOOTH_HINT, GL_NICEST);

      for (auto& it : _wmos_by_filename)
      {
        bool hidden = it.second[0]->wmo->is_hidden();

        if (hidden && !draw_hidden_models)
        {
          continue;
        }

        math::vector_4d color = hidden
          ? math::vector_4d(0.f, 0.f, 1.f, 1.f)
          : math::vector_4d(0.75f, 0.75f, 0.75f, 1.f)
          ;

        wmo_box_shader.uniform("color", color);
        it.second[0]->wmo->draw_boxes_instanced(wmo_box_shader);
      }
    }

    for (auto& selection : current_selection())
    {
      if (selection.index() == eEntry_WMO)
      {
        auto wmo = std::get<selected_wmo_type>(selection);

        {
          wmo->draw_box_selected(model_view, projection, _only_one_model_box);
        }
      }
    }
  }

  // model particles
  if (draw_model_animations && !model_with_particles.empty())
  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    opengl::scoped::use_program particles_shader {*_m2_particles_program.get()};

    particles_shader.uniform("model_view_projection", mvp);

#ifndef USE_BINDLESS_TEXTURES
    opengl::texture::set_active_texture(0);
    particles_shader.uniform("tex_array", 0);
#endif

    for (auto& it : model_with_particles)
    {
      it.first->draw_particles(model_view, particles_shader, it.second, _model_texture_handler);
    }
  }

  if (draw_model_animations && !model_with_particles.empty())
  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    opengl::scoped::use_program ribbon_shader {*_m2_ribbons_program.get()};

    ribbon_shader.uniform("model_view_projection", mvp);

#ifndef USE_BINDLESS_TEXTURES
    opengl::texture::set_active_texture(0);
    ribbon_shader.uniform("tex_array", 0);
#endif

    gl.blendFunc(GL_SRC_ALPHA, GL_ONE);

    for (auto& it : model_with_particles)
    {
      it.first->draw_ribbons(ribbon_shader, it.second, _model_texture_handler);
    }
  }

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // render before the water and enable depth right
  // so it's visible under water
  // the checker board pattern is used to see the water under it
  if (angled_mode || use_ref_pos)
  {
    opengl::scoped::depth_mask_setter<GL_TRUE> const depth_mask;

    math::degrees orient = math::degrees(orientation);
    math::degrees incl = math::degrees(angle);
    math::vector_4d color = cursor_color;

    color.w = 0.75f;

    float radius = 1.2f * brush_radius;

    if (angled_mode && !use_ref_pos)
    {
      math::vector_3d pos = cursor_pos;
      pos.y += 0.1f; // to avoid z-fighting with the ground
      _square_render.draw(mvp, pos, radius, incl, orient, color);
    }
    else if (use_ref_pos)
    {
      if (angled_mode)
      {
        math::vector_3d pos = cursor_pos;
        pos.y = misc::angledHeight(ref_pos, pos, incl, orient);
        pos.y += 0.1f;
        _square_render.draw(mvp, pos, radius, incl, orient, color);

        // display the plane when the cursor is far from ref_point
        if (misc::dist(pos.x, pos.z, ref_pos.x, ref_pos.z) > 10.f + radius)
        {
          math::vector_3d ref = ref_pos;
          ref.y += 0.1f;
          _square_render.draw(mvp, ref, 10.f, incl, orient, color);
        }
      }
      else
      {
        math::vector_3d pos = cursor_pos;
        pos.y = ref_pos.y + 0.1f;
        _square_render.draw(mvp, pos, radius, math::degrees(0.f), math::degrees(0.f), color);
      }
    }
  }

  if(gizmo.is_linked())
  {
    opengl::scoped::use_program gizmo_shader{ *_gizmo_program.get() };

    gizmo_shader.uniform("view_projection", mvp);
    {
      gizmo_shader.uniform("alpha", 1.f);
      gizmo.draw(gizmo_shader);
    }
    {
      opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> const disable_depth_test;
      gizmo_shader.uniform("alpha", 0.5f);
      gizmo.draw(gizmo_shader);
    }
  }

  if (draw_water)
  {
    opengl::scoped::use_program water_shader{ _liquid_render->shader_program() };

    water_shader.uniform("animtime", static_cast<float>(animtime / 60.f));

    water_shader.uniform("model_view", model_view);
    water_shader.uniform("projection", projection);

    // use some default colors when not using light dbc data
    math::vector_4d ocean_color_light(use_dbc_lighting_data ? skies->color_set[OCEAN_COLOR_LIGHT] : math::vector_3d(0.2f, 0.3f, 0.35f), skies->ocean_shallow_alpha());
    math::vector_4d ocean_color_dark (use_dbc_lighting_data ? skies->color_set[OCEAN_COLOR_DARK]  : math::vector_3d(0.0f, 0.1f, 0.2f),  skies->ocean_deep_alpha());
    math::vector_4d river_color_light(use_dbc_lighting_data ? skies->color_set[RIVER_COLOR_LIGHT] : math::vector_3d(0.3f, 0.3f, 0.4f),  skies->river_shallow_alpha());
    math::vector_4d river_color_dark (use_dbc_lighting_data ? skies->color_set[RIVER_COLOR_DARK]  : math::vector_3d(0.2f, 0.2f, 0.3f),  skies->river_deep_alpha());

    water_shader.uniform("ocean_color_light", ocean_color_light);
    water_shader.uniform("ocean_color_dark", ocean_color_dark);
    water_shader.uniform("river_color_light", river_color_light);
    water_shader.uniform("river_color_dark", river_color_dark);

    if (cursor == cursor_mode::terrain && show_liquid_cursor)
    {
      water_shader.uniform("draw_cursor_circle", 1);
      water_shader.uniform("cursor_position", cursor_pos);
      water_shader.uniform("cursor_radius", brush_radius);
      water_shader.uniform("cursor_color", cursor_color);
    }
    else
    {
      water_shader.uniform("draw_cursor_circle", 0);
    }

    for (int i = 0; i < _liquid_render->array_count(); ++i)
    {
      water_shader.uniform("textures[" + std::to_string(i) + "]", i);
    }

    _liquid_render->bind_arrays();

    // draw the water on both sides
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;

    water_shader.uniform ("use_transform", 0);

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawWater ( frustum
                      , culldistance
                      , camera_pos
                      , camera_moved
                      , _liquid_render.value()
                      , water_shader
                      , animtime
                      , water_layer
                      , display
                      );
    }

    if (draw_wmo)
    {
      water_shader.uniform("use_transform", 1);

      for (auto& it : _wmo_liquids_to_draw)
      {
        it.first->draw(it.second, _liquid_render.value());
      }
    }

    gl.bindVertexArray(0);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  // draw last because of the transparency
  if (draw_mfbo)
  {
    // don't write on the depth buffer
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    opengl::scoped::use_program mfbo_shader {*_mfbo_program.get()};

    mfbo_shader.uniform("model_view_projection", model_view * projection);

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawMFBO(mfbo_shader);
    }
  }
}

void World::update_legacy_shadow_for_tile_at(math::vector_3d const& pos, float pitch, int threshold)
{
  for_tile_at(pos, [&](MapTile* tile) { update_legacy_shadows(tile, pitch, threshold); });
}

#include <math/projection.hpp>

void World::update_legacy_shadows(MapTile* tile, float pitch, int threshold)
{
  if(!_m2_depth_program)
  {
    _m2_depth_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_depth_vs") }
        #ifdef USE_BINDLESS_TEXTURES
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_depth_fs", {"use_bindless"}) }
        #else
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_depth_fs") }
        #endif
          }
      );
  }
  if(!_wmo_depth_program)
  {
    _wmo_depth_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("wmo_depth_vs") }
        #ifdef USE_BINDLESS_TEXTURES
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("wmo_depth_fs", {"use_bindless"}) }
        #else
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("wmo_depth_fs") }
        #endif
          }
      );
  }
  if (!_shadow_program)
  {
    _shadow_program.reset
      ( new opengl::program
        { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("shadow_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("shadow_fs") }
        }
      );
  }

  int depth_texture_size = 2048;

  if (!_framebuffers.buffer_generated())
  {
    _framebuffers.upload();

    {
      opengl::scoped::framebuffer_binder _ (_depth_framebuffer);

      gl.drawBuffer(GL_NONE);
      gl.readBuffer(GL_NONE);

      _depth_texture.bind();
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

      _depth_texture.attach_to_framebuffer(GL_DEPTH_ATTACHMENT, 0);
    }

    {
      opengl::scoped::framebuffer_binder _ (_shadow_framebuffer);

      _shadow_texture.bind();
      gl.texImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 1024, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

      _shadow_texture.attach_to_framebuffer(GL_COLOR_ATTACHMENT0, 0);
    }
  }

  auto const& extents = tile->extents();
  math::vector_3d center = (extents[0] + extents[1]) * 0.5f;
  center.y = extents[1].y;

  math::matrix_4x4 light_rotation = math::matrix_4x4(math::matrix_4x4::rotation_yzx, math::degrees::vec3(math::degrees(0), math::degrees(-45.f), math::degrees(pitch)));
  math::vector_3d light_forward = (light_rotation * math::vector_3d(1.f, 0.f, 0.f)).normalized();
  math::vector_3d light_up = (light_rotation * math::vector_3d(0.f, 1.f, 0.f)).normalized();

  math::vector_3d sun_position = center - (light_forward * 1000.f);

  math::matrix_4x4 depth_view = math::look_at(sun_position, center, light_up).transposed();
  math::matrix_4x4 depth_view_inverted = depth_view.inverted();

  math::vector_4d terrain_box(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
  float far_z = std::numeric_limits<float>::lowest();

  for (auto& point : tile->intersect_points())
  {
    math::vector_3d p = depth_view_inverted * (point - center);
    terrain_box.x = std::min(p.x, terrain_box.x);
    terrain_box.y = std::max(p.x, terrain_box.y);
    terrain_box.z = std::min(p.y, terrain_box.z);
    terrain_box.w = std::max(p.y, terrain_box.w);

    float dist = (point - sun_position).length();

    far_z = std::max(dist, far_z);
  }

  math::matrix_4x4 depth_proj = math::ortho(terrain_box.x, terrain_box.y, terrain_box.z, terrain_box.w, 100.f, far_z).transposed();



  // depth test
  {
    opengl::scoped::framebuffer_binder _ (_depth_framebuffer);

    opengl::texture::set_active_texture(2);


    _depth_texture.bind();
    gl.texImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depth_texture_size, depth_texture_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    gl.viewport(0, 0, depth_texture_size, depth_texture_size);
    gl.clear(GL_DEPTH_BUFFER_BIT);

    opengl_model_state_changer ogl_state;

    // models
    {
      opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> cull;
      opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_TRUE> depth;
      opengl::scoped::depth_mask_setter<GL_TRUE> const depth_mask;

      opengl::scoped::use_program depth_shader{ *_m2_depth_program.get() };

      depth_shader.uniform("view_proj", depth_view * depth_proj);

#ifndef USE_BINDLESS_TEXTURES
      depth_shader.uniform("array_0", 0);
      depth_shader.uniform("array_1", 1);
#endif

      for (auto& it : _models_by_filename_with_wmo_doodads)
      {
        it.second[0]->model->draw_depth(it.second, depth_shader, _model_texture_handler, ogl_state);
      }
    }

    // wmos
    {
      opengl::scoped::use_program depth_shader{ *_wmo_depth_program.get() };

      // make sure the depth and culling states are right, m2s can change those
      opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
      opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_TRUE> depth;
      opengl::scoped::depth_mask_setter<GL_TRUE> const depth_mask;

      depth_shader.uniform("view_proj", depth_view * depth_proj);

#ifndef USE_BINDLESS_TEXTURES
      depth_shader.uniform("array_0", 0);
#endif

      for (auto& it : _wmos_by_filename)
      {
        it.second[0]->wmo->draw_depth(depth_shader, it.second, _model_texture_handler);
      }
    }
  }

  // shadows
  {
    opengl::scoped::framebuffer_binder _ (_shadow_framebuffer);

    opengl::texture::set_active_texture(3);

    gl.viewport(0, 0, 1024, 1024);
    gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    math::vector_3d shadow_eye_pos = center + math::vector_3d(0.f, extents[1].y, 0.f);
    math::matrix_4x4 shadow_view = math::look_at(shadow_eye_pos, center, math::vector_3d(0.f, 0.f, -1.f)).transposed();
    math::matrix_4x4 shadow_view_inv = shadow_view.inverted();

    math::vector_4d shadow_box(std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
    float shadow_far_z = std::numeric_limits<float>::lowest();

    for (auto& point : tile->intersect_points())
    {
      math::vector_3d p = shadow_view_inv * (point - center);
      shadow_box.x = std::min(p.x, shadow_box.x);
      shadow_box.y = std::max(p.x, shadow_box.y);
      shadow_box.z = std::min(p.y, shadow_box.z);
      shadow_box.w = std::max(p.y, shadow_box.w);

      float dist = (point - sun_position).length();

      shadow_far_z = std::max(dist, shadow_far_z);
    }


    math::matrix_4x4 shadow_proj = math::ortho(shadow_box.x, shadow_box.y, shadow_box.z, shadow_box.w, -1.f, shadow_far_z).transposed();

    {
      opengl::scoped::use_program shadow_shader{ *_shadow_program.get() };
      shadow_shader.uniform("depth_texture", 2);
      shadow_shader.uniform("view_proj", shadow_view * shadow_proj);
      shadow_shader.uniform("light_view_proj", depth_view * depth_proj);
      shadow_shader.uniform("light_dir", light_forward);

      tile->draw_shadows(shadow_shader);
    }
  }


  opengl::texture::set_active_texture(3);
  _shadow_texture.bind();
  std::vector<std::uint8_t> shadow_data(1024 * 1024);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, shadow_data.data());

  tile->set_shadows(shadow_data, threshold);
}

selection_result World::intersect ( math::matrix_4x4 const& model_view
                                  , math::ray const& ray
                                  , bool pOnlyMap
                                  , bool do_objects
                                  , bool draw_terrain
                                  , bool draw_wmo
                                  , bool draw_models
                                  , bool draw_hidden_models
                                  , bool intersect_liquids
                                  , bool ignore_terrain_holes
                                  )
{
  selection_result results;

  if (draw_terrain)
  {
    for (auto&& tile : mapIndex.loaded_tiles())
    {
      tile->intersect (ray, &results, ignore_terrain_holes);
    }
  }

  if (intersect_liquids)
  {
    for (auto&& tile : mapIndex.loaded_tiles())
    {
      tile->Water.intersect(ray, &results);
    }
  }

  if (!pOnlyMap && do_objects)
  {
    if (draw_models)
    {
      _model_instance_storage.for_each_m2_instance([&] (ModelInstance& model_instance)
      {
        if (draw_hidden_models || !model_instance.model->is_hidden())
        {
          model_instance.intersect(model_view, ray, &results, animtime);
        }
      });
    }

    if (draw_wmo)
    {
      _model_instance_storage.for_each_wmo_instance([&] (WMOInstance& wmo_instance)
      {
        if (draw_hidden_models || !wmo_instance.wmo->is_hidden())
        {
          wmo_instance.intersect(ray, &results);
        }
      });
    }
  }

  std::sort ( results.begin()
          , results.end()
          , [](selection_entry const& lhs, selection_entry const& rhs)
            {
              return lhs.first < rhs.first;
            }
          );

  return results;
}

void World::update_models_emitters(float dt)
{
  while (dt > 0.1f)
  {
    ModelManager::updateEmitters(0.1f);
    dt -= 0.1f;
  }
  ModelManager::updateEmitters(dt);
}

unsigned int World::getAreaID (math::vector_3d const& pos)
{
  return for_maybe_chunk_at (pos, [&] (MapChunk* chunk) { return chunk->getAreaID(); }).value_or(-1);
}

void World::clearHeight(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk) {
    chunk->clearHeight();
  });
  for_all_chunks_on_tile(pos, [this] (MapChunk* chunk) {
      recalc_norms (chunk);
  });
}

void World::clearAllModelsOnADT(tile_index const& tile)
{
  _model_instance_storage.delete_instances_from_tile(tile, true, true);
  update_models_by_filename();
}

void World::CropWaterADT(const tile_index& pos)
{
  for_tile_at(pos, [](MapTile* tile) { tile->CropWater(); });
}

void World::setAreaID(math::vector_3d const& pos, int id, bool adt)
{
  if (adt)
  {
    for_all_chunks_on_tile(pos, [&](MapChunk* chunk) { chunk->setAreaID(id);});
  }
  else
  {
    for_chunk_at(pos, [&](MapChunk* chunk) { chunk->setAreaID(id);});
  }
}

std::optional<math::degrees::vec3> World::get_terrain_normal(math::vector_3d const& pos, bool smooth_normal)
{
  selection_result results;

  for_chunk_at(pos, [&](MapChunk* chunk)
  {
    {
      math::ray intersect_ray(pos, math::vector_3d(0.f, -1.f, 0.f));
      chunk->intersect(intersect_ray, &results, true);
    }
    // object is below ground
    if (results.empty())
    {
      math::ray intersect_ray(pos, math::vector_3d(0.f, 1.f, 0.f));
      chunk->intersect(intersect_ray, &results, true);
    }
  });

  if (results.empty())
  {
    return std::nullopt;
  }


  // We hit the terrain, now we take the normal of this position and use it to get the rotation we want.
  auto const& hitChunkInfo = std::get<selected_chunk_type>(results.front().second);

  math::quaternion q;
  math::vector_3d normal;

  // Surface Normal
  auto& p0 = hitChunkInfo.chunk->vertices[std::get<0>(hitChunkInfo.triangle)].position;
  auto& p1 = hitChunkInfo.chunk->vertices[std::get<1>(hitChunkInfo.triangle)].position;
  auto& p2 = hitChunkInfo.chunk->vertices[std::get<2>(hitChunkInfo.triangle)].position;

  math::vector_3d v1 = p1 - p0;
  math::vector_3d v2 = p2 - p0;

  const auto triangle_normal = v2 % v1;
  normal.x = triangle_normal.z;
  normal.y = triangle_normal.y;
  normal.z = triangle_normal.x;

  // Smooth option, gradient the normal towards closest vertex
  if (smooth_normal)
  {
    auto normalWeights = getBarycentricCoordinatesAt(p0, p1, p2, hitChunkInfo.position, normal);

    const auto& vNormal0 = hitChunkInfo.chunk->vertices[std::get<0>(hitChunkInfo.triangle)].normal;
    const auto& vNormal1 = hitChunkInfo.chunk->vertices[std::get<1>(hitChunkInfo.triangle)].normal;
    const auto& vNormal2 = hitChunkInfo.chunk->vertices[std::get<2>(hitChunkInfo.triangle)].normal;

    normal.x =
      vNormal0.x * normalWeights.x +
      vNormal1.x * normalWeights.y +
      vNormal2.x * normalWeights.z;

    normal.y =
      vNormal0.y * normalWeights.x +
      vNormal1.y * normalWeights.y +
      vNormal2.y * normalWeights.z;

    normal.z =
      vNormal0.z * normalWeights.x +
      vNormal1.z * normalWeights.y +
      vNormal2.z * normalWeights.z;
  }

  math::vector_3d a = math::vector_3d(0, 1, 0) % (normal);

  q.x = a.x;
  q.y = a.y;
  q.z = a.z;
  q.w = std::sqrt(normal.length_squared() + normal.y);
  q.normalize();

  return q.ToEulerAngles();
}

bool World::GetVertex(float x, float z, math::vector_3d *V) const
{
  tile_index tile({x, 0, z});

  if (!mapIndex.tileLoaded(tile))
  {
    return false;
  }

  MapTile* adt = mapIndex.getTile(tile);

  return adt->finishedLoading() && adt->GetVertex(x, z, V);
}

std::optional<float> World::get_exact_height_at(math::vector_3d const& pos)
{
  std::optional<float> height;

  for_chunk_at(pos, [&] (MapChunk* chunk)
  {
    height = *chunk->get_exact_height_at(pos);
  });

  return height;
}

template<typename Fun>
  bool World::for_all_chunks_in_range (math::vector_3d const& pos, float radius, Fun&& fun)
{
  bool changed (false);

  for (MapTile* tile : mapIndex.tiles_in_range (pos, radius))
  {
    if (!tile->finishedLoading())
    {
      continue;
    }

    for (MapChunk* chunk : tile->chunks_in_range (pos, radius))
    {
      if (fun (chunk))
      {
        changed = true;
        mapIndex.setChanged (tile);
      }
    }
  }

  return changed;
}
template<typename Fun, typename Post>
  bool World::for_all_chunks_in_range (math::vector_3d const& pos, float radius, Fun&& fun, Post&& post)
{
  std::forward_list<MapChunk*> modified_chunks;

  bool changed ( for_all_chunks_in_range
                   ( pos, radius
                   , [&] (MapChunk* chunk)
                     {
                       if (fun (chunk))
                       {
                         modified_chunks.emplace_front (chunk);
                         return true;
                       }
                       return false;
                     }
                   )
               );

  for (MapChunk* chunk : modified_chunks)
  {
    post (chunk);
  }

  return changed;
}

  void World::load_full_map()
  {
    for (int x = 0; x < 64; ++x)
    {
      for (int z = 0; z < 64; ++z)
      {
        tile_index idx(x, z);

        if (mapIndex.hasTile(idx))
        {
          // todo: find a better solution
          // to keep the tiles loaded
          mapIndex.setChanged(idx);
        }
      }
    }
  }


void World::changeShader(math::vector_3d const& pos, math::vector_4d const& color, float change, float radius, bool editMode)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->ChangeMCCV(pos, color, change, radius, editMode);
      }
    );
}

math::vector_3d World::pickShaderColor(math::vector_3d const& pos)
{
  math::vector_3d color = math::vector_3d(1.0f, 1.0f, 1.0f);
  for_all_chunks_in_range
  (pos, 0.1f
    , [&] (MapChunk* chunk)
  {
    color = chunk->pickMCCV(pos);
    return true;
  }
  );

  return color;
}

void World::changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius, terrain_edit_mode edit_mode)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->changeTerrain(pos, change, radius, BrushType, inner_radius, edit_mode);
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::flattenTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode, const math::vector_3d& origin, math::degrees angle, math::degrees orientation)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->flattenTerrain(pos, remain, radius, BrushType, mode, origin, angle, orientation);
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::blurTerrain(math::vector_3d const& pos, float remain, float radius, int BrushType, flatten_mode const& mode)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->blurTerrain ( pos
                                  , remain
                                  , radius
                                  , BrushType
                                  , mode
                                  , [this] (float x, float z) -> std::optional<float>
                                    {
                                      math::vector_3d vec;
                                      auto res (GetVertex (x, z, &vec));
                                      return res ? std::make_optional(vec.y) : std::nullopt;
                                    }
                                  );
      }
    , [this] (MapChunk* chunk)
      {
        recalc_norms (chunk);
      }
    );
}

void World::recalc_norms (MapChunk* chunk) const
{
  chunk->recalcNorms ( [this] (float x, float z) -> std::optional<float>
                       {
                         math::vector_3d vec;
                         auto res (GetVertex (x, z, &vec));
                         return res ? std::make_optional(vec.y) : std::nullopt;
                       }
                     );
}

bool World::paintTexture(math::vector_3d const& pos, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  return for_all_chunks_in_range
    ( pos, brush->get_radius()
    , [&] (MapChunk* chunk)
      {
        return chunk->paintTexture(pos, brush, strength, pressure, texture);
      }
    );
}

bool World::sprayTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, scoped_blp_texture_reference texture)
{
  bool succ = false;
  float inc = brush->get_radius() / 4.0f;

  for (float pz = pos.z - spraySize; pz < pos.z + spraySize; pz += inc)
  {
    for (float px = pos.x - spraySize; px < pos.x + spraySize; px += inc)
    {
      if ((sqrt(pow(px - pos.x, 2) + pow(pz - pos.z, 2)) <= spraySize) && ((rand() % 1000) < sprayPressure))
      {
        succ |= paintTexture({px, pos.y, pz}, brush, strength, pressure, texture);
      }
    }
  }

  return succ;
}

bool World::replaceTexture(math::vector_3d const& pos, Brush const& brush, float change, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture)
{
  return for_all_chunks_in_range
    ( pos, brush.get_radius()
      , [&](MapChunk* chunk)
      {
        return chunk->replaceTexture(pos, brush, change, old_texture, new_texture);
      }
    );
}

void World::clear_on_chunks ( math::vector_3d const& pos, float radius, bool height, bool textures, bool duplicate_textures
                            , bool textures_below_threshold, float alpha_threshold, bool texture_flags, bool liquids
                            , bool m2s, bool wmos, bool shadows, bool mccv, bool impassible_flag, bool holes
                            )
{
  for_all_chunks_in_range
  ( pos, radius
    , [&](MapChunk* chunk)
    {
      clear_on_chunk ( chunk, height, textures, duplicate_textures, textures_below_threshold, alpha_threshold
                     , texture_flags, liquids, shadows, mccv, impassible_flag, holes
                     );
      return true;
    }
  );

  // handle models separatly front the rest to avoid having to
  // check for each model on each chunk
  if (m2s || wmos)
  {
    _model_instance_storage.delete_instances_from_chunks_in_range(pos, radius, m2s, wmos);
    need_model_updates = true;
  }
}
void World::clear_on_tiles ( math::vector_3d const& pos, float radius, bool height, bool textures, bool duplicate_textures
                           , bool textures_below_threshold, float alpha_threshold, bool texture_flags, bool liquids
                           , bool m2s, bool wmos, bool shadows, bool mccv, bool impassible_flag, bool holes
                           )
{
  for_all_tiles_in_range
  ( pos, radius
    , [&](MapTile* tile)
    {
      if (m2s || wmos)
      {
        _model_instance_storage.delete_instances_from_tile(tile->index, m2s, wmos);
        need_model_updates = true;
      }

      for_all_chunks_on_tile({ tile->xbase + CHUNKSIZE, 0.f, tile->zbase + CHUNKSIZE },
        [&](MapChunk* chunk)
        {
          clear_on_chunk ( chunk, height, textures, duplicate_textures, textures_below_threshold, alpha_threshold
                         , texture_flags, liquids, shadows, mccv, impassible_flag, holes
                         );
          return true;
        }
      );
    }
  );

  need_model_updates = true;
}

void World::clear_on_chunk( MapChunk* chunk, bool height, bool textures, bool duplicate_textures
                          , bool textures_below_threshold, float alpha_threshold, bool texture_flags
                          , bool liquids, bool shadows, bool mccv, bool impassible_flag, bool holes
                          )
{
  if (height)
  {
    chunk->clearHeight();
  }
  if (textures)
  {
    chunk->eraseTextures();
  }
  if (duplicate_textures)
  {
    chunk->remove_texture_duplicates();
  }
  if (textures_below_threshold)
  {
    chunk->remove_unused_textures(alpha_threshold);
  }
  if (texture_flags)
  {
    chunk->clear_texture_flags();
  }
  if (liquids)
  {
    chunk->liquid_chunk()->clear_layers();
  }
  if (shadows)
  {
    chunk->clear_shadows();
  }
  if (mccv)
  {
    chunk->reset_mccv();
  }
  if (impassible_flag)
  {
    chunk->setFlag(false, 0x2);
  }
  if (holes)
  {
    chunk->setHole(chunk->vcenter, true, false);
  }
}

void World::eraseTextures(math::vector_3d const& pos)
{
  for_chunk_at(pos, [](MapChunk* chunk) {chunk->eraseTextures();});
}

void World::overwriteTextureAtCurrentChunk(math::vector_3d const& pos, scoped_blp_texture_reference const& oldTexture, scoped_blp_texture_reference newTexture)
{
  for_chunk_at(pos, [&](MapChunk* chunk) {chunk->switchTexture(oldTexture, std::move (newTexture));});
}

void World::setHole(math::vector_3d const& pos, bool big, bool hole)
{
  for_chunk_at(pos, [&](MapChunk* chunk) { chunk->setHole(pos, big, hole); });
}

void World::setHoleADT(math::vector_3d const& pos, bool hole)
{
  for_all_chunks_on_tile(pos, [&](MapChunk* chunk) { chunk->setHole(pos, true, hole); });
}

void World::set_shadow(math::vector_3d const& pos, float radius, bool add)
{
  for_all_chunks_in_range
  ( pos, radius
    , [&](MapChunk* chunk)
    {
      chunk->set_shadow(pos, radius, add);
      return true;
    }
  );
}


template<typename Fun>
  void World::for_all_chunks_on_tile (math::vector_3d const& pos, Fun&& fun)
{
  MapTile* tile (mapIndex.getTile (pos));

  if (tile && tile->finishedLoading())
  {
    mapIndex.setChanged(tile);

    for (size_t ty = 0; ty < 16; ++ty)
    {
      for (size_t tx = 0; tx < 16; ++tx)
      {
        fun(tile->getChunk(ty, tx));
      }
    }
  }
}

template<typename Fun>
  void World::for_chunk_at(math::vector_3d const& pos, Fun&& fun)
{
  MapTile* tile(mapIndex.getTile(pos));

  if (tile && tile->finishedLoading())
  {
    mapIndex.setChanged(tile);
    fun(tile->getChunk((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE));
  }
}

template<typename Fun>
  auto World::for_maybe_chunk_at(math::vector_3d const& pos, Fun&& fun) -> std::optional<decltype (fun (nullptr))>
{
  MapTile* tile (mapIndex.getTile (pos));
  if (tile && tile->finishedLoading())
  {
    return fun (tile->getChunk ((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE));
  }
  else
  {
    return std::nullopt;
  }
}

template<typename Fun>
  void World::for_tile_at(tile_index const& pos, Fun&& fun)
  {
    MapTile* tile(mapIndex.getTile(pos));
    if (tile && tile->finishedLoading())
    {
      mapIndex.setChanged(tile);
      fun(tile);
    }
  }

template<typename Fun>
  bool World::for_all_tiles_in_range(math::vector_3d const& pos, float radius, Fun&& fun)
  {
    bool changed = false;

    for (MapTile* tile : mapIndex.tiles_in_range(pos, radius))
    {
      if (tile && tile->finishedLoading())
      {
        changed = true;
        mapIndex.setChanged(tile);
        fun(tile);
      }
    }

    return changed;
  }

MapChunk * World::get_chunk_at(math::vector_3d const& pos)
{
  MapTile* tile(mapIndex.getTile(pos));
  if (tile && tile->finishedLoading())
  {
    return tile->getChunk((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE);
  }
  return nullptr;
}

void World::convert_alphamap(bool to_big_alpha)
{
  if (to_big_alpha == mapIndex.hasBigAlpha())
  {
    return;
  }

  for (size_t z = 0; z < 64; z++)
  {
    for (size_t x = 0; x < 64; x++)
    {
      tile_index tile(x, z);

      bool unload = !mapIndex.tileLoaded(tile) && !mapIndex.tileAwaitingLoading(tile);
      MapTile* mTile = mapIndex.loadTile(tile);

      if (mTile)
      {
        mTile->wait_until_loaded();

        mTile->convert_alphamap(to_big_alpha);
        mTile->saveTile(this);
        mapIndex.markOnDisc (tile, true);
        mapIndex.unsetChanged(tile);

        if (unload)
        {
          mapIndex.unloadTile(tile);
        }
      }
    }
  }

  mapIndex.convert_alphamap(to_big_alpha);
  mapIndex.save();
}

void World::saveMap (int, int)
{
  throw std::runtime_error("minimap saving not implemented");
}

void World::deleteModelInstance(int pUniqueID)
{
  auto instance = _model_instance_storage.get_model_instance(pUniqueID);

  if (instance)
  {
    remove_from_selection(instance.value());
  }
}

void World::deleteWMOInstance(int pUniqueID)
{
  auto instance = _model_instance_storage.get_wmo_instance(pUniqueID);

  if (instance)
  {
    remove_from_selection(instance.value());
  }
}

bool World::uid_duplicates_found() const
{
  return _model_instance_storage.uid_duplicates_found();
}

void World::delete_duplicate_model_and_wmo_instances()
{
  reset_selection();

  _model_instance_storage.clear_duplicates();
  need_model_updates = true;
}

void World::unload_every_model_and_wmo_instance()
{
  reset_selection();

  _model_instance_storage.clear();

  _wmos_with_skybox.clear();
  _wmos_by_filename.clear();
  _models_by_filename.clear();
  _wmo_doodads_by_filename.clear();
  _models_by_filename_with_wmo_doodads.clear();
}

ModelInstance* World::addM2 ( std::string const& filename
                  , math::vector_3d newPos
                  , float scale
                  , math::degrees::vec3 rotation
                  , noggit::object_paste_params* paste_params
                  )
{
  ModelInstance model_instance = ModelInstance(filename);

  model_instance.uid = mapIndex.newGUID();

  if (paste_params)
  {
    if (NoggitSettings.value("model/random_rotation", false).toBool())
    {
      float min = paste_params->minRotation;
      float max = paste_params->maxRotation;
      rotation.y += math::degrees(misc::randfloat(min, max));
    }

    if (NoggitSettings.value("model/random_tilt", false).toBool())
    {
      float min = paste_params->minTilt;
      float max = paste_params->maxTilt;
      rotation.x += math::degrees(misc::randfloat(min, max));
      rotation.z += math::degrees(misc::randfloat(min, max));
    }

    if (NoggitSettings.value("model/random_size", false).toBool())
    {
      float min = paste_params->minScale;
      float max = paste_params->maxScale;
      scale = misc::randfloat(min, max);
    }
  }

  model_instance.set_position(newPos);
  model_instance.set_scale(scale);
  model_instance.set_rotation(rotation);

  // to ensure the tiles are updated correctly
  model_instance.model->wait_until_loaded();
  model_instance.recalcExtents();

  std::uint32_t uid = _model_instance_storage.add_model_instance(std::move(model_instance), true);
  auto model = _model_instance_storage.get_model_instance(uid).value();

  _models_by_filename[filename].push_back(model);
  _models_by_filename_with_wmo_doodads[filename].push_back(model);

  return model;
}

WMOInstance* World::addWMO ( std::string const& filename
                   , math::vector_3d newPos
                   , math::degrees::vec3 rotation
                   )
{
  WMOInstance wmo_instance(filename);

  wmo_instance.mUniqueID = mapIndex.newGUID();
  wmo_instance.set_position(newPos);
  wmo_instance.set_rotation(rotation);

  // to ensure the tiles are updated correctly
  wmo_instance.wmo->wait_until_loaded();
  wmo_instance.recalcExtents();

  std::uint32_t uid = _model_instance_storage.add_wmo_instance(std::move(wmo_instance), true);
  auto wmo = _model_instance_storage.get_wmo_instance(uid).value();

  _wmos_by_filename[filename].push_back(wmo);

  if (wmo->wmo->skybox)
  {
    _wmos_with_skybox.push_back(wmo);
  }

  need_model_updates = true;

  return wmo;
}

void World::add_model(noggit::model_placement_data const& data)
{
  if (data.wmo)
  {
    addWMO(data.name, data.position, data.rotation);
  }
  else
  {
    addM2(data.name, data.position, data.scale, data.rotation, nullptr);
  }
}

std::uint32_t World::add_model_instance(ModelInstance model_instance, bool from_reloading)
{
  return _model_instance_storage.add_model_instance(std::move(model_instance), from_reloading);
}

std::uint32_t World::add_wmo_instance(WMOInstance wmo_instance, bool from_reloading)
{
  return _model_instance_storage.add_wmo_instance(std::move(wmo_instance), from_reloading);
}

std::optional<selection_type> World::get_model(std::uint32_t uid)
{
  return _model_instance_storage.get_instance(uid);
}

void World::remove_models_if_needed(std::vector<uint32_t> const& uids)
{
  // todo: manage instances properly
  // don't unload anything during the uid fix all,
  // otherwise models spanning several adts will be unloaded too soon
  if (mapIndex.uid_fix_all_in_progress())
  {
    return;
  }

  for (uint32_t uid : uids)
  {
    // it handles the removal from the selection if necessary
    _model_instance_storage.unload_instance_and_remove_from_selection_if_necessary(uid);
  }

  // deselect the terrain when an adt is unloaded
  if (_current_selection.size() == 1 && _current_selection.at(0).index() == eEntry_MapChunk)
  {
    reset_selection();
  }

  need_model_updates = true;
}

void World::reload_tile(tile_index const& tile)
{
  reset_selection();
  mapIndex.reloadTile(tile);
}

void World::ensure_tile_is_loaded(tile_index const& tile)
{
  MapTile* adt = mapIndex.loadTile(tile);

  if (adt)
  {
    adt->wait_until_loaded();
  }
}

void World::updateTilesEntry(selection_type const& entry, model_update type)
{
  if (entry.index() == eEntry_WMO)
  {
    updateTilesWMO (std::get<selected_wmo_type> (entry), type);
  }
  else if (entry.index() == eEntry_Model)
  {
    updateTilesModel (std::get<selected_model_type> (entry), type);
  }
}

void World::updateTilesWMO(WMOInstance* wmo, model_update type)
{
  _tile_update_queue.queue_update(wmo, type);

  if (wmo->wmo->has_liquids())
  {
    _need_wmo_liquid_update = true;
  }
  if (type == model_update::doodadset)
  {
    need_model_updates = true;
  }
}

void World::updateTilesModel(ModelInstance* m2, model_update type)
{
  _tile_update_queue.queue_update(m2, type);
}

void World::wait_for_all_tile_updates()
{
  _tile_update_queue.wait_for_all_update();
}

unsigned int World::getMapID()
{
  return mapIndex._map_id;
}

void World::clearTextures(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    chunk->eraseTextures();
  });
}

void World::setBaseTexture(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk)
  {
    chunk->eraseTextures();
    if (!!noggit::ui::selected_texture::get())
    {
      chunk->addTexture(*noggit::ui::selected_texture::get());
    }
  });
}

void World::clear_shadows(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [] (MapChunk* chunk)
  {
    chunk->clear_shadows();
  });
}

void World::swapTexture(math::vector_3d const& pos, scoped_blp_texture_reference tex)
{
  if (!!noggit::ui::selected_texture::get())
  {
    for_all_chunks_on_tile(pos, [&](MapChunk* chunk) { chunk->switchTexture(tex, *noggit::ui::selected_texture::get()); });
  }
}

void World::removeTexDuplicateOnADT(math::vector_3d const& pos)
{
  for_all_chunks_on_tile(pos, [](MapChunk* chunk) { chunk->texture_set->removeDuplicate(); } );
}

void World::change_texture_flag(math::vector_3d const& pos, scoped_blp_texture_reference const& tex, std::size_t flag, bool add)
{
  for_chunk_at(pos, [&] (MapChunk* chunk) { chunk->change_texture_flag(tex, flag, add); });
}

void World::paintLiquid( math::vector_3d const& pos
                       , float radius
                       , int liquid_id
                       , bool add
                       , math::radians const& angle
                       , math::radians const& orientation
                       , bool lock
                       , math::vector_3d const& origin
                       , bool override_height
                       , bool override_liquid_id
                       , float opacity_factor
                       )
{
  for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk)
  {
    chunk->liquid_chunk()->paintLiquid(pos, radius, liquid_id, add, angle, orientation, lock, origin, override_height, override_liquid_id, chunk, opacity_factor);
    return true;
  });
}

void World::setWaterType(const tile_index& pos, int type, int layer)
{
  for_tile_at ( pos
              , [&] (MapTile* tile)
                {
                  tile->Water.setType (type, layer);
                }
              );
}

int World::getWaterType(const tile_index& tile, int layer)
{
  if (mapIndex.tileLoaded(tile))
  {
    return mapIndex.getTile(tile)->Water.getType (layer);
  }
  else
  {
    return 0;
  }
}

void World::autoGenWaterTrans(const tile_index& pos, float factor)
{
  for_tile_at(pos, [&](MapTile* tile) { tile->Water.autoGen(factor); });
}

void World::update_water_opacity(math::vector_3d const& pos, float radius)
{
  for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk)
  {
    chunk->liquid_chunk()->auto_update_water_opacity(chunk);
    return true;
  });
}

void World::fixAllGaps()
{
  std::vector<MapChunk*> chunks;

  for (MapTile* tile : mapIndex.loaded_tiles())
  {
    MapTile* left = mapIndex.getTileLeft(tile);
    MapTile* above = mapIndex.getTileAbove(tile);
    bool tileChanged = false;

    // fix the gaps with the adt at the left of the current one
    if (left)
    {
      for (size_t ty = 0; ty < 16; ty++)
      {
        MapChunk* chunk = tile->getChunk(0, ty);
        if (chunk->fixGapLeft(left->getChunk(15, ty)))
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }

    // fix the gaps with the adt above the current one
    if (above)
    {
      for (size_t tx = 0; tx < 16; tx++)
      {
        MapChunk* chunk = tile->getChunk(tx, 0);
        if (chunk->fixGapAbove(above->getChunk(tx, 15)))
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }

    // fix gaps within the adt
    for (size_t ty = 0; ty < 16; ty++)
    {
      for (size_t tx = 0; tx < 16; tx++)
      {
        MapChunk* chunk = tile->getChunk(tx, ty);
        bool changed = false;

        // if the chunk isn't the first of the row
        if (tx && chunk->fixGapLeft(tile->getChunk(tx - 1, ty)))
        {
          changed = true;
        }

        // if the chunk isn't the first of the column
        if (ty && chunk->fixGapAbove(tile->getChunk(tx, ty - 1)))
        {
          changed = true;
        }

        if (changed)
        {
          chunks.emplace_back(chunk);
          tileChanged = true;
        }
      }
    }
    if (tileChanged)
    {
      mapIndex.setChanged(tile);
    }
  }

  for (MapChunk* chunk : chunks)
  {
    recalc_norms (chunk);
  }
}

bool World::isUnderMap(math::vector_3d const& pos)
{
  tile_index const tile (pos);

  if (mapIndex.tileLoaded(tile))
  {
    size_t chnkX = (pos.x / CHUNKSIZE) - tile.x * 16;
    size_t chnkZ = (pos.z / CHUNKSIZE) - tile.z * 16;

    // check using the cursor height
    return (mapIndex.getTile(tile)->getChunk(chnkX, chnkZ)->getMinHeight()) > pos.y + 2.0f;
  }

  return true;
}

void World::selectVertices(math::vector_3d const& pos, float radius)
{
  _vertex_center_updated = false;
  _vertex_border_updated = false;

  for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk){
    _vertex_chunks.emplace(chunk);
    _vertex_tiles.emplace(chunk->mt);
    chunk->selectVertex(pos, radius, _vertices_selected);
    return true;
  });
}

void World::delete_models(std::vector<selection_type> const& types)
{
  _model_instance_storage.delete_instances(types);
  need_model_updates = true;
}

void World::selectVertices(math::vector_3d const& pos1, math::vector_3d const& pos2)
{
  math::vector_3d pos_min = math::vector_3d(std::min(pos1.x,pos2.x),std::min(pos1.y,pos2.y),std::min(pos1.z,pos2.z));
  math::vector_3d pos_max = math::vector_3d(std::max(pos1.x,pos2.x),std::max(pos1.y,pos2.y),std::max(pos1.z,pos2.z));
  _vertex_center_updated = false;
  _vertex_border_updated = false;

  for_all_chunks_between(pos_min, pos_max, [&](MapChunk* chunk){
    _vertex_chunks.emplace(chunk);
    _vertex_tiles.emplace(chunk->mt);
    chunk->selectVertex(pos_min, pos_max, _vertices_selected);
    return true;
  });
}

void World::select_all_chunks_between(math::vector_3d const& pos1, math::vector_3d const& pos2, std::vector<MapChunk*>& chunks_in)
{
  math::vector_3d pos_min = math::vector_3d(std::min(pos1.x,pos2.x),std::min(pos1.y,pos2.y),std::min(pos1.z,pos2.z));
  math::vector_3d pos_max = math::vector_3d(std::max(pos1.x,pos2.x),std::max(pos1.y,pos2.y),std::max(pos1.z,pos2.z));

  for_all_chunks_between(pos_min, pos_max, [&](MapChunk* chunk){
    chunks_in.push_back(chunk);
    return true;
  });
}

std::set<math::vector_3d*>* World::getSelectedVertices()
{
  return &_vertices_selected;
}

template<typename Fun>
bool World::for_all_chunks_between (math::vector_3d const& pos1, math::vector_3d const& pos2,Fun&& fun)
{
  bool changed (false);

  for (MapTile* tile : mapIndex.tiles_between (pos1, pos2))
  {
    if (!tile->finishedLoading())
    {
      continue;
    }

    for (MapChunk* chunk : tile->chunks_between(pos1, pos2))
    {
      if (fun (chunk))
      {
        changed = true;
        mapIndex.setChanged (tile);
      }
    }
  }
  return changed;
}

bool World::deselectVertices(math::vector_3d const& pos, float radius)
{
  _vertex_center_updated = false;
  _vertex_border_updated = false;
  std::set<math::vector_3d*> inRange;

  for (math::vector_3d* v : _vertices_selected)
  {
    if (misc::dist(*v, pos) <= radius)
    {
      inRange.emplace(v);
    }
  }

  for (math::vector_3d* v : inRange)
  {
    _vertices_selected.erase(v);
  }

  return _vertices_selected.empty();
}

void World::moveVertices(float h)
{
  _vertex_center_updated = false;
  for (math::vector_3d* v : _vertices_selected)
  {
    v->y += h;
  }

  updateVertexCenter();
  updateSelectedVertices();
}

void World::updateSelectedVertices()
{
  for (MapTile* tile : _vertex_tiles)
  {
    mapIndex.setChanged(tile);
  }

  // fix only the border chunks to be more efficient
  for (MapChunk* chunk : vertexBorderChunks())
  {
    chunk->fixVertices(_vertices_selected);
  }

  for (MapChunk* chunk : _vertex_chunks)
  {
    chunk->updateVerticesData();
    recalc_norms (chunk);
  }
}

void World::orientVertices ( math::vector_3d const& ref_pos
                           , math::degrees vertex_angle
                           , math::degrees vertex_orientation
                           )
{
  for (math::vector_3d* v : _vertices_selected)
  {
    v->y = misc::angledHeight(ref_pos, *v, vertex_angle, vertex_orientation);
  }
  updateSelectedVertices();
}

void World::flattenVertices (float height)
{
  for (math::vector_3d* v : _vertices_selected)
  {
    v->y = height;
  }
  updateSelectedVertices();
}

void World::clearVertexSelection()
{
  _vertex_border_updated = false;
  _vertex_center_updated = false;
  _vertices_selected.clear();
  _vertex_chunks.clear();
  _vertex_tiles.clear();
}

void World::updateVertexCenter()
{
  _vertex_center_updated = true;
  _vertex_center = { 0,0,0 };
  float f = 1.0f / _vertices_selected.size();
  for (math::vector_3d* v : _vertices_selected)
  {
    _vertex_center += (*v) * f;
  }
}

math::vector_3d const& World::vertexCenter()
{
  if (!_vertex_center_updated)
  {
    updateVertexCenter();
  }

  return _vertex_center;
}

std::set<MapChunk*>& World::vertexBorderChunks()
{
  if (!_vertex_border_updated)
  {
    _vertex_border_updated = true;
    _vertex_border_chunks.clear();

    for (MapChunk* chunk : _vertex_chunks)
    {
      if (chunk->isBorderChunk(_vertices_selected))
      {
        _vertex_border_chunks.emplace(chunk);
      }
    }
  }
  return _vertex_border_chunks;
}

void World::update_models_by_filename()
{
  _wmos_with_skybox.clear();
  _wmos_by_filename.clear();
  _models_by_filename.clear();
  _wmo_doodads_by_filename.clear();
  _models_by_filename_with_wmo_doodads.clear();

  _models_still_loading = false;

  _model_instance_storage.for_each_m2_instance([&] (ModelInstance& model_instance)
  {
    _models_by_filename[model_instance.model->filename].push_back(&model_instance);
    _models_by_filename_with_wmo_doodads[model_instance.model->filename].push_back(&model_instance);

    // to make sure the transform matrix are up to date
    if(model_instance.need_recalc_extents())
    {
      if (!model_instance.recalcExtents())
      {
        _models_still_loading = true;
      }
    }
  });


  _last_unloaded_doodad_check = 0;

  _model_instance_storage.for_each_wmo_instance([&](WMOInstance& wmo_instance)
  {
    _wmos_by_filename[wmo_instance.wmo->filename].push_back(&wmo_instance);

    if (wmo_instance.wmo->skybox)
    {
      _wmos_with_skybox.push_back(&wmo_instance);
    }

    if (wmo_instance.need_recalc_extents())
    {
      wmo_instance.recalcExtents();
    }

    if(wmo_instance.need_doodads_update())
    {
      wmo_instance.update_doodads();
    }

    for (auto& doodad : wmo_instance.get_current_doodads())
    {
      if (!doodad->model->finishedLoading())
      {
        _models_still_loading = true;
        continue;
      }

      _wmo_doodads_by_filename[doodad->model->filename].push_back(doodad);
      _models_by_filename_with_wmo_doodads[doodad->model->filename].push_back(doodad);

      if (doodad->need_matrix_update())
      {
        doodad->update_transform_matrix_wmo(&wmo_instance);
      }
    }
  });

  need_model_updates = false;
}

void World::select_chunks_in_range(math::vector_3d const& pos, float radius, bool square_select, bool deselect, noggit::chunk_mover& chunk_mover)
{
  std::vector<selection_type> chunks;

  if (square_select)
  {
    math::vector_3d bounds(radius, 0.f, radius);
    for_all_chunks_between(pos - bounds, pos + bounds, [&](MapChunk* chunk)
    {
      // we only need the chunk's pointer here
      chunks.push_back(selected_chunk_type(chunk, { 0,0,0 }, math::vector_3d()));
      return true;
    });
  }
  else
  {
    for_all_chunks_in_range(pos, radius, [&](MapChunk* chunk)
    {
      // we only need the chunk's pointer here
      chunks.push_back(selected_chunk_type(chunk, { 0,0,0 }, math::vector_3d()));
      return true;
    });
  }


  if (deselect)
  {
    chunk_mover.remove_from_selection(chunks);
  }
  else
  {
    chunk_mover.add_to_selection(chunks);
  }
}
