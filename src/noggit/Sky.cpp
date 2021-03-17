// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/Log.h>
#include <noggit/Model.h> // Model
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Sky.h>
#include <noggit/World.h>
#include <opengl/shader.hpp>

#include <algorithm>
#include <string>

const float skymul = 36.0f;

SkyColor::SkyColor(int t, int col)
{
  time = t;
  color.z = ((col & 0x0000ff)) / 255.0f;
  color.y = ((col & 0x00ff00) >> 8) / 255.0f;
  color.x = ((col & 0xff0000) >> 16) / 255.0f;
}

Sky::Sky(DBCFile::Iterator data)
{
  pos = math::vector_3d(data->getFloat(LightDB::PositionX) / skymul, data->getFloat(LightDB::PositionY) / skymul, data->getFloat(LightDB::PositionZ) / skymul);
  r1 = data->getFloat(LightDB::RadiusInner) / skymul;
  r2 = data->getFloat(LightDB::RadiusOuter) / skymul;

  for (int i = 0; i < 36; ++i)
  {
    mmin[i] = -2;
  }

  global = (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f);

  int light_param_0 = data->getInt(LightDB::DataIDs);
  int light_int_start = light_param_0 * NUM_SkyColorNames - 17; // cromons light fix ;) Thanks

  for (int i = 0; i < NUM_SkyColorNames; ++i)
  {
    try
    {
      DBCFile::Record rec = gLightIntBandDB.getByID(light_int_start + i);
      int entries = rec.getInt(LightIntBandDB::Entries);

      if (entries == 0)
      {
        mmin[i] = -1;
      }
      else
      {
        mmin[i] = rec.getInt(LightIntBandDB::Times);
        for (int l = 0; l < entries; l++)
        {
          SkyColor sc(rec.getInt(LightIntBandDB::Times + l), rec.getInt(LightIntBandDB::Values + l));
          colorRows[i].push_back(sc);
        }
      }
    }
    catch (...)
    {
      LogError << "When trying to intialize sky " << data->getInt(LightDB::ID) << ", there was an error with getting an entry in a DBC (" << i << "). Sorry." << std::endl;
      DBCFile::Record rec = gLightIntBandDB.getByID(i);
      int entries = rec.getInt(LightIntBandDB::Entries);

      if (entries == 0)
      {
        mmin[i] = -1;
      }
      else
      {
        mmin[i] = rec.getInt(LightIntBandDB::Times);
        for (int l = 0; l < entries; l++)
        {
          SkyColor sc(rec.getInt(LightIntBandDB::Times + l), rec.getInt(LightIntBandDB::Values + l));
          colorRows[i].push_back(sc);
        }
      }
    }
  }

  try
  {
    DBCFile::Record light_param = gLightParamsDB.getByID(light_param_0);
    int skybox_id = light_param.getInt(LightParamsDB::skybox);

    _river_shallow_alpha = light_param.getFloat(LightParamsDB::water_shallow_alpha);
    _river_deep_alpha = light_param.getFloat(LightParamsDB::water_deep_alpha);
    _ocean_shallow_alpha = light_param.getFloat(LightParamsDB::ocean_shallow_alpha);
    _ocean_deep_alpha = light_param.getFloat(LightParamsDB::ocean_deep_alpha);

    if (skybox_id)
    {
      skybox.emplace(gLightSkyboxDB.getByID(skybox_id).getString(LightSkyboxDB::filename));
    }
  }
  catch (...)
  {
    LogError << "When trying to get the skybox for the entry " << light_param_0 << " in LightParams.dbc. Sad." << std::endl;
  }
}

math::vector_3d Sky::colorFor(int r, int t) const
{
  if (mmin[r]<0)
  {
    return math::vector_3d(0, 0, 0);
  }
  math::vector_3d c1, c2;
  int t1, t2;
  size_t last = colorRows[r].size() - 1;

  if (t<mmin[r])
  {
    // reverse interpolate
    c1 = colorRows[r][last].color;
    c2 = colorRows[r][0].color;
    t1 = colorRows[r][last].time;
    t2 = colorRows[r][0].time + 2880;
    t += 2880;
  }
  else
  {
    for (size_t i = last; true; i--)
    { //! \todo iterator this.
      if (colorRows[r][i].time <= t)
      {
        c1 = colorRows[r][i].color;
        t1 = colorRows[r][i].time;

        if (i == last)
        {
          c2 = colorRows[r][0].color;
          t2 = colorRows[r][0].time + 2880;
        }
        else
        {
          c2 = colorRows[r][i + 1].color;
          t2 = colorRows[r][i + 1].time;
        }
        break;
      }
    }
  }

  float tt = static_cast<float>(t - t1) / static_cast<float>(t2 - t1);
  return c1*(1.0f - tt) + c2*tt;
}

