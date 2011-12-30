#include <noggit/MapView.h>

#include <QMenu>
#include <QMenuBar>
#include <QKeyEvent>
#include <QSettings>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QSlider>
#include <QLabel>

#include <helper/qt/signal_blocker.h>

#include <noggit/application.h>
#include <noggit/Brush.h>
#include <noggit/Environment.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>
#include <noggit/UICheckBox.h>
#include <noggit/UIDoodadSpawner.h>
#include <noggit/UIFrame.h>
#include <noggit/UIGradient.h>
#include <noggit/UIMapViewGUI.h>
#include <noggit/UISlider.h>
#include <noggit/UITexturePicker.h>
#include <noggit/UITextureSwitcher.h>
#include <noggit/UITexturingGUI.h>
#include <noggit/UIToolbar.h>
#include <noggit/UIZoneIDBrowser.h>
#include <noggit/WMOInstance.h>
#include <noggit/World.h>
#include <noggit/ui/about_widget.h>
#include <noggit/ui/minimap_widget.h>
#include <noggit/ui/help_widget.h>
#include <noggit/ui/cursor_selector.h>

#include <noggit/Log.h>

#include <opengl/texture.h>

#ifdef __FILESAREMISSING
#include <IL/il.h>
#endif

static const qreal shaping_radius_minimum (1.0);
static const qreal shaping_radius_maximum (534.0);
static const qreal shaping_radius_scale (100.0);
static const qreal shaping_speed_minimum (0.0);
static const qreal shaping_speed_maximum (10.0);
static const qreal shaping_speed_scale (10.0);

static const qreal smoothing_radius_minimum (1.0);
static const qreal smoothing_radius_maximum (534.0);
static const qreal smoothing_radius_scale (100.0);
static const qreal smoothing_speed_minimum (0.0);
static const qreal smoothing_speed_maximum (10.0);
static const qreal smoothing_speed_scale (10.0);

float mh,mv,rh,rv;

float keyx,keyy,keyz,keyr,keys;

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

UISlider* paint_brush;

float brushPressure=0.9f;
float brushLevel=255.0f;

brush textureBrush;

UIMapViewGUI* mainGui;

UIToggleGroup * gFlagsToggleGroup;

UIWindow *settings_paint;


void setTextureBrushHardness(float f)
{
  textureBrush.setHardness(f);
}

void setTextureBrushRadius(float f)
{
  textureBrush.setRadius(f);
}

void setTextureBrushPressure(float f)
{
  brushPressure=f;
}

void setTextureBrushLevel(float f)
{
  brushLevel=(1.0f-f)*255.0f;
}

void openSwapper( UIFrame*, int )
{
  mainGui->TextureSwitcher->setPosition(settings_paint->x() , settings_paint->y()) ;
  mainGui->TextureSwitcher->show();
  settings_paint->hide();
}

//! \todo Make this a member of MapView. Also correctly add the actions below again.
/*!
  \brief Import a new model form a text file or a hard coded one.
  Imports a model from the import.txt, the wowModelViewer log or just insert some hard coded testing models.
  \param id the id switch the import kind
*/
/*void InsertObject( UIFrame* button, int id )
{
  //! \todo Beautify.

  // Test if there is an selection
  if( !_world->HasSelection() )
    return;
  // the list of the models to import
  std::vector<std::string> m2s_to_add;
  std::vector<std::string> wmos_to_add;

  // the import file
  std::string importFile;

  const char* filesToAdd[15] = {"","","World\\Scale\\humanmalescale.m2","World\\Scale\\50x50.m2","World\\Scale\\100x100.m2","World\\Scale\\250x250.m2","World\\Scale\\500x500.m2","World\\Scale\\1000x1000.m2","World\\Scale\\50yardradiusdisc.m2","World\\Scale\\200yardradiusdisc.m2","World\\Scale\\777yardradiusdisc.m2","World\\Scale\\50yardradiussphere.m2","World\\Scale\\200yardradiussphere.m2","World\\Scale\\777yardradiussphere.m2",""};

  // MODELINSERT FROM TEXTFILE
  // is a source file set in config file?

  switch(id)
    {
       case 0:
       case 14:
       case 15:
       //! \todo do this somehow else or not at all.
        if (false)
        {
          ConfigFile config( "noggIt.conf" );
          config.readInto( importFile, "ImportFile" );
        }
       break;

       case 1:
        importFile="Import.txt"; //  use import.txt in noggit folder!
       break;

       default:
        m2s_to_add.push_back( filesToAdd[id] );
        break;
  }

  std::string lastModel;
  std::string lastWMO;

  if(importFile!="")
  {
    size_t foundString;
    std::string line;
    std::string findThis;
    std::ifstream fileReader(importFile.c_str());
    if (fileReader.is_open())
    {
      while (! fileReader.eof() )
      {
        getline (fileReader,line);
        if(line.find(".m2")!= std::string::npos || line.find(".M2")!= std::string::npos || line.find(".MDX")!= std::string::npos || line.find(".mdx")!= std::string::npos )
        {
          // M2 inside line
          // is it the modelviewer log then cut the log messages out
          findThis =   "Loading model: ";
          foundString = line.find(findThis);
          if(foundString!= std::string::npos)
          {
            // cut path
            line = line.substr( foundString+findThis.size() );
          }

          // swap mdx to m2
          size_t found = line.rfind( ".mdx" );
          if( found != std::string::npos )
            line.replace( found, 4, ".m2" );
          found = line.rfind( ".MDX" );
          if( found != std::string::npos )
            line.replace( found, 4, ".m2" );

          m2s_to_add.push_back( line );
          lastModel = line;
        }
        else if(line.find(".wmo")!= std::string::npos || line.find(".WMO")!= std::string::npos )
        {
          // WMO inside line
          findThis = "Loading WMO ";
          foundString = line.find(findThis);
          // is it the modelviewer log then cut the log messages out
          if(foundString != std::string::npos)
          {
            // cut path
            line = line.substr( foundString+findThis.size() );
          }
          wmos_to_add.push_back(line);
          lastWMO = line;
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


  Vec3D selectionPosition;
  switch( _world->GetCurrentSelection()->type )
  {
    case eEntry_Model:
      selectionPosition = _world->GetCurrentSelection()->data.model->pos;
      break;
    case eEntry_WMO:
      selectionPosition = _world->GetCurrentSelection()->data.wmo->pos;
      break;
    case eEntry_MapChunk:
      selectionPosition = _world->GetCurrentSelection()->data.mapchunk->GetSelectionPosition();
      break;
  }


  if(id==14)
  {
    LogError << "M2 Problem 14:" << lastModel << " - " << id << std::endl;
    if(lastModel!="")
      if( !MPQFile::exists(lastModel) )
        LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
      else
        _world->addM2( ModelManager::add( lastModel ), selectionPosition );
  }
  else if(id==15)
  {
      LogError << "M2 Problem 15:" << lastModel << " - " << id << std::endl;
    if(lastWMO!="")
      if( !MPQFile::exists(lastWMO) )
        LogError << "Failed adding " << lastWMO << ". It was not in any MPQ." << std::endl;
      else
        _world->addWMO( WMOManager::add( lastWMO ), selectionPosition );
  }
  else
  {

    for( std::vector<std::string>::iterator it = wmos_to_add.begin(); it != wmos_to_add.end(); ++it )
    {

      if( !MPQFile::exists(*it) )
      {
        LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
        continue;
      }

      _world->addWMO( WMOManager::add( *it ), selectionPosition );
    }

    for( std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it )
    {

      if( !MPQFile::exists(*it) )
      {

        LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
        continue;
      }

      _world->addM2( ModelManager::add( *it ), selectionPosition );
    }
  }
  //! \todo Memoryleak: These models will never get deleted.
}
*/

void view_texture_palette( UIFrame* /*button*/, int /*id*/ )
{
  mainGui->TexturePalette->toggleVisibility();
}

void MapView::TEST_save_wdt()
{
  _world->saveWDT();
}

