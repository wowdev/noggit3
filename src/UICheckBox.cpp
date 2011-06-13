#include "UICheckBox.h"

#include <string>

#include "Noggit.h" // arialn13
#include "UIText.h"
#include "UITexture.h"
#include "UIToggleGroup.h"

UICheckBox::UICheckBox( float xPos, float yPos, const std::string& pText )
{
  x = xPos;
  y = yPos;
  width = 30.0f;
  height = 30.0f;
  checked = false;

  addChild( new UITexture( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp" ) );
  check = new UITexture( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp" );
  check->hidden = true;
  addChild( check );
  text = new UIText( 32.0f, 8.0f, pText, arialn13, eJustifyLeft );
  addChild( text );
  
  clickFunc = 0;
  mToggleGroup = 0;
}

UICheckBox::UICheckBox( float xPos, float yPos, const std::string& pText, void (*pClickFunc)(bool,int), int pClickFuncParameter )
{
  x = xPos;
  y = yPos;
  width = 30.0f;
  height = 30.0f;
  checked = false;

  addChild( new UITexture( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp" ) );
  check = new UITexture( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp" );
  check->hidden = true;
  addChild( check );
  text = new UIText( 32.0f, 8.0f, pText, arialn13, eJustifyLeft );
  addChild( text );
  
  clickFunc = pClickFunc;
  id = pClickFuncParameter;
  mToggleGroup = 0;
}

UICheckBox::UICheckBox( float pX, float pY, const std::string& pText, UIToggleGroup * pToggleGroup, int pToggleID )
{
  x = pX;
  y = pY;
  width = 30.0f;
  height = 30.0f;
  checked = false;

  addChild( new UITexture( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp" ) );
  check = new UITexture( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp" );
  check->hidden = true;
  addChild( check );
  text = new UIText( 32.0f, 8.0f, pText, arialn13, eJustifyLeft );
  addChild( text );
  
  clickFunc = 0;

  mToggleGroup = pToggleGroup;
  mToggleGroup->Add( this, pToggleID );
}

void UICheckBox::SetToggleGroup( UIToggleGroup * pToggleGroup, int pToggleID )
{
  mToggleGroup = pToggleGroup;
  if( mToggleGroup )
    mToggleGroup->Add( this, pToggleID );
  clickFunc = 0;
}

void UICheckBox::setText( const std::string& pText )
{
  text->setText( pText );
}

void UICheckBox::setState( bool c )
{
  checked = c;
  check->hidden= !checked;
}

bool UICheckBox::getState()
{
  return checked;
}

UIFrame* UICheckBox::processLeftClick( float /*mx*/, float /*my*/ )
{
  checked = !checked;
  check->hidden = !checked;

  if( mToggleGroup )
    mToggleGroup->Activate( this );
  else if( clickFunc )
    clickFunc( checked, id );

  return this;
}

void UICheckBox::setClickFunc( void (*f)(bool,int), int i )
{
  id = i;
  clickFunc = f;
  mToggleGroup = 0;
}
