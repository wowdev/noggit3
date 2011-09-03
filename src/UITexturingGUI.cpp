#include "UITexturingGUI.h"

#include <algorithm>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "DBC.h"
#include "Log.h"
#include "MapChunk.h"
#include "Misc.h"
#include "MPQ.h"
#include "Noggit.h" // arial14, arialn13
#include "TextureManager.h" // TextureManager, Texture
#include "UIButton.h" // UIButton
#include "UICheckBox.h" // UICheckBox
#include "UICloseWindow.h" // UICloseWindow
#include "UIMapViewGUI.h" // UIMapViewGUI
#include "UIText.h" // UIText
#include "UITexture.h" // UITexture
#include "UIToolbar.h" // Toolbar
#include "Video.h"

//! \todo  Get this whole thing in a seperate class.

//! \todo  Get this via singleton.
UIMapViewGUI* textGui;

int pal_rows;
int pal_cols;

//! \todo  Maybe get this out?
bool gFilenameFiltersInited = false;

extern std::list<std::string> gListfile;

std::map<int,std::string> gFilenameFilters;
std::vector<std::string> gActiveFilenameFilters;
std::vector<std::string> gActiveDirectoryFilters;
std::vector<std::string> textureNames;
std::vector<std::string> tilesetDirectories;
std::vector<OpenGL::Texture*> gTexturesInList;

//Texture Palette Window
UICloseWindow  *windowTexturePalette;

UITexture  *curTextures[64];
OpenGL::Texture* gTexturesInPage[64];
UIText *gPageNumber;

//Selected Texture Window
UICloseWindow  *windowSelectedTexture;
UITexture  *textureSelected;
UIText    *textSelectedTexture;

//Texture Loading Window
UICloseWindow  *windowTilesetLoader;

//Texture Filter Window
UICloseWindow  *windowTextureFilter;

//Map Chunk Window
UICloseWindow  *windowMapChunk;
UIText      *chunkLocation;
UIText      *chunkAreaID;
UIText      *chunkFlags;
UICheckBox    *chunkFlagChecks[5];
UIText      *chunkEffectID;
UIText      *chunkNumEffects;
UIText      *chunkEffectModels[4];
UITexture    *chunkTexture[4];
UIText      *chunkTextureNames[4];
UIText      *chunkTextureFlags[4];
UIText      *chunkTextureEffectID[4];

OpenGL::Texture* UITexturingGUI::selectedTexture = NULL;

void LoadTextureNames()
{
  if( textureNames.size() )
  {
    return;
  }

  bool tilesetsfound = false;

  while(!MPQArchive::allFinishedLoading()) MPQArchive::allFinishLoading(); // wait for listfiles.

  for( std::list<std::string>::iterator it = gListfile.begin(); it != gListfile.end(); ++it )
  {
    if( it->find( "tileset" ) != std::string::npos )
    {
      tilesetsfound = true;
      if( it->find( "_s.blp" ) == std::string::npos )
      {
        textureNames.push_back( *it );
      }
    }
    else
    {
      if( tilesetsfound )
      {
        // we don't need the rest of this vector as it is sorted.
        break;
      }
    }
  }

  for( std::vector<std::string>::iterator it = textureNames.begin(); it != textureNames.end(); ++it )
  {
    std::string tString = it->substr( it->find_first_of( "\\/" ) + 1, it->find_last_of( "\\/" ) - it->find_first_of( "\\/" ) - 1 );
    tilesetDirectories.push_back( tString );
  }

  std::sort( tilesetDirectories.begin(), tilesetDirectories.end() );
  tilesetDirectories.resize( std::unique( tilesetDirectories.begin(), tilesetDirectories.end() ) - tilesetDirectories.begin() );
}

