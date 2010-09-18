#include <algorithm>
#include <map>
#include <list>

#include "noggit.h"
#include "log.h"
#include "video.h"
#include "MapChunk.h"
#include "ui.h"
#include "dbc.h"
#include "mpq.h"

//! \todo  Get this whole thing in a seperate class.

//Texture Files
std::vector<std::string> textureNames;
std::vector<std::string> tilesetNames;
std::vector<std::string> tilesetDirectories;
std::vector<std::string> gFilters;
std::vector<std::string> filterNames;
std::vector<GLuint> gTexturesInList;

//Texture Palette Window
closeWindowUI	*windowTexturePalette;
textureUI	*curTextures[64];
GLuint gTexturesInPage[64];
ManagedItem	*selectedTexture;

//Selected Texture Window
closeWindowUI	*windowSelectedTexture;
textureUI	*textureSelected;
textUI		*textSelectedTexture;

//Texture Loading Window
closeWindowUI	*windowTilesetLoader;

//Texture Filter Window
closeWindowUI	*windowTextureFilter;
bool	tilesetFilter[200];

//Map Chunk Window
closeWindowUI	*windowMapChunk;
textUI			*chunkLocation;
textUI			*chunkAreaID;
textUI			*chunkFlags;
checkboxUI		*chunkFlagChecks[5];
textUI			*chunkEffectID;
textUI			*chunkNumEffects;
textUI			*chunkEffectModels[4];
textureUI		*chunkTexture[4];
textUI			*chunkTextureNames[4];
textUI			*chunkTextureFlags[4];
textUI			*chunkTextureEffectID[4];

//! \todo  Get this via singleton.
Gui				*textGui;

int pal_rows;
int pal_cols;

extern std::list<std::string> gListfile;


std::map<int,std::string> gFilenameFilters;
std::vector<std::string> gActiveFilenameFilters;

//! \todo  Maybe get this out?
bool gFilenameFiltersInited = false;

void LoadTextureNames()
{
	if( textureNames.size() != 0 )
		return;

	std::string	tString;

	bool tilesetsfound = false;
	for( std::list<std::string>::iterator it = gListfile.begin(); it != gListfile.end(); ++it )
	{
		if( it->find( "tileset" ) != std::string::npos )
		{
			tilesetsfound = true;
			if( it->find( "_s.blp" ) == std::string::npos )
				textureNames.push_back( *it );
		}
		else
		{
			if( tilesetsfound )
				break;					// we don't need the rest of this vector as it is sorted.
		}
	}

	//! \todo  Add real names again somehow.

	for( std::vector<std::string>::iterator it = textureNames.begin(); it != textureNames.end(); ++it )
	{
		std::string tString = it->substr( it->find_first_of( "\\/" ) + 1, it->find_last_of( "\\/" ) - it->find_first_of( "\\/" ) - 1 );
		tilesetDirectories.push_back( tString );
		tilesetNames.push_back( tString );
	}

	std::vector<std::string> temp;

	std::sort( tilesetDirectories.begin(), tilesetDirectories.end() );
	std::sort( tilesetNames.begin(), tilesetNames.end() );
	std::vector<std::string>::iterator it = std::unique( tilesetDirectories.begin(), tilesetDirectories.end() );
	tilesetDirectories.resize( it - tilesetDirectories.begin() );
	it = std::unique( tilesetNames.begin(), tilesetNames.end() );
	tilesetNames.resize( it - tilesetNames.begin() );
}

int checkTileset(const char*Texture)
{
	std::vector<std::string>::iterator	i;
	int j=0;
	for(i=tilesetDirectories.begin();i!=tilesetDirectories.end();i++)
	{
		if(strstr(Texture,i->c_str())!=NULL)
			return j;
		j++;
	}
	return -1;
}

