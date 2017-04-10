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
        "Basic controls:"
        "  Left mouse dragged - Rotate camera"
        "  Left mouse click - Select chunk or object"
        "  Both mouse buttons - Move forward"
        "  I - Invert mouse up and down"
        "  Q, E - Move vertically up, down"
        "  A, D, W, S - Move left, right, forward, backward"
        "  C - Switch cursor type"
        "  M - Show map"
        "  U - 2D texture editor"
        "  H - This help (if not in the object editor)"
        "  CTRL + ALT + C - Toggle cursor options"
        "  SHIFT + R - Turn camera 180 degres"
        "  SHIFT + F4 - Change to auto select mode"
        "  SHIFT + 1, 2, 3 or 4 - Set a predefined speed."
        "  ESC - exit to main menu"
        "\n"
        "Toggles:"
        "  F1 - Toggle M2s"
        "  F2 - Toggle WMO doodads set"
        "  F3 - Toggle ground"
        "  F4 - Toggle water"
        "  F6 - Toggle WMOs"
        "  F7 - Toggle chunk (red) and ADT (green) lines"
        "  F8 - Toggle detailed window"
        "  F9 - Toggle map contour"
        "  F - Toggle fog"
        "  TAB - toggle UI view"
        "  X - Toggle tool settings"
        "  CTRL + X - detail window"
        "  1,2,3,4,5 and 6 - Select the editing modes"
        "\n"
        "Files:"
        "  F5 - save bookmark"
        "  F10 - reload BLP"
        "  F11 - reload M2s"
        "  F12 - reload wmo"
        "  SHIFT + J - reload ADT tile"
        "  CTRL + S -  Save all changed ADT tiles"
        "  CTRL + SHIFT + S - Save ADT tile at camera position"
        "  CTRL + SHIFT + A - Save all loaded ADT tiles"
        "  g - Save port commands to ports.txt"
        "\n"
        "Adjust:"
        "  O / P - slower/faster movement"
        "  B / N - slower/faster time"
        "  SHIFT + +/-: fog distance when no model is selected"
      ));

      auto flag_widget (new QWidget (this));
      auto flag_layout (new QFormLayout (flag_widget));

      flag_layout->addRow (new QLabel (
        "Holes:"
        "  SHIFT + Left mouse - Remove hole"
        "  CTRL + Left mouse  - Add hole"
        "  T - Remove all holes on ADT"
        "  ALT + T - Remove all ground on ADT"
        "\n"
        "Impassible Flags:"
        "  SHIFT + Left mouse - Paint flag"
        "  CTRL + Left mouse  - Clear flag"
        "\n"
        "AreaID:"
        "  CTRL + Left mouse  - Pick existing AreaID"
        "  SHIFT + Left mouse - Paint selected AreaID"
      ));

      auto ground_widget (new QWidget (this));
      auto ground_layout (new QFormLayout (ground_widget));

      ground_layout->addRow (new QLabel (
        "Edit ground:"
        "  SHIFT + F1 - toggle ground edit mode"
        "  ALT + left mouse + mouse move - change brush size"
        "  Z - Change the mode in option window"
        "Terrain mode \"raise / lower\":"
        "  SHIFT + Left mouse - raise terrain"
        "  CTRL + Left mouse - lower terrain"
        "  Y - switch to next type"
        "  ALT + right mouse + horizontal movement - change inner radius"
        "Terrain mode \"raise / lower\" (vertex mode only):"
        "  SHIFT + Left mouse - select vertices"
        "  CTRL + Left mouse - deselect vertices"
        "  C - clear selection"
        "  SPACE + F - Flatten vertices"
        "  SHIFT + right mouse + move - change vertices height"
        "  SHIFT + mouse wheel - change angle"
        "  ALT + mouse wheel - change orientation"
        "Terrain mode \"flatten / blur\"\n"
        "  SHIFT + Left mouse click - flatten terrain"
        "  CTRL + Left mouse  click - blur terrain"
        "  T - Toggle flatten angle"
        "  SPACE + T - Toggle flatten type"
        "  SHIFT + mouse wheel - change angle"
        "  ALT + mouse wheel - change orientation"
        "  Y - switch to next type"
        "  F - set relative point"
        "  SPACE + F - toggle flatten relative mode"
        "  SPACE + mouse wheel - change height"
      ));

      auto texture_widget (new QWidget (this));
      auto texture_layout (new QFormLayout (texture_widget));

      texture_layout->addRow (new QLabel (
        "Edit texture:"
        "  CTRL + SHIFT + ALT + left mouse - clear all textures on chunk"
        "  SHIFT + left mouse - draw texture or fills if chunk is empty"
        "  CTRL + left mouse - open texture picker for the chunk"
        "  T - toggle spray brush"
        "  ALT + left mouse + horizontal movement - change radius"
        "  ALT + right mouse + horizontal movement - change hardness"
        "  SPACE + mouse wheel - change strength (gradient)"
        "  ALT + mouse wheel - change spray radius"
        "  SHIFT + mouse wheel - change spray pressure"
      ));

      auto water_widget (new QWidget (this));
      auto water_layout (new QFormLayout (water_widget));

      water_layout->addRow (new QLabel (
        "Edit water:"
        "  SHIFT + Left mouse - add liquid"
        "  CTRL + Left mouse - remove liquid"
        "  ALT + left mouse + mouse move - change brush size"
        "  T - Toggle angled mode"
        "  ALT + mouse wheel - change orientation"
        "  SHIFT + mouse wheel - change angle"
        "  F - set lock pos to cursor pos"
        "  SPACE + F - toggle lock mode"
        "  SPACE + mouse wheel - change height"
      ));

      auto object_widget (new QWidget (this));
      auto object_layout (new QFormLayout (object_widget));

      object_layout->addRow (new QLabel (
        "Edit objects if a model is selected with left click (in object editor):"
        "  Hold middle mouse - move object"
        "  ALT + Hold middle mouse - scale M2"
        "  SHIFT / CTRL / ALT + Hold left mouse - rotate object"
        "  0 - 9 - change doodads set of selected WMO"
        "  CTRL + R - Reset rotation"
        "  H - Toggle selected model/wmo visibility"
        "  SPACE + H - Hide/Show hidden model/wmo"
        "  SHIFT + H - clear hidden model/wmo list"
        "  PageDown - Set object to Groundlevel"
        "  CTRL + C or C - Copy object to clipboard"
        "  CTRL + V or V - Paste object on mouse position"
        "  SHIFT + V - Insert last M2 from WMV"
        "  ALT + V - Insert last WMO from WMV"
        "  T - Toggle between paste modes"
        "  F - Move selection to cursor position"
        "  - / + - scale M2"
        "  Numpad 7 / 9 - rotate object"
        "  Numpad 4 / 8 / 6 / 2 - vertical position"
        "  Numpad 1 / 3 -  move up/dow"
        "    holding SHIFT: double speed "
        "    holding CTRL: triple speed "
        "    holding SHIFT and CTRL together: half speed "
      ));

      layout->addWidget(tabs);
      tabs->addTab(base_widget, "Base");
      tabs->addTab(flag_widget, "Flags/Hole/Area");
      tabs->addTab(ground_widget, "Terrain");
      tabs->addTab(texture_widget, "Texture");
      tabs->addTab(water_widget, "Water");
      tabs->addTab(object_widget, "Objects");  
    }
  }
}
