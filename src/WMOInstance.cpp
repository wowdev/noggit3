#include "WMOInstance.h"

#include "Log.h"
#include "MapHeaders.h"
#include "WMO.h" // WMO
#include "World.h" // gWorld
#include "Misc.h" // checkinside

WMOInstance::WMOInstance()
  : mSelectionID( SelectionNames.add( this ) )
  , uidLock(false)
{
}

WMOInstance::WMOInstance( WMO* _wmo, MPQFile* _file )
  : wmo( _wmo )
  , mSelectionID( SelectionNames.add( this ) )
  , uidLock(false)
{
  _file->read( &mUniqueID, 4 );
  _file->read( &pos, 12 );
  _file->read( &dir, 12 );
  _file->read( &extents[0], 12 );
  _file->read( &extents[1], 12 );
  _file->read( &mFlags, 2 );
  _file->read( &doodadset, 2 );
  _file->read( &mNameset, 2 );
  _file->read( &mUnknown, 2 );
}

WMOInstance::WMOInstance( WMO* _wmo, ENTRY_MODF* d )
  : wmo( _wmo )
  , pos( Vec3D( d->pos[0], d->pos[1], d->pos[2] ) )
  , dir( Vec3D( d->rot[0], d->rot[1], d->rot[2] ) )
  , mUniqueID( d->uniqueID ), mFlags( d->flags )
  , mUnknown( d->unknown ), mNameset( d->nameSet )
  , doodadset( d->doodadSet )
  , mSelectionID( SelectionNames.add( this ) )
  , uidLock(false)
{
  extents[0] = Vec3D( d->extents[0][0], d->extents[0][1], d->extents[0][2] );
  extents[1] = Vec3D( d->extents[1][0], d->extents[1][1], d->extents[1][2] );
}

WMOInstance::WMOInstance( WMO* _wmo )
  : wmo( _wmo )
  , pos( Vec3D( 0.0f, 0.0f, 0.0f ) )
  , dir( Vec3D( 0.0f, 0.0f, 0.0f ) )
  , mUniqueID( 0 )
  , mFlags( 0 )
  , mUnknown( 0 )
  , mNameset( 0 )
  , doodadset( 0 )
  , mSelectionID( SelectionNames.add( this ) )
  , uidLock(false)
{
}

WMOInstance::~WMOInstance()
{
  SelectionNames.del( mSelectionID );
}

void DrawABox( Vec3D pMin, Vec3D pMax, Vec4D pColor, float pLineWidth );

void WMOInstance::draw()
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

  if( gWorld->IsSelection( eEntry_WMO ) && gWorld->GetCurrentSelection()->data.wmo->mUniqueID == this->mUniqueID )
  {
    glDisable( GL_LIGHTING );

    glDisable( GL_COLOR_MATERIAL );
    glActiveTexture( GL_TEXTURE0 );
    glDisable( GL_TEXTURE_2D );
    glActiveTexture( GL_TEXTURE1 );
    glDisable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    DrawABox(extents[0], extents[1], Vec4D(0.0f, 1.0f, 0.0f, 1.0f ), 1.0f);

    glActiveTexture( GL_TEXTURE1 );
    glDisable( GL_TEXTURE_2D );
    glActiveTexture( GL_TEXTURE0 );
    glEnable( GL_TEXTURE_2D );

    glEnable( GL_LIGHTING );
  }
}

