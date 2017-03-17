// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Brush.h>

void Brush::init()
{
  radius = 15;
  hardness = 0.5f;
  iradius = hardness * radius;
  oradius = radius - iradius;
}

void Brush::setHardness(float H)
{
  hardness = H;
  iradius = hardness * radius;
  oradius = radius - iradius;
}
void Brush::setRadius(float R)
{
  radius = R;
  iradius = hardness * radius;
  oradius = radius - iradius;
}
float Brush::getHardness() const
{
  return hardness;
}
float Brush::getRadius() const
{
  return radius;
}
float Brush::getValue(float dist) const
{
  if (dist > radius)
    return 0.0f;
  if (dist < iradius)
    return 1.0f;
  return(1.0f - (dist - iradius) / oradius);
}
