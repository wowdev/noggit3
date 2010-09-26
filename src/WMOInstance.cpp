#include "WMOInstance.h"

#include "wmo.h" // WMO
#include "world.h" // gWorld

WMOInstance::WMOInstance( WMO *_wmo, MPQFile &f ) : wmo (_wmo)
{
	nameID = 0xFFFFFFFF;

	//! \todo  Where is the name ID? Oo What is up with this ctor?
    f.read( &id, 4 );
	f.read( (float*)pos, 12 );
	f.read( (float*)dir, 12 );
	f.read( (float*)extents[0], 12 );
	f.read( (float*)extents[1], 12 );
	f.read( &mFlags, 2 );
	f.read( &doodadset, 2 );
	f.read( &mNameset, 2 );
	f.read( &mUnknown, 2 );
	
	//! \todo  This really seems wrong. Where is this used and why doesn't this crash?
	wmoID = id;
}

WMOInstance::WMOInstance( WMO *_wmo, ENTRY_MODF *d ) : wmo (_wmo)
{
	nameID = 0xFFFFFFFF;

	wmoID = d->nameID;
	id = d->uniqueID;
	pos = Vec3D( d->pos[0], d->pos[1], d->pos[2] );
	dir = Vec3D( d->rot[0], d->rot[1], d->rot[2] );
	extents[0] = Vec3D( d->extents[0][0], d->extents[0][1], d->extents[0][2] );
	extents[1] = Vec3D( d->extents[1][0], d->extents[1][1], d->extents[1][2] );
	mFlags = d->flags;
	doodadset = d->doodadSet;
	mNameset = d->nameSet;
	mUnknown = d->unknown;
}

WMOInstance::WMOInstance(WMO *_wmo) : wmo (_wmo)
{
	nameID = 0xFFFFFFFF;
}


void WMOInstance::draw()
{
	if( ids.find( id ) != ids.end() ) 
		return;

	ids.insert( id );

	glPushMatrix();
	glTranslatef( pos.x, pos.y, pos.z );

	float rot = 90.0f - dir.y;

	//! \todo  replace this with a single transform matrix calculated at load time

	glRotatef( dir.y - 90.0f, 0.0f, 1.0f, 0.0f );
	glRotatef( -dir.x, 0.0f, 0.0f, 1.0f );
	glRotatef( dir.z, 1.0f, 0.0f, 0.0f );

	if( gWorld->IsSelection( eEntry_WMO ) && gWorld->GetCurrentSelection()->data.wmo->id == this->id )
		wmo->draw( doodadset, pos, rot, true, true, true );
	else
		wmo->draw( doodadset, pos, rot, false, false, false );

	glPopMatrix();
}

void WMOInstance::drawSelect()
{
	if (ids.find(id) != ids.end()) return;
	ids.insert(id);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);

	float rot = -90.0f + dir.y;

	//! \todo  replace this with a single transform matrix calculated at load time

	glRotatef(dir.y - 90.0f, 0, 1, 0);
	glRotatef(-dir.x, 0, 0, 1);
	glRotatef(dir.z, 1, 0, 0);

	if( nameID == 0xFFFFFFFF )
		nameID = SelectionNames.add( this );
	glPushName(nameID);
	wmo->drawSelect(doodadset,pos,-rot);
	glPopName();

	glPopMatrix();
}


/*
void WMOInstance::drawPortals()
{
	if (ids.find(id) != ids.end()) return;
	ids.insert(id);

	glPushMatrix();
	glTranslatef(pos.x, pos.y, pos.z);

	glRotatef(dir.y - 90.0f, 0, 1, 0);
	glRotatef(-dir.x, 0, 0, 1);
	glRotatef(dir.z, 1, 0, 0);

	wmo->drawPortals();
	glPopMatrix();
}
*/

void WMOInstance::reset()
{
    ids.clear();
}

void WMOInstance::resetPosition(){
	pos.x=0;
	pos.y=0;
	pos.z=0;
}
void WMOInstance::resetDirection(){
	dir.x=0;
	//dir.y=0; only reset incline
	dir.z=0;
}

std::set<int> WMOInstance::ids;

WMOInstance::~WMOInstance()
{
	if( nameID != 0xFFFFFFFF )
	{
		SelectionNames.del( nameID );
		nameID = 0xFFFFFFFF;
	}
}