void WMOInstance::recalcExtents()
{
  Vec3D min(100000, 100000, 100000);
  Vec3D max(-100000, -100000, -100000);
  Matrix rot(Matrix::newTranslation(pos)
             * Matrix::newRotate((dir.y - 90.0f) * PI/180.0f, Vec3D(0, 1, 0))
             * Matrix::newRotate(dir.x * -1.0f * PI/180.0f, Vec3D(0, 0, 1))
             * Matrix::newRotate(dir.z * PI/180.0f, Vec3D(1, 0, 0))
             );

  Vec3D *bounds = new Vec3D[8*(wmo->nGroups+1)];
  Vec3D *ptr = bounds;
  Vec3D wmoMin(wmo->extents[0].x, wmo->extents[0].z, -wmo->extents[0].y);
  Vec3D wmoMax(wmo->extents[1].x, wmo->extents[1].z, -wmo->extents[1].y);

  *ptr++ = rot * Vec3D(wmoMax.x, wmoMax.y, wmoMin.z);
  *ptr++ = rot * Vec3D(wmoMin.x, wmoMax.y, wmoMin.z);
  *ptr++ = rot * Vec3D(wmoMin.x, wmoMin.y, wmoMin.z);
  *ptr++ = rot * Vec3D(wmoMax.x, wmoMin.y, wmoMin.z);
  *ptr++ = rot * Vec3D(wmoMax.x, wmoMin.y, wmoMax.z);
  *ptr++ = rot * Vec3D(wmoMax.x, wmoMax.y, wmoMax.z);
  *ptr++ = rot * Vec3D(wmoMin.x, wmoMax.y, wmoMax.z);
  *ptr++ = rot * Vec3D(wmoMin.x, wmoMin.y, wmoMax.z);

  for (int i = 0; i < wmo->nGroups; ++i)
  {
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMax.y, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, wmo->groups[i].BoundingBoxMin.y, wmo->groups[i].BoundingBoxMax.z);
  }


  for (int i = 0; i < 8*(wmo->nGroups+1); ++i)
  {
    if(bounds[i].x < min.x) min.x = bounds[i].x;
    if(bounds[i].y < min.y) min.y = bounds[i].y;
    if(bounds[i].z < min.z) min.z = bounds[i].z;

    if(bounds[i].x > max.x) max.x = bounds[i].x;
    if(bounds[i].y > max.y) max.y = bounds[i].y;
    if(bounds[i].z > max.z) max.z = bounds[i].z;
  }

  extents[0] = min;
  extents[1] = max;

  delete bounds;
}

bool WMOInstance::isInsideTile(Vec3D lTileExtents[2])
{
  Matrix rot(Matrix::newTranslation(pos)
             * Matrix::newRotate((dir.y - 90.0f) * PI/180.0f, Vec3D(0, 1, 0))
             * Matrix::newRotate(dir.x * -1.0f * PI/180.0f, Vec3D(0, 0, 1))
             * Matrix::newRotate(dir.z * PI/180.0f, Vec3D(1, 0, 0))
             );

  Vec3D *bounds = new Vec3D[4*(wmo->nGroups)+5];
  Vec3D *ptr = bounds;
  Vec3D wmoMin(wmo->extents[0].x, wmo->extents[0].z, -wmo->extents[0].y);
  Vec3D wmoMax(wmo->extents[1].x, wmo->extents[1].z, -wmo->extents[1].y);

  *ptr++ = pos;

  *ptr++ = rot * Vec3D(wmoMax.x, 0, wmoMax.z);
  *ptr++ = rot * Vec3D(wmoMax.x, 0, wmoMin.z);
  *ptr++ = rot * Vec3D(wmoMin.x, 0, wmoMax.z);
  *ptr++ = rot * Vec3D(wmoMin.x, 0, wmoMin.z);

  for (int i = 0; i < wmo->nGroups; ++i)
  {
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, 0, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMax.x, 0, wmo->groups[i].BoundingBoxMin.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, 0, wmo->groups[i].BoundingBoxMax.z);
    *ptr++ = rot * Vec3D(wmo->groups[i].BoundingBoxMin.x, 0, wmo->groups[i].BoundingBoxMin.z);
  }


  for (int i = 0; i < 4*(wmo->nGroups)+5; ++i)
  {
    if(pointInside(bounds[i], lTileExtents))
    {
      delete bounds;
      return true;
    }
  }

  delete bounds;
  return false;
}

bool WMOInstance::isInsideChunk(Vec3D lTileExtents[2])
{
  if(isInsideTile(lTileExtents))
    return true;

  //maybe model > chunk || tile
  recalcExtents();

  for (int i = 0; i < 2; ++i)
    if(pointInside(lTileExtents[i], extents))
      return true;

  return false;
}

void WMOInstance::drawSelect()
{
  glPushMatrix();

  glTranslatef( pos.x, pos.y, pos.z );

  const float roty = dir.y - 90.0f;

  glRotatef( roty, 0.0f, 1.0f, 0.0f );
  glRotatef( -dir.x, 0.0f, 0.0f, 1.0f );
  glRotatef( dir.z, 1.0f, 0.0f, 0.0f );

  mSelectionID = SelectionNames.add( this );
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



void WMOInstance::lockUID()
{
  uidLock = true;
}

void WMOInstance::unlockUID()
{
  uidLock = false;
}

bool WMOInstance::hasUIDLock()
{
  return uidLock;
}
