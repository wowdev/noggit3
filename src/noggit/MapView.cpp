// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#undef _UNICODE

#include <noggit/Brush.h> // brush
#include <noggit/ConfigFile.h>
#include <noggit/DBC.h>
#include <noggit/Environment.h>
#include <noggit/Environment.h>
#include <noggit/FreeType.h> // freetype::
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Project.h>
#include <noggit/Settings.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/application.h> // app.getStates(), gPop, gFPS, app.getArial14(), morpheus40, arial...
#include <noggit/map_index.hpp>
#include <noggit/ui/AppInfo.h> // appInfo
#include <noggit/ui/CapsWarning.h>
#include <noggit/ui/scWarning.h>
#include <noggit/ui/CheckBox.h> // UICheckBox
#include <noggit/ui/CursorSwitcher.h> // UICursorSwitcher
#include <noggit/ui/DetailInfos.h> // detailInfos
#include <noggit/ui/ExitWarning.h>
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/Gradient.h> // UIGradient
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/MapViewGUI.h> // UIMapViewGUI
#include <noggit/ui/MenuBar.h> // UIMenuBar, menu items, ..
#include <noggit/ui/MinimapWindow.h> // UIMinimapWindow
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/shader_tool.hpp>
#include <noggit/ui/Slider.h> // UISlider
#include <noggit/ui/StatusBar.h> // statusBar
#include <noggit/ui/terrain_tool.hpp>
#include <noggit/ui/Text.h> // UIText
#include <noggit/ui/Texture.h> // textureUI
#include <noggit/ui/TexturePicker.h>
#include <noggit/ui/TextureSwitcher.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/texturing_tool.hpp>
#include <noggit/ui/ToggleGroup.h> // UIToggleGroup
#include <noggit/ui/Toolbar.h> // UIToolbar
#include <noggit/ui/ToolbarIcon.h> // ToolbarIcon
#include <noggit/ui/Water.h>
#include <noggit/ui/WaterSaveWarning.h>
#include <noggit/ui/WaterTypeBrowser.h>
#include <noggit/ui/ZoneIDBrowser.h>
#include <opengl/matrix.hpp>
#include <opengl/scoped.hpp>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

static const float XSENS = 15.0f;
static const float YSENS = 15.0f;
static const float SPEED = 200.6f;

int MouseX;
int MouseY;
float mh, mv, rh, rv;

float moveratio = 0.1f;
float rotratio = 0.2f;
float keyx, keyy, keyz, keyr, keys;

float tool_settings_x;
float tool_settings_y;

bool MoveObj;

math::vector_3d objMove;
math::vector_3d objMoveOffset;
math::vector_3d objRot;

boost::optional<selection_type> lastSelected;

bool TestSelection = false;

bool  leftMouse = false;
bool  leftClicked = false;
bool  rightMouse = false;
bool  painting = false;

// Vars for the ground editing toggle mode
// store the status of some view settings when
// the ground editing mode is switched on
// to restore them if switch back again

bool  alloff = true;
bool  alloff_models = false;
bool  alloff_doodads = false;
bool  alloff_contour = false;
bool  alloff_wmo = false;
bool  alloff_detailselect = false;
bool  alloff_fog = false;
bool  alloff_terrain = false;

editing_mode terrainMode = editing_mode::ground;
editing_mode saveterrainMode = terrainMode;

UICursorSwitcher* CursorSwitcher;

bool Saving = false;

UIFrame* LastClicked;

// main GUI object
UIMapViewGUI* mainGui;

UIFrame* MapChunkWindow;
UIToggleGroup * gFlagsToggleGroup;

void MapView::set_editing_mode (editing_mode mode)
{
  mainGui->guiWaterTypeSelector->hide();
  mainGui->terrainTool->hide();
  mainGui->flattenTool->hide();
  mainGui->texturingTool->hide();
  mainGui->shaderTool->hide();
  mainGui->guiWater->hide();
  mainGui->TexturePicker->hide();
  mainGui->objectEditor->hide();
  mainGui->objectEditor->filename->hide();
  mainGui->objectEditor->modelImport->hide();
  mainGui->rotationEditor->hide();
  mainGui->ZoneIDBrowser->hide();

  if (!mainGui || !mainGui->TexturePalette)
    return;
  mainGui->TexturePalette->hide();
  // fetch old win position
  switch (terrainMode)
  {
  case editing_mode::ground:
    tool_settings_x = mainGui->terrainTool->x();
    tool_settings_y = mainGui->terrainTool->y();
    break;
  case editing_mode::flatten_blur:
    tool_settings_x = mainGui->flattenTool->x();
    tool_settings_y = mainGui->flattenTool->y();
    break;
  case editing_mode::paint:
    tool_settings_x = mainGui->texturingTool->x();
    tool_settings_y = mainGui->texturingTool->y();
    break;
  case editing_mode::areaid:
    tool_settings_x = mainGui->ZoneIDBrowser->x() + 230;
    tool_settings_y = mainGui->ZoneIDBrowser->y();
    break;
  case editing_mode::water:
    tool_settings_x = mainGui->guiWater->x();
    tool_settings_y = mainGui->guiWater->y();
    break;
  case editing_mode::mccv:
    tool_settings_x = mainGui->shaderTool->x();
    tool_settings_y = mainGui->shaderTool->y();
    break;
  }
  // set new win pos and make visible
  switch (mode)
  {
  case editing_mode::ground:
    mainGui->terrainTool->x(tool_settings_x);
    mainGui->terrainTool->y(tool_settings_y);
    mainGui->terrainTool->show();
    break;
  case editing_mode::flatten_blur:
    mainGui->flattenTool->x(tool_settings_x);
    mainGui->flattenTool->y(tool_settings_y);
    mainGui->flattenTool->show();
    break;
  case editing_mode::paint:
    mainGui->texturingTool->x(tool_settings_x);
    mainGui->texturingTool->y(tool_settings_y);
    mainGui->texturingTool->show();
    break;
  case editing_mode::areaid:
    mainGui->ZoneIDBrowser->x(tool_settings_x - 230);
    mainGui->ZoneIDBrowser->y(tool_settings_y);
    mainGui->ZoneIDBrowser->show();
    break;
  case editing_mode::water:
    mainGui->guiWater->x(tool_settings_x);
    mainGui->guiWater->y(tool_settings_y);
    mainGui->guiWater->show();
    break;
  case editing_mode::mccv:
    mainGui->shaderTool->x(tool_settings_x);
    mainGui->shaderTool->y(tool_settings_y);
    mainGui->shaderTool->show();
    break;
  case editing_mode::object:
    mainGui->objectEditor->x(tool_settings_x - 90.0f);
    mainGui->objectEditor->y(tool_settings_y);
    mainGui->objectEditor->show();
    mainGui->objectEditor->filename->show();
    mainGui->rotationEditor->x(mainGui->objectEditor->getX() - mainGui->rotationEditor->getW() - 10.0f);
    mainGui->rotationEditor->y(tool_settings_y);
  }

  terrainMode = mode;
  Environment::getInstance()->view_holelines = (mode == editing_mode::holes);
  mainGui->guiToolbar->IconSelect (mode);
}

void MapView::ResetSelectedObjectRotation()
{
  if (gWorld->IsSelection(eEntry_WMO))
  {
    WMOInstance* wmo = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection());
    gWorld->updateTilesWMO(wmo);
    wmo->resetDirection();
    gWorld->updateTilesWMO(wmo);
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    ModelInstance* m2 = boost::get<selected_model_type> (*gWorld->GetCurrentSelection());
    gWorld->updateTilesModel(m2);
    m2->resetDirection();
    m2->recalcExtents();
    gWorld->updateTilesModel(m2);
  }
}

void MapView::SnapSelectedObjectToGround()
{
  if (gWorld->IsSelection(eEntry_WMO))
  {
    WMOInstance* wmo = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection());
    math::vector_3d t = math::vector_3d(wmo->pos.x, wmo->pos.z, 0);
    gWorld->GetVertex(wmo->pos.x, wmo->pos.z, &t);
    wmo->pos.y = t.y;
    gWorld->updateTilesWMO(wmo);
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    ModelInstance* m2 = boost::get<selected_model_type> (*gWorld->GetCurrentSelection());
    math::vector_3d t = math::vector_3d(m2->pos.x, m2->pos.z, 0);
    gWorld->GetVertex(m2->pos.x, m2->pos.z, &t);
    m2->pos.y = t.y;
    gWorld->updateTilesModel(m2);
  }
}


void MapView::DeleteSelectedObject()
{
  if (gWorld->IsSelection(eEntry_WMO))
  {
    gWorld->deleteWMOInstance(boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->mUniqueID);
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    gWorld->deleteModelInstance(boost::get<selected_model_type> (*gWorld->GetCurrentSelection())->d1);
  }
}

