#include <noggit/UIScrollableFrame.h>

#include <opengl/types.h>

#include <noggit/Log.h>

void UIScrollableFrame_Func_Hor( UIFrame::Ptr sender, int value )
{
  static_cast<UIScrollableFrame::Ptr>( sender->parent() )->scrolledHorizontal( value );
}
void UIScrollableFrame_Func_Vert( UIFrame::Ptr sender, int value )
{
  static_cast<UIScrollableFrame::Ptr>( sender->parent() )->scrolledVertical( value );
}

UIScrollableFrame::UIScrollableFrame( float x, float y, float w, float h
                                    , UIFrame::Ptr content )
: UIFrame( x, y, std::min( w, content->width() ) + UIScrollBar::WIDTH
         , std::min( h, content->height() ) + UIScrollBar::WIDTH )
, _content( content )
, _scrollbarHorizontal( new UIScrollBar( 0.0f, height() - UIScrollBar::WIDTH
                      , width() - UIScrollBar::WIDTH, 0, UIScrollBar::Horizontal ) )
, _scrollbarVertical( new UIScrollBar( width() - UIScrollBar::WIDTH, 0.0f
                    , height() - UIScrollBar::WIDTH, 0, UIScrollBar::Vertical ) )
, _scrollPositionX( 0.0f )
, _scrollPositionY( 0.0f )
{
  addChild( _scrollbarHorizontal );
  addChild( _scrollbarVertical );
  addChild( _content );

  _scrollbarHorizontal->setChangeFunc( UIScrollableFrame_Func_Hor );
  _scrollbarVertical->setChangeFunc( UIScrollableFrame_Func_Vert );

  contentUpdated();
}

void UIScrollableFrame::contentUpdated()
{
  _scrollbarHorizontal->setNum( int (_content->width() - width() - UIScrollBar::WIDTH) );
  _scrollbarVertical->setNum( int (_content->height() - height() - UIScrollBar::WIDTH) );

  if( _content->width() == width() - UIScrollBar::WIDTH )
    _scrollbarHorizontal->hide();
  if( _content->height() == height() - UIScrollBar::WIDTH )
    _scrollbarVertical->hide();
}

void UIScrollableFrame::scrolledHorizontal( int value )
{
  _scrollPositionX = static_cast<float>( value );
}
void UIScrollableFrame::scrolledVertical( int value )
{
  _scrollPositionY = static_cast<float>( value );
}

void UIScrollableFrame::render() const
{
  if( hidden() )
  {
    return;
  }

  glPushMatrix();
  glTranslatef( x(), y(), 0.0f );

  glClearStencil( 0 );
  glClear( GL_STENCIL_BUFFER_BIT );

  glColorMask( false, false, false, false );

  glEnable( GL_STENCIL_TEST );

  glStencilFunc( GL_ALWAYS, 1, 1 );
  glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );

  glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
  glBegin( GL_TRIANGLE_STRIP );
  glVertex2f( 0.0f, 0.0f );
  glVertex2f( width() - _scrollbarVertical->width(), 0.0f );
  glVertex2f( 0.0f, height() - _scrollbarHorizontal->height() );
  glVertex2f( width() - _scrollbarVertical->width(), height() - _scrollbarHorizontal->height() );
  glEnd();

  glColorMask( true, true, true, true );

  glStencilFunc( GL_EQUAL, 1, 1 );
  glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

  glPushMatrix();
  glTranslatef( -_scrollPositionX, -_scrollPositionY, 0.0f );

  _content->render();

  glPopMatrix();

  glDisable( GL_STENCIL_TEST );

  _scrollbarHorizontal->render();
  _scrollbarVertical->render();

  glPopMatrix();
}
