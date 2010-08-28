#include "menuBar.h"
#include "video.h"

#include "Settings.h"
#include "Project.h"
#include "Environment.h"

menuBar::menuBar( ) : window( 0.0f, 0.0f, video.xres, video.yres )
{
	texture = video.textures.add( "interface\\tooltips\\ui-tooltip-border.blp" );

	mustresize = true;
	mNumMenus = 0;
}

void menuBar::render()
{
	glColor4f(0.2f,0.2f,0.2f,0.5f);
	glBegin(GL_TRIANGLE_STRIP);
	glVertex2f(0.0f,0.0f);
	glVertex2f(video.xres,0.0f);
	glVertex2f(0.0f,30.0f);
	glVertex2f(video.xres,30.0f);
	glEnd();


	for(unsigned int i=0;i<children.size();i++)
		if(!children[i]->hidden)
			children[i]->render();

	glColor3f(1.0f,1.0f,1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_TEXTURE_2D);

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

	glDisable(GL_TEXTURE_2D);
}

void menuBar::resize()
{
	this->width=video.xres;
}

void menuBar::CloseAll( )
{
	if (!Environment::getInstance()->CtrlDown)
	{
		std::map<std::string, MenuPane*>::iterator lPaneIterator;
		for( lPaneIterator = this->mMenuPanes.begin( ); lPaneIterator != this->mMenuPanes.end( ); lPaneIterator++ )
		{
			lPaneIterator->second->hidden = true;
		}
	}
}

void menuBar::AddMenu( std::string pName )
{
	MenuPane * lMenuPane = new MenuPane( this, 1.0f + mNumMenus * 101.0f, 33.0f );
	MenuButton * lMenuButton = new MenuButton( lMenuPane, 3.0f + mNumMenus * 100.0f, 5.0f, pName );

	mMenuPanes[pName] = lMenuPane;

	this->addChild( reinterpret_cast<frame*>( lMenuPane ) );
	this->addChild( reinterpret_cast<frame*>( lMenuButton ) );

	mNumMenus++;
}

MenuPane * menuBar::GetMenu( std::string pName )
{
	return mMenuPanes[pName];
}

frame *menuBar::processLeftClick(float mx,float my)
{
	for(int i=children.size()-1;i>=0;i--)
	{
		if(!children[i]->hidden && (children[i]->x<mx)&&(children[i]->x+children[i]->width>mx)&&(children[i]->y<my)&&(children[i]->y+children[i]->height>my))
		{
			return children[i]->processLeftClick(mx-children[i]->x,my-children[i]->y);
		}
	}
	
	CloseAll( );	
	return 0;
}



MenuButton::MenuButton( MenuPane * pPane, float pX, float pY, std::string pText ) : buttonUI( pX, pY, 95.0f, 27.0f, video.textures.add( "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp" ), video.textures.add( "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" ) )
{
	this->setText( pText.c_str( ) );
	mPane = pPane;
}

frame* MenuButton::processLeftClick( float pX, float pY )
{
	clicked = true;
	mPane->Open( );

	return this;
}

MenuItem::MenuItem( MenuPane * pParent, float pX, float pY, float pHeight, std::string pText, std::string pNormal, std::string pDown ) : buttonUI( pX, pY, 170.0f, 30.0f, video.textures.add( pNormal), video.textures.add( pDown ) )
{
	this->setText( pText.c_str( ) );
	this->height = pHeight;
	this->setLeft( );

	mParent = pParent;
}

MenuItemButton::MenuItemButton( MenuPane * pParent, float pX, float pY, std::string pText, void ( *pClickFunc )( frame *, int ), int pClickFuncID ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setClickFunc( pClickFunc, pClickFuncID );
}

frame* MenuItemButton::processLeftClick( float pX, float pY )
{
	clicked = true;
	if( this->clickFunc )
		this->clickFunc( this, this->id );

	if(!Environment::getInstance()->CtrlDown) this->mParent->Close();
	return this;
}

