#ifndef __FRAME_H
#define __FRAME_H

#include <vector>

class UIFrame
{
public:
  std::vector<UIFrame *> children;
  float x, y;
  float width, height;

  UIFrame* parent;

  bool movable;
  bool hidden;
  bool clickable;

  UIFrame()
  : x( 0.0f )
  , y( 0.0f )
  , width( 0.0f )
  , height( 0.0f )
  , parent( 0 )
  , movable( false )
  , hidden( false )
  , clickable( false )
  {
  }

  UIFrame( float pX, float pY, float w, float h )
  : x( pX )
  , y( pY )
  , width( w )
  , height( h )
  , parent( 0 )
  , movable( false )
  , hidden( false )
  , clickable( false )
  {
  }
  
  virtual ~UIFrame()
  {
    for( std::vector<UIFrame*>::iterator it = children.begin(); it != children.end(); ++it )
    {
      if( *it )
      {
        delete *it;
        *it = NULL;
      }
    }
  }

  void addChild( UIFrame * );
  void removeChild( UIFrame* );
  virtual void render() const;
  virtual UIFrame *processLeftClick( float mx, float my );
  virtual bool processLeftDrag( float mx, float my, float xChange, float yChange );
  virtual void processUnclick() { }
  virtual bool processRightClick( float mx, float my );
  virtual bool processKey( char key, bool shift, bool alt, bool ctrl );
  virtual void resize()
  {
    for( std::vector<UIFrame*>::iterator it = children.begin(); it != children.end(); ++it )
      (*it)->resize();
  }
  void getOffset( float* xOff, float* yOff );

  bool IsHit( float pX, float pY )
  {
    return this->x < pX && this->x + this->width > pX && this->y < pY && this->y + this->height > pY;
  }
  
  void hide();
  void show();
  void toggleVisibility();
};

#endif
