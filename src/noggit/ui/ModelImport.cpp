// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ModelImport.h>

#include <noggit/MapView.h>
#include <noggit/ModelInstance.h>
#include <noggit/Selection.h>
#include <noggit/Settings.h>
#include <noggit/WMOInstance.h>

#include <string>
#include <fstream>

#include <QtWidgets/QFormLayout>

namespace noggit
{
  namespace ui
  {
    model_import::model_import (MapView* mapview)
      : QWidget (nullptr)
    {
      setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
      auto layout (new QFormLayout (this));

      layout->addRow ("Filter", _textBox = new QLineEdit (this));
      connect ( _textBox, &QLineEdit::textChanged
              , [this]
                {
                  buildModelList();
                }
              );

      layout->addWidget (_list = new QListWidget (this));

      buildModelList();

      connect ( _list, &QListWidget::itemDoubleClicked
              , [mapview] (QListWidgetItem* item)
                {
                  if (item->text().endsWith (".m2"))
                  {
                    auto mi (new ModelInstance (item->text().toStdString()));
                    mi->sc = 1.0f;
                    mapview->selectModel(mi);
                  }
                  else if (item->text().endsWith (".wmo"))
                  {
                    mapview->selectModel (new WMOInstance (item->text().toStdString()));
                  }
                }
              );
    }

    void model_import::buildModelList()
    {
      _list->clear();

      std::ifstream fileReader (Settings::getInstance()->importFile);
      std::string const filter
        (mpq::normalized_filename (_textBox->text().toStdString()));

      std::string line;
      while (std::getline (fileReader, line))
      {
        if (line.empty())
        {
          continue;
        }

        std::string path (mpq::normalized_filename (line));

        if (!filter.empty() && path.find (filter) == std::string::npos)
        {
          continue;
        }

        _list->addItem (QString::fromStdString (path));
      }
    }
  }
}
