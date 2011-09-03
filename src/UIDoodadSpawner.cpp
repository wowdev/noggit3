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

const int winWidth = 500;
const int winHeight = 100;

void AddM2Click( UIFrame* f, int i )
{
  ( static_cast<UIDoodadSpawner *>( f->parent() ) )->AddM2();
}

UIDoodadSpawner::UIDoodadSpawner( )
: UICloseWindow( video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f, winWidth, winHeight, "Test", true )
, _button(new UIButton( 145.0f, winHeight - 24.0f, 132.0f, 28.0f, "Test", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", AddM2Click, 0 ))
, _tbox(new UITextBox( 30.0f, 30.0f, 400.0f, 40.0f, "Interface\\Common\\Common-Input-Border.blp", "Interface\\Common\\Common-Input-Border.blp") )
{
  addChild( _button );
  addChild( _tbox );
}

void UIDoodadSpawner::AddM2()
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

  if( MPQFile::exists(_tbox->getValue()) )
  {
	  std::string ext = boost::filesystem::extension( _tbox->getValue() );
	  std::transform(ext.begin(), ext.end(),ext.begin(), ::toupper);
	  if(ext == ".M2")
	  {
      gWorld->addM2( ModelManager::add( _tbox->getValue() ), selectionPosition );
	  }
	  if(ext == ".WMO")
	  {
      gWorld->addWMO( WMOManager::add( _tbox->getValue() ), selectionPosition );
	  }
  }
 }
