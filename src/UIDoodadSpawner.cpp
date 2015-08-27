#include "UIDoodadSpawner.h"

#include <boost/filesystem.hpp>
#include <boost/bind.hpp>

#include "MapChunk.h"
#include "ModelManager.h" // ModelManager
#include "Noggit.h" // app.getArial14()
#include "UIButton.h"
#include "UITextBox.h"
#include "Video.h" // video
#include "WMOInstance.h" // WMOInstance
#include "World.h"

#include "Log.h"

//! \todo Add TreeView. Add ScrollBar. Add ModelPreview

static const float winWidth(1000.0f);
static const float winHeight(500.0f);

static UIDoodadSpawner* global_doodadSpawner_evil(NULL);

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

extern std::list<std::string> gListfile;

UIDoodadSpawner::UIDoodadSpawner()
	: UICloseWindow(video.xres() / 2.0f - winWidth / 2.0f, video.yres() / 2.0f - winHeight / 2.0f, winWidth, winHeight, "Test", true)
	, _button(new UIButton(145.0f, winHeight - 24.0f, 132.0f, 28.0f, "Test", "Interface\\Buttons\\UI-DialogBox-Button-Up.blp", "Interface\\Buttons\\UI-DialogBox-Button-Down.blp", AddM2Click, 0))
	, _tbox(new UITextBox(30.0f, 30.0f, 400.0f, 40.0f, UIDoodadSpawner__TextBoxEnter))
	, _treeView(UITreeView::Ptr())
	, modelView(new UIModel(500.0f, 30.0f, 400.0f, 400.0f))
{
	global_doodadSpawner_evil = this;
	addChild(modelView);

	//  addChild( _button );
	//  addChild( _tbox );

	Directory::Ptr fileList(new Directory());

	size_t found(std::string::npos);
	for (std::list<std::string>::const_iterator it(gListfile.begin()), end(gListfile.end()); it != end; ++it)
	{
		if (it->find("m2") != std::string::npos)
		{
			found = it->find_last_of("/\\");
			if (found != std::string::npos)
				fileList->addDirectory(it->substr(0, found))->addFile(it->substr(found + 1));
			else
				fileList->addFile(*it);
		}
	}

	_treeView = UITreeView::Ptr(new UITreeView(30.0f, 80.0f, "Models", fileList, UITreeView::Ptr(), boost::bind(&UIModel::setModel, modelView, _1)));
	addChild(_treeView.get());
}

void UIDoodadSpawner::AddM2(const std::string& filename)
{
	Vec3D selectionPosition;
	if (!gWorld->GetCurrentSelection())
		return;

	switch (gWorld->GetCurrentSelection()->type)
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
			gWorld->addWMO(WMOManager::add(filename), selectionPosition, false);
		}
	}
}
