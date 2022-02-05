// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <math/bounding_box.hpp>
#include <noggit/AsyncLoader.h>
#include <noggit/Log.h>
#include <noggit/Model.h>
#include <noggit/ModelInstance.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <opengl/scoped.hpp>
#include <opengl/shader.hpp>

#include <algorithm>
#include <cassert>
#include <map>
#include <sstream>
#include <string>

Model::Model(const std::string& filename_)
  : AsyncObject(filename_)
  , _finished_upload(false)
{
  memset(&header, 0, sizeof(ModelHeader));
}

void Model::finishLoading()
{
  MPQFile f(filename);

  if (f.isEof())
  {
    LogError << "Error loading file \"" << filename << "\". Aborting to load model." << std::endl;
    finished = true;
    return;
  }

  memcpy(&header, f.getBuffer(), sizeof(ModelHeader));

  // blend mode override
  if (header.Flags & 8)
  {
    // go to the end of the header (where the blend override data is)    
    uint32_t const* blend_override_info = reinterpret_cast<uint32_t const*>(f.getBuffer() + sizeof(ModelHeader));
    uint32_t n_blend_override = *blend_override_info++;
    uint32_t ofs_blend_override = *blend_override_info;

    blend_override = M2Array<uint16_t>(f, ofs_blend_override, n_blend_override);
  }


  _vertex_box_points = math::box_points ( misc::transform_model_box_coords(header.bounding_box_min)
                                        , misc::transform_model_box_coords(header.bounding_box_max)
                                        );

  animated = isAnimated(f);  // isAnimated will set animGeometry and animTextures

  trans = 1.0f;
  _current_anim_seq = 0;

  rad = header.bounding_box_radius;

  if (header.nGlobalSequences)
  {
    _global_sequences = M2Array<int>(f, header.ofsGlobalSequences, header.nGlobalSequences);
  }

  //! \todo  This takes a biiiiiit long. Have a look at this.
  initCommon(f);

  if (animated)
  {
    initAnimated(f);
  }

  f.close();

  finished = true;
  _state_changed.notify_all();
}


bool Model::isAnimated(const MPQFile& f)
{
  // see if we have any animated bones
  ModelBoneDef const* bo = reinterpret_cast<ModelBoneDef const*>(f.getBuffer() + header.ofsBones);

  animGeometry = false;
  animBones = false;
  _per_instance_animation = false;

  ModelVertex const* verts = reinterpret_cast<ModelVertex const*>(f.getBuffer() + header.ofsVertices);
  for (size_t i = 0; i<header.nVertices && !animGeometry; ++i) 
  {
    for (size_t b = 0; b<4; b++) 
    {
      if (verts[i].weights[b]>0) 
      {
        ModelBoneDef const& bb = bo[verts[i].bones[b]];
        bool billboard = (bb.flags & (0x78)); // billboard | billboard_lock_[xyz]

        if ((bb.flags & 0x200) || billboard) 
        {
          if (billboard) 
          {
            // if we have billboarding, the model will need per-instance animation
            _per_instance_animation = true;
          }
          animGeometry = true;
          break;
        }
      }
    }
  }

  if (animGeometry || header.nParticleEmitters || header.nRibbonEmitters || header.nLights)
  {
    animBones = true;
  }
  else
  {
    for (size_t i = 0; i<header.nBones; ++i)
    {
      ModelBoneDef const& bb = bo[i];
      if (bb.translation.type || bb.rotation.type || bb.scaling.type)
      {
        animBones = true;
        break;
      }
    }
  }

  animTextures = header.nTexAnims > 0;

  // animated colors
  if (header.nColors)
  {
    ModelColorDef const* cols = reinterpret_cast<ModelColorDef const*>(f.getBuffer() + header.ofsColors);
    for (size_t i = 0; i<header.nColors; ++i)
    {
      if (cols[i].color.type != 0 || cols[i].opacity.type != 0)
      {
        return true;
      }
    }
  }

  // animated opacity
  if (header.nTransparency)
  {
    ModelTransDef const* trs = reinterpret_cast<ModelTransDef const*>(f.getBuffer() + header.ofsTransparency);
    for (size_t i = 0; i<header.nTransparency; ++i)
    {
      if (trs[i].trans.type != 0)
      {
        return true;
      }
    }
  }

  // guess not...
  return animGeometry || animTextures || animBones;
}


math::vector_3d fixCoordSystem(math::vector_3d v)
{
  return math::vector_3d(v.x, v.z, -v.y);
}

namespace
{
  math::vector_3d fixCoordSystem2(math::vector_3d v)
  {
    return math::vector_3d(v.x, v.z, v.y);
  }

  math::quaternion fixCoordSystemQuat(math::quaternion v)
  {
    return math::quaternion(-v.x, -v.z, v.y, v.w);
  }
}