/*!
\brief Import a new model form a text file or a hard coded one.
Imports a model from the import.txt (or the ImportFile from the config), the wowModelViewer log or just insert some hard coded testing models.
\param id the id switch the import kind
*/
void InsertObject(int id)
{
  //! \todo Beautify.

  // Test if there is an selection
  if (!gWorld->HasSelection())
    return;
  // the list of the models to import
  std::vector<std::string> m2s_to_add;
  std::vector<std::string> wmos_to_add;

  // the import file
  std::string importFile;

  const char* filesToAdd[15] = { "", "", "World\\Scale\\humanmalescale.m2", "World\\Scale\\50x50.m2", "World\\Scale\\100x100.m2", "World\\Scale\\250x250.m2", "World\\Scale\\500x500.m2", "World\\Scale\\1000x1000.m2", "World\\Scale\\50yardradiusdisc.m2", "World\\Scale\\200yardradiusdisc.m2", "World\\Scale\\777yardradiusdisc.m2", "World\\Scale\\50yardradiussphere.m2", "World\\Scale\\200yardradiussphere.m2", "World\\Scale\\777yardradiussphere.m2", "" };

  // MODELINSERT FROM TEXTFILE
  // is a source file set in config file?


  if (id == 0 || id == 14 || id == 15)
  {
    importFile = Settings::getInstance()->wmvLogFile;
  }
  else if (id == 1 || id > 99)
  {
    importFile = Settings::getInstance()->importFile;
  }
  else
  {
    m2s_to_add.push_back(filesToAdd[id]);
  }

  LogDebug << id << "-" << importFile << std::endl;

  std::string lastModel;
  std::string lastWMO;

  if (importFile != "")
  {
    size_t foundString;
    std::string line;
    std::string findThis;
    std::ifstream fileReader(importFile.c_str());
    int counter = 1;
    if (fileReader.is_open())
    {
      while (!fileReader.eof())
      {
        getline(fileReader, line);
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        if (line.find(".m2") != std::string::npos || line.find(".mdx") != std::string::npos)
        {
          if (id < 99 || (id - 99) == counter)
          {
            if (id == 14)
            {
              // M2 inside line
              // is it the modelviewer log then cut the log messages out
              findThis = "loading model: ";
              foundString = line.find(findThis);
              if (foundString != std::string::npos)
              {
                // cut path
                line = line.substr(foundString + findThis.size());
              }
              else
              {
                // invalid line
                continue;
              }
            }
            // swap mdx to m2
            size_t found = line.rfind(".mdx");
            if (found != std::string::npos)
            {
              line.replace(found, 4, ".m2");
            }

            line = line.substr(0, line.find(".m2") + 3);

            m2s_to_add.push_back(line);
            lastModel = line;
          }
          counter++;
        }
        else if (line.find(".wmo") != std::string::npos)
        {
          if (id < 99 || (id - 99) == counter)
          {
            if (id == 15)
            {
              // WMO inside line
              findThis = "loading wmo ";
              foundString = line.find(findThis);
              // is it the modelviewer log then cut the log messages out
              if (foundString != std::string::npos)
              {
                // cut path
                line = line.substr(foundString + findThis.size());
              }
              else
              {
                // invalid line
                continue;
              }
            }

            line = line.substr(0, line.find(".wmo") + 4);
            wmos_to_add.push_back(line);
            lastWMO = line;
          }
          counter++;
        }
      }
      fileReader.close();
    }
    else
    {
      // file not exist, no rights ore other error
      LogError << importFile << std::endl;
    }
  }


  math::vector_3d selectionPosition;
  switch (gWorld->GetCurrentSelection()->which())
  {
  case eEntry_Model:
    selectionPosition = boost::get<selected_model_type> (*gWorld->GetCurrentSelection())->pos;
    break;
  case eEntry_WMO:
    selectionPosition = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->pos;
    break;
  case eEntry_MapChunk:
    selectionPosition = boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).position;
    break;
  }


  if (id == 14)
  {
    LogDebug << "M2 Problem 14:" << lastModel << " - " << id << std::endl;
    if (lastModel != "")
    {
      if (!MPQFile::exists(lastModel))
      {
        LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
      }
      else
      {
        gWorld->addM2(lastModel, selectionPosition, false);
      }
    }
  }
  else if (id == 15)
  {
    LogDebug << "Wmo Problem 15:" << lastModel << " - " << id << std::endl;
    if (lastWMO != "")
    {
      if (!MPQFile::exists(lastWMO))
      {
        LogError << "Failed adding " << lastWMO << ". It was not in any MPQ." << std::endl;
      }
      else
      {
        gWorld->addWMO(lastWMO, selectionPosition, false);
      }
    }
  }
  else
  {

    for (std::vector<std::string>::iterator it = wmos_to_add.begin(); it != wmos_to_add.end(); ++it)
    {

      if (!MPQFile::exists(*it))
      {
        LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
        continue;
      }

      gWorld->addWMO(*it, selectionPosition, false);
    }

    for (std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it)
    {

      if (!MPQFile::exists(*it))
      {

        LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
        continue;
      }

      gWorld->addM2(*it, selectionPosition, false);
    }
  }
  //! \todo Memoryleak: These models will never get deleted.
}

void MapView::changeZoneIDValue (int set)
{
  Environment::getInstance()->selectedAreaID = set;
  if (Environment::getInstance()->areaIDColors.find(set) == Environment::getInstance()->areaIDColors.end())
  {
    math::vector_3d newColor = math::vector_3d(misc::randfloat(0.0f, 1.0f), misc::randfloat(0.0f, 1.0f), misc::randfloat(0.0f, 1.0f));
    Environment::getInstance()->areaIDColors.insert(std::pair<int, math::vector_3d>(set, newColor));
  }
}

std::string getCurrentHeightmapPath()
{
  // get MapName
  std::string mapName;
  int id = gWorld->getMapID();
  for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
  {
    if (i->getInt(MapDB::MapID) == id)
      mapName = i->getString(MapDB::InternalName);
  }

  // build the path and filename string.
  std::stringstream png_filename;
  png_filename << Project::getInstance()->getPath() << "world\\maps\\" << mapName << "\\H_" << mapName
    << "_" << misc::FtoIround((gWorld->camera.x - (TILESIZE / 2)) / TILESIZE) << "_" << misc::FtoIround((gWorld->camera.z - (TILESIZE / 2)) / TILESIZE) << ".png";
  return png_filename.str();

}

#ifdef __FILESAREMISSING
void exportPNG(UIFrame *f, int set)
{
  // create the image and write to disc.
  GLfloat* data = new GLfloat[272 * 272];

  ilInit();

  int width = 272;
  int height = 272;
  int bytesToUsePerPixel = 32;  // 16 bit per channel
  int sizeOfByte = sizeof(unsigned char);
  int theSize = width * height * sizeOfByte * bytesToUsePerPixel;

  unsigned char * imData = (unsigned char*)malloc(theSize);

  int colors = 0;
  // write the height data to the image array
  for (int i = 0; i < theSize; i++)
  {
    imData[i] = colors;
    if (i == 100)colors = 200;
    if (i == 200)colors = 4000;
  }


  ILuint ImageName; // The image name.
  ilGenImages(1, &ImageName); // Grab a new image name.
  ilBindImage(ImageName); // bind it
  ilTexImage(width, height, 1, bytesToUsePerPixel, GL_LUMINANCE, IL_UNSIGNED_BYTE, nullptr);
  ilSetData(imData);
  ilEnable(IL_FILE_OVERWRITE);
  //ilSave(IL_PNG, getCurrentHeightmapPath().c_str());
  ilSave(IL_PNG, "test2.png");
  free(imData);
}

void importPNG(UIFrame *f, int set)
{
  ilInit();

  //ILboolean loadImage = ilLoadImage( getCurrentHeightmapPath().c_str() ) ;
  const char *image = "test.png";
  ILboolean loadImage = ilLoadImage(image);

  std::stringstream MessageText;
  if (loadImage)
  {

    LogDebug << "Image loaded: " << image << "\n";
    LogDebug << "ImageSize: " << ilGetInteger(IL_IMAGE_SIZE_OF_DATA) << "\n";
    LogDebug << "BPP: " << ilGetInteger(IL_IMAGE_BITS_PER_PIXEL) << "\n";
    LogDebug << "Format: " << ilGetInteger(IL_IMAGE_FORMAT) << "\n";
    LogDebug << "SizeofData: " << ilGetInteger(IL_IMAGE_SIZE_OF_DATA) << "\n";

  }
  else
  {
    LogDebug << "Cant load Image: " << image << "\n";
    ILenum err = ilGetError();

    MessageText << err << "\n";
    //MessageText << ilGetString(ilGetError()) << "\n";
    LogDebug << MessageText.str();
  }
}
#else
void exportPNG(UIFrame*, int) {}
void importPNG(UIFrame*, int) {}
#endif

