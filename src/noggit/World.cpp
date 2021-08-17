// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/World.h>

#include <math/frustum.hpp>
#include <noggit/Brush.h> // brush
#include <noggit/ChunkWater.hpp>
#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/TextureManager.h>
#include <noggit/TileWater.hpp>// tile water
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/map_index.hpp>
#include <noggit/texture_set.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/TexturingGUI.h>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <boost/filesystem.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/thread/thread.hpp>

#include <QtWidgets/QMessageBox>

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
  ssfilename << "World\\Maps\\" << lMapName << "\\" << lMapName << ".wdt";

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
  , mapIndex (name, map_id, this)
  , horizon(name, &mapIndex)
  , mWmoFilename("")
  , mWmoEntry(ENTRY_MODF())
  , detailtexcoords(0)
  , alphatexcoords(0)
  , ol(nullptr)
  , animtime(0)
  , time(1450)
  , basename(name)
  , fogdistance(777.0f)
  , culldistance(fogdistance)
  , skies(nullptr)
  , outdoorLightStats(OutdoorLightStats())
  , _current_selection()
  , _settings (new QSettings())
  , _view_distance(_settings->value ("view_distance", 1000.f).toFloat())
{
  LogDebug << "Loading world \"" << name << "\"." << std::endl;
}

void World::update_selection_pivot()
{
  if (has_multiple_model_selected())
  {
    math::vector_3d pivot;
    int model_count = 0;

    for (auto const& entry : _current_selection)
    {
      if (entry.which() == eEntry_Model)
      {
        pivot += boost::get<selected_model_type>(entry)->pos;
        model_count++;
      }
      else if (entry.which() == eEntry_WMO)
      {
        pivot += boost::get<selected_wmo_type>(entry)->pos;
        model_count++;
      }
    }

    _multi_select_pivot = pivot / static_cast<float>(model_count);
  }
  else
  {
    _multi_select_pivot = boost::none;
  }
}

