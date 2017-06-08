// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Help.h>

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTabWidget>

namespace noggit
{
  namespace ui
  {
    help::help()
      : widget (nullptr)
    {
      setWindowTitle ("Help");
      setWindowIcon (QIcon (":/icon"));
      setWindowState(Qt::WindowMaximized);

      auto layout (new QFormLayout (this));

      auto tabs (new QTabWidget (this));

      auto base_widget (new QWidget (this));
      auto base_layout (new QFormLayout (base_widget));
      
      base_layout->addRow (new QLabel (
        "Basic controls:\n"
        "  Left mouse dragged - Rotate camera\n"
        "  Left mouse click - Select chunk or object\n"
        "  I - Invert mouse up and down\n"
        "  Q, E - Move vertically up, down\n"
        "  A, D, W, S - Move left, right, forward, backward\n"
		"  Home (pos1) - Move position to the cursor\n"
        "  SHIFT+ C - Switch cursor type\n"
        "  CTRL + ALT + C - Toggle cursor options\n"
        "  M - Show map\n"
        "  U - 2D texture editor\n"
        "  CTRL + F1 - This help\n"
        "  SHIFT + R - Turn camera 180 degres\n"
        "  SHIFT + 1, 2, 3 or 4 - Set a predefined camera speed.\n"
        "  ALT + F4 - exit to main menu\n"
        "\n"
        "Toggles:\n"
        "  F1 - Toggle M2s\n"
        "  F2 - Toggle WMO doodads set\n"
        "  F3 - Toggle ground\n"
        "  F4 - Toggle water\n"
        "  F6 - Toggle WMOs\n"
        "  F7 - Toggle chunk (red) and ADT (green) lines\n"
        "  F8 - Toggle detailed window\n"
        "  F9 - Toggle map contour\n"
        "  F10 - Toggle wireframe\n"
        "  F11 - Toggle model animations\n"
        "  F12 - Toggle fog\n"
        "  1-9 - Select the editing modes\n"
        "\n"
        "Files:\n"
        "  F5 - save bookmark\n"
        "  CTRL + S -  Save all changed ADT tiles\n"
        "  CTRL + SHIFT + S - Save ADT tile at camera position\n"
        "  CTRL + SHIFT + A - Save all loaded ADT tiles\n"
        "  G - Save port commands to ports.txt\n"
        "\n"
        "Adjust:\n"
        "  O / P - slower/faster movement\n"
        "  B / N - slower/faster time\n"
        "  J - Pause time\n"
        "  SHIFT + +/-: fog distance when no model is selected\n"
      ));

      auto flag_widget (new QWidget (this));
      auto flag_layout (new QFormLayout (flag_widget));

      flag_layout->addRow (new QLabel (
        "Holes:\n"
        "  SHIFT + Left mouse - Remove hole\n"
        "  CTRL + Left mouse  - Add hole\n"
        "  T - Remove all holes on ADT\n"
        "  ALT + T - Remove all ground on ADT\n"
        "\n"
        "Impassible Flags:\n"
        "  SHIFT + Left mouse - Paint flag\n"
        "  CTRL + Left mouse  - Clear flag\n"
        "\n"
        "AreaID:\n"
        "  CTRL + Left mouse  - Pick existing AreaID\n"
        "  SHIFT + Left mouse - Paint selected AreaID\n"
      ));

      auto ground_widget (new QWidget (this));
      auto ground_layout (new QFormLayout (ground_widget));

      ground_layout->addRow (new QLabel (
        "Edit ground:\n"
        "  SHIFT + F1 - toggle ground edit mode\n"
        "  ALT + left mouse + mouse move - change brush size\n"
        "  SPACE + left mouse + mouse move - change speed\n"
        "  Z - Change the mode in option window\n"
        "\n"
        "Terrain mode \"raise / lower\":\n"
        "  SHIFT + Left mouse - raise terrain\n"
        "  CTRL + Left mouse - lower terrain\n"
        "  Y - switch to next type\n"
        "  ALT + right mouse + horizontal movement - change inner radius\n"
        "\n"
        "Terrain mode \"raise / lower\" (vertex mode only):\n"
        "  SHIFT + Left mouse - select vertices\n"
        "  CTRL + Left mouse - deselect vertices\n"
        "  C - clear selection\n"
        "  SPACE + F - Flatten vertices\n"
        "  SHIFT + right mouse + move - change vertices height\n"
        "  SHIFT + mouse wheel - change angle\n"
        "  ALT + mouse wheel - change orientation\n"
        "Terrain mode \"flatten / blur\":\n"
        "\n"
        "  SHIFT + Left mouse click - flatten terrain\n"
        "  CTRL + Left mouse  click - blur terrain\n"
        "  T - Toggle flatten angle\n"
        "  SPACE + T - Toggle flatten type\n"
        "  SHIFT + mouse wheel - change angle\n"
        "  ALT + mouse wheel - change orientation\n"
        "  Y - switch to next type\n"
        "  F - set relative point\n"
        "  SPACE + F - toggle flatten relative mode\n"
        "  SPACE + mouse wheel - change height\n"
      ));

      auto texture_widget (new QWidget (this));
      auto texture_layout (new QFormLayout (texture_widget));

      texture_layout->addRow (new QLabel (
        "Common controls:\n"
        "  CTRL + left mouse - open texture picker for the chunk\n"
        "\n"
        "Paint:\n"
        "  CTRL + SHIFT + ALT + left mouse - clear all textures on chunk\n"
        "  SHIFT + left mouse - draw texture or fills if chunk is empty\n"
        "  ALT + left mouse + horizontal movement - change radius\n"
        "  ALT + right mouse + horizontal movement - change hardness\n"
        "  SPACE + left mouse + mouse move - change pressure\n"
        "  SPACE + mouse wheel - change strength (gradient)\n"
		"  SPACE + R - Toggle min and max strength (gradient)\n"
        "  T - toggle spray brush\n"
        "  ALT + mouse wheel - change spray radius\n"
        "  SHIFT + mouse wheel - change spray pressure\n"
        "\n"
        "Swap:\n"
        "  SHIFT + left mouse - swap textures\n"
        "\n"
        "Anim:\n"
        "  F - set animation of the current texture on the chunk\n"
        "  R - remove current texture animation on the chunk\n"
      ));

      auto water_widget (new QWidget (this));
      auto water_layout (new QFormLayout (water_widget));

      water_layout->addRow (new QLabel (
        "Edit water:\n"
        "  SHIFT + Left mouse - add liquid\n"
        "  CTRL + Left mouse - remove liquid\n"
        "  ALT + left mouse + mouse move - change brush size\n"
        "  T - Toggle angled mode\n"
        "  ALT + mouse wheel - change orientation\n"
        "  SHIFT + mouse wheel - change angle\n"
        "  F - set lock pos to cursor pos\n"
        "  SPACE + F - toggle lock mode\n"
        "  SPACE + mouse wheel - change height\n"
      ));

      auto object_widget (new QWidget (this));
      auto object_layout (new QFormLayout (object_widget));

      object_layout->addRow (new QLabel (
        "Edit objects if a model is selected with left click (in object editor):\n"
        "  Hold middle mouse - move object\n"
        "  ALT + Hold middle mouse - scale M2\n"
        "  SHIFT / CTRL / ALT + Hold left mouse - rotate object\n"
        "  0 - 9 - change doodads set of selected WMO\n"
        "  CTRL + R - Reset rotation\n"
        "  H - Toggle selected model/wmo visibility\n"
        "  SPACE + H - Hide/Show hidden model/wmo\n"
        "  SHIFT + H - clear hidden model/wmo list\n"
        "  PageDown - Set object to Groundlevel\n"
        "  CTRL + C or C - Copy object to clipboard\n"
        "  CTRL + V or V - Paste object on mouse position\n"
	    "  CTRL + B - Duplicate selected object to mouse position\n"
        "  SHIFT + V - Insert last M2 from WMV\n"
        "  ALT + V - Insert last WMO from WMV\n"
        "  T - Toggle between paste modes\n"
        "  F - Move selection to cursor position\n"
        "  - / + - scale M2\n"
        "  Numpad 7 / 9 - rotate object\n"
        "  Numpad 4 / 8 / 6 / 2 - vertical position\n"
        "  Numpad 1 / 3 -  move up/dow\n"
        "    holding SHIFT: double speed \n"
        "    holding CTRL: triple speed \n"
        "    holding SHIFT and CTRL together: half speed \n"
      ));

      auto shader_widget (new QWidget (this));
      auto shader_layout (new QFormLayout (shader_widget));

      shader_layout->addRow (new QLabel (
        "  SHIFT + Left mouse - add shader\n"
        "  CTRL + Left mouse - remove shader\n"
        "  ALT + left mouse + mouse move - change brush size\n"
        "  SPACE + left mouse + mouse move - change speed\n"
      ));

      layout->addWidget(tabs);
      tabs->addTab(base_widget, "Base");
      tabs->addTab(ground_widget, "Terrain");
      tabs->addTab(texture_widget, "Texture");
      tabs->addTab(water_widget, "Water");
      tabs->addTab(object_widget, "Objects");
      tabs->addTab(shader_widget, "Shader");
      tabs->addTab(flag_widget, "Flags/Hole/Area");
    }
  }
}
