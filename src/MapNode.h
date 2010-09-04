#ifndef MAPNODE_H
#define MAPNODE_H

#include "vec3d.h"

class MapTile;

class MapNode 
{
public:
	MapNode(int x, int y, int s):px(x),py(y),size(s) {}

	int px, py, size;

	Vec3D vmin, vmax, vcenter;

	MapNode *children[4];
	MapTile *mt;

	virtual void draw();
	virtual void drawSelect();
	virtual void drawColor();
	void setup(MapTile *t);
	void cleanup();

};

#endif // MAPNODE_H