void Model::initCommon(const MPQFile& f)
{
  // vertices, normals, texcoords
  _vertices = M2Array<ModelVertex>(f, header.ofsVertices, header.nVertices);

  for (auto& v : _vertices)
  {
    v.position = fixCoordSystem(v.position);
    v.normal = fixCoordSystem(v.normal);
  }
 
  if (!animGeometry)
  {
    _current_vertices.swap(_vertices);
  }

  // textures
  ModelTextureDef const* texdef = reinterpret_cast<ModelTextureDef const*>(f.getBuffer() + header.ofsTextures);
  _textureFilenames.resize(header.nTextures);
  _specialTextures.resize(header.nTextures);

  for (size_t i = 0; i < header.nTextures; ++i)
  {
    if (texdef[i].type == 0)
    {
      if (texdef[i].nameLen == 0)
      {
        LogDebug << "Texture " << i << " has a lenght of 0 for '" << filename << std::endl;
        continue;
      }

      _specialTextures[i] = -1;
      const char* blp_ptr = f.getBuffer() + texdef[i].nameOfs;
      // some tools export the size without accounting for the \0
      bool invalid_size = *(blp_ptr + texdef[i].nameLen-1) != '\0';
      _textureFilenames[i] = std::string(blp_ptr, texdef[i].nameLen - (invalid_size ? 0 : 1));
    }
    else
    {
#ifndef NO_REPLACIBLE_TEXTURES_HACK
      _specialTextures[i] = -1;
      _textureFilenames[i] = "tileset/generic/black.blp";
#else
      //! \note special texture - only on characters and such... Noggit should not even render these.
      //! \todo Check if this is actually correct. Or just remove it.

      _specialTextures[i] = texdef[i].type;

      if (texdef[i].type == 3)
      {
        _textureFilenames[i] = "Item\\ObjectComponents\\Weapon\\ArmorReflect4.BLP";
        // a fix for weapons with type-3 textures.
        _replaceTextures.emplace (texdef[i].type, _textureFilenames[i]);
      }
#endif
    }
  }

  // init colors
  if (header.nColors) {
    ModelColorDef const* colorDefs = reinterpret_cast<ModelColorDef const*>(f.getBuffer() + header.ofsColors);
    for (size_t i = 0; i < header.nColors; ++i)
    {
      _colors.emplace_back (f, colorDefs[i], _global_sequences.data());
    }
  }

  // init transparency
  _transparency_lookup = M2Array<int16_t>(f, header.ofsTransparencyLookup, header.nTransparencyLookup);

  if (header.nTransparency) {
    ModelTransDef const* trDefs = reinterpret_cast<ModelTransDef const*>(f.getBuffer() + header.ofsTransparency);
    for (size_t i = 0; i < header.nTransparency; ++i)
    {
      _transparency.emplace_back (f, trDefs[i], _global_sequences.data());
    }
  }


  // just use the first LOD/view

  if (header.nViews > 0) {
    // indices - allocate space, too
    std::string lodname = filename.substr(0, filename.length() - 3);
    lodname.append("00.skin");
    MPQFile g(lodname.c_str());
    if (g.isEof()) {
      LogError << "loading skinfile " << lodname << std::endl;
      g.close();
      return;
    }

    ModelView const* view = reinterpret_cast<ModelView const*>(g.getBuffer());
    uint16_t const* indexLookup = reinterpret_cast<uint16_t const*>(g.getBuffer() + view->ofs_index);
    uint16_t const* triangles = reinterpret_cast<uint16_t const*>(g.getBuffer() + view->ofs_triangle);

    _indices.resize (view->n_triangle);

    for (size_t i (0); i < _indices.size(); ++i) {
      _indices[i] = indexLookup[triangles[i]];
    }

    // render ops
    ModelGeoset const* model_geosets = reinterpret_cast<ModelGeoset const*>(g.getBuffer() + view->ofs_submesh);
    ModelTexUnit const* texture_unit = reinterpret_cast<ModelTexUnit const*>(g.getBuffer() + view->ofs_texture_unit);
    
    _texture_lookup = M2Array<uint16_t>(f, header.ofsTexLookup, header.nTexLookup);
    _texture_animation_lookups = M2Array<int16_t>(f, header.ofsTexAnimLookup, header.nTexAnimLookup);
    _texture_unit_lookup = M2Array<int16_t>(f, header.ofsTexUnitLookup, header.nTexUnitLookup);

    showGeosets.resize (view->n_submesh);
    for (size_t i = 0; i<view->n_submesh; ++i) 
    {
      showGeosets[i] = true;
    }

    _render_flags = M2Array<ModelRenderFlags>(f, header.ofsRenderFlags, header.nRenderFlags);

    for (size_t j = 0; j<view->n_texture_unit; j++) 
    {
      size_t geoset = texture_unit[j].submesh;

      ModelRenderPass pass(texture_unit[j], this);
      pass.ordering_thingy = model_geosets[geoset].BoundingBox[0].x;

      pass.index_start = model_geosets[geoset].istart;
      pass.index_count = model_geosets[geoset].icount;
      pass.vertex_start = model_geosets[geoset].vstart;
      pass.vertex_end = pass.vertex_start + model_geosets[geoset].vcount;

      _render_passes.push_back(pass);
    }

    g.close();

    fix_shader_id_blend_override();
    fix_shader_id_layer();
    compute_pixel_shader_ids();

    for (auto& pass : _render_passes)
    {
      pass.init_uv_types(this);
    }
    
    // transparent parts come later
    std::sort(_render_passes.begin(), _render_passes.end());

    // add fake geometry for selection
    if (_render_passes.empty())
    {
      _fake_geometry.emplace(this);
    }
  }  
}

void Model::fix_shader_id_blend_override()
{
  for (auto& pass : _render_passes)
  {
    if (pass.shader_id & 0x8000)
    {
      continue; 
    }

    int shader = 0;
    bool blend_mode_override = (header.Flags & 8);

    // fuckporting check
    if (pass.texture_coord_combo_index + pass.texture_count - 1 >= _texture_unit_lookup.size())
    {
      LogDebug << "wrong texture coord combo index on fuckported model: " << filename << std::endl;
      // use default stuff
      pass.shader_id = 0;
      pass.texture_count = 1;

      continue;
    }

    if (!blend_mode_override)
    {
      uint16_t texture_unit_lookup = _texture_unit_lookup[pass.texture_coord_combo_index];

      if (_render_flags[pass.renderflag_index].blend)
      {
        shader = 1;

        if (texture_unit_lookup == 0xFFFF)
        {
          shader |= 0x8;
        }
      }

      shader <<= 4;

      if (texture_unit_lookup == 1)
      {
        shader |= 0x4000;
      }
    }
    else
    {
      uint16_t runtime_shader_val[2] = { 0, 0 };

      for (int i = 0; i < pass.texture_count; ++i)
      {
        uint16_t override_blend = blend_override[pass.shader_id + i];
        uint16_t texture_unit_lookup = _texture_unit_lookup[pass.texture_coord_combo_index + i];

        if (i == 0 && _render_flags[pass.renderflag_index].blend == 0)
        {
          override_blend = 0;
        }

        runtime_shader_val[i] = override_blend;

        if (texture_unit_lookup == 0xFFFF)
        {
          runtime_shader_val[i] |= 0x8;
        }

        if (texture_unit_lookup == 1 && i + 1 == pass.texture_count)
        {
          shader |= 0x4000;
        }
      }

      shader |= (runtime_shader_val[1] & 0xFFFF) | ((runtime_shader_val[0] << 4) & 0xFFFF);
    }

    pass.shader_id = shader;
  }
}

