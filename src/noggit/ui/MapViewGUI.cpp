// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/MapViewGUI.h>

#include <sstream>
#include <algorithm>
#include <vector>

#include <noggit/DBC.h>
#include <noggit/Environment.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/application.h> // app.getStates(), gPop, app.getArial14(), arial...
#include <noggit/Project.h>
#include <noggit/ui/CurrentTexture.h>
#include <noggit/ui/DetailInfos.h> // ui::detail_infos
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/Help.h>
#include <noggit/ui/shader_tool.hpp>
#include <noggit/ui/terrain_tool.hpp>
#include <noggit/ui/texturing_tool.hpp>
#include <noggit/ui/TexturePicker.h> //
#include <noggit/ui/TexturingGUI.h>
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

#include <noggit/ModelManager.h>

#include <noggit/Settings.h>



UIMapViewGUI::UIMapViewGUI ( MapView *setMapview
                           , noggit::camera* camera
                           , float* tablet_pressure
                           , bool* tablet_active
                           )
  : UIFrame(0.0f, 0.0f, (float)video::width, (float)video::height)
  , _camera (camera)
  , _tablet_pressure (tablet_pressure)
  , _tablet_active (tablet_active)
  , theMapview(setMapview)
{
  objectEditor = new UIObjectEditor((float)video::width - 410.0f, 10.0f, this);
  objectEditor->hide();

  rotationEditor = new UIRotationEditor();
  rotationEditor->hide();

  _terrain = new QDockWidget ("Raise / Lower", setMapview);
  _terrain->setFeatures ( QDockWidget::DockWidgetMovable
                        | QDockWidget::DockWidgetFloatable
                        );
  _terrain->setWidget (terrainTool = new ui::terrain_tool());
  setMapview->_main_window->addDockWidget (Qt::RightDockWidgetArea, _terrain);


  flattenTool = new ui::FlattenTool();
  flattenTool->hide();

  shaderTool = new ui::shader_tool(theMapview->cursor_color);
  shaderTool->hide();

  texturingTool = new ui::texturing_tool (&camera->position);
  texturingTool->hide();


  TexturePalette = UITexturingGUI::createTexturePalette(this);
  TexturePalette->hide();
  addChild(TexturePalette);
  addChild(UITexturingGUI::createTilesetLoader());
  addChild(UITexturingGUI::createTextureFilter());

  guiCurrentTexture = new noggit::ui::current_texture(TexturePalette);

  // DetailInfoWindow
  guidetailInfos = new ui::detail_infos(1.0f, video::height - 282.0f, 600.0f, 250.0f);
  guidetailInfos->hide();

  // ZoneIDBrowser
  ZoneIDBrowser = new ui::zone_id_browser();
  ZoneIDBrowser->hide();

  TexturePicker = new UITexturePicker(video::width / 2 - 100.0f, video::height / 2 - 100.0f, 490.0f, 170.0f);
  TexturePicker->hide();
  TexturePicker->movable(true);
  addChild(TexturePicker);

  _help = new UIHelp();

  guiWater = new UIWater(this);
  guiWater->hide();
  guiWater->movable(true);
  addChild(guiWater);

  guiWaterTypeSelector = new ui::water_type_browser(guiWater);
  guiWaterTypeSelector->hide();
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
  _help->setVisible (!_help->isVisible());
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

  tile_index tile (_camera->position);
  guiWater->updatePos(tile);

  if (!_tilemode && !guidetailInfos->isHidden())
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
          detailInfo << "filename: " << instance->wmo->_filename
                     << "\nunique ID: " << instance->mUniqueID
                     << "\nposition X/Y/Z: " << instance->pos.x << " / " << instance->pos.y << " / " << instance->pos.z
                     << "\nrotation X/Y/Z: " << instance->dir.x << " / " << instance->dir.y << " / " << instance->dir.z
                     << "\ndoodad set: " << instance->doodadset
                     << "\ntextures used: " << instance->wmo->textures.size();


          const unsigned int texture_count (std::min((unsigned int)(instance->wmo->textures.size()), 8U));
          for (unsigned int j = 0; j < texture_count; j++)
          {
            detailInfo << "\n " << (j + 1) << ": " << instance->wmo->textures[j];
          }
          if (instance->wmo->textures.size() > 25)
          {
            detailInfo << "\n and more.";
          }

          detailInfo << "\n";
          break;
        }
      case eEntry_MapChunk:
        {
          auto chunk (boost::get<selected_chunk_type> (*lSelection).chunk);
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
                     << "\ntextures used: " << chunk->_texture_set.num();

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