void MapView::createGUI()
{
  // create main gui object that holds all other gui elements for access ( in the future ;) )
  mainGui = new UIMapViewGUI(this);
  mainGui->guiCurrentTexture->current_texture->setClickFunc ([] { mainGui->TexturePalette->toggleVisibility(); });

  mainGui->ZoneIDBrowser->setMapID(gWorld->getMapID());
  mainGui->ZoneIDBrowser->setChangeFunc([this] (int id){ changeZoneIDValue (id); });
  tool_settings_x = video.xres() - 186;
  tool_settings_y = 38;



  mainGui->addChild(mainGui->TexturePalette = UITexturingGUI::createTexturePalette(mainGui));
  mainGui->TexturePalette->hide();
  mainGui->addChild(UITexturingGUI::createTilesetLoader());
  mainGui->addChild(UITexturingGUI::createTextureFilter());
  mainGui->addChild(MapChunkWindow = UITexturingGUI::createMapChunkWindow());
  MapChunkWindow->hide();

  // create the menu
  UIMenuBar * mbar = new UIMenuBar();

  mbar->AddMenu("File");
  mbar->AddMenu("Edit");
  mbar->AddMenu("View");
  mbar->AddMenu("Assist");
  mbar->AddMenu("Help");

  mbar->GetMenu( "File" )->AddMenuItemButton( "CTRL+SHIFT+S Save current", [] { mainGui->scWarning->show(); });
  mbar->GetMenu("File")->AddMenuItemButton("CTRL+S Save", [] { gWorld->mapIndex->saveChanged(); });
  mbar->GetMenu("File")->AddMenuItemButton("CTRL+SHIFT+A Save all", [] { gWorld->mapIndex->saveall(); });
  addHotkey(SDLK_s, MOD_ctrl + MOD_shift, [this] { mainGui->scWarning->show(); });
  addHotkey(SDLK_a, MOD_ctrl + MOD_shift, [this] { gWorld->mapIndex->saveall(); });
  addHotkey (SDLK_s, MOD_ctrl, [this] { gWorld->mapIndex->saveChanged(); });
  addHotkey (SDLK_s, MOD_meta, [this] { gWorld->mapIndex->saveChanged(); });
  mbar->GetMenu( "File" )->AddMenuItemButton( "SHIFT+J Reload tile", [] { gWorld->mapIndex->reloadTile(tile_index(gWorld->camera)); });
  addHotkey (SDLK_j, MOD_shift, [] { gWorld->mapIndex->reloadTile(tile_index(gWorld->camera)); });
  //  mbar->GetMenu( "File" )->AddMenuItemSeperator( "Import and Export" );
  // mbar->GetMenu( "File" )->AddMenuItemButton( "Export heightmap", exportPNG, 1 );
  // mbar->GetMenu( "File" )->AddMenuItemButton( "Import heightmap", importPNG, 1 );
  mbar->GetMenu("File")->AddMenuItemSeperator(" ");
  mbar->GetMenu("File")->AddMenuItemButton("ESC Exit", [] { mainGui->escWarning->show(); });
  addHotkey (SDLK_ESCAPE, MOD_none, [this] { mainGui->escWarning->show(); });

  mbar->GetMenu("Edit")->AddMenuItemSeperator("selected object");
  mbar->GetMenu("Edit")->AddMenuItemButton("DEL delete", [this] { DeleteSelectedObject(); });
  addHotkey (SDLK_DELETE, MOD_none, [this] { DeleteSelectedObject(); });
  mbar->GetMenu("Edit")->AddMenuItemButton("CTRL + R reset rotation", [this] { ResetSelectedObjectRotation(); });
  addHotkey (SDLK_r, MOD_ctrl, [this] { ResetSelectedObjectRotation(); });
  mbar->GetMenu("Edit")->AddMenuItemButton("PAGE DOWN set to ground", [this] { SnapSelectedObjectToGround(); });
  addHotkey (SDLK_PAGEDOWN, MOD_none, [this] { SnapSelectedObjectToGround(); });

  mbar->GetMenu("Edit")->AddMenuItemSeperator("Options");
  mbar->GetMenu("Edit")->AddMenuItemToggle("Auto select mode", &Settings::getInstance()->AutoSelectingMode, false);


  mbar->GetMenu("Assist")->AddMenuItemSeperator("Model");
  mbar->GetMenu("Assist")->AddMenuItemButton("Last M2 from MV", [] { InsertObject (14); });
  mbar->GetMenu("Assist")->AddMenuItemButton("Last WMO from MV", [] { InsertObject (15); });
  mbar->GetMenu("Assist")->AddMenuItemButton("Helper models", [] { mainGui->HelperModels->show(); });
  mbar->GetMenu("Assist")->AddMenuItemSeperator("Current ADT");
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Set Area ID"
                                             , []
                                               {
                                                 if (Environment::getInstance()->selectedAreaID)
                                                 {
                                                   gWorld->setAreaID(gWorld->camera, Environment::getInstance()->selectedAreaID, true);
                                                 }
                                               }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear height map"
                                             , []
                                               {
                                                 if (Environment::getInstance()->selectedAreaID)
                                                 {
                                                   gWorld->clearHeight(gWorld->camera);
                                                 }
                                               }
                                             );

  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear texture"
                                             , [] { gWorld->setBaseTexture(gWorld->camera); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear models"
                                             , [] { gWorld->clearAllModelsOnADT(gWorld->camera); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear duplicate models"
                                             , [] { gWorld->delete_duplicate_model_and_wmo_instances(); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemSeperator("Loaded ADTs");
  mbar->GetMenu("Assist")->AddMenuItemButton("Fix gaps (all loaded ADTs)", [] { gWorld->fixAllGaps(); });

  mbar->GetMenu("Assist")->AddMenuItemSeperator("Global");
  mbar->GetMenu("Assist")->AddMenuItemButton("Map to big alpha", [] { gWorld->convert_alphamap(true); });
  mbar->GetMenu("Assist")->AddMenuItemButton("Map to old", [] { gWorld->convert_alphamap(false); });

  mbar->GetMenu("View")->AddMenuItemSeperator("Windows");
  mbar->GetMenu("View")->AddMenuItemToggle("Toolbar", mainGui->guiToolbar->hidden_evil(), true);

  mbar->GetMenu("View")->AddMenuItemToggle("Texture palette", mainGui->TexturePalette->hidden_evil(), true);
  mbar->GetMenu("View")->AddMenuItemButton("Cursor options", [] { mainGui->showCursorSwitcher(); });
  mbar->GetMenu("View")->AddMenuItemSeperator("Toggle");
  mbar->GetMenu("View")->AddMenuItemToggle("F1 M2s", &_draw_models);
  addHotkey (SDLK_F1, MOD_none, [this] { _draw_models = !_draw_models; });
  mbar->GetMenu("View")->AddMenuItemToggle("F2 WMO doodadsets", &_draw_wmo_doodads);
  addHotkey (SDLK_F2, MOD_none, [this] { _draw_wmo_doodads = !_draw_wmo_doodads; });
  mbar->GetMenu("View")->AddMenuItemToggle("F3 Terrain", &_draw_terrain);
  addHotkey (SDLK_F3, MOD_none, [this] { _draw_terrain = !_draw_terrain; });
  mbar->GetMenu("View")->AddMenuItemToggle("F4 Water", &_draw_water);
  addHotkey (SDLK_F4, MOD_none, [this] { _draw_water = !_draw_water; });
  mbar->GetMenu("View")->AddMenuItemToggle("F6 WMOs", &_draw_wmo);
  addHotkey (SDLK_F6, MOD_none, [this] { _draw_wmo = !_draw_wmo; });
  mbar->GetMenu("View")->AddMenuItemToggle("F7 Lines", &_draw_lines);
  addHotkey (SDLK_F7, MOD_none, [this] { _draw_lines = !_draw_lines; });
  mbar->GetMenu("View")->AddMenuItemToggle("F8 Detail infos", mainGui->guidetailInfos->hidden_evil(), true);
  addHotkey (SDLK_F8, MOD_none, [this] { mainGui->guidetailInfos->toggleVisibility(); });
  mbar->GetMenu("View")->AddMenuItemToggle("F9 Map contour infos", &_draw_contour);
  addHotkey (SDLK_F9, MOD_none, [this] { _draw_contour = !_draw_contour; });
  mbar->GetMenu("View")->AddMenuItemToggle("F10 Wireframe", &_draw_wireframe);
  addHotkey(SDLK_F10, MOD_none, [this] { _draw_wireframe = !_draw_wireframe; });
  mbar->GetMenu("View")->AddMenuItemToggle("F11 Toggle Animation", &_draw_model_animations);
  addHotkey (SDLK_F11, MOD_none, [this] { _draw_model_animations = !_draw_model_animations; });
  mbar->GetMenu("View")->AddMenuItemToggle("F12 Fog", &gWorld->drawfog);
  addHotkey(SDLK_F12, MOD_none, [] { gWorld->drawfog = !gWorld->drawfog; });
  mbar->GetMenu("View")->AddMenuItemToggle("Flight Bounds", &_draw_mfbo);
  mbar->GetMenu("View")->AddMenuItemToggle("Hole lines always on", &Environment::getInstance()->view_holelines, false);
  mbar->GetMenu("View")->AddMenuItemToggle("Models with box", &Settings::getInstance()->renderModelsWithBox);

  mbar->GetMenu("Help")->AddMenuItemButton("H Key Bindings", [] { mainGui->showHelp(); });
#if defined(_WIN32) || defined(WIN32)
  mbar->GetMenu("Help")->AddMenuItemButton ( "Manual online"
                                           , []
                                             {
                                               ShellExecute ( nullptr
                                                            , "open"
                                                            , "http://modcraft.superparanoid.de/wiki/index.php5?title=Noggit_user_manual"
                                                            , nullptr
                                                            , nullptr
                                                            , SW_SHOWNORMAL
                                                            );
                                             }
                                           );
  mbar->GetMenu("Help")->AddMenuItemButton ( "Homepage"
                                           , []
                                             {
                                               ShellExecute ( nullptr
                                                            , "open"
                                                            , "http://modcraft.superparanoid.de"
                                                            , nullptr
                                                            , nullptr
                                                            , SW_SHOWNORMAL
                                                            );
                                             }
                                           );
#endif

  mainGui->addChild(mbar);

  addHotkey (SDLK_m, MOD_none, [this] { mainGui->minimapWindow->toggleVisibility(); });

  addHotkey ( SDLK_F1
            , MOD_shift
            , [this]
              {
                if (alloff)
                {
                  alloff_models = _draw_models;
                  alloff_doodads = _draw_wmo_doodads;
                  alloff_contour = _draw_contour;
                  alloff_wmo = _draw_wmo;
                  alloff_fog = gWorld->drawfog;
                  alloff_terrain = _draw_terrain;

                  _draw_models = false;
                  _draw_wmo_doodads = false;
                  _draw_contour = true;
                  _draw_wmo = false;
                  _draw_terrain = true;
                  gWorld->drawfog = false;
                }
                else
                {
                  _draw_models = alloff_models;
                  _draw_wmo_doodads = alloff_doodads;
                  _draw_contour = alloff_contour;
                  _draw_wmo = alloff_wmo;
                  _draw_terrain = alloff_terrain;
                  gWorld->drawfog = alloff_fog;
                }
                alloff = !alloff;
              }
            );

  addHotkey ( SDLK_F5
            , MOD_none
            , [this]
              {
                std::ofstream f("bookmarks.txt", std::ios_base::app);
                f << gWorld->getMapID() << " " << gWorld->camera.x << " " << gWorld->camera.y << " " << gWorld->camera.z << " " << _camera_ah << " " << _camera_av << " " << gWorld->getAreaID (gWorld->camera) << std::endl;
              }
            );

  addHotkey (SDLK_n, MOD_none, [this] { mTimespeed += 90.0f; });
  addHotkey (SDLK_b, MOD_none, [this] { mTimespeed = std::max (0.0f, mTimespeed - 90.0f); });
  addHotkey (SDLK_j, MOD_none, [this] { mTimespeed = 0.0f; });

  addHotkey (SDLK_TAB, MOD_none, [this] { _GUIDisplayingEnabled = !_GUIDisplayingEnabled; });

  addHotkey ( SDLK_c
            , MOD_ctrl
            , [this]
              {
                mainGui->objectEditor->copy (*gWorld->GetCurrentSelection());
              }
            , [this]
              {
                return !!gWorld->GetCurrentSelection();
              }
            );

  addHotkey ( SDLK_c
            , MOD_alt | MOD_ctrl
            , [this]
              {
                mainGui->toggleCursorSwitcher();
              }
            );

  addHotkey ( SDLK_c
            , MOD_none
            , [this]
              {
                mainGui->objectEditor->copy(*gWorld->GetCurrentSelection());
              }
            , [] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( SDLK_c
            , MOD_shift
            , [this]
              {
                cursor_type = ++cursor_type % 4;
              }
            , [] { return terrainMode != editing_mode::object; }
            );

  addHotkey (SDLK_v, MOD_shift, [] { InsertObject (14); });
  addHotkey (SDLK_v, MOD_alt, [] { InsertObject (15); });
  addHotkey (SDLK_v, MOD_ctrl, [this] { mainGui->objectEditor->pasteObject (_cursor_pos); });
  addHotkey ( SDLK_v
            , MOD_none
            , [this] { mainGui->objectEditor->pasteObject (_cursor_pos); }
            , [] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( SDLK_c
            , MOD_none
            , [] { gWorld->clearVertexSelection(); }
            , [] { return terrainMode == editing_mode::ground; }
            );

  addHotkey ( SDLK_x
            , MOD_none
            , [] { mainGui->TexturePalette->toggleVisibility(); }
            , [] { return terrainMode == editing_mode::paint; }
            );

  addHotkey (SDLK_F4, MOD_shift, [] { Settings::getInstance()->AutoSelectingMode = !Settings::getInstance()->AutoSelectingMode; });
  addHotkey (SDLK_F7, MOD_shift, [] { Environment::getInstance()->view_holelines = !Environment::getInstance()->view_holelines; });

  addHotkey (SDLK_x, MOD_ctrl, [] { mainGui->guidetailInfos->toggleVisibility(); });

  addHotkey (SDLK_i, MOD_none, [this] { mousedir *= -1.f; });

  addHotkey (SDLK_i, MOD_ctrl, [] { Environment::getInstance()->paintMode = !Environment::getInstance()->paintMode; });

  addHotkey (SDLK_o, MOD_none, [this] { movespd *= 0.5f; });
  addHotkey (SDLK_p, MOD_none, [this] { movespd *= 2.0f; });

  addHotkey (SDLK_p, MOD_shift | MOD_ctrl, [] { Saving = true; });

  addHotkey (SDLK_r, MOD_none, [this] { _camera_ah += 180.f; });

  addHotkey ( SDLK_g
            , MOD_none
            , []
              {
                // write teleport cords to txt file
                std::ofstream f("ports.txt", std::ios_base::app);
                f << "Map: " << gAreaDB.getAreaName(gWorld->getAreaID (gWorld->camera)) << " on ADT " << std::floor(gWorld->camera.x / TILESIZE) << " " << std::floor(gWorld->camera.z / TILESIZE) << std::endl;
                f << "Trinity:" << std::endl << ".go " << (ZEROPOINT - gWorld->camera.z) << " " << (ZEROPOINT - gWorld->camera.x) << " " << gWorld->camera.y << " " << gWorld->getMapID() << std::endl;
                f << "ArcEmu:" << std::endl << ".worldport " << gWorld->getMapID() << " " << (ZEROPOINT - gWorld->camera.z) << " " << (ZEROPOINT - gWorld->camera.x) << " " << gWorld->camera.y << " " << std::endl << std::endl;
                f.close();
              }
            );

  addHotkey ( SDLK_y
            , MOD_none
            , [] { mainGui->terrainTool->nextType(); }
            , [] { return terrainMode == editing_mode::ground; }
            );

  addHotkey ( SDLK_y
            , MOD_none
            , [] { mainGui->flattenTool->nextFlattenType(); }
            , [] { return terrainMode == editing_mode::flatten_blur; }
            );

  addHotkey ( SDLK_u
            , MOD_none
            , [this]
              {
                if (mViewMode == eViewMode_2D)
                {
                  mViewMode = eViewMode_3D;
                  set_editing_mode (saveterrainMode);
                }
                else
                {
                  mViewMode = eViewMode_2D;
                  saveterrainMode = terrainMode;
                  set_editing_mode (editing_mode::paint);
                }
              }
            );

  addHotkey ( SDLK_t
            , MOD_none
            , [&]
              {
                //! \todo space as global modifier?
                if (_mod_space_down)
                {
                  mainGui->flattenTool->nextFlattenMode();
                }
                else
                {
                  mainGui->flattenTool->toggleFlattenAngle();
                }
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );

  addHotkey ( SDLK_t
            , MOD_none
            , [&]
              {
                mainGui->texturingTool->toggle_spray();
              }
            , [&] { return terrainMode == editing_mode::paint; }
            );

  addHotkey ( SDLK_t
            , MOD_none
            , [&]
              {
                gWorld->setHoleADT (gWorld->camera, _mod_alt_down);
              }
            , [&] { return terrainMode == editing_mode::holes; }
            );

  addHotkey( SDLK_t
           , MOD_none
           , [&]
             {
               mainGui->guiWater->toggle_angled_mode();
             }
          , [&] { return terrainMode == editing_mode::water; }
          );


  addHotkey ( SDLK_t
            , MOD_none
            , [&]
              {
                mainGui->objectEditor->togglePasteMode();
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( SDLK_h
            , MOD_none
            , [&]
              {
                mainGui->toggleHelp();
              }
            , [&] { return terrainMode != editing_mode::object; }
            );

  addHotkey ( SDLK_h
            , MOD_none
            , [&]
              {
                // toggle hidden models visibility
                if (_mod_space_down)
                {
                  Environment::getInstance()->showModelFromHiddenList = !Environment::getInstance()->showModelFromHiddenList;
                }
                else if (_mod_shift_down)
                {
                  gWorld->clearHiddenModelList();
                }
                else
                {
                  // toggle selected model visibility
                  if (gWorld->HasSelection())
                  {
                    auto selection = gWorld->GetCurrentSelection();
                    if (selection->which() == eEntry_Model)
                    {
                      auto&& entity (boost::get<selected_model_type> (*selection)->model.get());
                      auto&& hidden (gWorld->_hidden_models);
                      if (hidden.count (entity))
                      {
                        hidden.erase (entity);
                      }
                      else
                      {
                        hidden.emplace (entity);
                      }
                    }
                    else if (selection->which() == eEntry_WMO)
                    {
                      auto&& entity (boost::get<selected_wmo_type> (*selection)->wmo.get());
                      auto&& hidden (gWorld->_hidden_map_objects);
                      if (hidden.count (entity))
                      {
                        hidden.erase (entity);
                      }
                      else
                      {
                        hidden.emplace (entity);
                      }
                    }
                  }
                }
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey ( SDLK_f
            , MOD_none
            , [&]
              {
                if (_mod_space_down)
                {
                  mainGui->terrainTool->flattenVertices();
                }
              }
            , [&] { return terrainMode == editing_mode::ground; }
            );
  addHotkey ( SDLK_f
            , MOD_none
            , [&]
              {
                if (_mod_space_down)
                {
                  mainGui->flattenTool->toggleFlattenLock();
                }
                else
                {
                  mainGui->flattenTool->lockPos (_cursor_pos);
                }
              }
            , [&] { return terrainMode == editing_mode::flatten_blur; }
            );
  addHotkey( SDLK_f
            , MOD_none
            , [&]
              {
                if (_mod_space_down)
                {
                  mainGui->guiWater->toggle_lock();
                }
                else
                {
                  mainGui->guiWater->lockPos(_cursor_pos);
                }
              }
          , [&] { return terrainMode == editing_mode::water; }
          );
  addHotkey ( SDLK_f
            , MOD_none
            , [&]
              {
                if (gWorld->HasSelection())
                {
                  auto selection = gWorld->GetCurrentSelection();

                  if (selection->which() == eEntry_Model)
                  {
                    gWorld->updateTilesModel(boost::get<selected_model_type> (*selection));
                    boost::get<selected_model_type> (*selection)->pos = _cursor_pos;
                    boost::get<selected_model_type> (*selection)->recalcExtents();
                    gWorld->updateTilesModel(boost::get<selected_model_type> (*selection));
                  }
                  else if (selection->which() == eEntry_WMO)
                  {
                    gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*selection));
                    boost::get<selected_wmo_type> (*selection)->pos = _cursor_pos;
                    boost::get<selected_wmo_type> (*selection)->recalcExtents();
                    gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*selection));
                  }
                }
              }
            , [&] { return terrainMode == editing_mode::object; }
            );

  addHotkey (SDLK_KP_PLUS, MOD_alt, [] { mainGui->terrainTool->changeRadius(0.01f); }, [] { return terrainMode == editing_mode::ground; });
  addHotkey (SDLK_PLUS, MOD_alt, [] { mainGui->terrainTool->changeRadius(0.01f); }, [] { return terrainMode == editing_mode::ground; });

  addHotkey (SDLK_KP_PLUS, MOD_alt, [] { mainGui->flattenTool->changeRadius(0.01f); }, [] { return terrainMode == editing_mode::flatten_blur; });
  addHotkey (SDLK_PLUS, MOD_alt, [] { mainGui->flattenTool->changeRadius(0.01f); }, [] { return terrainMode == editing_mode::flatten_blur; });

  addHotkey ( SDLK_KP_PLUS
            , MOD_alt
            , [&]
              {
                mainGui->texturingTool->change_radius(0.1f);
              }
            , [] { return terrainMode == editing_mode::paint; }
            );
  addHotkey ( SDLK_PLUS
            , MOD_alt
            , [&]
              {
                mainGui->texturingTool->change_radius(0.1f);
              }
            , [] { return terrainMode == editing_mode::paint; }
            );

  addHotkey (SDLK_KP_PLUS, MOD_shift, [] { gWorld->fogdistance += 60.0f; });
  addHotkey (SDLK_PLUS, MOD_shift, [] { gWorld->fogdistance += 60.0f; });


  addHotkey (SDLK_KP_MINUS, MOD_alt, [] { mainGui->terrainTool->changeRadius(-0.01f); }, [] { return terrainMode == editing_mode::ground; });
  addHotkey (SDLK_MINUS, MOD_alt, [] { mainGui->terrainTool->changeRadius(-0.01f); }, [] { return terrainMode == editing_mode::ground; });

  addHotkey (SDLK_KP_MINUS, MOD_alt, [] { mainGui->flattenTool->changeRadius(-0.01f); }, [] { return terrainMode == editing_mode::flatten_blur; });
  addHotkey (SDLK_MINUS, MOD_alt, [] { mainGui->flattenTool->changeRadius(-0.01f); }, [] { return terrainMode == editing_mode::flatten_blur; });

  addHotkey ( SDLK_KP_MINUS
            , MOD_alt
            , [&]
              {
                mainGui->texturingTool->change_radius(-0.1f);
              }
            , [] { return terrainMode == editing_mode::paint; }
            );
  addHotkey ( SDLK_MINUS
            , MOD_alt
            , [&]
              {
                mainGui->texturingTool->change_radius(-0.1f);
              }
            , [] { return terrainMode == editing_mode::paint; }
            );

  addHotkey (SDLK_KP_MINUS, MOD_shift, [] { gWorld->fogdistance -= 60.0f; });
  addHotkey (SDLK_MINUS, MOD_shift, [] { gWorld->fogdistance -= 60.0f; });

  addHotkey (SDLK_1, MOD_shift, [this] { movespd = 15.0f; });
  addHotkey (SDLK_2, MOD_shift, [this] { movespd = 50.0f; });
  addHotkey (SDLK_3, MOD_shift, [this] { movespd = 200.0f; });
  addHotkey (SDLK_4, MOD_shift, [this] { movespd = 800.0f; });
  addHotkey (SDLK_1, MOD_alt, [] { mainGui->texturingTool->set_brush_level(0.0f); });
  addHotkey (SDLK_2, MOD_alt, [] { mainGui->texturingTool->set_brush_level(255.0f* 0.25f); });
  addHotkey (SDLK_3, MOD_alt, [] { mainGui->texturingTool->set_brush_level(255.0f* 0.5f); });
  addHotkey (SDLK_4, MOD_alt, [] { mainGui->texturingTool->set_brush_level(255.0f* 0.75f); });
  addHotkey (SDLK_5, MOD_alt, [] { mainGui->texturingTool->set_brush_level(255.0f); });

  addHotkey (SDLK_1, MOD_none, [this] { set_editing_mode (editing_mode::ground); });
  addHotkey (SDLK_2, MOD_none, [this] { set_editing_mode (editing_mode::flatten_blur); });
  addHotkey (SDLK_3, MOD_none, [this] { set_editing_mode (editing_mode::paint); });
  addHotkey (SDLK_4, MOD_none, [this] { set_editing_mode (editing_mode::holes); });
  addHotkey (SDLK_5, MOD_none, [this] { set_editing_mode (editing_mode::areaid); });
  addHotkey (SDLK_6, MOD_none, [this] { set_editing_mode (editing_mode::flags); });
  addHotkey (SDLK_7, MOD_none, [this] { set_editing_mode (editing_mode::water); });
  addHotkey (SDLK_8, MOD_none, [this] { set_editing_mode (editing_mode::light); });
  addHotkey (SDLK_9, MOD_none, [this] { set_editing_mode (editing_mode::mccv); });
  addHotkey (SDLK_0, MOD_none, [this] { set_editing_mode (editing_mode::object); });

  addHotkey (SDLK_0, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 0; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_1, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 1; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_2, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 2; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_3, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 3; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_4, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 4; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_5, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 5; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_6, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 6; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_7, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 7; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_8, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 8; }, [] { return gWorld->IsSelection(eEntry_WMO); });
  addHotkey (SDLK_9, MOD_ctrl, [] { boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = 9; }, [] { return gWorld->IsSelection(eEntry_WMO); });


  // ESC warning
  mainGui->escWarning = new UIExitWarning(this);
  mainGui->escWarning->hide();
  mainGui->addChild(mainGui->escWarning);

  // CAPS warning
  mainGui->capsWarning = new UICapsWarning;
  mainGui->capsWarning->hide();
  mainGui->addChild(mainGui->capsWarning);

  // Save current warning
  mainGui->scWarning = new UISaveCurrentWarning (mainGui);
  mainGui->scWarning->hide();
  mainGui->addChild(mainGui->scWarning);

  // Water unable to save warning
  mainGui->waterSaveWarning = new UIWaterSaveWarning;
  mainGui->waterSaveWarning->hide();
  mainGui->addChild(mainGui->waterSaveWarning);

  // modelimport
  mainGui->objectEditor->modelImport = new UIModelImport(this);
  mainGui->objectEditor->modelImport->hide();
  mainGui->addChild(mainGui->objectEditor->modelImport);

  // helper models
  mainGui->HelperModels = new UIHelperModels(this);
  mainGui->HelperModels->hide();
  mainGui->HelperModels->movable(true);
  mainGui->addChild(mainGui->HelperModels);
}

MapView::MapView(float _camera_ah0, float _camera_av0, math::vector_3d camera_lookat)
  : _camera_ah(_camera_ah0)
  , _camera_av(_camera_av0)
  , _camera_lookat (camera_lookat)
  , _GUIDisplayingEnabled(true)
  , mTimespeed(0.0f)
{
  LastClicked = nullptr;

  // load cursor settings
  if (boost::filesystem::exists("noggit.conf"))
  {
    ConfigFile myConfigfile = ConfigFile("noggit.conf");
    if (myConfigfile.keyExists("RedColor") && myConfigfile.keyExists("GreenColor") && myConfigfile.keyExists("BlueColor") && myConfigfile.keyExists("AlphaColor"))
    {
      cursor_color.x = myConfigfile.read<float>("RedColor");
      cursor_color.y = myConfigfile.read<float>("GreenColor");
      cursor_color.z = myConfigfile.read<float>("BlueColor");
      cursor_color.w = myConfigfile.read<float>("AlphaColor");
    }

    if (myConfigfile.keyExists("CursorType"))
    {
      cursor_type = myConfigfile.read<int>("CursorType");
    }
  }

  moving = strafing = updown = lookat = turn = 0.0f;

  mousedir = -1.0f;

  movespd = SPEED;

  lastBrushUpdate = 0;

  look = false;
  mViewMode = eViewMode_3D;

  createGUI();

  set_editing_mode (editing_mode::ground);

  // Set camera y (height) position to current ground height plus some space.
  math::vector_3d t = math::vector_3d(0, 0, 0);
  tile_index tile(gWorld->camera);
  if (!gWorld->mapIndex->tileLoaded(tile))
  {
    gWorld->mapIndex->loadTile(tile);
  }

  gWorld->GetVertex(gWorld->camera.x, gWorld->camera.z, &t);

  // min elevation according to https://wowdev.wiki/AreaTable.dbc
  //! \ todo use the current area's MinElevation
  if (t.y < -5000.0f)
  {
    //! \todo use the height of a model/wmo of the tile (or the map) ?
    t.y = 0.0f;
  }

  gWorld->camera.y = t.y + 50.0f;
}

MapView::~MapView()
{
  delete mainGui;
  mainGui = nullptr;
  delete gWorld;
  gWorld = nullptr;
}

void MapView::tick(float t, float dt)
{
  // start unloading tiles
  gWorld->mapIndex->enterTile (tile_index (gWorld->camera));
  gWorld->mapIndex->unloadTiles (tile_index (gWorld->camera));

  dt = std::min(dt, 1.0f);

  update_cursor_pos();

  // write some stuff into infos window for debuging
  std::stringstream appinfoText;
  appinfoText << "Project Path: " << Project::getInstance()->getPath() << std::endl;
  appinfoText << "Current cursor: " << cursor_type << std::endl;
  mainGui->guiappInfo->setText(appinfoText.str());
#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      mainGui->terrainTool->setTabletControlValue((float)app.pressure);
    case editing_mode::flatten_blur:
      mainGui->flattenTool->setRadius((float)app.pressure / 20.0f);
      break;
    case editing_mode::paint:
      mainGui->texturingTool->change_pressure((float)app.pressure / 2048.0f);
      break;
    case editing_mode::mccv:
      mainGui->shaderTool->setTabletControlValue((float)app.pressure);
      break;
    }
  }
#endif

  if (SDL_GetAppState() & SDL_APPINPUTFOCUS)
  {
    math::vector_3d dir(1.0f, 0.0f, 0.0f);
    math::vector_3d dirUp(1.0f, 0.0f, 0.0f);
    math::vector_3d dirRight(0.0f, 0.0f, 1.0f);
    math::rotate(0.0f, 0.0f, &dir.x, &dir.y, math::degrees(_camera_av));
    math::rotate(0.0f, 0.0f, &dir.x, &dir.z, math::degrees(_camera_ah));

    if (_mod_shift_down)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      dirRight *= 0.0f; //! \todo  WAT?
    }
    else if (_mod_ctrl_down)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.y, math::degrees(_camera_av));
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.y, math::degrees(_camera_av));
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, math::degrees(_camera_ah));
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, math::degrees(_camera_ah));
    }
    else
    {
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, math::degrees(_camera_ah));
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, math::degrees(_camera_ah));
    }
    auto Selection = gWorld->GetCurrentSelection();
    if (Selection)
    {
      // update rotation editor if the selection has changed
      if (!lastSelected || lastSelected != Selection)
      {
        mainGui->rotationEditor->select(*Selection);
      }

      bool canMoveObj = !mainGui->rotationEditor->hasFocus();

      // Set move scale and rotate for numpad keys
      if (_mod_ctrl_down && _mod_shift_down)  moveratio = 0.1f;
      else if (_mod_shift_down) moveratio = 0.01f;
      else if (_mod_ctrl_down) moveratio = 0.005f;
      else moveratio = 0.001f;

      if (canMoveObj && (keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0))
      {
        // Move scale and rotate with numpad keys
        if (Selection->which() == eEntry_WMO)
        {
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          boost::get<selected_wmo_type> (*Selection)->pos.x += keyx * moveratio;
          boost::get<selected_wmo_type> (*Selection)->pos.y += keyy * moveratio;
          boost::get<selected_wmo_type> (*Selection)->pos.z += keyz * moveratio;
          boost::get<selected_wmo_type> (*Selection)->dir.y += keyr * moveratio * 5;

          boost::get<selected_wmo_type> (*Selection)->recalcExtents();
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          mainGui->rotationEditor->updateValues();
        }

        if (Selection->which() == eEntry_Model)
        {
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          boost::get<selected_model_type> (*Selection)->pos.x += keyx * moveratio;
          boost::get<selected_model_type> (*Selection)->pos.y += keyy * moveratio;
          boost::get<selected_model_type> (*Selection)->pos.z += keyz * moveratio;
          boost::get<selected_model_type> (*Selection)->dir.y += keyr * moveratio * 5;
          boost::get<selected_model_type> (*Selection)->sc += keys * moveratio / 50;
          boost::get<selected_model_type> (*Selection)->recalcExtents();
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          mainGui->rotationEditor->updateValues();
        }
      }

      math::vector_3d ObjPos;
      if (gWorld->IsSelection(eEntry_Model))
      {
        //! \todo  Tell me what this is.
        ObjPos = boost::get<selected_model_type> (*Selection)->pos - gWorld->camera;
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.y, math::degrees(_camera_av));
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.z, math::degrees(_camera_ah));
        ObjPos.x = std::abs(ObjPos.x);
      }

      // moving and scaling objects
      //! \todo  Alternatively automatically align it to the terrain.
      if (MoveObj && canMoveObj)
      {
        ObjPos.x = 80.0f;
        if (Selection->which() == eEntry_WMO)
        {
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));

          if (_mod_shift_down)
          {
            boost::get<selected_wmo_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
            boost::get<selected_wmo_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
          }
          else
          {
            if (Environment::getInstance()->moveModelToCursorPos)
            {
              boost::get<selected_wmo_type> (*Selection)->pos.x = _cursor_pos.x - objMoveOffset.x;
              boost::get<selected_wmo_type> (*Selection)->pos.z = _cursor_pos.z - objMoveOffset.z;
            }
            else
            {
              boost::get<selected_wmo_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
              boost::get<selected_wmo_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
            }
          }

          boost::get<selected_wmo_type> (*Selection)->recalcExtents();
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          mainGui->rotationEditor->updateValues();
        }
        else if (Selection->which() == eEntry_Model)
        {
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          if (_mod_alt_down)
          {
            float ScaleAmount = pow(2.0f, mv * 4.0f);

            boost::get<selected_model_type> (*Selection)->sc *= ScaleAmount;
            if (boost::get<selected_model_type> (*Selection)->sc > 63.9f)
              boost::get<selected_model_type> (*Selection)->sc = 63.9f;
            else if (boost::get<selected_model_type> (*Selection)->sc < 0.00098f)
              boost::get<selected_model_type> (*Selection)->sc = 0.00098f;
          }
          else
          {
            if (_mod_shift_down)
            {
              boost::get<selected_model_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
              boost::get<selected_model_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
            }
            else
            {
              if (Environment::getInstance()->moveModelToCursorPos)
              {
                boost::get<selected_model_type> (*Selection)->pos.x = _cursor_pos.x - objMoveOffset.x;
                boost::get<selected_model_type> (*Selection)->pos.z = _cursor_pos.z - objMoveOffset.z;
              }
              else
              {
                boost::get<selected_model_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
                boost::get<selected_model_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
              }

            }
          }

          mainGui->rotationEditor->updateValues();
          boost::get<selected_model_type> (*Selection)->recalcExtents();
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
        }
      }


      // rotating objects
      if (look && canMoveObj)
      {
        float * lTarget = nullptr;
        bool lModify = false;

        if (Selection->which() == eEntry_Model)
        {
          lModify = _mod_shift_down | _mod_ctrl_down | _mod_alt_down;
          if (_mod_shift_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.y;
          else if (_mod_ctrl_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.x;
          else if (_mod_alt_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.z;

        }
        else if (Selection->which() == eEntry_WMO)
        {
          lModify = _mod_shift_down | _mod_ctrl_down | _mod_alt_down;
          if (_mod_shift_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.y;
          else if (_mod_ctrl_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.x;
          else if (_mod_alt_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.z;

        }

        if (lModify && lTarget)
        {
          gWorld->updateTilesEntry(*Selection);

          *lTarget = *lTarget + rh + rv;

          if (*lTarget > 360.0f)
            *lTarget = *lTarget - 360.0f;
          else if (*lTarget < -360.0f)
            *lTarget = *lTarget + 360.0f;

          mainGui->rotationEditor->updateValues();

          if (Selection->which() == eEntry_WMO)
          {
            boost::get<selected_wmo_type> (*Selection)->recalcExtents();
            gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          }
          else if (Selection->which() == eEntry_Model)
          {
            boost::get<selected_model_type> (*Selection)->recalcExtents();
            gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          }
        }
      }

      mh = 0;
      mv = 0;
      rh = 0;
      rv = 0;


      if (leftMouse && Selection->which() == eEntry_MapChunk)
      {
        bool underMap = gWorld->isUnderMap(_cursor_pos);

        switch (terrainMode)
        {
        case editing_mode::ground:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->terrainTool->changeTerrain(_cursor_pos, 7.5f * dt);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->terrainTool->changeTerrain(_cursor_pos, -7.5f * dt);
            }
          }
          break;
        case editing_mode::flatten_blur:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->flattenTool->flatten(_cursor_pos, dt);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->flattenTool->blur(_cursor_pos, dt);
            }
          }
          break;
        case editing_mode::paint:
          if (_mod_shift_down && _mod_ctrl_down && _mod_alt_down)
          {
            // clear chunk texture
            if (mViewMode == eViewMode_3D && !underMap)
              gWorld->eraseTextures(_cursor_pos);
            else if (mViewMode == eViewMode_2D)
              gWorld->eraseTextures({CHUNKSIZE * 4.0f * video.ratio() * (static_cast<float>(MouseX) / static_cast<float>(video.xres()) - 0.5f) / _2d_zoom + gWorld->camera.x, 0.f, CHUNKSIZE * 4.0f * (static_cast<float>(MouseY) / static_cast<float>(video.yres()) - 0.5f) / _2d_zoom + gWorld->camera.z});
          }
          else if (_mod_ctrl_down)
          {
            // Pick texture
            mainGui->TexturePicker->getTextures(*gWorld->GetCurrentSelection());
          }
          else  if (_mod_shift_down && !!UITexturingGUI::getSelectedTexture())
          {
            if (mViewMode == eViewMode_3D && !underMap)
            {
              mainGui->texturingTool->paint(_cursor_pos, dt, *UITexturingGUI::getSelectedTexture());
            }
            else if (mViewMode == eViewMode_2D)
            {
              math::vector_3d pos( CHUNKSIZE * 4.0f * video.ratio() * ((float)MouseX / (float)video.xres() - 0.5f ) / _2d_zoom
                                  , 0.0f
                                  , CHUNKSIZE * 4.0f * ((float)MouseY / (float)video.yres() - 0.5f) / _2d_zoom
                                  );

              pos += gWorld->camera;
              mainGui->texturingTool->paint(pos, dt, *UITexturingGUI::getSelectedTexture());
            }
          }
          break;

        case editing_mode::holes:
          if (mViewMode == eViewMode_3D)
          {
            // no undermap check here, else it's impossible to remove holes
            if (_mod_shift_down)
            {
              auto pos (boost::get<selected_chunk_type> (*Selection).position);
              gWorld->setHole(pos, _mod_alt_down, false);
            }
            else if (_mod_ctrl_down && !underMap)
            {
              gWorld->setHole(_cursor_pos, _mod_alt_down, true);
            }
          }
          break;
        case editing_mode::areaid:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              // draw the selected AreaId on current selected chunk
              gWorld->setAreaID(_cursor_pos, Environment::getInstance()->selectedAreaID, false);
            }
            else if (_mod_ctrl_down)
            {
              // pick areaID from chunk
              MapChunk* chnk (boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).chunk);
              int newID = chnk->getAreaID();
              Environment::getInstance()->selectedAreaID = newID;
              mainGui->ZoneIDBrowser->setZoneID(newID);
            }
          }
          break;
        case editing_mode::flags:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              gWorld->mapIndex->setFlag(true, _cursor_pos);
            }
            else if (_mod_ctrl_down)
            {
              gWorld->mapIndex->setFlag(false, _cursor_pos);
            }
          }
          break;
        case editing_mode::water:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->guiWater->paintLiquid(_cursor_pos, true);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->guiWater->paintLiquid(_cursor_pos, false);
            }
          }
          break;
        case editing_mode::mccv:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->shaderTool->changeShader(_cursor_pos, dt, true);
            }
            if (_mod_ctrl_down)
            {
              mainGui->shaderTool->changeShader(_cursor_pos, dt, false);
            }
          }
          break;
        }
      }
    }

    if (mViewMode != eViewMode_2D)
    {
      if (turn != 0.0f)
      {
        _camera_ah += turn;
        mainGui->minimapWindow->changePlayerLookAt(math::degrees (_camera_ah));
      }
      if (lookat)
      {
        _camera_av += lookat;
        if (_camera_av < -80.0f)
          _camera_av = -80.0f;
        else if (_camera_av > 80.0f)
          _camera_av = 80.0f;
        mainGui->minimapWindow->changePlayerLookAt(math::degrees (_camera_ah));
      }
      if (moving)
        gWorld->camera += dir * dt * movespd * moving;
      if (strafing)
      {
        math::vector_3d right = dir % math::vector_3d(0.0f, 1.0f, 0.0f);
        right.normalize();
        gWorld->camera += right * dt * movespd * strafing;
      }
      if (updown)
        gWorld->camera += math::vector_3d(0.0f, dt * movespd * updown, 0.0f);

      _camera_lookat = gWorld->camera + dir;
    }
    else
    {
      if (moving)
        gWorld->camera.z -= dt * movespd * moving / (_2d_zoom * 1.5f);
      if (strafing)
        gWorld->camera.x += dt * movespd * strafing / (_2d_zoom * 1.5f);
      if (updown)
        _2d_zoom *= pow(2.0f, dt * updown * 4.0f);

      _2d_zoom = std::min(std::max(_2d_zoom, 0.1f), 2.0f);
    }
  }
  else
  {
    leftMouse = false;
    rightMouse = false;
    look = false;
    MoveObj = false;

    moving = 0;
    strafing = 0;
    updown = 0;
  }

  if ((t - lastBrushUpdate) > 0.1f)
  {
    mainGui->texturingTool->update_brushes();
  }


  gWorld->time += this->mTimespeed * dt;


  gWorld->animtime += dt * 1000.0f;
  globalTime = static_cast<int>(gWorld->animtime);

  gWorld->tick(dt);

  lastSelected = gWorld->GetCurrentSelection();

  if (!MapChunkWindow->hidden() && gWorld->GetCurrentSelection() && gWorld->GetCurrentSelection()->which() == eEntry_MapChunk)
  {
    UITexturingGUI::setChunkWindow(boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).chunk);
  }
}