void Model::fix_shader_id_layer()
{
  int non_layered_count = 0;

  for (auto const& pass : _render_passes)
  {
    if (pass.material_layer <= 0)
    {
      non_layered_count++;
    }
  }

  if (non_layered_count < _render_passes.size())
  {
    std::vector<ModelRenderPass> passes;

    ModelRenderPass* first_pass = nullptr;
    bool need_reducing = false;
    uint16_t previous_render_flag = -1, some_flags = 0;

    for (auto& pass : _render_passes)
    {
      if (pass.renderflag_index == previous_render_flag)
      {
        need_reducing = true;
        continue;
      }

      previous_render_flag = pass.renderflag_index;

      uint8_t lower_bits = pass.shader_id & 0x7;

      if (pass.material_layer == 0)
      {
        if (pass.texture_count >= 1 && _render_flags[pass.renderflag_index].blend == 0)
        {
          pass.shader_id &= 0xFF8F;
        }

        first_pass = &pass;
      }

      bool xor_unlit = ((_render_flags[pass.renderflag_index].flags.unlit ^ _render_flags[first_pass->renderflag_index].flags.unlit) & 1) == 0;

      if ((some_flags & 0xFF) == 1)
      {
        if ((_render_flags[pass.renderflag_index].blend == 1 || _render_flags[pass.renderflag_index].blend == 2)
          && pass.texture_count == 1
          && xor_unlit
          && pass.texture_combo_index == first_pass->texture_combo_index
          )
        {
          if (_transparency_lookup[pass.transparency_combo_index] == _transparency_lookup[first_pass->transparency_combo_index])
          {
            pass.shader_id = 0x8000;
            first_pass->shader_id = 0x8001;

            some_flags = (some_flags & 0xFF00) | 3;

            // current pass removed (not needed)
            continue;
          }
        }

        some_flags = (some_flags & 0xFF00);
      }

      int16_t texture_unit_lookup = _texture_unit_lookup[pass.texture_coord_combo_index];

      if ((some_flags & 0xFF) < 2)
      {
        if ((_render_flags[pass.renderflag_index].blend == 0) && (pass.texture_count == 2) && ((lower_bits == 4) || (lower_bits == 6)))
        {
          if (texture_unit_lookup == 0 && (_texture_unit_lookup[pass.texture_coord_combo_index + 1] == -1))
          {
            some_flags = (some_flags & 0xFF00) | 1;
          }
        }
      }

      if ((some_flags >> 8) != 0)
      {
        if ((some_flags >> 8) == 1)
        {
          if (((_render_flags[pass.renderflag_index].blend != 4) && (_render_flags[pass.renderflag_index].blend != 6)) || (pass.texture_count != 1) || (texture_unit_lookup >= 0))
          {
            some_flags &= 0xFF00;
          }
          else  if (_transparency_lookup[pass.transparency_combo_index] == _transparency_lookup[first_pass->transparency_combo_index])
          {
            pass.shader_id = 0x8000;
            first_pass->shader_id = _render_flags[pass.renderflag_index].blend != 4 ? 0xE : 0x8002;

            some_flags = (some_flags & 0xFF) | (2 << 8);

            first_pass->texture_count = 2;

            first_pass->textures[1] = pass.texture_combo_index;
            first_pass->uv_animations[1] = pass.animation_combo_index;

            // current pass removed (merged with the previous one)
            continue;
          }
        }
        else
        {
          if ((some_flags >> 8) != 2)
          {
            continue;
          }

          if ( ((_render_flags[pass.renderflag_index].blend != 2) && (_render_flags[pass.renderflag_index].blend != 1))
            || (pass.texture_count != 1)
            || xor_unlit
            || ((pass.texture_combo_index & 0xff) != (first_pass->texture_combo_index & 0xff))
            )
          {
            some_flags &= 0xFF00;
          }
          else  if (_transparency_lookup[pass.transparency_combo_index] == _transparency_lookup[first_pass->transparency_combo_index])
          {
            // current pass ignored/removed
            pass.shader_id = 0x8000;
            first_pass->shader_id = ((first_pass->shader_id == 0x8002 ? 2 : 0) - 0x7FFF) & 0xFFFF;
            some_flags = (some_flags & 0xFF) | (3 << 8);
            continue;
          }
        }
        some_flags = (some_flags & 0xFF);
      }

      if ((_render_flags[pass.renderflag_index].blend == 0) && (pass.texture_count == 1) && (texture_unit_lookup == 0))
      {
        some_flags = (some_flags & 0xFF) | (1 << 8);
      }

      // setup texture and anim lookup indices
      pass.textures[0] = pass.texture_combo_index;
      pass.textures[1] = pass.texture_count > 1 ? pass.texture_combo_index + 1 : 0;
      pass.uv_animations[0] = pass.animation_combo_index;
      pass.uv_animations[1] = pass.texture_count > 1 ? pass.animation_combo_index + 1 : 0;

      passes.push_back(pass);
    }

    if (need_reducing)
    {
      previous_render_flag = -1;
      for (int i = 0; i < passes.size(); ++i)
      {
        auto& pass = _render_passes[i];
        uint16_t renderflag_index = pass.renderflag_index;

        if (renderflag_index == previous_render_flag)
        {
          pass.shader_id = _render_passes[i - 1].shader_id;
          pass.texture_count = _render_passes[i - 1].texture_count;
          pass.texture_combo_index = _render_passes[i - 1].texture_combo_index;
          pass.texture_coord_combo_index = _render_passes[i - 1].texture_coord_combo_index;
        }
        else
        {
          previous_render_flag = renderflag_index;
        }
      }
    }

    _render_passes = passes;
  }
  // no layering, just setting some infos
  else
  {
    for (auto& pass : _render_passes)
    {
      pass.textures[0] = pass.texture_combo_index;
      pass.textures[1] = pass.texture_count > 1 ? pass.texture_combo_index + 1 : 0;
      pass.uv_animations[0] = pass.animation_combo_index;
      pass.uv_animations[1] = pass.texture_count > 1 ? pass.animation_combo_index + 1 : 0;
    }
  }
}


ModelRenderPass::ModelRenderPass(ModelTexUnit const& tex_unit, Model* m)
  : ModelTexUnit(tex_unit)
  , blend_mode(m->_render_flags[renderflag_index].blend)
{
}

