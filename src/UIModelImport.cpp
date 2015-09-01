#include "UIModelImport.h"

#include <algorithm>
#include <iostream>
#include <sstream>
//#include <string>
#include "string.h"

#include "Noggit.h" // fonts
#include "Misc.h" // fonts
#include "revision.h"
#include "UIText.h"
#include "UITexture.h"
#include "Video.h" // video
#include "MapView.h"
#include "UIListView.h"
#include "UIButton.h"
#include <fstream>
#include <iostream>

#include "Log.h"

void addTXTModelext(UIFrame *f, int id)
{
	(static_cast<UIModelImport *>(f->parent()->parent()))->addTXTModel(id);
}

UIModelImport::UIModelImport(MapView *mapview)
	: UICloseWindow((float)video.xres() / 2.0f - (float)winWidth / 2.0f, (float)video.yres() / 2.0f - (float)winHeight / 2.0f, (float)winWidth, (float)winHeight, "")
{

	addChild(new UIText(12.0f, 7.0f, "Select model.", app.getArial14(), eJustifyLeft));

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
	MoldelList = new UIListView(5, 27, width() - 8, height() - 28, 20);
	MoldelList->clickable(true);
	addChild(MoldelList);
	//  Read out Area List.
	std::string line;
	int counter = 100;
	std::ifstream fileReader("Import.txt");

	if (fileReader.is_open())
	{
		while (!fileReader.eof())
		{
			getline(fileReader, line);
			UIFrame *curFrame = new UIFrame(1, 1, 1, 1);
			std::string diveded;

			diveded = misc::explode(line, "\\");
			if (line != "")
			{
				std::stringstream ss;
				ss << counter << ": " << diveded;
				UIButton *tempButton = new UIButton(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", addTXTModelext, counter);
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
