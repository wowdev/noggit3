#ifndef NOGGIT_H
#define NOGGIT_H

#define DEBUG__LOGGINGTOCONSOLE 1

#include <vector>
#include <string>
#include "appstate.h"
#include "freetype.h"

/// XXX this really needs to be refactored into a singleton class

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

#endif

/// Automated comments for needed commit to get the revision in this file ._. Talk to schlumpf for the script.
// revision by schlumpf 
// revision by Steff 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by schlumpf 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by schlumpf 
// revision by Steff 
// revision by schlumpf 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by Steff 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by Steff 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by schlumpf 
// revision by Steff 