void MapView::move_heightmap()
{
  // set areaid on all chunks of the current ADT
  if(Environment::getInstance()->selectedAreaID)
    _world->moveHeight(Environment::getInstance()->selectedAreaID ,misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::clear_heightmap()
{
  // set areaid on all chunks of the current ADT
  if(Environment::getInstance()->selectedAreaID)
    _world->clearHeight(Environment::getInstance()->selectedAreaID ,misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::set_area_id()
{
  // set areaid on all chunks of the current ADT
  if(Environment::getInstance()->selectedAreaID)
    _world->setAreaID(Environment::getInstance()->selectedAreaID ,misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::clear_all_models()
{
  // call the clearAllModelsOnADT method to clear them all on current ADT
    _world->clearAllModelsOnADT(misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void changeZoneIDValue(UIFrame* /*f*/,int set)
{
  Environment::getInstance()->selectedAreaID = set;
  if( Environment::getInstance()->areaIDColors.find(set) == Environment::getInstance()->areaIDColors.end() )
  {
    Vec3D newColor = Vec3D( misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) , misc::randfloat(0.0f,1.0f) );
    Environment::getInstance()->areaIDColors.insert( std::pair<int,Vec3D>(set, newColor) );
  }
}

void MapView::clear_texture()
{
  // set areaid on all chunks of the current ADT
  _world->setBaseTexture(misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE),misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE));
}

void MapView::show_texture_switcher()
{
  mainGui->TextureSwitcher->getTextures(_world->GetCurrentSelection());
  //mainGui->TextureSwitcher->show();
}

#ifdef __FILESAREMISSING

//! \todo make this a member of MapView.
std::string getCurrentHeightmapPath()
{
  // get MapName
  std::string mapName;
  int id = _world->getMapID();
  for( DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i )
  {
    if( i->getInt( MapDB::MapID ) == id)
      mapName = i->getString( MapDB::InternalName );
  }

  // build the path and filename string.
  std::stringstream png_filename;
  png_filename << Project::getInstance()->getPath() << "world\\maps\\" << mapName << "\\H_" << mapName
    << "_" << misc::FtoIround((_world->camera.x-(TILESIZE/2))/TILESIZE) << "_" << misc::FtoIround((_world->camera.z-(TILESIZE/2))/TILESIZE) << ".png" ;
  return png_filename.str();

}

void MapView::export_heightmap()
{

  // create the image and write to disc.
  GLfloat* data = new GLfloat[272*272];

  ilInit();

  int width  = 272 ;
  int height = 272 ;
  int bytesToUsePerPixel = 32;  // 16 bit per channel
  int sizeOfByte = sizeof( unsigned char ) ;
  int theSize = width * height * sizeOfByte * bytesToUsePerPixel ;

  unsigned char * imData =(unsigned char*)malloc( theSize ) ;

  int colors = 0;
  // write the height data to the image array
  for( int i = 0 ; i < theSize ; i++ )
  {
    imData[ i ] = colors ;
    if(i==100)colors = 200;
    if(i==200)colors = 4000;
  }


  ILuint ImageName; // The image name.
  ilGenImages(1, &ImageName); // Grab a new image name.
  ilBindImage(ImageName); // bind it
  ilTexImage(width,height,1,bytesToUsePerPixel,GL_LUMINANCE,IL_UNSIGNED_BYTE,NULL);
  ilSetData(imData);
  ilEnable(IL_FILE_OVERWRITE);
  //ilSave(IL_PNG, getCurrentHeightmapPath().c_str());
  ilSave(IL_PNG, "test2.png");
  free(imData);
}

void MapView::import_heightmap()
{
  ilInit();

  //ILboolean loadImage = ilLoadImage( getCurrentHeightmapPath().c_str() ) ;
  const char *image = "test.png";
  ILboolean loadImage = ilLoadImage( image ) ;

  std::stringstream MessageText;
  if(loadImage)
  {

    LogDebug << "Image loaded: " << image << "\n";
    LogDebug <<  "ImageSize: " << ilGetInteger( IL_IMAGE_SIZE_OF_DATA ) << "\n";
    LogDebug <<  "BPP: " << ilGetInteger( IL_IMAGE_BITS_PER_PIXEL ) << "\n";
    LogDebug <<  "Format: " << ilGetInteger( IL_IMAGE_FORMAT ) << "\n";
    LogDebug <<  "SizeofData: " << ilGetInteger( IL_IMAGE_SIZE_OF_DATA ) << "\n";

  }
  else
  {
    LogDebug << "Cant load Image: " << image << "\n";
    ILenum err = ilGetError() ;

    MessageText << err << "\n";
    //MessageText << ilGetString(ilGetError()) << "\n";
    LogDebug << MessageText.str();
  }
}
#else
void MapView::import_heightmap() { }
void MapView::export_heightmap() { }
#endif

void MapView::shaping_formula (int id)
{
  shaping_formula (shaping_formula_type::formula (id));
}

void MapView::shaping_formula (shaping_formula_type::formula id)
{
  _shaping_formula = id;

  helper::qt::signal_blocker block (_shaping_formula_radio_group);
  _shaping_formula_radio_group->button (id)->click();
}

const MapView::shaping_formula_type::formula& MapView::shaping_formula() const
{
  return _shaping_formula;
}

void MapView::shaping_radius (int value)
{
  shaping_radius (value / shaping_radius_scale);
}

void MapView::shaping_speed (int value)
{
  shaping_speed (value / shaping_speed_scale);
}

void MapView::shaping_radius (qreal value)
{
  _shaping_radius = qBound ( shaping_radius_minimum
                           , value
                           , shaping_radius_maximum
                           );

  helper::qt::signal_blocker block (_shaping_radius_slider);
  _shaping_radius_slider->setValue (shaping_radius() * shaping_radius_scale);
}

void MapView::shaping_speed (qreal value)
{
  _shaping_speed = qBound ( shaping_speed_minimum
                          , value
                          , shaping_speed_maximum
                          );

  helper::qt::signal_blocker block (_shaping_speed_slider);
  _shaping_speed_slider->setValue (shaping_speed() * shaping_speed_scale);
}

const qreal& MapView::shaping_radius() const
{
  return _shaping_radius;
}

const qreal& MapView::shaping_speed() const
{
  return _shaping_speed;
}

void MapView::smoothing_formula (int id)
{
  smoothing_formula (smoothing_formula_type::formula (id));
}

void MapView::smoothing_formula (smoothing_formula_type::formula id)
{
  _smoothing_formula = id;

  helper::qt::signal_blocker block (_smoothing_formula_radio_group);
  _smoothing_formula_radio_group->button (id)->click();
}

const MapView::smoothing_formula_type::formula& MapView::smoothing_formula() const
{
  return _smoothing_formula;
}

void MapView::smoothing_radius (int value)
{
  smoothing_radius (value / smoothing_radius_scale);
}

void MapView::smoothing_speed (int value)
{
  smoothing_speed (value / smoothing_speed_scale);
}

void MapView::smoothing_radius (qreal value)
{
  _smoothing_radius = qBound ( smoothing_radius_minimum
                            , value
                            , smoothing_radius_maximum
                            );

  helper::qt::signal_blocker block (_smoothing_radius_slider);
  _smoothing_radius_slider->setValue (smoothing_radius() * smoothing_radius_scale);
}

void MapView::smoothing_speed (qreal value)
{
  _smoothing_speed = qBound ( smoothing_speed_minimum
                            , value
                            , smoothing_speed_maximum
                            );

  helper::qt::signal_blocker block (_smoothing_speed_slider);
  _smoothing_speed_slider->setValue (smoothing_speed() * smoothing_speed_scale);
}

const qreal& MapView::smoothing_radius() const
{
  return _smoothing_radius;
}

const qreal& MapView::smoothing_speed() const
{
  return _smoothing_speed;
}

void MapView::create_shaping_settings_widget()
{
  delete _shaping_settings_widget;

  _shaping_settings_widget = new QWidget (NULL);
  _shaping_settings_widget->setWindowTitle (tr ("Shaping settings"));

  QVBoxLayout* widget_layout (new QVBoxLayout (_shaping_settings_widget));

  QWidget* button_holder (new QWidget (_shaping_settings_widget));
  widget_layout->addWidget (button_holder);

  QGridLayout* button_layout (new QGridLayout (button_holder));

  _shaping_formula_radio_group = new QButtonGroup (this);

  QRadioButton* flat_checkbox (new QRadioButton (tr ("Flat"), NULL));
  QRadioButton* linear_checkbox (new QRadioButton (tr ("Linear"), NULL));
  QRadioButton* smooth_checkbox (new QRadioButton (tr ("Smooth"), NULL));
  QRadioButton* polynomial_checkbox (new QRadioButton (tr ("Polynomial"), NULL));
  QRadioButton* trigonometric_checkbox (new QRadioButton (tr ("Trigonometric"), NULL));
  QRadioButton* square_checkbox (new QRadioButton (tr ("Square"), NULL));

  _shaping_formula_radio_group->addButton (flat_checkbox, shaping_formula_type::flat);
  _shaping_formula_radio_group->addButton (linear_checkbox, shaping_formula_type::linear);
  _shaping_formula_radio_group->addButton (smooth_checkbox, shaping_formula_type::smooth);
  _shaping_formula_radio_group->addButton (polynomial_checkbox, shaping_formula_type::polynomial);
  _shaping_formula_radio_group->addButton (trigonometric_checkbox, shaping_formula_type::trigonometric);
  _shaping_formula_radio_group->addButton (square_checkbox, shaping_formula_type::square);

  connect (_shaping_formula_radio_group, SIGNAL (buttonClicked (int)), SLOT (shaping_formula (int)));

  button_layout->addWidget (flat_checkbox, 0, 0);
  button_layout->addWidget (linear_checkbox, 1, 0);
  button_layout->addWidget (smooth_checkbox, 2, 0);
  button_layout->addWidget (polynomial_checkbox, 0, 1);
  button_layout->addWidget (trigonometric_checkbox, 1, 1);
  button_layout->addWidget (square_checkbox, 2, 1);

  _shaping_radius_slider = new QSlider (Qt::Horizontal, _shaping_settings_widget);
  _shaping_radius_slider->setMinimum (shaping_radius_minimum * shaping_radius_scale);
  _shaping_radius_slider->setMaximum (shaping_radius_maximum * shaping_radius_scale);
  connect (_shaping_radius_slider, SIGNAL (valueChanged (int)), SLOT (shaping_radius (int)));

  _shaping_speed_slider = new QSlider (Qt::Horizontal, _shaping_settings_widget);
  _shaping_speed_slider->setMinimum (shaping_speed_minimum * shaping_speed_scale);
  _shaping_speed_slider->setMaximum (shaping_speed_maximum * shaping_speed_scale);
  connect (_shaping_speed_slider, SIGNAL (valueChanged (int)), SLOT (shaping_speed (int)));

  QLabel* radius_label (new QLabel (tr ("Brush &radius"), _shaping_settings_widget));
  QLabel* speed_label (new QLabel (tr ("Shaping &speed"), _shaping_settings_widget));

  radius_label->setBuddy (_shaping_radius_slider);
  speed_label->setBuddy (_shaping_speed_slider);

  widget_layout->addWidget (radius_label);
  widget_layout->addWidget (_shaping_radius_slider);
  widget_layout->addWidget (speed_label);
  widget_layout->addWidget (_shaping_speed_slider);

  //! \note Looks funny, but sets the UI to the default position.
  shaping_radius (shaping_radius());
  shaping_speed (shaping_speed());
  shaping_formula (shaping_formula());
}

void MapView::create_smoothing_settings_widget()
{
  delete _smoothing_settings_widget;

  _smoothing_settings_widget = new QWidget (NULL);
  _smoothing_settings_widget->setWindowTitle (tr ("Smoothing settings"));

  QVBoxLayout* widget_layout (new QVBoxLayout (_smoothing_settings_widget));

  QWidget* button_holder (new QWidget (_smoothing_settings_widget));
  widget_layout->addWidget (button_holder);

  _smoothing_formula_radio_group = new QButtonGroup (this);

  QRadioButton* flat_checkbox (new QRadioButton (tr ("Flat"), NULL));
  QRadioButton* linear_checkbox (new QRadioButton (tr ("Linear"), NULL));
  QRadioButton* smooth_checkbox (new QRadioButton (tr ("Smooth"), NULL));

  _smoothing_formula_radio_group->addButton (flat_checkbox, smoothing_formula_type::flat);
  _smoothing_formula_radio_group->addButton (linear_checkbox, smoothing_formula_type::linear);
  _smoothing_formula_radio_group->addButton (smooth_checkbox, smoothing_formula_type::smooth);

  connect (_smoothing_formula_radio_group, SIGNAL (buttonClicked (int)), SLOT (smoothing_formula (int)));

  widget_layout->addWidget (flat_checkbox, 0, 0);
  widget_layout->addWidget (linear_checkbox, 1, 0);
  widget_layout->addWidget (smooth_checkbox, 2, 0);

  _smoothing_radius_slider = new QSlider (Qt::Horizontal, _smoothing_settings_widget);
  _smoothing_radius_slider->setMinimum (smoothing_radius_minimum * smoothing_radius_scale);
  _smoothing_radius_slider->setMaximum (smoothing_radius_maximum * smoothing_radius_scale);
  connect (_smoothing_radius_slider, SIGNAL (valueChanged (int)), SLOT (smoothing_radius (int)));

  _smoothing_speed_slider = new QSlider (Qt::Horizontal, _smoothing_settings_widget);
  _smoothing_speed_slider->setMinimum (smoothing_speed_minimum * smoothing_speed_scale);
  _smoothing_speed_slider->setMaximum (smoothing_speed_maximum * smoothing_speed_scale);
  connect (_smoothing_speed_slider, SIGNAL (valueChanged (int)), SLOT (smoothing_speed (int)));

  QLabel* radius_label (new QLabel (tr ("Brush &radius"), _smoothing_settings_widget));
  QLabel* speed_label (new QLabel (tr ("Shaping &speed"), _smoothing_settings_widget));

  radius_label->setBuddy (_smoothing_radius_slider);
  speed_label->setBuddy (_smoothing_speed_slider);

  widget_layout->addWidget (radius_label);
  widget_layout->addWidget (_smoothing_radius_slider);
  widget_layout->addWidget (speed_label);
  widget_layout->addWidget (_smoothing_speed_slider);

  //! \note Looks funny, but sets the UI to the default position.
  smoothing_radius (smoothing_radius());
  smoothing_speed (smoothing_speed());
  smoothing_formula (smoothing_formula());
}

void MapView::createGUI()
{
  create_shaping_settings_widget();
  create_smoothing_settings_widget();

  //3D Paint settings UIWindow
  settings_paint=new UIWindow(0.0f,0.0f,180.0f,140.0f);
  settings_paint->hide();
  settings_paint->movable( true );

  settings_paint->addChild( new UIText( 78.5f, 2.0f, "3D Paint", arial14, eJustifyCenter ) );

  UIGradient *G1;
  G1=new UIGradient;
  G1->width( 20.0f );
  G1->x( settings_paint->width() - 4.0f - G1->width() );
  G1->y( 4.0f );
  G1->height( 92.0f );
  G1->setMaxColor(1.0f,1.0f,1.0f,1.0f);
  G1->setMinColor(0.0f,0.0f,0.0f,1.0f);
  G1->horiz=false;
  G1->setClickColor(1.0f,0.0f,0.0f,1.0f);
  G1->setClickFunc(setTextureBrushLevel);
  G1->setValue(0.0f);

  settings_paint->addChild(G1);

  UISlider* S1;
  S1=new UISlider(6.0f,33.0f,145.0f,1.0f,0.0f);
  S1->setFunc(setTextureBrushHardness);
  S1->setValue(textureBrush.getHardness());
  S1->setText("Hardness: ");
  settings_paint->addChild(S1);

  paint_brush=new UISlider(6.0f,59.0f,145.0f,100.0f,0.00001f);
  paint_brush->setFunc(setTextureBrushRadius);
  paint_brush->setValue(textureBrush.getRadius() / 100 );
  paint_brush->setText("Radius: ");
  settings_paint->addChild(paint_brush);

  S1=new UISlider(6.0f,85.0f,145.0f,0.99f,0.01f);
  S1->setFunc(setTextureBrushPressure);
  S1->setValue(brushPressure);
  S1->setText("Pressure: ");
  settings_paint->addChild(S1);


  UIButton* B1;
  B1=new UIButton( 6.0f, 111.0f, 170.0f, 30.0f, "Texture swapper", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", openSwapper, 1 ) ;
  settings_paint->addChild(B1);

  // create main gui object that holds all other gui elements for access ( in the future ;) )
  mainGui = new UIMapViewGUI (_world, this, width(), height());

  mainGui->addChild(settings_paint);

  mainGui->guiToolbar->current_texture->setClickFunc( view_texture_palette, 0 );

  mainGui->ZoneIDBrowser->setMapID( _world->getMapID() );
  mainGui->ZoneIDBrowser->setChangeFunc( changeZoneIDValue );

  _doodad_spawner = new UIDoodadSpawner (_world);
  _doodad_spawner->hide();
  mainGui->addChild (_doodad_spawner);

  mainGui->addChild(mainGui->TexturePalette = UITexturingGUI::createTexturePalette(4,8,mainGui));
  mainGui->TexturePalette->hide();
  mainGui->addChild(mainGui->SelectedTexture = UITexturingGUI::createSelectedTexture());
  mainGui->SelectedTexture->hide();
  mainGui->addChild(UITexturingGUI::createTilesetLoader());
  mainGui->addChild(UITexturingGUI::createTextureFilter());
  mainGui->addChild(_map_chunk_properties_window = UITexturingGUI::createMapChunkWindow());
  _map_chunk_properties_window->hide();

#define NEW_ACTION(__NAME__, __TEXT, __SLOT, __KEYS) QAction* __NAME__ (new_action (__TEXT, __SLOT, __KEYS));
#define NEW_ACTION_OTHER(__NAME__, __TEXT, __RECEIVER, __SLOT, __KEYS) QAction* __NAME__ (new_action (__TEXT, __RECEIVER, __SLOT, __KEYS));
#define NEW_TOGGLE_ACTION(__NAME__, __TEXT, __SLOT, __KEYS, __DEFAULT) QAction* __NAME__ (new_toggleable_action (__TEXT, __SLOT, __DEFAULT, __KEYS));

  NEW_ACTION (save_current_tile, tr ("Save current tile"), SLOT (save()), Qt::CTRL + Qt::SHIFT + Qt::Key_S);
  NEW_ACTION (save_modified_tiles, tr ("Save all modified tiles"), SLOT (save_all()), QKeySequence::Save);
  NEW_ACTION (reload_tile, tr ("Reload current tile"), SLOT (reload_current_tile()), Qt::SHIFT + Qt::Key_J);

  NEW_ACTION (bookmark, tr ("Add bookmark for this location"), SLOT (add_bookmark()), Qt::Key_F5);

  NEW_ACTION (export_heightmap, tr ("Export heightmap"), SLOT (export_heightmap()), 0);
  NEW_ACTION (import_heightmap, tr ("Import heightmap"), SLOT (import_heightmap()), 0);

  NEW_ACTION (to_menu, tr ("Return to menu"), SLOT (exit_to_menu()), Qt::Key_Escape);


  NEW_ACTION (copy_object, tr ("Copy object"), SLOT (copy_selected_object()), QKeySequence::Copy);
  NEW_ACTION (paste_object, tr ("Paste object"), SLOT (paste_object()), QKeySequence::Paste);
  NEW_ACTION (delete_object, tr ("Delete object"), SLOT (delete_selected_object()), QKeySequence::Delete);
  //delete_object->setShortcuts (QList<QKeySequence>() << QKeySequence::Delete << Qt::Key_Backspace);

  NEW_ACTION (reset_rotation, tr ("Reset object's rotation"), SLOT (reset_selected_object_rotation()), Qt::CTRL + Qt::Key_R);
  NEW_ACTION (snap_object_to_ground, tr ("Snap object to ground"), SLOT (snap_selected_object_to_ground()), Qt::Key_PageDown);


  NEW_TOGGLE_ACTION (toolbar, tr ("Toolbar"), SLOT (toggle_toolbar_visibility (bool)), 0, true);
  NEW_TOGGLE_ACTION (current_texture, tr ("Selected texture"), SLOT (toggle_current_texture_visiblity (bool)), 0, false);
  NEW_TOGGLE_ACTION (map_chunk_settings, tr ("Map chunk settings"), SLOT (show_map_chunk_settings(bool)), 0, false);
  NEW_TOGGLE_ACTION (toggle_minimap, tr ("Show minimap"), SLOT (toggle_minimap (bool)), Qt::Key_M, false);
  NEW_TOGGLE_ACTION (detail_infos, tr ("Object information"), SLOT (toggle_detail_info_window (bool)), Qt::Key_F8, false);


  NEW_TOGGLE_ACTION (doodad_drawing, tr ("Draw doodads"), SLOT (toggle_doodad_drawing (bool)), Qt::Key_F1, true);
  NEW_TOGGLE_ACTION (wmo_doodad_drawing, tr ("Draw doodads inside of WMOs"), SLOT (toggle_wmo_doodad_drawing (bool)), Qt::Key_F2, true);
  NEW_TOGGLE_ACTION (terrain_drawing, tr ("Draw terrain"), SLOT (toggle_terrain_drawing (bool)), Qt::Key_F3, true);
  NEW_TOGGLE_ACTION (water_drawing, tr ("Draw water"), SLOT (toggle_water_drawing (bool)), Qt::Key_F4, true);
  NEW_TOGGLE_ACTION (wmo_drawing, tr ("Draw WMOs"), SLOT (toggle_wmo_drawing (bool)), Qt::Key_F6, true);
  NEW_TOGGLE_ACTION (line_drawing, tr ("Draw lines"), SLOT (toggle_line_drawing (bool)), Qt::Key_F7, false);
  NEW_TOGGLE_ACTION (hole_line_drawing, tr ("Draw lines for holes"), SLOT (toggle_hole_line_drawing (bool)), Qt::SHIFT + Qt::Key_F7, false);
  //! \todo on OSX this shows up as "8" in menu and does not react to the keybinding.
  NEW_TOGGLE_ACTION (contour_drawing, tr ("Draw contours"), SLOT (toggle_contour_drawing (bool)), Qt::Key_F9, false);
  NEW_TOGGLE_ACTION (fog_drawing, tr ("Draw fog"), SLOT (toggle_fog_drawing (bool)), Qt::Key_F, true);
  NEW_TOGGLE_ACTION (toggle_lighting, tr ("Enable Lighting"), SLOT (toggle_lighting (bool)), Qt::Key_L, true);

  NEW_ACTION (turn_around, tr ("Turn camera 180 degrees"), SLOT (turn_around()), Qt::Key_R);

  // lukas letzte zeile. Yay. we:r )hätte das gedacht? ,_,
  // VERDAMMTER VOLLIDIOT
  NEW_ACTION_OTHER (cursor_selector, tr ("Choose selection cursor"), _cursor_selector, SLOT (show()), Qt::Key_Alt & Qt::Key_C);
  NEW_TOGGLE_ACTION (invert_mouse_y_axis, tr ("Invert mouse y-axis"), SLOT (invert_mouse_y_axis (bool)), Qt::Key_I, false);
  NEW_TOGGLE_ACTION (auto_selection, tr ("Automatic selection"), SLOT (toggle_auto_selecting (bool)), Qt::SHIFT + Qt::Key_F4, false);

  NEW_TOGGLE_ACTION (rotation_randomization, tr ("Randomized rotation when copying"), SLOT (toggle_copy_rotation_randomization (bool)), 0, false);
  NEW_TOGGLE_ACTION (position_randomization, tr ("Randomized position when copying"), SLOT (toggle_copy_position_randomization (bool)), 0, false);
  NEW_TOGGLE_ACTION (size_randomization, tr ("Randomized size when copying"), SLOT (toggle_copy_size_randomization (bool)), 0, false);

  NEW_ACTION (decrease_time_speed, tr ("Decrease time speed"), SLOT (decrease_time_speed()), Qt::Key_B);
  NEW_ACTION (increase_time_speed, tr ("Increase time speed"), SLOT (increase_time_speed()), Qt::Key_N);
  NEW_ACTION (decrease_moving_speed, tr ("Decrease movement speed"), SLOT (decrease_moving_speed()), Qt::Key_O);
  NEW_ACTION (increase_moving_speed, tr ("Increase movement speed"), SLOT (increase_moving_speed()), Qt::Key_P);


  NEW_ACTION_OTHER (key_bindings, tr ("Key bindings"), _help_widget, SLOT (show()), Qt::Key_H);
  NEW_TOGGLE_ACTION (application_infos, tr ("Show application infos"), SLOT (toggle_app_info (bool)), 0, false);
  NEW_ACTION_OTHER (about_noggit, tr ("About Noggit"), _about_widget, SLOT (show()), 0);

  NEW_ACTION (save_wdt, tr ("Save WDT"), SLOT (TEST_save_wdt()), 0);
  NEW_ACTION (save_minimap, tr ("Save minimap as raw files"), SLOT (save_minimap()), Qt::Key_P + Qt::SHIFT + Qt::CTRL);
  NEW_ACTION (toggle_doodad_spawner, tr ("toggle_doodad_spawner"), SLOT (toggle_doodad_spawner()), Qt::Key_T);

#undef NEW_ACTION
#undef NEW_ACTION_OTHER
#undef NEW_TOGGLE_ACTION

#ifdef Q_WS_MAC
  QMenuBar* menu_bar (new QMenuBar (NULL));
#else
  QMenuBar* menu_bar (new QMenuBar (this));
#endif

  QMenu* file_menu (menu_bar->addMenu (tr ("File")));
  file_menu->addAction (save_current_tile);
  file_menu->addAction (save_modified_tiles);
  file_menu->addAction (reload_tile);
  file_menu->addSeparator();
  file_menu->addAction (bookmark);
  file_menu->addSeparator();
  file_menu->addAction (export_heightmap);
  file_menu->addAction (import_heightmap);
  file_menu->addSeparator();
  file_menu->addAction (to_menu);

  QMenu* edit_menu (menu_bar->addMenu (tr ("Edit")));
  edit_menu->addAction (copy_object);
  edit_menu->addAction (paste_object);
  edit_menu->addAction (delete_object);
  edit_menu->addSeparator();
  edit_menu->addAction (reset_rotation);
  edit_menu->addAction (snap_object_to_ground);

  QMenu* assist_menu (menu_bar->addMenu (tr ("Assist")));
  QMenu* insertion_menu (assist_menu->addMenu (tr ("Insert helper model")));

  /*
  assist_menu->addAction (tr ("all from MV", InsertObject, 0  );
  assist_menu->addAction (tr ("last M2 from MV", InsertObject, 14  );
  assist_menu->addAction (tr ("last WMO from MV", InsertObject, 15  );
  assist_menu->addAction (tr ("from Text File", InsertObject, 1  );
  insertion_menu->addAction (tr ("Human scale", InsertObject, 2  );
  insertion_menu->addAction (tr ("Cube 50", InsertObject, 3  );
  insertion_menu->addAction (tr ("Cube 100", InsertObject, 4  );
  insertion_menu->addAction (tr ("Cube 250", InsertObject, 5  );
  insertion_menu->addAction (tr ("Cube 500", InsertObject, 6  );
  insertion_menu->addAction (tr ("Cube 1000", InsertObject, 7  );
  insertion_menu->addAction (tr ("Disc 50", InsertObject, 8  );
  insertion_menu->addAction (tr ("Disc 200", InsertObject, 9  );
  insertion_menu->addAction (tr ("Disc 777", InsertObject, 10  );
  insertion_menu->addAction (tr ("Sphere 50", InsertObject, 11  );
  insertion_menu->addAction (tr ("Sphere 200", InsertObject, 12  );
  insertion_menu->addAction (tr ("Sphere 777", InsertObject, 13  );
  assist_menu->addSeparator();
  assist_menu->addAction (tr ("Set Area ID", set_area_id, 0  );
  assist_menu->addAction (tr ("Clear height map", clear_heightmap, 0  );
  assist_menu->addAction (tr ("Move to position", move_heightmap, 0  );
  assist_menu->addAction (tr ("Clear texture", clear_texture, 0  );
  assist_menu->addAction (tr ("Clear models", clear_all_models, 0  );
  assist_menu->addAction (tr ("Switch texture", show_texture_switcher, 0  );
*/
  QMenu* view_menu (menu_bar->addMenu (tr ("View")));
  view_menu->addAction (toolbar);
  view_menu->addAction (current_texture);
  view_menu->addAction (toggle_minimap);
  view_menu->addAction (detail_infos);
  //! \todo re-add this when reimplemented.
  //view_menu->addAction (map_chunk_settings);
  view_menu->addSeparator();
  view_menu->addAction (doodad_drawing);
  view_menu->addAction (wmo_doodad_drawing);
  view_menu->addAction (terrain_drawing);
  view_menu->addAction (water_drawing);
  view_menu->addAction (wmo_drawing);
  view_menu->addAction (line_drawing);
  view_menu->addAction (hole_line_drawing);
  view_menu->addAction (contour_drawing);
  view_menu->addAction (fog_drawing);
  view_menu->addAction (toggle_lighting);

  QMenu* settings_menu (menu_bar->addMenu (tr ("Settings")));
  settings_menu->addAction (cursor_selector);
  settings_menu->addSeparator();
  settings_menu->addAction (rotation_randomization);
  settings_menu->addAction (position_randomization);
  settings_menu->addAction (size_randomization);
  settings_menu->addSeparator();
  settings_menu->addAction (auto_selection);
  settings_menu->addAction (invert_mouse_y_axis);
  settings_menu->addSeparator();
  settings_menu->addAction (decrease_time_speed);
  settings_menu->addAction (increase_time_speed);
  settings_menu->addAction (decrease_moving_speed);
  settings_menu->addAction (increase_moving_speed);

  QMenu* help_menu (menu_bar->addMenu (tr ("Help")));
  help_menu->addAction (key_bindings);
  help_menu->addAction (application_infos);
  help_menu->addAction (about_noggit);

  QMenu* debug_menu (menu_bar->addMenu (tr ("Testing and Debugging")));
  debug_menu->addAction (save_wdt);
  debug_menu->addAction (toggle_doodad_spawner);
  debug_menu->addAction (save_minimap);

  QMenu* useless_menu (debug_menu->addMenu (tr ("Stuff that should only be on keys")));
  useless_menu->addAction (turn_around);

  // Minimap window
  _minimap = new ui::minimap_widget (NULL);
  _minimap->world (_world);
  _minimap->draw_skies (true);
  _minimap->draw_camera (true);
  _minimap->draw_boundaries (true);
  _minimap->hide();
}

QAction* MapView::new_action (const QString& text, const char* slot, const QKeySequence& shortcut)
{
  QAction* action (new QAction (text, this));
  connect (action, SIGNAL (triggered()), slot);
  if (shortcut != QKeySequence (0))
  {
    action->setShortcuts (QList<QKeySequence>() << shortcut);
  }
  return action;
}

QAction* MapView::new_action (const QString& text, QObject* receiver, const char* slot, const QKeySequence& shortcut)
{
  QAction* action (new QAction (text, this));
  receiver->connect (action, SIGNAL (triggered()), slot);
  if (shortcut != QKeySequence (0))
  {
    action->setShortcuts (QList<QKeySequence>() << shortcut);
  }
  return action;
}

QAction* MapView::new_toggleable_action (const QString& text, const char* slot, bool default_value, const QKeySequence& shortcut)
{
  QAction* action (new QAction (text, this));
  connect (action, SIGNAL (toggled (bool)), slot);
  action->setCheckable (true);
  action->setChecked (default_value);
  if (shortcut != QKeySequence (0))
  {
    action->setShortcuts (QList<QKeySequence>() << shortcut);
  }
  return action;
}

void MapView::show_map_chunk_settings (bool value)
{
  _map_chunk_properties_window->hidden (!value);
}

void MapView::toggle_toolbar_visibility (bool value)
{
  mainGui->guiToolbar->hidden (!value);
}

void MapView::toggle_current_texture_visiblity (bool value)
{
  mainGui->SelectedTexture->hidden (!value);
}

void MapView::toggle_copy_size_randomization (bool value)
{
  _copy_size_randomization = value;
}
void MapView::toggle_copy_position_randomization (bool value)
{
  _copy_position_randomization = value;
}
void MapView::toggle_copy_rotation_randomization (bool value)
{
  _copy_rotation_randomization = value;
}

MapView::MapView ( World* world
                 , qreal viewing_distance
                 , float ah0
                 , float av0
                 , QGLWidget* shared
                 , QWidget* parent
                 )
  : QGLWidget (parent, shared)
  , _startup_time ()
  , _last_update (0.0)
  , ah( ah0 )
  , av( av0 )
  , _GUIDisplayingEnabled( true )
  , mTimespeed( 0.0f )
  , _world (world)
  , _help_widget (new ui::help_widget (NULL))
  , _about_widget (NULL)
  //  , _about_widget (new ui::about_widget (NULL))
  , _cursor_selector (new ui::cursor_selector (NULL))
  , _is_currently_moving_object (false)
  , _draw_terrain_height_contour (false)
  , _draw_wmo_doodads (true)
  , _draw_fog (true)
  , _draw_lines (false)
  , _draw_doodads (true)
  , _draw_terrain (true)
  , _draw_water (false)
  , _draw_wmos (true)
  , _draw_hole_lines (false)
  , _draw_lighting (true)
  , _holding_left_mouse_button (false)
  , _holding_right_mouse_button (false)
  , _current_terrain_editing_mode (shaping)
  , _terrain_editing_mode_before_2d (_current_terrain_editing_mode)
  , _save_to_minimap_on_next_drawing (false)
  , _last_clicked_ui_frame (NULL)
  , _map_chunk_properties_window (NULL)
  , _shaping_radius (15.0)
  , _shaping_speed (1.0)
  , _shaping_formula (shaping_formula_type::smooth)
  , _shaping_formula_radio_group (NULL)
  , _shaping_radius_slider (NULL)
  , _shaping_speed_slider (NULL)
  , _shaping_settings_widget (NULL)
  , _smoothing_radius (15.0)
  , _smoothing_speed (1.0)
  , _smoothing_formula (smoothing_formula_type::smooth)
  , _smoothing_formula_radio_group (NULL)
  , _smoothing_radius_slider (NULL)
  , _smoothing_speed_slider (NULL)
  , _smoothing_settings_widget (NULL)
  , _automatically_update_terrain_selection (true)
  , _copy_size_randomization (false)
  , _copy_position_randomization (false)
  , _copy_rotation_randomization (false)
  , _viewing_distance (viewing_distance)
  , _tile_mode_zoom (0.25)
{
  moving = strafing = updown = 0.0f;

  mousedir = -1.0f;

  static const float default_moving_speed (66.6f);
  movespd = default_moving_speed;

  textureBrush.init();

  look = false;
  mViewMode = eViewMode_3D;

  createGUI();

  startTimer (40);
  _startup_time.start();

  setFocusPolicy (Qt::StrongFocus);
  setMouseTracking (true);
}

void MapView::timerEvent (QTimerEvent*)
{
  const qreal now (_startup_time.elapsed() / 1000.0);

  tick (now, now - _last_update);
  updateGL();

  _last_update = now;
}

void MapView::initializeGL()
{
  qglClearColor (Qt::black);

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_NORMAL_ARRAY);
  glEnableClientState (GL_TEXTURE_COORD_ARRAY);

  glEnable (GL_DEPTH_TEST);
  glEnable (GL_CULL_FACE);
  glShadeModel (GL_SMOOTH);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_MULTISAMPLE);
  static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
  glLightfv (GL_LIGHT0, GL_POSITION, lightPosition);
}

void MapView::paintGL()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  display();
}

void MapView::resizeGL (int width, int height)
{
  glViewport (0.0f, 0.0f, width, height);
}

MapView::~MapView()
{
  delete mainGui;
  mainGui = NULL;

  delete _world;
  _world = NULL;
}

void MapView::tick( float /*t*/, float dt )
{
  if (textureBrush.needUpdate())
  {
    textureBrush.GenerateTexture();
  }

  dt = std::min( dt, 1.0f );

  Vec3D dir( 1.0f, 0.0f, 0.0f );
  Vec3D dirUp( 1.0f, 0.0f, 0.0f );
  Vec3D dirRight( 0.0f, 0.0f, 1.0f );
  rotate( 0.0f, 0.0f, &dir.x,&dir.y, av * PI / 180.0f );
  rotate( 0.0f, 0.0f, &dir.x,&dir.z, ah * PI / 180.0f );

  if( Environment::getInstance()->ShiftDown )
  {
    dirUp.x = 0.0f;
    dirUp.y = 1.0f;
    dirRight *= 0.0f; //! \todo  WAT?
  }
  else if( Environment::getInstance()->CtrlDown )
  {
    dirUp.x = 0.0f;
    dirUp.y = 1.0f;
    rotate( 0.0f, 0.0f, &dirUp.x, &dirUp.y, av * PI / 180.0f );
    rotate( 0.0f, 0.0f, &dirRight.x, &dirRight.y, av * PI / 180.0f );
    rotate( 0.0f, 0.0f, &dirUp.x, &dirUp.z, ah * PI / 180.0f );
    rotate( 0.0f, 0.0f, &dirRight.x, &dirRight.z, ah * PI / 180.0f );
  }
  else
  {
    rotate( 0.0f, 0.0f, &dirUp.x, &dirUp.z, ah * PI / 180.0f );
    rotate( 0.0f, 0.0f, &dirRight.x, &dirRight.z, ah * PI / 180.0f );
  }

  nameEntry * Selection = _world->GetCurrentSelection();

  if( Selection )
  {
    qreal keypad_object_move_ratio (0.1);
    // Set move scale and rotate for numpad keys
    if(Environment::getInstance()->CtrlDown && Environment::getInstance()->ShiftDown) keypad_object_move_ratio = 0.05;
    else if(Environment::getInstance()->ShiftDown) keypad_object_move_ratio=0.2;
    else if(Environment::getInstance()->CtrlDown) keypad_object_move_ratio=0.3;

    if( keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0)
    {
      // Move scale and rotate with numpad keys
      if( Selection->type == eEntry_WMO )
      {
        _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
        Selection->data.wmo->pos.x += keyx * keypad_object_move_ratio;
        Selection->data.wmo->pos.y += keyy * keypad_object_move_ratio;
        Selection->data.wmo->pos.z += keyz * keypad_object_move_ratio;
        Selection->data.wmo->dir.y += keyr * keypad_object_move_ratio * 2.0;
        _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
      }

      if( Selection->type == eEntry_Model )
      {
        _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
        Selection->data.model->pos.x += keyx * keypad_object_move_ratio;
        Selection->data.model->pos.y += keyy * keypad_object_move_ratio;
        Selection->data.model->pos.z += keyz * keypad_object_move_ratio;
        Selection->data.model->dir.y += keyr * keypad_object_move_ratio * 2.0;
        Selection->data.model->sc += keys * keypad_object_move_ratio / 50.0;
        _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
      }
    }

    Vec3D ObjPos;
    if( _world->IsSelection( eEntry_Model ) )
    {
      //! \todo  Tell me what this is.
      ObjPos = Selection->data.model->pos - _world->camera;
      rotate( 0.0f, 0.0f, &ObjPos.x, &ObjPos.y, av * PI / 180.0f );
      rotate( 0.0f, 0.0f, &ObjPos.x, &ObjPos.z, ah * PI / 180.0f );
      ObjPos.x = abs( ObjPos.x );
    }

    // moving and scaling objects
    //! \todo  Alternatively automatically align it to the terrain. Also try to move it where the mouse points.
    if( _is_currently_moving_object )
      if( Selection->type == eEntry_WMO )
      {
         _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z); // before move
         ObjPos.x = 80.0f;
         Selection->data.wmo->pos+=mv * dirUp * ObjPos.x;
         Selection->data.wmo->pos-=mh * dirRight * ObjPos.x;
         Selection->data.wmo->extents[0] = Selection->data.wmo->pos - Vec3D(1,1,1);
         Selection->data.wmo->extents[1] = Selection->data.wmo->pos + Vec3D(1,1,1);
         _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z); // after move. If moved to another ADT
      }
      else if( Selection->type == eEntry_Model )
        if( Environment::getInstance()->AltDown )
        {
          _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
          float ScaleAmount;
          ScaleAmount = pow( 2.0f, mv * 4.0f );
          Selection->data.model->sc *= ScaleAmount;
          if(Selection->data.model->sc > 63.9f )
            Selection->data.model->sc = 63.9f;
          else if (Selection->data.model->sc < 0.00098f )
            Selection->data.model->sc = 0.00098f;
        }
        else
        {
          _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z); // before move
          ObjPos.x = 80.0f;
          Selection->data.model->pos += mv * dirUp * ObjPos.x;
          Selection->data.model->pos -= mh * dirRight * ObjPos.x;
          _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z); // after move. If moved to another ADT
        }


    // rotating objects
    if( look )
    {
      float * lTarget = NULL;
      bool lModify = false;

      if( Selection->type == eEntry_Model )
      {
        _world->setChanged(Selection->data.model->pos.x,Selection->data.model->pos.z);
        lModify = Environment::getInstance()->ShiftDown | Environment::getInstance()->CtrlDown | Environment::getInstance()->AltDown;
        if( Environment::getInstance()->ShiftDown )
          lTarget = &Selection->data.model->dir.y;
        else if( Environment::getInstance()->CtrlDown )
          lTarget = &Selection->data.model->dir.x;
        else if(Environment::getInstance()->AltDown )
          lTarget = &Selection->data.model->dir.z;
      }
      else if( Selection->type == eEntry_WMO )
      {
        _world->setChanged(Selection->data.wmo->pos.x,Selection->data.wmo->pos.z);
        lModify = Environment::getInstance()->ShiftDown | Environment::getInstance()->CtrlDown | Environment::getInstance()->AltDown;
        if( Environment::getInstance()->ShiftDown )
          lTarget = &Selection->data.wmo->dir.y;
        else if( Environment::getInstance()->CtrlDown )
          lTarget = &Selection->data.wmo->dir.x;
        else if( Environment::getInstance()->AltDown )
          lTarget = &Selection->data.wmo->dir.z;
      }

      if( lModify && lTarget )
      {
        *lTarget = *lTarget + rh + rv;

        if( *lTarget > 360.0f )
          *lTarget = *lTarget - 360.0f;
        else if( *lTarget < -360.0f )
          *lTarget = *lTarget + 360.0f;
      }
    }

    mh = 0;
    mv = 0;
    rh = 0;
    rv = 0;

    if( _holding_left_mouse_button && Selection->type==eEntry_MapChunk )
    {
      float xPos = Environment::getInstance()->Pos3DX;
      float yPos = Environment::getInstance()->Pos3DY;
      float zPos = Environment::getInstance()->Pos3DZ;

      switch( _current_terrain_editing_mode )
      {
      case shaping:
        if( mViewMode == eViewMode_3D )
        {
          if( Environment::getInstance()->ShiftDown )
          {
            _world->changeTerrain ( xPos
                                  , zPos
                                  , 7.5f * dt * shaping_speed()
                                  , shaping_radius()
                                  , shaping_formula()
                                  );
          }
          else if( Environment::getInstance()->CtrlDown )
          {
            _world->changeTerrain ( xPos
                                  , zPos
                                  , -7.5f * dt * shaping_speed()
                                  , shaping_radius()
                                  , shaping_formula()
                                  );
          }
        }
        break;

      case smoothing:
        if( mViewMode == eViewMode_3D )
        {
          if( Environment::getInstance()->ShiftDown )
          {
            _world->flattenTerrain ( xPos
                                   , zPos
                                   , yPos
                                   , pow( 0.2f, dt ) * smoothing_speed()
                                   , smoothing_radius()
                                   , smoothing_formula()
                                   );
          }
          else if( Environment::getInstance()->CtrlDown )
          {
            _world->blurTerrain ( xPos
                                , zPos
                                , pow( 0.2f, dt ) * smoothing_speed()
                                , std::min( smoothing_radius(), 30.0 )
                                , smoothing_formula()
                                );
          }
        }
        break;

      case texturing:
        {
          const QPointF brush_position ( mViewMode == eViewMode_3D
                                       ? QPointF (xPos, zPos)
                                       : tile_mode_brush_position()
                                       + QPointF ( _world->camera.x
                                                 , _world->camera.z
                                                 )
                                       );

          if ( Environment::getInstance()->ShiftDown
            && Environment::getInstance()->CtrlDown
             )
          {
            _world->eraseTextures (brush_position.x(), brush_position.y());
          }
          else if (Environment::getInstance()->CtrlDown)
          {
            mainGui->TexturePicker->getTextures (_world->GetCurrentSelection());
          }
          else if ( Environment::getInstance()->ShiftDown
                 && UITexturingGUI::getSelectedTexture()
                  )
          {
            _world->paintTexture ( brush_position.x()
                                 , brush_position.y()
                                 , &textureBrush
                                 , brushLevel
                                 , 1.0f - pow ( 1.0f - brushPressure
                                              , dt * 10.0f
                                              )
                                 , UITexturingGUI::getSelectedTexture()
                                 );
          }
        }
      break;

      case hole_setting:
        if (Environment::getInstance()->ShiftDown)
        {
      // if there is no terain the projection mothod dont work. So get the cords by selection.
    Selection->data.mapchunk->getSelectionCoord( &xPos, &zPos );
    yPos = Selection->data.mapchunk->getSelectionHeight();
          if( mViewMode == eViewMode_3D )      _world->removeHole( xPos, zPos );
          //else if( mViewMode == eViewMode_2D )  _world->removeHole( CHUNKSIZE * 4.0f * ratio * ( _mouse_position.x() / float( width() ) - 0.5f ) / _tile_mode_zoom + _world->camera.x, CHUNKSIZE * 4.0f * ( _mouse_position.y() / float( height() ) - 0.5f) / _tile_mode_zoom + _world->camera.z );
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          if( mViewMode == eViewMode_3D )      _world->addHole( xPos, zPos );
          //else if( mViewMode == eViewMode_2D )  _world->addHole( CHUNKSIZE * 4.0f * ratio * ( _mouse_position.x() / float( width() ) - 0.5f ) / _tile_mode_zoom+_world->camera.x, CHUNKSIZE * 4.0f * ( _mouse_position.y() / float( height() ) - 0.5f) / _tile_mode_zoom+_world->camera.z );
        }
      break;

      case area_id_setting:
        if( Environment::getInstance()->ShiftDown  )
        {
          if( mViewMode == eViewMode_3D )
          {
          // draw the selected AreaId on current selected chunk
          nameEntry * lSelection = _world->GetCurrentSelection();
          int mtx,mtz,mcx,mcy;
          mtx = lSelection->data.mapchunk->mt->mPositionX;
          mtz = lSelection->data.mapchunk->mt->mPositionZ ;
          mcx = lSelection->data.mapchunk->px;
          mcy = lSelection->data.mapchunk->py;
          _world->setAreaID( Environment::getInstance()->selectedAreaID, mtx,mtz, mcx, mcy );
          }
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          if( mViewMode == eViewMode_3D )
          {
          // pick areaID from chunk
          int newID = _world->GetCurrentSelection()->data.mapchunk->areaID;
          Environment::getInstance()->selectedAreaID = newID;
          mainGui->ZoneIDBrowser->setZoneID(newID);
          }
        }

      break;

      case impassable_flag_setting:
        if( Environment::getInstance()->ShiftDown  )
        {
          if( mViewMode == eViewMode_3D ) _world->setFlag( true, xPos, zPos );
        }
        else if( Environment::getInstance()->CtrlDown )
        {
          if( mViewMode == eViewMode_3D ) _world->setFlag( false, xPos, zPos );
        }
      break;
      }
    }
  }

  if( mViewMode != eViewMode_2D )
  {
    if( moving )
      _world->camera += dir * dt * movespd * moving;
    if( strafing )
    {
      Vec3D right = dir % Vec3D( 0.0f, 1.0f ,0.0f );
      right.normalize();
      _world->camera += right * dt * movespd * strafing;
    }
    if( updown )
      _world->camera += Vec3D( 0.0f, dt * movespd * updown, 0.0f );

    _world->lookat = _world->camera + dir;
  }
  else
  {
    if( moving )
      _world->camera.z -= dt * movespd * moving / ( _tile_mode_zoom * 1.5f );
    if( strafing )
      _world->camera.x += dt * movespd * strafing / ( _tile_mode_zoom * 1.5f );
    if( updown )
      _tile_mode_zoom *= pow( 2.0f, dt * updown * 4.0f );

    _tile_mode_zoom = qBound (0.1, _tile_mode_zoom, 2.0);
  }

  _world->time += mTimespeed * dt;
  _world->animtime += dt * 1000.0f;
  globalTime = static_cast<int>( _world->animtime );

  _world->tick(dt);

  if( !_map_chunk_properties_window->hidden() && _world->GetCurrentSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk )
  {
    UITexturingGUI::setChunkWindow( _world->GetCurrentSelection()->data.mapchunk );
  }
}

