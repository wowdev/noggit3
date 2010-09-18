#include "Toolbar.h"
#include "noggit.h"
#include "MinimizeButton.h"

Toolbar::Toolbar(float xPos, float yPos, float w, float h, Gui *setGui) : window(xPos, yPos, w, h), mainGui( setGui )
{
	this->movable = true;
	texture = video.textures.add("interface\\tooltips\\ui-tooltip-border.blp");

	// set title
	text = new textUI( 8, 7, "Raise/Lower", &arialn13, eJustifyLeft );
	addChild( text );

	//close button
	this->addChild( reinterpret_cast<frame*>( new MinimizeButton( w, this ) ) );

	// ground edit
	SetIcon( 0, "Interface\\ICONS\\INV_Elemental_Mote_Earth01.blp" );
	// Flat/blur
	SetIcon( 1, "Interface\\ICONS\\INV_Elemental_Mote_Air01.blp" );
	//3D Paint
	SetIcon( 2, "Interface\\ICONS\\INV_Feather_16.blp" );
	//Holes
	SetIcon( 3, "Interface\\ICONS\\INV_Gizmo_HardenedAdamantiteTube.blp" );
	SetIcon( 4, "Interface\\ICONS\\INV_Elemental_Mote_Fire01.blp" );
	SetIcon( 5, "Interface\\ICONS\\INV_Enchant_EssenceAstralLarge.blp" );
	SetIcon( 6, "Interface\\ICONS\\INV_Misc_Flower_02.blp" );
	SetIcon( 7, "Interface\\Icons\\INV_Misc_Map07.blp" );
	SetIcon( 8, "Interface\\Icons\\Spell_Shaman_ThunderStorm.blp" );
	SetIcon( 9, "Interface\\Icons\\Spell_Shaman_TidalWaves.blp" );

	IconSelect( 0 );

	current_texture = new textureUI( 0, 0, 92.0f, 92.0f, video.textures.add( "tileset\\generic\\black.blp" ) );
	
	window *texture_border = new window( 5, 280, 95.0f, 95.0f );
	texture_border->addChild( current_texture );
	this->addChild( texture_border );
	this->height = 380;

}

void Toolbar::SetIcon( int pIcon, const std::string& pIconFile )
{
	mToolbarIcons[pIcon] = new Icon( 
			( pIcon % 2 ) * 50.0f + 5.0f, 
			( pIcon / 2 ) * 50.0f + 30.0f, 
			45.0f, 
			45.0f, 
			video.textures.add( pIconFile ), 
			video.textures.add( "Interface\\BUTTONS\\CheckButtonGlow.blp" ) 
		);
	this->addChild( mToolbarIcons[pIcon] );	
}

void Toolbar::IconSelect( int pIcon )
{
	for( int j = 0; j < 10; j++ )
		if( this->mToolbarIcons[j] )
			this->mToolbarIcons[j]->selected = false;
	
	if( !this->mToolbarIcons[pIcon] )
		return;
	this->selectedIcon = pIcon;
	this->mToolbarIcons[pIcon]->selected = true;
}