selection_result MapView::intersect_result(bool terrain_only)
{
  math::vector_3d const pos
    ( ( ( math::look_at ( gWorld->camera
                        , _camera_lookat
                        , { 0.0f, 1.0f, 0.0f }
                        ).transposed()
        * math::perspective ( video.fov()
                            , video.ratio()
                            , video.nearclip()
                            , video.farclip()
                            ).transposed()
        ).inverted().transposed()
      * video.normalized_device_coords ( Environment::getInstance()->screenX
                                       , Environment::getInstance()->screenY
                                       )
      ).xyz_normalized_by_w()
    );

  math::ray ray (gWorld->camera, pos - gWorld->camera);

  selection_result results
    ( gWorld->intersect ( ray
                        , terrain_only
                        , terrainMode == editing_mode::object
                        , _draw_terrain
                        , _draw_wmo
                        , _draw_models
                        )
    );

  std::sort ( results.begin()
            , results.end()
            , [](selection_entry const& lhs, selection_entry const& rhs)
              {
                return lhs.first < rhs.first;
              }
            );

  return results;
}

void MapView::doSelection (bool selectTerrainOnly)
{
  selection_result results(intersect_result(selectTerrainOnly));

  if (results.empty())
  {
    gWorld->SetCurrentSelection (boost::none);
  }
  else
  {
    auto const& hit (results.front().second);
    gWorld->SetCurrentSelection (hit);

    _cursor_pos = hit.which() == eEntry_Model ? boost::get<selected_model_type>(hit)->pos
      : hit.which() == eEntry_WMO ? boost::get<selected_wmo_type>(hit)->pos
      : hit.which() == eEntry_MapChunk ? boost::get<selected_chunk_type>(hit).position
      : throw std::logic_error("bad variant");
  }
}

