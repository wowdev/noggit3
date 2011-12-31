#include <noggit/ui/help_widget.h>

#include <QStringBuilder>

namespace ui
{
  help_widget::help_widget (QWidget* parent)
    : QTextBrowser (parent)
  {
    setWindowTitle(tr("Help"));
    resize (800, 800);

    #define __(key, action)   "<li><span class='key'>" \
                          % trUtf8 (key) \
                          % ": </span><span class='action'>" \
                          % trUtf8 (action) \
                          % "</span></li>"

    //! \todo This should be automatically generated so there aren't that many errors.

    setHtml ( QString (
		              "<html>"
					   "<head>"
						"<style>"
			          )
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
                       %         __ ("Right mouse dragged", "Rotate camera")
                       %         __ ("Left mouse", "Select chunk or object")
                       %         __ ("Both mouse buttons", "Move forward")
                       %         __ ("I", "Invert mouse y-axis")
                       %         __ ("Q, E", "Move vertically, up and down")
                       %         __ ("A, D, W, S", "Move left, right, up, down")
                       %         __ ("M", "Show minimap")
                       %         __ ("U", "2D texture editor")
               //! \todo: C chunk settings must get fixed first. Then turn on this again
               //      %         __ ("C", "Chunk settings")
                       %         __ ("C", "Switch cursor")
                       %         __ ("Alt + C", "Show cursor switcher")
                       %         __ ("H", "Help")
                       %         __ ("Shift + R", "Turn camera 180°")
                       %         __ ("Shift + F4", "Toggle automatic selection")
                       %        "</ul>"
			           %        "<h2>"
                       %         tr ("Toggles")
                       %        "</h2>"
			           %        "<ul>"
                       %         __ ("F1", "Models (M2s)")
                       %         __ ("F2", "Models in WMOs (doodad sets)")
                       %         __ ("F3", "Terrain")
               //! \todo This is NOT GUI. I bet.
                       %         __ ("F4", "GUI")
                       %         __ ("F6", "WMOs")
                       %         __ ("F7", "Lines around chunks(red) and ADTs (green)")
                       %         __ ("F8", "Detailed informations")
                       %         __ ("F9", "Height contours")
                       %         __ ("F", "Fog")
                       %         __ ("Tab", "UI")
                       %         __ ("X", "Texture palette in paint-mode")
                       %         __ ("Ctrl + X", "Detail window")
                       %         __ ("R, T", "Editing mode")
                       %        "</ul>"
			           %         "<h2>"
                       %         tr ("Files")
                       %        "</h2>"
			           %        "<ul>"
                       %         __ ("F5", "Save bookmark")
                       %         __ ("F10", "Reload textures (BLP)")
                       %         __ ("F11", "Reload models (M2)")
                       %         __ ("F12", "Reload objects (WMO)")
                       %         __ ("Shift + J", "Reload current ADT")
                       %         __ ("Ctrl + S", "Save all changed ADTs")
                       %         __ ("Ctrl + Shift + S", "Save the current ADT")
                       %        "</ul>"
			           %        "<h2>"
                       %         tr ("Adjust")
                       %        "</h2>"
			           %        "<ul>"
                       %         __ ("O, P", "Decrease, increase movement speed")
                       %         __ ("B, N", "Decrease, increase animation speed")
                       %         __ ("Shift + -, Shift + +", "Decrease, increase fog distance")
                       %        "</ul>"
                       %       "</div>"
                       %      "</td>"
                       %      "<td width='50%'>"
                       %       "<div class='right'>"
                       %        "<h2>"
                       %         "Edit ground"
                       %        "</h2>"
                       %        "<ul>"
                       %         __ ("SHIFT + F1", "Toggle ground edit mode")
                       %         __ ("T", "Change terrain mode")
                       %         __ ("Y", "Changes brush type")
                       %         __ ("ALT + Left mouse + Mouse move", "Change brush size")
                       %        "</ul> "
                       %        "<h2>"
                       %         "Terrain Mode"
                       %        "</h2>"
                       %        "<h3>"
                       %         "Raise / Lower"
                       %        "</h3>"
                       %        "<ul>"
                       %         __ ("SHIFT + Left mouse", "Raise terrain")
                       %         __ ("ALT + Left mouse", "Lower terrain")
                       %        "</ul>"
                       %        "<h3>"
                       %         "Flatten / Blur"
                       %        "</h3>"
                       %        "<ul>"
                       %         __ ("SHIFT + Left mouse", "Flatten terrain")
                       %         __ ("ALT + Left mouse", "Blur terrain")
                       %         __ ("Z", "Change the mode in the option window")
                       %        "</ul>"
                       %        "<br>"
                       %        "<h2>"
                       %         "Edit objects if a model is selected with left click"
                       %        "</h2>"
                       %        "<ul>"
                       %         __ ("Hold middle mouse", "Move object")
                       %         __ ("ALT + Hold middle mouse", "Scale M2")
                       %         __ ("SHIFT / CTRL / ALT + Hold left mouse", "Rotate object")
                       %         __ ("0 - 9", "Change doodads set of selected WMO")
                       %         __ ("CTRL + R", "Reset rotation")
                       %         __ ("PageDown", "Set object to Groundlevel")
                       %         __ ("CTRL + C", "Copy object to clipboard")
                       %         __ ("CTRL + V", "Paste object on mouse position")
                       %         __ ("- / +", "Scale M2")
                       %         __ ("Numpad 7 / 9", "rotate object")
                       %         __ ("Numpad 4 / 8 / 6 / 2", "Vertical position")
                       %         __ ("Numpad 1 / 3", "Move up/dow")
                       %         __ ("  holding SHIFT", "Double speed")
                       %         __ ("  holding CTRL", "Triple speed")
                       %         __ ("  holding SHIFT and CTRL together", "Half speed")
                       %        "</ul>"
                       %        "<h2>"
                       %         "Edit texture"
                       %        "</h2>"
                       %        "<ul>"
                       %         __ ("CTRL + SHIFT + Left mouse", "Clear all textures on chunk")
                       %         __ ("CTRL + Left mouse", "Draw texture or fills if chunk is empty")
                       %        "</ul>"
                       %        "<h2>"
                       %         "Holes"
                       %        "</h2>"
                       %        "<ul>"
                       %         __ ("SHIFT + Left mouse", "Add texture")
                       %         __ ("CTRL + Left mouse", "Remove texture")
                       %        "</ul>"
                       %        "<h2>"
                       %         "AreaID"
                       %        "</h2>"
                       %        "<ul>"
                       %         __ ("X", "Show browser with existing AreaID in AreaTable.dbc, click show sub-zones")
                       %         __ ("CTRL + Left mouse", "Pick existing AreaID")
                       %         __ ("SHIFT + Left mouse", "Paint selected AreaID")
                       %        "</ul>"
                       %        "<h2>"
                       %         "Impassible Flags"
                       %        "</h2>"
                       %        "<ul>"
                       %         __ ("SHIFT + Left mouse", "Paint flag")
                       %         __ ("CTRL + Left mouse", "Clear flag")
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
    #undef __
  }
}
