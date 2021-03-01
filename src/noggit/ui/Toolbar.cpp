// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/Toolbar.h>

namespace noggit
{
  namespace ui
  {
    toolbar::toolbar(std::function<void (editing_mode)> set_editing_mode)
      : _set_editing_mode (set_editing_mode)
      , _tool_group(this)
    {
      add_tool_icon (editing_mode::ground,       tr("Raise / Lower"),   font_awesome::chartarea);
      add_tool_icon (editing_mode::flatten_blur, tr("Flatten / Blur"),  font_awesome::bacon);
      add_tool_icon (editing_mode::paint,        tr("3D Paint"),        font_awesome::paintbrush);
      add_tool_icon (editing_mode::holes,        tr("Holes"),           font_awesome::cut);
      add_tool_icon (editing_mode::areaid,       tr("AreaID Paint"),    font_awesome::objectgroup);
      add_tool_icon (editing_mode::flags,        tr("Impassible Flag"), font_awesome::ban);
      add_tool_icon (editing_mode::water,        tr("Water edit"),      font_awesome::water);
      add_tool_icon (editing_mode::mccv,         tr("Shader editor"),   font_awesome::eyedropper);
      add_tool_icon (editing_mode::object,       tr("Object editor"),   font_awesome::cube);
#ifdef NOGGIT_HAS_SCRIPTING
      add_tool_icon (editing_mode::scripting,    tr("Script Editor"),   font_awesome::book);
#endif
    }

    void toolbar::add_tool_icon(editing_mode mode, const QString& name, const font_awesome::icons& icon)
    {
      auto action = addAction(font_awesome_icon(icon), name);
      connect (action, &QAction::triggered, [this, mode] () {
        _set_editing_mode (mode);
      });
      action->setActionGroup(&_tool_group);
      action->setCheckable(true);
    }

    void toolbar::check_tool(editing_mode mode)
    {
      _tool_group.actions()[static_cast<std::size_t> (mode)]->setChecked(true);
    }
  }
}
