// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

namespace OpenGL
{
	class Texture;
}

class Brush
{
private:
	float hardness;
	float iradius;
	float oradius;
	float radius;
	OpenGL::Texture* _texture;
	char tex[256 * 256];
	bool update;

public:
	void GenerateTexture();
	void setHardness(float H);
	void setRadius(float R);
	float getHardness();
	float getRadius();
	float getValue(float dist);
	OpenGL::Texture* getTexture();
	bool needUpdate();
	void init();
};