bool ModelRenderPass::prepare_draw(opengl::scoped::use_program& m2_shader, Model *m)
{
  if (!m->showGeosets[submesh] || !pixel_shader)
  {
    return false;
  }

  // COLOUR
  // Get the colour and transparency and check that we should even render
  math::vector_4d mesh_color = math::vector_4d(1.0f, 1.0f, 1.0f, m->trans); // ??
  math::vector_4d emissive_color = math::vector_4d(0.0f, 0.0f, 0.0f, 0.0f);

  auto const& renderflag(m->_render_flags[renderflag_index]);

  // emissive colors
  if (color_index != -1 && m->_colors[color_index].color.uses(0))
  {
    ::math::vector_3d c (m->_colors[color_index].color.getValue (0, m->_anim_time, m->_global_animtime));
    if (m->_colors[color_index].opacity.uses (m->_current_anim_seq))
    {
      mesh_color.w = m->_colors[color_index].opacity.getValue (m->_current_anim_seq, m->_anim_time, m->_global_animtime);
    }

    if (renderflag.flags.unlit)
    {
      mesh_color.x = c.x; mesh_color.y = c.y; mesh_color.z = c.z;
    }
    else
    {
      mesh_color.x = mesh_color.y = mesh_color.z = 0;
    }

    emissive_color = math::vector_4d(c, mesh_color.w);
  }

  // opacity
  if (transparency_combo_index != 0xFFFF)
  {
    auto& transparency (m->_transparency[m->_transparency_lookup[transparency_combo_index]].trans);
    if (transparency.uses (0))
    {
      mesh_color.w = mesh_color.w * transparency.getValue(0, m->_anim_time, m->_global_animtime);
    }
  }

  // exit and return false before affecting the opengl render state
  if (!((mesh_color.w > 0) && (color_index == -1 || emissive_color.w > 0)))
  {
    return false;
  }
  
  switch (static_cast<M2Blend>(renderflag.blend))
  {
  default:
  case M2Blend::Opaque:
    gl.disable(GL_BLEND);
    m2_shader.uniform("alpha_test", -1.f);    
    m2_shader.uniform("fog_mode", 1);
    break;
  case M2Blend::Alpha_Key:
    gl.disable(GL_BLEND);
    m2_shader.uniform("alpha_test", (224.f / 255.f) * mesh_color.w);
    m2_shader.uniform("fog_mode", 1);
    break;
  case M2Blend::Alpha:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m2_shader.uniform("alpha_test", (1.f / 255.f) * mesh_color.w);
    m2_shader.uniform("fog_mode", 1);
    break;
  case M2Blend::No_Add_Alpha:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_ONE, GL_ONE);
    m2_shader.uniform("alpha_test", (1.f / 255.f) * mesh_color.w);
    m2_shader.uniform("fog_mode", 2); // Warning: wiki is unsure on that
    break;
  case M2Blend::Add:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE);
    m2_shader.uniform("alpha_test", (1.f / 255.f) * mesh_color.w);
    m2_shader.uniform("fog_mode", 2);
    break;
  case M2Blend::Mod:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_DST_COLOR, GL_ZERO);
    m2_shader.uniform("alpha_test", (1.f / 255.f) * mesh_color.w);
    m2_shader.uniform("fog_mode", 3);
    break;
  case M2Blend::Mod2x:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_DST_COLOR, GL_SRC_COLOR);
    m2_shader.uniform("alpha_test", (1.f / 255.f) * mesh_color.w);
    m2_shader.uniform("fog_mode", 4);
    break;
  }

  if (renderflag.flags.two_sided)
  {
    gl.disable(GL_CULL_FACE);
  }
  else
  {
    gl.enable(GL_CULL_FACE);
  }

  if (renderflag.flags.z_buffered)
  {
    gl.depthMask(GL_FALSE);
  }
  else
  {
    gl.depthMask(GL_TRUE);
  }

  m2_shader.uniform("unfogged", (int)renderflag.flags.unfogged);
  m2_shader.uniform("unlit", (int)renderflag.flags.unlit);

  if (texture_count > 1)
  {
    bind_texture(1, m);
  }

  bind_texture(0, m);

  GLint tu1 = static_cast<GLint>(tu_lookups[0]), tu2 = static_cast<GLint>(tu_lookups[1]);

  m2_shader.uniform("tex_unit_lookup_1", tu1);
  m2_shader.uniform("tex_unit_lookup_2", tu2);

  int16_t tex_anim_lookup = m->_texture_animation_lookups[uv_animations[0]];
  math::matrix_4x4 unit(math::matrix_4x4::unit);

  if (tex_anim_lookup != -1)
  {
    m2_shader.uniform("tex_matrix_1", m->_texture_animations[tex_anim_lookup].mat);
    if (texture_count > 1)
    {
      tex_anim_lookup = m->_texture_animation_lookups[uv_animations[1]];
      if (tex_anim_lookup != -1)
      {
        m2_shader.uniform("tex_matrix_2", m->_texture_animations[tex_anim_lookup].mat);
    }
    else
    {
        m2_shader.uniform("tex_matrix_2", unit);
    }
  }
  }
  else
  {
    m2_shader.uniform("tex_matrix_1", unit);
    m2_shader.uniform("tex_matrix_2", unit);
  }
  

  m2_shader.uniform("pixel_shader", static_cast<GLint>(pixel_shader.get()));
  m2_shader.uniform("mesh_color", mesh_color);

  return true;
}