inline bool TextureInPalette( std::string pFName )
{
	//! \todo  Fix this.

	if( pFName.find( "tileset" ) == std::string::npos )
		return false;

	int i = 0;
	if( gActiveFilenameFilters.size( ) )
	{
		for( std::vector<std::string>::iterator lFilter = gActiveFilenameFilters.begin( ); lFilter != gActiveFilenameFilters.end( ); lFilter++ )
		{
			if( pFName.find( *lFilter ) != std::string::npos )
			{
				return true;
			}
		}
		return false;
	}
	
	i = 0;
	
	//! \todo  Do this via a list too. 
	for( std::vector<std::string>::iterator lFilter = tilesetDirectories.begin( ); lFilter != tilesetDirectories.end( ); lFilter++ )
	{
		if( tilesetFilter[i++] )
		{
			if( pFName.find( *lFilter ) == std::string::npos )
				return false;
		}
	}
	
	return true;
}

int gCurrentPage;

std::vector<GLuint>::iterator getPage( int pPage )
{
	if( pPage == 0 )
		return gTexturesInList.begin( );

	unsigned int lOnePageSize = pal_cols * pal_rows;

	if( gTexturesInList.size( ) < pPage * lOnePageSize )
		return gTexturesInList.begin( );

	std::vector<GLuint>::iterator it = gTexturesInList.begin( );
	it += lOnePageSize * pPage;
	return it;
}

void showPage( int pPage )
{
	GLuint lSelectedTexture = 0;

	if( selectedTexture )
		for( std::map<GLuint, ManagedItem*>::iterator i = video.textures.items.begin( ); i != video.textures.items.end( ); i++ )
			if( i->second->name == selectedTexture->name )
				lSelectedTexture = i->first;

	int i = 0;
	for( std::vector<GLuint>::iterator lPageStart = getPage( pPage ); lPageStart != gTexturesInList.end( ); lPageStart++ )
	{
		curTextures[i]->hidden = false;
		curTextures[i]->setTexture( *lPageStart );
		gTexturesInPage[i] = *lPageStart;
		curTextures[i]->setHighlight( *lPageStart == lSelectedTexture );

		if( ++i >= ( pal_cols * pal_rows ) )
			return;
	}

	while( i < ( pal_cols * pal_rows ) )
	{
		curTextures[i]->hidden = true;
		i++;
	}
}

void updateTextures()
{
	for( std::map<GLuint, ManagedItem*>::iterator t = video.textures.items.begin( ); t != video.textures.items.end( ); t++ )
		if( TextureInPalette( t->second->name ) )
			gTexturesInList.push_back( t->first );

	showPage( gCurrentPage );
}

void changePage( frame*, int direction )
{
	if (gTexturesInList.size( ) / ( pal_cols * pal_rows )>1)
	{
		gCurrentPage += direction;
		gCurrentPage = gCurrentPage % ( gTexturesInList.size( ) / ( pal_cols * pal_rows ) );
		showPage( gCurrentPage );
	}
}

void updatedSelectedTexture( const ManagedItem* T )
{
	if( !T )
		return;
	textureSelected->setTexture( video.textures.add( T->name.c_str( ) ) );
	textSelectedTexture->setText( T->name );
	textGui->guiToolbar->current_texture->setTexture( video.textures.add( T->name.c_str( ) ) );
}

void texturePaletteClick( frame *f, int id )
{
	if( curTextures[id]->hidden )
		return;
	
	selectedTexture = video.textures.items.find( gTexturesInPage[id] )->second;

	if( textureSelected )
		updatedSelectedTexture( selectedTexture );
	for( int i = 0; i < ( pal_cols * pal_rows ); i++ )
		curTextures[i]->setHighlight( i == id );
}

void showTextureLoader( frame *button, int id )
{
	windowTilesetLoader->hidden = !windowTilesetLoader->hidden;
}

void showTextureFilter( frame *button, int id )
{
	windowTextureFilter->hidden = !windowTextureFilter->hidden;
}

