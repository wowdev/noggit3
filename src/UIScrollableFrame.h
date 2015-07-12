#ifndef __UISCROLLFRAME_H
#define __UISCROLLFRAME_H

#include "UIFrame.h"
#include "UIScrollBar.h"

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

#endif
