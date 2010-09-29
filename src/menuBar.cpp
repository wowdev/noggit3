#include "menuBar.h"
#include "video.h"
#include "textUI.h"
#include "Gui.h"
#include "checkboxUI.h"
#include "TextureManager.h" // TextureManager, Texture

#include "Environment.h" // CtrlDown

menuBar::menuBar() : window( 0.0f, 0.0f, video.xres, video.yres )
{
	texture = TextureManager::newTexture( "interface\\tooltips\\ui-tooltip-border.blp" );

	mustresize = true;
	mNumMenus = 0;
}

void menuBar::render() const
{
	glColor4f(0.2f,0.2f,0.2f,0.5f);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2f(0.0f,0.0f);
	glVertex2f(video.xres,0.0f);
	glVertex2f(0.0f,30.0f);
	glVertex2f(video.xres,30.0f);
	glEnd();


	for(unsigned int i=0;i<children.size();++i)
		if(!children[i]->hidden)
			children[i]->render();

	glColor3f(1.0f,1.0f,1.0f);

  Texture::setActiveTexture();
  Texture::enableTexture();
  
  texture->render();

	//Draw Top Side
	glBegin(GL_TRIANGLE_STRIP);	
		glTexCoord2f(0.5f,1.0f);
		glVertex2f(0.0f,33.0f);	
		glTexCoord2f(0.5f,0.0f);
		glVertex2f(video.xres,30.0f+3);	
		glTexCoord2f(0.375f,1.0f);
		glVertex2f(0.0f,17.0f);
		glTexCoord2f(0.375f,0.0f);
		glVertex2f(video.xres,17.0f);
	glEnd();
  
  Texture::disableTexture();
}

void menuBar::resize()
{
	this->width=video.xres;
}

void menuBar::CloseAll()
{
	if (!Environment::getInstance()->CtrlDown)
	{
		std::map<std::string, MenuPane*>::iterator lPaneIterator;
		for( lPaneIterator = this->mMenuPanes.begin(); lPaneIterator != this->mMenuPanes.end(); lPaneIterator++ )
		{
			lPaneIterator->second->hidden = true;
		}
	}
}

void menuBar::AddMenu( const std::string& pName )
{
	mMenuPanes[pName] = new MenuPane( this, 1.0f + mNumMenus * 101.0f, 33.0f );

	this->addChild( mMenuPanes[pName] );
	this->addChild( new MenuButton( mMenuPanes[pName], 3.0f + mNumMenus * 100.0f, 5.0f, pName ) );

	mNumMenus++;
}

MenuPane* menuBar::GetMenu( const std::string& pName )
{
	return mMenuPanes[pName];
}

frame* menuBar::processLeftClick(float mx,float my)
{
  for( std::vector<frame*>::iterator it = children.begin(); it != children.end(); ++it )
  {
		if( !(*it)->hidden && ( (*it)->x < mx ) && ( ( (*it)->x + (*it)->width ) > mx ) && ( (*it)->y < my ) && ( ( (*it)->y + (*it)->height ) > my ) )
		{
			return (*it)->processLeftClick( mx - (*it)->x, my - (*it)->y );
		}
  }
	CloseAll();	
	return 0;
}



MenuButton::MenuButton( MenuPane * pPane, float pX, float pY, const std::string& pText ) : buttonUI( pX, pY, 95.0f, 27.0f, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setText( pText );
	mPane = pPane;
}

frame* MenuButton::processLeftClick( float pX, float pY )
{
	clicked = true;
	mPane->Open();

	return this;
}

MenuItem::MenuItem( MenuPane * pParent, float pX, float pY, float pHeight, const std::string& pText, const std::string& pNormal, const std::string& pDown ) : buttonUI( pX, pY, 170.0f, 30.0f, pNormal, pDown )
{
	this->setText( pText );
	this->height = pHeight;
	this->setLeft();

	mParent = pParent;
}