bool TextureInPalette( const std::string& pFName )
{
  if( pFName.find( "tileset" ) == std::string::npos )
  {
    return false;
  }

  if( gActiveFilenameFilters.size() )
  {
    for( std::vector<std::string>::iterator lFilter = gActiveFilenameFilters.begin(); lFilter != gActiveFilenameFilters.end(); lFilter++ )
    {
      if( pFName.find( *lFilter ) != std::string::npos )
      {
        return true;
      }
    }
    return false;
  }

  if( gActiveDirectoryFilters.size() )
  {
    for( std::vector<std::string>::iterator lFilter = gActiveDirectoryFilters.begin(); lFilter != gActiveDirectoryFilters.end(); lFilter++ )
    {
      if( pFName.find( *lFilter ) != std::string::npos )
      {
        return true;
      }
    }
    return false;
  }

  return true;
}

int gCurrentPage;

void showPage( int pPage )
{
  OpenGL::Texture* lSelectedTexture = UITexturingGUI::getSelectedTexture();

  if( gPageNumber )
  {
    std::stringstream pagenumber;
    pagenumber << (pPage+1) << " / " << static_cast<int>( gTexturesInList.size() / ( pal_cols * pal_rows ) + 1 );
    gPageNumber->setText( pagenumber.str() );
  }

  int i = 0;
  const unsigned int lIndex = pal_cols * pal_rows * pPage;

  for( std::vector<OpenGL::Texture*>::iterator lPageStart = gTexturesInList.begin() + ( lIndex > gTexturesInList.size() ? 0 : lIndex ); lPageStart != gTexturesInList.end(); lPageStart++ )
  {
    curTextures[i]->show();
    curTextures[i]->setTexture( *lPageStart );
    curTextures[i]->setHighlight( *lPageStart == lSelectedTexture );
    gTexturesInPage[i] = *lPageStart;

    if( ++i >= ( pal_cols * pal_rows ) )
    {
      return;
    }
  }

  while( i < ( pal_cols * pal_rows ) )
  {
    curTextures[i]->hide();
    ++i;
  }
}

void updateTextures()
{
  gTexturesInList.clear();
  gTexturesInList = TextureManager::getAllTexturesMatching( TextureInPalette );
  showPage( gCurrentPage );
}

void changePage( UIFrame*, int direction )
{
  gCurrentPage = std::max( gCurrentPage + direction, 0 );
  gCurrentPage = std::min( gCurrentPage, static_cast<int>( gTexturesInList.size() / ( pal_cols * pal_rows ) ) );
  showPage( gCurrentPage );
}

void UITexturingGUI::updateSelectedTexture()
{
  if( textureSelected )
    textureSelected->setTexture( UITexturingGUI::getSelectedTexture() );
  if( textSelectedTexture )
    textSelectedTexture->setText( UITexturingGUI::getSelectedTexture()->filename() );
  if( textGui )
    textGui->guiToolbar->current_texture->setTexture( UITexturingGUI::getSelectedTexture() );
}

void texturePaletteClick( UIFrame* /*f*/, int id )
{
  if( curTextures[id]->hidden() )
    return;

  UITexturingGUI::setSelectedTexture( gTexturesInPage[id] );

  if( UITexturingGUI::getSelectedTexture() )
  {
    UITexturingGUI::updateSelectedTexture();
  }
  else{
    Log << "Somehow getting the texture failed oO";
  }

  for( int i = 0; i < ( pal_cols * pal_rows ); ++i )
  {
    curTextures[i]->setHighlight( i == id );
  }
}

// --- List stuff ------------------------

void LoadTileset( UIFrame* /*button*/, int id )
{
  for( std::vector<std::string>::iterator it = textureNames.begin(); it != textureNames.end(); ++it )
  {
    if( it->find( tilesetDirectories[id] ) != std::string::npos )
    {
      //! \todo Actually save the texture returned here and do no longer iterate over all cached textures.
      TextureManager::newTexture( *it );
    }
  }
  updateTextures();
}

// --- Filtering stuff ---------------------

void InitFilenameFilterList()
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

