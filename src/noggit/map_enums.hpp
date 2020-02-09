// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

enum class tile_mode
{
  // regular mode
  edit,
  // uid fix mode, ignore/change loading and saving to
  // reduce the uid fix all time
  uid_fix_all
};

enum class model_update
{
  add,
  remove,
  none
};
