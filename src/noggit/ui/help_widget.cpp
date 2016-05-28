// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/help_widget.h>

#include <QStringBuilder>

namespace noggit
{
  namespace ui
  {
    help_widget::help_widget (QWidget* parent)
    : QTextBrowser (parent)
    {
      setWindowTitle (tr ("Help"));
      resize (800, 800);

      #define entry(key, action) "<li><span class='key'>" \
                            % trUtf8 (key) \
                            % ": </span><span class='action'>" \
                            % trUtf8 (action) \
                            % "</span></li>"

      //! \todo This should be automatically generated so there aren't that many errors.

      setHtml ( QString ("<html>")
              % "<head>"
              % "<style>"
              % ".template {width: 825px; font-size: 11pt; font-family: Times new Roman, sans-serif;}"
              % ".key {font-weight: bold;}"
              % ".title {text-decoration: underline;}"
              % ".left {}"
              % ".right {margin-left: 10px;}"
              % "h1 {font-size: 16px; font-family: Friz Quadrata TT, Helvetica, sans-serif;}"
              % "h2 {font-size: 14px; font-weight: bold;}"
              % "h3 {margin-left: 10px; font-variant: small-caps; font-size: 12px;}"
              % ".clear {clear: both;}"
              % "</style>"
              % "</head>"
              % "<body class='template'>"
              %  "<center>"
              %   "<h1 class='title' align='center'>"
              %    tr ("Key bindings")
              %   "</h1>"
              %   "<br>"
              %   "<div class='wrapper'>"
              %    "<table width='100%' border='0'>"
              %     "<tr>"
              %      "<td width='50%'>"
              %       "<div class='left'>"
              %        "<h2>"
              %         tr ("Basic controls")
              %        "</h2>"
              %        "<ul>"
              %         entry ("Right mouse dragged", "Rotate camera")
              %         entry ("Left mouse", "Select chunk or object")
              %         entry ("Both mouse buttons", "Move forward")
              %         entry ("I", "Invert mouse y-axis")
              %         entry ("Q, E", "Move vertically, up and down")
              %         entry ("A, D, W, S", "Move left, right, up, down")
              %         entry ("M", "Show minimap")
              %         entry ("U", "2D texture editor")
              //! \todo: C chunk settings must get fixed first. Then turn on this again
              //      %         entry ("C", "Chunk settings")
              %         entry ("C", "Switch cursor")
              %         entry ("Alt + C", "Show cursor switcher")
              %         entry ("H", "Help")
              %         entry ("Shift + R", "Turn camera 180°")
              %         entry ("Shift + F4", "Toggle automatic selection")
              %        "</ul>"
              %        "<h2>"
              %         tr ("Toggles")
              %        "</h2>"
              %        "<ul>"
              %         entry ("F1", "Models (M2s)")
              %         entry ("F2", "Models in WMOs (doodad sets)")
              %         entry ("F3", "Terrain")
              //! \todo This is NOT GUI. I bet.
              %         entry ("F4", "GUI")
              %         entry ("F6", "WMOs")
              %         entry ("F7", "Lines around chunks(red) and ADTs (green)")
              %         entry ("F8", "Detailed informations")
              %         entry ("F9", "Height contours")
              %         entry ("F", "Fog")
              %         entry ("Tab", "UI")
              %         entry ("X", "Texture palette in paint-mode")
              %         entry ("Ctrl + X", "Detail window")
              %         entry ("R, T", "Editing mode")
              %        "</ul>"
              %         "<h2>"
              %         tr ("Files")
              %        "</h2>"
              %        "<ul>"
              %         entry ("F5", "Save bookmark")
              %         entry ("F10", "Reload textures (BLP)")
              %         entry ("F11", "Reload models (M2)")
              %         entry ("F12", "Reload objects (WMO)")
              %         entry ("Shift + J", "Reload current ADT")
              %         entry ("Ctrl + S", "Save all changed ADTs")
              %         entry ("Ctrl + Shift + S", "Save the current ADT")
              %        "</ul>"
              %        "<h2>"
              %         tr ("Adjust")
              %        "</h2>"
              %        "<ul>"
              %         entry ("O, P", "Decrease, increase movement speed")
              %         entry ("B, N", "Decrease, increase animation speed")
              %         entry ("Shift + -, Shift + +", "Decrease, increase fog distance")
              %        "</ul>"
              %       "</div>"
              %      "</td>"
              %      "<td width='50%'>"
              %       "<div class='right'>"
              %        "<h2>"
              %         "Edit ground"
              %        "</h2>"
              %        "<ul>"
              %         entry ("SHIFT + F1", "Toggle ground edit mode")
              %         entry ("T", "Change terrain mode")
              %         entry ("Y", "Changes brush type")
              %         entry ("ALT + Left mouse + Mouse move", "Change brush size")
              %        "</ul> "
              +        QString ("<h2>")
              %         "Terrain Mode"
              %        "</h2>"
              %        "<h3>"
              %         "Raise / Lower"
              %        "</h3>"
              %        "<ul>"
              %         entry ("SHIFT + Left mouse", "Raise terrain")
              %         entry ("ALT + Left mouse", "Lower terrain")
              %        "</ul>"
              %        "<h3>"
              %         "Flatten / Blur"
              %        "</h3>"
              %        "<ul>"
              %         entry ("SHIFT + Left mouse", "Flatten terrain")
              %         entry ("ALT + Left mouse", "Blur terrain")
              %         entry ("Z", "Change the mode in the option window")
              %        "</ul>"
              %        "<br>"
              %        "<h2>"
              %         "Edit objects if a model is selected with left click"
              %        "</h2>"
              %        "<ul>"
              %         entry ("Hold middle mouse", "Move object")
              %         entry ("ALT + Hold middle mouse", "Scale M2")
              %         entry ("SHIFT / CTRL / ALT + Hold left mouse", "Rotate object")
              %         entry ("0 - 9", "Change doodads set of selected WMO")
              %         entry ("CTRL + R", "Reset rotation")
              %         entry ("PageDown", "Set object to Groundlevel")
              %         entry ("CTRL + C", "Copy object to clipboard")
              %         entry ("CTRL + V", "Paste object on mouse position")
              %         entry ("- / +", "Scale M2")
              %         entry ("Numpad 7 / 9", "rotate object")
              %         entry ("Numpad 4 / 8 / 6 / 2", "Vertical position")
              %         entry ("Numpad 1 / 3", "Move up/dow")
              %         entry ("  holding SHIFT", "Double speed")
              %         entry ("  holding CTRL", "Triple speed")
              %         entry ("  holding SHIFT and CTRL together", "Half speed")
              %        "</ul>"
              %        "<h2>"
              %         "Edit texture"
              %        "</h2>"
              %        "<ul>"
              %         entry ("CTRL + SHIFT + Left mouse", "Clear all textures on chunk")
              %         entry ("CTRL + Left mouse", "Draw texture or fills if chunk is empty")
              %        "</ul>"
              %        "<h2>"
              %         "Holes"
              %        "</h2>"
              %        "<ul>"
              %         entry ("SHIFT + Left mouse", "Add texture")
              %         entry ("CTRL + Left mouse", "Remove texture")
              %        "</ul>"
              %        "<h2>"
              %         "AreaID"
              %        "</h2>"
              %        "<ul>"
              %         entry ("X", "Show browser with existing AreaID in AreaTable.dbc, click show sub-zones")
              %         entry ("CTRL + Left mouse", "Pick existing AreaID")
              %         entry ("SHIFT + Left mouse", "Paint selected AreaID")
              %        "</ul>"
              %        "<h2>"
              %         "Impassible Flags"
              %        "</h2>"
              %        "<ul>"
              %         entry ("SHIFT + Left mouse", "Paint flag")
              %         entry ("CTRL + Left mouse", "Clear flag")
              %        "</ul>"
              %       "</div>"
              %       "<br class='clear'"
              %      "</td>"
              %     "</tr>"
              %    "</table>"
              %   "</div>"
              %  "</center>"
              % "</body>"
              % "</html>"
              );
      #undef entry
    }
  }
}