void MapView::setup_tile_mode_rendering() const
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();

  const qreal ratio (width() / qreal (height()));
  glOrtho (-2.0f * ratio, 2.0f * ratio, 2.0f, -2.0f, -100.0f, 300.0f );

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
}

void MapView::setup_2d_rendering() const
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();

  glOrtho (0.0f, width(), height(), 0.0f, -1.0f, 1.0f);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
}

static const qreal nearclip (1.0);
static const qreal fov (45.0);
void MapView::setup_3d_rendering() const
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();

  const qreal ratio (width() / qreal (height()));
  gluPerspective (fov, ratio, 1.0f, _viewing_distance);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
}

void MapView::setup_3d_selection_rendering() const
{
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();

  GLint viewport[4] = {0, 0, width(), height()};
  gluPickMatrix ( _mouse_position.x()
                , height() - _mouse_position.y()
                , 7
                , 7
                , viewport
                );

  const qreal ratio (width() / qreal (height()));
  gluPerspective (fov, ratio, 1.0f, _viewing_distance);

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();
}

void MapView::doSelection( bool selectTerrainOnly )
{
  setup_3d_selection_rendering();

  _world->drawSelection ( _draw_wmo_doodads
                        , _draw_wmos && !selectTerrainOnly
                        , _draw_doodads && !selectTerrainOnly
                        , _draw_terrain
                        );
}