void ModelRenderPass::after_draw()
{
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ModelRenderPass::bind_texture(size_t index, Model* m)
{
  opengl::texture::set_active_texture(index);

  uint16_t tex = m->_texture_lookup[textures[index]];
  
  if (m->_specialTextures[tex] == -1)
  {
    m->_textures[tex]->bind();
  }
  else
  {
    m->_replaceTextures.at (m->_specialTextures[tex])->bind();
  }    
}

void ModelRenderPass::init_uv_types(Model* m)
{
  tu_lookups[0] = texture_unit_lookup::none;
  tu_lookups[1] = texture_unit_lookup::none;

  if (m->_texture_unit_lookup.size() < texture_coord_combo_index + texture_count)
  {
    throw std::out_of_range("model: texture_coord_combo_index out of range " + m->filename);
  }

  for (int i = 0; i < texture_count; ++i)
  {
    switch (m->_texture_unit_lookup[texture_coord_combo_index + i])
    {
      case (int16_t)(-1): tu_lookups[i] = texture_unit_lookup::environment; break;
      case 0: tu_lookups[i] = texture_unit_lookup::t1; break;
      case 1: tu_lookups[i] = texture_unit_lookup::t2; break;
    }
  }
}

FakeGeometry::FakeGeometry(Model* m)
{
  math::vector_3d min = m->header.bounding_box_min, max = m->header.bounding_box_max;

  vertices.emplace_back(min.x, max.y, min.z);
  vertices.emplace_back(min.x, max.y, max.z);
  vertices.emplace_back(max.x, max.y, max.z);
  vertices.emplace_back(max.x, max.y, min.z);

  vertices.emplace_back(min.x, min.y, min.z);
  vertices.emplace_back(min.x, min.y, max.z);
  vertices.emplace_back(max.x, min.y, max.z);
  vertices.emplace_back(max.x, min.y, min.z);

  indices =
  {
    0,1,2,  2,3,0,
    0,4,5,  5,1,0,
    0,3,7,  7,4,0,
    1,5,6,  6,2,1,
    2,6,7,  7,3,2,
    5,6,7,  7,4,5
  }; 
}

namespace
{

// https://wowdev.wiki/M2/.skin/WotLK_shader_selection
boost::optional<ModelPixelShader> GetPixelShader(uint16_t texture_count, uint16_t shader_id)
{
  uint16_t texture1_fragment_mode = (shader_id >> 4) & 7;
  uint16_t texture2_fragment_mode = shader_id & 7;
  // uint16_t texture1_env_map = (shader_id >> 4) & 8;
  // uint16_t texture2_env_map = shader_id & 8;

  boost::optional<ModelPixelShader> pixel_shader;

  if (texture_count == 1)
  {
    switch (texture1_fragment_mode)
    {
    case 0:
      pixel_shader = ModelPixelShader::Combiners_Opaque;
      break;
    case 2:
      pixel_shader = ModelPixelShader::Combiners_Decal;
      break;
    case 3:
      pixel_shader = ModelPixelShader::Combiners_Add;
      break;
    case 4:
      pixel_shader = ModelPixelShader::Combiners_Mod2x;
      break;
    case 5:
      pixel_shader = ModelPixelShader::Combiners_Fade;
      break;
    default:
      pixel_shader = ModelPixelShader::Combiners_Mod;
      break;
    }
  }
  else
  {
    if (!texture1_fragment_mode)
    {
      switch (texture2_fragment_mode)
      {
      case 0:
        pixel_shader = ModelPixelShader::Combiners_Opaque_Opaque;
        break;
      case 3:
        pixel_shader = ModelPixelShader::Combiners_Opaque_Add;
        break;
      case 4:
        pixel_shader = ModelPixelShader::Combiners_Opaque_Mod2x;
        break;
      case 6:
        pixel_shader = ModelPixelShader::Combiners_Opaque_Mod2xNA;
        break;
      case 7:
        pixel_shader = ModelPixelShader::Combiners_Opaque_AddNA;
        break;
      default:
        pixel_shader = ModelPixelShader::Combiners_Opaque_Mod;
        break;
      }
    }
    else if (texture1_fragment_mode == 1)
    {
      switch (texture2_fragment_mode)
      {
      case 0:
        pixel_shader = ModelPixelShader::Combiners_Mod_Opaque;
        break;
      case 3:
        pixel_shader = ModelPixelShader::Combiners_Mod_Add;
        break;
      case 4:
        pixel_shader = ModelPixelShader::Combiners_Mod_Mod2x;
        break;
      case 6:
        pixel_shader = ModelPixelShader::Combiners_Mod_Mod2xNA;
        break;
      case 7:
        pixel_shader = ModelPixelShader::Combiners_Mod_AddNA;
        break;
      default:
        pixel_shader = ModelPixelShader::Combiners_Mod_Mod;
        break;
      }
    }
    else if (texture1_fragment_mode == 3)
    {
      if (texture2_fragment_mode == 1)
      {
        pixel_shader = ModelPixelShader::Combiners_Add_Mod;
      }
    }
    else if (texture1_fragment_mode == 4 && texture2_fragment_mode == 4)
    {
      pixel_shader = ModelPixelShader::Combiners_Mod2x_Mod2x;
    }
    else if (texture2_fragment_mode == 1)
    {
      pixel_shader = ModelPixelShader::Combiners_Mod_Mod2x;
    }
  }
 

  return pixel_shader;
}

boost::optional<ModelPixelShader> M2GetPixelShaderID (uint16_t texture_count, uint16_t shader_id)
{
  boost::optional<ModelPixelShader> pixel_shader;

  if (!(shader_id & 0x8000))
  {
    pixel_shader = GetPixelShader(texture_count, shader_id);

    if (!pixel_shader)
    {
      pixel_shader = GetPixelShader(texture_count, 0x11);
    }
  }
  else
  {
    switch (shader_id & 0x7FFF)
    {
    case 1:
      pixel_shader = ModelPixelShader::Combiners_Opaque_Mod2xNA_Alpha;
      break;
    case 2:
      pixel_shader = ModelPixelShader::Combiners_Opaque_AddAlpha;
      break;
    case 3:
      pixel_shader = ModelPixelShader::Combiners_Opaque_AddAlpha_Alpha;
      break;
    }  
  }

  return pixel_shader;
}
}

void Model::compute_pixel_shader_ids()
{
  for (auto& pass : _render_passes)
  {
    pass.pixel_shader = M2GetPixelShaderID(pass.texture_count, pass.shader_id);
  }
}

void Model::initAnimated(const MPQFile& f)
{
  std::vector<std::unique_ptr<MPQFile>> animation_files;

  if (header.nAnimations > 0) 
  {
    std::vector<ModelAnimation> animations(header.nAnimations);

    memcpy(animations.data(), f.getBuffer() + header.ofsAnimations, header.nAnimations * sizeof(ModelAnimation));

    for (auto& anim : animations)
    {
      anim.length = std::max(anim.length, 1U);

      _animation_length[anim.animID] += anim.length;
      _animations_seq_per_id[anim.animID][anim.subAnimID] = anim;

      std::string lodname = filename.substr(0, filename.length() - 3);
      std::stringstream tempname;
      tempname << lodname << anim.animID << "-" << anim.subAnimID << ".anim";
      if (MPQFile::exists(tempname.str()))
      {
        animation_files.push_back(std::make_unique<MPQFile>(tempname.str()));
      }
    }
  }

  if (animBones)
  {
    ModelBoneDef const* mb = reinterpret_cast<ModelBoneDef const*>(f.getBuffer() + header.ofsBones);
    for (size_t i = 0; i<header.nBones; ++i)
    {
      bones.emplace_back(f, mb[i], _global_sequences.data(), animation_files);
    }
  }  

  if (animTextures) 
  {
    ModelTexAnimDef const* ta = reinterpret_cast<ModelTexAnimDef const*>(f.getBuffer() + header.ofsTexAnims);
    for (size_t i=0; i<header.nTexAnims; ++i) {
      _texture_animations.emplace_back (f, ta[i], _global_sequences.data());
    }
  }

  
  // particle systems
  if (header.nParticleEmitters) {
    ModelParticleEmitterDef const* pdefs = reinterpret_cast<ModelParticleEmitterDef const*>(f.getBuffer() + header.ofsParticleEmitters);
    for (size_t i = 0; i<header.nParticleEmitters; ++i) 
    {
      try
      {
        _particles.emplace_back (this, f, pdefs[i], _global_sequences.data());
      }
      catch (std::logic_error error)
      {
        LogError << "Loading particles for '" << filename << "' " << error.what() << std::endl;
      }      
    }
  }
  

  
  // ribbons
  if (header.nRibbonEmitters) {
    ModelRibbonEmitterDef const* rdefs = reinterpret_cast<ModelRibbonEmitterDef const*>(f.getBuffer() + header.ofsRibbonEmitters);
    for (size_t i = 0; i<header.nRibbonEmitters; ++i) {
      _ribbons.emplace_back(this, f, rdefs[i], _global_sequences.data());
    }
  }
  

  // init lights
  if (header.nLights) {
    ModelLightDef const* lDefs = reinterpret_cast<ModelLightDef const*>(f.getBuffer() + header.ofsLights);
    for (size_t i=0; i<header.nLights; ++i)
      _lights.emplace_back (f, lDefs[i], _global_sequences.data());
  }

  animcalc = false;
}

void Model::calcBones( math::matrix_4x4 const& model_view
                     , int _anim
                     , int time
                     , int animation_time
                     )
{
  for (size_t i = 0; i<header.nBones; ++i) {
    bones[i].calc = false;
  }

  for (size_t i = 0; i<header.nBones; ++i) {
    bones[i].calcMatrix(model_view, bones.data(), _anim, time, animation_time);
  }
}

void Model::animate(math::matrix_4x4 const& model_view, int anim_id, int anim_time)
{
  if (_animations_seq_per_id.empty() || _animations_seq_per_id[anim_id].empty())
  {
    // use "default" vertices if the animation hasn't been found
    if (_current_vertices.empty())
    {
      _current_vertices = _vertices;

      opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder(_vertices_buffer);
      gl.bufferData(GL_ARRAY_BUFFER, _current_vertices.size() * sizeof(ModelVertex), _current_vertices.data(), GL_STATIC_DRAW);
    }

    return;
  }

  int tmax = _animation_length[anim_id];
  int t = anim_time % tmax;
  int current_sub_anim = 0;
  int time_for_anim = t;

  for (auto const& sub_animation : _animations_seq_per_id[anim_id])
  {
    if (sub_animation.second.length > time_for_anim)
    {
      current_sub_anim = sub_animation.first;
      break;
    }

    time_for_anim -= sub_animation.second.length;
  }

  ModelAnimation const& a = _animations_seq_per_id[anim_id][current_sub_anim];

  _current_anim_seq = a.Index;//_animations_seq_lookup[anim_id][current_sub_anim];
  _anim_time = t;
  _global_animtime = anim_time;

  if (animBones) 
  {
    calcBones(model_view, _current_anim_seq, t, _global_animtime);
  }

  if (animGeometry) 
  {
    // transform vertices
    _current_vertices = _vertices;

    for (auto& vertex : _current_vertices)
    {
      ::math::vector_3d v(0, 0, 0), n(0, 0, 0);

      for (size_t b (0); b < 4; ++b)
      {
        if (vertex.weights[b] <= 0)
          continue;

        ::math::vector_3d tv = bones[vertex.bones[b]].mat * vertex.position;
        ::math::vector_3d tn = bones[vertex.bones[b]].mrot * vertex.normal;

        v += tv * (static_cast<float> (vertex.weights[b]) / 255.0f);
        n += tn * (static_cast<float> (vertex.weights[b]) / 255.0f);
      }

      vertex.position = v;
      vertex.normal = n.normalized();
    }

    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder (_vertices_buffer);
    gl.bufferData (GL_ARRAY_BUFFER, _current_vertices.size() * sizeof (ModelVertex), _current_vertices.data(), GL_STREAM_DRAW);
  }

  for (size_t i=0; i<header.nLights; ++i) 
  {
    if (_lights[i].parent >= 0) 
    {
      _lights[i].tpos = bones[_lights[i].parent].mat * _lights[i].pos;
      _lights[i].tdir = bones[_lights[i].parent].mrot * _lights[i].dir;
    }
  }

  for (auto& particle : _particles)
  {
    // random time distribution for teh win ..?
    int pt = (t + static_cast<int>(tmax*particle.tofs)) % tmax;
    particle.setup(_current_anim_seq, pt, _global_animtime);
  }

  for (size_t i = 0; i<header.nRibbonEmitters; ++i) 
  {
    _ribbons[i].setup(_current_anim_seq, t, _global_animtime);
  }

  for (auto& tex_anim : _texture_animations)
  {
    tex_anim.calc(_current_anim_seq, t, _anim_time);
  }
}

void TextureAnim::calc(int anim, int time, int animtime)
{
  mat = math::matrix_4x4::unit;
  if (trans.uses(anim)) 
  {  
    mat *= math::matrix_4x4 (math::matrix_4x4::translation, trans.getValue(anim, time, animtime));
  }
  if (rot.uses(anim)) 
  {
    mat *= math::matrix_4x4 (math::matrix_4x4::rotation, rot.getValue(anim, time, animtime));
  }
  if (scale.uses(anim)) 
  {
    mat *= math::matrix_4x4 (math::matrix_4x4::scale, scale.getValue(anim, time, animtime));
  }
}

ModelColor::ModelColor(const MPQFile& f, const ModelColorDef &mcd, int *global)
  : color (mcd.color, f, global)
  , opacity(mcd.opacity, f, global)
{}

ModelTransparency::ModelTransparency(const MPQFile& f, const ModelTransDef &mcd, int *global)
  : trans (mcd.trans, f, global)
{}

ModelLight::ModelLight(const MPQFile& f, const ModelLightDef &mld, int *global)
  : type (mld.type)
  , parent (mld.bone)
  , pos (fixCoordSystem(mld.pos))
  , tpos (fixCoordSystem(mld.pos))
  , dir (::math::vector_3d(0,1,0))
  , tdir (::math::vector_3d(0,1,0)) // obviously wrong
  , diffColor (mld.color, f, global)
  , ambColor (mld.ambColor, f, global)
  , diffIntensity (mld.intensity, f, global)
  , ambIntensity (mld.ambIntensity, f, global)
{}

void ModelLight::setup(int time, opengl::light, int animtime)
{
  math::vector_4d ambcol(ambColor.getValue(0, time, animtime) * ambIntensity.getValue(0, time, animtime), 1.0f);
  math::vector_4d diffcol(diffColor.getValue(0, time, animtime) * diffIntensity.getValue(0, time, animtime), 1.0f);
  math::vector_4d p;

  enum ModelLightTypes {
    MODELLIGHT_DIRECTIONAL = 0,
    MODELLIGHT_POINT
  };

  if (type == MODELLIGHT_DIRECTIONAL) {
    // directional
    p = math::vector_4d(tdir, 0.0f);
  }
  else if (type == MODELLIGHT_POINT) {
    // point
    p = math::vector_4d(tpos, 1.0f);
  }
  else {
    p = math::vector_4d(tpos, 1.0f);
    LogError << "Light type " << type << " is unknown." << std::endl;
  }
 
  // todo: use models' light
}

TextureAnim::TextureAnim (const MPQFile& f, const ModelTexAnimDef &mta, int *global)
  : trans (mta.trans, f, global)
  , rot (mta.rot, f, global)
  , scale (mta.scale, f, global)
  , mat (math::matrix_4x4::uninitialized)
{}

Bone::Bone( const MPQFile& f,
            const ModelBoneDef &b,
            int *global,
            const std::vector<std::unique_ptr<MPQFile>>& animation_files)
  : trans (b.translation, f, global, animation_files)
  , rot (b.rotation, f, global, animation_files)
  , scale (b.scaling, f, global, animation_files)
  , pivot (fixCoordSystem (b.pivot))
  , parent (b.parent)
{
  memcpy(&flags, &b.flags, sizeof(uint32_t));

  trans.apply(fixCoordSystem);
  rot.apply(fixCoordSystemQuat);
  scale.apply(fixCoordSystem2);
}

void Bone::calcMatrix( math::matrix_4x4 const& model_view
                     , Bone *allbones
                     , int anim
                     , int time
                     , int animtime
                     )
{
  if (calc) return;

  math::matrix_4x4 m {math::matrix_4x4::unit};
  math::quaternion q;

  if ( flags.transformed
    || flags.billboard 
    || flags.cylindrical_billboard_lock_x 
    || flags.cylindrical_billboard_lock_y 
    || flags.cylindrical_billboard_lock_z
      )
  {
    m = {math::matrix_4x4::translation, pivot};

    if (trans.uses(anim))
    {
      m *= math::matrix_4x4 (math::matrix_4x4::translation, trans.getValue (anim, time, animtime));
    }

    if (rot.uses(anim))
    {
      m *= math::matrix_4x4 (math::matrix_4x4::rotation, q = rot.getValue (anim, time, animtime));
    }

    if (scale.uses(anim))
    {
      m *= math::matrix_4x4 (math::matrix_4x4::scale, scale.getValue (anim, time, animtime));
    }

    if (flags.billboard)
    {
      math::vector_3d vRight (model_view[0], model_view[4], model_view[8]);
      math::vector_3d vUp (model_view[1], model_view[5], model_view[9]); // Spherical billboarding
      //math::vector_3d vUp = math::vector_3d(0,1,0); // Cylindrical billboarding
      vRight = vRight * -1;
      m (0, 2, vRight.x);
      m (1, 2, vRight.y);
      m (2, 2, vRight.z);
      m (0, 1, vUp.x);
      m (1, 1, vUp.y);
      m (2, 1, vUp.z);
    }

    m *= math::matrix_4x4 (math::matrix_4x4::translation, -pivot);
  }

  if (parent >= 0)
  {
    allbones[parent].calcMatrix (model_view, allbones, anim, time, animtime);
    mat = allbones[parent].mat * m;
  }
  else
  {
    mat = m;
  }

  // transform matrix for normal vectors ... ??
  if (rot.uses(anim))
  {
    if (parent >= 0)
    {
      mrot = allbones[parent].mrot * math::matrix_4x4 (math::matrix_4x4::rotation, q);
    }
    else
    {
      mrot = math::matrix_4x4 (math::matrix_4x4::rotation, q);
    }
  }
  else
  {
    mrot = math::matrix_4x4::unit;
  }

  calc = true;
}

void Model::draw( math::matrix_4x4 const& model_view
                , ModelInstance& instance
                , opengl::scoped::use_program& m2_shader
                , math::frustum const& frustum
                , const float& cull_distance
                , const math::vector_3d& camera
                , int animtime
                , bool // draw_particles
                , bool // all_boxes
                , display_mode display
                )
{
  if (!finishedLoading() || loading_failed())
  {
    return;
  }

  if (!instance.is_visible(frustum, cull_distance, camera, display))
  {
    return;
  }

  if (!_finished_upload)
  {
    upload();
  }

  if (animated && (!animcalc || _per_instance_animation))
  {
    animate(model_view, 0, animtime);
    animcalc = true;
  }

  opengl::scoped::vao_binder const _(_vao);

  m2_shader.uniform("transform", instance.transform_matrix_transposed());

  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder(_vertices_buffer);
    m2_shader.attrib(_, "pos", opengl::array_buffer_is_already_bound{}, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, position));
    //m2_shader.attrib(_, "bones_weight", opengl::array_buffer_is_already_bound{},  4, GL_UNSIGNED_BYTE,  GL_FALSE, sizeof (ModelVertex), (void*)offsetof (ModelVertex, weights));
    //m2_shader.attrib(_, "bones_indices", opengl::array_buffer_is_already_bound{}, 4, GL_UNSIGNED_BYTE,  GL_FALSE, sizeof (ModelVertex), (void*)offsetof (ModelVertex, bones));
    m2_shader.attrib(_, "normal", opengl::array_buffer_is_already_bound{}, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, normal));
    m2_shader.attrib(_, "texcoord1", opengl::array_buffer_is_already_bound{}, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, texcoords[0]));
    m2_shader.attrib(_, "texcoord2", opengl::array_buffer_is_already_bound{}, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, texcoords[1]));
  }

  for (ModelRenderPass& p : _render_passes)
  {
    if (p.prepare_draw(m2_shader, this))
    {
      gl.drawElements(GL_TRIANGLES, p.index_count, _indices, sizeof (_indices[0]) * p.index_start);
      p.after_draw();
    }
  }

  gl.disable(GL_BLEND);
  gl.enable(GL_CULL_FACE);
  gl.depthMask(GL_TRUE);
}

