#include "Gui.h"

#include "Toolbar.h" // Toolbar
#include "statusBar.h" // statusBar
#include "detailInfos.h" // detailInfos
#include "appInfo.h" // appInfo
#include "ui_ZoneIdBrowser.h" // 
#include "uiTexturePicker.h" // 
#include "video.h" // video
#include "MapView.h"
#include "minimapWindowUI.h"
#include "noggit.h" // gStates, gPop, gFPS, arial14, morpheus40, arial...
#include "dbc.h"
#include "world.h"
#include "Project.h"
#include "Environment.h"
// Detail window. Move there?
#include "MapChunk.h"
#include "WMOInstance.h"
#include "TexturingUI.h"
#include "TextureManager.h"

#include <sstream>
#include <algorithm>
#include <vector>

Gui::Gui(MapView *setMapview) : theMapview( setMapview )
{
  this->tileFrames = new frame( 0.0f, 0.0f, video.xres, video.yres );
  
  // Minimap window
  this->minimapWindow = new minimapWindowUI(gWorld);
  this->minimapWindow->hidden = true;
  this->tileFrames->addChild(this->minimapWindow);

  // Toolbar
  this->guiToolbar = new Toolbar( 6.0f, 38.0f, 105.0f, 600.0f, this );
  this->tileFrames->addChild(this->guiToolbar);
  
  // Statusbar
  this->guiStatusbar = new statusBar( 0.0f, video.yres - 30.0f, video.xres, 30.0f );
  this->tileFrames->addChild(this->guiStatusbar);

  // DetailInfoWindow
  this->guidetailInfos = new detailInfos( 1.0f, video.yres - 282.0f, 600.0f, 250.0f, this );
  this->guidetailInfos->movable = true;
  this->guidetailInfos->hidden = true;
  this->tileFrames->addChild(this->guidetailInfos);

  // ZoneIDBrowser
  this->ZoneIDBrowser = new ui_ZoneIdBrowser(200, 200, 435, 400, this);
  this->ZoneIDBrowser->movable = true;
  this->ZoneIDBrowser->hidden = true;
  this->tileFrames->addChild(this->ZoneIDBrowser);

  // AppInfosWindow
  this->guiappInfo = new appInfo( 1.0f, video.yres - 440.0f, 420.0f, 410.0f, this );
  this->guiappInfo->movable = true;
  this->guiappInfo->hidden = true;
  std::stringstream appinfoText;
  appinfoText << "Project Path: " << Project::getInstance()->getPath() << std::endl;
  this->guiappInfo->setText( appinfoText.str() );
  this->tileFrames->addChild(this->guiappInfo);

  this->TexturePicker = new uiTexturePicker(video.xres / 2 - 100.0f,video.yres / 2 - 100.0f,490.0f,150.0f,this);
  this->TexturePicker->hidden = true;
  this->TexturePicker->movable = true;
  this->tileFrames->addChild( this->TexturePicker);

}

Gui::~Gui()
{
  delete tileFrames;
}

void Gui::resize()
{
  for( std::vector<frame*>::iterator child = tileFrames->children.begin(); child != tileFrames->children.end(); ++child )
    (*child)->resize();
}

