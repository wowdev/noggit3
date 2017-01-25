// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/MapViewGUI.h>

#include <sstream>
#include <algorithm>
#include <vector>

#include <noggit/DBC.h>
#include <noggit/Environment.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/application.h> // app.getStates(), gPop, app.getArial14(), morpheus40, arial...
#include <noggit/Project.h>
#include <noggit/ui/AppInfo.h> // UIAppInfo
#include <noggit/ui/CursorSwitcher.h> // UICursorSwitcher
#include <noggit/ui/DetailInfos.h> // UIDetailInfos
#include <noggit/ui/DoodadSpawner.h>
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/Help.h>
#include <noggit/ui/MinimapWindow.h>
#include <noggit/ui/shader_tool.hpp>
#include <noggit/ui/StatusBar.h> // UIStatusBar
#include <noggit/ui/terrain_tool.hpp>
#include <noggit/ui/TexturePicker.h> //
#include <noggit/ui/TextureSwitcher.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/Toolbar.h> // UIToolbar
#include <noggit/ui/ZoneIDBrowser.h> //
#include <noggit/ui/Water.h> //
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/Video.h> // video
#include <noggit/WMOInstance.h>
#include <noggit/World.h>
#include <noggit/texture_set.hpp>
#include <noggit/map_index.hpp>
#include <noggit/Misc.h>
#include <noggit/ui/WaterTypeBrowser.h>

#include <noggit/ui/Model.h>
#include <noggit/ModelManager.h>

#include <noggit/ui/Alphamap.h>
#include <noggit/Settings.h>



UIMapViewGUI::UIMapViewGUI(MapView *setMapview)
  : UIFrame(0.0f, 0.0f, (float)video.xres(), (float)video.yres())
  , theMapview(setMapview)
{
  // Minimap window
  minimapWindow = new UIMinimapWindow(gWorld);
  minimapWindow->hide();
  addChild(minimapWindow);

  objectEditor = new UIObjectEditor((float)video.xres() - 410.0f, 10.0f, this);
  objectEditor->movable(true);
  objectEditor->hide();
  addChild(objectEditor);


  rotationEditor = new UIRotationEditor((float)video.xres() - 520.0f, 30.0f);
  rotationEditor->movable(true);
  rotationEditor->hide();
  addChild(rotationEditor);

#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
    terrainTool = new ui::terrain_tool((float)video.xres() - 185.0f, 30.0f, true);
  else
#endif
    terrainTool = new ui::terrain_tool((float)video.xres() - 185.0f, 30.0f, false);

  terrainTool->show();
  addChild(terrainTool);

  flattenTool = new ui::FlattenTool((float)video.xres() - 210.0f, 30.0f);
  flattenTool->hide();
  addChild(flattenTool);

#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
    shaderTool = new ui::shader_tool((float)video.xres() - 185.0f, 30.0f, true);
  else
#endif
    shaderTool = new ui::shader_tool((float)video.xres() - 185.0f, 30.0f, false);
  
  shaderTool->hide();
  addChild(shaderTool);

  // UICurrentTexture
  guiCurrentTexture = new UICurrentTexture(6.0f, 35.0f, this);
  addChild(guiCurrentTexture);

  // UIToolbar
  guiToolbar = new UIToolbar(6.0f, 145.0f);
  addChild(guiToolbar);


  // Statusbar
  guiStatusbar = new UIStatusBar(0.0f, (float)video.yres() - 30.0f, (float)video.xres(), 30.0f);
  addChild(guiStatusbar);

  // DetailInfoWindow
  guidetailInfos = new UIDetailInfos(1.0f, video.yres() - 282.0f, 600.0f, 250.0f);
  guidetailInfos->movable(true);
  guidetailInfos->hide();
  addChild(guidetailInfos);

  // ZoneIDBrowser
  ZoneIDBrowser = new UIZoneIDBrowser(200, 200, 410, 400, this);
  ZoneIDBrowser->movable(true);
  ZoneIDBrowser->hide();
  addChild(ZoneIDBrowser);

  // AppInfosWindow
  guiappInfo = new UIAppInfo(1.0f, video.yres() - 440.0f, 420.0f, 410.0f);
  guiappInfo->movable(true);
  guiappInfo->hide();
  std::stringstream appinfoText;
  appinfoText << "Project Path: " << Project::getInstance()->getPath() << std::endl;
  guiappInfo->setText(appinfoText.str());
  addChild(guiappInfo);

  TexturePicker = new UITexturePicker(video.xres() / 2 - 100.0f, video.yres() / 2 - 100.0f, 490.0f, 170.0f);
  TexturePicker->hide();
  TexturePicker->movable(true);
  addChild(TexturePicker);

  TextureSwitcher = new UITextureSwitcher(100, 100);
  TextureSwitcher->setPosition(video.xres() - TextureSwitcher->getW(), 40);
  TextureSwitcher->hide();
  TextureSwitcher->movable(true);
  addChild(TextureSwitcher);

  // Cursor Switcher
  CursorSwitcher = new UICursorSwitcher();
  CursorSwitcher->hide();
  CursorSwitcher->movable(true);
  addChild(CursorSwitcher);

  _help = new UIHelp();
  _help->hide();
  addChild(_help);

  guiWater = new UIWater(this);
  guiWater->hide();
  guiWater->movable(true);
  addChild(guiWater);

  guiWaterTypeSelector = new UIWaterTypeBrowser(this->width() - 430.0f, 38.0f, 240.0f, 300.0f, this);
  guiWaterTypeSelector->hide();
  guiWaterTypeSelector->movable(true);
  addChild(guiWaterTypeSelector);

  //UIDoodadSpawner *_test = new UIDoodadSpawner();
  //_test->show();
  //addChild( _test );
}

