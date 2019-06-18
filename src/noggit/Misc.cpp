// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Misc.h>
#include <noggit/Selection.h>
#include <noggit/ModelInstance.h>
#include <noggit/WMOInstance.h>

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <boost/optional.hpp>

namespace misc
{
  void find_and_replace(std::string& source, const std::string& find, const std::string& replace)
  {
    size_t found = source.rfind(find);
    while (found != std::string::npos) //fixed unknown letters replace. Now it works correctly and replace all found symbold instead of only one at previous versions
    {
      source.replace(found, find.length(), replace);
      found = source.rfind(find);
    }
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

  float dist(float x1, float z1, float x2, float z2)
  {
    float xdiff = x2 - x1, zdiff = z2 - z1;
    return std::sqrt(xdiff*xdiff + zdiff*zdiff);
  }

  float dist(math::vector_3d const& p1, math::vector_3d const& p2)
  {
    return dist(p1.x, p1.z, p2.x, p2.z);
  }

  // return the shortest distance between the point (x, z)
  // and square at (squareX, squareZ) with a size of unitSize
  float getShortestDist(float x, float z, float squareX, float squareZ, float unitSize)
  {
    float px, pz;

    if (x >= squareX && x < squareX + unitSize)
    {
      px = x;
    }
    else
    {
      px = (squareX < x) ? squareX + unitSize : squareX;
    }

    if (z >= squareZ && z < squareZ + unitSize)
    {
      pz = z;
    }
    else
    {
      pz = (squareZ < z) ? squareZ + unitSize : squareZ;
    }

    return (px == x && pz == z) ? 0.0f : dist(x, z, px, pz);
  }

  float getShortestDist(math::vector_3d const& pos, math::vector_3d const& square_pos, float unitSize)
  {
    return getShortestDist(pos.x, pos.z, square_pos.x, square_pos.z, unitSize);
  }

  bool square_is_in_circle(float x, float z, float radius, float square_x, float square_z, float square_size)
  {
    float px, pz;

    if (std::abs(square_x - x) < std::abs(square_x + square_size - x))
    {
      px = square_x + square_size;
    }
    else
    {
      px = square_x;
    }

    if (std::abs(square_z - z) < std::abs(square_z + square_size - z))
    {
      pz = square_z + square_size;
    }
    else
    {
      pz = square_z;
    }

    // check if the furthest is in the circle
    float d = dist(x, z, px, pz);
    return d <= radius;
  }

  bool rectOverlap(math::vector_3d const* r1, math::vector_3d const* r2)
  {
    return r1[0].x <= r2[1].x
      && r2[0].x <= r1[1].x
      && r1[0].z <= r2[1].z
      && r2[0].z <= r1[1].z;
  }

  float angledHeight(math::vector_3d const& origin, math::vector_3d const& pos, math::radians const& angle, math::radians const& orientation)
  {
    return ( origin.y
           + (  (pos.x - origin.x) * math::cos(orientation)
              + (pos.z - origin.z) * math::sin(orientation)
             ) * math::tan(angle));
  }

  void extract_v3d_min_max(math::vector_3d const& point, math::vector_3d& min, math::vector_3d& max)
  {
    min.x = std::min(min.x, point.x);
    max.x = std::max(max.x, point.x);
    min.y = std::min(min.y, point.y);
    max.y = std::max(max.y, point.y);
    min.z = std::min(min.z, point.z);
    max.z = std::max(max.z, point.z);
  }

  std::vector<math::vector_3d> intersection_points(math::vector_3d const& vmin, math::vector_3d const& vmax)
  {
    std::vector<math::vector_3d> points;

    points.emplace_back (vmin.x, vmin.y, vmin.z);
    points.emplace_back (vmin.x, vmin.y, vmax.z);
    points.emplace_back (vmin.x, vmax.y, vmin.z);
    points.emplace_back (vmin.x, vmax.y, vmax.z);
    points.emplace_back (vmax.x, vmin.y, vmin.z);
    points.emplace_back (vmax.x, vmin.y, vmax.z);
    points.emplace_back (vmax.x, vmax.y, vmin.z);
    points.emplace_back (vmax.x, vmax.y, vmax.z);

    return points;
  }

  std::vector<math::vector_3d> box_points(math::vector_3d const& box_min, math::vector_3d const& box_max)
  {
    std::vector<math::vector_3d> points;

    points.emplace_back (box_max.x, box_max.y, box_max.z);
    points.emplace_back (box_max.x, box_max.y, box_min.z);
    points.emplace_back (box_max.x, box_min.y, box_max.z);
    points.emplace_back (box_max.x, box_min.y, box_min.z);
    points.emplace_back (box_min.x, box_max.y, box_max.z);
    points.emplace_back (box_min.x, box_max.y, box_min.z);
    points.emplace_back (box_min.x, box_min.y, box_max.z);
    points.emplace_back (box_min.x, box_min.y, box_min.z);

    return points;
  }

  math::vector_3d transform_model_box_coords(math::vector_3d const& pos)
  {
    return {pos.x, pos.z, -pos.y};
  }

  std::string normalize_adt_filename(std::string filename)
  {
    std::transform (filename.begin(), filename.end(), filename.begin(), ::toupper);
    std::transform ( filename.begin(), filename.end(), filename.begin()
                   , [](char c)
                     {
                       return c == '/' ? '\\' : c;
                     }
                   );
    return filename;
  }
}

void SetChunkHeader(sExtendableArray& pArray, int pPosition, int pMagix, int pSize)
{
  sChunkHeader* Header = pArray.GetPointer<sChunkHeader>(pPosition);
  Header->mMagic = pMagix;
  Header->mSize = pSize;
}

bool pointInside(math::vector_3d point, math::vector_3d extents[2])
{
  minmax(&extents[0], &extents[1]);

  return point.x >= extents[0].x && point.z >= extents[0].z &&
    point.x <= extents[1].x && point.z <= extents[1].z;
}

void minmax(math::vector_3d* a, math::vector_3d* b)
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

bool checkInside(math::vector_3d extentA[2], math::vector_3d extentB[2])
{
  minmax(&extentA[0], &extentA[1]);
  minmax(&extentB[0], &extentB[1]);

  return pointInside(extentA[0], extentB) ||
    pointInside(extentA[1], extentB) ||
    pointInside(extentB[0], extentA) ||
    pointInside(extentB[1], extentA);
}

bool checkOriginInside(math::vector_3d extentA[2], math::vector_3d modelPos)
{
  return pointInside(modelPos, extentA);
}

math::vector_3d getObjectPosition(selection_type object)
{
  if (object.which() == eEntry_Model)
  {
    return boost::get<selected_model_type>(object)->pos;
  }
  else if (object.which() == eEntry_WMO)
  {
    return boost::get<selected_wmo_type>(object)->pos;
  }
}

math::vector_3d getMedianPivotPoint(std::vector<selection_type> entries)
{
  // get pivot point
  math::vector_3d minPos;
  math::vector_3d maxPos;
  auto firstSelectionObject = entries.front();
  minPos = getObjectPosition(firstSelectionObject);
  maxPos = getObjectPosition(firstSelectionObject);

  for (auto& selection : entries)
  {
    minPos = std::min(minPos, getObjectPosition(selection));
    maxPos = std::max(maxPos, getObjectPosition(selection));
  }

  auto pivotPoint = (minPos + maxPos) / 2.0f;

  return pivotPoint;
}

void rotateByXAxis(selection_type object, math::vector_3d pivotPoint, float angle)
{
  auto cos = std::cos(angle);
  auto sin = std::sin(angle);

  math::vector_3d* position;

  if (object.which() == eEntry_Model)
  {
    position = &boost::get<selected_model_type>(object)->pos;
  }
  else if (object.which() == eEntry_WMO)
  {
    position = &boost::get<selected_wmo_type>(object)->pos;
  }

  position->y -= pivotPoint.y;
  position->z -= pivotPoint.z;

  float newY = position->y * cos - position->z * sin;
  float newZ = position->y * sin + position->z * cos;

  position->y = newY + pivotPoint.y;
  position->z = newZ + pivotPoint.z;
}

void rotateByYAxis(selection_type object, math::vector_3d pivotPoint, float angle)
{
  auto cos = std::cos(angle);
  auto sin = std::sin(angle);

  math::vector_3d* position;

  if (object.which() == eEntry_Model)
  {
    position = &boost::get<selected_model_type>(object)->pos;
  }
  else if (object.which() == eEntry_WMO)
  {
    position = &boost::get<selected_wmo_type>(object)->pos;
  }

  position->x -= pivotPoint.x;
  position->z -= pivotPoint.z;

  float newX = position->x * cos + position->z * sin;
  float newZ = -position->x * sin + position->z * cos;

  position->x = newX + pivotPoint.x;
  position->z = newZ + pivotPoint.z;
}

void rotateByZAxis(selection_type object, math::vector_3d pivotPoint, float angle)
{
  auto cos = std::cos(angle);
  auto sin = std::sin(angle);

  math::vector_3d* position;

  if (object.which() == eEntry_Model)
  {
    position = &boost::get<selected_model_type>(object)->pos;
  }
  else if (object.which() == eEntry_WMO)
  {
    position = &boost::get<selected_wmo_type>(object)->pos;
  }

  position->x -= pivotPoint.x;
  position->y -= pivotPoint.y;

  float newX = position->x * cos - position->y * sin;
  float newY = position->x * sin + position->y * cos;

  position->x = newX + pivotPoint.x;
  position->y = newY + pivotPoint.y;
}

float calculateRotationXAngle(math::vector_3d position1, math::vector_3d position2, math::vector_3d pivotPoint)
{
  auto numerator = position2.z * (pivotPoint.y - position1.y) + pivotPoint.z * (position1.y - position2.y) + position1.z * (position2.y - pivotPoint.y);
  auto denominator = (position2.y - pivotPoint.y) * (pivotPoint.y - position1.y) + (position2.z - pivotPoint.z) * (pivotPoint.z - position1.z);
  auto ratio = numerator / denominator;
  auto angleRad = std::atan(ratio);
  auto angleDeg = (angleRad * 180) / math::constants::pi;
  return angleDeg;
}

float calculateRotationYAngle(math::vector_3d position1, math::vector_3d position2, math::vector_3d pivotPoint)
{
  auto numerator = position2.z * (pivotPoint.x - position1.x) + pivotPoint.z * (position1.x - position2.x) + position1.z * (position2.x - pivotPoint.x);
  auto denominator = (position2.x - pivotPoint.x) * (pivotPoint.x - position1.x) + (position2.z - pivotPoint.z) * (pivotPoint.z - position1.z);
  auto ratio = numerator / denominator;
  auto angleRad = std::atan(ratio);
  return (angleRad * 180) / math::constants::pi;
}

float calculateRotationZAngle(math::vector_3d position1, math::vector_3d position2, math::vector_3d pivotPoint)
{
  auto numerator = position2.y * (pivotPoint.x - position1.x) + pivotPoint.y * (position1.x - position2.x) + position1.y * (position2.x - pivotPoint.x);
  auto denominator = (position2.x - pivotPoint.x) * (pivotPoint.x - position1.x) + (position2.y - pivotPoint.y) * (pivotPoint.y - position1.y);
  auto ratio = numerator / denominator;
  auto angleRad = std::atan(ratio);
  return (angleRad * 180) / math::constants::pi;
}
