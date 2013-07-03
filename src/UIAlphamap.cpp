#include "UIAlphamap.h"

#include "Video.h"
#include "World.h"
#include "MapIndex.h"
#include "MapTile.h"
#include "MapChunk.h"
#include "TextureSet.h"

UIAlphamap::UIAlphamap(float xPos, float yPos)
  : UICloseWindow(xPos, yPos, 600, 600, "Alphamap", true)
{
}

void UIAlphamap::render() const
{
  UICloseWindow::render();

  int px = std::floor( gWorld->camera.x / TILESIZE );
  int pz = std::floor( gWorld->camera.z / TILESIZE );

  if(!gWorld->mapIndex->tileLoaded(pz, px))
    return;

  int size = 512;
  int unit = size/16;
  int border = (width() - size)/2;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);

  //glDisable(GL_DEPTH_TEST);

  for(size_t j = 0; j < 16; ++j)
  {
    for(size_t i = 0; i < 16; ++i)
    {
      for(size_t t = 0; t < gWorld->mapIndex->getTile(pz, px)->getChunk(i, j)->textureSet->num()-1; ++t)
        gWorld->mapIndex->getTile(pz, px)->getChunk(i, j)->textureSet->bindAlphamap(t,t);


      glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE2, 0.0f, 0.0f);
        glVertex2f(x() + border + i*unit, y() + border + j*unit);

        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE2, 1.0f, 0.0f);
        glVertex2f(x() + border + i*unit + unit, y() + border + j*unit);

        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE2, 1.0f, 1.0f);
        glVertex2f(x() + border + i*unit + unit, y() + border + j*unit + unit);

        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE2, 0.0f, 1.0f);
        glVertex2f(x() + border + i*unit, y() + border + j*unit + unit);
      glEnd();

      for(size_t t = 0; t < gWorld->mapIndex->getTile(pz, px)->getChunk(i, j)->textureSet->num()-1; ++t)
        OpenGL::Texture::disableTexture(t);

    }
  }
}
