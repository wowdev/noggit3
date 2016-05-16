// UITexture.h is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#pragma once
#include <string>

#include <noggit/UIFrame.h>

namespace noggit
{
  class blp_texture;
}

class UITexture : public UIFrame
{
public:
  typedef UITexture* Ptr;

protected:
  noggit::blp_texture* texture;
  std::string _textureFilename;

  bool highlight;
  void (*clickFunc)(UIFrame *,int);
  int id;

public:
  UITexture( float x, float y, float width, float height, const std::string& tex );
  ~UITexture();

  void setTexture( const std::string& tex );
  void setTexture( noggit::blp_texture* tex );
  void render() const;

  UIFrame *processLeftClick( float mx, float my );
  void setClickFunc( void (*f)( UIFrame *,int ), int num );
  void setHighlight( bool h )
  {
    highlight = h;
  }
  noggit::blp_texture* getTexture( );
};