void MapView::update_cursor_pos()
{
  selection_result results (intersect_result (true));

  if (!results.empty())
  {
    auto const& hit(results.front().second);
    // hit cannot be something else than a chunk
    _cursor_pos = boost::get<selected_chunk_type>(hit).position;
  }
}

void MapView::displayGUIIfEnabled()
{
  if (_GUIDisplayingEnabled)
  {
    video.set2D();

    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    opengl::texture::disable_texture(1);
    opengl::texture::enable_texture(0);

    gl.disable(GL_DEPTH_TEST);
    gl.disable(GL_CULL_FACE);
    gl.disable(GL_LIGHTING);
    gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);

    opengl::texture::disable_texture(0);

    mainGui->setTilemode(mViewMode != eViewMode_3D);
    mainGui->render();

    opengl::texture::enable_texture(0);
  }
}

void MapView::displayViewMode_2D(float /*t*/, float /*dt*/)
{
  video.setTileMode();
  gWorld->drawTileMode ( _camera_ah
                       , gWorld->camera
                       , _draw_lines
                       , _2d_zoom
                       );


  const float mX = (CHUNKSIZE * 4.0f * video.ratio() * (static_cast<float>(MouseX) / static_cast<float>(video.xres()) - 0.5f) / _2d_zoom + gWorld->camera.x) / CHUNKSIZE;
  const float mY = (CHUNKSIZE * 4.0f * (static_cast<float>(MouseY) / static_cast<float>(video.yres()) - 0.5f) / _2d_zoom + gWorld->camera.z) / CHUNKSIZE;

  // draw brush
  {
    opengl::scoped::matrix_pusher const matrix;

    gl.scalef(_2d_zoom, _2d_zoom, 1.0f);
    gl.translatef(-gWorld->camera.x / CHUNKSIZE, -gWorld->camera.z / CHUNKSIZE, 0);

    gl.color4f(1.0f, 1.0f, 1.0f, 0.5f);
    opengl::texture::set_active_texture(1);
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture(0);
    opengl::texture::enable_texture();

    mainGui->texturingTool->bind_brush_texture();

    const float tRadius = mainGui->texturingTool->brush_radius() / CHUNKSIZE;// *_2d_zoom;
    gl.begin(GL_QUADS);
    gl.texCoord2f(0.0f, 0.0f);
    gl.vertex3f(mX - tRadius, mY + tRadius, 0);
    gl.texCoord2f(1.0f, 0.0f);
    gl.vertex3f(mX + tRadius, mY + tRadius, 0);
    gl.texCoord2f(1.0f, 1.0f);
    gl.vertex3f(mX + tRadius, mY - tRadius, 0);
    gl.texCoord2f(0.0f, 1.0f);
    gl.vertex3f(mX - tRadius, mY - tRadius, 0);
    gl.end();
  }

  displayGUIIfEnabled();
}