void Model::draw ( math::matrix_4x4 const& model_view
                 , std::vector<ModelInstance*> instances
                 , opengl::scoped::use_program& m2_shader
                 , math::frustum const& frustum
                 , const float& cull_distance
                 , const math::vector_3d& camera
                 , bool // draw_fog
                 , int animtime
                 , bool draw_particles
                 , bool all_boxes
                 , std::unordered_map<Model*, std::size_t>& models_with_particles
                 , std::unordered_map<Model*, std::size_t>& model_boxes_to_draw
                 , display_mode display
                 )
{
  if (!finishedLoading() || loading_failed())
  {
    return;
  }

  if (!_finished_upload) 
  {
    upload();
  }

  if (animated && (!animcalc || _per_instance_animation))
  {
    animate(model_view, 0, animtime);
    animcalc = true;
  }

  std::vector<math::matrix_4x4> transform_matrix;

  for (ModelInstance* mi : instances)
  {
    if (mi->is_visible(frustum, cull_distance, camera, display))
    {
      transform_matrix.push_back(mi->transform_matrix_transposed());
    }    
  }

  if (transform_matrix.empty())
  {
    return;
  }

  // store the model count to draw the bounding boxes later
  if (all_boxes || _hidden)
  {
    model_boxes_to_draw.emplace(this, transform_matrix.size());    
  }
  if (draw_particles && (!_particles.empty() || !_ribbons.empty()))
  {
    models_with_particles.emplace(this, transform_matrix.size());
  }  

  opengl::scoped::vao_binder const _ (_vao);

  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const transform_binder (_transform_buffer);
    gl.bufferData(GL_ARRAY_BUFFER, transform_matrix.size() * sizeof(::math::matrix_4x4), transform_matrix.data(), GL_DYNAMIC_DRAW);
    m2_shader.attrib(_, "transform", opengl::array_buffer_is_already_bound{}, static_cast<math::matrix_4x4*> (nullptr), 1);
  }
  
  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder (_vertices_buffer);
    m2_shader.attrib(_, "pos", opengl::array_buffer_is_already_bound{}, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, position));
    //m2_shader.attrib(_, "bones_weight", opengl::array_buffer_is_already_bound{},  4, GL_UNSIGNED_BYTE,  GL_FALSE, sizeof (ModelVertex), (void*)offsetof (ModelVertex, weights));
    //m2_shader.attrib(_, "bones_indices", opengl::array_buffer_is_already_bound{}, 4, GL_UNSIGNED_BYTE,  GL_FALSE, sizeof (ModelVertex), (void*)offsetof (ModelVertex, bones));
    m2_shader.attrib(_, "normal", opengl::array_buffer_is_already_bound{}, 3, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, normal));
    m2_shader.attrib(_, "texcoord1", opengl::array_buffer_is_already_bound{}, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, texcoords[0]));
    m2_shader.attrib(_, "texcoord2", opengl::array_buffer_is_already_bound{}, 2, GL_FLOAT, GL_FALSE, sizeof(ModelVertex), (void*)offsetof (ModelVertex, texcoords[1]));
  }

  for (ModelRenderPass& p : _render_passes)
  {
    if (p.prepare_draw(m2_shader, this))
    {
      gl.drawElementsInstanced(GL_TRIANGLES, p.index_count, transform_matrix.size(), _indices, sizeof (_indices[0]) * p.index_start);
      p.after_draw();
    }
  }

  gl.disable(GL_BLEND);
  gl.enable(GL_CULL_FACE);
  gl.depthMask(GL_TRUE);
}

