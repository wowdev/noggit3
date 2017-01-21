// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Frame.h>
#include <noggit/ui/ScrollBar.h>

class UIScrollableFrame : public UIFrame
{
public:
  typedef UIScrollableFrame* Ptr;

private:
  UIFrame::Ptr _content;
  UIScrollBar::Ptr _scrollbarHorizontal;
  UIScrollBar::Ptr _scrollbarVertical;

  float _scrollPositionX;
  float _scrollPositionY;

public:
  UIScrollableFrame(float x, float y, float w, float h, UIFrame::Ptr content);

  //public slots:
  void contentUpdated();
  void scrolledHorizontal(int value);
  void scrolledVertical(int value);

  void render() const;
};