//! \todo  Make this cleaner.
frame *CreateTexturePalette( int rows, int cols, Gui *setgui )
{
	gCurrentPage = 0;

	textGui = setgui;
	pal_rows = rows;
	pal_cols = cols;
	windowTexturePalette=new closeWindowUI(115.0f,38.0f,(pal_rows * 68.0f) + 10.0f, ( pal_cols * 68.0f) + 50.0f,"Texture Palette");
	windowTexturePalette->movable=true;

	for(int i=0;i<(pal_cols*pal_rows);i++)
	{
		curTextures[i]=new textureUI(8.0f+(i%pal_rows)*68.0f,22.0f+(i/pal_rows)*68.0f,64.0f,64.0f,video.textures.add("tileset\\generic\\black.blp"));
		curTextures[i]->setClickFunc(texturePaletteClick,i);
		windowTexturePalette->addChild(curTextures[i]);
	}
	
	updateTextures();
	texturePaletteClick(0,0);

	float DistFromMiddle=54.0f;
	buttonUI	*B1;
	B1=new buttonUI(284.0f/2.0f+DistFromMiddle,2.0f,20.0f,20.0f,video.textures.add("Interface\\Buttons\\UI-SpellbookIcon-NextPage-Up.blp"),video.textures.add("Interface//Buttons//UI-SpellbookIcon-NextPage-Down.blp"));
	B1->setClickFunc( changePage, +1 );
	windowTexturePalette->addChild(B1);

	B1=new buttonUI(284.0f/2.0f-DistFromMiddle-20.0f,2.0f,20.0f,20.0f,video.textures.add("Interface\\Buttons\\UI-SpellbookIcon-PrevPage-Up.blp"),video.textures.add("Interface//Buttons//UI-SpellbookIcon-PrevPage-Down.blp"));
	B1->setClickFunc( changePage, -1 );
	windowTexturePalette->addChild(B1);

	B1=new buttonUI(145.0f,windowTexturePalette->height-24.0f,132.0f,28.0f,video.textures.add("Interface\\Buttons\\UI-DialogBox-Button-Up.blp"),video.textures.add("Interface//Buttons//UI-DialogBox-Button-Down.blp"));
	B1->setText("Load Textures");
	B1->setClickFunc(showTextureLoader,0);
	windowTexturePalette->addChild(B1);

	B1=new buttonUI(7.0f,windowTexturePalette->height-24.0f,132.0f,28.0f,video.textures.add("Interface\\Buttons\\UI-DialogBox-Button-Up.blp"),video.textures.add("Interface//Buttons//UI-DialogBox-Button-Down.blp"));
	B1->setText("Filter Textures");
	B1->setClickFunc(showTextureFilter,0);
	windowTexturePalette->addChild(B1);

	return windowTexturePalette;
}

frame *CreateSelectedTexture()
{
	windowSelectedTexture = new closeWindowUI(video.xres-148.0f-128.0f,video.yres-320.0f,274.0f,288.0f,"Current Texture");
	windowSelectedTexture->movable = true;

	std::string lTexture = selectedTexture ? selectedTexture->name : "tileset\\generic\\black.blp";

	textureSelected = new textureUI( 9.0f, 24.0f, 256.0f, 256.0f, lTexture );
	windowSelectedTexture->addChild( textureSelected );

	textSelectedTexture = new textUI( 137.0f, 264.0f, lTexture, &arialn13, eJustifyCenter );
	textSelectedTexture->setBackground(0.0f,0.0f,0.0f,0.5f);
	windowSelectedTexture->addChild( textSelectedTexture );

	return windowSelectedTexture;
}

void LoadTileset(frame *button,int id)
{
	std::vector<std::string>::iterator i;
	for(i=textureNames.begin();i!=textureNames.end();i++)
	{
		if(checkTileset(i->c_str())==id)
			video.textures.add(*i);
	}
	updateTextures();
}


