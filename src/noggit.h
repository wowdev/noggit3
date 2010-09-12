#ifndef NOGGIT_H
#define NOGGIT_H

#include <vector>
#include <string>
#include "appstate.h"
#include "freetype.h"

//! \todo this really needs to be refactored into a singleton class

#define APP_TITLE "Noggit3 Studio"
#define APP_SUBTITLE "a wow map editor"
#define APP_VERSION "Rev: 120" 
#define APP_DATE __DATE__ ", " __TIME__

extern std::vector<AppState*> gStates;
extern bool gPop;

typedef struct {
	int mapid;
	std::string mapname;
	float x,y,z;
} GotoInfo;
extern GotoInfo gGoto;

extern freetype::font_data arialn13,arial12,arial14,arial16,arial24,arial32,morpheus40,skurri32,fritz16;

extern float gFPS;

float frand();
float randfloat(float lower, float upper);
int randint(int lower, int upper);

class AreaDB;
extern AreaDB gAreaDB;

int TimerStop();
void TimerStart();

class AsyncLoader;
extern AsyncLoader* gAsyncLoader;

#endif
