// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <math/vector_3d.hpp>
#include <noggit/Log.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// namespace for static helper functions.

namespace misc
{
  void find_and_replace(std::string& source, const std::string& find, const std::string& replace);
  int FtoIround(float d);
  char roundc(float a);
  float frand();
  float randfloat(float lower, float upper);
  int randint(int lower, int upper);
  int getADTCord(float cord);
  std::string explode(std::string original, std::string exploder = ".");
  std::string floatToStr(float f, int precision = 2);
  float dist(float x1, float z1, float x2, float z2);
  float getShortestDist(float x, float z, float squareX, float squareZ, float unitSize);
  bool rectOverlap(math::vector_3d *r1, math::vector_3d *r2);
  // used for angled tools, get the height a point (pos) should be given an origin, angle and orientation
  float angledHeight(math::vector_3d const& origin, math::vector_3d const& pos, math::radians const& angle, math::radians const& orientation);
}

//! \todo collect all lose functions/classes/structs for now, sort them later

class sExtendableArray
{
public:
  std::vector<char> data;

	void Allocate(int pSize)
	{
    data.resize (pSize);
	}

	void Extend(int pAddition)
	{
    data.resize (data.size() + pAddition);
	}

  void Insert(int pPosition, int pAddition)
	{
    std::vector<char> tmp (pAddition);
    data.insert (data.begin() + pPosition, tmp.begin(), tmp.end());
  }

	void Insert(int pPosition, int pAddition, const char * pAdditionalData)
	{
    data.insert (data.begin() + pPosition, pAdditionalData, pAdditionalData + pAddition);
	}

	template<typename To>
	To * GetPointer(unsigned int pPosition = 0)
	{
		return(reinterpret_cast<To*>(data.data() + pPosition));
	}

  sExtendableArray() = default;
	sExtendableArray(int pSize, const char *pData)
    : data (pData, pData + pSize)
	{}
};

struct sChunkHeader
{
  int mMagic;
  int mSize;
};

struct filenameOffsetThing
{
  int nameID;
  int filenamePosition;
};

void SetChunkHeader(sExtendableArray& pArray, int pPosition, int pMagix, int pSize = 0);

bool pointInside(math::vector_3d point, math::vector_3d extents[2]);
void minmax(math::vector_3d* a, math::vector_3d* b);

bool checkInside(math::vector_3d extentA[2], math::vector_3d extentB[2]);
bool checkOriginInside(math::vector_3d extentA[2], math::vector_3d modelPos);
