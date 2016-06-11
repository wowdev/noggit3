#include "TextureSet.h"

#include "Brush.h"
#include "Environment.h"
#include "TextureManager.h" // TextureManager, Texture
#include "Video.h"
#include "MapHeaders.h"
#include "MapTile.h"
#include "Log.h"
#include "World.h"

#include <iostream>     // std::cout
#include <algorithm>    // std::min

TextureSet::TextureSet()
{}

TextureSet::~TextureSet()
{
	for (size_t i = 1; i < nTextures; ++i)
		delete alphamaps[i - 1];
}

void TextureSet::initTextures(MPQFile* f, MapTile* maintile, uint32_t size)
{
	// texture info
	nTextures = size / 16U;

	for (size_t i = 0; i<nTextures; ++i) {
		f->read(&tex[i], 4);
		f->read(&texFlags[i], 4);
		f->read(&MCALoffset[i], 4);
		f->read(&effectID[i], 4);

		if (texFlags[i] & FLAG_ANIMATE)
		{
			animated[i] = texFlags[i];
		}
		else
		{
			animated[i] = 0;
		}
		textures[i] = TextureManager::newTexture(maintile->mTextureFilenames[tex[i]]);
	}
}

void TextureSet::initAlphamaps(MPQFile* f, size_t nLayers, bool mBigAlpha, bool doNotFixAlpha)
{
	unsigned int MCALbase = f->getPos();

	for (size_t i = 0; i < 3; ++i)
	{
		alphamaps[i] = NULL;
	}

	for (unsigned int layer = 0; layer < nLayers; ++layer)
	{
		if (texFlags[layer] & 0x100)
		{
			f->seek(MCALbase + MCALoffset[layer]);
			alphamaps[layer - 1] = new Alphamap(f, texFlags[layer], mBigAlpha, doNotFixAlpha);
		}
	}
}

int TextureSet::addTexture(OpenGL::Texture* texture)
{
	int texLevel = -1;

	if (nTextures < 4U)
	{
		texLevel = nTextures;
		nTextures++;

		textures[texLevel] = texture;
		animated[texLevel] = 0;
		texFlags[texLevel] = 0;
		effectID[texLevel] = 0;

		if (texLevel)
		{
			if (alphamaps[texLevel - 1])
			{
				LogError << "Alpha Map has invalid texture binding" << std::endl;
				nTextures--;
				return -1;
			}
			alphamaps[texLevel - 1] = new Alphamap();
		}
	}

	return texLevel;
}

void TextureSet::switchTexture(OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture)
{
	int texLevel = -1;
	for (size_t i = 0; i < nTextures; ++i)
	{
		if (textures[i] == oldTexture)
			texLevel = i;
		// prevent texture duplication
		if (textures[i] == newTexture) 
			return;
	}		

	if (texLevel != -1)
	{
		textures[texLevel] = newTexture;
	}
}

void TextureSet::eraseTextures()
{
	for (size_t i = 0; i < nTextures; ++i)
	{
		TextureManager::delbyname(textures[i]->filename());
		tex[i] = 0;

		if (i < 1) continue;

		delete alphamaps[i - 1];
		alphamaps[i - 1] = NULL;
	}

	nTextures = 0U;
}

const std::string& TextureSet::filename(size_t id)
{
	return textures[id]->filename();
}

void TextureSet::bindAlphamap(size_t id, size_t activeTexture)
{
	OpenGL::Texture::enableTexture(activeTexture);

	alphamaps[id]->bind();
}

void TextureSet::bindTexture(size_t id, size_t activeTexture)
{
	OpenGL::Texture::enableTexture(activeTexture);

	textures[id]->bind();
}