void MapView::displayViewMode_3D(float /*t*/, float /*dt*/)
{
  //! \note Select terrain below mouse, if no item selected or the item is map.
  if (!gWorld->IsSelection(eEntry_Model) && !gWorld->IsSelection(eEntry_WMO) && Settings::getInstance()->AutoSelectingMode)
  {
    doSelection(true);
  }

  video.set3D();

  //! \ todo: make the current tool return the radius
  float radius = 0.0f, hardness = 0.0f, inner_radius = 0.0f, angle = 0.0f, orientation = 0.0f;
  math::vector_3d ref_pos;
  bool angled_mode = false, use_ref_pos = false;

  switch (terrainMode)
  {
  case editing_mode::ground:
    radius = mainGui->terrainTool->brushRadius();
    inner_radius = mainGui->terrainTool->innerRadius();
    break;
  case editing_mode::flatten_blur:
    radius = mainGui->flattenTool->brushRadius();
    angle = mainGui->flattenTool->angle();
    orientation = mainGui->flattenTool->orientation();
    ref_pos = mainGui->flattenTool->ref_pos();
    angled_mode = mainGui->flattenTool->angled_mode();
    use_ref_pos = mainGui->flattenTool->use_ref_pos();
    break;
  case editing_mode::paint:
    radius = mainGui->texturingTool->brush_radius();
    hardness = mainGui->texturingTool->hardness();
    break;
  case editing_mode::water:
    radius = mainGui->guiWater->brushRadius();
    angle = mainGui->guiWater->angle();
    orientation = mainGui->guiWater->orientation();
    ref_pos = mainGui->guiWater->ref_pos();
    angled_mode = mainGui->guiWater->angled_mode();
    use_ref_pos = mainGui->guiWater->use_ref_pos();
    break;
  case editing_mode::mccv:
    radius = mainGui->shaderTool->brushRadius();
    break;
  }

  gWorld->draw ( _cursor_pos
               , cursor_color
               , cursor_type
               , radius
               , hardness
               , mainGui->texturingTool->highlight_paintable_chunks()
               , _draw_contour
               , inner_radius
               , ref_pos
               , angle
               , orientation
               , use_ref_pos
               , angled_mode
               , terrainMode == editing_mode::paint
               , terrainMode == editing_mode::flags
               , terrainMode == editing_mode::water
               , terrainMode == editing_mode::areaid
               , terrainMode
               , gWorld->camera
               , _camera_lookat
               , _draw_mfbo
               , _draw_wireframe
               , _draw_lines
               , _draw_terrain
               , _draw_wmo
               , _draw_water
               , _draw_wmo_doodads
               , _draw_models
               , _draw_model_animations
               );

  displayGUIIfEnabled();
}

