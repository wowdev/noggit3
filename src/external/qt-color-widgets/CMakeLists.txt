#
# Copyright (C) 2013-2017 Mattia Basaglia
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

set(SOURCES
src/abstract_widget_list.cpp
src/alphaback.png
src/bound_color_selector.cpp
src/color_2d_slider.cpp
src/color_delegate.cpp
src/color_dialog.cpp
src/color_line_edit.cpp
src/color_list_widget.cpp
src/color_names.cpp
src/color_palette.cpp
src/color_palette_model.cpp
src/color_palette_widget.cpp
src/color_preview.cpp
src/color_selector.cpp
src/color_utils.cpp
src/color_wheel.cpp
src/color_widgets.qrc
src/gradient_slider.cpp
src/hue_slider.cpp
src/swatch.cpp
)

set(HEADERS
qt-color-widgets/abstract_widget_list.hpp
qt-color-widgets/bound_color_selector.hpp
qt-color-widgets/color_2d_slider.hpp
qt-color-widgets/color_delegate.hpp
qt-color-widgets/color_dialog.hpp
qt-color-widgets/color_line_edit.hpp
qt-color-widgets/color_list_widget.hpp
qt-color-widgets/color_palette.hpp
qt-color-widgets/color_palette_model.hpp
qt-color-widgets/color_palette_widget.hpp
qt-color-widgets/color_preview.hpp
qt-color-widgets/color_selector.hpp
qt-color-widgets/color_wheel.hpp
qt-color-widgets/gradient_slider.hpp
qt-color-widgets/hue_slider.hpp
qt-color-widgets/swatch.hpp
)

qt5_wrap_cpp(SOURCES ${HEADERS})
qt5_wrap_ui(SOURCES src/color_dialog.ui src/color_palette_widget.ui)
qt5_add_resources(SOURCES src/color_widgets.qrc)

# Library
add_library(ColorWidgets-qt5 ${SOURCES})
target_link_libraries(ColorWidgets-qt5 Qt5::Widgets)
target_include_directories (ColorWidgets-qt5 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (ColorWidgets-qt5 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_local_compiler_flag_if_supported ("-Wno-error=deprecated-declarations" ColorWidgets_CXX_FLAGS)
target_compile_options (ColorWidgets-qt5 PRIVATE ${ColorWidgets_CXX_FLAGS})