MenuItemButton::MenuItemButton( MenuPane * pParent, float pX, float pY, const std::string& pText, void ( *pClickFunc )( frame *, int ), int pClickFuncID ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setClickFunc( pClickFunc, pClickFuncID );
}

frame* MenuItemButton::processLeftClick( float pX, float pY )
{
	clicked = true;
	if( this->clickFunc )
		this->clickFunc( this, this->id );

	if(!Environment::getInstance()->CtrlDown) 
    this->mParent->Close();

	return this;
}

MenuItemToggle::MenuItemToggle( MenuPane * pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setText( pText );
	this->setLeft();

	mMyCheckbox = new checkboxUI( 147.0f, -1.0f, "" );
	this->addChild( mMyCheckbox );

	mMyState = pMyState;
	mInvert = pInvert;
	mMyCheckbox->setState( *mMyState );
}

frame* MenuItemToggle::processLeftClick( float pX, float pY )
{
	clicked = true;
	*mMyState = !( *mMyState );
	if( mInvert )
		mMyCheckbox->setState( !( *mMyState ) );
	else
		mMyCheckbox->setState( *mMyState );
	this->mParent->Close();
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
  glTranslatef( x, y, 0.0f );
  
  Texture::setActiveTexture();
  Texture::enableTexture();
  
	if( !clicked )
    texture->render();
  else
    textureDown->render();

  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( 0.0f, 0.0f );
  glTexCoord2f( 1.0f, .0f );
  glVertex2f( width, 0.0f );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( 0.0f, height );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width, height );
  glEnd();
  
  Texture::disableTexture();
  
  text->render();
	mMyCheckbox->render();
  
	glPopMatrix();
}

MenuItemSwitch::MenuItemSwitch( MenuPane * pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setText( pText );
	this->setLeft();

	mMyState = pMyState;
	mInvert = pInvert;
}

frame* MenuItemSwitch::processLeftClick( float pX, float pY )
{
	clicked = true;
	*mMyState = mInvert;
	this->mParent->Close();
	return this;
}


MenuItemSet::MenuItemSet( MenuPane * pParent, float pX, float pY, const std::string& pText, int * pMyState, int pSet ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setText( pText );
	this->setLeft();

	mMyState = pMyState;
	mSet = pSet;
}

frame* MenuItemSet::processLeftClick( float pX, float pY )
{
	clicked = true;
	*mMyState = mSet;
	this->mParent->Close();
	return this;
}

MenuItemSeperator::MenuItemSeperator( MenuPane * pParent, float pX, float pY, const std::string& pText ) : 
	MenuItem( pParent, pX, pY, 20.0f, pText, "Interface\\BUTTONS\\UI-SliderBar-Background.blp", "Interface\\BUTTONS\\UI-SliderBar-Background.blp" )
{
	this->setText( pText );
}

frame* MenuItemSeperator::processLeftClick( float pX, float pY )
{
	return 0;
}

MenuPane::MenuPane( menuBar * pMenuBar, float pX, float pY ) : window( pX, pY, 180.0f, 1.0f )
{
	this->movable = false;
	this->hidden = true;

	mMenuBar = pMenuBar;
	mNumItems = 0;
}

void MenuPane::Close()
{
	mMenuBar->CloseAll();
}

void MenuPane::Open()
{
	mMenuBar->CloseAll();
	this->hidden = false;
}

void MenuPane::AddMenuItemButton( const std::string& pName, void ( *pClickFunc )( frame *, int ), int pClickFuncID )
{
	this->addChild( new MenuItemButton( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pClickFunc, pClickFuncID ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemToggle( const std::string& pName, bool * pMyState, bool pInvert )
{
	this->addChild( new MenuItemToggle( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemSwitch( const std::string& pName, bool * pMyState, bool pInvert )
{
	this->addChild( new MenuItemSwitch( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemSet( const std::string& pName, int * pMyState, int pSet )
{
	this->addChild( new MenuItemSet( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pSet ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemSeperator( const std::string& pName )
{
	this->addChild( new MenuItemSeperator( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