frame *CreateTilesetLoader()
{	
	LoadTextureNames();

	int columns = tilesetNames.size() / 4;
	if( tilesetNames.size() % 4 != 0 )
		columns++;

	buttonUI * name;
	windowTilesetLoader = new closeWindowUI(
		video.xres / 2.0f - 308.0f,
		video.yres / 2.0f - 139.0f,
		616.0f,
		22.0f + 21.0f * columns + 5.0f,
		"Tileset Loading" );
	windowTilesetLoader->movable=true;

	for( unsigned int i = 0; i < tilesetNames.size(); i++ )
	{
		name = new buttonUI(
			5.0f + 152.0f * ( i / columns ),
			23.0f + 21.0f * ( i % columns ),
			150.0f,
			28.0f,
			video.textures.add( "Interface\\Buttons\\UI-DialogBox-Button-Up.blp" ),
			video.textures.add( "Interface\\Buttons\\UI-DialogBox-Button-Down.blp" )
		);
		name->setText( tilesetNames[i] );
		name->setClickFunc( LoadTileset, i );
		windowTilesetLoader->addChild( name );
	}
	windowTilesetLoader->hidden = true;

	return windowTilesetLoader;
}

void InitFilenameFilterList( )
{
	if( gFilenameFiltersInited )
		return;

	gFilenameFiltersInited = true;

	gFilenameFilters.insert( std::pair<int,std::string>( 0, "Base" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 1, "Brick" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 2, "Brush" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 3, "Cobblestone" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 4, "Cracked" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 5, "Creep" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 6, "Crop" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 7, "Dirt" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 8, "Fern" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 9, "Flower" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 10, "Footprints" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 11, "Grass" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 12, "Ice" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 13, "Leaf" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 14, "Mud" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 15, "Moss" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 16, "Road" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 17, "Rock" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 18, "Root" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 19, "Rubble" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 20, "Sand" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 21, "Shore" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 22, "Straw" ) );
	gFilenameFilters.insert( std::pair<int,std::string>( 23, "Weed" ) );
}

void clickFilterTexture(bool value,int id)
{
	tilesetFilter[id]=value;
	updateTextures();
}

void clickFileFilterTexture(bool value,int id)
{
	if( value )
	{
		std::transform( gFilenameFilters[id].begin(), gFilenameFilters[id].end(), gFilenameFilters[id].begin(), ::tolower );
		gActiveFilenameFilters.push_back( gFilenameFilters[id] );
	}
	else
		for( std::vector<std::string>::iterator it = gActiveFilenameFilters.begin( ); it != gActiveFilenameFilters.end( ); it++ )
			if( *it == gFilenameFilters[id] )
			{
				gActiveFilenameFilters.erase( it );
				break;
			}

	updateTextures( );
}

frame *CreateTextureFilter()
{	
	InitFilenameFilterList( );
  
	LoadTextureNames( );
	windowTextureFilter = new closeWindowUI( video.xres / 2.0f - 308.0f, video.yres / 2.0f - 314.5f, 616.0f, 630.0f, "Texture Filtering" );
	windowTextureFilter->movable = true;
	windowTextureFilter->hidden = true;

	//Filename Filters
	windowTextureFilter->addChild( new textUI( 308.0f, 26.0f, "Filename Filters", &arial14, eJustifyCenter ) );

	for( std::map<int,std::string>::iterator it = gFilenameFilters.begin( ); it != gFilenameFilters.end( ); it++ )
		windowTextureFilter->addChild( new checkboxUI( 5.0f + 152.0f * ( it->first / 6 ), 43.0f + 30.0f * ( it->first % 6 ), it->second, clickFileFilterTexture, it->first ) );

	windowTextureFilter->addChild( new checkboxUI( 240.0f, 43.0f + 30.0f * 6.0f, "Misc (Everything Else)", clickFileFilterTexture, 24 ) );

	//Tileset Filters
	windowTextureFilter->addChild( new textUI( 308.0f, 254.0f, "Tileset Filters", &arial14, eJustifyCenter ) );

	for( unsigned int i = 0; i < tilesetNames.size(); i++ )
		windowTextureFilter->addChild( new checkboxUI( 5.0f + 152.0f * ( i / 12 ), 267.0f + 30.0f * ( i % 12 ), tilesetNames[i], clickFilterTexture, i ) );

	return windowTextureFilter;
}

