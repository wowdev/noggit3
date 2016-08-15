#include "UIModelImport.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "Noggit.h" // fonts
#include "Misc.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UITexture.h"
#include "Video.h" // video
#include "MapView.h"
#include "UIListView.h"
#include "UIButton.h"
#include "Settings.h"

#include "Log.h"

void addTXTModelext(UIFrame *f, int id)
{
	(static_cast<UIModelImport *>(f->parent()->parent()))->addTXTModel(id);
}

void updateModelList(UIFrame *f, int id)
{
  (static_cast<UIModelImport *>(f->parent())->builModelList());
}

void UIModelImport__TextBoxEnter(UITextBox::Ptr textBox, const std::string& value)
{
  (static_cast<UIModelImport *>(textBox->parent())->builModelList());
}

UIModelImport::UIModelImport(MapView *mapview)
	: UICloseWindow((float)video.xres() - (float)winWidth - 5.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f, (float)winWidth, (float)winHeight, "", true)
{
  _mapView = mapview;
	addChild(new UIText(12.0f, 10.0f, "Select model", app.getArial14(), eJustifyLeft));
  _textBox = new UITextBox(105.0f, 7.0f, 220.0f, 40.0f, UIModelImport__TextBoxEnter);
  addChild(_textBox);
  addChild(new UIButton(330.0f, 7.0f, 75.0f, 30.0f, "Filter", "Interface\\BUTTONS\\UI-DialogBox-Button-Up.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", updateModelList, 0));

	builModelList();

}

void UIModelImport::addTXTModel(int id)
{
	// do it
	_mapView->addModelFromTextSelection(id);

}

void UIModelImport::builModelList()
{
	removeChild(MoldelList);
	MoldelList = new UIListView(5, 37, width() - 8, height() - 38, 20);
	MoldelList->clickable(true);
	addChild(MoldelList);
	//  Read out Area List.
	std::string line;
  const std::string filter = _textBox->value();
	int counter = 1;
  bool filtered = filter != "";
	std::ifstream fileReader(Settings::getInstance()->importFile);

	if (fileReader.is_open())
	{
		while (!fileReader.eof())
		{
			getline(fileReader, line);
			
			if (line != "")
			{
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        std::string diveded = misc::explode(line, "\\");

        if (filtered && diveded.find(filter, 0) == std::string::npos)
        {
          continue;
        }

				std::stringstream ss;
				ss << counter << ": " << diveded;
        UIFrame *curFrame = new UIFrame(1, 1, 1, 1);
				UIButton *tempButton = new UIButton(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), 
                                            "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", 
                                            "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", 
                                            addTXTModelext, 
                                            counter+99
                                           ); // first id from file = 100 
				tempButton->setLeft();
				curFrame->addChild(tempButton);
				MoldelList->addElement(curFrame);
				counter++;
			}
		}
	}
	MoldelList->recalcElements(1);
}


void UIModelImport::resize()
{
	x(std::max((video.xres() / 2.0f) - (winWidth / 2.0f), 0.0f));
	y(std::max((video.yres() / 2.0f) - (winHeight / 2.0f), 0.0f));
}