void Model::draw_particles( math::matrix_4x4 const& model_view
                          , opengl::scoped::use_program& particles_shader
                          , std::size_t instance_count
                          )
{
  for (auto& p : _particles)
  {
    p.draw(model_view, particles_shader, _transform_buffer, instance_count);
  }
}

void Model::draw_ribbons( opengl::scoped::use_program& ribbons_shader
                        , std::size_t instance_count
                        )
{
  for (auto& r : _ribbons)
  {
    r.draw(ribbons_shader, _transform_buffer, instance_count);
  }
}

void Model::draw_box (opengl::scoped::use_program& m2_box_shader, std::size_t box_count)
{
  static std::vector<uint16_t> const indices ({5, 7, 3, 2, 0, 1, 3, 1, 5, 4, 0, 4, 6, 2, 6, 7});

  opengl::scoped::vao_binder const _ (_box_vao);

  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const transform_binder (_transform_buffer);
    m2_box_shader.attrib(_, "transform", opengl::array_buffer_is_already_bound{}, static_cast<math::matrix_4x4*> (nullptr), 1);
  }
  m2_box_shader.attrib(_, "position", _box_vbo, 3, GL_FLOAT, GL_FALSE, 0, 0);

  gl.drawElementsInstanced (GL_LINE_STRIP, indices.size(), box_count, indices);
}


