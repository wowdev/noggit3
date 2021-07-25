// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Misc.h>
#include <noggit/Particle.h>
#include <opengl/context.hpp>
#include <opengl/shader.hpp>

#include <list>

static const unsigned int MAX_PARTICLES = 10000;

template<class T>
T lifeRamp(float life, float mid, const T &a, const T &b, const T &c)
{
  if (life <= mid) return math::interpolation::linear(life / mid, a, b);
  else return math::interpolation::linear((life - mid) / (1.0f - mid), b, c);
}

ParticleSystem::ParticleSystem(Model* model_, const MPQFile& f, const ModelParticleEmitterDef &mta, int *globals)
  : model (model_)
  , emitter_type(mta.EmitterType)
  , emitter ( mta.EmitterType == 1 ? std::unique_ptr<ParticleEmitter> (std::make_unique<PlaneParticleEmitter>())
            : mta.EmitterType == 2 ? std::unique_ptr<ParticleEmitter> (std::make_unique<SphereParticleEmitter>())
            : throw std::logic_error ("unimplemented emitter type: " + std::to_string(mta.EmitterType))
            )
  , speed (mta.EmissionSpeed, f, globals)
  , variation (mta.SpeedVariation, f, globals)
  , spread (mta.VerticalRange, f, globals)
  , lat (mta.HorizontalRange, f, globals)
  , gravity (mta.Gravity, f, globals)
  , lifespan (mta.Lifespan, f, globals)
  , rate (mta.EmissionRate, f, globals)
  , areal (mta.EmissionAreaLength, f, globals)
  , areaw (mta.EmissionAreaWidth, f, globals)
  , deacceleration (mta.Gravity2, f, globals)
  , enabled (mta.en, f, globals)
  , mid (0.5)
  , slowdown (mta.p.slowdown)
  , pos (fixCoordSystem(mta.pos))
  , _texture_id (mta.texture)
  , blend (mta.blend)
  , order (mta.ParticleType > 0 ? -1 : 0)
  , type (mta.ParticleType)
  , manim (0)
  , mtime (0)
  , rows (mta.rows)
  , cols (mta.cols)
  , billboard (!(mta.flags & 4096))
  , rem(0)
  , parent (&model->bones[mta.bone])
  , flags(mta.flags)
  , tofs (misc::frand())
{
  math::vector_3d colors2[3];
  memcpy(colors2, f.getBuffer() + mta.p.colors.ofsKeys, sizeof(math::vector_3d) * 3);
  for (size_t i = 0; i<3; ++i) {
    float opacity = *reinterpret_cast<int16_t const*>(f.getBuffer() + mta.p.opacity.ofsKeys + i * 2);
    colors[i] = math::vector_4d(colors2[i].x / 255.0f, colors2[i].y / 255.0f, colors2[i].z / 255.0f, opacity / 32767.0f);
    sizes[i] = (*reinterpret_cast<float const*>(f.getBuffer() + mta.p.sizes.ofsKeys + i * 4))*mta.p.scales[i];
  }

  //transform = mta.flags & 1024;

  // init tiles
  for (int i = 0; i<rows*cols; ++i) {
    TexCoordSet tc;
    initTile(tc.tc, i);
    tiles.push_back(tc);
  }
}

ParticleSystem::ParticleSystem(ParticleSystem const& other)
  : model(other.model)
  , emitter_type(other.emitter_type)
  , emitter( emitter_type == 1 ? std::unique_ptr<ParticleEmitter>(std::make_unique<PlaneParticleEmitter>())
           : emitter_type == 2 ? std::unique_ptr<ParticleEmitter>(std::make_unique<SphereParticleEmitter>())
           : throw std::logic_error("unimplemented emitter type: " + std::to_string(emitter_type))
           )
  , speed(other.speed)
  , variation(other.variation)
  , spread(other.spread)
  , lat(other.lat)
  , gravity(other.gravity)
  , lifespan(other.lifespan)
  , rate(other.rate)
  , areal(other.areal)
  , areaw(other.areaw)
  , deacceleration(other.deacceleration)
  , enabled(other.enabled)
  , colors(other.colors)
  , sizes(other.sizes)
  , mid(other.mid)
  , slowdown(other.slowdown)
  , pos(other.pos)
  , _texture_id(other._texture_id)
  , particles(other.particles)
  , blend(other.blend)
  , order(other.order)
  , type(other.type)
  , manim(other.manim)
  , mtime(other.mtime)
  , manimtime(other.manimtime)
  , rows(other.rows)
  , cols(other.cols)
  , tiles(other.tiles)
  , billboard(other.billboard)
  , rem(other.rem)
  , parent(other.parent)
  , flags(other.flags)
  , tofs(other.tofs)
{

}

