#include "UIDoodadSpawner.h"

#include <boost/filesystem.hpp>

#include "MapChunk.h"
#include "ModelManager.h" // ModelManager
#include "Noggit.h" // arial14
#include "UIButton.h"
#include "UITextBox.h"
#include "Video.h" // video
#include "WMOInstance.h" // WMOInstance
#include "World.h"

// TODO : Add TreeView. Add ScrollBar. Add ModelPreview

static const float winWidth( 500.0f );
static const float winHeight( 100.0f );

void UIDoodadSpawner__TextBoxEnter( UITextBox::Ptr textBox, const std::string& value )
{
  ( static_cast<UIDoodadSpawner *>( textBox->parent() ) )->AddM2( value );
}

void AddM2Click( UIFrame* f, int i )
{
  UITextBox::Ptr textBox( static_cast<UITextBox::Ptr>( f ) );
  ( static_cast<UIDoodadSpawner *>( textBox->parent() ) )->AddM2( textBox->value() );
}

UIDoodadSpawner::UIDoodadSpawner( )
: UICloseWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f, winWidth, winHeight, "Test", true )
, _button( new UIButton( 145.0f, winHeight - 24.0f, 132.0f, 28.0f, "Test", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", AddM2Click, 0 ) )
, _tbox( new UITextBox( 30.0f, 30.0f, 400.0f, 40.0f, UIDoodadSpawner__TextBoxEnter ) )
{
  addChild( _button );
  addChild( _tbox );
}

void UIDoodadSpawner::AddM2( const std::string& filename )
{
  Vec3D selectionPosition;

  switch( gWorld->GetCurrentSelection()->type )
  {
    case eEntry_Model:
      selectionPosition = gWorld->GetCurrentSelection()->data.model->pos;
      break;
    case eEntry_WMO:
      selectionPosition = gWorld->GetCurrentSelection()->data.wmo->pos;
      break;
    case eEntry_MapChunk:
      selectionPosition = gWorld->GetCurrentSelection()->data.mapchunk->GetSelectionPosition();
      break;
  }

  if( MPQFile::exists( filename ) )
  {
    std::string ext( boost::filesystem::extension( filename ) );
    std::transform( ext.begin(), ext.end(), ext.begin(), ::toupper );
    if(ext == ".M2")
    {
      gWorld->addM2( ModelManager::add( filename ), selectionPosition );
    }
    if(ext == ".WMO")
    {
      gWorld->addWMO( WMOManager::add( filename ), selectionPosition );
    }
  }
 }