frame *createMapChunkWindow()
{
	window *chunkSettingsWindow,*chunkTextureWindow,*chunkEffectWindow;
	windowMapChunk=new closeWindowUI(video.xres/2.0f-316.0f,video.yres-369.0f,634.0f,337.0f,"Map Chunk Settings");
	windowMapChunk->movable=true;
	
	chunkSettingsWindow=new window(11.0f,26.0f,300.0f,300.0f);
	windowMapChunk->addChild(chunkSettingsWindow);

	chunkLocation=new textUI(5.0f,4.0f,"Chunk x, y of Tile x, y at (x, y, z)", &arial14, eJustifyLeft);
	chunkSettingsWindow->addChild(chunkLocation);

	chunkAreaID=new textUI(5.0,chunkLocation->y+25.0f,"AreaID:", &arial14, eJustifyLeft);
	chunkSettingsWindow->addChild(chunkAreaID);

	chunkFlags=new textUI(5.0,chunkAreaID->y+25.0f,"Flags:", &arial14, eJustifyLeft);
	chunkSettingsWindow->addChild(chunkFlags);


	chunkFlagChecks[0]=new checkboxUI(6,chunkFlags->y+22.0f,"Shadow");
	chunkSettingsWindow->addChild(chunkFlagChecks[0]);


	chunkFlagChecks[1]=new checkboxUI(150,chunkFlags->y+22.0f,"Impassible");
	chunkSettingsWindow->addChild(chunkFlagChecks[1]);

	chunkFlagChecks[2]=new checkboxUI(chunkFlagChecks[0]->x,chunkFlagChecks[0]->y+30.0f,"River");
	chunkSettingsWindow->addChild(chunkFlagChecks[2]);

	chunkFlagChecks[3]=new checkboxUI(chunkFlagChecks[1]->x,chunkFlagChecks[1]->y+30.0f,"Ocean");
	chunkSettingsWindow->addChild(chunkFlagChecks[3]);

	chunkFlagChecks[4]=new checkboxUI(chunkFlagChecks[2]->x,chunkFlagChecks[2]->y+30.0f,"Magma");
	chunkSettingsWindow->addChild(chunkFlagChecks[4]);


	chunkEffectID=new textUI(5.0f,chunkFlagChecks[4]->y+35.0f,"EffectID:", &arial14, eJustifyLeft);
	chunkSettingsWindow->addChild(chunkEffectID);
	chunkEffectID->hidden=true;
	chunkNumEffects=new textUI(150.0f,chunkEffectID->y,"Num Effects:", &arial14, eJustifyLeft);
	chunkSettingsWindow->addChild(chunkNumEffects);
	chunkNumEffects->hidden=true;

	chunkEffectWindow=new window(8.0f,chunkEffectID->y+23.0f,284.0f,300.0f-(chunkEffectID->y+23.0f+8.0f));
	chunkSettingsWindow->addChild(chunkEffectWindow);
	chunkEffectWindow->hidden=true;
	
	chunkEffectModels[0]=new textUI(8.0f,8.0f,"Effect Doodad", &arial14, eJustifyLeft);
	chunkEffectWindow->addChild(chunkEffectModels[0]);
	chunkEffectModels[0]->hidden=true;
	
	chunkTextureWindow=new window(324.0f,26.0f,300.0f,300.0f);
	windowMapChunk->addChild(chunkTextureWindow);
	
	float yPos = 11.0f;

    for(int i=1;i<4;i++)
	{
		chunkEffectModels[i]=new textUI(8.0f,chunkEffectModels[i-1]->y+20.0f,"Effect Doodad", &arial14, eJustifyLeft);
		chunkEffectWindow->addChild(chunkEffectModels[i]);
		chunkEffectModels[i]->hidden=true;

		chunkTexture[i]=new textureUI( 10.0f, yPos, 64.0f, 64.0f, "tileset\\generic\\black.blp" );
		chunkTextureWindow->addChild(chunkTexture[i]);
		
		chunkTextureNames[i]=new textUI(83.0f,yPos+5.0f,"Texture Name", &arial14, eJustifyLeft);
		chunkTextureWindow->addChild(chunkTextureNames[i]);

		chunkTextureFlags[i]=new textUI(83.0f,yPos+30.0f,"Flags -", &arial14, eJustifyLeft);
		chunkTextureWindow->addChild(chunkTextureFlags[i]);

		chunkTextureEffectID[i]=new textUI(184.0f,yPos+30.0f,"EffectID -", &arial14, eJustifyLeft);
		chunkTextureWindow->addChild(chunkTextureEffectID[i]);

		yPos+=64.0f+8.0f;
	}

	return windowMapChunk;
}