ParticleSystem::ParticleSystem(ParticleSystem&& other)
  : model(other.model)
  , emitter_type(other.emitter_type)
  , emitter(std::move(other.emitter))
  , speed(other.speed)
  , variation(other.variation)
  , spread(other.spread)
  , lat(other.lat)
  , gravity(other.gravity)
  , lifespan(other.lifespan)
  , rate(other.rate)
  , areal(other.areal)
  , areaw(other.areaw)
  , deacceleration(other.deacceleration)
  , enabled(other.enabled)
  , colors(other.colors)
  , sizes(other.sizes)
  , mid(other.mid)
  , slowdown(other.slowdown)
  , pos(other.pos)
  , _texture_id(other._texture_id)
  , particles(other.particles)
  , blend(other.blend)
  , order(other.order)
  , type(other.type)
  , manim(other.manim)
  , mtime(other.mtime)
  , manimtime(other.manimtime)
  , rows(other.rows)
  , cols(other.cols)
  , tiles(other.tiles)
  , billboard(other.billboard)
  , rem(other.rem)
  , parent(other.parent)
  , flags(other.flags)
  , tofs(other.tofs)
{

}

void ParticleSystem::initTile(math::vector_2d *tc, int num)
{
  math::vector_2d otc[4];
  math::vector_2d a, b;
  int x = num % cols;
  int y = num / cols;
  a.x = x * (1.0f / cols);
  b.x = (x + 1) * (1.0f / cols);
  a.y = y * (1.0f / rows);
  b.y = (y + 1) * (1.0f / rows);

  otc[0] = a;
  otc[2] = b;
  otc[1].x = b.x;
  otc[1].y = a.y;
  otc[3].x = a.x;
  otc[3].y = b.y;

  for (int i = 0; i<4; ++i) {
    tc[(i + 4 - order) & 3] = otc[i];
  }
}


void ParticleSystem::update(float dt)
{
  float grav = gravity.getValue(manim, mtime, manimtime);
  float deaccel = deacceleration.getValue(manim, mtime, manimtime);

  // spawn new particles
  if (emitter) {
    float frate = rate.getValue(manim, mtime, manimtime);
    float flife = 1.0f;
    flife = lifespan.getValue(manim, mtime, manimtime);

    float ftospawn = (dt * frate / flife) + rem;
    if (ftospawn < 1.0f) {
      rem = ftospawn;
      if (rem<0)
        rem = 0;
    }
    else {
      int tospawn = (int)ftospawn;

      if ((tospawn + particles.size()) > MAX_PARTICLES) // Error check to prevent the program from trying to load insane amounts of particles.
        tospawn = particles.size() - MAX_PARTICLES;

      rem = ftospawn - static_cast<float>(tospawn);


      float w = areal.getValue(manim, mtime, manimtime) * 0.5f;
      float l = areaw.getValue(manim, mtime, manimtime) * 0.5f;
      float spd = speed.getValue(manim, mtime, manimtime);
      float var = variation.getValue(manim, mtime, manimtime);
      float spr = spread.getValue(manim, mtime, manimtime);
      float spr2 = lat.getValue(manim, mtime, manimtime);
      bool en = true;
      if (enabled.uses(manim))
        en = enabled.getValue(manim, mtime, manimtime) != 0;

      //rem = 0;
      if (en) {
        for (int i = 0; i<tospawn; ++i) {
          Particle p = emitter->newParticle(this, manim, mtime, manimtime, w, l, spd, var, spr, spr2);
          // sanity check:
          //if (particles.size() < MAX_PARTICLES) // No need to check this every loop iteration. Already checked above.
          particles.push_back(p);
        }
      }
    }
  }

  float mspeed = 1.0f;

  for (ParticleList::iterator it = particles.begin(); it != particles.end();) {
    Particle &p = *it;
    p.speed += p.down * grav * dt - p.dir * deaccel * dt;

    if (slowdown>0) {
      mspeed = expf(-1.0f * slowdown * p.life);
    }
    p.pos += p.speed * mspeed * dt;

    p.life += dt;
    float rlife = p.life / p.maxlife;
    // calculate size and color based on lifetime
    p.size = lifeRamp<float>(rlife, mid, sizes[0], sizes[1], sizes[2]);
    p.color = lifeRamp<math::vector_4d>(rlife, mid, colors[0], colors[1], colors[2]);

    // kill off old particles
    if (rlife >= 1.0f)
    {
      it = particles.erase (it);
    }
    else
    {
      ++it;
    }
  }
}

