#include <ui/help_widget.h>

#include <QStringBuilder>

namespace ui
{
  help_widget::help_widget (QWidget* parent)
    : QTextBrowser (parent)
  {
    resize (765, 800);

  #define __(key, action)   "<li><span class='key'>" \
                          % trUtf8 (key) \
                          % ": </span><span class='action'>" \
                          % trUtf8 (action) \
                          % "</span></li>"

    //! \todo This should be automatically generated so there aren't that many errors.

    setHtml ( QString ("<html><head><style>")
    //! \todo Style.
            % ".key {font-weight: bold;}"
            % "</style></head><body><h1>"
            % tr ("Key bindings")
            % "</h1><h2>"
            % tr ("Basic controls")
            % "</h2><ul>"
            % __ ("Right mouse dragged", "Rotate camera")
            % __ ("Left mouse", "Select chunk or object")
            % __ ("Both mouse buttons", "Move forward")
            % __ ("I", "Invert mouse y-axis")
            % __ ("Q, E", "Move vertically, up and down")
            % __ ("A, D, W, S", "Move left, right, up, down")
            % __ ("M", "Show minimap")
            % __ ("U", "2D texture editor")
    //! \todo: C chunk settings must get fixed first. Then turn on this again
    //      % __ ("C", "Chunk settings")
            % __ ("C", "Switch cursor")
            % __ ("Alt + C", "Show cursor switcher")
            % __ ("H", "Help")
            % __ ("Shift + R", "Turn camera 180°")
            % __ ("Shift + F4", "Toggle automatic selection")
            % "</ul><h2>"
            % tr ("Toggles")
            % "</h2><ul>"
            % __ ("F1", "Models (M2s)")
            % __ ("F2", "Models in WMOs (doodad sets)")
            % __ ("F3", "Terrain")
    //! \todo This is NOT GUI. I bet.
            % __ ("F4", "GUI")
            % __ ("F6", "WMOs")
            % __ ("F7", "Lines around chunks(red) and ADTs (green)")
            % __ ("F8", "Detailed informations")
            % __ ("F9", "Height contours")
            % __ ("F", "Fog")
            % __ ("Tab", "UI")
            % __ ("X", "Texture palette in paint-mode")
            % __ ("Ctrl + X", "Detail window")
            % __ ("R, T", "Editing mode")
            % "</ul><h2>"
            % tr ("Files")
            % "</h2><ul>"
            % __ ("F5", "Save bookmark")
            % __ ("F10", "Reload textures (BLP)")
            % __ ("F11", "Reload models (M2)")
            % __ ("F12", "Reload objects (WMO)")
            % __ ("Shift + J", "Reload current ADT")
            % __ ("Ctrl + S", "Save all changed ADTs")
            % __ ("Ctrl + Shift + S", "Save the current ADT")
            % "</ul><h2>"
            % tr ("Adjust")
            % "</h2><ul>"
            % __ ("O, P", "Decrease, increase movement speed")
            % __ ("B, N", "Decrease, increase animation speed")
            % __ ("Shift + -, Shift + +", "Decrease, increase fog distance")
            % "</ul>"
            % "</body></html>"

  #undef __

    //! \todo Also add these correctly. I really can't be arsed right now.

            %
      "Edit ground:\n<ul>"
      "  <li>SHIFT + F1 - toggle ground edit mode\n"
      "  <li>T - change terrain mode\n"
      "  <li>Y - changes brush type\n"
      "  <li>ALT + left mouse + mouse move - change brush size\n"
      "</ul>Terrain mode \"raise / lower\":\n<ul>"
      "  <li>SHIFT + Left mouse - raise terrain\n"
      "  <li>ALT + Left mouse - lower terrain\n"
      "</ul>Terrain mode \"flatten / blur\"\n"
      "  <li>SHIFT + Left mouse click - flatten terrain\n"
      "  <li>ALT + Left mouse  click - blur terrain\n"
      "  <li>Z - change the mode in the option window\n"
      "\n"
      "</ul>Edit objects if a model is selected with left click:\n<ul>"
      "  <li>Hold middle mouse - move object\n"
      "  <li>ALT + Hold middle mouse - scale M2\n"
      "  <li>SHIFT / CTRL / ALT + Hold left mouse - rotate object\n"
      "  <li>0 - 9 - change doodads set of selected WMO\n"
      "  <li>CTRL + R - Reset rotation\n"
      "  <li>PageDown - Set object to Groundlevel\n"
      "  <li>CTRL + C - Copy object to clipboard\n"
      "  <li>CTRL + V - Paste object on mouse position\n"
      "  <li>- / + - scale M2\n"
      "  <li>Numpad 7 / 9 - rotate object\n"
      "  <li>Numpad 4 / 8 / 6 / 2 - vertical position\n"
      "  <li>Numpad 1 / 3 -  move up/dow\n"
      "  <li>  holding SHIFT: double speed \n"
      "  <li>  holding CTRL: triple speed \n"
      "  <li>  holding SHIFT and CTRL together: half speed \n"
      "\n"
      "</ul>Edit texture:\n<ul>"
      "  <li>CTRL + SHIFT + left mouse - clear all textures on chunk\n"
      "  <li>CTRL + left mouse - draw texture or fills if chunk is empty\n"
      "\n"
    "</ul>Holes:\n<ul>"
    "  <li>SHIFT + Left mouse - Add texture\n"
    "  <li>CTRL + Left mouse  - Remove texture\n"
    "\n"
    "</ul>AreaID:\n<ul>"
    "  <li>X - Show browser with existing AreaID in AreaTable.dbc\n"
    "  <li>    Click show sub-zones\n"
    "  <li>CTRL + Left mouse  - Pick existing AreaID\n"
    "  <li>SHIFT + Left mouse - Paint selected AreaID\n"
    "\n"
    "</ul>Impassible Flags:\n<ul>"
    "  <li>SHIFT + Left mouse - Paint flag\n"
    "  <li>CTRL + Left mouse  - Clear flag\n</ul>"
            );
  }
}
