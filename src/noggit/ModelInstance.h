#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include <math/vector_3d.h>

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

  ::math::vector_3d pos, dir;

  //! \todo  Get this out and do somehow else.
  unsigned int d1;

  float w, sc;

  ::math::vector_3d ldir;
  ::math::vector_3d lcol;

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
  void draw2 (const ::math::vector_3d& ofs, const float rot );
  void draw2Select (const ::math::vector_3d& ofs, const float rot );

  void resetDirection();

private:
  World* _world;
};

#endif // MODELINSTANCE_H
