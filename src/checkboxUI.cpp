#include "checkboxUI.h"

#include <string>

#include "noggit.h" // arialn13
#include "textUI.h"
#include "textureUI.h"
#include "ToggleGroup.h"

checkboxUI::checkboxUI( float xPos, float yPos, const std::string& pText )
{
  x = xPos;
  y = yPos;
  width = 30.0f;
  height = 30.0f;
  checked = false;

  addChild( new textureUI( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp" ) );
  check = new textureUI( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp" );
  check->hidden = true;
  addChild( check );
  text = new textUI( 32.0f, 8.0f, pText, arialn13, eJustifyLeft );
  addChild( text );
  
  clickFunc = 0;
  mToggleGroup = 0;
}

checkboxUI::checkboxUI( float xPos, float yPos, const std::string& pText, void (*pClickFunc)(bool,int), int pClickFuncParameter )
{
  x = xPos;
  y = yPos;
  width = 30.0f;
  height = 30.0f;
  checked = false;

  addChild( new textureUI( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp" ) );
  check = new textureUI( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp" );
  check->hidden = true;
  addChild( check );
  text = new textUI( 32.0f, 8.0f, pText, arialn13, eJustifyLeft );
  addChild( text );
  
  clickFunc = pClickFunc;
  id = pClickFuncParameter;
  mToggleGroup = 0;
}

checkboxUI::checkboxUI( float pX, float pY, const std::string& pText, ToggleGroup * pToggleGroup, int pToggleID )
{
  x = pX;
  y = pY;
  width = 30.0f;
  height = 30.0f;
  checked = false;

  addChild( new textureUI( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Up.blp" ) );
  check = new textureUI( 0.0f, 0.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-CheckBox-Check.blp" );
  check->hidden = true;
  addChild( check );
  text = new textUI( 32.0f, 8.0f, pText, arialn13, eJustifyLeft );
  addChild( text );
  
  clickFunc = 0;

  mToggleGroup = pToggleGroup;
  mToggleGroup->Add( this, pToggleID );
}

void checkboxUI::SetToggleGroup( ToggleGroup * pToggleGroup, int pToggleID )
{
  mToggleGroup = pToggleGroup;
  if( mToggleGroup )
    mToggleGroup->Add( this, pToggleID );
  clickFunc = 0;
}

void checkboxUI::setText( const std::string& pText )
{
  text->setText( pText );
}

void checkboxUI::setState( bool c )
{
  checked = c;
  check->hidden= !checked;
}

bool checkboxUI::getState()
{
  return checked;
}

frame *checkboxUI::processLeftClick( float /*mx*/, float /*my*/ )
{
  checked = !checked;
  check->hidden = !checked;

  if( mToggleGroup )
    mToggleGroup->Activate( this );
  else if( clickFunc )
    clickFunc( checked, id );

  return this;
}

void checkboxUI::setClickFunc( void (*f)(bool,int), int i )
{
  id = i;
  clickFunc = f;
  mToggleGroup = 0;
}