// -----------------------------------------------

void showTextureLoader( UIFrame* /*button*/, int /*id*/ )
{
  windowTilesetLoader->toggleVisibility();
}

void showTextureFilter( UIFrame* /*button*/, int /*id*/ )
{
  windowTextureFilter->toggleVisibility();
}

void clickFilterTexture(bool value,int id)
{
  if( value )
  {
    std::transform( tilesetDirectories[id].begin(), tilesetDirectories[id].end(), tilesetDirectories[id].begin(), ::tolower );
    gActiveDirectoryFilters.push_back( tilesetDirectories[id] );
  }
  else
  {
    for( std::vector<std::string>::iterator it = gActiveDirectoryFilters.begin(); it != gActiveDirectoryFilters.end(); ++it )
    {
      if( *it == tilesetDirectories[id] )
      {
        gActiveDirectoryFilters.erase( it );
        break;
      }
    }
  }

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
  {
    for( std::vector<std::string>::iterator it = gActiveFilenameFilters.begin(); it != gActiveFilenameFilters.end(); ++it )
    {
      if( *it == gFilenameFilters[id] )
      {
        gActiveFilenameFilters.erase( it );
        break;
      }
    }
  }

  updateTextures();
}


//! \todo  Make this cleaner.
UIFrame* UITexturingGUI::createTexturePalette( int rows, int cols, UIMapViewGUI *setgui )
{
  gCurrentPage = 0;

  textGui = setgui;
  pal_rows = rows;
  pal_cols = cols;
  windowTexturePalette = new UICloseWindow( 115.0f, 38.0f, ( pal_rows * 68.0f ) + 10.0f, ( pal_cols * 68.0f ) + 50.0f, "Texture Palette", true );

  for(int i=0;i<(pal_cols*pal_rows);++i)
  {
    curTextures[i]=new UITexture(8.0f+(i%pal_rows)*68.0f,22.0f+(i/pal_rows)*68.0f,64.0f,64.0f,"tileset\\generic\\black.blp");
    curTextures[i]->setClickFunc(texturePaletteClick,i);
    windowTexturePalette->addChild(curTextures[i]);
  }

  gPageNumber = NULL;
  textSelectedTexture = NULL;

  updateTextures();
  texturePaletteClick( 0, 0 );

  windowTexturePalette->addChild( gPageNumber = new UIText( 44.0f, 4.0f, "1 / 1", arialn13, eJustifyLeft ) );
  windowTexturePalette->addChild( new UIButton( 20.0f, 2.0f, 20.0f, 20.0f, "", "Interface\\Buttons\\UI-SpellbookIcon-NextPage-Up.blp", "Interface\\Buttons\\UI-SpellbookIcon-NextPage-Down.blp", changePage, +1 ) );
  windowTexturePalette->addChild( new UIButton( 2.0f, 2.0f, 20.0f, 20.0f, "", "Interface\\Buttons\\UI-SpellbookIcon-PrevPage-Up.blp", "Interface\\Buttons\\UI-SpellbookIcon-PrevPage-Down.blp", changePage, -1 ) );

  windowTexturePalette->addChild( new UIButton( 145.0f, windowTexturePalette->height() - 24.0f, 132.0f, 28.0f, "Load Textures", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", showTextureLoader, 0 ) );
  windowTexturePalette->addChild( new UIButton( 7.0f, windowTexturePalette->height() - 24.0f, 132.0f, 28.0f, "Filter Textures", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", showTextureFilter, 0 ) );

  return windowTexturePalette;
}

UIFrame* UITexturingGUI::createSelectedTexture()
{
  windowSelectedTexture = new UICloseWindow( video.xres() - 148.0f - 128.0f, video.yres() - 320.0f, 274.0f, 288.0f, "Current Texture", true );

  std::string lTexture = UITexturingGUI::selectedTexture ? selectedTexture->filename() : "tileset\\generic\\black.blp";

  textureSelected = new UITexture( 9.0f, 24.0f, 256.0f, 256.0f, lTexture );
  windowSelectedTexture->addChild( textureSelected );

  textSelectedTexture = new UIText( 137.0f, 264.0f, lTexture, arialn13, eJustifyCenter );
  textSelectedTexture->setBackground( 0.0f, 0.0f, 0.0f, 0.5f );
  windowSelectedTexture->addChild( textSelectedTexture );

  return windowSelectedTexture;
}

UIFrame* UITexturingGUI::createTilesetLoader()
{
  LoadTextureNames();

  int columns = tilesetDirectories.size() / 4;
  if( tilesetDirectories.size() % 4 != 0 )
    columns++;

  UIButton * name;
  windowTilesetLoader = new UICloseWindow(
    video.xres() / 2.0f - 308.0f,
    video.yres() / 2.0f - 139.0f,
    616.0f,
    22.0f + 21.0f * columns + 5.0f,
    "Tileset Loading" );
  windowTilesetLoader->movable( true );

  for( unsigned int i = 0; i < tilesetDirectories.size(); ++i )
  {
    name = new UIButton(
      5.0f + 152.0f * ( i / columns ),
      23.0f + 21.0f * ( i % columns ),
      150.0f,
      28.0f,
      "Interface\\Buttons\\UI-DialogBox-Button-Up.blp",
      "Interface\\Buttons\\UI-DialogBox-Button-Down.blp"
    );

    std::string setname;
    setname = tilesetDirectories[i];
    misc::find_and_replace(setname,"expansion01\\","");
    misc::find_and_replace(setname,"expansion02\\","");

    name->setText( setname );
    name->setClickFunc( LoadTileset, i );
    windowTilesetLoader->addChild( name );
  }
  windowTilesetLoader->hide();

  return windowTilesetLoader;
}

UIFrame* UITexturingGUI::createTextureFilter()
{
  InitFilenameFilterList();

  LoadTextureNames();
  windowTextureFilter = new UICloseWindow( video.xres() / 2.0f - 450.0f, video.yres() / 2.0f - 300.0f, 900.0f, 610.0f, "Texture Filtering", true );
  windowTextureFilter->hide();

  //Filename Filters
  windowTextureFilter->addChild( new UIText( 60.0f, 23.0f, "Filename Filters", arial14, eJustifyCenter ) );

  for( std::map<int,std::string>::iterator it = gFilenameFilters.begin(); it != gFilenameFilters.end(); ++it )
  {
    windowTextureFilter->addChild( new UICheckBox( 5.0f + 152.0f * ( it->first / 4 ), 43.0f + 30.0f * ( it->first % 4 ), it->second, clickFileFilterTexture, it->first ) );
  }

  windowTextureFilter->addChild( new UICheckBox( 350.0f, 45.0f + 30.0f * 4.0f, "Misc (Everything Else)", clickFileFilterTexture, 24 ) );

  //Tileset Filters
  windowTextureFilter->addChild( new UIText( 55.0f, 190.0f, "Tileset Filters", arial14, eJustifyCenter ) );

  for( unsigned int i = 0; i < tilesetDirectories.size(); ++i )
  {
    std::string name;
    name = tilesetDirectories[i];
    misc::find_and_replace(name,"expansion01\\","");
    misc::find_and_replace(name,"expansion02\\","");
    windowTextureFilter->addChild( new UICheckBox( 5.0f + 152.0f * ( i / 13 ), 210.0f + 30.0f * ( i % 13 ), name, clickFilterTexture, i ) );
  }

  return windowTextureFilter;
}

UIFrame* UITexturingGUI::createMapChunkWindow()
{
  UIWindow *chunkSettingsWindow,*chunkTextureWindow,*chunkEffectWindow;
  windowMapChunk=new UICloseWindow(video.xres()/2.0f-316.0f,video.yres()-369.0f,634.0f,337.0f,"Map Chunk Settings");
  windowMapChunk->movable( true );

  chunkSettingsWindow=new UIWindow(11.0f,26.0f,300.0f,300.0f);
  windowMapChunk->addChild(chunkSettingsWindow);

  chunkLocation=new UIText(5.0f,4.0f,"Chunk x, y of Tile x, y at (x, y, z)", arial14, eJustifyLeft);
  chunkSettingsWindow->addChild(chunkLocation);

  chunkAreaID=new UIText(5.0,chunkLocation->y()+25.0f,"AreaID:", arial14, eJustifyLeft);
  chunkSettingsWindow->addChild(chunkAreaID);

  chunkFlags=new UIText(5.0,chunkAreaID->y()+25.0f,"Flags:", arial14, eJustifyLeft);
  chunkSettingsWindow->addChild(chunkFlags);


  chunkFlagChecks[0]=new UICheckBox(6,chunkFlags->y()+22.0f,"Shadow");
  chunkSettingsWindow->addChild(chunkFlagChecks[0]);


  chunkFlagChecks[1]=new UICheckBox(150,chunkFlags->y()+22.0f,"Impassible");
  chunkSettingsWindow->addChild(chunkFlagChecks[1]);

  chunkFlagChecks[2]=new UICheckBox(chunkFlagChecks[0]->x(),chunkFlagChecks[0]->y()+30.0f,"River");
  chunkSettingsWindow->addChild(chunkFlagChecks[2]);

  chunkFlagChecks[3]=new UICheckBox(chunkFlagChecks[1]->x(),chunkFlagChecks[1]->y()+30.0f,"Ocean");
  chunkSettingsWindow->addChild(chunkFlagChecks[3]);

  chunkFlagChecks[4]=new UICheckBox(chunkFlagChecks[2]->x(),chunkFlagChecks[2]->y()+30.0f,"Magma");
  chunkSettingsWindow->addChild(chunkFlagChecks[4]);


  chunkEffectID=new UIText(5.0f,chunkFlagChecks[4]->y()+35.0f,"EffectID:", arial14, eJustifyLeft);
  chunkSettingsWindow->addChild(chunkEffectID);
  chunkEffectID->hide();
  chunkNumEffects=new UIText(150.0f,chunkEffectID->y(),"Num Effects:", arial14, eJustifyLeft);
  chunkSettingsWindow->addChild(chunkNumEffects);
  chunkNumEffects->hide();

  chunkEffectWindow=new UIWindow(8.0f,chunkEffectID->y()+23.0f,284.0f,300.0f-(chunkEffectID->y()+23.0f+8.0f));
  chunkSettingsWindow->addChild(chunkEffectWindow);
  chunkEffectWindow->hide();

  chunkEffectModels[0]=new UIText(8.0f,8.0f,"Effect Doodad", arial14, eJustifyLeft);
  chunkEffectWindow->addChild(chunkEffectModels[0]);
  chunkEffectModels[0]->hide();

  chunkTextureWindow=new UIWindow(324.0f,26.0f,300.0f,300.0f);
  windowMapChunk->addChild(chunkTextureWindow);

  float yPos = 11.0f;

    for(int i=1;i<4;++i)
  {
    chunkEffectModels[i]=new UIText(8.0f,chunkEffectModels[i-1]->y()+20.0f,"Effect Doodad", arial14, eJustifyLeft);
    chunkEffectWindow->addChild(chunkEffectModels[i]);
    chunkEffectModels[i]->hide();

    chunkTexture[i]=new UITexture( 10.0f, yPos, 64.0f, 64.0f, "tileset\\generic\\black.blp" );
    chunkTextureWindow->addChild(chunkTexture[i]);

    chunkTextureNames[i]=new UIText(83.0f,yPos+5.0f,"Texture Name", arial14, eJustifyLeft);
    chunkTextureWindow->addChild(chunkTextureNames[i]);

    chunkTextureFlags[i]=new UIText(83.0f,yPos+30.0f,"Flags -", arial14, eJustifyLeft);
    chunkTextureWindow->addChild(chunkTextureFlags[i]);

    chunkTextureEffectID[i]=new UIText(184.0f,yPos+30.0f,"EffectID -", arial14, eJustifyLeft);
    chunkTextureWindow->addChild(chunkTextureEffectID[i]);

    yPos+=64.0f+8.0f;
  }

  return windowMapChunk;
}

void UITexturingGUI::setChunkWindow(MapChunk *chunk)
{
  std::stringstream Temp;
  Temp << "Chunk " << chunk->px << ", " << chunk->py << " at (" << chunk->xbase << ", " << chunk->ybase<< ", " << chunk->zbase << ")";
  chunkLocation->setText(Temp.str().c_str());///



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
  Temp.clear();
  Temp << "AreaID: " << areaName.c_str() << " (" << chunk->areaID << ")" ;
  chunkAreaID->setText(Temp.str().c_str());///

  Temp.clear();
  Temp << "Flags: " << chunk->Flags;
  chunkFlags->setText(Temp.str().c_str());///

  for(int ch=0;ch<5;ch++)
    chunkFlagChecks[ch]->setState(false);


  if(chunk->Flags & FLAG_SHADOW)
    chunkFlagChecks[0]->setState(true);
  if(chunk->Flags & FLAG_IMPASS)
    chunkFlagChecks[1]->setState(true);
  if(chunk->Flags & FLAG_LQ_RIVER)
    chunkFlagChecks[2]->setState(true);
  if(chunk->Flags & FLAG_LQ_OCEAN)
    chunkFlagChecks[3]->setState(true);
  if(chunk->Flags & FLAG_LQ_MAGMA)
    chunkFlagChecks[4]->setState(true);

  //sprintf(Temp,"EffectID: %d",chunk->header.effectId);
  //chunkEffectID->setText(Temp);///

  std::stringstream ss;
  ss << "Num Effects: " << chunk->header.nEffectDoodad;
  chunkNumEffects->setText(ss.str().c_str());///

  //! /todo rework texture reading
  /*
  int pl=0;
  for(pl=0;pl<(chunk->nTextures);pl++)
  {
    chunkTexture[pl]->show();
  chunkTextureNames[pl]->show();
    chunkTextureFlags[pl]->show();
  chunkTextureEffectID[pl]->show();

    sprintf(Temp,"Flags- %d",chunk->texFlags[pl]);
  chunkTextureFlags[pl]->setText(Temp);

  sprintf(Temp,"EffectID- %d",chunk->effectID[pl]);
  chunkTextureEffectID[pl]->setText(Temp);

  chunkTexture[pl]->setTexture(TextureManager::items[chunk->textures[pl]]->name);

  chunkTextureNames[pl]->setText(TextureManager::items[chunk->textures[pl]]->name);
  }
  for(;pl<4;pl++)
  {
    chunkTexture[pl]->hide();
      chunkTextureNames[pl]->hide();
    chunkTextureFlags[pl]->hide();
    chunkTextureEffectID[pl]->hide();
  }*/

}

OpenGL::Texture* UITexturingGUI::getSelectedTexture(){
  return UITexturingGUI::selectedTexture;
}

void UITexturingGUI::setSelectedTexture(OpenGL::Texture * t){
  UITexturingGUI::selectedTexture = t;
}

/* //! /todo rework!
void setChunk(MapChunk *chunk)//I dont remember, but is is maybe mine ground texture changing function
{
  int i;
for(i=0;i<4;i++)
   {
     if((chunk->textures[i]==textureChunkSelected->texture)&&(selectedTexture!=0))
     {
    chunk->textures[i]=video.textures.add(selectedTexture->name.c_str());
     }
   };
}
*/
