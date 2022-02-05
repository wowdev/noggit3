// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "map_horizon.h"

#include <noggit/MPQ.h>
#include <noggit/Log.h>
#include <noggit/map_index.hpp>
#include <noggit/World.h>
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
    return (_b) | (_g << 8) | (_r << 16) | (uint32_t)(0xFFu << 24);
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
namespace noggit
{

map_horizon::map_horizon(const std::string& basename, const MapIndex * const index)
{
  std::stringstream filename;
  filename << "World\\Maps\\" << basename << "\\" << basename << ".wdl";
  _filename = filename.str();

  if (!MPQFile::exists(_filename))
  {
    LogError << "file \"World\\Maps\\" << basename << "\\" << basename << ".wdl\" does not exist." << std::endl;
    return;
  }

  MPQFile wdl_file (_filename);

  uint32_t fourcc;
  uint32_t size;

  bool done = false;

  do
  {
    wdl_file.read(&fourcc, 4);
    wdl_file.read(&size, 4);

    switch (fourcc)
    {
      case 'MVER':
      {
        uint32_t version;
        wdl_file.read(&version, 4);
        assert(size == 4 && version == 18);

        break;
      }
      // todo: handle those too ?
      case 'MWMO':
      case 'MWID':
      case 'MODF':
        wdl_file.seekRelative(size);
        break;
      case 'MAOF':
      {
        assert(size == 64 * 64 * sizeof(uint32_t));

        uint32_t mare_offsets[64][64];
        wdl_file.read(mare_offsets, 64 * 64 * sizeof(uint32_t));

        // - MARE and MAHO by offset ---------------------------
        for (size_t y(0); y < 64; ++y)
        {
          for (size_t x(0); x < 64; ++x)
          {
            if (!mare_offsets[y][x])
            {
              continue;
            }

            wdl_file.seek(mare_offsets[y][x]);
            wdl_file.read(&fourcc, 4);
            wdl_file.read(&size, 4);

            assert(fourcc == 'MARE');
            assert(size == 0x442);

            _tiles[y][x] = std::make_unique<map_horizon_tile>();

            //! \todo There also is MAHO giving holes into this heightmap.
            wdl_file.read(_tiles[y][x]->height_17, 17 * 17 * sizeof(int16_t));
            wdl_file.read(_tiles[y][x]->height_16, 16 * 16 * sizeof(int16_t));
          }
        }

        done = true;
        break;
      }
      default:
        LogError << "unknown chunk in wdl: code=" << fourcc << std::endl;
        wdl_file.seekRelative(size);
        break;
    }
  } while (!done && !wdl_file.isEof());

  wdl_file.close();

  _qt_minimap = QImage (16 * 64, 16 * 64, QImage::Format_ARGB32);
  _qt_minimap.fill (Qt::transparent);

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (_tiles[y][x])
      {
        //! \todo There also is a second heightmap appended which has additional 16*16 pixels.
        //! \todo There also is MAHO giving holes into this heightmap.

        for (size_t j (0); j < 16; ++j)
        {
          for (size_t i (0); i < 16; ++i)
          {
            //! \todo R and B are inverted here
            _qt_minimap.setPixel(x * 16 + i, y * 16 + j, color_for_height(_tiles[y][x]->height_17[j][i]));
          }
        }
      }
      // the adt exist but there's no data in the wdl
      else if (index->hasTile(tile_index(x, y)))
      {
        for (size_t j(0); j < 16; ++j)
        {
          for (size_t i(0); i < 16; ++i)
          {
            _qt_minimap.setPixel(x * 16 + i, y * 16 + j, color(200, 100, 25));
          }
        }
      }
    }
  }
}

map_horizon::minimap::minimap(const map_horizon& horizon)
{
  std::vector<uint32_t> texture_data(1024 * 1024);

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (!horizon._tiles[y][x])
        continue;

      //! \todo There also is a second heightmap appended which has additional 16*16 pixels.

      // use the (nearly) full resolution available to us.
      // the data is layed out as a triangle fans with with 17 outer values
      // and 16 midpoints per tile. which in turn means:
      //      _tiles[y][x]->height_17[16][16] == _tiles[y][x + 1]->height_17[0][0]
      for (size_t j (0); j < 16; ++j)
      {
        for (size_t i (0); i < 16; ++i)
        {
          texture_data[(y * 16 + j) * 1024 + x * 16 + i] = color_for_height (horizon._tiles[y][x]->height_17[j][i]);
        }
      }
    }
  }

  bind();
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture_data.data());
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

