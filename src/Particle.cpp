#include "Particle.h"

#include <list>

#include "Misc.h"

static const unsigned int MAX_PARTICLES = 10000;

Vec4D fromARGB(uint32_t color)
{
  const float a = ((color & 0xFF000000) >> 24) / 255.0f;
  const float r = ((color & 0x00FF0000) >> 16) / 255.0f;
  const float g = ((color & 0x0000FF00) >>  8) / 255.0f;
  const float b = ((color & 0x000000FF)      ) / 255.0f;
    return Vec4D(r,g,b,a);
}
Vec4D fromBGRA(uint32_t color)
{
  const float b = ((color & 0xFF000000) >> 24) / 255.0f;
  const float g = ((color & 0x00FF0000) >> 16) / 255.0f;
  const float r = ((color & 0x0000FF00) >>  8) / 255.0f;
  const float a = ((color & 0x000000FF)      ) / 255.0f;
  return Vec4D(r,g,b,a);
}

template<class T>
T lifeRamp(float life, float mid, const T &a, const T &b, const T &c)
{
  if (life<=mid) return interpolate<T>(life / mid,a,b);
  else return interpolate<T>((life-mid) / (1.0f-mid),b,c);
}


void ParticleSystem::init(const MPQFile& f, const ModelParticleEmitterDef &mta, int *globals)
{
  speed.init   (mta.EmissionSpeed, f, globals);
  variation.init(mta.SpeedVariation, f, globals);
  spread.init   (mta.VerticalRange, f, globals);
  lat.init   (mta.HorizontalRange, f, globals);
  gravity.init (mta.Gravity, f, globals);
  lifespan.init(mta.Lifespan, f, globals);
  rate.init   (mta.EmissionRate, f, globals);
  areal.init   (mta.EmissionAreaLength, f, globals);
  areaw.init   (mta.EmissionAreaWidth, f, globals);
  deacceleration.init (mta.Gravity2, f, globals);
  enabled.init (mta.en, f, globals);
  
  Vec3D colors2[3];
  memcpy(colors2, f.getBuffer()+mta.p.colors.ofsKeys, sizeof(Vec3D)*3);
  for (size_t i=0; i<3; ++i) {
    float opacity = *reinterpret_cast<int16_t*>(f.getBuffer()+mta.p.opacity.ofsKeys+i*2);
    colors[i] = Vec4D(colors2[i].x/255.0f, colors2[i].y/255.0f, colors2[i].z/255.0f, opacity/32767.0f);
    sizes[i] = (*reinterpret_cast<float*>(f.getBuffer()+mta.p.sizes.ofsKeys+i*4))*mta.p.scales[i];
  }
  mid = 0.5;
  slowdown = mta.p.slowdown;
  rotation = mta.p.rotation;
  pos = fixCoordSystem(mta.pos);
  _texture = model->_textures[mta.texture];
  blend = mta.blend;
  rows = mta.rows;
  cols = mta.cols;
  type = mta.ParticleType;
  //order = mta.s2;
  order = mta.ParticleType>0 ? -1 : 0;
  parent = model->bones + mta.bone;

  switch (mta.EmitterType) {
  case 1:
    emitter = new PlaneParticleEmitter(this);
    break;
  case 2:
    emitter = new SphereParticleEmitter(this);
    break;
  }

  //transform = mta.flags & 1024;

  billboard = !(mta.flags & 4096);

  manim = mtime = 0;
  rem = 0;

  tofs = misc::frand();

  // init tiles
  for (int i=0; i<rows*cols; ++i) {
    TexCoordSet tc;
    initTile(tc.tc,i);
    tiles.push_back(tc);
  }
}

void ParticleSystem::initTile(Vec2D *tc, int num)
{
  Vec2D otc[4];
  Vec2D a,b;
  int x = num % cols;
  int y = num / cols;
  a.x = x * (1.0f / cols);
  b.x = (x+1) * (1.0f / cols);
  a.y = y * (1.0f / rows);
  b.y = (y+1) * (1.0f / rows);

  otc[0] = a;
  otc[2] = b;
  otc[1].x = b.x;
  otc[1].y = a.y;
  otc[3].x = a.x;
  otc[3].y = b.y;

  for (int i=0; i<4; ++i) {
    tc[(i+4-order) & 3] = otc[i];
  }
}