void ParticleSystem::setup(int anim, int time, int animtime)
{
  manim = anim;
  mtime = time;
  manimtime = animtime;
}

void ParticleSystem::draw( math::matrix_4x4 const& model_view
                         , opengl::scoped::use_program& shader
                         , GLuint const& transform_vbo
                         , int instances_count
)
{
  if (!_uploaded)
  {
    upload();
  }

  // setup blend mode
  float alpha_test = -1.f;

  switch (blend) 
  {
  case 0:
    gl.disable(GL_BLEND);
    break;
  case 1:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_COLOR, GL_ONE);
    break;
  case 2:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  case 3:
    gl.disable(GL_BLEND);
    alpha_test = 0.5f;
    break;
  case 4:
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE);
    break;
  }

  model->_textures[_texture_id]->bind();

  math::vector_3d vRight(1, 0, 0);
  math::vector_3d vUp(0, 1, 0);

  // position stuff
  const float f = 1;//0.707106781f; // sqrt(2)/2
  math::vector_3d bv0 = math::vector_3d(-f, +f, 0);
  math::vector_3d bv1 = math::vector_3d(+f, +f, 0);

  std::vector<std::uint16_t> indices;
  std::vector<math::vector_3d> vertices;
  std::vector<math::vector_3d> offsets;
  std::vector<math::vector_4d> colors_data;
  std::vector<math::vector_2d> texcoords;

  std::uint16_t indice = 0;

  if (billboard) 
  {
    vRight = math::vector_3d(model_view[0], model_view[4], model_view[8]);
    vUp = math::vector_3d(model_view[1], model_view[5], model_view[9]); // Spherical billboarding
    //vUp = math::vector_3d(0,1,0); // Cylindrical billboarding
  }

  auto add_quad_indices([&indices] (std::uint16_t& start)
  {
    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);

    indices.push_back(start + 2);
    indices.push_back(start + 3);
    indices.push_back(start + 0);

    start += 4;
  });

  /*
  * type:
  * 0   "normal" particle
  * 1  large quad from the particle's origin to its position (used in Moonwell water effects)
  * 2  seems to be the same as 0 (found some in the Deeprun Tram blinky-lights-sign thing)
  */
  if (type == 0 || type == 2) 
  {
    //! \todo figure out type 2 (deeprun tram subway sign)
    // - doesn't seem to be any different from 0 -_-
    // regular particles

    if (billboard) 
    {
      //! \todo per-particle rotation in a non-expensive way?? :|
      for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) 
      {
        if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
        {
          break;
        }

        const float size = it->size;// / 2;

        texcoords.push_back(tiles[it->tile].tc[0]);
        vertices.push_back(it->pos);
        offsets.push_back(-(vRight + vUp) * size);
        colors_data.push_back(it->color);

        texcoords.push_back(tiles[it->tile].tc[1]);
        vertices.push_back(it->pos);
        offsets.push_back((vRight - vUp) * size);
        colors_data.push_back(it->color);

        texcoords.push_back(tiles[it->tile].tc[2]);
        vertices.push_back(it->pos);
        offsets.push_back((vRight + vUp) * size);
        colors_data.push_back(it->color);

        texcoords.push_back(tiles[it->tile].tc[3]);
        vertices.push_back(it->pos);
        offsets.push_back(-(vRight - vUp) * size);
        colors_data.push_back(it->color);

        add_quad_indices(indice);
      }
    }
    else 
    {
      for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) 
      {
        if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
        {
          break;
        }

        texcoords.push_back(tiles[it->tile].tc[0]);
        vertices.push_back(it->pos + it->corners[0] * it->size);
        colors_data.push_back(it->color);

        texcoords.push_back(tiles[it->tile].tc[1]);
        vertices.push_back(it->pos + it->corners[1] * it->size);
        colors_data.push_back(it->color);

        texcoords.push_back(tiles[it->tile].tc[2]);
        vertices.push_back(it->pos + it->corners[2] * it->size);
        colors_data.push_back(it->color);

        texcoords.push_back(tiles[it->tile].tc[3]);
        vertices.push_back(it->pos + it->corners[3] * it->size);
        colors_data.push_back(it->color);

        add_quad_indices(indice);
      }
    }
  }  
  else if (type == 1) 
  { // Sphere particles
    // particles from origin to position
    /*
    bv0 = mbb * math::vector_3d(0,-1.0f,0);
    bv1 = mbb * math::vector_3d(0,+1.0f,0);


    bv0 = mbb * math::vector_3d(-1.0f,0,0);
    bv1 = mbb * math::vector_3d(1.0f,0,0);
    */

    for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) 
    {
      if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
      {
        break;
      }

      texcoords.push_back(tiles[it->tile].tc[0]);
      vertices.push_back(it->pos + bv0 * it->size);
      colors_data.push_back(it->color);

      texcoords.push_back(tiles[it->tile].tc[1]);
      vertices.push_back(it->pos + bv1 * it->size);
      colors_data.push_back(it->color);

      texcoords.push_back(tiles[it->tile].tc[2]);
      vertices.push_back(it->origin + bv1 * it->size);
      colors_data.push_back(it->color);

      texcoords.push_back(tiles[it->tile].tc[3]);
      vertices.push_back(it->origin + bv0 * it->size);
      colors_data.push_back(it->color);

      add_quad_indices(indice);
    }
  }

  gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>(_vertices_vbo, vertices, GL_STREAM_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER, math::vector_4d>(_colors_vbo, colors_data, GL_STREAM_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER, math::vector_2d>(_texcoord_vbo, texcoords, GL_STREAM_DRAW);
  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_indices_vbo, indices, GL_STREAM_DRAW);

  shader.uniform("alpha_test", alpha_test);
  shader.uniform("billboard", (int)billboard);

  opengl::scoped::vao_binder const _ (_vao);

  shader.attrib(_, "position", _vertices_vbo, 3, GL_FLOAT, GL_FALSE, 0, 0);

  if(billboard)
  {
    // \todo duplicate bind
    gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>(_offsets_vbo, offsets, GL_STREAM_DRAW);
    shader.attrib(_, "offset", _offsets_vbo, 3, GL_FLOAT, GL_FALSE, 0, 0);
  }

  shader.attrib(_, "uv", _texcoord_vbo, 2, GL_FLOAT, GL_FALSE, 0, 0);
  shader.attrib(_, "color", _colors_vbo, 4, GL_FLOAT, GL_FALSE, 0, 0);
  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const transform_binder (transform_vbo);
    shader.attrib(_, "transform", opengl::array_buffer_is_already_bound{}, static_cast<math::matrix_4x4*> (nullptr), 1);
  }

  gl.drawElementsInstanced(GL_TRIANGLES, indices.size(), instances_count, GL_UNSIGNED_SHORT, _indices_vbo);
}

