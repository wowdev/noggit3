// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Alphamap.h>

#include <cstring>

#include <noggit/Video.h>
#include <noggit/World.h>
#include <noggit/map_index.hpp>
#include <noggit/MapTile.h>
#include <noggit/MapChunk.h>
#include <noggit/texture_set.hpp>

UIAlphamap::UIAlphamap(float xPos, float yPos)
	: UICloseWindow(xPos, yPos, 600, 600, "Alphamap", true)
{

}

void UIAlphamap::render() const
{
	UICloseWindow::render();

  tile_index tile(gWorld->camera);
	float colorf[3];

	if (!gWorld->mapIndex->tileLoaded(tile))
		return;

	gl.enable(GL_BLEND);
	gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	for (size_t j = 0; j < 16; ++j)
	{
		for (size_t i = 0; i < 16; ++i)
		{
      TextureSet* tex = gWorld->mapIndex->getTile(tile)->getChunk(i, j)->textureSet;
			for (size_t t = 0; t < tex->num() - 1; ++t)
			{
				memset(colorf, 0, 3 * sizeof(float));
				colorf[t] = 1.0f;

				gl.color3fv(colorf);

				tex->bindAlphamap(t, 0);
				drawQuad(i, j);
			}
		}
	}

	opengl::texture::disable_texture (0);
}

void UIAlphamap::drawQuad(size_t i, size_t j) const
{
	int size = 512;
	int unit = size / 16;
	int border = (int)((width() - size) / 2);

	gl.begin(GL_QUADS);
	gl.texCoord2f(0.0f, 0.0f);
	gl.vertex2f(x() + border + i*unit, y() + border + j*unit);

	gl.texCoord2f(1.0f, 0.0f);
	gl.vertex2f(x() + border + i*unit + unit, y() + border + j*unit);

	gl.texCoord2f(1.0f, 1.0f);
	gl.vertex2f(x() + border + i*unit + unit, y() + border + j*unit + unit);

	gl.texCoord2f(0.0f, 1.0f);
	gl.vertex2f(x() + border + i*unit, y() + border + j*unit + unit);
	gl.end();
}
