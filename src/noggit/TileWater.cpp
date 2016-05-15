#include <noggit/TileWater.hpp>

#include <noggit/ChunkWater.hpp>
#include <noggit/MapTile.h>

namespace noggit
{
  TileWater::TileWater(MapTile *pTile, float pXbase, float pZbase)
    : tile(pTile)
    , xbase(pXbase)
    , zbase(pZbase)
  {
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        chunks[i][j].reset (new ChunkWater(xbase + CHUNKSIZE * j, zbase + CHUNKSIZE * i));
      }
    }
  }

  void TileWater::readFromFile(mpq::file& theFile, size_t basePos)
  {
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        theFile.seek(basePos + (i * 16 + j) * sizeof(MH2O_Header));
        chunks[i][j]->fromFile(theFile, basePos);
      }
    }
    reload();
  }

  void TileWater::reload()
  {
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        chunks[i][j]->reloadRendering();
      }
    }
  }

  void TileWater::draw (Skies const* skies)
  {
    glDisable (GL_COLOR_MATERIAL);
    glDisable (GL_LIGHTING);

    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        chunks[i][j]->draw (skies);
      }
    }

    glEnable (GL_LIGHTING);
    glEnable (GL_COLOR_MATERIAL);
  }

  ChunkWater* TileWater::getChunk(int x, int y)
  {
    return chunks[x][y].get();
  }

  void TileWater::autoGen(int factor)
  {
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        chunks[i][j]->autoGen(tile->getChunk(j, i), factor);
      }
    }
  }

  namespace
  {
    template<typename T>
      T* get_pointer (std::vector<char>& vector, std::size_t position = 0)
    {
      return reinterpret_cast<T*> (&vector[position]);
    }
    void set_chunk_header (std::vector<char>& vector, std::size_t position, uint32_t magic, uint32_t size = 0 )
    {
      *get_pointer<uint32_t> (vector, position + 0) = magic;
      *get_pointer<uint32_t> (vector, position + 4) = size;
    }
  }

  void TileWater::saveToFile (std::vector<char>& adt_file, int& mhdr_position, int& write_position)
  {
    if(!hasData()) return;
    size_t ofsW = write_position + 0x8; //water Header pos

    get_pointer<MHDR> (adt_file, mhdr_position + 8)->mh2o = write_position - 0x14; //setting offset to MH2O data in Header
    adt_file.resize (adt_file.size() + 8);
    write_position = ofsW;
    size_t headerOffsets[16][16];

    //writing MH2O_Header
    for(int i = 0; i < 16; ++i)
    {
      for(int j = 0; j < 16; ++j)
      {
        headerOffsets[i][j] = write_position;
        chunks[i][j]->writeHeader(adt_file, write_position);
      }
    }

    //writing MH2O_Information
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        chunks[i][j]->writeInfo(adt_file, ofsW, write_position); //let chunk check if there is info!
      }
    }

    //writing other Info
    for(int i = 0; i < 16; ++i)
    {
      for(int j = 0; j < 16; ++j)
      {
        chunks[i][j]->writeData(headerOffsets[i][j], adt_file, ofsW, write_position);
      }
    }

    set_chunk_header (adt_file, ofsW - 8, 'MH2O', write_position - ofsW);
    write_position += 8;
  }

  bool TileWater::hasData()
  {
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        if(chunks[i][j]->hasData())
        {
          return true;
        }
      }
    }

    return false;
  }

  void TileWater::deleteLayer()
  {
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        chunks[i][j]->deleteLayer();
      }
    }
  }

  void TileWater::addLayer(float height, unsigned char trans)
  {
    addLayer();
    setHeight (height);
    setTrans (trans);
  }

  void TileWater::addLayer()
  {
    for(int i (0); i < 16; ++i)
    {
      for(int j (0); j < 16; ++j)
      {
        chunks[i][j]->addLayer();
      }
    }
  }

#define accessor(T_, name_)                           \
  void TileWater::set ## name_ (T_ value)             \
  {                                                   \
    for(int i (0); i < 16; ++i)                       \
    {                                                 \
      for(int j (0); j < 16; ++j)                     \
      {                                               \
        chunks[i][j]->set ## name_ (value);           \
      }                                               \
    }                                                 \
  }                                                   \
  boost::optional<T_> TileWater::get ## name_() const \
  {                                                   \
    for(int i (0); i < 16; ++i)                       \
    {                                                 \
      for(int j (0); j < 16; ++j)                     \
      {                                               \
        if(chunks[i][j]->hasData())                   \
        {                                             \
          return chunks[i][j]->get ## name_();        \
        }                                             \
      }                                               \
    }                                                 \
    return boost::none;                               \
  }

  accessor (float, Height)
  accessor (unsigned char, Trans)
  accessor (int, Type)

#undef accessor
}