void ParticleSystem::upload()
{
  _vertex_array.upload();
  _buffers.upload();
  _uploaded = true;
}

namespace
{
  //Generates the rotation matrix based on spread
  math::matrix_4x4 CalcSpreadMatrix(float Spread1, float Spread2, float w, float l)
  {
    int i, j;
    float a[2], c[2], s[2];

    math::matrix_4x4 SpreadMat (math::matrix_4x4::unit);

    a[0] = misc::randfloat(-Spread1, Spread1) / 2.0f;
    a[1] = misc::randfloat(-Spread2, Spread2) / 2.0f;

    /*SpreadMat.m[0][0]*=l;
    SpreadMat.m[1][1]*=l;
    SpreadMat.m[2][2]*=w;*/

    for (i = 0; i<2; ++i)
    {
      c[i] = cos(a[i]);
      s[i] = sin(a[i]);
    }

    {
      math::matrix_4x4 Temp (math::matrix_4x4::unit);
      Temp (1, 1, c[0]);
      Temp (2, 1, s[0]);
      Temp (2, 2, c[0]);
      Temp (1, 2, -s[0]);

      SpreadMat = SpreadMat*Temp;
    }

    {
      math::matrix_4x4 Temp (math::matrix_4x4::unit);
      Temp (0, 0, c[1]);
      Temp (1, 0, s[1]);
      Temp (1, 1, c[1]);
      Temp (0, 1, -s[1]);

      SpreadMat = SpreadMat*Temp;
    }

    float Size = std::abs(c[0])*l + std::abs(s[0])*w;
    for (i = 0; i<3; ++i)
      for (j = 0; j<3; j++)
        SpreadMat (i, j, SpreadMat (i, j) * Size);

    return SpreadMat;
  }
}

