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
    chunk::chunk(script_context * ctx, MapChunk* chunk)
    : script_object(ctx)
    , _chunk(chunk)
    {}

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
      world()->recalc_norms(_chunk);
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
      apply_heightmap();
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

    chunk_iterator::chunk_iterator(script_context * ctx, std::shared_ptr<std::vector<MapChunk*>> chunks)
    : script_object(ctx)
    , _chunks(chunks)
    {}

    bool chunk_iterator::next()
    {
      if(_cur>=int(_chunks->size()-1))
      {
        return false;
      }
      ++_cur;
      return _cur<_chunks->size();
    }

    chunk chunk_iterator::get()
    {
      return chunk(state(), (*_chunks)[_cur]);
    }

    void chunk_iterator::reset()
    {
      _cur = -1;
    }

    std::shared_ptr<selection> chunk::to_selection()
    {
      return std::make_shared<selection>(state(), "chunk#to_selection", _chunk->vmin,_chunk->vmax);
    }

    void register_chunk(script_context * state)
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
        , "to_selection", &chunk::to_selection
      ); 

      state->new_usertype<chunk_iterator>("chunk_iterator"
        , "next", &chunk_iterator::next
        , "get", &chunk_iterator::get
        , "reset", &chunk_iterator::reset
      );
    }
  } // namespace scripting
} // namespace noggit
