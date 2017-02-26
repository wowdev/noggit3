#pragma once

#include <math/vector_3d.hpp>

#include <QtWidgets/QMainWindow>

#include <string>

namespace noggit
{
  namespace ui
  {
    class minimap_widget;

    struct main_window : QMainWindow
    {
      main_window();

      void prompt_exit();

    private:
      void loadMap (int mapID);
      void enterMapAt (math::vector_3d pos, float av, float ah);

      void createBookmarkList();
      void build_menu();

      struct MapEntry
      {
        int mapID;
        std::string name;
        int areaType;
      };

      struct BookmarkEntry
      {
        int mapID;
        std::string name;
        math::vector_3d pos;
        float ah;
        float av;
      };

      std::vector<MapEntry> mMaps;
      std::vector<BookmarkEntry> mBookmarks;

      minimap_widget* _minimap;
      QWidget* _null_widget;

      virtual void closeEvent (QCloseEvent*) override;
    };
  }
}