Particle PlaneParticleEmitter::newParticle(ParticleSystem* sys, int anim, int time, int animtime, float w, float l, float spd, float var, float spr, float /*spr2*/)
{
  // Model Flags - *shrug* gotta write this down somewhere.
  // 0x1 =
  // 0x2 =
  // 0x4 =
  // 0x8 =
  // 0x10 =
  // 19 = 0x13 = blue ball in thunderfury = should be billboarded?

  // Particle Flags
  // 0x0  / 0    = Basilisk has no flags?
  // 0x1  / 1    = Pretty much everything I know of except Basilisks have this flag..  Billboard?
  // 0x2  / 2    =
  // 0x4  / 4    =
  // 0x8  / 8    =
  // 0x10  / 16  = Position Relative to bone pivot?
  // 0x20  / 32  =
  // 0x40  / 64  =
  // 0x80 / 128  =
  // 0x100 / 256  =
  // 0x200 / 512  =
  // 0x400 / 1024 =
  // 0x800 / 2048 =
  // 0x1000/ 4096 =
  // 0x0000/ 1593 = [1,8,16,32,512,1024]"Warp Storm" - aura type particle effect
  // 0x419 / 1049 = [1,8,16,1024] Forest Wind shoulders
  // 0x411 / 1041 = [1,16,1024] Halo
  // 0x000 / 541  = [1,4,8,16,512] Staff glow
  // 0x000 / 537 = "Warp Storm"
  // 0x31 / 49 = [1,16,32] particle moving up?
  // 0x00 / 41 = [1,8,32] Blood elf broom, dust spread out on the ground (X, Z axis)
  // 0x1D / 29 = [1,4,8,16] particle being static
  // 0x19 / 25 = [1,8,16] flame on weapon - move up/along the weapon
  // 17 = 0x11 = [1,16] glow on weapon - static, random direction.  - Aurastone Hammer
  // 1 = 0x1 = perdition blade
  // 4121 = water ele
  // 4097 = water elemental
  // 1041 = Transcendance Halo
  // 1039 = water ele

  Particle p;

  //Spread Calculation
  auto mrot = sys->parent->mrot*CalcSpreadMatrix(spr, spr, 1.0f, 1.0f);

  if (sys->flags == 1041) { // Trans Halo
    p.pos = sys->parent->mat * (sys->pos + math::vector_3d(misc::randfloat(-l, l), 0, misc::randfloat(-w, w)));

    const float t = misc::randfloat(0.0f, 2.0f * (float)math::constants::pi);

    p.pos = math::vector_3d(0.0f, sys->pos.y + 0.15f, sys->pos.z) + math::vector_3d(cos(t) / 8, 0.0f, sin(t) / 8); // Need to manually correct for the halo - why?

    // var isn't being used, which is set to 1.0f,  whats the importance of this?
    // why does this set of values differ from other particles

    math::vector_3d dir(0.0f, 1.0f, 0.0f);
    p.dir = dir;

    p.speed = dir.normalize() * spd * misc::randfloat(0, var);
  }
  else if (sys->flags == 25 && sys->parent->parent<1) { // Weapon Flame
    p.pos = sys->parent->pivot + (sys->pos + math::vector_3d(misc::randfloat(-l, l), misc::randfloat(-l, l), misc::randfloat(-w, w)));
    math::vector_3d dir = mrot * math::vector_3d(0.0f, 1.0f, 0.0f);
    p.dir = dir.normalize();
    //math::vector_3d dir = sys->model->bones[sys->parent->parent].mrot * sys->parent->mrot * math::vector_3d(0.0f, 1.0f, 0.0f);
    //p.speed = dir.normalize() * spd;

  }
  else if (sys->flags == 25 && sys->parent->parent > 0) { // Weapon with built-in Flame (Avenger lightsaber!)
    p.pos = sys->parent->mat * (sys->pos + math::vector_3d(misc::randfloat(-l, l), misc::randfloat(-l, l), misc::randfloat(-w, w)));
    math::vector_3d dir = math::vector_3d(sys->parent->mat (1, 0), sys->parent->mat (1, 1), sys->parent->mat (1, 2)) + math::vector_3d(0.0f, 1.0f, 0.0f);
    p.speed = dir.normalize() * spd * misc::randfloat(0, var * 2);

  }
  else if (sys->flags == 17 && sys->parent->parent<1) { // Weapon Glow
    p.pos = sys->parent->pivot + (sys->pos + math::vector_3d(misc::randfloat(-l, l), misc::randfloat(-l, l), misc::randfloat(-w, w)));
    math::vector_3d dir = mrot * math::vector_3d(0, 1, 0);
    p.dir = dir.normalize();

  }
  else {
    p.pos = sys->pos + math::vector_3d(misc::randfloat(-l, l), 0, misc::randfloat(-w, w));
    p.pos = sys->parent->mat * p.pos;

    //math::vector_3d dir = mrot * math::vector_3d(0,1,0);
    math::vector_3d dir = sys->parent->mrot * math::vector_3d(0, 1, 0);

    p.dir = dir;//.normalize();
    p.down = math::vector_3d(0, -1.0f, 0); // dir * -1.0f;
    p.speed = dir.normalize() * spd * (1.0f + misc::randfloat(-var, var));
  }

  if (!sys->billboard)  {
    p.corners[0] = mrot * math::vector_3d(-1, 0, +1);
    p.corners[1] = mrot * math::vector_3d(+1, 0, +1);
    p.corners[2] = mrot * math::vector_3d(+1, 0, -1);
    p.corners[3] = mrot * math::vector_3d(-1, 0, -1);
  }

  p.life = 0;
  p.maxlife = sys->lifespan.getValue(anim, time, animtime);

  p.origin = p.pos;

  p.tile = misc::randint(0, sys->rows*sys->cols - 1);
  return p;
}

