// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class Brush
{
private:
  float hardness;
  float iradius;
  float oradius;
  float radius;

public:
  void setHardness(float H);
  void setRadius(float R);
  float getHardness() const;
  float getRadius() const;
  float getValue(float dist) const;
  void init();
};
