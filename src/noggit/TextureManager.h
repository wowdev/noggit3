// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/blp_texture.h>
#include <noggit/multimap_with_normalized_key.hpp>

#include <map>
#include <string>
#include <vector>

namespace noggit
{
  class blp_texture;

  struct texture_manager : private multimap_with_normalized_key<blp_texture>
  {
    //! \todo This should not be there.
    //! \note This is only for getting all cached textures in UITexturingGUI.
    std::vector<noggit::blp_texture*> getAllTexturesMatching
      (std::function<bool (std::string const&)>);

    friend struct scoped_blp_texture_reference;
  };

  struct scoped_blp_texture_reference
  {
    scoped_blp_texture_reference (std::string const& filename);

    scoped_blp_texture_reference (scoped_blp_texture_reference const&);
    scoped_blp_texture_reference (scoped_blp_texture_reference&&);
    scoped_blp_texture_reference& operator= (scoped_blp_texture_reference const&);
    scoped_blp_texture_reference& operator= (scoped_blp_texture_reference&&);

    ~scoped_blp_texture_reference();

    blp_texture* operator->() const
    {
      return _texture;
    }

    bool operator== (scoped_blp_texture_reference const& other) const
    {
      return _valid && other._valid && _filename == other._filename;
    }

  private:
    bool _valid;
    std::string _filename;
    blp_texture* _texture;
  };
}