Particle SphereParticleEmitter::newParticle(ParticleSystem* sys, int anim, int time, int animtime, float w, float l, float spd, float var, float spr, float spr2)
{
  Particle p;
  math::vector_3d dir;
  float radius;

  radius = misc::randfloat(0, 1);

  // Old method
  //float t = misc::randfloat(0,2*math::constants::pi);

  // New
  // Spread should never be zero for sphere particles ?
  math::radians t (0);
  if (spr == 0)
    t._ = misc::randfloat((float)-math::constants::pi, (float)math::constants::pi);
  else
    t._ = misc::randfloat(-spr, spr);

  //Spread Calculation
  auto mrot =  sys->parent->mrot*CalcSpreadMatrix(spr * 2, spr2 * 2, w, l);

  // New
  // Length should never technically be zero ?
  //if (l==0)
  //  l = w;

  // New method
  // math::vector_3d bdir(w*math::cos(t), 0.0f, l*math::sin(t));
  // --

  //! \todo fix shpere emitters to work properly
  /* // Old Method
  //math::vector_3d bdir(l*math::cos(t), 0, w*math::sin(t));
  //math::vector_3d bdir(0, w*math::cos(t), l*math::sin(t));


  float theta_range = sys->spread.getValue(anim, time, animtime);
  float theta = -0.5f* theta_range + misc::randfloat(0, theta_range);
  math::vector_3d bdir(0, l*math::cos(theta), w*math::sin(theta));

  float phi_range = sys->lat.getValue(anim, time, animtime);
  float phi = misc::randfloat(0, phi_range);
  rotate(0,0, &bdir.z, &bdir.x, phi);
  */

  if (sys->flags == 57 || sys->flags == 313) { // Faith Halo
    math::vector_3d bdir(w*math::cos(t)*1.6f, 0.0f, l*math::sin(t)*1.6f);

    p.pos = sys->pos + bdir;
    p.pos = sys->parent->mat * p.pos;

    if (bdir.length_squared() == 0)
      p.speed = math::vector_3d(0, 0, 0);
    else {
      dir = sys->parent->mrot * (bdir.normalize());//mrot * math::vector_3d(0, 1.0f,0);
      p.speed = dir.normalize() * spd * (1.0f + misc::randfloat(-var, var));   // ?
    }

  }
  else {
    math::vector_3d bdir;
    float temp;

    bdir = mrot * math::vector_3d(0, 1, 0) * radius;
    temp = bdir.z;
    bdir.z = bdir.y;
    bdir.y = temp;

    p.pos = sys->parent->mat * sys->pos + bdir;


    //p.pos = sys->pos + bdir;
    //p.pos = sys->parent->mat * p.pos;


    if (!bdir.length_squared() && !(sys->flags & 0x100))
    {
      p.speed = math::vector_3d(0, 0, 0);
      dir = sys->parent->mrot * math::vector_3d(0, 1, 0);
    }
    else
    {
      if (sys->flags & 0x100)
        dir = sys->parent->mrot * math::vector_3d(0, 1, 0);
      else
        dir = bdir.normalize();

      p.speed = dir.normalize() * spd * (1.0f + misc::randfloat(-var, var));   // ?
    }
  }

  p.dir = dir.normalize();//mrot * math::vector_3d(0, 1.0f,0);
  p.down = math::vector_3d(0, -1.0f, 0);

  p.life = 0;
  p.maxlife = sys->lifespan.getValue(anim, time, animtime);

  p.origin = p.pos;

  p.tile = misc::randint(0, sys->rows*sys->cols - 1);
  return p;
}