MenuItemToggle::MenuItemToggle( MenuPane * pParent, float pX, float pY, std::string pText, bool * pMyState, bool pInvert ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setText( pText.c_str( ) );
	this->setLeft( );

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

void MenuItemToggle::render( )
{
	if( mInvert )
		mMyCheckbox->setState( !( *mMyState ) );
	else
		mMyCheckbox->setState( *mMyState );

	glColor3f( 1.0f, 1.0f, 1.0f );

	glActiveTexture( GL_TEXTURE0 );

	if( !clicked )
		glBindTexture( GL_TEXTURE_2D, texture );
	else
		glBindTexture( GL_TEXTURE_2D, textureDown );

	glPushMatrix( );
		glTranslatef( x, y, 0.0f );

		glEnable( GL_TEXTURE_2D );
			glBegin( GL_TRIANGLE_STRIP );
				glTexCoord2f( 0.0f, 0.0f );
				glVertex2f( 0.0f, 0.0f );
				glTexCoord2f( 1.0f, 0.0f );
				glVertex2f( width, 0.0f );
				glTexCoord2f( 0.0f, 1.0f );
				glVertex2f( 0.0f, height );
				glTexCoord2f( 1.0f, 1.0f );
				glVertex2f( width, height );
			glEnd( );
		glDisable( GL_TEXTURE_2D );
	
		text->render( );
		mMyCheckbox->render( );
	glPopMatrix( );
}

MenuItemSwitch::MenuItemSwitch( MenuPane * pParent, float pX, float pY, std::string pText, bool * pMyState, bool pInvert ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setText( pText.c_str( ) );
	this->setLeft( );

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

void MenuItemSwitch::render( )
{
	glColor3f( 1.0f, 1.0f, 1.0f );

	glActiveTexture( GL_TEXTURE0 );

	if( !clicked )
		glBindTexture( GL_TEXTURE_2D, texture );
	else
		glBindTexture( GL_TEXTURE_2D, textureDown );

	glPushMatrix( );
		glTranslatef( x, y, 0.0f );

		glEnable( GL_TEXTURE_2D );
			glBegin( GL_TRIANGLE_STRIP );
				glTexCoord2f( 0.0f, 0.0f );
				glVertex2f( 0.0f, 0.0f );
				glTexCoord2f( 1.0f, 0.0f );
				glVertex2f( width, 0.0f );
				glTexCoord2f( 0.0f, 1.0f );
				glVertex2f( 0.0f, height );
				glTexCoord2f( 1.0f, 1.0f );
				glVertex2f( width, height );
			glEnd( );
		glDisable( GL_TEXTURE_2D );
	
		text->render( );
	glPopMatrix( );
}


MenuItemSet::MenuItemSet( MenuPane * pParent, float pX, float pY, std::string pText, int * pMyState, int pSet ) : 
	MenuItem( pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp" )
{
	this->setText( pText.c_str( ) );
	this->setLeft( );

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

void MenuItemSet::render( )
{
	glColor3f( 1.0f, 1.0f, 1.0f );

	glActiveTexture( GL_TEXTURE0 );

	if( !clicked )
		glBindTexture( GL_TEXTURE_2D, texture );
	else
		glBindTexture( GL_TEXTURE_2D, textureDown );

	glPushMatrix( );
		glTranslatef( x, y, 0.0f );

		glEnable( GL_TEXTURE_2D );
			glBegin( GL_TRIANGLE_STRIP );
				glTexCoord2f( 0.0f, 0.0f );
				glVertex2f( 0.0f, 0.0f );
				glTexCoord2f( 1.0f, 0.0f );
				glVertex2f( width, 0.0f );
				glTexCoord2f( 0.0f, 1.0f );
				glVertex2f( 0.0f, height );
				glTexCoord2f( 1.0f, 1.0f );
				glVertex2f( width, height );
			glEnd( );
		glDisable( GL_TEXTURE_2D );
	
		text->render( );
	glPopMatrix( );
}


MenuItemSeperator::MenuItemSeperator( MenuPane * pParent, float pX, float pY, std::string pText ) : 
	MenuItem( pParent, pX, pY, 20.0f, pText, "Interface\\BUTTONS\\UI-SliderBar-Background.blp", "Interface\\BUTTONS\\UI-SliderBar-Background.blp" )
{
	this->setText( pText.c_str( ) );
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

void MenuPane::AddMenuItemButton( std::string pName, void ( *pClickFunc )( frame *, int ), int pClickFuncID )
{
	this->addChild( reinterpret_cast<frame*>( new MenuItemButton( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pClickFunc, pClickFuncID ) ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemToggle( std::string pName, bool * pMyState, bool pInvert )
{
	this->addChild( reinterpret_cast<frame*>( new MenuItemToggle( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert ) ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemSwitch( std::string pName, bool * pMyState, bool pInvert )
{
	this->addChild( reinterpret_cast<frame*>( new MenuItemSwitch( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert ) ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemSet( std::string pName, int * pMyState, int pSet )
{
	this->addChild( reinterpret_cast<frame*>( new MenuItemSet( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pSet ) ) );
	this->height = 6.0f + mNumItems * 25.0f;
}
void MenuPane::AddMenuItemSeperator( std::string pName )
{
	this->addChild( reinterpret_cast<frame*>( new MenuItemSeperator( this, 5.0f, 5.0f + 25.0f * mNumItems++, pName ) ) );
	this->height = 6.0f + mNumItems * 25.0f;
}