bool World::is_selected(selection_type selection) const
{
  if (selection.which() == eEntry_Model)
  {
    uint uid = boost::get<selected_model_type>(selection)->uid;
    auto const& it = std::find_if(_current_selection.begin()
                                  , _current_selection.end()
                                  , [uid] (selection_type type)
    {
      return type.type() == typeid(selected_model_type)
        && boost::get<selected_model_type>(type)->uid == uid;
    }
    );

    if (it != _current_selection.end())
    {
      return true;
    }
  }
  else if (selection.which() == eEntry_WMO)
  {
    uint uid = boost::get<selected_wmo_type>(selection)->mUniqueID;
    auto const& it = std::find_if(_current_selection.begin()
                            , _current_selection.end()
                            , [uid] (selection_type type)
    {
      return type.type() == typeid(selected_wmo_type)
        && boost::get<selected_wmo_type>(type)->mUniqueID == uid;
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
    if (entry.which() == eEntry_WMO)
    {
      if (boost::get<selected_wmo_type>(entry)->mUniqueID == uid)
      {
        return true;
      }
    }
    else if (entry.which() == eEntry_Model)
    {
      if (boost::get<selected_model_type>(entry)->uid == uid)
      {
        return true;
      }
    }
  }

  return false;
}

boost::optional<selection_type> World::get_last_selected_model() const
{
  auto const it
    ( std::find_if ( _current_selection.rbegin()
                   , _current_selection.rend()
                   , [&] (selection_type const& entry)
                     {
                       return entry.which() != eEntry_MapChunk;
                     }
                   )
    );

  return it == _current_selection.rend()
    ? boost::optional<selection_type>() : boost::optional<selection_type> (*it);
}

void World::set_current_selection(selection_type entry)
{
  _current_selection.clear();
  _current_selection.push_back(entry);
  _multi_select_pivot = boost::none;

  _selected_model_count = entry.which() == eEntry_MapChunk ? 0 : 1;
}

void World::add_to_selection(selection_type entry)
{
  if (entry.which() != eEntry_MapChunk)
  {
    _selected_model_count++;
  }

  _current_selection.push_back(entry);
  update_selection_pivot();
}

void World::remove_from_selection(selection_type entry)
{
  std::vector<selection_type>::iterator position = std::find(_current_selection.begin(), _current_selection.end(), entry);
  if (position != _current_selection.end())
  {
    if (entry.which() != eEntry_MapChunk)
    {
      _selected_model_count--;
    }

    _current_selection.erase(position);
    update_selection_pivot();
  }
}

void World::remove_from_selection(std::uint32_t uid)
{
  for (auto it = _current_selection.begin(); it != _current_selection.end(); ++it)
  {
    if (it->which() == eEntry_Model && boost::get<selected_model_type>(*it)->uid == uid)
    {
      _current_selection.erase(it);
      update_selection_pivot();
      return;
    }
    else if (it->which() == eEntry_WMO && boost::get<selected_wmo_type>(*it)->mUniqueID == uid)
    {
      _current_selection.erase(it);
      update_selection_pivot();
      return;
    }
  }
}

void World::reset_selection()
{
  _current_selection.clear();
  _multi_select_pivot = boost::none;
  _selected_model_count = 0;
}

void World::delete_selected_models()
{
  _model_instance_storage.delete_instances(_current_selection);
  need_model_updates = true;
  reset_selection();
}

void World::snap_selected_models_to_the_ground()
{
  for (auto& entry : _current_selection)
  {
    auto type = entry.which();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    bool entry_is_m2 = type == eEntry_Model;

    math::vector_3d& pos = entry_is_m2
      ? boost::get<selected_model_type>(entry)->pos
      : boost::get<selected_wmo_type>(entry)->pos
      ;

    boost::optional<float> height = get_exact_height_at(pos);

    // this should never happen
    if (!height)
    {
      LogError << "Snap to ground ray intersection failed" << std::endl;
      continue;
    }

    // the ground can only be intersected once
    pos.y = height.get();

    if (entry_is_m2)
    {
      boost::get<selected_model_type>(entry)->recalcExtents();
    }
    else
    {
      boost::get<selected_wmo_type>(entry)->recalcExtents();
    }

    updateTilesEntry(entry, model_update::add);
  }

  update_selection_pivot();
}

void World::scale_selected_models(float v, m2_scaling_type type)
{
  for (auto& entry : _current_selection)
  {
    if (entry.which() == eEntry_Model)
    {
      ModelInstance* mi = boost::get<selected_model_type>(entry);

      float scale = mi->scale;

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
      if (std::abs(scale - mi->scale) < ModelInstance::min_scale())
      {
        continue;
      }

      updateTilesModel(mi, model_update::remove);
      mi->scale = std::min(ModelInstance::max_scale(), std::max(ModelInstance::min_scale(), scale));
      mi->recalcExtents();
      updateTilesModel(mi, model_update::add);
    }
  }
}

void World::move_selected_models(float dx, float dy, float dz)
{
  for (auto& entry : _current_selection)
  {
    auto type = entry.which();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    bool entry_is_m2 = type == eEntry_Model;

    math::vector_3d& pos = entry_is_m2
      ? boost::get<selected_model_type>(entry)->pos
      : boost::get<selected_wmo_type>(entry)->pos
      ;

    updateTilesEntry(entry, model_update::remove);

    pos.x += dx;
    pos.y += dy;
    pos.z += dz;

    if (entry_is_m2)
    {
      boost::get<selected_model_type>(entry)->recalcExtents();
    }
    else
    {
      boost::get<selected_wmo_type>(entry)->recalcExtents();
    }

    updateTilesEntry(entry, model_update::add);
  }

  update_selection_pivot();
}

void World::set_selected_models_pos(math::vector_3d const& pos, bool change_height)
{
  // move models relative to the pivot when several are selected
  if (has_multiple_model_selected())
  {
    math::vector_3d diff = pos - _multi_select_pivot.get();

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
    auto type = entry.which();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    bool entry_is_m2 = type == eEntry_Model;

    updateTilesEntry(entry, model_update::remove);

    if (entry_is_m2)
    {
      ModelInstance* mi = boost::get<selected_model_type>(entry);
      mi->pos = pos;
      mi->recalcExtents();
    }
    else
    {
      WMOInstance* wi = boost::get<selected_wmo_type>(entry);
      wi->pos = pos;
      wi->recalcExtents();
    }

    updateTilesEntry(entry, model_update::add);
  }

  update_selection_pivot();
}

void World::rotate_selected_models(math::degrees rx, math::degrees ry, math::degrees rz, bool use_pivot)
{
  math::degrees::vec3 dir_change(rx, ry, rz);
  bool has_multi_select = has_multiple_model_selected();

  for (auto& entry : _current_selection)
  {
    auto type = entry.which();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    bool entry_is_m2 = type == eEntry_Model;

    updateTilesEntry(entry, model_update::remove);

    if (use_pivot && has_multi_select)
    {
      math::vector_3d& pos = entry_is_m2
        ? boost::get<selected_model_type>(entry)->pos
        : boost::get<selected_wmo_type>(entry)->pos
        ;

      math::vector_3d diff_pos = pos - _multi_select_pivot.get();
      math::vector_3d rot_result = math::matrix_4x4(math::matrix_4x4::rotation_xyz, {rx, ry, rz}) * diff_pos;

      pos += rot_result - diff_pos;
    }

    math::degrees::vec3& dir = entry_is_m2
        ? boost::get<selected_model_type>(entry)->dir
        : boost::get<selected_wmo_type>(entry)->dir
        ;

    dir += dir_change;

    if (entry_is_m2)
    {
      boost::get<selected_model_type>(entry)->recalcExtents();
    }
    else
    {
      boost::get<selected_wmo_type>(entry)->recalcExtents();
    }

    updateTilesEntry(entry, model_update::add);
  }
}

void World::rotate_selected_models_randomly(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
  for (auto& entry : _current_selection)
  {
    auto type = entry.which();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    bool entry_is_m2 = type == eEntry_Model;

    updateTilesEntry(entry, model_update::remove);

    math::degrees::vec3& dir = entry_is_m2
      ? boost::get<selected_model_type>(entry)->dir
      : boost::get<selected_wmo_type>(entry)->dir
      ;
    float rx = misc::randfloat(minX, maxX);
    float ry = misc::randfloat(minY, maxY);
    float rz = misc::randfloat(minZ, maxZ);

    math::quaternion baseRotation = math::quaternion(math::radians(math::degrees(dir.z)), math::radians(math::degrees(-dir.y)), math::radians(math::degrees(dir.x)));
    math::quaternion newRotation = math::quaternion(math::radians(math::degrees(rx)), math::radians(math::degrees(ry)), math::radians(math::degrees(rz)));

    math::quaternion finalRotation = baseRotation % newRotation;
    finalRotation.normalize();

    dir = finalRotation.ToEulerAngles();

    if (entry_is_m2)
    {
      boost::get<selected_model_type>(entry)->recalcExtents();
    }
    else
    {
      boost::get<selected_wmo_type>(entry)->recalcExtents();
    }

    updateTilesEntry(entry, model_update::add);
  }
}

void World::set_selected_models_rotation(math::degrees rx, math::degrees ry, math::degrees rz)
{
  math::degrees::vec3 new_dir(rx, ry, rz);

  for (auto& entry : _current_selection)
  {
    auto type = entry.which();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    bool entry_is_m2 = type == eEntry_Model;

    updateTilesEntry(entry, model_update::remove);

    math::degrees::vec3& dir = entry_is_m2
      ? boost::get<selected_model_type>(entry)->dir
      : boost::get<selected_wmo_type>(entry)->dir
      ;

    dir = new_dir;

    if (entry_is_m2)
    {
      boost::get<selected_model_type>(entry)->recalcExtents();
    }
    else
    {
      boost::get<selected_wmo_type>(entry)->recalcExtents();
    }

    updateTilesEntry(entry, model_update::add);
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
    auto type = entry.which();
    if (type == eEntry_MapChunk)
    {
      continue;
    }

    bool entry_is_m2 = type == eEntry_Model;

    updateTilesEntry(entry, model_update::remove);

    math::vector_3d rayPos = entry_is_m2
      ? boost::get<selected_model_type>(entry)->pos
      : boost::get<selected_wmo_type>(entry)->pos
      ;

    math::degrees::vec3& dir = entry_is_m2
      ? boost::get<selected_model_type>(entry)->dir
      : boost::get<selected_wmo_type>(entry)->dir
      ;

    selection_result results;
    for_chunk_at(rayPos, [&](MapChunk* chunk)
    {
      {
        math::ray intersect_ray(rayPos, math::vector_3d(0.f, -1.f, 0.f));
        chunk->intersect(intersect_ray, &results);
      }
      // object is below ground
      if (results.empty())
      {
        math::ray intersect_ray(rayPos, math::vector_3d(0.f, 1.f, 0.f));
        chunk->intersect(intersect_ray, &results);
      }
    });

    // !\ todo We shouldn't end up with empty ever (but we do, on completely flat ground)
    if (results.empty())
    {
      // just to avoid models disappearing when this happens
      updateTilesEntry(entry, model_update::add);
      continue;
    }

    // We hit the terrain, now we take the normal of this position and use it to get the rotation we want.
    auto const& hitChunkInfo = boost::get<selected_chunk_type>(results.front().second);

    math::quaternion q;
    math::vector_3d varnormal;

    // Surface Normal
    auto &p0 = hitChunkInfo.chunk->mVertices[std::get<0>(hitChunkInfo.triangle)];
    auto &p1 = hitChunkInfo.chunk->mVertices[std::get<1>(hitChunkInfo.triangle)];
    auto &p2 = hitChunkInfo.chunk->mVertices[std::get<2>(hitChunkInfo.triangle)];

    math::vector_3d v1 = p1 - p0;
    math::vector_3d v2 = p2 - p0;

    const auto tmpVec = v2 % v1;
    varnormal.x = tmpVec.z;
    varnormal.y = tmpVec.y;
    varnormal.z = tmpVec.x;

    // Smooth option, gradient the normal towards closest vertex
    if (smoothNormals) // Vertex Normal
    {
      auto normalWeights = getBarycentricCoordinatesAt(p0, p1, p2, hitChunkInfo.position, varnormal);

      const auto& vNormal0 = hitChunkInfo.chunk->mNormals[std::get<0>(hitChunkInfo.triangle)];
      const auto& vNormal1 = hitChunkInfo.chunk->mNormals[std::get<1>(hitChunkInfo.triangle)];
      const auto& vNormal2 = hitChunkInfo.chunk->mNormals[std::get<2>(hitChunkInfo.triangle)];

      varnormal.x =
        vNormal0.x * normalWeights.x +
        vNormal1.x * normalWeights.y +
        vNormal2.x * normalWeights.z;

      varnormal.y =
        vNormal0.y * normalWeights.x +
        vNormal1.y * normalWeights.y +
        vNormal2.y * normalWeights.z;

      varnormal.z =
        vNormal0.z * normalWeights.x +
        vNormal1.z * normalWeights.y +
        vNormal2.z * normalWeights.z;
    }

    math::vector_3d worldUp = math::vector_3d(0, 1, 0);
    math::vector_3d a = worldUp % (varnormal);

    q.x = a.x;
    q.y = a.y;
    q.z = a.z;
    q.w = std::sqrt((worldUp.length_squared() * (varnormal.length_squared()))
                    + (worldUp * varnormal));
    q.normalize();

    // To euler, because wow
    dir = q.ToEulerAngles();

    if (entry_is_m2)
    {
      boost::get<selected_model_type>(entry)->recalcExtents();
    }
    else
    {
      boost::get<selected_wmo_type>(entry)->recalcExtents();
    }

    updateTilesEntry(entry, model_update::add);
  }
}

void World::initGlobalVBOs(GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords)
{
  if (!*pDetailTexCoords && !*pAlphaTexCoords)
  {
    math::vector_2d temp[mapbufsize], *vt;
    float tx, ty;

    // init texture coordinates for detail map:
    vt = temp;
    const float detail_half = 0.5f * detail_size / 8.0f;
    for (int j = 0; j<17; ++j) {
      for (int i = 0; i<((j % 2) ? 8 : 9); ++i) {
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
    gl.bufferData<GL_ARRAY_BUFFER> (*pDetailTexCoords, sizeof(temp), temp, GL_STATIC_DRAW);

    // init texture coordinates for alpha map:
    vt = temp;

    const float alpha_half = TEXDETAILSIZE / MINICHUNKSIZE;
    for (int j = 0; j<17; ++j) {
      for (int i = 0; i<((j % 2) ? 8 : 9); ++i) {
        tx = alpha_half * i *2.0f;
        ty = alpha_half * j;
        if (j % 2) {
          // offset by half
          tx += alpha_half;
        }
        *vt++ = math::vector_2d(tx, ty);
      }
    }

    gl.genBuffers(1, pAlphaTexCoords);
    gl.bufferData<GL_ARRAY_BUFFER> (*pAlphaTexCoords, sizeof(temp), temp, GL_STATIC_DRAW);
  }
}

void World::initDisplay()
{
  initGlobalVBOs(&detailtexcoords, &alphatexcoords);

  mapIndex.setAdt(false);

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
                 , math::vector_3d const& cursor_pos
                 , math::vector_4d const& cursor_color
                 , int cursor_type
                 , float brush_radius
                 , bool show_unpaintable_chunks
                 , bool draw_contour
                 , float inner_radius_ratio
                 , math::vector_3d const& ref_pos
                 , float angle
                 , float orientation
                 , bool use_ref_pos
                 , bool angled_mode
                 , bool draw_paintability_overlay
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
                 , std::map<int, misc::random_color>& area_id_colors
                 , bool draw_fog
                 , eTerrainType ground_editing_brush
                 , int water_layer
                 , display_mode display
                 )
{
  if (!_display_initialized)
  {
    initDisplay();
    _display_initialized = true;
  }

  math::matrix_4x4 const mvp(model_view * projection);
  math::frustum const frustum (mvp);

  cursor_mode cursor = static_cast<cursor_mode>(cursor_type);

  if (!_m2_program)
  {
    _m2_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
          }
      );
  }
  if (!_m2_instanced_program)
  {
    _m2_instanced_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("m2_vs", {"instanced"}) }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("m2_fs") }
          }
      );
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
        { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("ribbon_vs") }
        , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("ribbon_fs") }
        }
      );
  }
  if (!_m2_particles_program)
  {
    _m2_particles_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("particle_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("particle_fs") }
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
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("wmo_fs") }
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

  math::vector_3d diffuse_color(skies->color_set[LIGHT_GLOBAL_DIFFUSE] * outdoorLightStats.dayIntensity);
  math::vector_3d ambient_color(skies->color_set[LIGHT_GLOBAL_AMBIENT] * outdoorLightStats.ambientIntensity);

  // only draw the sky in 3D
  if(display == display_mode::in_3D)
  {
    opengl::scoped::use_program m2_shader {*_m2_program.get()};

    m2_shader.uniform("model_view", model_view);
    m2_shader.uniform("projection", projection);
    m2_shader.uniform("tex1", 0);
    m2_shader.uniform("tex2", 1);

    m2_shader.uniform("draw_fog", 0);

    m2_shader.uniform("light_dir", light_dir);
    m2_shader.uniform("diffuse_color", diffuse_color);
    m2_shader.uniform("ambient_color", ambient_color);

    bool hadSky = false;

    if (draw_wmo || mapIndex.hasAGlobalWMO())
    {
      _model_instance_storage.for_each_wmo_instance
      (
        [&] (WMOInstance& wmo)
        {
          if (wmo.wmo->finishedLoading() && wmo.wmo->skybox)
          {
            if (wmo.group_extents.empty())
            {
              wmo.recalcExtents();
            }

            hadSky = wmo.wmo->draw_skybox( model_view
                                         , camera_pos
                                         , m2_shader
                                         , frustum
                                         , culldistance
                                         , animtime
                                         , draw_model_animations
                                         , wmo.extents[0]
                                         , wmo.extents[1]
                                         , wmo.group_extents
                                         );
          }
        }
        , [&] () { return hadSky; }
      );
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
                 , outdoorLightStats
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

    mcnk_shader.uniform("model_view", model_view);
    mcnk_shader.uniform("projection", projection);

    mcnk_shader.uniform ("draw_lines", (int)draw_lines);
    mcnk_shader.uniform ("draw_hole_lines", (int)draw_hole_lines);
    mcnk_shader.uniform("draw_areaid_overlay", (int)draw_areaid_overlay);
    mcnk_shader.uniform ("draw_terrain_height_contour", (int)draw_contour);

    // the flag stays on if the last chunk drawn before leaving the editing tool has it
    if (!draw_chunk_flag_overlay)
    {
      mcnk_shader.uniform ("draw_impassible_flag", 0);
    }

    mcnk_shader.uniform ("draw_wireframe", (int)draw_wireframe);
    mcnk_shader.uniform ("wireframe_type", _settings->value("wireframe/type", 0).toInt());
    mcnk_shader.uniform ("wireframe_radius", _settings->value("wireframe/radius", 1.5f).toFloat());
    mcnk_shader.uniform ("wireframe_width", _settings->value ("wireframe/width", 1.f).toFloat());
    // !\ todo store the color somewhere ?
    QColor c = _settings->value("wireframe/color").value<QColor>();
    math::vector_4d wireframe_color(c.redF(), c.greenF(), c.blueF(), c.alphaF());
    mcnk_shader.uniform ("wireframe_color", wireframe_color);

    mcnk_shader.uniform ("draw_fog", (int)draw_fog);
    mcnk_shader.uniform ("fog_color", math::vector_4d(skies->color_set[FOG_COLOR], 1));
    // !\ todo use light dbcs values
    mcnk_shader.uniform ("fog_end", fogdistance);
    mcnk_shader.uniform ("fog_start", 0.5f);
    mcnk_shader.uniform ("camera", camera_pos);

    mcnk_shader.uniform("light_dir", terrain_light_dir);
    mcnk_shader.uniform("diffuse_color", diffuse_color);
    mcnk_shader.uniform("ambient_color", ambient_color);


    if (cursor == cursor_mode::terrain)
    {
      mcnk_shader.uniform ("draw_cursor_circle", 1);
      mcnk_shader.uniform ("cursor_position", cursor_pos);
      mcnk_shader.uniform ("outer_cursor_radius", brush_radius);
      mcnk_shader.uniform ("inner_cursor_ratio", inner_radius_ratio);
      mcnk_shader.uniform ("cursor_color", cursor_color);
    }
    else
    {
      mcnk_shader.uniform ("draw_cursor_circle", 0);
    }

    mcnk_shader.uniform("alphamap", 0);
    mcnk_shader.uniform("tex0", 1);
    mcnk_shader.uniform("tex1", 2);
    mcnk_shader.uniform("tex2", 3);
    mcnk_shader.uniform("tex3", 4);
    mcnk_shader.uniform("shadow_map", 5);

    mcnk_shader.uniform("tex_anim_0", math::vector_2d());
    mcnk_shader.uniform("tex_anim_1", math::vector_2d());
    mcnk_shader.uniform("tex_anim_2", math::vector_2d());
    mcnk_shader.uniform("tex_anim_3", math::vector_2d());

    bool previous_chunk_could_be_painted = true;
    bool previous_chunk_was_textured = true;
    mcnk_shader.uniform("cant_paint", 0);
    mcnk_shader.uniform("is_textured", 1);

    std::vector<int> textures_bound = { -1, -1, -1, -1 };

    // start true so the first chunk update the shadow texture regardless of whether it has shadows or not
    bool previous_chunk_had_shadows = true;    

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->draw ( frustum
                 , mcnk_shader
                 , detailtexcoords
                 , culldistance
                 , camera_pos
                 , camera_moved
                 , show_unpaintable_chunks
                 , draw_paintability_overlay
                 , draw_chunk_flag_overlay
                 , draw_areaid_overlay
                 , area_id_colors
                 , animtime
                 , display
                 , previous_chunk_had_shadows
                 , previous_chunk_was_textured
                 , previous_chunk_could_be_painted
                 , textures_bound
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

  std::unordered_map<std::string, std::vector<ModelInstance*>> _wmo_doodads;

  bool draw_doodads_wmo = draw_wmo && draw_wmo_doodads;
  if (draw_doodads_wmo)
  {
    _model_instance_storage.for_each_wmo_instance([&] (WMOInstance& wmo)
    {
      for (auto& doodad : wmo.get_visible_doodads(frustum, culldistance, camera_pos, draw_hidden_models, display))
      {
        _wmo_doodads[doodad->model->filename].push_back(doodad);
      }
    });
  }

  std::unordered_map<Model*, std::size_t> model_with_particles;

  // M2s / models
  if (draw_models || draw_doodads_wmo)
  {
    if (draw_model_animations)
    {
      ModelManager::resetAnim();
    }

    if (need_model_updates)
    {
      update_models_by_filename();
    }

    std::unordered_map<Model*, std::size_t> model_boxes_to_draw;

    {
      opengl::scoped::use_program m2_shader {*_m2_instanced_program.get()};

      m2_shader.uniform("model_view", model_view);
      m2_shader.uniform("projection", projection);
      m2_shader.uniform("tex1", 0);
      m2_shader.uniform("tex2", 1);

      m2_shader.uniform("fog_color", math::vector_4d(skies->color_set[FOG_COLOR], 1));
      // !\ todo use light dbcs values
      m2_shader.uniform("fog_end", fogdistance);
      m2_shader.uniform("fog_start", 0.5f);
      m2_shader.uniform("draw_fog", (int)draw_fog);

      m2_shader.uniform("light_dir", light_dir);
      m2_shader.uniform("diffuse_color", diffuse_color);
      m2_shader.uniform("ambient_color", ambient_color);

      if (draw_models)
      {
        for (auto& it : _models_by_filename)
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
                                     );
          }
        }
      }

      if (draw_doodads_wmo)
      {
        for (auto& it : _wmo_doodads)
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
      if (selection.which() == eEntry_Model)
      {
        auto model = boost::get<selected_model_type>(selection);
        if (model->is_visible(frustum, culldistance, camera_pos, display))
        {
          model->draw_box(model_view, projection, true);
        }
      }
    }
  }

  // set anim time only once per frame
  {
    opengl::scoped::use_program water_shader {_liquid_render->shader_program()};
    water_shader.uniform("animtime", static_cast<float>(animtime) / 2880.f);

    water_shader.uniform("model_view", model_view);
    water_shader.uniform("projection", projection);

    math::vector_4d ocean_color_light(skies->color_set[OCEAN_COLOR_LIGHT], skies->ocean_shallow_alpha());
    math::vector_4d ocean_color_dark(skies->color_set[OCEAN_COLOR_DARK], skies->ocean_deep_alpha());
    math::vector_4d river_color_light(skies->color_set[RIVER_COLOR_LIGHT], skies->river_shallow_alpha());
    math::vector_4d river_color_dark(skies->color_set[RIVER_COLOR_DARK], skies->river_deep_alpha());

    water_shader.uniform("ocean_color_light", ocean_color_light);
    water_shader.uniform("ocean_color_dark", ocean_color_dark);
    water_shader.uniform("river_color_light", river_color_light);
    water_shader.uniform("river_color_dark", river_color_dark);

    if (draw_wmo || mapIndex.hasAGlobalWMO())
    {
      water_shader.uniform("use_transform", 1);
    }
  }

  // WMOs / map objects
  if (draw_wmo || mapIndex.hasAGlobalWMO())
  {
    {
      opengl::scoped::use_program wmo_program {*_wmo_program.get()};

      wmo_program.uniform("model_view", model_view);
      wmo_program.uniform("projection", projection);
      wmo_program.uniform("tex1", 0);
      wmo_program.uniform("tex2", 1);

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

      _model_instance_storage.for_each_wmo_instance([&] (WMOInstance& wmo)
      {
        bool is_hidden = wmo.wmo->is_hidden();
        if (draw_hidden_models || !is_hidden)
        {
          wmo.draw( wmo_program
                  , model_view
                  , projection
                  , frustum
                  , culldistance
                  , camera_pos
                  , is_hidden
                  , draw_wmo_doodads
                  , draw_fog
                  , _liquid_render.get()
                  , current_selection()
                  , animtime
                  , skies->hasSkies()
                  , display
                  , wmo_uniform_data
                  );
        }
      });

      gl.enable(GL_BLEND);
      gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      gl.enable(GL_CULL_FACE);
    }
  }

  // model particles
  if (draw_model_animations && !model_with_particles.empty())
  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    opengl::scoped::use_program particles_shader {*_m2_particles_program.get()};

    particles_shader.uniform("model_view_projection", mvp);
    particles_shader.uniform("tex", 0);
    opengl::texture::set_active_texture(0);

    for (auto& it : model_with_particles)
    {
      it.first->draw_particles(model_view, particles_shader, it.second);
    }
  }

  if (draw_model_animations && !model_with_particles.empty())
  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    opengl::scoped::use_program ribbon_shader {*_m2_ribbons_program.get()};

    ribbon_shader.uniform("model_view_projection", mvp);
    ribbon_shader.uniform("tex", 0);

    gl.blendFunc(GL_SRC_ALPHA, GL_ONE);

    for (auto& it : model_with_particles)
    {
      it.first->draw_ribbons(ribbon_shader, it.second);
    }
  }

  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (terrainMode == editing_mode::object && has_multiple_model_selected())
  {
    opengl::scoped::bool_setter<GL_DEPTH_TEST, GL_FALSE> const disable_depth_test;
    
    float dist = (camera_pos - _multi_select_pivot.get()).length();
    _sphere_render.draw(mvp, _multi_select_pivot.get(), cursor_color, std::min(2.f, std::max(0.15f, dist * 0.02f)));
  }

  if (draw_water)
  {
    _liquid_render->force_texture_update();

    // draw the water on both sides
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> const cull;

    opengl::scoped::use_program water_shader{ _liquid_render->shader_program() };

    water_shader.uniform ("use_transform", 0);

    for (MapTile* tile : mapIndex.loaded_tiles())
    {
      tile->drawWater ( frustum
                      , culldistance
                      , camera_pos
                      , camera_moved
                      , _liquid_render.get()
                      , water_shader
                      , animtime
                      , water_layer
                      , display
                      );
    }

    gl.bindVertexArray(0);
    gl.bindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  if (angled_mode || use_ref_pos)
  {
    opengl::scoped::bool_setter<GL_CULL_FACE, GL_FALSE> cull;
    opengl::scoped::depth_mask_setter<GL_FALSE> const depth_mask;

    math::degrees orient = math::degrees(orientation);
    math::degrees incl = math::degrees(angle);
    math::vector_4d color = cursor_color;
    // always half transparent regardless or the cursor transparency
    color.w = 0.5f;

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

selection_result World::intersect ( math::matrix_4x4 const& model_view
                                  , math::ray const& ray
                                  , bool pOnlyMap
                                  , bool do_objects
                                  , bool draw_terrain
                                  , bool draw_wmo
                                  , bool draw_models
                                  , bool draw_hidden_models
                                  )
{
  selection_result results;

  if (draw_terrain)
  {
    for (auto&& tile : mapIndex.loaded_tiles())
    {
      tile->intersect (ray, &results);
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
  return for_maybe_chunk_at (pos, [&] (MapChunk* chunk) { return chunk->getAreaID(); }).get_value_or (-1);
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
  _model_instance_storage.delete_instances_from_tile(tile);
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

boost::optional<float> World::get_exact_height_at(math::vector_3d const& pos)
{
  boost::optional<float> height;

  for_chunk_at(pos, [&] (MapChunk* chunk)
  {
    height = chunk->get_exact_height_at(pos);
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

void World::changeTerrain(math::vector_3d const& pos, float change, float radius, int BrushType, float inner_radius)
{
  for_all_chunks_in_range
    ( pos, radius
    , [&] (MapChunk* chunk)
      {
        return chunk->changeTerrain(pos, change, radius, BrushType, inner_radius);
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
                                  , [this] (float x, float z) -> boost::optional<float>
                                    {
                                      math::vector_3d vec;
                                      auto res (GetVertex (x, z, &vec));
                                      return boost::make_optional (res, vec.y);
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
  chunk->recalcNorms ( [this] (float x, float z) -> boost::optional<float>
                       {
                         math::vector_3d vec;
                         auto res (GetVertex (x, z, &vec));
                         return boost::make_optional (res, vec.y);
                       }
                     );
}

bool World::paintTexture(math::vector_3d const& pos, Brush* brush, float strength, float pressure, scoped_blp_texture_reference texture)
{
  return for_all_chunks_in_range
    ( pos, brush->getRadius()
    , [&] (MapChunk* chunk)
      {
        return chunk->paintTexture(pos, brush, strength, pressure, texture);
      }
    );
}

bool World::sprayTexture(math::vector_3d const& pos, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, scoped_blp_texture_reference texture)
{
  bool succ = false;
  float inc = brush->getRadius() / 4.0f;

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

bool World::replaceTexture(math::vector_3d const& pos, float radius, scoped_blp_texture_reference const& old_texture, scoped_blp_texture_reference new_texture)
{
  return for_all_chunks_in_range
    ( pos, radius
      , [&](MapChunk* chunk)
      {
        return chunk->replaceTexture(pos, radius, old_texture, new_texture);
      }
    );
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
  auto World::for_maybe_chunk_at(math::vector_3d const& pos, Fun&& fun) -> boost::optional<decltype (fun (nullptr))>
{
  MapTile* tile (mapIndex.getTile (pos));
  if (tile && tile->finishedLoading())
  {
    return fun (tile->getChunk ((pos.x - tile->xbase) / CHUNKSIZE, (pos.z - tile->zbase) / CHUNKSIZE));
  }
  else
  {
    return boost::none;
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
    remove_from_selection(instance.get());
  }
}

void World::deleteWMOInstance(int pUniqueID)
{
  auto instance = _model_instance_storage.get_wmo_instance(pUniqueID);

  if (instance)
  {
    remove_from_selection(instance.get());
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

  _models_by_filename.clear();
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
  model_instance.pos = newPos;
  model_instance.scale = scale;
  model_instance.dir = rotation;

  if (_settings->value("model/random_rotation", false).toBool())
  {
    float min = paste_params->minRotation;
    float max = paste_params->maxRotation;
    model_instance.dir.y += math::degrees(misc::randfloat(min, max));
  }

  if (_settings->value ("model/random_tilt", false).toBool ())
  {
    float min = paste_params->minTilt;
    float max = paste_params->maxTilt;
    model_instance.dir.x += math::degrees(misc::randfloat(min, max));
    model_instance.dir.z += math::degrees(misc::randfloat(min, max));
  }

  if (_settings->value ("model/random_size", false).toBool ())
  {
    float min = paste_params->minScale;
    float max = paste_params->maxScale;
    model_instance.scale = misc::randfloat(min, max);
  }

  // to ensure the tiles are updated correctly
  model_instance.model->wait_until_loaded();
  model_instance.recalcExtents();

  std::uint32_t uid = _model_instance_storage.add_model_instance(std::move(model_instance), true);
  auto model = _model_instance_storage.get_model_instance(uid).get();
  _models_by_filename[filename].push_back(model);
  return model;
}

WMOInstance* World::addWMO ( std::string const& filename
                   , math::vector_3d newPos
                   , math::degrees::vec3 rotation
                   )
{
  WMOInstance wmo_instance(filename);

  wmo_instance.mUniqueID = mapIndex.newGUID();
  wmo_instance.pos = newPos;
  wmo_instance.dir = rotation;

  // to ensure the tiles are updated correctly
  wmo_instance.wmo->wait_until_loaded();
  wmo_instance.recalcExtents();

  auto uid = _model_instance_storage.add_wmo_instance(std::move(wmo_instance), true);
  return _model_instance_storage.get_wmo_instance(uid).get();
}

std::uint32_t World::add_model_instance(ModelInstance model_instance, bool from_reloading)
{
  return _model_instance_storage.add_model_instance(std::move(model_instance), from_reloading);
}

std::uint32_t World::add_wmo_instance(WMOInstance wmo_instance, bool from_reloading)
{
  return _model_instance_storage.add_wmo_instance(std::move(wmo_instance), from_reloading);
}

boost::optional<selection_type> World::get_model(std::uint32_t uid)
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
  if (_current_selection.size() == 1 && _current_selection.at(0).which() == eEntry_MapChunk)
  {
    reset_selection();
  }

  update_models_by_filename();
}

void World::reload_tile(tile_index const& tile)
{
  reset_selection();
  mapIndex.reloadTile(tile);
}

void World::updateTilesEntry(selection_type const& entry, model_update type)
{
  if (entry.which() == eEntry_WMO)
  {
    updateTilesWMO (boost::get<selected_wmo_type> (entry), type);
  }
  else if (entry.which() == eEntry_Model)
  {
    updateTilesModel (boost::get<selected_model_type> (entry), type);
  }
}

void World::updateTilesWMO(WMOInstance* wmo, model_update type)
{
  _tile_update_queue.queue_update(wmo, type);
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
  _models_by_filename.clear();

  _model_instance_storage.for_each_m2_instance([&] (ModelInstance& model_instance)
  {
    _models_by_filename[model_instance.model->filename].push_back(&model_instance);
    // to make sure the transform matrix are up to date
    model_instance.recalcExtents();
  });

  need_model_updates = false;
}
