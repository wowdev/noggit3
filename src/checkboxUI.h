#ifndef __CHECKBOXUI_H
#define __CHECKBOXUI_H

class checkboxUI;

#include "video.h"
#include "frame.h"
#include "textureUI.h"
#include "textUI.h"
#include "ToggleGroup.h"

class checkboxUI : public frame
{
protected:
	textureUI	*box;
	textureUI	*check;
	textUI		*text;
	bool		checked;
	int			id;
	void (*clickFunc)( bool, int );

	ToggleGroup * mToggleGroup;
public:
	checkboxUI( float, float, const std::string& );
	checkboxUI( float, float, const std::string&, ToggleGroup *, int );
	checkboxUI( float xPos, float yPos, const std::string& pText, void (*pClickFunc)(bool,int), int pClickFuncParameter );
	void SetToggleGroup( ToggleGroup * , int );
	void setText( const std::string& );
	void setState( bool );
	bool getState();
	void setClickFunc( void (*f)( bool, int ),int );

	frame *processLeftClick( float, float );
};
#endif
