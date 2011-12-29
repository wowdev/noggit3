#ifndef __SLIDER_H
#define __SLIDER_H

#include <string>

#include <noggit/UIFrame.h>

namespace noggit
{
  class blp_texture;
}

class UISlider : public UIFrame
{
protected:
  noggit::blp_texture* texture;
  noggit::blp_texture* sliderTexture;
  float scale;
  float offset;
  void (*func)(float value);
  std::string text;

public:
  float value;
  void setFunc( void ( *f )( float value ) );
  void setValue( float f );
  void setText( const std::string& text );
  UISlider( float x, float y, float width, float s, float o );
  ~UISlider();
  UIFrame* processLeftClick( float mx, float my );
  bool processLeftDrag( float mx, float my, float xChange, float yChange );
  void render() const;
};
#endif
