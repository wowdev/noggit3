// This file is part of Noggit3, licensed via GNU General Public License (version 3).

#include <noggit/TextureManager.h>

#include <noggit/application.h>

namespace noggit
{
  std::vector<noggit::blp_texture*> texture_manager::getAllTexturesMatching
    (std::function<bool (std::string const&)> function)
  {
    std::vector<noggit::blp_texture*> returnVector;
    apply ( [&] (std::string const& key, blp_texture& element)
            {
              if (function (key))
              {
                returnVector.emplace_back (&element);
              }
            }
          );
    return returnVector;
  }

  scoped_blp_texture_reference::scoped_blp_texture_reference (std::string const& filename)
    : _valid (true)
    , _filename (filename)
    , _texture (noggit::app().texture_manager().emplace (_filename))
  {}

  scoped_blp_texture_reference::scoped_blp_texture_reference (scoped_blp_texture_reference const& other)
    : scoped_blp_texture_reference (other._filename)
  {}
  scoped_blp_texture_reference::scoped_blp_texture_reference (scoped_blp_texture_reference&& other)
    : _valid (std::move (other._valid))
    , _filename (std::move (other._filename))
    , _texture (std::move (other._texture))
  {
    other._valid = false;
  }
  scoped_blp_texture_reference& scoped_blp_texture_reference::operator= (scoped_blp_texture_reference const& other)
  {
    if (_valid)
    {
      noggit::app().texture_manager().erase (_filename);
    }
    _valid = other._valid;
    _filename = other._filename;
    _texture = noggit::app().texture_manager().emplace (_filename);
    return *this;
  }
  scoped_blp_texture_reference& scoped_blp_texture_reference::operator= (scoped_blp_texture_reference&& other)
  {
    std::swap (_valid, other._valid);
    std::swap (_filename, other._filename);
    std::swap (_texture, other._texture);
    return *this;
  }

  scoped_blp_texture_reference::~scoped_blp_texture_reference()
  {
    if (_valid)
    {
      noggit::app().texture_manager().erase (_filename);
    }
  }
}
