// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/types.hpp>

#include <noggit/MPQ.h>
#include <noggit/Log.h>

class Alphamap
{
public:
	Alphamap();
	Alphamap(MPQFile* f, unsigned int & flags, bool mBigAlpha, bool doNotFixAlpha);
	~Alphamap();

	void loadTexture();

	void bind();
	bool isValid();

	void setAlpha(size_t offset, unsigned char value);
	void setAlpha(unsigned char *pAmap);

	const unsigned char getAlpha(size_t offset);
	const unsigned char *getAlpha();

private:
	void readCompressed(MPQFile *f);
	void readBigAlpha(MPQFile *f);
	void readNotCompressed(MPQFile *f, bool doNotFixAlpha);

	void createNew();

	void genTexture();

	unsigned char amap[64 * 64];
	GLuint map;
};