void ParticleSystem::update(float dt)
{
  float grav = gravity.getValue(manim, mtime);
  float deaccel = deacceleration.getValue(manim, mtime);
  
  // spawn new particles
  if (emitter) {
    float frate = rate.getValue(manim, mtime);
    float flife = 1.0f;
    flife = lifespan.getValue(manim, mtime);
    
    float ftospawn = (dt * frate / flife) + rem;
    if (ftospawn < 1.0f) {
      rem = ftospawn;
      if (rem<0) 
        rem = 0;
    } else {
      int tospawn = ftospawn;
      
      if ((tospawn + particles.size()) > MAX_PARTICLES) // Error check to prevent the program from trying to load insane amounts of particles.
        tospawn = particles.size() - MAX_PARTICLES;
      
      rem = ftospawn - static_cast<float>(tospawn);
      
      
      float w = areal.getValue(manim, mtime) * 0.5f;
      float l = areaw.getValue(manim, mtime) * 0.5f;
      float spd = speed.getValue(manim, mtime);
      float var = variation.getValue(manim, mtime);
      float spr = spread.getValue(manim, mtime);
      float spr2 = lat.getValue(manim, mtime);
      bool en = true;
      if (enabled.uses(manim))
        en = enabled.getValue(manim, mtime)!=0;
      
      //rem = 0;
      if (en) {
        for (int i=0; i<tospawn; ++i) {
          Particle p = emitter->newParticle(manim, mtime, w, l, spd, var, spr, spr2);
          // sanity check:
          //if (particles.size() < MAX_PARTICLES) // No need to check this every loop iteration. Already checked above.
          particles.push_back(p);
        }
      }
    }
  }
  
  float mspeed = 1.0f;
  
  for (ParticleList::iterator it = particles.begin(); it != particles.end(); ) {
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
    p.color = lifeRamp<Vec4D>(rlife, mid, colors[0], colors[1], colors[2]);
    
    // kill off old particles
    if (rlife >= 1.0f) 
      particles.erase(it);
    
    ++it;
  }
}

void ParticleSystem::setup(int anim, int time)
{
  manim = anim;
  mtime = time;

  /*
  if (transform) {
    // transform every particle by the parent trans matrix   - apparently this isn't needed
    Matrix m = parent->mat;
    for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
      it->tpos = m * it->pos;
    }
  } else {
    for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
      it->tpos = it->pos;
    }
  }
  */   
}

void ParticleSystem::draw()
{
  /*
  // just draw points:
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glColor4f(1,1,1,1);
  glBegin(GL_POINTS);
  for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
    glVertex3fv(it->tpos);
  }
  glEnd();
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  */                                   

  // setup blend mode
  switch (blend) {
  case 0:
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    break;
  case 1:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE);
    glDisable(GL_ALPHA_TEST);
    break;
  case 2:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);
    break;
  case 3:
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    break;
  case 4:
    glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glDisable(GL_ALPHA_TEST);
    break;
  }
  
  //glDisable(GL_LIGHTING);
  //glDisable(GL_CULL_FACE);
  //glDepthMask(GL_FALSE);