void MapView::displayGUIIfEnabled()
{
  if( _GUIDisplayingEnabled )
  {
    setup_2d_rendering();

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    opengl::texture::disable_texture (1);
    opengl::texture::enable_texture (0);

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glDisable( GL_LIGHTING );
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

    opengl::texture::disable_texture (0);

    mainGui->render();

    opengl::texture::enable_texture (0);
  }

  //! \todo This should only be done when actually needed. (on movement and camera changes as well as modifying an adt)
  _minimap->update();
}

QPointF MapView::tile_mode_brush_position() const
{
  const QPointF mouse_pos ( _mouse_position.x() / qreal (width()) - 0.5
                          , _mouse_position.y() / qreal (height()) - 0.5
                          );

  static const qreal arbitrary_constant_due_to_viewport (CHUNKSIZE * 4.0);

  return QPointF ( arbitrary_constant_due_to_viewport
                 * width() / qreal (height())
                 * mouse_pos.x()
                 / _tile_mode_zoom
                 , arbitrary_constant_due_to_viewport
                 * mouse_pos.y()
                 / _tile_mode_zoom
                 );
}

void MapView::draw_tile_mode_brush() const
{
  const qreal brush_radius (textureBrush.getRadius());
  const qreal brush_diameter (brush_radius * 2.0);
  const QPointF brush_position (tile_mode_brush_position());

  glPushMatrix();

  glColor4f (1.0f, 1.0f, 1.0f, 0.5f);
  opengl::texture::enable_texture (0);
  textureBrush.getTexture()->bind();

  glScalef (_tile_mode_zoom / CHUNKSIZE, _tile_mode_zoom / CHUNKSIZE, 1.0f);
  glTranslatef ( brush_position.x() - textureBrush.getRadius()
               , brush_position.y() - textureBrush.getRadius()
               , 0.0f
               );

  glBegin (GL_QUADS);
  glTexCoord2f (0.0f, 0.0f);
  glVertex3f (0.0f, brush_diameter, 0.0f);
  glTexCoord2f (1.0f, 0.0f);
  glVertex3f (brush_diameter, brush_diameter, 0.0f);
  glTexCoord2f (1.0f, 1.0f);
  glVertex3f (brush_diameter, 0.0f, 0.0f);
  glTexCoord2f (0.0f, 1.0f);
  glVertex3f (0.0f, 0.0f, 0.0f);
  glEnd();
  glPopMatrix();
}

