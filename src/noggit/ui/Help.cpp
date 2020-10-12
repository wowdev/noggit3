// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Help.h>
#include <noggit/ui/font_noggit.hpp>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTabWidget>

#include <initializer_list>

namespace noggit
{
  namespace ui
  {

    help::help(QWidget* parent)
      : widget (parent)
    {
      setWindowTitle ("Help");
      setWindowIcon (QIcon (":/icon"));
      setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);

      QString header_style =
        "QLabel { \n "
        "  font-weight: bold; \n "
        "} \n ";


      auto layout (new QFormLayout (this));
      layout->setSizeConstraint(QLayout::SetFixedSize);

      auto tabs (new QTabWidget (this));

      auto base_widget (new QWidget (this));
      auto base_layout(new QGridLayout (base_widget));


      auto basic_controls_layout (new QFormLayout (this));
      base_layout->addLayout(basic_controls_layout, 0, 0);

      base_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      
      auto label = new QLabel("Basic controls:");
      label->setStyleSheet(header_style);
      basic_controls_layout->addRow(label);

      generate_hotkey_row({font_noggit::rmb_drag},                                            "\a - Rotate camera", basic_controls_layout);
      generate_hotkey_row({font_noggit::lmb},                                                 "\a - Select chunk or object", basic_controls_layout);
      generate_hotkey_row({font_noggit::i},                                                   "\a - Invert mouse up and down", basic_controls_layout);
      generate_hotkey_row({font_noggit::q, font_noggit::e},                                   "\a,\a - Invert mouse up and down", basic_controls_layout);
      generate_hotkey_row({font_noggit::w, font_noggit::a , font_noggit::s , font_noggit::d}, "\a\a\a\a - Move left, right, forward, backwards", basic_controls_layout);
      generate_hotkey_row({font_noggit::home},                                                "\a - Move position to the cursor", basic_controls_layout);
      generate_hotkey_row({font_noggit::shift, font_noggit::c},                               "\a+\a - Switch cursor type", basic_controls_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::alt},                              "\a+\a - Toggle cursor options", basic_controls_layout);
      generate_hotkey_row({font_noggit::m},                                                   "\a - Show map", basic_controls_layout);
      generate_hotkey_row({font_noggit::u},                                                   "\a - 2D texture editor", basic_controls_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::f1},                               "\a+\a - This help", basic_controls_layout);
      generate_hotkey_row({font_noggit::shift, font_noggit::j},                               "\a+\a - reload an adt under the camera", basic_controls_layout);
      generate_hotkey_row({font_noggit::shift, font_noggit::r},                               "\a+\a - Turn camera 180 degrees", basic_controls_layout);
      generate_hotkey_row({font_noggit::shift},                                               "\a + 1, 2, 3, or 4 - Set a predefined camera speed", basic_controls_layout);
      generate_hotkey_row({font_noggit::alt, font_noggit::f4},                                "\a+\a - exit to main menu", basic_controls_layout);
      generate_hotkey_row({font_noggit::l},                                                   "\a - Change lookat (useful for assigning to graphic tablet styli buttons).", basic_controls_layout);
      generate_hotkey_row({},                                                                 "", basic_controls_layout); // padding
       
      auto toggles_layout(new QFormLayout(this));
      base_layout->addLayout(toggles_layout, 0, 1);

      auto label_toggle = new QLabel("Toggles:");
      label_toggle->setStyleSheet(header_style);
      toggles_layout->addRow(label_toggle);

      generate_hotkey_row({font_noggit::f1},  "\a - Toggle M2s", toggles_layout);
      generate_hotkey_row({font_noggit::f2},  "\a - Toggle WMO doodads set", toggles_layout);
      generate_hotkey_row({font_noggit::f3},  "\a - Toggle ground", toggles_layout);
      generate_hotkey_row({font_noggit::f4},  "\a - Toggle water", toggles_layout);
      generate_hotkey_row({font_noggit::f6},  "\a - Toggle WMOs", toggles_layout);
      generate_hotkey_row({font_noggit::f7},  "\a - Toggle chunk (red) and ADT (green) lines", toggles_layout);
      generate_hotkey_row({font_noggit::f8},  "\a - Toggle detailed window", toggles_layout);
      generate_hotkey_row({font_noggit::f9},  "\a - Toggle map contour", toggles_layout);
      generate_hotkey_row({font_noggit::f10}, "\a - Toggle wireframe", toggles_layout);
      generate_hotkey_row({font_noggit::f11}, "\a - Toggle model animations", toggles_layout);
      generate_hotkey_row({font_noggit::f12}, "\a - Toggle fog", toggles_layout);
      generate_hotkey_row({},                 "1-9 - Select the editing modes", toggles_layout);

      auto files_layout(new QFormLayout(this));
      base_layout->addLayout(files_layout, 1, 0);

      auto label_files = new QLabel("Files:");
      label_files->setStyleSheet(header_style);
      files_layout->addRow(label_files);

      generate_hotkey_row({font_noggit::f5},                                       "\a - Save bookmark", files_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::s},                     "\a+\a - Save all changed ADT tiles", files_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::shift, font_noggit::s}, "\a+\a+\a - Save ADT tile at camera position", files_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::shift, font_noggit::a}, "\a+\a+\a- Save all loaded ADT tiles", files_layout);
      generate_hotkey_row({font_noggit::g},                                        "\a - Save port commands to ports.txt", files_layout);

      auto adjust_layout(new QFormLayout(this));
      base_layout->addLayout(adjust_layout, 1, 1);

      auto label_adjust = new QLabel("Adjust:");
      label_adjust->setStyleSheet(header_style);
      adjust_layout->addRow(label_adjust);

      generate_hotkey_row({font_noggit::o, font_noggit::p},                            "\a/\a- Slower / Faster movement", adjust_layout);
      generate_hotkey_row({font_noggit::b, font_noggit::n},                            "\a/\a- Slower / Faster time", adjust_layout);
      generate_hotkey_row({font_noggit::j},                                            "\a- Pause time", adjust_layout);
      generate_hotkey_row({font_noggit::shift, font_noggit::plus, font_noggit::minus}, "\a+\a/\a- Fog distance when no model is selected", adjust_layout);

      auto flag_widget (new QWidget (this));
      auto flag_layout (new QFormLayout (flag_widget));

      auto holes_label = new QLabel("Holes:");
      holes_label->setStyleSheet(header_style);
      flag_layout->addRow(holes_label);

      generate_hotkey_row({font_noggit::shift, font_noggit::lmb}, "\a+\a- Fog distance when no model is selected", flag_layout);
      generate_hotkey_row({font_noggit::ctrl, font_noggit::lmb},  "\a+\a- Add hole", flag_layout);
      generate_hotkey_row({font_noggit::t},                       "\a- Remove all holes on ADT", flag_layout);
      generate_hotkey_row({font_noggit::alt, font_noggit::t},     "\a+\a- Remove all ground on ADT", flag_layout);

      auto impass_flags_label = new QLabel("Impassible Flags:");
      impass_flags_label->setStyleSheet(header_style);
      flag_layout->addRow(impass_flags_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\a - Paint flag", flag_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },  "\a+\a - Clear flag", flag_layout);

      auto areaid_label = new QLabel("AreaID Flags:");
      areaid_label->setStyleSheet(header_style);
      flag_layout->addRow(areaid_label);

      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },  "\a+\a -  Pick existing AreaID", flag_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\a -  Paint selected AreaID", flag_layout);


      auto ground_widget (new QWidget (this));
      auto ground_layout (new QGridLayout (ground_widget));

      auto ground_column1_layout(new QFormLayout(this));
      ground_layout->addLayout(ground_column1_layout, 0, 0);

      auto ground_label = new QLabel("Edit ground:");
      ground_label->setStyleSheet(header_style);
      ground_column1_layout->addRow(ground_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::f1 },       "\a+\a -  Toggle ground edit mode", ground_column1_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag },   "\a+\a -  Change brush size", ground_column1_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::lmb_drag }, "\a+\a -  Change speed", ground_column1_layout);

      auto raise_label = new QLabel("Terrain mode \"raise / lower\":");
      raise_label->setStyleSheet(header_style);
      ground_column1_layout->addRow(raise_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },    "\a+\a -  Raise terrain", ground_column1_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },     "\a+\a -  Lower terrain", ground_column1_layout);
      generate_hotkey_row({ font_noggit::y },                          "\a -  Switch to next type", ground_column1_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::rmb_drag }, "\a+\a -  Change inner radius", ground_column1_layout);

      auto raise_label_vm = new QLabel("Terrain mode \"raise / lower\" (vertex mode only):");
      raise_label_vm->setStyleSheet(header_style);
      ground_column1_layout->addRow(raise_label_vm);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },      "\a+\a -  Select vertices", ground_column1_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },       "\a+\a -  Deselect vertices", ground_column1_layout);
      generate_hotkey_row({ font_noggit::c },                            "\a -  Clear selection", ground_column1_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::f },        "\a+\a - Flatten vertices", ground_column1_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::rmb_drag }, "\a+\a -  Orient vertices toward the mouse cursor", ground_column1_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::rmb_drag }, "\a+\a -  Change vertices height", ground_column1_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb },      "\a+\a -  Change angle", ground_column1_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },        "\a+\a -  Change orientation", ground_column1_layout);

      auto ground_column2_layout(new QFormLayout(this));
      ground_layout->addLayout(ground_column2_layout, 0, 1);

      auto flatten_label = new QLabel("Terrain mode \"flatten / blur\":");
      flatten_label->setStyleSheet(header_style);
      ground_column2_layout->addRow(flatten_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\a -  Flatten terrain", ground_column2_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },  "\a+\a -  Blur terrain", ground_column2_layout);
      generate_hotkey_row({ font_noggit::t },                       "\a - Toggle flatten angle", ground_column2_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::t },   "\a+\a -  Toggle flatten type", ground_column2_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb }, "\a+\a -  Change angle", ground_column2_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },   "\a+\a -  Change orientation", ground_column2_layout);
      generate_hotkey_row({ font_noggit::y },                       "\a -  Switch to next type", ground_column2_layout);
      generate_hotkey_row({ font_noggit::f },                       "\a -  Set relative point", ground_column2_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::f },   "\a+\a -  Toggle flatten relative mode", ground_column2_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::mmb }, "\a+\a -  Change height", ground_column2_layout);


      auto texture_widget (new QWidget (this));
      auto texture_layout (new QFormLayout (texture_widget));

      auto common_controls_label = new QLabel("Common controls:");
      common_controls_label->setStyleSheet(header_style);
      texture_layout->addRow(common_controls_label);

      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb }, "\a+\a - Open texture picker for the chunk", texture_layout);

      auto paint_label = new QLabel("Paint:");
      paint_label->setStyleSheet(header_style);
      texture_layout->addRow(paint_label);

      generate_hotkey_row({ font_noggit::ctrl, font_noggit::shift, font_noggit::alt, font_noggit::lmb }, "\a+\a+\a+\a - Deletes all textures for the chunk", texture_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },                                      "\a+\a - Draw texture or fills if chunk is empty", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag },                                   "\a+\a - Change radius", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::rmb_drag },                                   "\a+\a - Change hardness", texture_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::lmb_drag },                                 "\a+\a - Change pressure", texture_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::mmb },                                      "\a+\a - Change strength (gradient)", texture_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::r },                                        "\a+\a - Toggle min and max strength (gradient)", texture_layout);
      generate_hotkey_row({ font_noggit::t },                                                            "\a - Toggle spray brush", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },                                        "\a+\a - Change spray radius", texture_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb },                                      "\a+\a - Change spray pressure", texture_layout);

      auto swapper_label = new QLabel("Swap:");
      swapper_label->setStyleSheet(header_style);
      texture_layout->addRow(swapper_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },    "\a+\a - Swap texture", texture_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag }, "\a+\a - Change radius", texture_layout);
      generate_hotkey_row({ font_noggit::t },                          "\a - Toggle brush swapper", texture_layout);

      auto anim_label = new QLabel("Anim:");
      anim_label->setStyleSheet(header_style);
      texture_layout->addRow(anim_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb }, "\a+\a - Update animation", texture_layout);
      generate_hotkey_row({ font_noggit::t },                       "\a - Switch between add/remove animation mode", texture_layout);


      auto water_widget (new QWidget (this));
      auto water_layout (new QFormLayout (water_widget));

      auto water_label = new QLabel("Water:");
      water_label->setStyleSheet(header_style);
      water_layout->addRow(water_label);

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },    "\a+\a - Add liquid", water_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },     "\a+\a - Remove liquid", water_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag }, "\a+\a - Change brush size", water_layout);
      generate_hotkey_row({ font_noggit::t },                          "\a - Toggle angled mode", water_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },      "\a+\a - Change orientation", water_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::mmb },    "\a+\a - Change angle", water_layout);
      generate_hotkey_row({ font_noggit::f },                          "\a - Set lock position to cursor position", water_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::f },      "\a+\a - Toggle lock mode", water_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::mmb },    "\a+\a - Change height", water_layout);


      auto object_widget (new QWidget (this));
      auto object_layout (new QFormLayout (object_widget));

      auto object_label = new QLabel("Edit objects if a model is selected with left click (in object editor):");
      object_label->setStyleSheet(header_style);
      object_layout->addRow(object_label);

      generate_hotkey_row({ font_noggit::mmb },                                                          "\a - Move object", object_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::mmb },                                        "\a+\a - Scale M2", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::ctrl, font_noggit::alt, font_noggit::lmb }, "\a/\a/\a+\a - Rotate object", object_layout);
      generate_hotkey_row({ font_noggit::ctrl },                                                         "\a+ 0-9 - Change doodadset of selected WMO", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::r },                                         "\a+\a - Reset rotation", object_layout);
      generate_hotkey_row({ font_noggit::h },                                                            "\a - Toggle selected model/wmo visibility", object_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::h },                                        "\a+\a - Hide/Show hidden model/wmo", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::h },                                        "\a+\a - Clear hidden model/wmo list", object_layout);
      generate_hotkey_row({ font_noggit::page_down },                                                    "\a - Set object to ground level", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::c, font_noggit::c },                         "\a+\a or \a - Copy object to clipboard", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::v, font_noggit::v },                         "\a+\a or \a - Paste object on mouse position", object_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::b },                                         "\a+\a - Duplicate selected object to mouse position", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::v },                                        "\a+\a - Import last M2 from WMV", object_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::v },                                          "\a+\a - Import last WMO from WMV", object_layout);
      generate_hotkey_row({ font_noggit::t },                                                            "\a - Switch between paste modes", object_layout);
      generate_hotkey_row({ font_noggit::f },                                                            "\a - Move selection to cursor position", object_layout);
      generate_hotkey_row({ font_noggit::minus, font_noggit::plus },                                     "\a/\a - Scale M2", object_layout);
      generate_hotkey_row({ font_noggit::num },                                                          "\a 7 / 9 - Rotate object", object_layout);
      generate_hotkey_row({ font_noggit::num },                                                          "\a 4 / 8 / 6 / 2 - Vertical position", object_layout);
      generate_hotkey_row({ font_noggit::num },                                                          "\a 1 / 3 -  Move up/down", object_layout);
      generate_hotkey_row({ font_noggit::shift },                                                        "Holding \a 1 / 3 - Double speed", object_layout);
      generate_hotkey_row({ font_noggit::ctrl },                                                         "Holding \a 1 / 3 - Triple speed", object_layout);
      generate_hotkey_row({ font_noggit::shift, font_noggit::ctrl },                                     "Holding \a and \a together - half speed", object_layout);

      auto shader_widget (new QWidget (this));
      auto shader_layout (new QFormLayout (shader_widget));

      generate_hotkey_row({ font_noggit::shift, font_noggit::lmb },      "\a+\a - Add shader", shader_layout);
      generate_hotkey_row({ font_noggit::ctrl, font_noggit::lmb },       "\a+\a - Remove shader", shader_layout);
      generate_hotkey_row({ font_noggit::alt, font_noggit::lmb_drag },   "\a+\a - Change brush size", shader_layout);
      generate_hotkey_row({ font_noggit::space, font_noggit::lmb_drag }, "\a+\a - Change speed", shader_layout);
      generate_hotkey_row({ font_noggit::mmb },                          "\a - Pick shader color from the ground", shader_layout);
      generate_hotkey_row({ font_noggit::plus },                         "\a - Add current color to palette", shader_layout);

      layout->addWidget(tabs);
      tabs->addTab(base_widget, "Base");
      tabs->addTab(ground_widget, "Terrain");
      tabs->addTab(texture_widget, "Texture");
      tabs->addTab(water_widget, "Water");
      tabs->addTab(object_widget, "Objects");
      tabs->addTab(shader_widget, "Shader");
      tabs->addTab(flag_widget, "Flags/Hole/Area");
    }


    void help::generate_hotkey_row(std::initializer_list<font_noggit::icons>&& hotkeys, const char* description, QFormLayout* layout)
    {
      auto row_layout = new QHBoxLayout(this);

      const char* from = nullptr;
      auto icon = hotkeys.begin();

      while (*description)
      {
        if (*description == '\a')
        {
          if (from)
          {
            auto label = new QLabel(::std::string(from, description - from).c_str());
            row_layout->addWidget(label);
          }
            
          auto label = new QLabel(this);
          QIcon hotkey_icon = font_noggit_icon(*icon++);
          label->setPixmap(hotkey_icon.pixmap(20, 20));
          row_layout->addWidget(label);

          from = ++description;
        }
        else
        {
          if (!from)
          {
            from = description;
          }     
          ++description;
        }
      }

      if (from && *from)
      {
        auto label = new QLabel(from);
        row_layout->addWidget(label);
      }

      row_layout->setAlignment(Qt::AlignLeft);
      layout->addRow(row_layout);

    }

  }
}