//  glPushName(texture);
  _texture->bind();

  /*
   if (supportPointSprites && rows==1 && cols==1) {
   // This is how will our point sprite's size will be modified by 
   // distance from the viewer
   float quadratic[] = {0.1f, 0.0f, 0.5f};
   //float quadratic[] = {0.88f, 0.001f, 0.000004f};
   glPointParameterfvARB( GL_POINT_DISTANCE_ATTENUATION_ARB, quadratic);
   
   // Query for the max point size supported by the hardware
   float maxSize = 512.0f;
   //glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxSize );
   
   // Clamp size to 100.0f or the sprites could get a little too big on some  
   // of the newer graphic cards. My ATI card at home supports a max point 
   // size of 1024.0f!
   //if( maxSize > 100.0f )
   //  maxSize = 100.0f;
   
   glPointSize(maxSize);
   
   // The alpha of a point is calculated to allow the fading of points 
   // instead of shrinking them past a defined threshold size. The threshold 
   // is defined by GL_POINT_FADE_THRESHOLD_SIZE_ARB and is not clamped to 
   // the minimum and maximum point sizes.
   glPointParameterfARB(GL_POINT_FADE_THRESHOLD_SIZE_ARB, 60.0f);
   
   glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f );
   glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, maxSize );
   
   // Specify point sprite texture coordinate replacement mode for each texture unit
   glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);
   // Render point sprites...
   glEnable(GL_POINT_SPRITE_ARB);
   
   glBegin(GL_POINTS);
   {
   for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
   glPointSize(it->size);
   glTexCoord2fv(tiles[it->tile].tc[0]);
   glColor4fv(it->color);
   glVertex3fv(it->pos);
   }
   }
   glEnd();
   
   glDisable(GL_POINT_SPRITE_ARB);
   glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_FALSE);
   
   } else { // Old slow method */
  
  Vec3D vRight(1,0,0);
  Vec3D vUp(0,1,0);
  
  // position stuff
  const float f = 1;//0.707106781f; // sqrt(2)/2
  Vec3D bv0 = Vec3D(-f,+f,0);
  Vec3D bv1 = Vec3D(+f,+f,0);
  Vec3D bv2 = Vec3D(+f,-f,0);
  Vec3D bv3 = Vec3D(-f,-f,0);
  
  if (billboard) {
    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    
    vRight = Vec3D(modelview[0], modelview[4], modelview[8]);
    vUp = Vec3D(modelview[1], modelview[5], modelview[9]); // Spherical billboarding
    //vUp = Vec3D(0,1,0); // Cylindrical billboarding
  }
  /*
   * type:
   * 0   "normal" particle
   * 1  large quad from the particle's origin to its position (used in Moonwell water effects)
   * 2  seems to be the same as 0 (found some in the Deeprun Tram blinky-lights-sign thing)
   */
  if (type==0 || type==2 ) {
    // TODO: figure out type 2 (deeprun tram subway sign)
    // - doesn't seem to be any different from 0 -_-
    // regular particles
    
    if (billboard) {
      glBegin(GL_QUADS);
      // TODO: per-particle rotation in a non-expensive way?? :|
      for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
        if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
          break;
        const float size = it->size;// / 2;
        glColor4fv(it->color);
        
        glTexCoord2fv(tiles[it->tile].tc[0]);
        glVertex3fv(it->pos - (vRight + vUp) * size);
        
        glTexCoord2fv(tiles[it->tile].tc[1]);
        glVertex3fv(it->pos + (vRight - vUp) * size);
        
        glTexCoord2fv(tiles[it->tile].tc[2]);
        glVertex3fv(it->pos + (vRight + vUp) * size);
        
        glTexCoord2fv(tiles[it->tile].tc[3]);
        glVertex3fv(it->pos - (vRight - vUp) * size);
      }
      glEnd();
      
    } else {
      glBegin(GL_QUADS);
      for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
        if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
          break;
        glColor4fv(it->color);
        
        glTexCoord2fv(tiles[it->tile].tc[0]);
        glVertex3fv(it->pos + it->corners[0] * it->size);
        
        glTexCoord2fv(tiles[it->tile].tc[1]);
        glVertex3fv(it->pos + it->corners[1] * it->size);
        
        glTexCoord2fv(tiles[it->tile].tc[2]);
        glVertex3fv(it->pos + it->corners[2] * it->size);
        
        glTexCoord2fv(tiles[it->tile].tc[3]);
        glVertex3fv(it->pos + it->corners[3] * it->size);
      }
      glEnd();
    }
  } else if (type==1) { // Sphere particles
    // particles from origin to position
    /*
     bv0 = mbb * Vec3D(0,-1.0f,0);
     bv1 = mbb * Vec3D(0,+1.0f,0);
     
     
     bv0 = mbb * Vec3D(-1.0f,0,0);
     bv1 = mbb * Vec3D(1.0f,0,0);
     */
    
    glBegin(GL_QUADS);
    for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
      if (tiles.size() - 1 < it->tile) // Alfred, 2009.08.07, error prevent
        break;
      glColor4fv(it->color);
      
      glTexCoord2fv(tiles[it->tile].tc[0]);
      glVertex3fv(it->pos + bv0 * it->size);
      
      glTexCoord2fv(tiles[it->tile].tc[1]);
      glVertex3fv(it->pos + bv1 * it->size);
      
      glTexCoord2fv(tiles[it->tile].tc[2]);
      glVertex3fv(it->origin + bv1 * it->size);
      
      glTexCoord2fv(tiles[it->tile].tc[3]);
      glVertex3fv(it->origin + bv0 * it->size);
    }
    glEnd();
    
  }
  //}
  
  //glEnable(GL_LIGHTING);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glDepthMask(GL_TRUE);
  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void ModelHighlight( Vec4D color );
