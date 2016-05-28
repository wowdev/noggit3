// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Brush.h>

#include <cmath>

#include <opengl/context.h>
#include <opengl/texture.h>

brush::brush(float r, float h)
  : radius (r)
  , _texture (new opengl::texture)
{
  setHardness (h);
}

void brush::GenerateTexture()
{
  float x, y, dist;
  float change = 2.0f / 256.0f;

  y = -1;
  for(int j = 0; j < 256; j++)
  {
    x = -1;
    for(int i = 0; i < 256; ++i)
    {
      dist = std::sqrt(x * x + y * y);
      if(dist > 1)
        tex[j * 256 + i] = 0;
      else if( dist < hardness )
        tex[j * 256 + i] = (unsigned char)255;
      else
        tex[j * 256 + i] = (unsigned char)(255.0f * (1 - (dist - hardness) / (1 - hardness)) + 0.5f);

      x += change;
    }

    y += change;
  }

  _texture->bind();
  gl.texImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, 256, 256, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tex);
  gl.texParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl.texParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  gl.texParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void brush::setHardness(float H)
{
  hardness = H;
  iradius = hardness * radius;
  oradius = radius - iradius;

  GenerateTexture();
}

void brush::setRadius(float R)
{
  radius = R;
  iradius = hardness * radius;
  oradius = radius - iradius;
}

float brush::getHardness() const
{
  return hardness;
}

float brush::getRadius() const
{
  return radius;
}

float brush::getValue(float dist) const
{
  if(dist > radius)
    return 0.0f;
  if(dist < iradius)
    return 1.0f;

  return (1.0f - (dist - iradius) / oradius);
}

opengl::texture* brush::getTexture()
{
  return _texture;
}
