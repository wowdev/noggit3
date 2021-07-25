// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <boost/optional.hpp>
#include <math/trig.hpp>
#include <math/vector_3d.hpp>
#include <math/vector_4d.hpp>
#include <noggit/Log.h>
#include <util/sExtendableArray.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <noggit/Selection.h>

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
  bool square_is_in_circle(float x, float z, float radius, float square_x, float square_z, float square_size);
  bool rectOverlap(math::vector_3d const*, math::vector_3d const*);
  // used for angled tools, get the height a point (pos) should be given an origin, angle and orientation
  float angledHeight(math::vector_3d const& origin, math::vector_3d const& pos, math::radians const& angle, math::radians const& orientation);
  void extract_v3d_min_max(math::vector_3d const& point, math::vector_3d& min, math::vector_3d& max);
  std::vector<math::vector_3d> intersection_points(math::vector_3d const& vmin, math::vector_3d const& vmax);  
  math::vector_3d transform_model_box_coords(math::vector_3d const& pos);
  // normalize the filename used in adts since TC extractors don't accept /
  std::string normalize_adt_filename(std::string filename);

  // see http://realtimecollisiondetection.net/blog/?p=89 for more info
  inline bool float_equals(float const& a, float const& b)
  {
    return std::abs(a - b) < (std::max(1.f, std::max(a, b)) * std::numeric_limits<float>::epsilon());
  }

  bool vec3d_equals(math::vector_3d const& v1, math::vector_3d const& v2);
  bool deg_vec3d_equals(math::degrees::vec3 const& v1, math::degrees::vec3 const& v2);

  inline int rounded_int_div(int value, int div)
  {
    return value / div + (value % div <= (div >> 1) ? 0 : 1);
  }
  inline int rounded_255_int_div(int value)
  {
    return value / 255 + (value % 255 <= 127 ? 0 : 1);
  }

  // treat the value as an 8x8 array of bit
  inline void set_bit(std::uint64_t& value, int x, int y, bool on)
  {
    std::uint64_t bit = std::uint64_t(1) << (y * 8 + x);
    value = on ? (value | bit) : (value & ~bit);
  }
  inline void bit_or(std::uint64_t& value, int x, int y, bool on)
  {
    if (on)
    {
      value |= (std::uint64_t(1) << (y * 8 + x));
    }
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

struct sChunkHeader
{
  int mMagic;
  int mSize;
};

void SetChunkHeader(util::sExtendableArray& pArray, int pPosition, int pMagix, int pSize = 0);

bool pointInside(math::vector_3d point, math::vector_3d extents[2]);
void minmax(math::vector_3d* a, math::vector_3d* b);