void ModelUnhighlight();
void ParticleSystem::drawHighlight()
{
  /*
  // just draw points:
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glColor4f(1,1,1,1);
  glBegin(GL_POINTS);
  for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
    glVertex3fv(it->tpos);
  }
  glEnd();
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  */                              

  Vec3D bv0,bv1,bv2,bv3;

  // setup blend mode
  switch (blend) {
  case 0:
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    break;
  case 1:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE);
    glDisable(GL_ALPHA_TEST);
    break;
  case 2:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);
    break;
  case 3:
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    break;
  case 4:
    glEnable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    break;
  }

  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDepthMask(GL_FALSE);

//  glPushName(texture);
  _texture->bind();

  Matrix mbb;
  mbb.unit();

  ModelHighlight( Vec4D( 1.00, 0.25, 0.25, 0.50 ) );
  if (billboard) {
    // get a billboard matrix
    Matrix mtrans;
    glGetFloatv(GL_MODELVIEW_MATRIX, &(mtrans.m[0][0]));
    mtrans.transpose();
    mtrans.invert();
    Vec3D camera = mtrans * Vec3D(0,0,0);
    Vec3D look = (camera - pos).normalize();
    Vec3D up = ((mtrans * Vec3D(0,1,0)) - camera).normalize();
    Vec3D right = (up % look).normalize();
    up = (look % right).normalize();
    // calculate the billboard matrix
    mbb.m[0][1] = right.x;
    mbb.m[1][1] = right.y;
    mbb.m[2][1] = right.z;
    mbb.m[0][2] = up.x;
    mbb.m[1][2] = up.y;
    mbb.m[2][2] = up.z;
    mbb.m[0][0] = look.x;
    mbb.m[1][0] = look.y;
    mbb.m[2][0] = look.z;
  }

  if (type==0 || type==2) {
    //! \todo  figure out type 2 (deeprun tram subway sign)
    // - doesn't seem to be any different from 0 -_-
    // regular particles
    float f = 0.707106781f; // sqrt(2)/2
    if (billboard) {
      bv0 = mbb * Vec3D(0,-f,+f);
      bv1 = mbb * Vec3D(0,+f,+f);
      bv2 = mbb * Vec3D(0,+f,-f);
      bv3 = mbb * Vec3D(0,-f,-f);
    } else {
      bv0 = Vec3D(-f,0,+f);
      bv1 = Vec3D(+f,0,+f);
      bv2 = Vec3D(+f,0,-f);
      bv3 = Vec3D(-f,0,-f);
    }
    //! \todo  per-particle rotation in a non-expensive way?? :|

    glBegin(GL_QUADS);
    for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
      //glColor4fv(it->color);
      glColor4f(1.0f,0.25f,0.25f,it->color.w*0.5f);

      glTexCoord2fv(tiles[it->tile].tc[0]);
      glVertex3fv(it->pos + bv0 * it->size);

      glTexCoord2fv(tiles[it->tile].tc[1]);
      glVertex3fv(it->pos + bv1 * it->size);

      glTexCoord2fv(tiles[it->tile].tc[2]);
      glVertex3fv(it->pos + bv2 * it->size);

      glTexCoord2fv(tiles[it->tile].tc[3]);
      glVertex3fv(it->pos + bv3 * it->size);
    }
    glEnd();
  }
  else if (type==1) {
    // particles from origin to position
    bv0 = mbb * Vec3D(0,-1.0f,0);
    bv1 = mbb * Vec3D(0,+1.0f,0);

    glBegin(GL_QUADS);
    for (ParticleList::iterator it = particles.begin(); it != particles.end(); ++it) {
      glColor4fv(it->color);

      glTexCoord2fv(tiles[it->tile].tc[0]);
      glVertex3fv(it->pos + bv0 * it->size);

      glTexCoord2fv(tiles[it->tile].tc[1]);
      glVertex3fv(it->pos + bv1 * it->size);

      glTexCoord2fv(tiles[it->tile].tc[2]);
      glVertex3fv(it->origin + bv1 * it->size);

      glTexCoord2fv(tiles[it->tile].tc[3]);
      glVertex3fv(it->origin + bv0 * it->size);
    }
    glEnd();
  }
  ModelUnhighlight();
  glEnable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_TRUE);
  glColor4f(1,1,1,1);
