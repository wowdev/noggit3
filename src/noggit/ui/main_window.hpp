#pragma once

#include <math/vector_3d.hpp>
#include <math/trig.hpp>

#include <QtWidgets/QMainWindow>

#include <string>

class World;

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
      void enterMapAt ( math::vector_3d pos
                      , math::degrees camera_pitch
                      , math::degrees camera_yaw
                      , World*
                      );

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
        float camera_yaw;
        float camera_pitch;
      };

      std::vector<MapEntry> mMaps;
      std::vector<BookmarkEntry> mBookmarks;

      minimap_widget* _minimap;
      QWidget* _null_widget;

      virtual void closeEvent (QCloseEvent*) override;
    };
  }
}
