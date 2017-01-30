// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/TextureManager.h>
#include <noggit/Video.h> // OpenGL::Texture
#include <noggit/Log.h> // LogDebug

#include <algorithm>

decltype (TextureManager::_) TextureManager::_;

void TextureManager::report()
{
  std::string output = "Still in the Texture manager:\n";
  _.apply ( [&] (std::string const& key, blp_texture const&)
            {
              output += " - " + key + "\n";
            }
          );
  LogDebug << output;
}

std::vector<scoped_blp_texture_reference> TextureManager::getAllTexturesMatching(bool(*function)(const std::string& name))
{
  std::vector<scoped_blp_texture_reference> results;
  _.apply ( [&] (std::string const& key, blp_texture const&)
            {
              if (function (key))
              {
                results.emplace_back (key);
              }
            }
          );
  return results;
}
