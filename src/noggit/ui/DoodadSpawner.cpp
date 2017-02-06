// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/DoodadSpawner.h>

#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#include <noggit/MapChunk.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/application.h> // app.getArial14()
#include <noggit/ui/Button.h>
#include <noggit/ui/TextBox.h>
#include <noggit/Video.h> // video
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>

#include <noggit/Log.h>

//! \todo Add TreeView. Add ScrollBar. Add ModelPreview

static const float winWidth(1000.0f);
static const float winHeight(500.0f);

static UIDoodadSpawner* global_doodadSpawner_evil(nullptr);

void UIDoodadSpawner__TreeSelect(const std::string& value)
{
  global_doodadSpawner_evil->AddM2(value);
}

void UIDoodadSpawner__TextBoxEnter(UITextBox::Ptr textBox, const std::string& value)
{
  (static_cast<UIDoodadSpawner *>(textBox->parent()))->AddM2(value);
}

void AddM2Click(UIFrame* f, int i)
{
  UITextBox::Ptr textBox(static_cast<UITextBox::Ptr>(f));
  (static_cast<UIDoodadSpawner *>(textBox->parent()))->AddM2(textBox->value());
}

UIDoodadSpawner::UIDoodadSpawner()
  : UICloseWindow(video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f, winWidth, winHeight, "Test", true)
  , _tbox(new UITextBox(30.0f, 30.0f, 400.0f, 40.0f, UIDoodadSpawner__TextBoxEnter))
  , _button(new UIButton(145.0f, winHeight - 24.0f, 132.0f, 28.0f, "Test", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", AddM2Click, 0))
  , _treeView(UITreeView::Ptr())
  , modelView(new UIModel(500.0f, 30.0f, 400.0f, 400.0f))
{
  global_doodadSpawner_evil = this;
  addChild(modelView);

  //  addChild( _button );
  //  addChild( _tbox );

  Directory::Ptr fileList(new Directory());

  size_t found(std::string::npos);
  for (std::string const& entry : gListfile)
  {
    if (entry.find("m2") != std::string::npos)
    {
      found = entry.find_last_of("/");
      if (found != std::string::npos)
        fileList->addDirectory(entry.substr(0, found))->addFile(entry.substr(found + 1));
      else
        fileList->addFile(entry);
    }
  }

  _treeView = UITreeView::Ptr(new UITreeView(30.0f, 80.0f, "Models", fileList, UITreeView::Ptr(), boost::bind(&UIModel::setModel, modelView, _1)));
  addChild(_treeView.get());
}

void UIDoodadSpawner::AddM2(const std::string& filename)
{
  math::vector_3d selectionPosition;
  if (!gWorld->GetCurrentSelection())
    return;

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

  if (MPQFile::exists(filename))
  {
    std::string ext(boost::filesystem::extension(filename));
    std::transform(ext.begin(), ext.end(), ext.begin(), ::toupper);
    if (ext == ".M2")
    {
      gWorld->addM2(filename, selectionPosition, false);
    }
    if (ext == ".WMO")
    {
      gWorld->addWMO(filename, selectionPosition, false);
    }
  }
}
