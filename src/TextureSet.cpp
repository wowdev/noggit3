#include "TextureSet.h"

#include "Brush.h"
#include "Environment.h"
#include "TextureManager.h" // TextureManager, Texture
#include "Video.h"
#include "MapHeaders.h"
#include "MapTile.h"
#include "Log.h"
#include "World.h"

TextureSet::TextureSet()
{}

TextureSet::~TextureSet()
{
  for(size_t i = 1; i < nTextures; ++i)
    delete alphamaps[i-1];
}

void TextureSet::initTextures(MPQFile* f, MapTile* maintile, uint32_t size)
{
  // texture info
  nTextures = size / 16U;

  for (size_t i=0; i<nTextures; ++i) {
    f->read(&tex[i],4);
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
    textures[i] = TextureManager::newTexture( maintile->mTextureFilenames[tex[i]] );
  }
}

void TextureSet::initAlphamaps(MPQFile* f, size_t nLayers, bool mBigAlpha)
{
  unsigned int MCALbase = f->getPos();

  for(size_t i = 0; i < 3; ++i)
  {
    alphamaps[i] = NULL;
  }

  for(unsigned int layer = 0; layer < nLayers; ++layer)
  {
    if(texFlags[layer] & 0x100)
    {
      f->seek(MCALbase + MCALoffset[layer]);
      alphamaps[layer-1] = new Alphamap(f, texFlags[layer], mBigAlpha);
    }
  }
}

int TextureSet::addTexture(OpenGL::Texture* texture)
{
  int texLevel = -1;

  if(nTextures < 4U)
  {
    texLevel = nTextures;
    nTextures++;

    textures[texLevel] = texture;
    animated[texLevel] = 0;
    texFlags[texLevel] = 0;
    effectID[texLevel] = 0;

    if(texLevel)
    {
      if(alphamaps[texLevel-1])
      {
        LogError << "Alpha Map has invalid texture binding" << std::endl;
        nTextures--;
        return -1;
      }
      alphamaps[texLevel-1] = new Alphamap();
    }
  }

  return texLevel;
}

void TextureSet::switchTexture(OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture)
{
  int texLevel = -1;
  for(size_t i = 0; i<nTextures; ++i)
    if(textures[i] == oldTexture)
      texLevel = i;

  if(texLevel != -1)
  {
    textures[texLevel] = newTexture;
  }
}

