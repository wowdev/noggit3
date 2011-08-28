#include "UIMenuBar.h"

#include <map>
#include <string>
#include <vector>

#include "Environment.h" // CtrlDown
#include "UICheckBox.h"
#include "UIText.h"
#include "Video.h"

UIMenuBar::UIMenuBar()
: UIWindow( 0.0f, 0.0f, static_cast<float>( video.xres() ), static_cast<float>( video.yres() ) )
, mNumMenus( 0 )
{
}

void UIMenuBar::render() const
{
  glColor4f( 0.2f, 0.2f, 0.2f, 0.5f );
  glBegin( GL_TRIANGLE_STRIP );
  glVertex2f( 0.0f, 0.0f );
  glVertex2f( static_cast<float>( video.xres() ), 0.0f );
  glVertex2f( 0.0f, 30.0f );
  glVertex2f( static_cast<float>( video.xres() ), 30.0f );
  glEnd();

  UIFrame::render();

  glColor3f( 1.0f, 1.0f, 1.0f );

  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();
  
  texture->bind();

  //Draw Top Side
  glBegin( GL_TRIANGLE_STRIP );
    glTexCoord2f( 0.5f, 1.0f );
    glVertex2f( 0.0f, 33.0f );  
    glTexCoord2f( 0.5f, 0.0f );
    glVertex2f( static_cast<float>( video.xres() ), 33.0f );  
    glTexCoord2f( 0.375f, 1.0f );
    glVertex2f( 0.0f, 17.0f );
    glTexCoord2f( 0.375f, 0.0f );
    glVertex2f( static_cast<float>(video.xres() ), 17.0f );
  glEnd();
  
  OpenGL::Texture::disableTexture();
}

void UIMenuBar::resize()
{
  width( static_cast<float>( video.xres() ) );
}

void UIMenuBar::CloseAll()
{
  if( !Environment::getInstance()->CtrlDown )
  {
    for( MenuPanes::iterator it( mMenuPanes.begin() ), end( mMenuPanes.end() )
       ; it != end; ++it )
    {
      it->second->hide();
    }
  }
}

void UIMenuBar::AddMenu( const std::string& pName )
{
  mMenuPanes[pName] = new MenuPane( 1.0f + mNumMenus * 101.0f, 33.0f );

  addChild( mMenuPanes[pName] );
  addChild( new MenuButton( mMenuPanes[pName], 3.0f + mNumMenus * 100.0f, 5.0f, pName ) );

  mNumMenus++;
}

MenuPane::Ptr UIMenuBar::GetMenu( const std::string& pName )
{
  return mMenuPanes[pName];
}

UIFrame::Ptr UIMenuBar::processLeftClick(float mx,float my)
{
  UIFrame::Ptr tmp( UIFrame::processLeftClick( mx, my ) );
  if( tmp )
  {
    return tmp;
  }
  
  CloseAll();
  return NULL;
}

MenuButton::MenuButton( MenuPane::Ptr pPane, float pX, float pY, const std::string& pText )
: UIButton( pX, pY, 95.0f, 27.0f, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
, mPane( pPane )
{
  setText( pText );
}

UIFrame* MenuButton::processLeftClick( float /*pX*/, float /*pY*/ )
{
  clicked = true;
  mPane->Open();

  return this;
}

MenuItem::MenuItem( MenuPane::Ptr pParent, float pX, float pY, float pHeight, const std::string& pText, const std::string& pNormal, const std::string& pDown )
: UIButton( pX, pY, pHeight, pText, pNormal, pDown )
, mParent( pParent )
{
  setLeft();
}

MenuItemButton::MenuItemButton( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, void ( *pClickFunc )( UIFrame::Ptr, int ), int pClickFuncID )
: MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
  setClickFunc( pClickFunc, pClickFuncID );
}

UIFrame* MenuItemButton::processLeftClick( float /*pX*/, float /*pY*/ )
{
  clicked = true;
  if( clickFunc )
    clickFunc( this, id );

  if(!Environment::getInstance()->CtrlDown) 
    mParent->Close();

  return this;
}

MenuItemToggle::MenuItemToggle( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert )
: MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
, mMyCheckbox( new UICheckBox( 147.0f, -1.0f, "" ) )
, mMyState( pMyState )
, mInvert( pInvert )
{
  setText( pText );
  setLeft();

  addChild( mMyCheckbox );

  mMyCheckbox->setState( *mMyState );
}