void MapView::displayViewMode_2D()
{
  setup_tile_mode_rendering();
  _world->drawTileMode ( _draw_lines, width() / qreal (height())
                       , _tile_mode_zoom
                       );
  draw_tile_mode_brush();
  displayGUIIfEnabled();
}

void MapView::displayViewMode_3D()
{
  //! \note Select terrain below mouse, if no item selected or the item is map.
  if ( !_world->IsSelection( eEntry_Model )
    && !_world->IsSelection( eEntry_WMO )
    && _automatically_update_terrain_selection
     )
  {
    doSelection (true);
  }

  setup_3d_rendering();

  float brush_radius (0.3f);

  if (_current_terrain_editing_mode == shaping)
    brush_radius = shaping_radius();
  else if (_current_terrain_editing_mode == smoothing)
    brush_radius = smoothing_radius();
  else if (_current_terrain_editing_mode == texturing)
    brush_radius = textureBrush.getRadius();

  _world->draw ( _draw_terrain_height_contour
               , _current_terrain_editing_mode == impassable_flag_setting
               , _current_terrain_editing_mode == area_id_setting
               , _current_terrain_editing_mode == hole_setting
               , brush_radius
               , brush_radius
               , _draw_wmo_doodads
               , _draw_fog
               , _draw_wmos
               , _draw_terrain
               , _draw_doodads
               , _draw_lines
               , _draw_hole_lines
               , _draw_water
               );

  displayGUIIfEnabled();
}

