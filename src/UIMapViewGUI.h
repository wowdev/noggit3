#ifndef __GUI_H
#define __GUI_H

class UIToolbar;
class UIStatusBar;
class UIDetailInfos;
class UIDoodadSpawner;
class UIAppInfo;
class UIMinimapWindow;
class UIZoneIDBrowser;
class MapView;
class UIFrame;
class UITexturePicker;
class UITextureSwitcher;
class UIHelp;
class UICursorSwitcher;
class UIWindow;
class UIExitWarning;
class UICapsWarning;
class UIWaterSaveWarning;
class UIHelperModels;
class UISlider;
class UIGradient;
class UIWater;
class UIObjectEditor;

class UIModel;
class UIAlphamap;
class UIWaterTypeBrowser;

#include "UIFrame.h"
#include "UICurrentTexture.h"

//! \todo Give better name.
class UIMapViewGUI : public UIFrame
{
private:
	bool _tilemode;
	UICursorSwitcher* CursorSwitcher;
	UIHelp* _help;
	//  UIDoodadSpawner* _test;


public:
	// position of the tools window(s)
	int tool_settings_x;
	int tool_settings_y;
	UISlider* paintPressureSlider;
  UISlider* paintHardnessSlider;
	UIGradient *G1;

	// Editor paramter
	int ground_edit_mode;
	int selection_view_mode;

	MapView* theMapview;
	// UI elements
	UIFrame* TexturePalette;
	UIModel* ModelBrowser;
	UIFrame* SelectedTexture;
	UIMinimapWindow* minimapWindow;
	UIToolbar* guiToolbar;
	UIStatusBar* guiStatusbar;
	UIDetailInfos* guidetailInfos;
	UIAppInfo* guiappInfo;
	UIZoneIDBrowser* ZoneIDBrowser;
	UITexturePicker* TexturePicker;
	UITextureSwitcher* TextureSwitcher;
	UIWater* guiWater;
	UIWaterTypeBrowser* guiWaterTypeSelector;
	UICurrentTexture* guiCurrentTexture;
	UIWindow *setting_ground;
	UIWindow *setting_blur;
	UIWindow *settings_paint;
  UIObjectEditor *objectEditor;

	UIExitWarning *escWarning;
	UICapsWarning *capsWarning;
	UIWaterSaveWarning *waterSaveWarning;
	UIHelperModels *HelperModels;

	explicit UIMapViewGUI(MapView* setMapview);

	void setTilemode(bool enabled);
	virtual void render() const;

	void showCursorSwitcher();
	void hideCursorSwitcher();
	void toggleCursorSwitcher();
	void showHelp();
	void hideHelp();
	void toggleHelp();

	void showTest();
	void hideTest();
	void toggleTest();
};

#endif