const float rad = 400.0f;

//...............................top....med....medh........horiz..........bottom
const math::degrees angles[] = { math::degrees (90.0f)
                               , math::degrees (30.0f)
                               , math::degrees (15.0f)
                               , math::degrees (5.0f)
                               , math::degrees (0.0f)
                               , math::degrees (-30.0f)
                               , math::degrees (-90.0f)
                               };
const int skycolors[] = { 2, 3, 4, 5, 6, 7, 7 };
const int cnum = 7;
const int hseg = 32;


Skies::Skies(unsigned int mapid)
  : stars (ModelInstance("Environments\\Stars\\Stars.mdx"))
{
  for (DBCFile::Iterator i = gLightDB.begin(); i != gLightDB.end(); ++i)
  {
    if (mapid == i->getUInt(LightDB::Map))
    {
      Sky s(i);
      skies.push_back(s);
      numSkies++;
    }
  }

  if (numSkies == 0)
  {
    for (DBCFile::Iterator i = gLightDB.begin(); i != gLightDB.end(); ++i)
    {
      if (0 == i->getUInt(LightDB::Map))
      {
        Sky s(i);
        skies.push_back(s);
        numSkies++;
        break;
      }
    }
  }

  // sort skies from smallest to largest; global last.
  // smaller skies will have precedence when calculating weights to achieve smooth transitions etc.
  std::sort(skies.begin(), skies.end());
}

void Skies::findSkyWeights(math::vector_3d pos)
{
  int maxsky = skies.size() - 1;
  skies[maxsky].weight = 1.0f;
  cs = maxsky;
  for (int i = maxsky - 1; i >= 0; i--) {
    Sky &s = skies[i];
    float dist = (pos - s.pos).length();
    if (dist < s.r1) {
      // we're in a sky, zero out the rest
      s.weight = 1.0f;
      cs = i;
      for (size_t j = i + 1; j<skies.size(); j++) skies[j].weight = 0.0f;
    }
    else if (dist < s.r2) {
      // we're in an outer area, scale down the other weights
      float r = (dist - s.r1) / (s.r2 - s.r1);
      s.weight = 1.0f - r;
      for (size_t j = i + 1; j<skies.size(); j++) skies[j].weight *= r;
    }
    else s.weight = 0.0f;
  }
  // weights are all normalized at this point :D
}

void Skies::update_sky_colors(math::vector_3d pos, int time)
{
  if (numSkies == 0 || (_last_time == time && _last_pos == pos))
  {
    return;
  }  

  findSkyWeights(pos);

  for (int i = 0; i < NUM_SkyColorNames; ++i)
  {
    color_set[i] = math::vector_3d(1, 1, 1);
  }

  _river_shallow_alpha = 0.f;
  _river_deep_alpha = 0.f;
  _ocean_shallow_alpha = 0.f;
  _ocean_deep_alpha = 0.f;

  // interpolation
  for (size_t j = 0; j<skies.size(); j++) 
  {
    Sky const& sky = skies[j];

    if (sky.weight>0)
    {
      // now calculate the color rows
      for (int i = 0; i<NUM_SkyColorNames; ++i) 
      {
        if ((sky.colorFor(i, time).x>1.0f) || (sky.colorFor(i, time).y>1.0f) || (sky.colorFor(i, time).z>1.0f))
        {
          LogDebug << "Sky " << j << " " << i << " is out of bounds!" << std::endl;
          continue;
        }
        color_set[i] += sky.colorFor(i, time) * sky.weight;
      }

      _river_shallow_alpha += sky.weight * sky.river_shallow_alpha();
      _river_deep_alpha += sky.weight * sky.river_deep_alpha();
      _ocean_shallow_alpha += sky.weight * sky.ocean_shallow_alpha();
      _ocean_deep_alpha += sky.weight * sky.ocean_deep_alpha();
    }
  }
  for (int i = 0; i<NUM_SkyColorNames; ++i)
  {
    color_set[i] -= math::vector_3d(1.f, 1.f, 1.f);
  }

  _last_pos = pos;
  _last_time = time;

  _need_color_buffer_update = true;  
}