void MapView::display()
{
  //! \todo  Get this out or do it somehow else. This is ugly and is a senseless if each draw.
  if (_save_to_minimap_on_next_drawing)
  {
    setup_tile_mode_rendering();
    _world->saveMap();
    _save_to_minimap_on_next_drawing = false;
  }

  switch( mViewMode )
  {
  case eViewMode_2D:
    displayViewMode_2D();
    break;

  case eViewMode_3D:
    displayViewMode_3D();
    break;
  }
}

void MapView::resizewindow()
{
  mainGui->resize();
}

void MapView::keyPressEvent (QKeyEvent* event)
{
  //! \todo Implement GUI stuff again?
//  if( _last_clicked_ui_frame && _last_clicked_ui_frame->keyPressEvent( event ) )
//    return;

  if (event->key() == Qt::Key_Shift)
    Environment::getInstance()->ShiftDown = true;

  if (event->key() == Qt::Key_Alt)
    Environment::getInstance()->AltDown = true;

  if (event->key() == Qt::Key_Control)
    Environment::getInstance()->CtrlDown = true;

  // movement
  if (event->key() == Qt::Key_W)
  {
      key_w = true;
      moving = 1.0f;
  }

  if (event->key() == Qt::Key_S)
    moving = -1.0f;

  if (event->key() == Qt::Key_A)
    strafing = -1.0f;

  if (event->key() == Qt::Key_D)
    strafing = 1.0f;

  if (event->key() == Qt::Key_E)
    updown = -1.0f;

  if (event->key() == Qt::Key_Q)
    updown = 1.0f;

  //! \todo FUCK.
/*
  // position correction with num pad
  if (event->key() == Qt::Key_KP8 )
    keyx = -1;

  if (event->key() == Qt::Key_KP2 )
    keyx = 1;

  if (event->key() == Qt::Key_KP6 )
    keyz = -1;

  if (event->key() == Qt::Key_KP4 )
    keyz = 1;

  if (event->key() == Qt::Key_KP1 )
    keyy = -1;

  if (event->key() == Qt::Key_KP3 )
    keyy = 1;

  if (event->key() == Qt::Key_KP7 )
    keyr = 1;

  if (event->key() == Qt::Key_KP9 )
    keyr = -1;
    */

  if (event->key() == Qt::Key_X)
    toggle_terrain_mode_window();

  if (event->key() == Qt::Key_I && event->modifiers() & Qt::ControlModifier)
    toggle_painting_mode (!Environment::getInstance()->paintMode);

//  NEW_ACTION (snap_object_to_ground, tr ("Snap object to ground"), SLOT (snap_selected_object_to_ground()), Qt::Key_PageDown);
//  NEW_TOGGLE_ACTION (rotation_randomization, tr ("Randomized rotation when copying"), SLOT (toggle_copy_rotation_randomization (bool)), 0, false);


  if (event->key() == Qt::Key_C)
    Environment::getInstance()->cursorType = (Environment::getInstance()->cursorType + 1) % 4;

  if (event->key() == Qt::Key_U)
    toggle_tile_mode();

  if (event->key() == Qt::Key_Y)
    cycle_brush_type();

  if (event->key() == Qt::Key_Tab)
    toggle_interface();


  if (event->key() == Qt::Key_F1 && event->modifiers() & Qt::ShiftModifier)
    toggle_terrain_texturing_mode();

  // fog distance or brush radius
  if (event->key() == Qt::Key_Plus)
    if( event->modifiers() & Qt::AltModifier )
    {
      increase_brush_size();
    }
    else if( event->modifiers() & Qt::ShiftModifier && ( !_world->HasSelection() || ( _world->HasSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk) )  )
      _world->fogdistance += 60.0f;// fog change only when no model is selected!
    else
    {
      //change selected model size
      keys=1;
    }

  if (event->key() == Qt::Key_Minus)
    if (event->modifiers() & Qt::AltModifier)
    {
      decrease_brush_size();
    }
    else if( event->modifiers() & Qt::ShiftModifier && ( !_world->HasSelection() || ( _world->HasSelection() && _world->GetCurrentSelection()->type == eEntry_MapChunk) )  )
      _world->fogdistance -= 60.0f; // fog change only when no model is selected!
    else
    {
      //change selected model sizesize
      keys=-1;
    }

  // doodads set
  if( event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9 )
  {
    if( _world->IsSelection( eEntry_WMO ) )
    {
      _world->GetCurrentSelection()->data.wmo->doodadset = event->key() - Qt::Key_0;
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
      switch (event->key())
      {
        case Qt::Key_1:
          movespd = 15.0f;
          break;

        case Qt::Key_2:
          movespd = 50.0f;
          break;

        case Qt::Key_3:
          movespd = 300.0f;
          break;

        case Qt::Key_4:
          movespd = 1000.0f;
          break;
      }
    }
    else if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_6)
    {
      set_terrain_editing_mode
        (terrain_editing_modes (event->key() - Qt::Key_1));
    }
  }
}

