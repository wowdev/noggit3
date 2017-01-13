// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <vector>

#include "Log.h"
#include "Menu.h"
#include "Sky.h"
#include "UIMinimapWindow.h"
#include "Video.h"
#include "World.h"
#include "UIText.h"
#include "application.h"
#include "map_index.hpp"
#include <opengl/scoped.hpp>

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

  math::vector_3d pos((mx - borderwidth), 0.0f, (my - borderwidth));
  pos *= TILESIZE / tilesize; // minimap pos => real pos
	
  // is there a tile?
	if (!gWorld->mapIndex->hasTile(tile_index(pos)))
		return NULL;

  if (mMenuLink)
  {
    mMenuLink->enterMapAt(pos);
  }		
  else if (map)
  {
    gWorld->GetVertex(pos.x, pos.z, &pos);
    pos.y += 50;
    map->jumpToCords(pos);
  }
		

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

void UIMinimapWindow::changePlayerLookAt(math::degrees ah)
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

  opengl::scoped::matrix_pusher const matrix;
	gl.translatef(x() + borderwidth, y() + borderwidth, 0.0f);

	if (gWorld->minimap)
	{
		opengl::texture::enable_texture();
		gl.bindTexture(GL_TEXTURE_2D, gWorld->minimap);

		gl.begin(GL_QUADS);
		gl.texCoord2f(0.0f, 0.0f);
		gl.vertex2f(0.0f, 0.0f);
		gl.texCoord2f(1.0f, 0.0f);
		gl.vertex2f(tilesize * 64.0f, 0.0f);
		gl.texCoord2f(1.0f, 1.0f);
		gl.vertex2f(tilesize * 64.0f, tilesize * 64.0f);
		gl.texCoord2f(0.0f, 1.0f);
		gl.vertex2f(0.0f, tilesize * 64.0f);
		gl.end();

		opengl::texture::disable_texture();
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
      tile_index tile(i, j);

      if (gWorld->mapIndex->hasTile(tile))
      {
        if (gWorld->mapIndex->isTileExternal(tile))
        {
          glColor4f(1.0f, 0.7f, 0.5f, 0.6f);
        }
        else if (gWorld->mapIndex->tileLoaded(tile))
        {
          gl.color4f(0.0f, 1.0f, 1.0f, 0.4f);
        }
        else
        {
          glColor4f(0.8f, 0.8f, 0.8f, 0.4f);
        }        
      }
      else
      {
        gl.color4f(1.0f, 1.0f, 1.0f, 0.05f);
      }
				

			gl.begin(GL_QUADS);
			gl.vertex2f(i * tilesize, j * tilesize);
			gl.vertex2f(((i + 1) * tilesize) - 1, j * tilesize);
			gl.vertex2f(((i + 1) * tilesize) - 1, ((j + 1) * tilesize) - 1);
			gl.vertex2f(i * tilesize, ((j + 1) * tilesize) - 1);
			gl.end();

			if (map && map->mapIndex->getChanged(tile) > 0)
			{
        if (map->mapIndex->getChanged(tile) == 1)
        {
          gl.color4f(1.0f, 1.0f, 1.0f, 0.6f);
        }
        else
        {
          gl.color4f(0.7f, 0.7f, 0.7f, 0.6f);
        }

				gl.begin(GL_LINES);
				gl.vertex2f(i * tilesize, j * tilesize);
				gl.vertex2f(((i + 1) * tilesize), j * tilesize);
				gl.vertex2f(((i + 1) * tilesize), j * tilesize);
				gl.vertex2f(((i + 1) * tilesize), ((j + 1) * tilesize) - 1);
				gl.vertex2f(((i + 1) * tilesize), ((j + 1) * tilesize) - 1);
				gl.vertex2f(i * tilesize, ((j + 1) * tilesize) - 1);
				gl.vertex2f(i * tilesize, ((j + 1) * tilesize) - 1);
				gl.vertex2f(i * tilesize, j * tilesize);
				gl.end();
			}
		}

	}

	// draw the arrow if shown inside a map
	//! \todo Change it from a simple line to an arrow
	if (map)
	{
		gl.begin(GL_LINES);
		gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);
		const float fx(map->camera.x / TILESIZE * tilesize);
		const float fz(map->camera.z / TILESIZE * tilesize);
		gl.vertex2f(fx, fz);
		gl.vertex2f(fx + 10.0f * math::cos(lookAt), fz + 10.0f * math::sin(lookAt));
		gl.end();

		int skycount = map->skies->skies.size();


		for (int j = 0; j < skycount; j++)
		{
			gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);
			float x_ = map->skies->skies[j].pos.x / TILESIZE * tilesize;
			float z_ = map->skies->skies[j].pos.z / TILESIZE * tilesize;
			gl.begin(GL_QUADS);
			gl.vertex2f(x_, z_);
			gl.vertex2f(x_ + 3, z_);
			gl.vertex2f(x_ + 3, z_ + 3);
			gl.vertex2f(x_, z_ + 3);
			gl.end();
		}
	}
}