map_horizon::render::render(const map_horizon& horizon)
{
  std::vector<math::vector_3d> vertices;

  for (size_t y (0); y < 64; ++y)
  {
    for (size_t x (0); x < 64; ++x)
    {
      if (!horizon._tiles[y][x])
        continue;

      _batches[y][x] = map_horizon_batch (vertices.size(), 17 * 17 + 16 * 16);

      for (size_t j (0); j < 17; ++j)
      {
        for (size_t i (0); i < 17; ++i)
        {
          vertices.emplace_back ( TILESIZE * (x + i / 16.0f)
                                , horizon._tiles[y][x]->height_17[j][i]
                                , TILESIZE * (y + j / 16.0f)
                                );
        }
      }

      for (size_t j (0); j < 16; ++j)
      {
        for (size_t i (0); i < 16; ++i)
        {
          vertices.emplace_back ( TILESIZE * (x + (i + 0.5f) / 16.0f)
                                , horizon._tiles[y][x]->height_16[j][i]
                                , TILESIZE * (y + (j + 0.5f) / 16.0f)
                                );
        }
      }
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d> (_vertex_buffer, vertices, GL_STATIC_DRAW);
}

static inline uint32_t outer_index(const map_horizon_batch &batch, int y, int x)
{
  return batch.vertex_start + y * 17 + x;
};

static inline uint32_t inner_index(const map_horizon_batch &batch, int y, int x)
{
  return batch.vertex_start + 17 * 17 + y * 16 + x;
};

void map_horizon::render::draw( math::matrix_4x4 const& model_view
                              , math::matrix_4x4 const& projection
                              , MapIndex *index
                              , const math::vector_3d& color
                              , const float& cull_distance
                              , const math::frustum& frustum
                              , const math::vector_3d& camera 
                              , display_mode display
                              )
{
  std::vector<uint32_t> indices;

  const tile_index current_index(camera);
  const int lrr = 2;

  for (size_t y (current_index.z - lrr); y <= current_index.z + lrr; ++y)
  {
    for (size_t x (current_index.x - lrr); x < current_index.x + lrr; ++x)
    {
      // x and y are unsigned so negative signed int value are positive and > 63
      if (x > 63 || y > 63)
      {
        continue;
      }

      map_horizon_batch const& batch = _batches[y][x];

      if (batch.vertex_count == 0)
        continue;

      for (size_t j (0); j < 16; ++j)
      {
        for (size_t i (0); i < 16; ++i)
        {
          // do not draw over visible chunks
          if (index->tileLoaded({y, x}) && index->getTile({y, x})->getChunk(j, i)->is_visible(cull_distance, frustum, camera, display))
          {
            continue;
          }

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j, i));
          indices.push_back (outer_index (batch, j + 1, i));

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j + 1, i));
          indices.push_back (outer_index (batch, j + 1, i + 1));

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j + 1, i + 1));
          indices.push_back (outer_index (batch, j, i + 1));

          indices.push_back (inner_index (batch, j, i));
          indices.push_back (outer_index (batch, j, i + 1));
          indices.push_back (outer_index (batch, j, i));
        }
      }
    }
  }

  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint32_t>(_index_buffer, indices, GL_STATIC_DRAW);

  if (!_map_horizon_program)
  {
    _map_horizon_program.reset
      ( new opengl::program
          { { GL_VERTEX_SHADER,   opengl::shader::src_from_qrc("horizon_vs") }
          , { GL_FRAGMENT_SHADER, opengl::shader::src_from_qrc("horizon_fs") }
          }
      );
  
    _vaos.upload();
  }
   

  opengl::scoped::use_program shader {*_map_horizon_program.get()};

  opengl::scoped::vao_binder const _ (_vao);

  shader.uniform ("model_view", model_view);
  shader.uniform ("projection", projection);
  shader.uniform ("color", color);

  shader.attrib (_, "position", _vertex_buffer, 3, GL_FLOAT, GL_FALSE, 0, 0);

  gl.drawElements (GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, _index_buffer);
}

}
