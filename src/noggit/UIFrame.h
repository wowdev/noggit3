#ifndef __FRAME_H
#define __FRAME_H

#include <vector>

class UIFrame
{
public:
  typedef UIFrame* Ptr;
  typedef std::vector<UIFrame::Ptr> Children;

protected:
  UIFrame::Ptr _parent;

  Children _children;

  float _x;
  float _y;
  float _width;
  float _height;

  bool _movable;
  bool _hidden;
  bool _clickable;

#define getter(var, type) inline const type& var() const { return _ ## var; }
#define setter(var, type) inline void var(const type& var) { _ ## var = var; }
#define boolsetter(var, enabled, disabled, toggle) inline void enabled() { _ ## var = true; } \
                                                   inline void disabled() { _ ## var = false; } \
                                                   inline void toggle() { _ ## var = !_ ## var; }
#define evilgetter(var, type) inline type* var ## _evil() { return &_ ## var; }

public:
  getter(width, float)
  setter(width, float)

  getter(height, float)
  setter(height, float)

  getter(x, float)
  setter(x, float)

  getter(y, float)
  setter(y, float)

  getter(children, Children)
  setter(children, Children)

  getter(parent, UIFrame::Ptr)
  setter(parent, UIFrame::Ptr)

  getter(movable, bool)
  setter(movable, bool)

  getter(hidden, bool)
  setter(hidden, bool)
  boolsetter(hidden, hide, show, toggleVisibility)
  evilgetter(hidden, bool)

  getter(clickable, bool)
  setter(clickable, bool)
#undef getter
#undef setter
#undef boolsetter
#undef evilgetter

  UIFrame()
  : _parent( NULL )
  , _children()
  , _x( 0.0f )
  , _y( 0.0f )
  , _width( 0.0f )
  , _height( 0.0f )
  , _movable( false )
  , _hidden( false )
  , _clickable( false )
  {
  }

  UIFrame( float pX, float pY, float w, float h )
  : _parent( NULL )
  , _children()
  , _x( pX )
  , _y( pY )
  , _width( w )
  , _height( h )
  , _movable( false )
  , _hidden( false )
  , _clickable( false )
  {
  }

  virtual ~UIFrame()
  {
    for( Children::iterator it( _children.begin() ), end( _children.end() )
       ; it != end; ++it )
    {
      if( *it )
      {
        delete *it;
        *it = NULL;
      }
    }
    _children.clear();
  }

  void addChild( UIFrame::Ptr );
  void removeChild( UIFrame::Ptr );

  void renderChildren() const;

  virtual void render() const;
  virtual UIFrame::Ptr processLeftClick( float mx, float my );
  virtual bool processLeftDrag( float mx, float my, float xChange, float yChange );
  virtual void processUnclick() { }
  virtual bool processRightClick( float mx, float my );
  virtual void resize()
  {
    for( Children::iterator it( _children.begin() ), end( _children.end() )
       ; it != end; ++it )
    {
      (*it)->resize();
    }
  }
  void getOffset( float* xOff, float* yOff );

  inline bool IsHit( float pX, float pY ) const
  {
    return x() < pX && x() + width() > pX && y() < pY && y() + height() > pY;
  }
};

#endif