void setChunkWindow(MapChunk *chunk)
{
	char Temp[255];

	sprintf(Temp,"Chunk %d, %d at (%.2f, %.2f, %.2f)",chunk->px,chunk->py,chunk->xbase,chunk->ybase,chunk->zbase);
	chunkLocation->setText(Temp);///

	
	std::string areaName;
	try
	{
		AreaDB::Record rec = gAreaDB.getByID(chunk->areaID);
		areaName = rec.getString(AreaDB::Name);
	}
	catch(...)
	{
		areaName = "";
	}
	sprintf(Temp,"AreaID: %s (%d)",areaName.c_str(),chunk->areaID);
	chunkAreaID->setText(Temp);///
	
	sprintf(Temp,"Flags: %d",chunk->Flags);
	chunkFlags->setText(Temp);///

	for(int ch=0;ch<5;ch++)
		chunkFlagChecks[ch]->setState(false);


	if(chunk->Flags&FLAG_SHADOW)
		chunkFlagChecks[0]->setState(true);
	if(chunk->Flags&FLAG_IMPASS)
		chunkFlagChecks[1]->setState(true);
	if(chunk->Flags&FLAG_LQ_RIVER)
		chunkFlagChecks[2]->setState(true);
	if(chunk->Flags&FLAG_LQ_OCEAN)
		chunkFlagChecks[3]->setState(true);
	if(chunk->Flags&FLAG_LQ_MAGMA)
		chunkFlagChecks[4]->setState(true);

	//sprintf(Temp,"EffectID: %d",chunk->header.effectId);
	//chunkEffectID->setText(Temp);///

	sprintf(Temp,"Num Effects: %d",chunk->header.nEffectDoodad);
	chunkNumEffects->setText(Temp);///
    
	int pl=0;
	for(pl=0;pl<(chunk->nTextures);pl++)
	{
    chunkTexture[pl]->hidden=false;
	chunkTextureNames[pl]->hidden=false;
    chunkTextureFlags[pl]->hidden=false;
	chunkTextureEffectID[pl]->hidden=false;

    sprintf(Temp,"Flags- %d",chunk->texFlags[pl]);
	chunkTextureFlags[pl]->setText(Temp);

	sprintf(Temp,"EffectID- %d",chunk->effectID[pl]);
	chunkTextureEffectID[pl]->setText(Temp);

	chunkTexture[pl]->setTexture(video.textures.add(video.textures.items[chunk->textures[pl]]->name));

	chunkTextureNames[pl]->setText(video.textures.items[chunk->textures[pl]]->name);
	}
	for(;pl<4;pl++)
	{
		chunkTexture[pl]->hidden=true;
	    chunkTextureNames[pl]->hidden=true;
		chunkTextureFlags[pl]->hidden=true;
		chunkTextureEffectID[pl]->hidden=true;
	}
}

