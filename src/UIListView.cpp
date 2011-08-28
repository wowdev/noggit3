#include "UIListView.h"

#include <vector>

#include "Log.h"
#include "Misc.h"
#include "UIScrollBar.h"

void changeValue( UIFrame::Ptr f, int set )
{
  ( static_cast<UIListView::Ptr>( f->parent() ) )->recalcElements( set + 1 );
}

UIListView::UIListView( float xPos, float yPos, float w, float h, int elementHeight )
: UIFrame( xPos, yPos, w, h )
, elements_height( elementHeight )
, elements_start( 0 )
, elements_rows( height() / elementHeight )
, scrollbar( new UIScrollBar( width() - 22.0f, 5.0f, height() - 10.0f, 0.0f ) )
{
  scrollbar->clickable( true );
  scrollbar->setChangeFunc( changeValue );
  addChild( scrollbar );
}

void UIListView::clear()
{
  _children.erase( _children.begin(), _children.end() - 1 );
  /*
  for( Children::iterator child( children.begin() ), end( children.end() )
     ; child != end && children.size() != 1; ++child )
  {
    children.erase( child );
  }
  */
  scrollbar->setNum( 0 );
}

void UIListView::addElement( UIFrame::Ptr element )
{
  element->x( 4.0f );
  element->y( 0.0f );
  element->height( elements_height );
  element->width( width() - 20.0f );
  addChild( element );
  scrollbar->setNum( children().size() - elements_rows );
  recalcElements( 1 );
}

int UIListView::getElementsCount()
{
  return children().size();
}

void UIListView::recalcElements( unsigned int value )
{
  elements_start = value;
  // recalculate the position and the hide value off all child.
  int rowCount( 0 );
  for( size_t i( 1 ); i < _children.size(); ++i )
  {
    if( i >= value && i < value + elements_rows )
    {
      // elements in the view block
      _children[i]->y( rowCount * elements_height );
      _children[i]->show();
      rowCount++;
    }
    else
    {
      _children[i]->hide();
    }
  }
}