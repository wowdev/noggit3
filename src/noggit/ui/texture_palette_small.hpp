// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/widget.hpp>
#include <unordered_set>
#include <string>



class QListWidget;
class QGridLayout;
class QPushButton;
class QDropEvent;
class QDragEnterEvent;


namespace noggit
{
  namespace ui
  {
    class current_texture;

    class texture_palette_small : public widget
    {
      Q_OBJECT

    public:
      texture_palette_small(QWidget* parent, current_texture* current_texture_window);

      void addTexture();
      void addTextureByFilename(const std::string& filename);

      void removeTexture(QString filename);

      void removeSelectedTexture();

      void updateWidget();

      void dragEnterEvent(QDragEnterEvent* event) override;
      void dropEvent(QDropEvent* event) override;


    signals:
      void selected(std::string);

    private:

      QGridLayout* layout;

      QListWidget* _texture_list;
      QPushButton* _add_button;
      QPushButton* _remove_button;
      std::unordered_set<std::string> _texture_paths;

    };
  }
}