#include <vector>

#include "Log.h"
#include "Menu.h"
#include "Sky.h"
#include "UIMinimapWindow.h"
#include "Video.h"
#include "World.h"
#include "UIText.h"
#include "Noggit.h"
#include "MapIndex.h"

#include <sstream>
#include <string>

UIMinimapWindow::UIMinimapWindow(Menu* menuLink)
	: UIWindow(0.0f, 0.0f, 0.0f, 0.0f)
	, borderwidth(5.0f)
	, tilesize(0.0f)
	, lookAt(0.0f)
	, mMenuLink(menuLink)
	, map(NULL)
{
	this->cursor_position = new UIText(10, height() - 20.0f, "Maptile: ", app.getArial14(), eJustifyLeft);
	this->addChild(cursor_position);
	resize();
}

UIMinimapWindow::UIMinimapWindow(World* setMap)
	: UIWindow(0.0f, 0.0f, 0.0f, 0.0f)
	, borderwidth(5.0f)
	, bottomspace(20.0f)
	, tilesize(0.0f)
	, lookAt(0.0f)
	, mMenuLink(NULL)
	, map(setMap)
{
	this->cursor_position = new UIText(10, height() - 20.0f, "Maptile: ", app.getArial14(), eJustifyLeft);
	this->addChild(cursor_position);
	resize();
}



void UIMinimapWindow::mousemove(SDL_MouseMotionEvent *e)
{
	if (hidden()) return;
	if (!gWorld) return;

	int mx = (int)(e->x - ((video.xres() - this->width()) / 2));
	int my = (int)(e->y - ((video.yres() - this->height()) / 2));

	if (
		mx < borderwidth || mx > height() - borderwidth ||
		my < borderwidth || my > height() - borderwidth)
		return;

	int i = (int)(static_cast<int>(mx - borderwidth) / tilesize);
	int j = (int)(static_cast<int>(my - borderwidth) / tilesize);

	if (i < 64 && j < 64 && i > -1 && j > -1)
	{
		std::stringstream sstr;
		sstr << "Maptile: " << gWorld->basename << "_" << i << "_" << j << ".adt      ";
		this->cursor_position->setText(sstr.str());
	}
}

UIFrame* UIMinimapWindow::processLeftClick(float mx, float my)
{
	// no click outside the adt block
	if (!gWorld ||
		mx < borderwidth || mx > height() - borderwidth ||
		my < borderwidth || my > height() - borderwidth)
		return NULL;

	// is there a tile?
	int i = (int)(static_cast<int>(mx - borderwidth) / tilesize);
	int j = (int)(static_cast<int>(my - borderwidth) / tilesize);
	if (!gWorld->mapIndex->hasTile(j, i))
		return NULL;

	if (mMenuLink)
		mMenuLink->enterMapAt(Vec3D(((mx - borderwidth) / tilesize) * TILESIZE, 0.0f, ((my - borderwidth) / tilesize) * TILESIZE));
	else if (map)
		map->jumpToCords(Vec3D(((mx - borderwidth) / tilesize) * TILESIZE, 50.0f, ((my - borderwidth) / tilesize) * TILESIZE));

	return this;
}


void UIMinimapWindow::resize()
{
	tilesize = (video.yres() - 90.0f - borderwidth * 2.0f) / 64.0f;

	width(borderwidth * 2.0f + tilesize * 64.0f);
	height(width() + 20.0f);
	x(video.xres() / 2.0f - width() / 2.0f);
	y(video.yres() / 2.0f - height() / 2.0f);

	this->cursor_position->y(height() - 20.0f);
}

void UIMinimapWindow::changePlayerLookAt(float ah)
{
	lookAt = ah;
}