bool Skies::draw( math::matrix_4x4 const& model_view
                , math::matrix_4x4 const& projection
                , math::vector_3d const& camera_pos
                , opengl::scoped::use_program& m2_shader
                , math::frustum const& frustum
                , const float& cull_distance
                , int animtime
                , bool draw_particles
                , OutdoorLightStats const& light_stats
                )
{
  if (numSkies == 0)
  {
    return false;
  }

  if (!_uploaded)
  {
    upload();
  }

  if (_need_color_buffer_update)
  {
    update_color_buffer();
  }

  {
    opengl::scoped::use_program shader {*_program.get()};

    if(_need_vao_update)
    {
      update_vao(shader);
    }

    {
      opengl::scoped::vao_binder const _ (_vao);

      shader.uniform("model_view_projection", model_view*projection);
      shader.uniform("camera_pos", camera_pos);

      gl.drawElements(GL_TRIANGLES, _indices_count, GL_UNSIGNED_SHORT, opengl::index_buffer_is_already_bound{});
    }
  }

  for (Sky& sky : skies)
  {
    if (sky.weight > 0.f && sky.skybox)
    {
      auto& model = sky.skybox.get();
      model.model->trans = sky.weight;
      model.pos = camera_pos;
      model.scale = 0.1f;
      model.recalcExtents();

      model.model->draw(model_view, model, m2_shader, frustum, cull_distance, camera_pos, animtime, draw_particles, false, display_mode::in_3D);
    }
  }
  // if it's night, draw the stars
  if (light_stats.nightIntensity > 0)
  {
    stars.model->trans = light_stats.nightIntensity;
    stars.pos = camera_pos;
    stars.scale = 0.1f;
    stars.recalcExtents();

    stars.model->draw(model_view, stars, m2_shader, frustum, cull_distance, camera_pos, animtime, draw_particles, false, display_mode::in_3D);
  }

  return true;
}

