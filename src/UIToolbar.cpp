#include "UIToolbar.h"

#include <string>

#include "Environment.h" // Environment
#include "FreeType.h" // fonts.
#include "Log.h"
#include "MapView.h" // MapView
#include "Noggit.h" // app.getapp.getArialn13()()
#include "UIMapViewGUI.h"
#include "UIMinimizeButton.h"
#include "UIText.h"
#include "UITexture.h"
#include "UIToolbarIcon.h"

UIToolbar::UIToolbar(float xPos, float yPos, UIMapViewGUI *setGui)
: UIWindow( xPos, yPos, 105.0f, 330.0f, "interface\\tooltips\\ui-tooltip-border.blp" )
, mainGui( setGui )
, text( new UIText( 8, 7, "Raise/Lower", app.getArialn13(), eJustifyLeft ) )
, selectedIcon( -1 )
, current_texture( new UITexture( 0, 0, 92.0f, 92.0f, "tileset\\generic\\black.blp" ) )
{
  movable( true );
  addChild( text );
  addChild( new UIMinimizeButton( width() ) );

  // Ground edit
  SetIcon( 0, "Interface\\ICONS\\INV_Elemental_Mote_Earth01.blp" );
  // Flat/blur
  SetIcon( 1, "Interface\\ICONS\\INV_Elemental_Mote_Air01.blp" );
  // 3D Paint
  SetIcon( 2, "Interface\\ICONS\\INV_Feather_16.blp" );
  // Holes
  SetIcon( 3, "Interface\\ICONS\\INV_Gizmo_HardenedAdamantiteTube.blp" );
  // AreaID Editor
  SetIcon( 4, "Interface\\ICONS\\INV_Misc_Map07.blp" );
  // Impassible
  SetIcon( 5, "Interface\\ICONS\\INV_Misc_Net_01.blp" );
  // Water editor
  SetIcon( 6, "Interface\\ICONS\\INV_Elemental_Primal_Water.blp" );
  // Light editor
  SetIcon( 7, "Interface\\ICONS\\INV_Enchant_ShardBrilliantSmall.blp" );
  
  
    IconSelect( 0 );

  UIWindow* texture_border = new UIWindow( 5, 230, 95.0f, 95.0f );
  texture_border->addChild( current_texture );
  addChild( texture_border );
}

void UIToolbar::SetIcon( int pIcon, const std::string& pIconFile )
{
  mToolbarIcons[pIcon] = new UIToolbarIcon( ( pIcon % 2 ) * 50.0f + 5.0f, ( pIcon / 2 ) * 50.0f + 30.0f, pIconFile, std::string( "Interface\\BUTTONS\\CheckButtonGlow.blp" ), pIcon, UIEventConstructorArgument(UIToolbarIcon, this, UIToolbar::IconSelect) );
  addChild( mToolbarIcons[pIcon] );
}

// MapView.cpp
void change_settings_window(int oldid, int newid);
extern int terrainMode;

void UIToolbar::IconSelect( int pIcon )
{
  change_settings_window( selectedIcon, pIcon + 1 > 7 ? 0 : pIcon + 1);

  const char * Names[] = { "Raise / Lower", "Flatten / Blur", "3D Paint", "Holes", "AreaID Paint", "Impassible Flag", "Water edit", "Light edit" };
  text->setText( Names[pIcon] );

  terrainMode = pIcon;

  Environment::getInstance()->view_holelines = ( pIcon == 3 );

  for( int j = 0; j < 8; j++ )
    if( mToolbarIcons[j] )
      mToolbarIcons[j]->selected = false;

  if( !mToolbarIcons[pIcon] )
    return;

  selectedIcon = pIcon;
  mToolbarIcons[pIcon]->selected = true;
}