void TextureSet::start2DAnim(int id)
{
	if (id < 0)
		return;

	if (animated[id])
	{
		OpenGL::Texture::setActiveTexture(0);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();

		// note: this is ad hoc and probably completely wrong
		const int spd = (animated[id] & 0x08) | ((animated[id] & 0x10) >> 2) | ((animated[id] & 0x20) >> 4) | ((animated[id] & 0x40) >> 6);
		const int dir = animated[id] & 0x07;
		const float texanimxtab[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
		const float texanimytab[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
		const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];

		const float f = (static_cast<int>(gWorld->animtime * (spd / 15.0f)) % 1600) / 1600.0f;
		glTranslatef(f*fdx, f*fdy, 0);
	}
}

void TextureSet::stop2DAnim(int id)
{
	if (id < 0)
		return;

	if (animated[id])
	{
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		OpenGL::Texture::setActiveTexture(1);
	}
}

//! \todo do they really differ? investigate
void TextureSet::startAnim(int id)
{
	if (id < 0)
		return;

	if (animated[id])
	{
		OpenGL::Texture::setActiveTexture(0);
		glMatrixMode(GL_TEXTURE);
		glPushMatrix();

		// note: this is ad hoc and probably completely wrong
		const int spd = (animated[id] & 0x08) | ((animated[id] & 0x10) >> 2) | ((animated[id] & 0x20) >> 4) | ((animated[id] & 0x40) >> 6);
		const int dir = animated[id] & 0x07;
		const float texanimxtab[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
		const float texanimytab[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
		const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];
		const int animspd = (const int)(200 * detail_size);
		float f = ((static_cast<int>(gWorld->animtime*(spd / 15.0f))) % animspd) / static_cast<float>(animspd);
		glTranslatef(f*fdx, f*fdy, 0);
	}
}

void TextureSet::stopAnim(int id)
{
	if (id < 0)
		return;

	if (animated[id])
	{
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		OpenGL::Texture::setActiveTexture(1);
	}
}

bool TextureSet::paintTexture(float xbase, float zbase, float x, float z, Brush* brush, float strength, float pressure, OpenGL::Texture* texture)
{

	if (Environment::getInstance()->paintMode == true)
	{
		float zPos, xPos, change, xdiff, zdiff, dist, radius;

		//xbase, zbase mapchunk pos
		//x, y mouse pos

		int texLevel = -1;

		radius = brush->getRadius();

		xdiff = xbase - x + CHUNKSIZE / 2;
		zdiff = zbase - z + CHUNKSIZE / 2;
		dist = sqrt(xdiff*xdiff + zdiff*zdiff);

		if (dist > (radius + MAPCHUNK_RADIUS))
			return false;

		//First Lets find out do we have the texture already
		for (size_t i = 0; i<nTextures; ++i)
			if (textures[i] == texture)
				texLevel = i;


		if ((texLevel == -1) && (nTextures == 4))
		{
			// Implement here auto texture slot freeing :)
			LogDebug << "paintTexture: No free texture slot" << std::endl;
			return false;
		}

		//Only 1 layer and its that layer
		if ((texLevel != -1) && (nTextures == 1))
			return true;

		if (texLevel == -1)
		{
			texLevel = addTexture(texture);
			if (texLevel == 0)
				return true;
			if (texLevel == -1)
			{
				LogDebug << "paintTexture: Unable to add texture." << std::endl;
				return false;
			}
		}

		zPos = zbase;

		for (int j = 0; j < 64; j++)
		{
			xPos = xbase;
			for (int i = 0; i < 64; ++i)
			{
				xdiff = xPos - x + (TEXDETAILSIZE / 2.0f); // Use the center instead of
				zdiff = zPos - z + (TEXDETAILSIZE / 2.0f); // the top left corner
				dist = std::abs(std::sqrt(xdiff*xdiff + zdiff*zdiff));

				if (dist>radius)
				{
					xPos += TEXDETAILSIZE;
					continue;
				}				

        float tPressure = pressure*brush->getValue(dist);
        float alphas[3] = { 0.0f, 0.0f, 0.0f };
        float visibility[4] = { 255.0f, 0.0f, 0.0f, 0.0f };
        
        for (size_t k = 0; k < nTextures - 1; k++)
        {
          float f = static_cast<float>(alphamaps[k]->getAlpha(i + j * 64));
          visibility[k+1] = f;
          alphas[k] = f;
          for (size_t n = 0; n <= k; n++)
            visibility[n] = (visibility[n] * ((255.0f - f)) / 255.0f);
        }

        // nothing to do
        if (visibility[texLevel] == strength)
        {
          xPos += TEXDETAILSIZE;
          continue;
        }

        // alpha delta
        float diffA = (strength - visibility[texLevel])* tPressure;

        // visibility = 255 => all other at 0
        if (visibility[texLevel] + diffA >= 255.0f)
        {
          for (size_t k = 0; k < nTextures; k++)
          {
            visibility[k] = (k == texLevel) ? 255.0f : 0.0f;
          }          
        }
        else
        {
          float other = 255.0f - visibility[texLevel];

          if (!texLevel && visibility[0] == 255.0f)
          {
            visibility[0] += diffA;
            visibility[1] -= diffA; // nTexture > 1 else it'd have returned true at the beginning
          }
          else
          {
            visibility[texLevel] += diffA;

            for (size_t k = 0; k < nTextures; k++)
            {
              if (k == texLevel || visibility[k] == 0)
                continue;

              visibility[k] = visibility[k] - (diffA * (visibility[k] / other));
            }
          }          
        }

        for (int k = nTextures - 2; k >= 0; k--)
        {
          alphas[k] = visibility[k+1];
          for (int n = nTextures - 2; n > k; n--)
          {
            // prevent 0 division
            alphas[k] = (alphas[k] / std::max((255.0f - alphas[n]), 0.001f)) * 255.0f;
          }
        }
          
        for (size_t k = 0; k < nTextures - 1; k++)
        {
          alphamaps[k]->setAlpha(i + j * 64, static_cast<unsigned char>(std::min(std::max(alphas[k], 0.0f), 255.0f)));
        }

				xPos += TEXDETAILSIZE;
			}
			zPos += TEXDETAILSIZE;
		}

		for (size_t j = 0; j < nTextures - 1; j++)
		{
			if (j > 2)
			{
				LogError << "WTF how did you get here??? Get a cookie." << std::endl;
				continue;
			}

			alphamaps[j]->loadTexture();
		}
	}

	return true;
}

const size_t TextureSet::num()
{
	return nTextures;
}

const unsigned int TextureSet::flag(size_t id)
{
	return texFlags[id];
}

const unsigned int TextureSet::effect(size_t id)
{
	return effectID[id];
}

void TextureSet::setAlpha(size_t id, size_t offset, unsigned char value)
{
	alphamaps[id]->setAlpha(offset, value);
}

void TextureSet::setAlpha(size_t id, unsigned char *amap)
{
	alphamaps[id]->setAlpha(amap);
}

const unsigned char TextureSet::getAlpha(size_t id, size_t offset)
{
	return alphamaps[id]->getAlpha(offset);
}

const unsigned char *TextureSet::getAlpha(size_t id)
{
	return alphamaps[id]->getAlpha();
}

OpenGL::Texture* TextureSet::texture(size_t id)
{
	return textures[id];
}
