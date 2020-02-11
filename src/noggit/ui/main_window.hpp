#pragma once

#include <math/vector_3d.hpp>
#include <math/trig.hpp>
#include <noggit/World.h>
#include <noggit/ui/uid_fix_window.hpp>

#include <QtWidgets/QMainWindow>

#include <string>
#include <unordered_set>

namespace noggit
{
  namespace ui
  {
    class minimap_widget;
    class settings;
    class about;

    struct main_window : QMainWindow
    {
      Q_OBJECT

    public:
      main_window();

      void prompt_exit(QCloseEvent* event);
      void prompt_uid_fix_failure();

      std::unordered_set<QWidget*> displayed_widgets;

    signals:
      void exit_prompt_opened();

    private:
      void loadMap (int mapID);

      void check_uid_then_enter_map ( math::vector_3d pos
                                    , math::degrees camera_pitch
                                    , math::degrees camera_yaw
                                    , bool from_bookmark = false
                                    );

      void enterMapAt ( math::vector_3d pos
                      , math::degrees camera_pitch
                      , math::degrees camera_yaw
                      , uid_fix_mode uid_fix = uid_fix_mode::none
                      , bool from_bookmark = false
                      );

      void createBookmarkList();
      void build_menu();
      void rebuild_menu();

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
      settings* _settings;
      about* _about;
      QWidget* _null_widget;

      std::unique_ptr<World> _world;

      bool map_loaded = false;

      virtual void closeEvent (QCloseEvent*) override;
    };
  }
}
