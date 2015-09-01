#include "UIZoneIDBrowser.h"

#include <iostream>
#include <sstream>
#include <string>

#include "DBC.h"
#include "Log.h"
#include "Misc.h"
#include "Noggit.h" // app.getArial14(), arialn13
#include "UIButton.h"
#include "UIListView.h"
#include "UIMapViewGUI.h"
#include "UIScrollBar.h"
#include "UIText.h" // UIText
#include "UICloseWindow.h" // UICloseWindow

void theButtonMapPressed(UIFrame *f, int id)
{
	(static_cast<UIZoneIDBrowser::Ptr>(f->parent()))->ButtonMapPressed(id);
}

void changeZoneValue(UIFrame *f, int id)
{
	//! \todo WAT?
	(static_cast<UIZoneIDBrowser::Ptr>(f->parent()->parent()->parent()))->setZoneID(id);
}

UIZoneIDBrowser::UIZoneIDBrowser(int xPos, int yPos, int w, int h, UIMapViewGUI *setGui)
	: UICloseWindow((float)xPos, (float)yPos, (float)w, (float)h, "", true)
	, changeFunc(NULL)
	, mainGui(setGui)
	, ZoneIdList(NULL)
	, mapID(-1)
	, zoneID(-1)
	, subZoneID(-1)
	, selectedAreaID(-1)
	, MapName("")
	, ZoneName("")
	, SubZoneName("")
	, backZone(new UIButton(387.5f, 4.0f, 24.0f, 24.0f, "", "Interface\\BUTTONS\\UI-RotationLeft-Button-Up.blp", "Interface\\BUTTONS\\UI-RotationLeft-Button-Down.blp", theButtonMapPressed, 0))
	, ZoneIDPath(new UIText(10.0f, 6.0f, "", app.getArial12(), eJustifyLeft))
{
	addChild(ZoneIDPath);
	addChild(backZone);
}

void UIZoneIDBrowser::setMapID(int id)
{
	mapID = id;
	zoneID = 0;
	subZoneID = 0;
	for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
	{
		if (i->getInt(MapDB::MapID) == id)
			MapName = i->getString(MapDB::InternalName);
	}
	buildAreaList();
	refreshMapPath();
}

void UIZoneIDBrowser::setZoneID(int id)
{
	for (DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i)
	{
		if (i->getInt(AreaDB::AreaID) == id)
		{
			if (i->getUInt(AreaDB::Region) == 0)
			{
				ZoneName = gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
				zoneID = id;
				subZoneID = 0;
				SubZoneName = "";;
				if (changeFunc)
					changeFunc(this, id);
			}
			else
			{
				SubZoneName = gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
				subZoneID = id;
				if (changeFunc)
					changeFunc(this, id);
			}
		}
	}
	buildAreaList();
	refreshMapPath();
}

void UIZoneIDBrowser::ButtonMapPressed(int id)
{
	if (id == 0)
	{
		if (subZoneID)
		{
			// clear subzone
			subZoneID = 0;
			SubZoneName = "";
			if (changeFunc)
				changeFunc(this, zoneID);
		}
		else
		{
			// clear zone
			zoneID = 0;
			ZoneName = "";
			if (changeFunc)
				changeFunc(this, 0);
		}
		refreshMapPath();
		buildAreaList();
	}
}

void UIZoneIDBrowser::buildAreaList()
{
	removeChild(ZoneIdList);
	ZoneIdList = new UIListView(4, 24, width() - 8, height() - 28, 20);
	ZoneIdList->clickable(true);
	addChild(ZoneIdList);
	//  Read out Area List.
	for (DBCFile::Iterator i = gAreaDB.begin(); i != gAreaDB.end(); ++i)
	{
		if (i->getInt(AreaDB::Continent) == mapID)
		{
			if (zoneID == 0)
			{
				if (i->getUInt(AreaDB::Region) == 0)
				{
					UIFrame *curFrame = new UIFrame(1, 1, 1, 1);
					std::stringstream ss;
					ss << i->getInt(AreaDB::AreaID) << "-" << gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
					UIButton *tempButton = new UIButton(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", changeZoneValue, i->getInt(AreaDB::AreaID));
					tempButton->setLeft();
					curFrame->addChild(tempButton);
					ZoneIdList->addElement(curFrame);
				}
			}
			else if (zoneID > 0)
			{
				if (i->getUInt(AreaDB::Region) == zoneID)
				{
					UIFrame *curFrame = new UIFrame(1, 1, 1, 1);
					std::stringstream ss;
					ss << i->getInt(AreaDB::AreaID) << "-" << gAreaDB.getAreaName(i->getInt(AreaDB::AreaID));
					UIButton *tempButton = new UIButton(0.0f, 0.0f, 400.0f, 28.0f, ss.str(), "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", "Interface\\DialogFrame\\UI-DialogBox-Background-Dark.blp", changeZoneValue, i->getInt(AreaDB::AreaID));
					tempButton->setLeft();
					curFrame->addChild(tempButton);
					ZoneIdList->addElement(curFrame);
				}
			}
		}

	}
	ZoneIdList->recalcElements(1);
}

void UIZoneIDBrowser::refreshMapPath()
{
	std::stringstream AreaPath;
	if (SubZoneName != "")
		AreaPath << MapName << " < " << SubZoneName;
	else
		AreaPath << MapName << " < " << ZoneName;
	ZoneIDPath->setText(AreaPath.str());
}

void UIZoneIDBrowser::setChangeFunc(void(*f)(UIFrame *, int))
{
	changeFunc = f;
}