void MapView::display(float t, float dt)
{
  //! \todo  Get this out or do it somehow else. This is ugly and is a senseless if each draw.
  if (Saving)
  {
    video.setTileMode();
    gWorld->saveMap();
    Saving = false;
  }

  switch (mViewMode)
  {
  case eViewMode_2D:
    displayViewMode_2D(t, dt);
    break;

  case eViewMode_3D:
    displayViewMode_3D(t, dt);
    break;
  }
}

void MapView::quit()
{
  app.pop = true;
}

void MapView::resizewindow()
{
  mainGui->resize();
}

void MapView::keyPressEvent (SDL_KeyboardEvent *e)
{
  if (LastClicked && LastClicked->key_down (e->keysym.sym, e->keysym.unicode))
  {
    return;
  }

  if (e->keysym.mod & KMOD_CAPS)
    mainGui->capsWarning->show();
  else
    mainGui->capsWarning->hide();

  if (handleHotkeys(e))
    return;

  if (e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT)
    _mod_shift_down = true;

  if (e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT)
    _mod_alt_down = true;

  if (e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL)
    _mod_ctrl_down = true;

  if (e->keysym.sym == SDLK_SPACE)
    _mod_space_down = true;

  // movement
  if (e->keysym.sym == SDLK_w)
  {
    key_w = true;
    moving = 1.0f;
  }

  if (e->keysym.sym == SDLK_UP)
  {
    lookat = 0.75f;
  }

  if (e->keysym.sym == SDLK_DOWN)
  {
    lookat = -0.75f;
  }

  if (e->keysym.sym == SDLK_LEFT)
  {
    turn = -0.75f;
  }

  if (e->keysym.sym == SDLK_RIGHT)
  {
    turn = 0.75f;
  }

  // save
  if (e->keysym.sym == SDLK_s)
    moving = -1.0f;

  if (e->keysym.sym == SDLK_a)
    strafing = -1.0f;

  if (e->keysym.sym == SDLK_d)
    strafing = 1.0f;

  if (e->keysym.sym == SDLK_e)
    updown = -1.0f;

  if (e->keysym.sym == SDLK_q)
    updown = 1.0f;

  // position correction with num pad
  if (e->keysym.sym == SDLK_KP8)
    keyx = -1;

  if (e->keysym.sym == SDLK_KP2)
    keyx = 1;

  if (e->keysym.sym == SDLK_KP6)
    keyz = -1;

  if (e->keysym.sym == SDLK_KP4)
    keyz = 1;

  if (e->keysym.sym == SDLK_KP1)
    keyy = -1;

  if (e->keysym.sym == SDLK_KP3)
    keyy = 1;

  if (e->keysym.sym == SDLK_KP7)
    keyr = 1;

  if (e->keysym.sym == SDLK_KP9)
    keyr = -1;

  // fog distance or brush radius
  if (e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
  {
    //change selected model size
    if (gWorld->HasSelection() && gWorld->GetCurrentSelection()->which() != eEntry_MapChunk)
    keys = 1;
  }

  if (e->keysym.sym == SDLK_KP_MINUS || e->keysym.sym == SDLK_MINUS)
  {
    //change selected model size
    if (gWorld->HasSelection() && gWorld->GetCurrentSelection()->which() != eEntry_MapChunk)
    keys = -1;
  }
}

void MapView::keyReleaseEvent (SDL_KeyboardEvent* e)
{
  if (e->keysym.mod & KMOD_CAPS)
    mainGui->capsWarning->show();
  else
    mainGui->capsWarning->hide();

  if (e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT)
    _mod_shift_down = false;

  if (e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT)
    _mod_alt_down = false;

  if (e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL)
    _mod_ctrl_down = false;

  if (e->keysym.sym == SDLK_SPACE)
    _mod_space_down = false;

  // movement
  if (e->keysym.sym == SDLK_w)
  {
    key_w = false;
    if (!(leftMouse && rightMouse) && moving > 0.0f)
    {
      moving = 0.0f;
    }
  }

  if (e->keysym.sym == SDLK_s && moving < 0.0f)
  {
    moving = 0.0f;
  }

  if (e->keysym.sym == SDLK_UP || e->keysym.sym == SDLK_DOWN)
    lookat = 0.0f;

  if (e->keysym.sym == SDLK_LEFT || e->keysym.sym == SDLK_RIGHT)
    turn = 0.0f;

  if (e->keysym.sym == SDLK_d && strafing > 0.0f)
  {
    strafing = 0.0f;
  }

  if (e->keysym.sym == SDLK_a && strafing < 0.0f)
  {
    strafing = 0.0f;
  }

  if (e->keysym.sym == SDLK_q && updown > 0.0f)
    updown = 0.0f;

  if (e->keysym.sym == SDLK_e && updown < 0.0f)
    updown = 0.0f;

  if (e->keysym.sym == SDLK_KP8)
    keyx = 0;

  if (e->keysym.sym == SDLK_KP2)
    keyx = 0;

  if (e->keysym.sym == SDLK_KP6)
    keyz = 0;

  if (e->keysym.sym == SDLK_KP4)
    keyz = 0;

  if (e->keysym.sym == SDLK_KP1)
    keyy = 0;

  if (e->keysym.sym == SDLK_KP3)
    keyy = 0;

  if (e->keysym.sym == SDLK_KP7)
    keyr = 0;

  if (e->keysym.sym == SDLK_KP9)
    keyr = 0;

  if (e->keysym.sym == SDLK_KP_MINUS || e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
    keys = 0;
}

void MapView::inserObjectFromExtern(int model)
{
  InsertObject (model);
}


void MapView::mousemove(SDL_MouseMotionEvent *e)
{
  if (look && !(_mod_shift_down || _mod_ctrl_down || _mod_alt_down || _mod_space_down))
  {
    _camera_ah += e->xrel / XSENS;
    _camera_av += mousedir * e->yrel / YSENS;
    if (_camera_av < -80.0f)
      _camera_av = -80.0f;
    else if (_camera_av > 80.0f)
      _camera_av = 80.0f;

    mainGui->minimapWindow->changePlayerLookAt(math::degrees (_camera_ah));
  }

  if (MoveObj)
  {
    mh = -video.ratio()*e->xrel / static_cast<float>(video.xres());
    mv = -e->yrel / static_cast<float>(video.yres());
  }
  else
  {
    mh = 0.0f;
    mv = 0.0f;
  }

  if (_mod_shift_down || _mod_ctrl_down || _mod_alt_down || _mod_space_down)
  {
    rh = e->xrel / XSENS * 5.0f;
    rv = e->yrel / YSENS * 5.0f;
  }

  if (rightMouse && _mod_alt_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      mainGui->terrainTool->changeInnerRadius(e->xrel / 100.0f);
    }
    if (terrainMode == editing_mode::paint)
    {
      mainGui->texturingTool->change_hardness(e->xrel / 300.0f);
    }
  }

  if (rightMouse && _mod_shift_down)
  {
    if (terrainMode == editing_mode::ground)
    {
      mainGui->terrainTool->moveVertices(-e->yrel / YSENS);
    }
  }


  if (rightMouse && _mod_space_down)
  {
    mainGui->terrainTool->setOrientRelativeTo(_cursor_pos);
  }

  if (leftMouse && _mod_alt_down)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      mainGui->terrainTool->changeRadius(e->xrel / XSENS);
      break;
    case editing_mode::flatten_blur:
      mainGui->flattenTool->changeRadius(e->xrel / XSENS);
      break;
    case editing_mode::paint:
      mainGui->texturingTool->change_radius(e->xrel / XSENS);
      break;
    case editing_mode::water:
      mainGui->guiWater->changeRadius(e->xrel / XSENS);
      break;
    case editing_mode::mccv:
      mainGui->shaderTool->changeRadius(e->xrel / XSENS);
      break;
    }
  }

  if (leftMouse && _mod_space_down)
  {
    switch (terrainMode)
    {
    case editing_mode::ground:
      mainGui->terrainTool->changeSpeed(e->xrel / 30.0f);
      break;
    case editing_mode::flatten_blur:
      mainGui->flattenTool->changeSpeed(e->xrel / 30.0f);
      break;
    case editing_mode::paint:
      mainGui->texturingTool->change_pressure(e->xrel / 300.0f);
      break;
    case editing_mode::mccv:
      mainGui->shaderTool->changeSpeed(e->xrel / XSENS);
      break;
    }
  }

  if (leftMouse && LastClicked)
  {
    LastClicked->processLeftDrag((float)(e->x - 4), (float)(e->y - 4), (float)(e->xrel), (float)(e->yrel));
  }

  if (mViewMode == eViewMode_2D && leftMouse && _mod_alt_down && _mod_shift_down)
  {
    strafing = ((e->xrel / XSENS) / -1) * 5.0f;
    moving = (e->yrel / YSENS) * 5.0f;
  }

  if (mViewMode == eViewMode_2D && rightMouse && _mod_shift_down)
  {
    updown = (e->yrel / YSENS);
  }

  mainGui->mouse_moved (e->x, e->y);

  Environment::getInstance()->screenX = MouseX = e->x;
  Environment::getInstance()->screenY = MouseY = e->y;
  checkWaterSave();
}