void Skies::upload()
{
  _program.reset(new opengl::program(
    {
        {GL_VERTEX_SHADER, R"code(
#version 330 core

uniform mat4 model_view_projection;
uniform vec3 camera_pos;

in vec3 position;
in vec3 color;

out vec3 f_color;

void main()
{
  vec4 pos = vec4(position+camera_pos, 1.f);
  gl_Position = model_view_projection * pos;
  f_color = color;
}
)code" }
        , {GL_FRAGMENT_SHADER, R"code(
#version 330 core

in vec3 f_color;

out vec4 out_color;

void main()
{
  out_color = vec4(f_color, 1.);
}
)code" }
    }
  ));

  _vertex_array.upload();
  _buffers.upload();

  std::vector<math::vector_3d> vertices;
  std::vector<std::uint16_t> indices;

  math::vector_3d basepos1[cnum], basepos2[cnum];

  for (int h = 0; h < hseg; h++)
  {
    for (int i = 0; i < cnum; ++i)
    {
      basepos1[i] = basepos2[i] = math::vector_3d(math::cos(angles[i])*rad, math::sin(angles[i])*rad, 0);
      math::rotate(0, 0, &basepos1[i].x, &basepos1[i].z, math::radians(math::constants::pi*2.0f / hseg * h));
      math::rotate(0, 0, &basepos2[i].x, &basepos2[i].z, math::radians(math::constants::pi*2.0f / hseg * (h + 1)));
    }

    for (int v = 0; v < cnum - 1; v++)
    {
      int start = vertices.size();

      vertices.push_back(basepos2[v]);
      vertices.push_back(basepos1[v]);
      vertices.push_back(basepos1[v + 1]);
      vertices.push_back(basepos2[v + 1]);

      indices.push_back(start+0);
      indices.push_back(start+1);
      indices.push_back(start+2);

      indices.push_back(start+2);
      indices.push_back(start+3);
      indices.push_back(start+0);
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>(_vertices_vbo, vertices, GL_STATIC_DRAW);
  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_indices_vbo, indices, GL_STATIC_DRAW);

  _indices_count = indices.size();

  _uploaded = true;
  _need_vao_update = true;
}

void Skies::update_vao(opengl::scoped::use_program& shader)
{
  opengl::scoped::index_buffer_manual_binder indices_binder (_indices_vbo);

  {
    opengl::scoped::vao_binder const _ (_vao);

    shader.attrib(_, "position", _vertices_vbo, 3, GL_FLOAT, GL_FALSE, 0, 0);
    shader.attrib(_, "color", _colors_vbo, 3, GL_FLOAT, GL_FALSE, 0, 0);

    indices_binder.bind();
  }

  _need_vao_update = false;
}

void Skies::update_color_buffer()
{
  std::vector<math::vector_3d> colors;

  for (int h = 0; h < hseg; h++)
  {
    for (int v = 0; v < cnum - 1; v++)
    {
      colors.push_back(color_set[skycolors[v]]);
      colors.push_back(color_set[skycolors[v]]);
      colors.push_back(color_set[skycolors[v + 1]]);
      colors.push_back(color_set[skycolors[v + 1]]);
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>(_colors_vbo, colors, GL_STATIC_DRAW);

  _need_vao_update = true;
}

//! \todo  figure out what dnc.db is _really_ used for

void OutdoorLightStats::init(MPQFile* f)
{
  float h, m;

  f->seekRelative(4);
  f->read(&h, 4);
  f->seekRelative(4);
  f->read(&m, 4);
  f->seekRelative(4);
  f->read(&dayIntensity, 4);

  f->seekRelative(4);
  f->read(&dayColor.x, 4);
  f->seekRelative(4);
  f->read(&dayColor.y, 4);
  f->seekRelative(4);
  f->read(&dayColor.z, 4);

  f->seekRelative(4);
  f->read(&dayDir.x, 4);
  f->seekRelative(4);
  f->read(&dayDir.y, 4);
  f->seekRelative(4);
  f->read(&dayDir.z, 4);

  f->seekRelative(4);
  f->read(&nightIntensity, 4);

  f->seekRelative(4);
  f->read(&nightColor.x, 4);
  f->seekRelative(4);
  f->read(&nightColor.y, 4);
  f->seekRelative(4);
  f->read(&nightColor.z, 4);

  f->seekRelative(4);
  f->read(&nightDir.x, 4);
  f->seekRelative(4);
  f->read(&nightDir.y, 4);
  f->seekRelative(4);
  f->read(&nightDir.z, 4);

  f->seekRelative(4);
  f->read(&ambientIntensity, 4);

  f->seekRelative(4);
  f->read(&ambientColor.x, 4);
  f->seekRelative(4);
  f->read(&ambientColor.y, 4);
  f->seekRelative(4);
  f->read(&ambientColor.z, 4);

  f->seekRelative(4);
  f->read(&fogDepth, 4);
  f->seekRelative(4);
  f->read(&fogIntensity, 4);

  f->seekRelative(4);
  f->read(&fogColor.x, 4);
  f->seekRelative(4);
  f->read(&fogColor.y, 4);
  f->seekRelative(4);
  f->read(&fogColor.z, 4);

  time = (int)((h * 60 + m) * 2);
}

void OutdoorLightStats::interpolate(OutdoorLightStats *a, OutdoorLightStats *b, float r)
{
  float ir = 1.0f - r;
  time = 0; // unused

  dayIntensity = a->dayIntensity * ir + b->dayIntensity * r;
  nightIntensity = a->nightIntensity * ir + b->nightIntensity * r;
  ambientIntensity = a->ambientIntensity * ir + b->ambientIntensity * r;
  fogIntensity = a->fogIntensity * ir + b->fogIntensity * r;
  fogDepth = a->fogDepth * ir + b->fogDepth * r;
  dayColor = a->dayColor * ir + b->dayColor * r;
  nightColor = a->nightColor * ir + b->nightColor * r;
  ambientColor = a->ambientColor * ir + b->ambientColor * r;
  fogColor = a->fogColor * ir + b->fogColor * r;
  dayDir = a->dayDir * ir + b->dayDir * r;
  nightDir = a->nightDir * ir + b->nightDir * r;
}

OutdoorLighting::OutdoorLighting(const std::string& fname)
{
  MPQFile f(fname);
  unsigned int n, d;

  f.seekRelative(4);
  f.read(&n, 4); // it's the same thing twice? :|

  f.seekRelative(4);
  f.read(&d, 4); // d is now the final offset
  f.seek(8 + n * 8);

  while (f.getPos() < d) 
  {
    OutdoorLightStats ols;
    ols.init(&f);

    lightStats.push_back(ols);
  }

  f.close();
}

OutdoorLightStats OutdoorLighting::getLightStats(int time)
{
  // ASSUME: only 24 light info records, one for each whole hour
  //! \todo  generalize this if the data file changes in the future

  OutdoorLightStats out;

  OutdoorLightStats *a, *b;
  int ta = time / 120;
  int tb = (ta + 1) % 24;
  float r = (time - (ta * 120)) / 120.0f;

  a = &lightStats[ta];
  b = &lightStats[tb];

  out.interpolate(a, b, r);

  return out;
}