void UIMinimapWindow::render() const
{
	if (hidden())
	{
		return;
	}

	UIWindow::render();

	glPushMatrix();
	glTranslatef(x() + borderwidth, y() + borderwidth, 0.0f);

	if (gWorld->minimap)
	{
		OpenGL::Texture::enableTexture();
		glBindTexture(GL_TEXTURE_2D, gWorld->minimap);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i((GLint)0.0f, (GLint)0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i((GLint)(tilesize * 64.0f), (GLint)0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i((GLint)(tilesize * 64.0f), (GLint)(tilesize * 64.0f));
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i((GLint)0.0f, (GLint)(tilesize * 64.0f));
		glEnd();

		OpenGL::Texture::disableTexture();
	}

	// draw the ADTs that are existing in the WDT with
	// a transparent 11x11 box. 12x12 is the full size
	// so we get a small border. Draw all not existing adts with
	// a lighter box to have all 64x64 possible adts on screen.
	// Later we can create adts over this view ore move them.
	for (int j = 0; j < 64; j++)
	{
		for (int i = 0; i < 64; ++i)
		{
			if (gWorld->mapIndex->hasTile(j, i))
			{
				if (gWorld->mapIndex->isTileExternal(i, j))
					glColor4f(1.0f, 0.7f, 0.5f, 0.6f);
				else if (gWorld->mapIndex->tileLoaded(j, i))
					glColor4f(0.0f, 1.0f, 1.0f, 0.4f);
				else
					glColor4f(0.8f, 0.8f, 0.8f, 0.4f);
			}
			else
				glColor4f(1.0f, 1.0f, 1.0f, 0.05f);

			glBegin(GL_QUADS);
			glVertex2i((GLint)(i * tilesize), (GLint)(j * tilesize));
			glVertex2i((GLint)(((i + 1) * tilesize) - 1), (GLint)(j * tilesize));
			glVertex2i((GLint)(((i + 1) * tilesize) - 1), (GLint)(((j + 1) * tilesize) - 1));
			glVertex2i((GLint)(i * tilesize), (GLint)(((j + 1) * tilesize) - 1));
			glEnd();

			if (map)
			{
				if (map->mapIndex->getChanged(j, i) > 0)
				{
					if (map->mapIndex->getChanged(j, i) == 1)
						glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
					else
						glColor4f(0.7f, 0.7f, 0.7f, 0.6f);

					glBegin(GL_LINES);
					glVertex2i((GLint)(i * tilesize), (GLint)(j * tilesize));
					glVertex2i((GLint)(((i + 1) * tilesize)), (GLint)(j * tilesize));
					glVertex2i((GLint)(((i + 1) * tilesize)), (GLint)(j * tilesize));
					glVertex2i((GLint)(((i + 1) * tilesize)), (GLint)(((j + 1) * tilesize) - 1));
					glVertex2i((GLint)(((i + 1) * tilesize)), (GLint)(((j + 1) * tilesize) - 1));
					glVertex2i((GLint)(i * tilesize), (GLint)(((j + 1) * tilesize) - 1));
					glVertex2i((GLint)(i * tilesize), (GLint)(((j + 1) * tilesize) - 1));
					glVertex2i((GLint)(i * tilesize), (GLint)(j * tilesize));
					glEnd();
				}
			}
		}

	}

	// draw the arrow if shown inside a map
	//! \todo Change it from a simple line to an arrow
	if (map)
	{
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		const float fx(map->camera.x / TILESIZE * tilesize);
		const float fz(map->camera.z / TILESIZE * tilesize);
		glVertex2f(fx, fz);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex2f(fx + 10.0f * cosf(lookAt / 180.0f * (float)PI), fz + 10.0f * sinf(lookAt / 180.0f * (float)PI));
		glEnd();

		int skycount = map->skies->skies.size();


		for (int j = 0; j < skycount; j++)
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			float x_ = map->skies->skies[j].pos.x / TILESIZE * tilesize;
			float z_ = map->skies->skies[j].pos.z / TILESIZE * tilesize;
			glBegin(GL_QUADS);
			glVertex2i((GLint)x_, (GLint)z_);
			glVertex2i((GLint)(x_ + 3), (GLint)z_);
			glVertex2i((GLint)(x_ + 3), (GLint)(z_ + 3));
			glVertex2i((GLint)x_, (GLint)(z_ + 3));
			glEnd();
		}
	}

	glPopMatrix();
}
