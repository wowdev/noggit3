// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cstdint>


class Mysql
{
public:
	uint32_t getGUIDFromDB();
	uint32_t UpdateUIDInDB(uint32_t NewUID);
};