void Gui::render( bool tilemode )
{
  this->tileFrames->render();
  
  //! \todo Make these some textUIs.
  freetype::shprint( *arial16, 510, 4, gAreaDB.getAreaName( gWorld->getAreaID() ) );
  
  std::stringstream fps; fps << gFPS << " fps";
  freetype::shprint( *arial16, video.xres - 200, 5, fps.str() );
  
  int time = static_cast<int>( gWorld->time ) % 2880;
  std::stringstream timestrs; timestrs << "Time: " << ( time / 120 ) << ":" << ( time % 120 );
  freetype::shprint( *arial16, video.xres - 100.0f, 5.0f, timestrs.str() );
  
  if ( gWorld->loading ) 
    freetype::shprint( *arial16, video.xres/2 - freetype::width( *arial16, gWorld->noadt ? "No ADT at this Point" : "Loading..." ) / 2, 30, ( gWorld->noadt ? "No ADT at this Point" : "Loading..." ) );
    
    
  std::ostringstream statusbarInfo;
  statusbarInfo << "tile: " << std::floor( gWorld->camera.x / TILESIZE ) << " " <<  std::floor( gWorld->camera.z / TILESIZE )
     << "; coordinates: client (x: " << gWorld->camera.x << ", y: " << gWorld->camera.z << ", z: "<<gWorld->camera.y
     << "), server (x: " << ( ZEROPOINT - gWorld->camera.x ) << ", y:" << ( ZEROPOINT - gWorld->camera.z ) << ", z:" << ( ZEROPOINT - gWorld->camera.y ) << ")" ;
  if(Environment::getInstance()->paintMode) statusbarInfo << "PM:1";else statusbarInfo << "PM:2";
  this->guiStatusbar->setLeftInfo( statusbarInfo.str() );
  
  this->guiStatusbar->setRightInfo( "" );
 
  if( !tilemode && !this->guidetailInfos->hidden )
  {
    nameEntry * lSelection = gWorld->GetCurrentSelection();
    if( lSelection )
    {
      this->guiStatusbar->setRightInfo( lSelection->returnName() );
      
      std::stringstream detailInfo;

      switch( lSelection->type ) 
      {
      case eEntry_Model:
        detailInfo << "filename: " << lSelection->data.model->model->filename
          << "\nunique ID: " << lSelection->data.model->d1
          << "\nposition X/Y/Z: " << lSelection->data.model->pos.x << " / " << lSelection->data.model->pos.y << " / " << lSelection->data.model->pos.z
          << "\nrotation X/Y/Z: " << lSelection->data.model->dir.x << " / " << lSelection->data.model->dir.y << " / " << lSelection->data.model->dir.z
          << "\nscale: " <<  lSelection->data.model->sc
          << "\ntextures Used: " << lSelection->data.model->model->header.nTextures;
        
        using std::min;
        for( unsigned int j = 0; j < min( lSelection->data.model->model->header.nTextures, 6U ); j++ )
        {
          detailInfo << "\n " << ( j + 1 ) << ": " << TextureManager::items[lSelection->data.model->model->textures[j]]->name;
        }
        if( lSelection->data.model->model->header.nTextures > 25 )
        {
          detailInfo << "\n and more.";
        }
        
        detailInfo << "\n";
        break;
        
      case eEntry_WMO:
        detailInfo << "filename: " << lSelection->data.wmo->wmo->filename
          << "\nunique ID: " << lSelection->data.wmo->mUniqueID
          << "\nposition X/Y/Z: " << lSelection->data.wmo->pos.x << " / " << lSelection->data.wmo->pos.y << " / " << lSelection->data.wmo->pos.z
          << "\nrotation X/Y/Z: " << lSelection->data.wmo->dir.x << " / " << lSelection->data.wmo->dir.y << " / " << lSelection->data.wmo->dir.z
          << "\ndoodad set: " << lSelection->data.wmo->doodadset
          << "\ntextures used: " << lSelection->data.wmo->wmo->nTextures;

        using std::min;
        for( unsigned int j = 0; j < min( lSelection->data.wmo->wmo->nTextures, 8U ); j++ )
        {
          detailInfo << "\n " << ( j + 1 ) << ": " << lSelection->data.wmo->wmo->textures[j];
        }
        if( lSelection->data.wmo->wmo->nTextures > 25 )
        {
          detailInfo << "\n and more.";
        }
        
        detailInfo << "\n";
        break;
        
      case eEntry_MapChunk:
        
        int flags = lSelection->data.mapchunk->Flags;
      
        detailInfo << "MCNK " << lSelection->data.mapchunk->px << ", " << lSelection->data.mapchunk->py << " (" << lSelection->data.mapchunk->py * 16 + lSelection->data.mapchunk->px
          << ") of tile (" << lSelection->data.mapchunk->mt->mPositionX << " " << lSelection->data.mapchunk->mt->mPositionZ << ")"
          << "\narea ID: " << lSelection->data.mapchunk->areaID << " (\"" << gAreaDB.getAreaName( lSelection->data.mapchunk->areaID ) << "\")"
          << "\nflags: "
          << ( flags & FLAG_SHADOW ? "shadows " : "" )
          << ( flags & FLAG_IMPASS ? "impassable " : "" )
          << ( flags & FLAG_LQ_RIVER ? "river " : "" )
          << ( flags & FLAG_LQ_OCEAN ? "ocean " : "" )
          << ( flags & FLAG_LQ_MAGMA ? "lava" : "" )
          << "\ntextures used: " << lSelection->data.mapchunk->nTextures;
          
        //! \todo get a list of textures and their flags as well as detail doodads.
        /*
        for( int q = 0; q < lSelection->data.mapchunk->nTextures; q++ )
        {
          //s << q << "- " << TextureManager::items[lSelection->data.mapchunk->textures[q]]->name;
          //s << " ";
          //s "  Flags - " << lSelection->data.mapchunk->texFlags[q] << " Effect ID - " << lSelection->data.mapchunk->effectID[q] << std::endl;
          //freetype::shprint( arial16, 20, 103 + TextOffset, "%d - %s  Flags - %x Effect ID - %d", q, TextureManager::items[lSelection->data.mapchunk->textures[q]]->name, lSelection->data.mapchunk->texFlags[q], lSelection->data.mapchunk->effectID[q] );
          
          if( lSelection->data.mapchunk->effectID[q] != 0 )
            for( int r = 0; r < 4; r++ )
            {
              const char *EffectModel = getGroundEffectDoodad( lSelection->data.mapchunk->effectID[q], r );
              if( EffectModel )
              {
                  s << r << " - World\\NoDXT\\" << EffectModel << endl;  
                  //freetype::shprint( arial16, 30, 103 + TextOffset, "%d - World\\NoDXT\\%s", r, EffectModel );
                  TextOffset += 20;
              }
            }
            
        }
        */
        
        detailInfo << "\n";
    
        break;
      }
      this->guidetailInfos->setText( detailInfo.str() );
    }
    else 
    {
      this->guidetailInfos->setText( "" );
    }
  }
}