RibbonEmitter::RibbonEmitter(Model* model_, const MPQFile &f, ModelRibbonEmitterDef const& mta, int *globals)
  : model (model_)
  , color (mta.color, f, globals)
  , opacity (mta.opacity, f, globals)
  , above (mta.above, f, globals)
  , below (mta.below, f, globals)
  , parent (&model->bones[mta.bone])
  , pos (fixCoordSystem(mta.pos))
  , seglen (mta.length)
  , length (mta.res * seglen)
   // just use the first texture for now; most models I've checked only had one
  , tpos (fixCoordSystem(mta.pos))
   //! \todo  figure out actual correct way to calculate length
   // in BFD, res is 60 and len is 0.6, the trails are very short (too long here)
   // in CoT, res and len are like 10 but the trails are supposed to be much longer (too short here)
{
  _texture_ids = Model::M2Array<uint16_t>(f, mta.ofsTextures, mta.nTextures);
  _material_ids = Model::M2Array<uint16_t>(f, mta.ofsMaterials, mta.nMaterials);

   // create first segment
  segs.emplace_back(tpos, 0);
}

RibbonEmitter::RibbonEmitter(RibbonEmitter const& other)
  : model(other.model)
  , color(other.color)
  , opacity(other.opacity)
  , above(other.above)
  , below(other.below)
  , parent(other.parent)
  , pos(other.pos)
  , manim(other.manim)
  , mtime(other.mtime)
  , seglen(other.seglen)
  , length(other.length)
  , tpos(other.tpos)
  , tcolor(other.tcolor)
  , tabove(other.tabove)
  , tbelow(other.tbelow)
  , _texture_ids(other._texture_ids)
  , _material_ids(other._material_ids)
  , segs(other.segs)
{

}

RibbonEmitter::RibbonEmitter(RibbonEmitter&& other)
  : model(other.model)
  , color(other.color)
  , opacity(other.opacity)
  , above(other.above)
  , below(other.below)
  , parent(other.parent)
  , pos(other.pos)
  , manim(other.manim)
  , mtime(other.mtime)
  , seglen(other.seglen)
  , length(other.length)
  , tpos(other.tpos)
  , tcolor(other.tcolor)
  , tabove(other.tabove)
  , tbelow(other.tbelow)
  , _texture_ids(other._texture_ids)
  , _material_ids(other._material_ids)
  , segs(other.segs)
{

}