//  glPopName();
}
//Generates the rotation matrix based on spread
Matrix  SpreadMat;
void CalcSpreadMatrix(float Spread1,float Spread2, float w, float l)
{
  int i,j;
  float a[2],c[2],s[2];
  Matrix  Temp;
  
  SpreadMat.unit();
  
  a[0]=misc::randfloat(-Spread1,Spread1)/2.0f;
  a[1]=misc::randfloat(-Spread2,Spread2)/2.0f;
  
  /*SpreadMat.m[0][0]*=l;
   SpreadMat.m[1][1]*=l;
   SpreadMat.m[2][2]*=w;*/
  
  for(i=0;i<2;++i)
  {    
    c[i]=cos(a[i]);
    s[i]=sin(a[i]);
  }
  Temp.unit();
  Temp.m[1][1]=c[0];
  Temp.m[2][1]=s[0];
  Temp.m[2][2]=c[0];
  Temp.m[1][2]=-s[0];
  
  SpreadMat=SpreadMat*Temp;
  
  Temp.unit();
  Temp.m[0][0]=c[1];
  Temp.m[1][0]=s[1];
  Temp.m[1][1]=c[1];
  Temp.m[0][1]=-s[1];
  
  SpreadMat=SpreadMat*Temp;
  
  float Size=abs(c[0])*l+abs(s[0])*w;
  for(i=0;i<3;++i)
    for(j=0;j<3;j++)
      SpreadMat.m[i][j]*=Size;
}

Particle PlaneParticleEmitter::newParticle(int anim, int time, float w, float l, float spd, float var, float spr, float /*spr2*/)
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
  Matrix mrot;
  
  CalcSpreadMatrix(spr,spr,1.0f,1.0f);
  mrot=sys->parent->mrot*SpreadMat;
  
  if (sys->flags == 1041) { // Trans Halo
    p.pos = sys->parent->mat * (sys->pos + Vec3D(misc::randfloat(-l,l), 0, misc::randfloat(-w,w)));
    
    const float t = misc::randfloat(0.0f, 2.0f * PI);
    
    p.pos = Vec3D(0.0f, sys->pos.y + 0.15f, sys->pos.z) + Vec3D(cos(t)/8, 0.0f, sin(t)/8); // Need to manually correct for the halo - why?
    
    // var isn't being used, which is set to 1.0f,  whats the importance of this?
    // why does this set of values differ from other particles
    
    Vec3D dir(0.0f, 1.0f, 0.0f);
    p.dir = dir;
    
    p.speed = dir.normalize() * spd * misc::randfloat(0, var);
  } else if (sys->flags == 25 && sys->parent->parent<1) { // Weapon Flame
    p.pos = sys->parent->pivot * (sys->pos + Vec3D(misc::randfloat(-l,l), misc::randfloat(-l,l), misc::randfloat(-w,w)));
    Vec3D dir = mrot * Vec3D(0.0f, 1.0f, 0.0f);
    p.dir = dir.normalize();
    //Vec3D dir = sys->model->bones[sys->parent->parent].mrot * sys->parent->mrot * Vec3D(0.0f, 1.0f, 0.0f);
    //p.speed = dir.normalize() * spd;
    
  } else if (sys->flags == 25 && sys->parent->parent > 0) { // Weapon with built-in Flame (Avenger lightsaber!)
    p.pos = sys->parent->mat * (sys->pos + Vec3D(misc::randfloat(-l,l), misc::randfloat(-l,l), misc::randfloat(-w,w)));
    Vec3D dir = Vec3D(sys->parent->mat.m[1][0],sys->parent->mat.m[1][1], sys->parent->mat.m[1][2]) * Vec3D(0.0f, 1.0f, 0.0f);
    p.speed = dir.normalize() * spd * misc::randfloat(0, var*2);
    
  } else if (sys->flags == 17 && sys->parent->parent<1) { // Weapon Glow
    p.pos = sys->parent->pivot * (sys->pos + Vec3D(misc::randfloat(-l,l), misc::randfloat(-l,l), misc::randfloat(-w,w)));
    Vec3D dir = mrot * Vec3D(0,1,0);
    p.dir = dir.normalize();
    
  } else {
    p.pos = sys->pos + Vec3D(misc::randfloat(-l,l), 0, misc::randfloat(-w,w));
    p.pos = sys->parent->mat * p.pos;
    
    //Vec3D dir = mrot * Vec3D(0,1,0);
    Vec3D dir = sys->parent->mrot * Vec3D(0,1,0);
    
    p.dir = dir;//.normalize();
    p.down = Vec3D(0,-1.0f,0); // dir * -1.0f;
    p.speed = dir.normalize() * spd * (1.0f+misc::randfloat(-var,var));
  }
  
  if(!sys->billboard)  {
    p.corners[0] = mrot * Vec3D(-1,0,+1);
    p.corners[1] = mrot * Vec3D(+1,0,+1);
    p.corners[2] = mrot * Vec3D(+1,0,-1);
    p.corners[3] = mrot * Vec3D(-1,0,-1);
  }
  
  p.life = 0;
  p.maxlife = sys->lifespan.getValue(anim, time);
  
  p.origin = p.pos;
  
  p.tile = misc::randint(0, sys->rows*sys->cols-1);
  return p;
}

