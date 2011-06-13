#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include "Vec3D.h" // Vec3D
#include "MPQ.h" // MPQFile
#include "MapHeaders.h" // ENTRY_MDDF

class Model;

class ModelInstance
{
public:
  Model *model;

  unsigned int nameID;

  Vec3D pos, dir;

  //! \todo  Get this out and do somehow else.
  unsigned int d1;

  float w, sc;

  Vec3D ldir;
  Vec3D lcol;

  ~ModelInstance();
  ModelInstance();
  explicit ModelInstance( Model *m );
  explicit ModelInstance( Model *m, MPQFile* f );
  explicit ModelInstance( Model *m, ENTRY_MDDF *d );
  void init2( Model *m, MPQFile* f );
  void draw();
  void drawMapTile();
//  void drawHighlight();
  void drawSelect();
  void draw2( const Vec3D& ofs, const float rot );
  void draw2Select( const Vec3D& ofs, const float rot );

  void resetDirection();
};

#endif // MODELINSTANCE_H
