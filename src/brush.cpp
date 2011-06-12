#include <cmath>

#include "brush.h"

void brush::init()
{
  radius = 15;
  hardness = 0.5f;
  iradius = hardness * radius;
  oradius = radius - iradius;
  glGenTextures( 1, &texID );
  GenerateTexture();
}

void brush::GenerateTexture()
{
  float x, y, dist;
  float change = 2.0f / 256.0f;

  update = false;

  y = -1;
  for( int j = 0; j < 256; j++ )
  {
    x = -1;
    for( int i = 0; i < 256; ++i )
    {
      dist = sqrt( x * x + y * y );
      if( dist > 1 )
                tex[j*256+i] = 0;
      else if( dist < hardness )
        tex[j*256+i] = (unsigned char) 255;
      else
        tex[j*256+i] = (unsigned char)( 255.0f * ( 1 - ( dist - hardness ) / ( 1 - hardness ) ) + 0.5f );

      x += change;
    }
    y += change;
  }
  glBindTexture( GL_TEXTURE_2D, texID );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_ALPHA, 256, 256, 0, GL_ALPHA, GL_UNSIGNED_BYTE, tex );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void brush::setHardness( float H )
{
  hardness = H;
  iradius = hardness * radius;
  oradius = radius - iradius;
  update = true;
}
void brush::setRadius( float R )
{
  radius = R;
  iradius = hardness * radius;
  oradius = radius - iradius;
}
float brush::getHardness()
{
  return hardness;
}
float brush::getRadius()
{
  return radius;
}
float brush::getValue( float dist )
{
  if( dist > radius )
    return 0.0f;
  if( dist < iradius )
    return 1.0f;  
  return( 1.0f - ( dist - iradius ) / oradius );
}
GLuint brush::getTexture()
{ 
  return texID; 
}
bool brush::needUpdate()
{ 
  return update; 
}