Particle SphereParticleEmitter::newParticle(int anim, int time, float w, float l, float spd, float var, float spr, float spr2)
{
  Particle p;
  Vec3D dir;
  float radius;
  
  radius = misc::randfloat(0,1);
  
  // Old method
  //float t = misc::randfloat(0,2*PI);
  
  // New
  // Spread should never be zero for sphere particles ?
  float t = 0;
  if (spr == 0)
    t = misc::randfloat(-PI, PI);
  else
    t = misc::randfloat(-spr, spr);
  
  //Spread Calculation
  Matrix mrot;
  
  CalcSpreadMatrix(spr*2,spr2*2,w,l);
  mrot=sys->parent->mrot*SpreadMat;
  
  // New
  // Length should never technically be zero ?
  //if (l==0)
  //  l = w;
  
  // New method
  // Vec3D bdir(w*cosf(t), 0.0f, l*sinf(t));
  // --
  
  // TODO: fix shpere emitters to work properly
  /* // Old Method
   //Vec3D bdir(l*cosf(t), 0, w*sinf(t));
   //Vec3D bdir(0, w*cosf(t), l*sinf(t));
   
   
   float theta_range = sys->spread.getValue(anim, time);
   float theta = -0.5f* theta_range + misc::randfloat(0, theta_range);
   Vec3D bdir(0, l*cosf(theta), w*sinf(theta));
   
   float phi_range = sys->lat.getValue(anim, time);
   float phi = misc::randfloat(0, phi_range);
   rotate(0,0, &bdir.z, &bdir.x, phi);
   */
  
  if (sys->flags == 57 || sys->flags == 313) { // Faith Halo
    Vec3D bdir(w*cosf(t)*1.6, 0.0f, l*sinf(t)*1.6);
    
    p.pos = sys->pos + bdir;
    p.pos = sys->parent->mat * p.pos;
    
    if (bdir.lengthSquared()==0) 
      p.speed = Vec3D(0,0,0);
    else {
      dir = sys->parent->mrot * (bdir.normalize());//mrot * Vec3D(0, 1.0f,0);
      p.speed = dir.normalize() * spd * (1.0f+misc::randfloat(-var,var));   // ?
    }
    
  } else {
    Vec3D bdir;
    float temp;
    
    bdir = mrot * Vec3D(0,1,0) * radius;
    temp = bdir.z;
    bdir.z = bdir.y;
    bdir.y = temp;
    
    p.pos = sys->parent->mat * sys->pos + bdir;
    
    
    //p.pos = sys->pos + bdir;
    //p.pos = sys->parent->mat * p.pos;
    
    
    if (!bdir.lengthSquared() && !(sys->flags & 0x100))
    {
      p.speed = Vec3D(0,0,0);
      dir = sys->parent->mrot * Vec3D(0,1,0);
    }
    else
    {
      if(sys->flags & 0x100)
        dir = sys->parent->mrot * Vec3D(0,1,0);
      else
        dir = bdir.normalize();
      
      p.speed = dir.normalize() * spd * (1.0f+misc::randfloat(-var,var));   // ?
    }
  }
  
  p.dir =  dir.normalize();//mrot * Vec3D(0, 1.0f,0);
  p.down = Vec3D(0,-1.0f,0);
  
  p.life = 0;
  p.maxlife = sys->lifespan.getValue(anim, time);
  
  p.origin = p.pos;
  
  p.tile = misc::randint(0, sys->rows*sys->cols-1);
  return p;
}

