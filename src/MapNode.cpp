#include "MapNode.h"
#include "MapChunk.h"
#include "MapTile.h"
#include "world.h"

void MapNode::draw()
{
	/*if (!gWorld->frustum.intersects(vmin,vmax)) return;
	for (int i=0; i<4; i++) children[i]->draw();*/
}

void MapNode::drawSelect()
{
	//if (!gWorld->frustum.intersects(vmin,vmax)) return;
	//for (int i=0; i<4; i++) children[i]->drawSelect();
}

void MapNode::drawColor()
{
	if( !gWorld->frustum.intersects( vmin, vmax ) )
		return;
	
	for( int i = 0; i < 4; i++ )
		children[i]->drawColor( );
}

void MapNode::setup( MapTile *t )
{
	vmin = Vec3D( 9999999.0f, 9999999.0f, 9999999.0f );
	vmax = Vec3D( -9999999.0f, -9999999.0f, -9999999.0f );
	mt = t;
	if( size == 2 )
	{
		// children will be mapchunks
		children[0] = mt->chunks[py][px];
		children[1] = mt->chunks[py][px+1];
		children[2] = mt->chunks[py+1][px];
		children[3] = mt->chunks[py+1][px+1];
	} 
	else 
	{
		int half = size / 2;
		children[0] = new MapNode( px, py, half );
		children[1] = new MapNode( px + half, py, half );
		children[2] = new MapNode( px, py + half, half );
		children[3] = new MapNode( px + half, py + half, half );
		for( int i = 0; i < 4; i++ )
			children[i]->setup(mt);
	}
	for( int i = 0; i < 4; i++ )
	{
		if( children[i]->vmin.x < vmin.x ) 
			vmin.x = children[i]->vmin.x;
		if( children[i]->vmin.y < vmin.y ) 
			vmin.y = children[i]->vmin.y;
		if( children[i]->vmin.z < vmin.z ) 
			vmin.z = children[i]->vmin.z;
		if( children[i]->vmax.x > vmax.x ) 
			vmax.x = children[i]->vmax.x;
		if( children[i]->vmax.y > vmax.y ) 
			vmax.y = children[i]->vmax.y;
		if( children[i]->vmax.z > vmax.z ) 
			vmax.z = children[i]->vmax.z;
	}
}

void MapNode::cleanup( )
{
	if( size > 2 )
	{
		for( int i = 0; i < 4; i++ )
		{
			children[i]->cleanup( );
			delete children[i];
		}
	}
}
