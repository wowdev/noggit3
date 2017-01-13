#ifndef MISC_H
#define MISC_H

#include <string>
#include <algorithm>
#include <cassert>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "Log.h"
#include "math/vector_3d.hpp"

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
}

//! \todo collect all lose functions/classes/structs for now, sort them later

class sExtendableArray
{
public:
	int mSize;
	char* mData;

	bool Allocate(int pSize)
	{
		mSize = pSize;
		mData = static_cast<char*>(realloc(mData, mSize));
		memset(mData, 0, mSize);
		return(mData != NULL);
	}

	bool Extend(int pAddition)
	{
		mSize = mSize + pAddition;
		mData = static_cast<char*>(realloc(mData, mSize));
		if (pAddition > 0)
			memset(mData + mSize - pAddition, 0, pAddition);
		return(mData != NULL);
	}

	bool Insert(int pPosition, int pAddition)
	{
		const int lPostSize = mSize - pPosition;

		char *lPost = static_cast<char*>(malloc(lPostSize));
		memcpy(lPost, mData + pPosition, lPostSize);

		if (!Extend(pAddition))
			return false;

		memcpy(mData + pPosition + pAddition, lPost, lPostSize);
		memset(mData + pPosition, 0, pAddition);

		free(lPost);

		return true;
	}

	bool Insert(int pPosition, int pAddition, const char * pAdditionalData)
	{
		const int lPostSize = mSize - pPosition;

		char *lPost = static_cast<char*>(malloc(lPostSize));
		memcpy(lPost, mData + pPosition, lPostSize);

		if (!Extend(pAddition))
			return false;

		memcpy(mData + pPosition + pAddition, lPost, lPostSize);
		memcpy(mData + pPosition, pAdditionalData, pAddition);

		free(lPost);

		return true;
	}

	template<typename To>
	To * GetPointer()
	{
		return(reinterpret_cast<To*>(mData));
	}

	template<typename To>
	To * GetPointer(unsigned int pPosition)
	{
		return(reinterpret_cast<To*>(mData + pPosition));
	}

	sExtendableArray()
	{
		mSize = 0;
		mData = NULL;
	}

	sExtendableArray(int pSize, const char *pData)
	{
		if (Allocate(pSize))
			memcpy(mData, pData, pSize);
		else
			LogError << "Allocating " << pSize << " bytes failed. This may crash soon." << std::endl;
	}

	void Destroy()
	{
		free(mData);
	}
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

void SetChunkHeader(sExtendableArray pArray, int pPosition, int pMagix, int pSize = 0);

bool pointInside(math::vector_3d point, math::vector_3d extents[2]);
void minmax(math::vector_3d* a, math::vector_3d* b);

bool checkInside(math::vector_3d extentA[2], math::vector_3d extentB[2]);
bool checkOriginInside(math::vector_3d extentA[2], math::vector_3d modelPos);

#endif