void UIMapViewGUI::showCursorSwitcher()
{
  CursorSwitcher->show();
}

void UIMapViewGUI::hideCursorSwitcher()
{
  CursorSwitcher->hide();
}

void UIMapViewGUI::toggleCursorSwitcher()
{
  CursorSwitcher->toggleVisibility();
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

void UIMapViewGUI::showTest()
{
  //_test->show();
}
void UIMapViewGUI::hideTest()
{
  //_test->hide();
}
void UIMapViewGUI::toggleTest()
{
  //_test->toggleVisibility();
}

void UIMapViewGUI::setTilemode(bool enabled)
{
  _tilemode = enabled;
}

void UIMapViewGUI::render() const
{
  UIFrame::render();

  //! \todo Make these some textUIs.
  app.getArial16().shprint(510, 4, gAreaDB.getAreaName(gWorld->getAreaID()));

  int time = static_cast<int>(gWorld->time) % 2880;
  std::stringstream timestrs;
#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode) {
    timestrs << "Time: " << (time / 120) << ":" << (time % 120) << ", FPS: " << (int)app.FPS << ", Pres: " << app.pressure;
    app.getArial16().shprint(video.xres() - 250.0f, 5.0f, timestrs.str());
  }
  else
#endif
  {
    timestrs << "Time: " << (time / 120) << ":" << (time % 120) << ", FPS: " << (int)app.FPS;
    app.getArial16().shprint(video.xres() - 200.0f, 5.0f, timestrs.str());
  }

  if (gWorld->loading)
  {
    std::string toDisplay(gWorld->mapIndex->hasAdt() ? "No ADT at this Point" : "Loading...");
    app.getArial16().shprint(video.xres() / 2.0f - app.getArial16().width(toDisplay) / 2.0f, 30.0f, toDisplay);
  }

  std::ostringstream statusbarInfo;
  statusbarInfo << "tile: " << std::floor(gWorld->camera.x / TILESIZE) << " " << std::floor(gWorld->camera.z / TILESIZE)
    << "; coordinates: client (x: " << gWorld->camera.x << ", y: " << gWorld->camera.z << ", z: " << gWorld->camera.y
    << "), server (x: " << (ZEROPOINT - gWorld->camera.z) << ", y:" << (ZEROPOINT - gWorld->camera.x) << ", z:" << (gWorld->camera.y) << ")";
  if (Environment::getInstance()->paintMode) statusbarInfo << "PM:1"; else statusbarInfo << "PM:2";
  guiStatusbar->setLeftInfo(statusbarInfo.str());

  guiStatusbar->setRightInfo("");
  guiWater->updatePos(misc::FtoIround((gWorld->camera.x - (TILESIZE / 2)) / TILESIZE), misc::FtoIround((gWorld->camera.z - (TILESIZE / 2)) / TILESIZE));

  if (!_tilemode && !guidetailInfos->hidden())
  {
    auto lSelection = gWorld->GetCurrentSelection();
    if (lSelection)
    {
      std::stringstream detailInfo;

      switch (lSelection->which())
      {
      case eEntry_Model:
        {
          auto instance (boost::get<selected_model_type> (*lSelection));
          guiStatusbar->setRightInfo (std::to_string (instance->d1) + ": " + instance->model->_filename);
          detailInfo << "filename: " << instance->model->_filename
                     << "\nunique ID: " << instance->d1
                     << "\nposition X/Y/Z: " << instance->pos.x << " / " << instance->pos.y << " / " << instance->pos.z
                     << "\nrotation X/Y/Z: " << instance->dir.x << " / " << instance->dir.y << " / " << instance->dir.z
                     << "\nscale: " << instance->sc
                     << "\ntextures Used: " << instance->model->header.nTextures;

          for (unsigned int j = 0; j < std::min(instance->model->header.nTextures, 6U); j++)
          {
            detailInfo << "\n " << (j + 1) << ": " << instance->model->_textures[j]->filename();
          }
          if (instance->model->header.nTextures > 25)
          {
            detailInfo << "\n and more.";
          }

          detailInfo << "\n";
          break;
        }
      case eEntry_WMO:
        {
          auto instance (boost::get<selected_wmo_type> (*lSelection));
          guiStatusbar->setRightInfo (std::to_string (instance->mUniqueID) + ": " + instance->wmo->_filename);
          detailInfo << "filename: " << instance->wmo->_filename
                     << "\nunique ID: " << instance->mUniqueID
                     << "\nposition X/Y/Z: " << instance->pos.x << " / " << instance->pos.y << " / " << instance->pos.z
                     << "\nrotation X/Y/Z: " << instance->dir.x << " / " << instance->dir.y << " / " << instance->dir.z
                     << "\ndoodad set: " << instance->doodadset
                     << "\ntextures used: " << instance->wmo->nTextures;

          for (unsigned int j = 0; j < std::min(instance->wmo->nTextures, 8U); j++)
          {
            detailInfo << "\n " << (j + 1) << ": " << instance->wmo->textures[j];
          }
          if (instance->wmo->nTextures > 25)
          {
            detailInfo << "\n and more.";
          }

          detailInfo << "\n";
          break;
        }
      case eEntry_MapChunk:
        {
          auto chunk (boost::get<selected_chunk_type> (*lSelection).chunk);
          guiStatusbar->setRightInfo (std::to_string (chunk->px) + ", " + std::to_string (chunk->py));
          int flags = chunk->Flags;

          detailInfo << "MCNK " << chunk->px << ", " << chunk->py << " (" << chunk->py * 16 + chunk->px
                     << ") of tile (" << chunk->mt->index.x << " " << chunk->mt->index.z << ")"
                     << "\narea ID: " << chunk->getAreaID() << " (\"" << gAreaDB.getAreaName(chunk->getAreaID()) << "\")"
                     << "\nflags: "
                     << (flags & FLAG_SHADOW ? "shadows " : "")
                     << (flags & FLAG_IMPASS ? "impassable " : "")
                     << (flags & FLAG_LQ_RIVER ? "river " : "")
                     << (flags & FLAG_LQ_OCEAN ? "ocean " : "")
                     << (flags & FLAG_LQ_MAGMA ? "lava" : "")
                     << "\ntextures used: " << chunk->textureSet->num();

          //! \todo get a list of textures and their flags as well as detail doodads.
          /*
            for( int q = 0; q < chunk->nTextures; q++ )
            {
            //s << " ";
            //s "  Flags - " << chunk->texFlags[q] << " Effect ID - " << chunk->effectID[q] << std::endl;

            if( chunk->effectID[q] != 0 )
            for( int r = 0; r < 4; r++ )
            {
            const char *EffectModel = getGroundEffectDoodad( chunk->effectID[q], r );
            if( EffectModel )
            {
            s << r << " - World\\NoDXT\\" << EffectModel << endl;
            //freetype::shprint( app.getArial16(), 30, 103 + TextOffset, "%d - World\\NoDXT\\%s", r, EffectModel );
            TextOffset += 20;
            }
            }

            }
          */

          detailInfo << "\n";

          break;
        }
      }
      guidetailInfos->setText(detailInfo.str());
    }
    else
    {
      guidetailInfos->setText("");
    }
  }
}