void RibbonEmitter::init(const MPQFile &f, ModelRibbonEmitterDef &mta, int *globals)
{
  color.init(mta.color, f, globals);
  opacity.init(mta.opacity, f, globals);
  above.init(mta.above, f, globals);
  below.init(mta.below, f, globals);

  parent = model->bones + mta.bone;
  uint32_t *texlist = reinterpret_cast<uint32_t*>(f.getBuffer() + mta.ofsTextures);
  // just use the first texture for now; most models I've checked only had one
  _texture = model->_textures[texlist[0]];

  tpos = pos = fixCoordSystem(mta.pos);

  //! \todo  figure out actual correct way to calculate length
  // in BFD, res is 60 and len is 0.6, the trails are very short (too long here)
  // in CoT, res and len are like 10 but the trails are supposed to be much longer (too short here)
  numsegs = mta.res;
  seglen = mta.length;
  length = mta.res * seglen;

  // create first segment
  RibbonSegment rs;
  rs.pos = tpos;
  rs.len = 0;
  segs.push_back(rs);
}

void RibbonEmitter::setup(int anim, int time)
{
  Vec3D ntpos = parent->mat * pos;
  Vec3D ntup = parent->mat * (pos + Vec3D(0,0,1));
  ntup -= ntpos;
  ntup.normalize();
  float dlen = (ntpos-tpos).length();

  manim = anim;
  mtime = time;

  // move first segment
  RibbonSegment &first = *segs.begin();
  if (first.len > seglen) {
    // add new segment
    first.back = (tpos-ntpos).normalize();
    first.len0 = first.len;
    RibbonSegment newseg;
    newseg.pos = ntpos;
    newseg.up = ntup;
    newseg.len = dlen;
    segs.push_front(newseg);
  } else {
    first.up = ntup;
    first.pos = ntpos;
    first.len += dlen;
  }

  // kill stuff from the end
  float l = 0;
  bool erasemode = false;
  for (std::list<RibbonSegment>::iterator it = segs.begin(); it != segs.end(); ) {
    if (!erasemode) {
      l += it->len;
      if (l > length) {
        it->len = l - length;
        erasemode = true;
      }
    } else {
      segs.erase(it);
    }
    ++it;
  }

  tpos = ntpos;
  tcolor = Vec4D(color.getValue(anim, time), opacity.getValue(anim, time));

  tabove = above.getValue(anim, time);
  tbelow = below.getValue(anim, time);
}

void RibbonEmitter::draw()
{
  /*
  // placeholders
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glColor4f(1,1,1,1);
  glBegin(GL_TRIANGLES);
  glVertex3fv(tpos);
  glVertex3fv(tpos + Vec3D(1,1,0));
  glVertex3fv(tpos + Vec3D(-1,1,0));
  glEnd();
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  */                              

//  glPushName(texture);
  _texture->bind();
  glEnable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_CULL_FACE);
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glColor4fv(tcolor);

  glBegin(GL_QUAD_STRIP);
  std::list<RibbonSegment>::iterator it = segs.begin();
  float l = 0;
  for (; it != segs.end(); ++it) {
        float u = l/length;

    glTexCoord2f(u,0);
    glVertex3fv(it->pos + tabove * it->up);
    glTexCoord2f(u,1);
    glVertex3fv(it->pos - tbelow * it->up);

    l += it->len;
  }

  if (segs.size() > 1) {
    // last segment...?
    --it;
    glTexCoord2f(1,0);
    glVertex3fv(it->pos + tabove * it->up + (it->len/it->len0) * it->back);
    glTexCoord2f(1,1);
    glVertex3fv(it->pos - tbelow * it->up + (it->len/it->len0) * it->back);
  }
  glEnd();

  glColor4f(1,1,1,1);
  glEnable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_TRUE);
//  glPopName();
}

