// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <opengl/texture.hpp>

#include <memory>

class MapIndex;

struct map_horizon_tile
{
    int16_t height_17[17][17];
    int16_t height_16[16][16];
};

class map_horizon
{
public:
    map_horizon(const std::string& basename);

    void upload();

    opengl::texture minimap;

    //! \todo make this private
    //! (create base class for all delayed ogl objects)
    bool _finished_upload;

private:
    std::string _filename;

    std::unique_ptr<map_horizon_tile> _tiles[64][64];
};