UIFrame* MenuItemToggle::processLeftClick( float /*pX*/, float /*pY*/ )
{
  clicked = true;
  *mMyState = !( *mMyState );
  if( mInvert )
    mMyCheckbox->setState( !( *mMyState ) );
  else
    mMyCheckbox->setState( *mMyState );
  mParent->Close();
  return this;
}

void MenuItemToggle::render() const
{
  if( mInvert )
    mMyCheckbox->setState( !( *mMyState ) );
  else
    mMyCheckbox->setState( *mMyState );

  glColor3f( 1.0f, 1.0f, 1.0f );

  glPushMatrix();
  glTranslatef( x(), y(), 0.0f );
  
  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();
  
  if( !clicked )
    texture->bind();
  else
    textureDown->bind();

  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( 0.0f, 0.0f );
  glTexCoord2f( 1.0f, .0f );
  glVertex2f( width(), 0.0f );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( 0.0f, height() );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width(), height() );
  glEnd();
  
  OpenGL::Texture::disableTexture();
  
  text->render();
  mMyCheckbox->render();
  
  glPopMatrix();
}

MenuItemSwitch::MenuItemSwitch( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert )
: MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
, mMyState( pMyState )
, mInvert( pInvert )
{
  setText( pText );
  setLeft();
}

UIFrame::Ptr MenuItemSwitch::processLeftClick( float /*pX*/, float /*pY*/ )
{
  clicked = true;
  *mMyState = mInvert;
  mParent->Close();
  return this;
}


MenuItemSet::MenuItemSet( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, int * pMyState, int pSet )
: MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
, mSet( pSet )
, mMyState( pMyState )
{
  setText( pText );
  setLeft();
}

UIFrame::Ptr MenuItemSet::processLeftClick( float /*pX*/, float /*pY*/ )
{
  clicked = true;
  *mMyState = mSet;
  mParent->Close();
  return this;
}

MenuItemSeperator::MenuItemSeperator( MenuPane::Ptr pParent, float pX, float pY, const std::string& pText )
: MenuItem( pParent, pX, pY, 20.0f, pText, "Interface\\BUTTONS\\UI-SliderBar-Background.blp", "Interface\\BUTTONS\\UI-SliderBar-Background.blp" )
{
  setText( pText );
}

UIFrame::Ptr MenuItemSeperator::processLeftClick( float /*pX*/, float /*pY*/ )
{
  return NULL;
}

MenuPane::MenuPane( float pX, float pY )
: UIWindow( pX, pY, 180.0f, 1.0f )
{
  movable( false );
  hide();
  
  mNumItems = 0;
}

void MenuPane::Close()
{
  static_cast<UIMenuBar::Ptr>( parent() )->CloseAll();
}

void MenuPane::Open()
{
  Close();
  show();
}

void MenuPane::fixSizes()
{
  height( 6.0f + mNumItems * 25.0f );
  
  width( std::max( ( *_children.rbegin() )->width() + 5.0f, width() ) );
  const float buttonWidth = width() - 5.0f;
  for( Children::iterator it( _children.begin() ), end( _children.end() )
     ; it != end; ++it )
  {
    (*it)->width( buttonWidth );
  }
}

void MenuPane::AddMenuItemButton( const std::string& pName, void ( *pClickFunc )( UIFrame::Ptr, int ), int pClickFuncID )
{
  addChild( new MenuItemButton( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pClickFunc, pClickFuncID ) );
  fixSizes();
}
void MenuPane::AddMenuItemToggle( const std::string& pName, bool * pMyState, bool pInvert )
{
  addChild( new MenuItemToggle( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert ) );
  fixSizes();
}
void MenuPane::AddMenuItemSwitch( const std::string& pName, bool * pMyState, bool pInvert )
{
  addChild( new MenuItemSwitch( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert ) );
  fixSizes();
}
void MenuPane::AddMenuItemSet( const std::string& pName, int * pMyState, int pSet )
{
  addChild( new MenuItemSet( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pSet ) );
  fixSizes();
}
void MenuPane::AddMenuItemSeperator( const std::string& pName )
{
  addChild( new MenuItemSeperator( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName ) );
  fixSizes();
}
