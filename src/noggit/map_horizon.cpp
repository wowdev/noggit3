#include "map_horizon.h"

#include <noggit/MPQ.h>
#include <noggit/Log.h>
#include <noggit/map_index.hpp>
#include <opengl/context.hpp>

#include <sstream>

struct color
{
  color(unsigned char r, unsigned char g, unsigned char b)
    : _r(r)
    , _g(g)
    , _b(b)
  {}

  uint32_t to_int() const {
    return (_r) | (_g << 8) | (_b << 16) | (uint32_t)(255 << 24);
  }

  operator uint32_t () const {
    return to_int();
  }

  unsigned char _r;
  unsigned char _g;
  unsigned char _b;
};

struct ranged_color
{
  ranged_color (const color& c, const int16_t& start, const int16_t& stop)
    : _color (c)
    , _start (start)
    , _stop (stop) 
  {}

  const color   _color;
  const int16_t _start;
  const int16_t _stop;
};

static inline color lerp_color(const color& start, const color& end, float t)
{
  return color ( (end._r) * t + (start._r) * (1.0 - t)
               , (end._g) * t + (start._g) * (1.0 - t)
               , (end._b) * t + (start._b) * (1.0 - t)
               ); 
}

static inline uint32_t color_for_height (int16_t height)
{
  static const ranged_color colors[] =
    { ranged_color (color (20, 149, 7), 0, 600)
    , ranged_color (color (137, 84, 21), 600, 1200)
    , ranged_color (color (96, 96, 96), 1200, 1600)
    , ranged_color (color (255, 255, 255), 1600, 0x7FFF)
    };
  static const size_t num_colors (sizeof (colors) / sizeof (ranged_color));

  if (height < colors[0]._start)
  {
    return color (0, 0, 255 + std::max (height / 2.0, -255.0));
  }
  else if (height >= colors[num_colors - 1]._stop)
  {
    return colors[num_colors]._color;
  }

  float t (1.0);
  size_t correct_color (num_colors - 1);

  for (size_t i (0); i < num_colors - 1; ++i)
  {
    if (height >= colors[i]._start && height < colors[i]._stop)
    {
      t = float(height - colors[i]._start) / float (colors[i]._stop - colors[i]._start);
      correct_color = i;
      break;
    }
  }

  return lerp_color(colors[correct_color]._color, colors[correct_color + 1]._color, t);
}

map_horizon::map_horizon(const std::string& basename)
  : _finished_upload(false)
{
  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdl";
  _filename = filename.str();

  MPQFile wdl_file (_filename);
  if (wdl_file.isEof())
  {
    LogError << "file \"World\\Maps\\" << basename << "\\" << basename << ".wdl\" does not exist." << std::endl;
    return;
  }

  uint32_t fourcc;
  uint32_t size;

  // - MVER ----------------------------------------------

  uint32_t version;

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);
  wdl_file.read (&version, 4);

  assert (fourcc == 'MVER' && size == 4 && version == 18);

  // - MWMO ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MWMO');
      // Filenames for WMO that appear in the low resolution map. Zero terminated strings.

  wdl_file.seekRelative (size);

  // - MWID ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MWID');
      // List of indexes into the MWMO tile.

  wdl_file.seekRelative (size);

  // - MODF ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MODF');
      // Placement information for the WMO. Appears to be the same 64 byte structure used in the WDT and ADT MODF tiles.

  wdl_file.seekRelative (size);

  // - MAOF ----------------------------------------------

  wdl_file.read (&fourcc, 4);
  wdl_file.read (&size, 4);

  assert (fourcc == 'MAOF' && size == 64 * 64 * sizeof (uint32_t));

  uint32_t mare_offsets[64][64];
  wdl_file.read (mare_offsets, 64 * 64 * sizeof (uint32_t));

  // - MARE and MAHO by offset ---------------------------
  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (!mare_offsets[y][x])
        continue;

      wdl_file.seek (mare_offsets[y][x]);
      wdl_file.read (&fourcc, 4);
      wdl_file.read (&size, 4);

      assert (fourcc == 'MARE');
      assert (size == 0x442);

      _tiles[y][x] = std::make_unique<map_horizon_tile>();

      //! \todo There also is MAHO giving holes into this heightmap.
      wdl_file.read(_tiles[y][x]->height_17, 17 * 17 * sizeof(int16_t));
      wdl_file.read(_tiles[y][x]->height_16, 16 * 16 * sizeof(int16_t));
    }
  }

  wdl_file.close();
}

void map_horizon::upload() {
  uint32_t texture[1024][1024];
  memset(texture, 0, 1024 * 1024 * sizeof(uint32_t));

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (!_tiles[y][x])
        continue;

      //! \todo There also is a second heightmap appended which has additional 16*16 pixels.

      // use the (nearly) the full resolution available to us.
      // the data is layed out as a triangle fans with with 17 outer values
      // and 16 midpoints per tile. which in turn means:
      //      _tiles[y][x]->height_17[16][16] == _tiles[y][x + 1]->height_17[0][0]
      for (size_t j (0); j < 16; ++j)
      {
        for (size_t i (0); i < 16; ++i)
        {
          texture[y * 16 + j][x * 16 + i] = color_for_height (_tiles[y][x]->height_17[j][i]);
        }
      }
    }
  }

  minimap.bind();
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  _finished_upload = true;
}