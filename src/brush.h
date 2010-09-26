#ifndef __BRUSH_H
#define __BRUSH_H

#include "video.h" // GLuint

class brush
{
private:
	float hardness;
	float iradius;
	float oradius;
	float radius;
	GLuint texID;
	char tex[256*256];
	bool update;

public:
	void GenerateTexture();
	void setHardness( float H );
	void setRadius( float R );
	float getHardness();
	float getRadius();
	float getValue( float dist );
	GLuint getTexture();
	bool needUpdate();
	void init();
};

#endif
