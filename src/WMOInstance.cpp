#include "WMOInstance.h"

#include "wmo.h" // WMO
#include "world.h" // gWorld

WMOInstance::WMOInstance( WMO* _wmo, MPQFile& _file ) : wmo( _wmo ), mSelectionID( SelectionNames.add( this ) )
{
	_file.read( &mUniqueID, 4 );
	_file.read( (float*)pos, 12 );
	_file.read( (float*)dir, 12 );
	_file.read( (float*)extents[0], 12 );
	_file.read( (float*)extents[1], 12 );
	_file.read( &mFlags, 2 );
	_file.read( &doodadset, 2 );
	_file.read( &mNameset, 2 );
	_file.read( &mUnknown, 2 );
}

WMOInstance::WMOInstance( WMO* _wmo, ENTRY_MODF* d ) : wmo( _wmo ), pos( Vec3D( d->pos[0], d->pos[1], d->pos[2] ) ), dir( Vec3D( d->rot[0], d->rot[1], d->rot[2] ) ), mUniqueID( d->uniqueID ), mFlags( d->flags ), mUnknown( d->unknown ), mNameset( d->nameSet ), doodadset( d->doodadSet ), mSelectionID( SelectionNames.add( this ) )
{
	extents[0] = Vec3D( d->extents[0][0], d->extents[0][1], d->extents[0][2] );
	extents[1] = Vec3D( d->extents[1][0], d->extents[1][1], d->extents[1][2] );
}

WMOInstance::WMOInstance( WMO* _wmo ) : wmo( _wmo ), pos( Vec3D( 0.0f, 0.0f, 0.0f ) ), dir( Vec3D( 0.0f, 0.0f, 0.0f ) ), mUniqueID( 0 ), mFlags( 0 ), mUnknown( 0 ), mNameset( 0 ), doodadset( 0 ), mSelectionID( SelectionNames.add( this ) )
{
}

void WMOInstance::draw() const
{
	glPushMatrix();
	glTranslatef( pos.x, pos.y, pos.z );
	
	const float roty = dir.y - 90.0f;
	
	glRotatef( roty, 0.0f, 1.0f, 0.0f );
	glRotatef( -dir.x, 0.0f, 0.0f, 1.0f );
	glRotatef( dir.z, 1.0f, 0.0f, 0.0f );

	if( gWorld->IsSelection( eEntry_WMO ) && gWorld->GetCurrentSelection()->data.wmo->mUniqueID == this->mUniqueID )
		wmo->draw( doodadset, pos, roty, true, true, true );
	else
		wmo->draw( doodadset, pos, roty, false, false, false );

	glPopMatrix();
}

void WMOInstance::drawSelect() const
{
	glPushMatrix();
	
	glTranslatef( pos.x, pos.y, pos.z );
	
	const float roty = dir.y - 90.0f;

	glRotatef( roty, 0.0f, 1.0f, 0.0f );
	glRotatef( -dir.x, 0.0f, 0.0f, 1.0f );
	glRotatef( dir.z, 1.0f, 0.0f, 0.0f );

	glPushName( mSelectionID );
	
	wmo->drawSelect( doodadset, pos, -roty );
	
	glPopName();

	glPopMatrix();
}

/*void WMOInstance::drawPortals()
{
	glPushMatrix();
 
	glTranslatef( pos.x, pos.y, pos.z );

	const float roty = dir.y - 90.0f;
 
	glRotatef( roty, 0.0f, 1.0f, 0.0f );
	glRotatef( -dir.x, 0.0f, 0.0f, 1.0f );
	glRotatef( dir.z, 1.0f, 0.0f, 0.0f );

	wmo->drawPortals();
 
	glPopMatrix();
}*/

void WMOInstance::resetDirection()
{
	dir = Vec3D( 0.0f, dir.y, 0.0f );
}

WMOInstance::~WMOInstance()
{
	SelectionNames.del( mSelectionID );
}
