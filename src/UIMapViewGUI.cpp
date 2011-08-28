#include "UIMapViewGUI.h"

#include <sstream>
#include <algorithm>
#include <vector>

#include "DBC.h"
#include "Environment.h"
#include "MapChunk.h"
#include "MapView.h"
#include "Noggit.h" // gStates, gPop, arial14, morpheus40, arial...
#include "Project.h"
#include "UIAppInfo.h" // UIAppInfo
#include "UIDetailInfos.h" // UIDetailInfos
#include "UIHelp.h"
#include "UIMinimapWindow.h"
#include "UIStatusBar.h" // UIStatusBar
#include "UITexturePicker.h" // 
#include "UITextureSwitcher.h"
#include "UITexturingGUI.h"
#include "UIToolbar.h" // UIToolbar
#include "UIZoneIDBrowser.h" // 
#include "Video.h" // video
#include "WMOInstance.h"
#include "World.h"

UIMapViewGUI::UIMapViewGUI(MapView *setMapview)
: UIFrame( 0.0f, 0.0f, video.xres(), video.yres() )
, theMapview( setMapview )
{
  // Minimap window
  minimapWindow = new UIMinimapWindow(gWorld);
  minimapWindow->hide();
  addChild(minimapWindow);

  // UIToolbar
  guiToolbar = new UIToolbar( 6.0f, 38.0f, this );
  addChild(guiToolbar);
  
  // Statusbar
  guiStatusbar = new UIStatusBar( 0.0f, video.yres() - 30.0f, video.xres(), 30.0f );
  addChild(guiStatusbar);

  // DetailInfoWindow
  guidetailInfos = new UIDetailInfos( 1.0f, video.yres() - 282.0f, 600.0f, 250.0f, this );
  guidetailInfos->movable( true );
  guidetailInfos->hide();
  addChild(guidetailInfos);

  // ZoneIDBrowser
  ZoneIDBrowser = new UIZoneIDBrowser(200, 200, 435, 400, this);
  ZoneIDBrowser->movable( true );
  ZoneIDBrowser->hide();
  addChild(ZoneIDBrowser);

  // AppInfosWindow
  guiappInfo = new UIAppInfo( 1.0f, video.yres() - 440.0f, 420.0f, 410.0f, this );
  guiappInfo->movable( true );
  guiappInfo->hide();
  std::stringstream appinfoText;
  appinfoText << "Project Path: " << Project::getInstance()->getPath() << std::endl;
  guiappInfo->setText( appinfoText.str() );
  addChild(guiappInfo);

  TexturePicker = new UITexturePicker(video.xres() / 2 - 100.0f, video.yres() / 2 - 100.0f,490.0f, 150.0f );
  TexturePicker->hide();
  TexturePicker->movable( true );
  addChild( TexturePicker);

  TextureSwitcher = new UITextureSwitcher(video.xres() / 2 - 100.0f, video.yres() / 2 - 100.0f,490.0f, 150.0f );
  TextureSwitcher->hide();
  TextureSwitcher->movable( true );
  addChild( TextureSwitcher);
  
  _help = new UIHelp();
  _help->hide();
  addChild( _help );
}

void UIMapViewGUI::showHelp()
{
  _help->show();
}
void UIMapViewGUI::hideHelp()
{
  _help->hide();
}
void UIMapViewGUI::toggleHelp()
{
  _help->toggleVisibility();
}

void UIMapViewGUI::setTilemode( bool enabled )
{
  _tilemode = enabled;
}

void UIMapViewGUI::render( ) const
{
  UIFrame::render();
  
  //! \todo Make these some textUIs.
  arial16.shprint( 510, 4, gAreaDB.getAreaName( gWorld->getAreaID() ) );
  
  int time = static_cast<int>( gWorld->time ) % 2880;
  std::stringstream timestrs; timestrs << "Time: " << ( time / 120 ) << ":" << ( time % 120 );
  arial16.shprint( video.xres() - 100.0f, 5.0f, timestrs.str() );
  
  if ( gWorld->loading )
  {
    std::string toDisplay( gWorld->noadt ? "No ADT at this Point" : "Loading..." );
    arial16.shprint( video.xres() / 2.0f - arial16.width( toDisplay ) / 2.0f, 30.0f, toDisplay );
  }

  std::ostringstream statusbarInfo;
  statusbarInfo << "tile: " << std::floor( gWorld->camera.x / TILESIZE ) << " " <<  std::floor( gWorld->camera.z / TILESIZE )
     << "; coordinates: client (x: " << gWorld->camera.x << ", y: " << gWorld->camera.z << ", z: "<<gWorld->camera.y
     << "), server (x: " << ( ZEROPOINT - gWorld->camera.x ) << ", y:" << ( ZEROPOINT - gWorld->camera.z ) << ", z:" << ( ZEROPOINT - gWorld->camera.y ) << ")" ;
  if(Environment::getInstance()->paintMode) statusbarInfo << "PM:1";else statusbarInfo << "PM:2";
  guiStatusbar->setLeftInfo( statusbarInfo.str() );
  
  guiStatusbar->setRightInfo( "" );
 
  if( !_tilemode && !guidetailInfos->hidden() )
  {
    nameEntry * lSelection = gWorld->GetCurrentSelection();
    if( lSelection )
    {
      guiStatusbar->setRightInfo( lSelection->returnName() );
      
      std::stringstream detailInfo;

      switch( lSelection->type ) 
      {
      case eEntry_Model:
        detailInfo << "filename: " << lSelection->data.model->model->_filename
          << "\nunique ID: " << lSelection->data.model->d1
          << "\nposition X/Y/Z: " << lSelection->data.model->pos.x << " / " << lSelection->data.model->pos.y << " / " << lSelection->data.model->pos.z
          << "\nrotation X/Y/Z: " << lSelection->data.model->dir.x << " / " << lSelection->data.model->dir.y << " / " << lSelection->data.model->dir.z
          << "\nscale: " <<  lSelection->data.model->sc
          << "\ntextures Used: " << lSelection->data.model->model->header.nTextures;
        
        for( unsigned int j = 0; j < std::min( lSelection->data.model->model->header.nTextures, 6U ); j++ )
        {
          detailInfo << "\n " << ( j + 1 ) << ": " << lSelection->data.model->model->_textures[j]->filename();
        }
        if( lSelection->data.model->model->header.nTextures > 25 )
        {
          detailInfo << "\n and more.";
        }
        
        detailInfo << "\n";
        break;
        
      case eEntry_WMO:
        detailInfo << "filename: " << lSelection->data.wmo->wmo->_filename
          << "\nunique ID: " << lSelection->data.wmo->mUniqueID
          << "\nposition X/Y/Z: " << lSelection->data.wmo->pos.x << " / " << lSelection->data.wmo->pos.y << " / " << lSelection->data.wmo->pos.z
          << "\nrotation X/Y/Z: " << lSelection->data.wmo->dir.x << " / " << lSelection->data.wmo->dir.y << " / " << lSelection->data.wmo->dir.z
          << "\ndoodad set: " << lSelection->data.wmo->doodadset
          << "\ntextures used: " << lSelection->data.wmo->wmo->nTextures;

        for( unsigned int j = 0; j < std::min( lSelection->data.wmo->wmo->nTextures, 8U ); j++ )
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
          //s << " ";
          //s "  Flags - " << lSelection->data.mapchunk->texFlags[q] << " Effect ID - " << lSelection->data.mapchunk->effectID[q] << std::endl;
          
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
      guidetailInfos->setText( detailInfo.str() );
    }
    else 
    {
      guidetailInfos->setText( "" );
    }
  }
}
