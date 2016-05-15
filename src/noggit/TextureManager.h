// TextureManager.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

#include <map>
#include <string>
#include <vector>

namespace noggit
{
  class blp_texture;
  struct scoped_blp_texture_reference;
}

class TextureManager
{
public:

  static void report();

  //! \todo This should not be there.
  //! \note This is only for getting all cached textures in UITexturingGUI.
  static std::vector<noggit::blp_texture*> getAllTexturesMatching( bool (*function)( const std::string& name ) );

private:
  typedef std::map<std::string, noggit::blp_texture*> mapType;
  static mapType items;

  friend struct noggit::scoped_blp_texture_reference;
  static void delbyname( std::string name );
  static noggit::blp_texture* newTexture(std::string name);
};

namespace noggit
{
  struct scoped_blp_texture_reference
  {
    scoped_blp_texture_reference (std::string const& filename)
      : _valid (true)
      , _filename (filename)
      , _texture (TextureManager::newTexture (_filename))
    {}

    scoped_blp_texture_reference (scoped_blp_texture_reference const& other)
      : scoped_blp_texture_reference (other._filename)
    {}
    scoped_blp_texture_reference (scoped_blp_texture_reference&& other)
      : _valid (std::move (other._valid))
      , _filename (std::move (other._filename))
      , _texture (std::move (other._texture))
    {
      other._valid = false;
    }
    scoped_blp_texture_reference& operator= (scoped_blp_texture_reference const& other)
    {
      if (_valid)
      {
        TextureManager::delbyname (_filename);
      }
      _valid = other._valid;
      _filename = other._filename;
      _texture = TextureManager::newTexture (_filename);
      return *this;
    }
    scoped_blp_texture_reference& operator= (scoped_blp_texture_reference&& other)
    {
      std::swap (_valid, other._valid);
      std::swap (_filename, other._filename);
      std::swap (_texture, other._texture);
      return *this;
    }

    ~scoped_blp_texture_reference()
    {
      if (_valid)
      {
        TextureManager::delbyname (_filename);
      }
    }

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

#endif