void MapView::toggle_detail_info_window (bool value)
{
  mainGui->guidetailInfos->setVisible (value);
}

void MapView::toggle_terrain_mode_window()
{
  if(_current_terrain_editing_mode == texturing)
    view_texture_palette( 0, 0 );
  else if(_current_terrain_editing_mode == area_id_setting)
    mainGui->ZoneIDBrowser->toggleVisibility();
}

void MapView::toggle_painting_mode (bool value)
{
  Environment::getInstance()->paintMode = value;
}

void MapView::invert_mouse_y_axis (bool value)
{
  mousedir = value ? 1.0f : -1.0f;
}

/*!
  \brief Delete the current selected model
*/
void MapView::delete_selected_object()
{
  if( _world->IsSelection( eEntry_WMO ) )
    _world->deleteWMOInstance( _world->GetCurrentSelection()->data.wmo->mUniqueID );
  else if( _world->IsSelection( eEntry_Model ) )
    _world->deleteModelInstance( _world->GetCurrentSelection()->data.model->d1 );
}

/*!
  \brief Paste a model
  Paste the current model stored in Environment::getInstance()->get_clipboard() at the cords of the selected model or chunk.
*/
void MapView::paste_object()
{
  if( _world->HasSelection() )
  {
    nameEntry lClipboard = Environment::getInstance()->get_clipboard();
    Vec3D position;
    switch( _world->GetCurrentSelection()->type )
     {
      case eEntry_Model:
        position = _world->GetCurrentSelection()->data.model->pos;
        break;
      case eEntry_WMO:
        position = _world->GetCurrentSelection()->data.wmo->pos;
        break;
      case eEntry_MapChunk:
        position = _world->GetCurrentSelection()->data.mapchunk->
                           GetSelectionPosition();
        break;
      default:
        break;
    }
    _world->addModel ( lClipboard
                     , position
                     , _copy_size_randomization
                     , _copy_position_randomization
                     , _copy_rotation_randomization
                     );
  }
}

/*!
  \brief Copy selected model to clipboard
  Copy the selected m2 or WMO with getInstance()->set_clipboard()
*/
void MapView::copy_selected_object()
{
  if( _world->HasSelection() )
  {
    Environment::getInstance()->set_clipboard( _world->GetCurrentSelection() );
  }
}

void MapView::increase_moving_speed()
{
  movespd *= 2.0f;
}

void MapView::decrease_moving_speed()
{
  movespd *= 0.5f;
}

void MapView::save_minimap()
{
  //! \todo This needs to be actually done here, not deferred to next display().
  _save_to_minimap_on_next_drawing = true;
}

void MapView::turn_around()
{
  ah += 180.0f;
}

void MapView::reset_selected_object_rotation()
{
  if( _world->IsSelection( eEntry_WMO ) )
  {
    _world->GetCurrentSelection()->data.wmo->resetDirection();
    _world->setChanged(_world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z);
  }
  else if( _world->IsSelection( eEntry_Model ) )
  {
    _world->GetCurrentSelection()->data.model->resetDirection();
    _world->setChanged(_world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z);
  }
}

void MapView::snap_selected_object_to_ground()
{
  if( _world->IsSelection( eEntry_WMO ) )
  {
    Vec3D t = Vec3D( _world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z, 0 );
    _world->GetVertex( _world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z, &t );
    _world->GetCurrentSelection()->data.wmo->pos = t;
    _world->setChanged(_world->GetCurrentSelection()->data.wmo->pos.x, _world->GetCurrentSelection()->data.wmo->pos.z);

  }
  else if( _world->IsSelection( eEntry_Model ) )
  {
    Vec3D t = Vec3D( _world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z, 0 );
    _world->GetVertex( _world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z, &t );
    _world->GetCurrentSelection()->data.model->pos = t;
    _world->setChanged(_world->GetCurrentSelection()->data.model->pos.x, _world->GetCurrentSelection()->data.model->pos.z);
  }
}

void MapView::toggle_doodad_spawner()
{
  _doodad_spawner->hidden (!_doodad_spawner->hidden());
}

void MapView::toggle_interface()
{
  _GUIDisplayingEnabled = !_GUIDisplayingEnabled;
}

void MapView::increase_time_speed()
{
  mTimespeed += 90.0f;
}
void MapView::decrease_time_speed()
{
  mTimespeed -= 90.0f;
}

void MapView::toggle_terrain_texturing_mode()
{
  if( alloff )
  {
    alloff_models = _draw_doodads;
    alloff_doodads = _draw_wmo_doodads;
    alloff_contour = _draw_terrain_height_contour;
    alloff_wmo = _draw_wmos;
    alloff_fog = _draw_fog;
    alloff_terrain = _draw_terrain;

    _draw_doodads = false;
    _draw_wmo_doodads = false;
    _draw_terrain_height_contour = true;
    _draw_wmos = false;
    _draw_terrain = true;
    _draw_fog = false;
  }
  else
  {
    _draw_doodads = alloff_models;
    _draw_wmo_doodads = alloff_doodads;
    _draw_terrain_height_contour = alloff_contour;
    _draw_wmos = alloff_wmo;
    _draw_terrain = alloff_terrain;
    _draw_fog = alloff_fog;
  }
  alloff = !alloff;
}

void MapView::toggle_auto_selecting (bool value)
{
  _automatically_update_terrain_selection = value;
}

/// --- Drawing toggles --------------------------------------------------------

void MapView::toggle_doodad_drawing (bool value)
{
  _draw_doodads = value;
}
void MapView::toggle_water_drawing (bool value)
{
  _draw_water = value;
}
void MapView::toggle_terrain_drawing (bool value)
{
  _draw_terrain = value;
}
void MapView::toggle_wmo_doodad_drawing (bool value)
{
  _draw_wmo_doodads = value;
}
void MapView::toggle_line_drawing (bool value)
{
  _draw_lines = value;
}
void MapView::toggle_wmo_drawing (bool value)
{
  _draw_wmos = value;
}
void MapView::toggle_hole_line_drawing (bool value)
{
  _draw_hole_lines = value;
}
void MapView::toggle_lighting (bool value)
{
  _draw_lighting = value;
}


//! \todo these should be symetrical, so maybe combine.
void MapView::increase_brush_size()
{
  switch( _current_terrain_editing_mode )
  {
  case shaping:
    shaping_radius (shaping_radius() + 0.01);
    break;
  case smoothing:
    smoothing_radius (smoothing_radius() + 0.01);
    break;
  case texturing:
    textureBrush.setRadius (qBound (0.1f, textureBrush.getRadius() + 0.1f, 100.0f));
    paint_brush->setValue( textureBrush.getRadius() / 100 );
    break;
  default:
    break;
  }
}