std::vector<float> Model::intersect (math::matrix_4x4 const& model_view, math::ray const& ray, int animtime)
{
  std::vector<float> results;

  if (!finishedLoading() || loading_failed())
  {
    return results;
  }

  if (animated && (!animcalc || _per_instance_animation))
  {
    animate (model_view, 0, animtime);
    animcalc = true;
  }

  if (use_fake_geometry())
  {
    auto& fake_geom = _fake_geometry.get();

    for (size_t i = 0; i < fake_geom.indices.size(); i += 3)
    {
      if (auto distance
        = ray.intersect_triangle(fake_geom.vertices[fake_geom.indices[i + 0]],
          fake_geom.vertices[fake_geom.indices[i + 1]],
          fake_geom.vertices[fake_geom.indices[i + 2]])
        )
      {
        results.emplace_back (*distance);
      }
    }

    return results;
  }

  for (auto&& pass : _render_passes)
  {
    for (size_t i (pass.index_start); i < pass.index_start + pass.index_count; i += 3)
    {
      if ( auto distance
          = ray.intersect_triangle( _current_vertices[_indices[i + 0]].position,
                                    _current_vertices[_indices[i + 1]].position,
                                    _current_vertices[_indices[i + 2]].position)
          )
      {
        results.emplace_back (*distance);
      }
    }
  }

  return results;
}

void Model::lightsOn(opengl::light lbase)
{
  // setup lights
  for (unsigned int i=0, l=lbase; i<header.nLights; ++i) _lights[i].setup(_anim_time, l++, _global_animtime);
}

void Model::lightsOff(opengl::light lbase)
{
  for (unsigned int i = 0, l = lbase; i<header.nLights; ++i) gl.disable(l++);
}

void Model::upload()
{
  for (std::string texture : _textureFilenames)
    _textures.emplace_back(texture);

  _buffers.upload();
  _vertex_arrays.upload();

  if (!animGeometry)
  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder (_vertices_buffer);
    gl.bufferData (GL_ARRAY_BUFFER, _current_vertices.size() * sizeof (ModelVertex), _current_vertices.data(), GL_STATIC_DRAW);
  }

  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder (_box_vbo);
    gl.bufferData (GL_ARRAY_BUFFER, _vertex_box_points.size() * sizeof (math::vector_3d), _vertex_box_points.data(), GL_STATIC_DRAW);
  }

  _finished_upload = true;
}

void Model::updateEmitters(float dt)
{
  if (finished)
  {
    for (auto& particle : _particles)
    {
      particle.update (dt);
    }
  }
}