void TextureSet::eraseTextures()
{
  for(size_t i = 0; i < nTextures; ++i)
  {
    TextureManager::delbyname(textures[i]->filename());
    tex[i] = 0;

    if(i < 1) continue;

    delete alphamaps[i-1];
    alphamaps[i-1] = NULL;
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
  if(id < 0)
    return;

  if (animated[id])
  {
    OpenGL::Texture::setActiveTexture(0);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();

    // note: this is ad hoc and probably completely wrong
    const int spd = (animated[id] & 0x08) | ((animated[id] & 0x10) >> 2) | ((animated[id] & 0x20) >> 4) | ((animated[id] & 0x40) >> 6);
    const int dir = animated[id] & 0x07;
    const float texanimxtab[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const float texanimytab[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];

    const float f = ( static_cast<int>( gWorld->animtime * (spd/15.0f) ) % 1600) / 1600.0f;
    glTranslatef(f*fdx, f*fdy, 0);
  }
}

void TextureSet::stop2DAnim(int id)
{
  if(id < 0)
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
  if(id < 0)
    return;

  if (animated[id])
  {
    OpenGL::Texture::setActiveTexture(0);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();

    // note: this is ad hoc and probably completely wrong
    const int spd = (animated[id] & 0x08) | ((animated[id] & 0x10) >> 2) | ((animated[id] & 0x20) >> 4) | ((animated[id] & 0x40) >> 6);
    const int dir = animated[id] & 0x07;
    const float texanimxtab[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const float texanimytab[8] = {1, 1, 0, -1, -1, -1, 0, 1};
    const float fdx = -texanimxtab[dir], fdy = texanimytab[dir];
    const int animspd = 200 * detail_size;
    float f = ( (static_cast<int>(gWorld->animtime*(spd/15.0f))) % animspd) / static_cast<float>(animspd);
    glTranslatef(f*fdx,f*fdy,0);
  }
}

void TextureSet::stopAnim(int id)
{
  if(id < 0)
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
  /* The correct way to do everything
  Visibility = (1-Alpha above)*Alpha

  Objective is Visibility = level

  if (not bottom texture)
    New Alpha = Pressure*Level+(1-Pressure)*Alpha;
    New Alpha Above = (1-Pressure)*Alpha Above;
  else Bottom Texture
    New Alpha Above = Pressure*(1-Level)+(1-Pressure)*Alpha Above

  For bottom texture with multiple above textures

  For 2 textures above
  x,y = current alphas
  u,v = target alphas
  v=sqrt((1-level)*y/x)
  u=(1-level)/v

  For 3 textures above
  x,y,z = current alphas
  u,v,w = target alphas
  L=(1-Level)
  u=pow(L*x*x/(y*y),1.0f/3.0f)
  w=sqrt(L*z/(u*y))
  */

  if(Environment::getInstance()->paintMode == true)
  {
    float zPos,xPos,change,xdiff,zdiff,dist, radius;

    //xbase, zbase mapchunk pos
    //x, y mouse pos

    int texLevel=-1;

    radius=brush->getRadius();

    xdiff= xbase - x + CHUNKSIZE/2;
    zdiff= zbase - z + CHUNKSIZE/2;
    dist= sqrt( xdiff*xdiff + zdiff*zdiff );

    if( dist > (radius+MAPCHUNK_DIAMETER) )
      return false;

    //First Lets find out do we have the texture already
    for(size_t i=0;i<nTextures;++i)
      if(textures[i]==texture)
        texLevel=i;


    if( (texLevel==-1) && (nTextures==4) )
    {
      // Implement here auto texture slot freeing :)
      LogDebug << "paintTexture: No free texture slot" << std::endl;
      return false;
    }

    //Only 1 layer and its that layer
    if( (texLevel!=-1) && (nTextures==1) )
      return true;


    change=CHUNKSIZE/62.0f;
    zPos=zbase;

    float target,tarAbove, tPressure;
    //int texAbove=nTextures-texLevel-1;


    for(int j=0; j < 63 ; j++)
    {
      xPos=xbase;
      for(int i=0; i < 63; ++i)
      {
        xdiff=xPos-x;
        zdiff=zPos-z;
        dist=abs(sqrt( xdiff*xdiff + zdiff*zdiff ));

        if(dist>radius)
        {
          xPos+=change;
          continue;
        }

        if(texLevel==-1)
        {
          texLevel=addTexture(texture);
          if(texLevel==0)
            return true;
          if(texLevel==-1)
          {
            LogDebug << "paintTexture: Unable to add texture." << std::endl;
            return false;
          }
        }

        target=strength;
        tarAbove=1-target;

        tPressure=pressure*brush->getValue(dist);

        if(texLevel>0)
        {
          alphamaps[texLevel-1]->setAlpha(i+j*64, static_cast<unsigned char>(std::max( std::min( (1.0f-tPressure)*( static_cast<float>(alphamaps[texLevel-1]->getAlpha(i+j*64)) ) + tPressure*target + 0.5f ,255.0f) , 0.0f)));
        }

        for(size_t k = texLevel; k < nTextures-1; k++)
        {
          alphamaps[k]->setAlpha(i+j*64, static_cast<unsigned char>(std::max( std::min( (1.0f-tPressure)*( static_cast<float>(alphamaps[k]->getAlpha(i+j*64)) ) + tPressure*tarAbove + 0.5f ,255.0f) , 0.0f)));
        }
        xPos+=change;
      }
      zPos+=change;
    }

    if( texLevel == -1 )
    {
      LogDebug << "Somehow no texture got painted." << std::endl;
      return false;
    }

    for( size_t j = texLevel; j < nTextures - 1; j++ )
    {
      if( j > 2 )
      {
        LogError << "WTF how did you get here??? Get a cookie." << std::endl;
        continue;
      }

      alphamaps[j]->loadTexture();
    }

    if( texLevel )
    {
      alphamaps[texLevel-1]->loadTexture();
    }

  }
  /*
  else
  {
    // new stuff from bernd.
    // need to get rework. Add old code with switch that the guys out there can use paint.
      const float radius = brush->getRadius();

      // Are we really painting on this chunk?
      const float xdiff = xbase + CHUNKSIZE / 2 - x;
      const float zdiff = zbase + CHUNKSIZE / 2 - z;

      if( ( xdiff * xdiff + zdiff * zdiff ) > ( MAPCHUNK_DIAMETER / 2 + radius ) * ( MAPCHUNK_DIAMETER / 2 + radius ) )
      return false;


      // Search for empty layer.
      int texLevel = -1;

      for( size_t i = 0; i < nTextures; ++i )
      {
        if( _textures[i] == texture )
        {
          texLevel = i;
        }
       }

      if( texLevel == -1 )
      {

        if( nTextures == 4 )
        {
          for( size_t layer = 0; layer < nTextures; ++layer )
          {
            unsigned char map[64*64];
            if( layer )
              memcpy( map, amap[layer-1], 64*64 );
            else
              memset( map, 255, 64*64 );

            for( size_t layerAbove = layer + 1; layerAbove < nTextures; ++layerAbove )
            {
              unsigned char* above = amap[layerAbove-1];
              for( size_t i = 0; i < 64 * 64; ++i )
              {
                map[i] = std::max( 0, map[i] - above[i] );
              }
            }

            size_t sum = 0;
            for( size_t i = 0; i < 64 * 64; ++i )
            {
              sum += map[i];
            }

            if( !sum )
            {
              for( size_t i = layer; i < nTextures - 1; ++i )
              {
                _textures[i] = _textures[i+1];
                animated[i] = animated[i+1];
                texFlags[i] = texFlags[i+1];
                effectID[i] = effectID[i+1];
                if( i )
                  memcpy( amap[i-1], amap[i], 64*64 );
              }

              for( size_t j = layer; j < nTextures; j++ )
                  {
                    glBindTexture( GL_TEXTURE_2D, alphamaps[j - 1] );
                    glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1] );
                  }
                  nTextures--;
            }
          }
        }

        if( nTextures == 4 )
        return false;

          texLevel = addTexture( texture );

      }
      else
      {
        if( nTextures == 1 )
          return true;
      }
      LogDebug << "TexLevel: " << texLevel << " -  NTextures: " << nTextures << "\n";
      // We now have a texture at texLevel > 0.
      static const float change = CHUNKSIZE / 62.0f; //! \todo 64? 63? 62? Wtf?

      if( texLevel == 0 )
        return true;

      for( size_t j = 0; j < 64; ++j )
      {
        for( size_t i = 0; i < 64; ++i )
        {
          const float xdiff_ = xbase + change * i - x;
          const float zdiff_ = zbase + change * j - z;
          const float dist = sqrtf( xdiff_ * xdiff_ + zdiff_ * zdiff_ );

          if( dist <= radius )
          {
              amap[texLevel - 1][i + j * 64] = (unsigned char)( std::max( std::min( amap[texLevel - 1][i + j * 64] + pressure * strength * brush->getValue( dist ) + 0.5f, 255.0f ), 0.0f ) );
          }
        }
      }


      // Redraw changed layers.

      for( size_t j = texLevel; j < nTextures; j++ )
      {
        glBindTexture( GL_TEXTURE_2D, alphamaps[j - 1] );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE, amap[j - 1] );
      }

  }
  */

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
