#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include <noggit/Vec3D.h>

class Model;
class World;
struct ENTRY_MDDF;
namespace noggit
{
  namespace mpq
  {
    class file;
  }
}

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
  ModelInstance(World*);
  explicit ModelInstance( World*, Model *m );
  explicit ModelInstance( World*, Model *m, noggit::mpq::file* f );
  explicit ModelInstance( World*, Model *m, ENTRY_MDDF *d );
  void init2( Model *m, noggit::mpq::file* f );
  void draw (bool draw_fog);
  void drawMapTile();
//  void drawHighlight();
  void drawSelect ();
  void draw2 (const Vec3D& ofs, const float rot );
  void draw2Select (const Vec3D& ofs, const float rot );

  void resetDirection();

private:
  World* _world;
};

#endif // MODELINSTANCE_H
