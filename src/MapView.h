#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "AppState.h"

class UIFrame;
class World;
class nameEntry;

enum eViewMode
{
	eViewMode_Minimap,
	eViewMode_2D,
	eViewMode_3D
};

class MapView : public AppState, public HotKeyReceiver
{
private:
	float ah, av, moving, strafing, updown, mousedir, movespd, turn, lookat;
	bool key_w;
	bool look;
	bool _GUIDisplayingEnabled;

	void save();

	float lastBrushUpdate;

	void doSelection(bool selectTerrainOnly);

	int mViewMode;

	void displayViewMode_2D(float t, float dt);
	void displayViewMode_3D(float t, float dt);

	void displayGUIIfEnabled();

	void createGUI();

	float mTimespeed;

	void checkWaterSave();

public:
	MapView(float ah0 = -90.0f, float av0 = -30.0f);
	~MapView();

	void tick(float t, float dt);
	void display(float t, float dt);

	void keypressed(SDL_KeyboardEvent *e);
	void mousemove(SDL_MouseMotionEvent *e);
	void mouseclick(SDL_MouseButtonEvent *e);
	void resizewindow();

	void quit();
	void quitask();
	void inserObjectFromExtern(int model);
	void addModelFromTextSelection(int id);
  void selectModel(nameEntry entry);
};


#endif