void MapView::decrease_brush_size()
{
  switch( _current_terrain_editing_mode )
  {
  case shaping:
    shaping_radius (shaping_radius() - 0.01f);
    break;
  case smoothing:
    smoothing_radius (smoothing_radius() - 0.01);
    break;
  case texturing:
    textureBrush.setRadius (qBound (0.1f, textureBrush.getRadius() - 0.1f, 100.0f));
    paint_brush->setValue( textureBrush.getRadius() / 100 );
    break;
  default:
    break;
  }
}

void MapView::toggle_minimap (bool value)
{
  _minimap->setVisible (value);
}

void MapView::save_all()
{
  _world->saveChanged();
}

void MapView::save()
{
  _world->saveTile( static_cast<int>( _world->camera.x ) / TILESIZE, static_cast<int>( _world->camera.z ) / TILESIZE );
}

void MapView::reload_current_tile()
{
  _world->reloadTile( static_cast<int>( _world->camera.x ) / TILESIZE, static_cast<int>( _world->camera.z ) / TILESIZE );
}

void MapView::exit_to_menu()
{
  close();
}

void MapView::cycle_brush_type()
{
  // toogle between smooth / flat / linear
  switch( _current_terrain_editing_mode )
  {
  case shaping:
    shaping_formula ((shaping_formula() + 1) % shaping_formula_type::shaping_formula_types);
    break;

  case smoothing:
    smoothing_formula ((smoothing_formula() + 1) % smoothing_formula_type::smoothing_formula_types);
    break;

  default:
    break;
  }
}

void MapView::toggle_contour_drawing (bool value)
{
  _draw_terrain_height_contour = value;
}

void MapView::toggle_fog_drawing (bool value)
{
  _draw_fog = value;
}

void MapView::toggle_tile_mode()
{
  if( mViewMode == eViewMode_2D )
  {
    mViewMode = eViewMode_3D;
    set_terrain_editing_mode (_terrain_editing_mode_before_2d);
  }
  else
  {
    mViewMode = eViewMode_2D;
    _terrain_editing_mode_before_2d = _current_terrain_editing_mode;
    set_terrain_editing_mode (texturing);
  }
}

struct BookmarkEntry
{
  int map_id;
  int area_id;
  Vec3D position;
  float rotation;
  float tilt;
};

void MapView::add_bookmark()
{
  QSettings settings;

  //! \todo This MUST be easier and not needing to read and insert everything.
  QList<BookmarkEntry> bookmarks;

  int bookmarks_count (settings.beginReadArray ("bookmarks"));
  for (int i (0); i < bookmarks_count; ++i)
  {
    settings.setArrayIndex (i);

    BookmarkEntry b;
    b.map_id = settings.value ("map_id").toInt();
    b.position.x = settings.value ("camera/position/x").toFloat();
    b.position.y = settings.value ("camera/position/y").toFloat();
    b.position.z = settings.value ("camera/position/z").toFloat();
    b.rotation = settings.value ("camera/rotation").toFloat();
    b.tilt = settings.value ("camera/tilt").toFloat();
    b.area_id = settings.value ("area_id").toInt();

    bookmarks.append (b);
  }
  settings.endArray();

  BookmarkEntry new_bookmark;
  new_bookmark.map_id = _world->getMapID();
  new_bookmark.area_id = _world->getAreaID();
  new_bookmark.position = Vec3D (_world->camera.x, _world->camera.y, _world->camera.z);
  new_bookmark.rotation = ah;
  new_bookmark.tilt = av;

  bookmarks.append (new_bookmark);

  settings.beginWriteArray ("bookmarks");
  for (int i (0); i < bookmarks.size(); ++i)
  {
    settings.setArrayIndex (i);

    settings.setValue ("map_id", bookmarks[i].map_id);
    settings.setValue ("camera/position/x", bookmarks[i].position.x);
    settings.setValue ("camera/position/y", bookmarks[i].position.y);
    settings.setValue ("camera/position/z", bookmarks[i].position.z);
    settings.setValue ("camera/rotation", bookmarks[i].rotation);
    settings.setValue ("camera/tilt", bookmarks[i].tilt);
    settings.setValue ("area_id", bookmarks[i].area_id);
  }
  settings.endArray();

  //! \todo Signal the change of settings somehow, so Menu can update.
}

void MapView::keyReleaseEvent (QKeyEvent* event)
{
  if (event->key() == Qt::Key_Shift)
  {
    Environment::getInstance()->ShiftDown = false;
  }

  if (event->key() == Qt::Key_Alt)
  {
    Environment::getInstance()->AltDown = false;
  }

  if (event->key() == Qt::Key_Control)
  {
    Environment::getInstance()->CtrlDown = false;
  }

  // movement
  if (event->key() == Qt::Key_W)
  {
    key_w = false;
    if( !(_holding_left_mouse_button && _holding_right_mouse_button) && moving > 0.0f) moving = 0.0f;
  }

  if (event->key() == Qt::Key_S && moving < 0.0f )
    moving = 0.0f;

  if (event->key() == Qt::Key_D && strafing > 0.0f )
    strafing = 0.0f;

  if (event->key() == Qt::Key_A && strafing < 0.0f )
    strafing = 0.0f;

  if (event->key() == Qt::Key_Q && updown > 0.0f )
    updown = 0.0f;

  if (event->key() == Qt::Key_E && updown < 0.0f )
    updown = 0.0f;

  //! \todo FUCK.
/*
  if (event->key() == Qt::Key_KP8 )
    keyx = 0;

  if (event->key() == Qt::Key_KP2 )
    keyx = 0;

  if (event->key() == Qt::Key_KP6 )
    keyz = 0;

  if (event->key() == Qt::Key_KP4 )
    keyz = 0;

  if (event->key() == Qt::Key_KP1 )
    keyy = 0;

  if (event->key() == Qt::Key_KP3 )
    keyy = 0;

  if (event->key() == Qt::Key_KP7 )
    keyr = 0;

  if (event->key() == Qt::Key_KP9 )
    keyr = 0;

  if (event->key() == Qt::Key_KP_MINUS || e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
    keys = 0;
    */
}

void MapView::mousePressEvent (QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    _holding_left_mouse_button = true;
  }
  if (event->button() == Qt::RightButton)
  {
    _holding_right_mouse_button = true;
  }
  if (event->button() == Qt::MidButton)
  {
    if (_world->HasSelection())
    {
      _is_currently_moving_object = true;
    }
  }

  if (_holding_left_mouse_button && _holding_right_mouse_button)
  {
   // Both buttons
   moving = 1.0f;
  }
  else if (_holding_left_mouse_button)
  {
    _last_clicked_ui_frame = mainGui->processLeftClick (event->x(), event->y());
    if (mViewMode == eViewMode_3D && !_last_clicked_ui_frame)
    {
      doSelection (false);
    }
  }
  else if (_holding_right_mouse_button)
  {
    look = true;
  }
}

void MapView::mouseReleaseEvent (QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    _holding_left_mouse_button = false;

    if( _last_clicked_ui_frame )
      _last_clicked_ui_frame->processUnclick();

    if(!key_w && moving > 0.0f )
      moving = 0.0f;

    if( mViewMode == eViewMode_2D )
    {
      strafing = 0;
      moving = 0;
    }
  }
  if (event->button() == Qt::RightButton)
  {
    _holding_right_mouse_button = false;

    look = false;

    if(!key_w && moving > 0.0f )moving = 0.0f;

    if( mViewMode == eViewMode_2D )
    {
      updown = 0;
    }
  }
  if (event->button() == Qt::MidButton)
  {
    _is_currently_moving_object = false;
  }
}

void MapView::mouseMoveEvent (QMouseEvent* event)
{
  static const float XSENS (15.0f);
  static const float YSENS (15.0f);

  const QPoint relative_move (event->pos() - _last_drag_position);

  if (look && event->modifiers() == Qt::NoModifier)
  {
    ah += relative_move.x() / XSENS;
    av += mousedir * relative_move.y() / YSENS;

    av = qBound (-80.0f, av, 80.0f);
  }

  if( _is_currently_moving_object )
  {
    const qreal ratio (height() / qreal (width()));
    mh = -ratio * relative_move.x() / qreal (width());
    mv = -relative_move.y() / qreal (height());
  }
  else
  {
    mh = 0.0f;
    mv = 0.0f;
  }

  if (event->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier))
  {
    rh = relative_move.x() / XSENS * 5.0f;
    rv = relative_move.y() / YSENS * 5.0f;
  }

  if (event->buttons() & Qt::LeftButton && event->modifiers() & Qt::AltModifier)
  {
    switch( _current_terrain_editing_mode )
    {
    case shaping:
      shaping_radius (shaping_radius() + relative_move.x() / XSENS);
      break;

    case smoothing:
      shaping_radius (shaping_radius() - relative_move.x() / XSENS);
      break;

    case texturing:
      textureBrush.setRadius( textureBrush.getRadius() + relative_move.x() / XSENS );
      if( textureBrush.getRadius() > 100.0f )
        textureBrush.setRadius(100.0f);
      else if( textureBrush.getRadius() < 0.1f )
        textureBrush.setRadius(0.1f);
      paint_brush->setValue( textureBrush.getRadius() / 100.0f );
      break;
  default:
    break;
    }
  }

  if (event->buttons() & Qt::LeftButton && _last_clicked_ui_frame)
  {
    _last_clicked_ui_frame->processLeftDrag( event->x() - 4, event->y() - 4, relative_move.x(), relative_move.y() );
  }

  if ( mViewMode == eViewMode_2D
    && event->buttons() & Qt::LeftButton
    && event->modifiers() == Qt::ControlModifier
     )
  {
    strafing = ((relative_move.x() / XSENS) / -1.0f) * 5.0f;
    moving = (relative_move.y() / YSENS) * 5.0f;
  }

  if ( mViewMode == eViewMode_2D
    && event->buttons() & Qt::RightButton
    && event->modifiers() == Qt::ControlModifier
     )
  {
    updown = (relative_move.y() / YSENS);
  }

  _mouse_position = event->pos();

  Environment::getInstance()->screenX = event->x();
  Environment::getInstance()->screenY = event->y();

  _last_drag_position = event->pos();
}

void MapView::set_terrain_editing_mode (const terrain_editing_modes& mode)
{
  _current_terrain_editing_mode = mode;

  _draw_hole_lines = mode == hole_setting;

  _shaping_settings_widget->hide();
  _smoothing_settings_widget->hide();
  settings_paint->hide();

  switch (mode)
  {
  case shaping:
    _shaping_settings_widget->show();
    break;

  case smoothing:
    _smoothing_settings_widget->show();
    break;

  case texturing:
    settings_paint->show();
    break;

  default:
    break;
  }

  if (mainGui && mainGui->guiToolbar)
    mainGui->guiToolbar->set_icon_visual (_current_terrain_editing_mode);
}
