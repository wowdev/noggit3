#include "Misc.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace misc
{
	std::string explode(std::string original, std::string exploder) {
		std::string tmp;
		tmp = original;
		int num, loc;
		num = 1;
		while (tmp.find(exploder) != std::string::npos) {
			loc = tmp.find(exploder);
			tmp = tmp.substr(loc + exploder.length());
			num++;
		}
		std::string *result;
		result = new std::string[num];
		num = 0;
		tmp = original;
		while (tmp.find(exploder) != std::string::npos) {
			loc = tmp.find(exploder);
			result[num] = tmp.substr(0, loc);
			tmp = tmp.substr(loc + exploder.length());
			num++;
		}
		result[num] = tmp;
		return result[num];
	}

	void find_and_replace(std::string& source, const std::string& find, const std::string& replace)
	{
		size_t found = source.rfind(find);
		while (found != std::string::npos) //fixed unknown letters replace. Now it works correctly and replace all found symbold instead of only one at previous versions
		{
			source.replace(found, find.length(), replace);
			found = source.rfind(find);
		}
	}

  std::string floatToStr(float f, int precision)
  {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << f;
    return ss.str();
  }

	//dirty hack
	int FtoIround(float d)
	{
		return (int)(d<0 ? d - .5f : d + .5f);
	}

	char roundc(float a)
	{
		if (a < 0)
			a -= 0.5f;
		if (a > 0)
			a += 0.5f;
		if (a < -127)
			a = -127;
		else if (a > 127)
			a = 127;
		return static_cast<char>(a);
	}

	float frand()
	{
		return rand() / static_cast<float>(RAND_MAX);
	}

	float randfloat(float lower, float upper)
	{
		return lower + (upper - lower) * frand();
	}

	int randint(int lower, int upper)
	{
		return lower + static_cast<int>((upper + 1 - lower) * frand());
	}

	int getADTCord(float cord)
	{
		return (int)(cord / 533.33333f);
	}
}

void SetChunkHeader(sExtendableArray pArray, int pPosition, int pMagix, int pSize)
{
	sChunkHeader* Header = pArray.GetPointer<sChunkHeader>(pPosition);
	Header->mMagic = pMagix;
	Header->mSize = pSize;
}

bool pointInside(Vec3D point, Vec3D extents[2])
{
	minmax(&extents[0], &extents[1]);

	return point.x >= extents[0].x && point.z >= extents[0].z &&
		point.x <= extents[1].x && point.z <= extents[1].z;
}

void minmax(Vec3D* a, Vec3D* b)
{
	if (a->x > b->x)
	{
		float t = b->x;
		b->x = a->x;
		a->x = t;
	}
	if (a->y > b->y)
	{
		float t = b->y;
		b->y = a->y;
		a->y = t;
	}
	if (a->z > b->z)
	{
		float t = b->z;
		b->z = a->z;
		a->z = t;
	}
}

bool checkInside(Vec3D extentA[2], Vec3D extentB[2])
{
	minmax(&extentA[0], &extentA[1]);
	minmax(&extentB[0], &extentB[1]);

	return pointInside(extentA[0], extentB) ||
		pointInside(extentA[1], extentB) ||
		pointInside(extentB[0], extentA) ||
		pointInside(extentB[1], extentA);
}

bool checkOriginInside(Vec3D extentA[2], Vec3D modelPos)
{
	return pointInside(modelPos, extentA);
}
