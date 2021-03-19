// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/scripting/script_chunk.hpp>
#include <noggit/scripting/script_selection.hpp>
#include <noggit/scripting/script_context.hpp>
#include <noggit/scripting/script_exception.hpp>
#include <noggit/MapChunk.h>
#include <noggit/MapHeaders.h>
#include <noggit/World.h>

#include <boost/algorithm/string/predicate.hpp>

namespace noggit
{
  namespace scripting
  {
    chunk::chunk(MapChunk* chunk)
    {
      _chunk = chunk;
    }

    void chunk::set_hole(bool hole)
    {
      _chunk->setHole(math::vector_3d(0, 0, 0), true, hole);
    }

    int chunk::add_texture(std::string const& texture)
    {
      std::string tex = std::string(texture);
      return _chunk->texture_set->addTexture(scoped_blp_texture_reference(tex));
    }

    void chunk::clear_textures()
    {
      _chunk->texture_set->eraseTextures();
    }

    void chunk::remove_texture(int index)
    {
      if(index<0||index>3)
      {
        throw script_exception(
          "chunk_remove_texture",
          "invalid texture index, must be between 0-3");
      }
      _chunk->texture_set->eraseTexture(index);
    }

    std::string chunk::get_texture(int index)
    {
      if(index<0||index>3)
      {
        throw script_exception(
          "chunk_get_texture",
          "invalid texture index, must be between 0-3");
      }
      return _chunk->texture_set->texture(index)->filename;
    }

    void chunk::apply_heightmap ()
    {
      _chunk->updateVerticesData();
      // TODO: fix
      //get_ctx (context, "chunk_apply_heightmap")->_world->recalc_norms(chunk._chunk);
    }

    void chunk::apply_textures()
    {
      _chunk->texture_set->apply_alpha_changes();
    }

    void chunk::apply_vertex_color()
    {
      _chunk->UpdateMCCV();
    }

    void chunk::apply_all()
    {
      // TODO: fix
      //chunk_apply_heightmap(chunk, context);
      apply_textures();
      apply_vertex_color();
    }

    int chunk::get_area_id()
    {
      return _chunk->getAreaID();
    }

    void chunk::set_area_id(int value)
    {
      return _chunk->setAreaID(value);
    }

    void chunk::set_impassable(bool add)
    {
      _chunk->setFlag(add, 0x2);
    }

    void chunk::clear_colors()
    {
      std::fill (
        _chunk->mccv,
        _chunk->mccv + mapbufsize,
        math::vector_3d (1.f, 1.f, 1.f)
      );
    }

    void register_chunk(lua_state * state)
    {
      state->new_usertype<chunk>("chunk"
        , "remove_texture", &chunk::remove_texture
        , "get_texture", &chunk::get_texture
        , "add_texture", &chunk::add_texture
        , "clear_textures", &chunk::clear_textures
        , "set_hole", &chunk::set_hole
        , "clear_colors", &chunk::clear_colors
        , "apply_textures", &chunk::apply_textures
        , "apply_heightmap", &chunk::apply_heightmap
        , "apply_vertex_color", &chunk::apply_vertex_color
        , "apply_all", &chunk::apply_all
        , "set_impassable", &chunk::set_impassable
        , "get_area_id", &chunk::get_area_id
        , "set_area_id", &chunk::set_area_id
      ); 
    }
  } // namespace scripting
} // namespace noggit
