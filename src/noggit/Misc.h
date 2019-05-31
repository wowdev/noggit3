// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/trig.hpp>
#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
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
  float frand();
  float randfloat(float lower, float upper);
  int randint(int lower, int upper);
  float dist(float x1, float z1, float x2, float z2);
  float dist(math::vector_3d const& p1, math::vector_3d const& p2);
  float getShortestDist(float x, float z, float squareX, float squareZ, float unitSize);
  float getShortestDist(math::vector_3d const& pos, math::vector_3d const& square_pos, float unitSize);
  bool rectOverlap(math::vector_3d const*, math::vector_3d const*);
  // used for angled tools, get the height a point (pos) should be given an origin, angle and orientation
  float angledHeight(math::vector_3d const& origin, math::vector_3d const& pos, math::radians const& angle, math::radians const& orientation);
  void extract_v3d_min_max(math::vector_3d const& point, math::vector_3d& min, math::vector_3d& max);
  std::vector<math::vector_3d> intersection_points(math::vector_3d const& vmin, math::vector_3d const& vmax);
  std::vector<math::vector_3d> box_points(math::vector_3d const& box_min, math::vector_3d const& box_max);
  math::vector_3d transform_model_box_coords(math::vector_3d const& pos);
  // normalize the filename used in adts since TC extractors don't accept /
  std::string normalize_adt_filename(std::string filename);

  inline int rounded_int_div(int value, int div)
  {
    return value / div + (value % div <= (div >> 1) ? 0 : 1);
  }
  inline int rounded_255_int_div(int value)
  {
    return value / 255 + (value % 255 <= 127 ? 0 : 1);
  }

  struct random_color : math::vector_4d
  {
    random_color()
      : math::vector_4d ( misc::randfloat(0.0f, 1.0f)
                        , misc::randfloat(0.0f, 1.0f)
                        , misc::randfloat(0.0f, 1.0f)
                        , 0.7f
                        )
    {}
  };

  template<typename Range>
    constexpr std::size_t max_element_index (Range const& range)
  {
    return std::distance (range.begin(), std::max_element (range.begin(), range.end()));
  }

  template<typename T, std::size_t Capacity>
    struct max_capacity_stack_vector
  {
    max_capacity_stack_vector (std::size_t size, T init = T()) : _size (size)
    {
      std::fill (begin(), end(), init);
    }

    T const* begin() const { return _data; } T* begin() { return _data; }
    T const* end() const { return _data + _size; } T* end() { return _data + _size; }
    T& operator[] (std::size_t i) { return _data[i]; }

  private:
    T _data[Capacity];
    std::size_t const _size;
  };
}

//! \todo collect all lose functions/classes/structs for now, sort them later

class sExtendableArray
{
public:
  std::vector<char> data;

	void Allocate (unsigned long pSize)
	{
    data.resize (pSize);
	}

	void Extend (long pAddition)
	{
    data.resize (data.size() + pAddition);
	}

  void Insert (unsigned long pPosition, unsigned long pAddition)
	{
    std::vector<char> tmp (pAddition);
    data.insert (data.begin() + pPosition, tmp.begin(), tmp.end());
  }

	void Insert (unsigned long pPosition, unsigned long pAddition, const char * pAdditionalData)
	{
    data.insert (data.begin() + pPosition, pAdditionalData, pAdditionalData + pAddition);
	}

	template<typename To>
	To * GetPointer(unsigned long pPosition = 0)
	{
		return(reinterpret_cast<To*>(data.data() + pPosition));
	}

  sExtendableArray() = default;
	sExtendableArray(unsigned long pSize, const char *pData)
    : data (pData, pData + pSize)
	{}
};

struct sChunkHeader
{
  int mMagic;
  int mSize;
};

void SetChunkHeader(sExtendableArray& pArray, int pPosition, int pMagix, int pSize = 0);

bool pointInside(math::vector_3d point, math::vector_3d extents[2]);
void minmax(math::vector_3d* a, math::vector_3d* b);

bool checkInside(math::vector_3d extentA[2], math::vector_3d extentB[2]);
bool checkOriginInside(math::vector_3d extentA[2], math::vector_3d modelPos);
