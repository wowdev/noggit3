#ifndef __FRAME_H
#define __FRAME_H

#include <vector>

class frame
{
public:
	std::vector<frame *> children;
	float x,y;
	float width, height;

	frame *parent;

	bool movable;
	bool hidden;
	bool clickable;
	bool mustresize;

	frame() : parent( 0 ), movable( false ), hidden( false ), clickable( false ), mustresize( false )
	{
		x = 0.0f;
		y = 0.0f;
		width = 0.0f;
		height = 0.0f;
	}

	frame( float pX, float pY, float w, float h ) : parent( 0 ), movable( false ), hidden( false ), clickable( false ), mustresize( false )
	{
		x = pX;
		y = pY;
		width = w;
		height = h;
	}
  
  virtual ~frame()
  {
    for(std::vector<frame*>::iterator it = children.begin(); it != children.end(); it++)
    {
      if( *it )
      {
        delete *it;
        *it = NULL;
      }
    }
  }

	void addChild( frame * );
	virtual void render();
	virtual frame *processLeftClick( float mx, float my );
	virtual bool processLeftDrag( float mx, float my, float xChange, float yChange );
	virtual	void processUnclick() { }
	virtual bool processRightClick( float mx, float my );
	virtual bool processKey( char key, bool shift, bool alt, bool ctrl );
	virtual void resize() { }
	void getOffset( float &xOff, float &yOff );

	bool IsHit( float pX, float pY )
	{
		return this->x < pX && this->x + this->width > pX && this->y < pY && this->y + this->height > pY;
	}
};

#endif