void RibbonEmitter::setup(int anim, int time, int animtime)
{
  math::vector_3d ntpos = parent->mat * pos;
  math::vector_3d ntup = parent->mat * (pos + math::vector_3d(0, 0, 1));
  ntup -= ntpos;
  ntup.normalize();
  float dlen = (ntpos - tpos).length();

  manim = anim;
  mtime = time;

  // move first segment
  RibbonSegment &first = *segs.begin();
  if (first.len > seglen) {
    // add new segment
    first.back = (tpos - ntpos).normalize();
    first.len0 = first.len;
    RibbonSegment newseg (ntpos, dlen);
    newseg.up = ntup;
    segs.push_front(newseg);
  }
  else {
    first.up = ntup;
    first.pos = ntpos;
    first.len += dlen;
  }

  // kill stuff from the end
  float l = 0;
  bool erasemode = false;
  for (std::list<RibbonSegment>::iterator it = segs.begin(); it != segs.end();) {
    if (!erasemode) {
      l += it->len;
      if (l > length) {
        it->len = l - length;
        erasemode = true;
      }
    }
    else {
      segs.erase(it);
    }
    ++it;
  }

  tpos = ntpos;
  tcolor = math::vector_4d(color.getValue(anim, time, animtime), opacity.getValue(anim, time, animtime));

  tabove = above.getValue(anim, time, animtime);
  tbelow = below.getValue(anim, time, animtime);
}

void RibbonEmitter::draw( opengl::scoped::use_program& shader
                        , GLuint const& transform_vbo
                        , int instances_count
                        )
{
  if (!_uploaded)
  {
    upload();
  }

  std::vector<std::uint16_t> indices;
  std::vector<math::vector_3d> vertices;
  std::vector<math::vector_2d> texcoords;

  model->_textures[_texture_ids[0]]->bind();

  gl.enable(GL_BLEND);
  
  shader.uniform("color", tcolor);

  std::uint16_t indice = 0;
  auto add_quad_indices([&indices] (std::uint16_t& start)
  {
    indices.push_back(start + 0);
    indices.push_back(start + 1);
    indices.push_back(start + 2);

    indices.push_back(start + 2);
    indices.push_back(start + 1);
    indices.push_back(start + 3);

    start += 2;
  });

  std::list<RibbonSegment>::iterator it = segs.begin();
  float l = 0;
  for (; it != segs.end(); ++it) 
  {
    float u = l / length;

    texcoords.emplace_back(u, 0);
    vertices.push_back(it->pos + tabove * it->up);
    texcoords.emplace_back(u, 1);
    vertices.push_back(it->pos - tbelow * it->up);

    l += it->len;

    add_quad_indices(indice);
  }

  if (segs.size() > 1) 
  {
    // last segment...?
    --it;
    texcoords.emplace_back(1, 0);
    vertices.push_back(it->pos + tabove * it->up + (it->len / it->len0) * it->back);
    texcoords.emplace_back(1, 1);
    vertices.push_back(it->pos - tbelow * it->up + (it->len / it->len0) * it->back);
  }

  gl.bufferData<GL_ARRAY_BUFFER, math::vector_3d>(_vertices_vbo, vertices, GL_STREAM_DRAW);
  gl.bufferData<GL_ARRAY_BUFFER, math::vector_2d>(_texcoord_vbo, texcoords, GL_STREAM_DRAW);
  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER, std::uint16_t>(_indices_vbo, indices, GL_STREAM_DRAW);

  opengl::scoped::vao_binder const _(_vao);

  shader.attrib(_, "position", _vertices_vbo, 3, GL_FLOAT, GL_FALSE, 0, 0);
  shader.attrib(_, "uv", _texcoord_vbo, 2, GL_FLOAT, GL_FALSE, 0, 0);
  {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const transform_binder(transform_vbo);
    shader.attrib(_, "transform", opengl::array_buffer_is_already_bound{}, static_cast<math::matrix_4x4*> (nullptr), 1);
  }

  gl.drawElementsInstanced(GL_TRIANGLES, indices.size(), instances_count, GL_UNSIGNED_SHORT, _indices_vbo);
}

void RibbonEmitter::upload()
{
  _vertex_array.upload();
  _buffers.upload();
  _uploaded = true;
}
