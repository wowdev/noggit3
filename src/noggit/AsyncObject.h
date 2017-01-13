// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class AsyncObject
{
protected:
	bool finished;
public:
	virtual ~AsyncObject() {}

	virtual bool finishedLoading() const
	{
		return finished;
	}
	virtual void finishLoading() = 0;
};
