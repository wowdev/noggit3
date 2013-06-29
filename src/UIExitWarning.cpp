#include "UIExitWarning.h"

#include <algorithm>

#include "Noggit.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UIButton.h"
#include "UITexture.h"
#include "MapView.h"
#include "Video.h" // video

#include "Log.h"


void exitMeNow(UIFrame *f,int /*set*/)
{
  ( static_cast<UIExitWarning *>( f->parent() ) )->exitNow();
}

void closeme(UIFrame *f,int /*set*/)
{
  ( static_cast<UIExitWarning *>( f->parent() ) )->hide();
}

UIExitWarning::UIExitWarning( MapView *mapview )
  : UICloseWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f - ( video.yres() /4), winWidth, winHeight, "" )
{
  addChild( new UITexture( 10.0f, 10.0f, 64.0f, 64.0f, "Interface\\ICONS\\INV_Misc_QuestionMark.blp" ) );
  addChild( new UIText( 95.0f, 20.0f, "Do you really want to exit?", app.getArial14(), eJustifyLeft ) );
  addChild( new UIText( 95.0f, 40.0f, "Changes will be lost!", app.getArial14(), eJustifyLeft ) );
  addChild( new UIButton( this->width() - 225.0f, 80.0f, 100.0f, 30.0f, "OK", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", exitMeNow, 1 ) );
  addChild( new UIButton( this->width() - 120.0f, 80.0f, 100.0f, 30.0f, "Cancel", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", closeme, 2 ) );

}

void UIExitWarning::resize()
{
  x( std::max( ( video.xres() / 2.0f ) - ( winWidth / 2.0f ), 0.0f ) );
  y( std::max( ( video.yres() / 2.0f ) - ( winHeight / 2.0f ), 0.0f )  - ( video.yres() /4) );
}

void UIExitWarning::exitNow()
{
  _MapView->quit();
}