void MapView::selectModel(selection_type entry)
{
  mainGui->objectEditor->copy(entry);
}

void MapView::mousePressEvent (SDL_MouseButtonEvent *e)
{
  switch (e->button)
  {
  case SDL_BUTTON_LEFT:
    leftMouse = true;
    break;

  case SDL_BUTTON_RIGHT:
    rightMouse = true;
    break;

  case SDL_BUTTON_MIDDLE:
    if (gWorld->HasSelection())
    {
      MoveObj = true;
      auto selection = gWorld->GetCurrentSelection();
      math::vector_3d objPos;
      if (selection->which() == eEntry_WMO)
      {
        objPos = boost::get<selected_wmo_type> (*selection)->pos;
      }
      else if (selection->which() == eEntry_Model)
      {
        objPos = boost::get<selected_model_type> (*selection)->pos;
      }

      objMoveOffset = _cursor_pos - objPos;
    }

    break;

  case SDL_BUTTON_WHEELUP:
    if (terrainMode == editing_mode::ground)
    {
      if (_mod_alt_down)
      {
        mainGui->terrainTool->changeAngle(_cursor_pos, _mod_ctrl_down ? 0.2f : 2.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->terrainTool->changeOrientation(_cursor_pos, _mod_ctrl_down ? 1.0f : 10.0f);
      }
    }
    else if (terrainMode == editing_mode::flatten_blur)
    {
      if (_mod_alt_down)
      {
        mainGui->flattenTool->changeOrientation(_mod_ctrl_down ? 1.0f : 10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->flattenTool->changeAngle(_mod_ctrl_down ? 0.2f : 2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->flattenTool->changeHeight(1.0f);
      }
    }
    else if (terrainMode == editing_mode::paint)
    {
      if (_mod_space_down)
      {
        mainGui->texturingTool->change_brush_level(10.0f);
      }
      else if (_mod_alt_down)
      {
        mainGui->texturingTool->change_spray_size(1.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->texturingTool->change_spray_pressure(0.25);
      }
    }
    else if (terrainMode == editing_mode::water)
    {
      if (_mod_alt_down)
      {
        mainGui->guiWater->changeOrientation(_mod_ctrl_down ? 1.0f : 10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->guiWater->changeAngle(_mod_ctrl_down ? 0.2f : 2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->guiWater->change_height(_mod_ctrl_down ? 0.1f : 1.0f);
      }
    }
    break;
  case SDL_BUTTON_WHEELDOWN:
    if (terrainMode == editing_mode::ground)
    {
      if (_mod_alt_down)
      {
        mainGui->terrainTool->changeAngle(_cursor_pos, _mod_ctrl_down ? -0.2f : -2.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->terrainTool->changeOrientation(_cursor_pos, _mod_ctrl_down ? -1.0f : -10.0f);
      }
    }
    else if (terrainMode == editing_mode::flatten_blur)
    {
      if (_mod_alt_down)
      {
        mainGui->flattenTool->changeOrientation(_mod_ctrl_down ? -1.0f : -10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->flattenTool->changeAngle(_mod_ctrl_down ? -0.2f : -2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->flattenTool->changeHeight(-1.0f);
      }
    }
    else if (terrainMode == editing_mode::paint)
    {
      if (_mod_space_down)
      {
        mainGui->texturingTool->change_brush_level(-10.0f);
      }
      else if (_mod_alt_down)
      {
        mainGui->texturingTool->change_spray_size(-1.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->texturingTool->change_spray_pressure(-0.25);
      }
    }
    else if (terrainMode == editing_mode::water)
    {
      if (_mod_alt_down)
      {
        mainGui->guiWater->changeOrientation(_mod_ctrl_down ? -1.0f : -10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->guiWater->changeAngle(_mod_ctrl_down ? -0.2f : -2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->guiWater->change_height(_mod_ctrl_down ? -0.1f : -1.0f);
      }
    }
    break;
  }

  if (leftMouse && rightMouse)
  {
    // Both buttons
    moving = 1.0f;
  }
  else if (leftMouse)
  {
    LastClicked = mainGui->processLeftClick(static_cast<float>(MouseX), static_cast<float>(MouseY));
    if (mViewMode == eViewMode_3D && !LastClicked)
    {
      doSelection(false);
    }
  }
  else if (rightMouse)
  {
    look = true;
  }
}

void MapView::mouseReleaseEvent (SDL_MouseButtonEvent* e)
{
  switch (e->button)
  {
  case SDL_BUTTON_LEFT:
    leftMouse = false;

    if (LastClicked)
      LastClicked->processUnclick();

    if (!key_w && moving > 0.0f)
      moving = 0.0f;

    if (mViewMode == eViewMode_2D)
    {
      strafing = 0;
      moving = 0;
    }
    break;

  case SDL_BUTTON_RIGHT:
    rightMouse = false;

    look = false;

    if (!key_w && moving > 0.0f)
      moving = 0.0f;

    if (mViewMode == eViewMode_2D)
      updown = 0;

    break;

  case SDL_BUTTON_MIDDLE:
    MoveObj = false;
    break;
  }
}

void MapView::checkWaterSave()
{
  tile_index const current (gWorld->camera);

  if (!gWorld->mapIndex->hasTile (current) || gWorld->canWaterSave(current))
  {
    mainGui->waterSaveWarning->hide();
  }
  else
  {
    mainGui->waterSaveWarning->show();
  }
}
