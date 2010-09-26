#include <cassert>
#include <algorithm>
#include <map>
#include <string>
#include <sstream>

#include "model.h"
#include "world.h"
#include "Log.h"
#include "TextureManager.h" // TextureManager, Texture

int globalTime = 0;

Model::Model(const std::string& _name, bool _forceAnim) : ManagedItem(_name), forceAnim(_forceAnim)
{
  filename = _name;
  
  transform( filename.begin(), filename.end(), filename.begin(), ::tolower );
  size_t found = filename.rfind( ".mdx" );
  if( found != std::string::npos )
    filename.replace( found, 4, ".m2" );
  
  found = filename.rfind( ".mdl" );
  if( found != std::string::npos )
    filename.replace( found, 4, ".m2" );
    
  found = filename.rfind( ".m2" );
  if( found == std::string::npos )
  {
    LogError << "I can't use the model \"" << filename << "\" as I can't get its ending to .m2." << std::endl;
    return;
  }

  
  textures = NULL;
  globalSequences = NULL;
  indices = NULL;
  anims = NULL;
  origVertices = NULL;
  bones = NULL;
  texanims = NULL;
  colors = NULL;
  transparency = NULL;
  lights = NULL;
  particleSystems = NULL;
  ribbons = NULL;
}

void Model::finishLoading()
{
  MPQFile f( filename );
  
  if( f.isEof() ) 
  {
    LogError << "Error loading file \"" << filename << "\". Aborting to load model." << std::endl;
    return;
  }
  
  LogDebug << "Loading model \"" << filename << "\"." << std::endl;
  
  memcpy( &header, f.getBuffer(), sizeof( ModelHeader ) );
  
  animated = isAnimated( f ) || forceAnim;  // isAnimated will set animGeometry and animTextures
  
  trans = 1.0f;
  
  vbuf = nbuf = tbuf = 0;
  
  globalSequences = 0;
  animtime = 0;
  anim = 0;
  colors = 0;
  lights = 0;
  transparency = 0;
  particleSystems = 0;
  header.nParticleEmitters = 0;      //! \todo  Get Particles to 3.*? ._.
  header.nRibbonEmitters = 0;      //! \todo  Get Particles to 3.*? ._.
  ribbons = 0;
  if (header.nGlobalSequences) 
  {
    globalSequences = new int[header.nGlobalSequences];
    memcpy(globalSequences, (f.getBuffer() + header.ofsGlobalSequences), header.nGlobalSequences * 4);
  }
  
  //! \todo  This takes a biiiiiit long. Have a look at this.
  if( animated )
    initAnimated( f );
  else 
    initStatic( f );
  
  f.close();
}

Model::~Model()
{
  LogDebug << "Unloading model \"" << filename << "\"." << std::endl;
    
  if (header.nTextures && textures) {
    for (size_t i=0; i<header.nTextures; i++) {
      if (textures[i]!=0) {
        //Texture *tex = (Texture*)TextureManager::items[textures[i]];
        TextureManager::del(textures[i]);
      }
    }
    delete[] textures;
  }

  if( globalSequences )
    delete[] globalSequences;

  if (animated) {
    // unload all sorts of crap
    //delete[] vertices;
    //delete[] normals;
    if(indices)
      delete[] indices;
    if(anims)
      delete[] anims;
    if(origVertices)
      delete[] origVertices;
    if(bones) 
      delete[] bones;
    if (!animGeometry) {
      glDeleteBuffers(1, &nbuf);
    }
    glDeleteBuffers(1, &vbuf);
    glDeleteBuffers(1, &tbuf);

    if (animTextures && texanims) 
      delete[] texanims;
    if (colors) 
      delete[] colors;
    if (transparency) 
      delete[] transparency;
    if (lights) 
      delete[] lights;

    if (particleSystems) 
      delete[] particleSystems;
    if (ribbons) 
      delete[] ribbons;
  } else {
    glDeleteLists(ModelDrawList, 1);
  }
}


bool Model::isAnimated(MPQFile &f)
{
  // see if we have any animated bones
  ModelBoneDef *bo = (ModelBoneDef*)(f.getBuffer() + header.ofsBones);

  animGeometry = false;
  animBones = false;


  ModelVertex *verts = (ModelVertex*)(f.getBuffer() + header.ofsVertices);
  for (size_t i=0; i<header.nVertices && !animGeometry; i++) {
    for (size_t b=0; b<4; b++) {
      if (verts[i].weights[b]>0) {
        ModelBoneDef &bb = bo[verts[i].bones[b]];
        if (bb.translation.type || bb.rotation.type || bb.scaling.type || (bb.flags&8)) {
          if (bb.flags&8) {
            // if we have billboarding, the model will need per-instance animation
            mPerInstanceAnimation = true;
          }
          animGeometry = false;///true;
          break;
        }
      }
    }
  }

  if (animGeometry) animBones = true;
  else {
    for (size_t i=0; i<header.nBones; i++) {
      ModelBoneDef &bb = bo[i];
      if (bb.translation.type || bb.rotation.type || bb.scaling.type) {
        animBones = true;
        break;
      }
    }
  }

  animTextures = header.nTexAnims > 0;

  bool animMisc = header.nCameras>0 || // why waste time, pretty much all models with cameras need animation
          header.nLights>0 || // same here
          header.nParticleEmitters>0 ||
          header.nRibbonEmitters>0;

  if (animMisc) animBones = true;

  // animated colors
  if (header.nColors) {
    ModelColorDef *cols = (ModelColorDef*)(f.getBuffer() + header.ofsColors);
    for (size_t i=0; i<header.nColors; i++) {
      if (cols[i].color.type!=0 || cols[i].opacity.type!=0) {
        animMisc = true;
        break;
      }
    }
  }

  // animated opacity
  if (header.nTransparency && !animMisc) {
    ModelTransDef *trs = (ModelTransDef*)(f.getBuffer() + header.ofsTransparency);
    for (size_t i=0; i<header.nTransparency; i++) {
      if (trs[i].trans.type!=0) {
        animMisc = true;
        break;
      }
    }
  }

  // guess not...
  return animGeometry || animTextures || animMisc;
}


Vec3D fixCoordSystem(Vec3D v)
{
  return Vec3D(v.x, v.z, -v.y);
}

Vec3D fixCoordSystem2(Vec3D v)
{
  return Vec3D(v.x, v.z, v.y);
}

Quaternion fixCoordSystemQuat(Quaternion v)
{
  return Quaternion(-v.x, -v.z, v.y, v.w);
}


void Model::initCommon(MPQFile &f)
{
  // assume: origVertices already set
  if (!animGeometry) {
    vertices = new Vec3D[header.nVertices];
    normals = new Vec3D[header.nVertices];
  }

  // vertices, normals
  for (size_t i=0; i<header.nVertices; i++) {
    origVertices[i].pos = fixCoordSystem(origVertices[i].pos);
    origVertices[i].normal = fixCoordSystem(origVertices[i].normal);

    if (!animGeometry) {
      vertices[i] = origVertices[i].pos;
      normals[i] = origVertices[i].normal.normalize();
    }

    float len = origVertices[i].pos.lengthSquared();
    if (len > rad){ 
      rad = len;
    }
  }

  rad = sqrtf(rad);

  // textures
  ModelTextureDef *texdef = (ModelTextureDef*)(f.getBuffer() + header.ofsTextures);
  char texname[256];
  textures = new GLuint[ header.nTextures ];
  for (size_t i=0; i<header.nTextures; i++) 
  {
    if (texdef[i].type == 0) 
    {
      // std::string( f.getBuffer() + texdef[i].nameOfs )
      strncpy(texname, reinterpret_cast<char*>( f.getBuffer() ) + texdef[i].nameOfs, texdef[i].nameLen);
      texname[texdef[i].nameLen] = 0;
      std::string path( texname );
      textures[i] = TextureManager::add( path );
    } 
    else 
    {
      // special texture - only on characters and such...
            textures[i] = 0;
    }
  }

  // init colors
  if (header.nColors) {
    colors = new ModelColor[header.nColors];
    ModelColorDef *colorDefs = (ModelColorDef*)(f.getBuffer() + header.ofsColors);
    for (size_t i=0; i<header.nColors; i++) colors[i].init(f, colorDefs[i], globalSequences);
  }
  // init transparency
  int16_t *transLookup = (int16_t*)(f.getBuffer() + header.ofsTransparencyLookup);
  if (header.nTransparency) {
    transparency = new ModelTransparency[header.nTransparency];
    ModelTransDef *trDefs = (ModelTransDef*)(f.getBuffer() + header.ofsTransparency);
    for (size_t i=0; i<header.nTransparency; i++) transparency[i].init(f, trDefs[i], globalSequences);
  }

  // indices - allocate space, too

  // replace .M2 with 0i.skin
  std::string skinfilename = filename;
  std::stringstream temp; temp << "0" << ( header.nViews - 1 ) << ".skin";
  
  size_t found = skinfilename.rfind( ".m2" );
  if( found != std::string::npos )
    skinfilename.replace( found, 3, temp.str() );

  MPQFile skin( skinfilename );
  if( skin.isEof() )
  {
    LogError << "Error when trying to load .skin file \"" << skinfilename << "\". Aborting." << std::endl;
    return;
  }

  ModelView *view = (ModelView*)(skin.getBuffer());

  uint16_t *indexLookup = (uint16_t*)(skin.getBuffer() + view->ofsIndex);
  uint16_t *triangles = (uint16_t*)(skin.getBuffer() + view->ofsTris);
  nIndices = view->nTris;
  indices = new uint16_t[nIndices];
  for (size_t i = 0; i<nIndices; i++) {
        indices[i] = indexLookup[triangles[i]];
  }

  // render ops
  ModelGeoset *ops = (ModelGeoset*)(skin.getBuffer() + view->ofsSub);
  ModelTexUnit *tex = (ModelTexUnit*)(skin.getBuffer() + view->ofsTex);
  ModelRenderFlags *renderFlags = (ModelRenderFlags*)(f.getBuffer() + header.ofsRenderFlags);
  uint16_t *texlookup = (uint16_t*)(f.getBuffer() + header.ofsTexLookup);
  uint16_t *texanimlookup = (uint16_t*)(f.getBuffer() + header.ofsTexAnimLookup);
  int16_t *texunitlookup = (int16_t*)(f.getBuffer() + header.ofsTexUnitLookup);

  /*
  for (size_t i = 0; i<view->nSub; i++) {
    ModelRenderPass pass;
    pass.usetex2 = false;
    pass.indexStart = ops[i].istart;
    pass.indexCount = ops[i].icount;

    // textures
    for (size_t j = 0; j<view->nTex; j++) {
      if (tex[j].op==i) {

        GLuint texid = textures[texlookup[tex[j].textureid]];

        if (tex[j].texunit==0) {
          pass.texture = texid;
          
          //! \todo  figure out these flags properly -_-
          ModelRenderFlags &rf = renderFlags[tex[j].flagsIndex];
          
          //pass.useenvmap = (rf.flags2 & 6)==6;
          //pass.useenvmap = rf.blend == 6; // ???
          pass.useenvmap = texunitlookup[tex[j].texunit] == -1;

          pass.blendmode = rf.blend;
          pass.color = tex[j].colorIndex;
          pass.opacity = transLookup[tex[j].transid];

          pass.cull = (rf.flags & 4)==0 && rf.blend==0;
          pass.unlit = (rf.flags & 3)!=0;

          pass.nozwrite = pass.blendmode >= 2; //(rf.flags & 16)!=0;

          pass.trans = pass.blendmode != 0;

          pass.p = ops[i].v.x;


          if (animTextures) {
            if (tex[j].flags & 16) {
              pass.texanim = -1; // no texture animation
            } else {
              pass.texanim = texanimlookup[tex[j].texanimid];
            }
          } else {
            pass.texanim = -1; // no texture animation
          }
        }
        else if (tex[j].texunit==1) {
          pass.texture2 = texid;
          //pass.usetex2 = true;
        }
      }
    }

        passes.push_back(pass);
  }
  */
  for (size_t j = 0; j<view->nTex; j++) {
    ModelRenderPass pass;
    pass.usetex2 = false;
    pass.texture2 = 0;
    size_t geoset = tex[j].op;
    pass.indexStart = ops[geoset].istart;
    pass.indexCount = ops[geoset].icount;
    pass.vertexStart = ops[geoset].vstart;
    pass.vertexEnd = pass.vertexStart + ops[geoset].vcount;

    pass.order = tex[j].order;

    GLuint texid = textures[texlookup[tex[j].textureid]];

    pass.texture = texid;
    
    //! \todo  figure out these flags properly -_-
    ModelRenderFlags &rf = renderFlags[tex[j].flagsIndex];
    
    pass.useenvmap = texunitlookup[tex[j].texunit] == -1;

    pass.blendmode = rf.blend;
    pass.color = tex[j].colorIndex;
    pass.opacity = transLookup[tex[j].transid];

    pass.cull = (rf.flags & 4)==0 && rf.blend==0;
    pass.unlit = (rf.flags & 3)!=0;

    pass.nozwrite = pass.blendmode >= 2; //(rf.flags & 16)!=0;

    pass.trans = pass.blendmode != 0;

    pass.p = ops[geoset].v.x;

    if (animTextures) {
      if (tex[j].flags & 16) {
        pass.texanim = -1; // no texture animation
      } else {
        pass.texanim = texanimlookup[tex[j].texanimid];
      }
    } else {
      pass.texanim = -1; // no texture animation
    }

    passes.push_back(pass);
  }

  // transparent parts come later
  std::sort(passes.begin(), passes.end());
}

void Model::initStatic( MPQFile &f )
{
  origVertices = ( ModelVertex* )( f.getBuffer() + header.ofsVertices );
  
  initCommon( f );
  
  ModelDrawList = glGenLists( 1 );
  glNewList( ModelDrawList, GL_COMPILE );
    drawModel( /*false*/ );
  glEndList();

  SelectModelDrawList = glGenLists( 1 );
  glNewList( SelectModelDrawList, GL_COMPILE );
    drawModelSelect();
  glEndList();

  // clean up vertices, indices etc
  if( vertices )
    delete[] vertices;
  if( normals )
    delete[] normals;
  if( indices )
    delete[] indices;
  if( colors ) 
    delete[] colors;
  if( transparency ) 
    delete[] transparency;
}

void Model::initAnimated(MPQFile &f)
{  
  origVertices = new ModelVertex[header.nVertices];
  memcpy(origVertices, f.getBuffer() + header.ofsVertices, header.nVertices * sizeof(ModelVertex));

  glGenBuffers(1,&vbuf);
  glGenBuffers(1,&tbuf);
  const size_t size = header.nVertices * sizeof(float);
  vbufsize = 3 * size;
  
  initCommon( f );
  
  if (animBones) 
  {
    // init bones...
    bones = new Bone[header.nBones];
    ModelBoneDef *mb = (ModelBoneDef*)(f.getBuffer() + header.ofsBones);
    for (size_t i=0; i<header.nBones; i++)
    {
      bones[i].init(f, mb[i], globalSequences);
    }
  }

  if (!animGeometry) {
    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
    glBufferData(GL_ARRAY_BUFFER, vbufsize, vertices, GL_STATIC_DRAW);
    glGenBuffers(1,&nbuf);
    glBindBuffer(GL_ARRAY_BUFFER, nbuf);
    glBufferData(GL_ARRAY_BUFFER, vbufsize, normals, GL_STATIC_DRAW);
    //delete[] vertices;
    delete[] normals;
  }
  
  Vec2D *texcoords = new Vec2D[header.nVertices];
  for (size_t i=0; i<header.nVertices; i++) texcoords[i] = origVertices[i].texcoords;
  glBindBuffer(GL_ARRAY_BUFFER, tbuf);
  glBufferData(GL_ARRAY_BUFFER, 2*size, texcoords, GL_STATIC_DRAW);
  delete[] texcoords;

  if (animTextures) {
    texanims = new TextureAnim[header.nTexAnims];
    ModelTexAnimDef *ta = (ModelTexAnimDef*)(f.getBuffer() + header.ofsTexAnims);
    for (size_t i=0; i<header.nTexAnims; i++) {
      texanims[i].init(f, ta[i], globalSequences);
    }
  }

  if (header.nParticleEmitters) {
    ModelParticleEmitterDef *pdefs = (ModelParticleEmitterDef *)(f.getBuffer() + header.ofsParticleEmitters);
    particleSystems = new ParticleSystem[header.nParticleEmitters];
    for (size_t i=0; i<header.nParticleEmitters; i++) {
      particleSystems[i].model = this;
      particleSystems[i].init(f, pdefs[i], globalSequences);
    }
  }

  // ribbons
  if (header.nRibbonEmitters) {
    ModelRibbonEmitterDef *rdefs = (ModelRibbonEmitterDef *)(f.getBuffer() + header.ofsRibbonEmitters);
    ribbons = new RibbonEmitter[header.nRibbonEmitters];
    for (size_t i=0; i<header.nRibbonEmitters; i++) {
      ribbons[i].model = this;
      ribbons[i].init(f, rdefs[i], globalSequences);
    }
  }

  // just use the first camera, meh
  if (header.nCameras>0) {
    ModelCameraDef *camDefs = (ModelCameraDef*)(f.getBuffer() + header.ofsCameras);
    cam.init(f, camDefs[0], globalSequences);
  }

  // init lights
  if (header.nLights) {
    lights = new ModelLight[header.nLights];
    ModelLightDef *lDefs = (ModelLightDef*)(f.getBuffer() + header.ofsLights);
    for (size_t i=0; i<header.nLights; i++) lights[i].init(f, lDefs[i], globalSequences);
  }

  anims = new ModelAnimation[header.nAnimations];
  memcpy(anims, f.getBuffer() + header.ofsAnimations, header.nAnimations * sizeof(ModelAnimation));

  animcalc = false;
}

void Model::calcBones(int _anim, int time)
{
  for (size_t i=0; i<header.nBones; i++) {
    bones[i].calc = false;
  }

  for (size_t i=0; i<header.nBones; i++) {
    bones[i].calcMatrix(bones, _anim, time);
  }
}

void Model::animate(int _anim)
{

  ModelAnimation &a = anims[_anim];
  if (!a.Index)return;
  int t = globalTime; //(int)(gWorld->animtime / a.playSpeed);
  int tmax = a.length;
  if (tmax==0)return;
  t %= tmax;
  t += 0;
  animtime = t;
  this->anim = _anim;

  if (animBones) {
    calcBones(_anim, t);
  }

  if (animGeometry) {

    glBindBuffer(GL_ARRAY_BUFFER, vbuf);
        glBufferData(GL_ARRAY_BUFFER, 2*vbufsize, NULL, GL_STREAM_DRAW);
    vertices = (Vec3D*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    // transform vertices
    ModelVertex *ov = origVertices;
    for (size_t i=0; i<header.nVertices; ++i,++ov) {
      Vec3D v(0,0,0), n(0,0,0);

      for (size_t b=0; b<4; b++) {
        if (ov->weights[b]>0) {
          Vec3D tv = bones[ov->bones[b]].mat * ov->pos;
          Vec3D tn = bones[ov->bones[b]].mrot * ov->normal;
          v += tv * ((float)ov->weights[b] / 255.0f);
          n += tn * ((float)ov->weights[b] / 255.0f);
        }
      }

      /*
      vertices[i] = v;
      normals[i] = n;
      */             
      vertices[i] = v;
      vertices[header.nVertices + i] = n.normalize(); // shouldn't these be normal by default?
    }

        glUnmapBuffer(GL_ARRAY_BUFFER);

  }

  for (size_t i=0; i<header.nLights; i++) {
    if (lights[i].parent>=0) {
      lights[i].tpos = bones[lights[i].parent].mat * lights[i].pos;
      lights[i].tdir = bones[lights[i].parent].mrot * lights[i].dir;
    }
  }
  //TEMPORARILY
  /*for (size_t i=0; i<header.nParticleEmitters; i++) {
    // random time distribution for teh win ..?
    int pt = (t + (int)(tmax*particleSystems[i].tofs)) % tmax;
    particleSystems[i].setup(anim, pt);
  }

  for (size_t i=0; i<header.nRibbonEmitters; i++) {
    ribbons[i].setup(anim, t);
  }*/

  if (animTextures) {
    for (size_t i=0; i<header.nTexAnims; i++) {
      texanims[i].calc(_anim, t);
    }
  }
}

bool ModelRenderPass::init(Model *m)
{
  // blend mode
  switch (blendmode) {
  case BM_OPAQUE:  // 0
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    break;
  case BM_TRANSPARENT: // 1
    glDisable(GL_BLEND);
    glEnable(GL_ALPHA_TEST);
    break;
  case BM_ALPHA_BLEND: // 2
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func
    break;
  case BM_ADDITIVE: // 3
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE);
    break;
  case BM_ADDITIVE_ALPHA: // 4
    glDisable(GL_ALPHA_TEST);
     glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    break;
  default:
    // ???
    glDisable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
  }

  if (nozwrite) {
    glDepthMask(GL_FALSE);
  }

  if (cull) {
        glEnable(GL_CULL_FACE);
  } else {
        glDisable(GL_CULL_FACE);
  }
  
//  glPushName(texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  if (usetex2) {
    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture2);
  }

  if (unlit) {
    glDisable(GL_LIGHTING);
    // unfogged = unlit?
    if((blendmode==3)||(blendmode==4))
      glDisable(GL_FOG);
  }

  if (useenvmap) {
    // env mapping
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    const GLint maptype = GL_SPHERE_MAP;
    //const GLint maptype = GL_REFLECTION_MAP;

    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, maptype);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, maptype);
  }

  if (texanim!=-1) {
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();

    m->texanims[texanim].setup();
    glMatrixMode(GL_MODELVIEW);
  }

  Vec4D ocol(1,1,1,m->trans);
  Vec4D ecol(0,0,0,0);

  // emissive colors
  if (color!=-1) {
    Vec3D c = m->colors[color].color.getValue(m->anim,m->animtime);
    ocol.w *= m->colors[color].opacity.getValue(m->anim,m->animtime);
    if (unlit) {
      ocol.x = c.x; ocol.y = c.y; ocol.z = c.z;
    } else {
      ocol.x = ocol.y = ocol.z = 0;
    }
    ecol = Vec4D(c, 1.0f);
  }
  glMaterialfv(GL_FRONT, GL_EMISSION, ecol);

  // opacity
  if (opacity!=-1) {
      ocol.w *= m->transparency[opacity].trans.getValue(m->anim,m->animtime);
  }

  // color
  glColor4fv(ocol);

  if (blendmode<=1 && ocol.w!=1.0f) glEnable(GL_BLEND);

  return (ocol.w > 0) || (ecol.lengthSquared() > 0);
}

void ModelRenderPass::deinit()
{
  switch (blendmode) {
  case BM_OPAQUE:
    break;
  case BM_TRANSPARENT:
    break;
  case BM_ALPHA_BLEND:
    //glDepthMask(GL_TRUE);
    break;
  default:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func
  }
  if (nozwrite) {
    glDepthMask(GL_TRUE);
  }
  if (texanim!=-1) {
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }
  if (unlit) {
    glEnable(GL_LIGHTING);
    //if (gWorld && gWorld->drawfog) glEnable(GL_FOG);
    if((blendmode==3)||(blendmode==4))
      glEnable(GL_FOG);
  }
  if (useenvmap) {
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
  }
  if (usetex2) {
    glActiveTexture(GL_TEXTURE1);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
  }
//  glPopName();
  //glColor4f(1,1,1,1); //???
}

void ModelHighlight( Vec4D color )
{
  glDisable( GL_ALPHA_TEST );
   glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glDisable( GL_CULL_FACE );
  glActiveTexture( GL_TEXTURE0 );
  glDisable( GL_TEXTURE_2D);
  glActiveTexture( GL_TEXTURE1 );
  glDisable( GL_TEXTURE_2D );
  glColor4fv( color );
  glMaterialfv( GL_FRONT, GL_EMISSION, color );
//  glDepthMask( GL_FALSE );
}

void ModelUnhighlight()
{
  glEnable( GL_ALPHA_TEST );
   glDisable( GL_BLEND );
  glEnable( GL_CULL_FACE );
  glActiveTexture( GL_TEXTURE0 );
  glEnable( GL_TEXTURE_2D );
  glColor4fv( Vec4D( 1, 1, 1, 1 ) );
//  glDepthMask( GL_TRUE );
}

void Model::drawModel( /*bool unlit*/ )
{  
  // assume these client states are enabled: GL_VERTEX_ARRAY, GL_NORMAL_ARRAY, GL_TEXTURE_COORD_ARRAY

  if( animated ) 
  {
    if( animGeometry ) 
    {
      glBindBuffer( GL_ARRAY_BUFFER, vbuf );
      glVertexPointer( 3, GL_FLOAT, 0, 0 );
      glNormalPointer( GL_FLOAT, 0, ((char *)(0) + (vbufsize)) );
    } 
    else 
    {
      glBindBuffer( GL_ARRAY_BUFFER, vbuf );
      glVertexPointer( 3, GL_FLOAT, 0, 0 );
      glBindBuffer( GL_ARRAY_BUFFER, nbuf );
      glNormalPointer( GL_FLOAT, 0, 0 );
    }

    glBindBuffer( GL_ARRAY_BUFFER, tbuf );
    glTexCoordPointer( 2, GL_FLOAT, 0, 0 );
  }

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glAlphaFunc( GL_GREATER, 0.1f );

  for( size_t i = 0; i < passes.size(); i++ ) 
  {
    ModelRenderPass &p = passes[i];

    //if( unlit )
    //  p.unlit = true;
    
    if( p.init( this ) ) 
    {      
      if( animated ) 
      {
        glDrawRangeElements( GL_TRIANGLES, p.vertexStart, p.vertexEnd, p.indexCount, GL_UNSIGNED_SHORT, indices + p.indexStart );
      } 
      else 
      {
        glBegin( GL_TRIANGLES );
        for( size_t k = 0, b = p.indexStart; k < p.indexCount; k++, b++ ) 
        {
          uint16_t a = indices[b];
          glTexCoord2fv( origVertices[a].texcoords );
          glVertex3fv( vertices[a] );
        }
        glEnd();
      }
    }
    p.deinit();
  }
}

//! \todo  Make animated models selectable.
void Model::drawModelSelect()
{
  // assume these client states are enabled: GL_VERTEX_ARRAY, GL_NORMAL_ARRAY, GL_TEXTURE_COORD_ARRAY

  for( size_t i = 0; i < passes.size(); i++ ) 
  {
    ModelRenderPass &p = passes[i];

//    glPushName( p.texture );
    glBegin( GL_TRIANGLES );
    for( size_t k = 0, b = p.indexStart; k < p.indexCount; k++, b++ ) 
    {
      uint16_t a = indices[b];
      glVertex3fv( vertices[a] );
    }
    glEnd();
//    glPopName();
  }
}

void TextureAnim::calc(int anim, int time)
{
  if (trans.used) {
    tval = trans.getValue(anim, time);
  }
  if (rot.used) {
        rval = rot.getValue(anim, time);
  }
  if (scale.used) {
        sval = scale.getValue(anim, time);
  }
}

void TextureAnim::setup()
{
  glLoadIdentity();
  if (trans.used) {
    glTranslatef(tval.x, tval.y, tval.z);
  }
  if (rot.used) {
    glRotatef(rval.x, 0, 0, 1); // this is wrong, I have no idea what I'm doing here ;)
  }
  if (scale.used) {
    glScalef(sval.x, sval.y, sval.z);
  }
}

void ModelCamera::init(MPQFile &f, ModelCameraDef &mcd, int *global)
{
  ok = true;
    nearclip = mcd.nearclip;
  farclip = mcd.farclip;
  fov = mcd.fov;
  pos = fixCoordSystem(mcd.pos);
  target = fixCoordSystem(mcd.target);
  tPos.init(mcd.transPos, f, global);
  tTarget.init(mcd.transTarget, f, global);
  rot.init(mcd.rot, f, global);
  tPos.fix(fixCoordSystem);
  tTarget.fix(fixCoordSystem);
}

void ModelCamera::setup(int time)
{
  if (!ok) return;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(fov * 34.5f, (GLfloat)video.xres/(GLfloat)video.yres, nearclip, farclip);

  Vec3D p = pos + tPos.getValue(0, time);
  Vec3D t = target + tTarget.getValue(0, time);

  Vec3D u(0,1,0);
  //float roll = rot.getValue(0, time) / PI * 180.0f;

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(p.x, p.y, p.z, t.x, t.y, t.z, u.x, u.y, u.z);
  //glRotatef(roll, 0, 0, 1);
}

void ModelColor::init(MPQFile &f, ModelColorDef &mcd, int *global)
{
  color.init(mcd.color, f, global);
  opacity.init(mcd.opacity, f, global);
}

void ModelTransparency::init(MPQFile &f, ModelTransDef &mcd, int *global)
{
  trans.init(mcd.trans, f, global);
}

void ModelLight::init(MPQFile &f, ModelLightDef &mld, int *global)
{
  tpos = pos = fixCoordSystem(mld.pos);
  tdir = dir = Vec3D(0,1,0); // no idea
  type = mld.type;
  parent = mld.bone;
  ambColor.init(mld.ambColor, f, global);
  ambIntensity.init(mld.ambIntensity, f, global);
  diffColor.init(mld.diffcolor, f, global);
  diffIntensity.init(mld.diffintensity, f, global);
  attStart.init(mld.attStart,f,global);
  attEnd.init(mld.attEnd,f,global);
}

void ModelLight::setup(int time, GLuint l)
{
  Vec4D ambcol(ambColor.getValue(0, time) * ambIntensity.getValue(0, time), 1.0f);
  Vec4D diffcol(diffColor.getValue(0, time) * diffIntensity.getValue(0, time), 1.0f);
  Vec4D p;
  if (type==0) {
    // directional
    p = Vec4D(tdir, 0.0f);
  } else {
    // point
    p = Vec4D(tpos, 1.0f);
  }
  //gLog("Light %d (%f,%f,%f) (%f,%f,%f) [%f,%f,%f]\n", l-GL_LIGHT4, ambcol.x, ambcol.y, ambcol.z, diffcol.x, diffcol.y, diffcol.z, p.x, p.y, p.z);
  glLightfv(l, GL_POSITION, p);
  glLightfv(l, GL_DIFFUSE, diffcol);
  glLightfv(l, GL_AMBIENT, ambcol);
  glEnable(l);
}

void TextureAnim::init(MPQFile &f, ModelTexAnimDef &mta, int *global)
{
  trans.init(mta.trans, f, global);
  rot.init(mta.rot, f, global);
  scale.init(mta.scale, f, global);
}

void Bone::init(MPQFile &f, ModelBoneDef &b, int *global)
{
  parent = b.parent;
  pivot = fixCoordSystem(b.pivot);
  billboard = (b.flags & 8) != 0;

  trans.init(b.translation, f, global);
  rot.init(b.rotation, f, global);
  scale.init(b.scaling, f, global);
  trans.fix(fixCoordSystem);
  rot.fix(fixCoordSystemQuat);
  scale.fix(fixCoordSystem2);
}

void Bone::calcMatrix(Bone *allbones, int anim, int time)
{
  if (calc) return;
  Matrix m;
  Quaternion q;

  bool tr = rot.used || scale.used || trans.used || billboard;
  if (tr) {
    m.translation(pivot);
    
    if (trans.used) {
      m *= Matrix::newTranslation(trans.getValue(anim, time));
    }
    if (rot.used) {
      q = rot.getValue(anim, time);
      m *= Matrix::newQuatRotate(q);
    }
    if (scale.used) {
      m *= Matrix::newScale(scale.getValue(anim, time));
    }
    if (billboard) {
      Matrix mtrans;
      glGetFloatv(GL_MODELVIEW_MATRIX, &(mtrans.m[0][0]));
      mtrans.transpose();
      mtrans.invert();
      Vec3D camera = mtrans * Vec3D(0,0,0);
      Vec3D look = (camera - pivot).normalize();
      //Vec3D up(0,1,0);
      Vec3D up = ((mtrans * Vec3D(0,1,0)) - camera).normalize();
      // these should be normalized by default but fp inaccuracy kicks in when looking down :(
      Vec3D right = (up % look).normalize();
      up = (look % right).normalize();

      // calculate a billboard matrix
      Matrix mbb;
      mbb.unit();
      mbb.m[0][2] = right.x;
      mbb.m[1][2] = right.y;
      mbb.m[2][2] = right.z;
      mbb.m[0][1] = up.x;
      mbb.m[1][1] = up.y;
      mbb.m[2][1] = up.z;
      mbb.m[0][0] = look.x;
      mbb.m[1][0] = look.y;
      mbb.m[2][0] = look.z;
      /*
      mbb.m[0][1] = right.x;
      mbb.m[1][1] = right.y;
      mbb.m[2][1] = right.z;
      mbb.m[0][2] = up.x;
      mbb.m[1][2] = up.y;
      mbb.m[2][2] = up.z;
      mbb.m[0][0] = look.x;
      mbb.m[1][0] = look.y;
      mbb.m[2][0] = look.z;
      */
      m *= mbb;
    }

    m *= Matrix::newTranslation(pivot*-1.0f);
    
  } else m.unit();

  if (parent>=0) {
    allbones[parent].calcMatrix(allbones, anim, time);
    mat = allbones[parent].mat * m;
  } else mat = m;

  // transform matrix for normal vectors ... ??
  if (rot.used) {
    if (parent>=0) {
      mrot = allbones[parent].mrot * Matrix::newQuatRotate(q);
    } else mrot = Matrix::newQuatRotate(q);
  } else mrot.unit();

  calc = true;

}


void Model::draw()
{
  if( !animated ) 
  {
    glCallList( ModelDrawList );
  } 
  else 
  {
    if( !animcalc || mPerInstanceAnimation ) 
    {
      animate( 0 );
      animcalc = true;
    }

    lightsOn( GL_LIGHT4 );
        drawModel( /*false*/ );
    lightsOff( GL_LIGHT4 );

    // effects are unfogged..?
    if( gWorld && gWorld->drawfog ) 
      glDisable( GL_FOG );

    // draw particle systems & ribbons
    for( size_t i = 0; i < header.nParticleEmitters; i++ ) 
      particleSystems[i].draw();

    for( size_t i = 0; i < header.nRibbonEmitters; i++ ) 
      ribbons[i].draw();

    if( gWorld && gWorld->drawfog ) 
      glEnable( GL_FOG );
  }
}

/*void Model::drawTileMode()
{
  if (!animated) {
    glCallList(TileModeModelDrawList);
  } 
  else 
  {
      if (!animcalc || mPerInstanceAnimation) {
        animate(0);
        animcalc = true;
      }
    drawModel( true );

    // effects are unfogged..?
    if( gWorld && gWorld->drawfog ) 
      glDisable( GL_FOG );

    // draw particle systems & ribbons
    for (size_t i=0; i<header.nParticleEmitters; i++)
      particleSystems[i].draw();

    for (size_t i=0; i<header.nRibbonEmitters; i++)
      ribbons[i].draw();

    if( gWorld && gWorld->drawfog ) 
      glEnable( GL_FOG );
  }
}*/

void Model::drawSelect()
{
  if( !animated )
    glCallList(SelectModelDrawList);
  else 
  {
      if( !animcalc || mPerInstanceAnimation )
      {
        animate( 0 );
        animcalc = true;
      }
    
        drawModelSelect();
    
    // effects are unfogged..?
    glDisable( GL_FOG );

    // draw particle systems
    for( size_t i = 0; i < header.nParticleEmitters; i++ )
      particleSystems[i].draw();
    
    // draw ribbons
    for( size_t i = 0; i < header.nRibbonEmitters; i++ )
      ribbons[i].draw();
    
    if( gWorld && gWorld->drawfog ) 
      glEnable(GL_FOG);
  }
}


void Model::lightsOn( GLuint lbase )
{
  for( size_t i = 0, l = lbase; i < header.nLights; i++ ) 
    lights[i].setup( animtime, l++ );
}

void Model::lightsOff( GLuint lbase )
{
  for( size_t i = 0, l = lbase; i < header.nLights; i++ ) 
    glDisable( l++ );
}

void Model::updateEmitters( float dt )
{
  for( size_t i = 0; i < header.nParticleEmitters; i++ )
    particleSystems[i].update( dt );
}

#include "AsyncLoader.h"
#include "noggit.h" // gAsyncLoader

int ModelManager::baseid = 0;

MODELIDTYPE ModelManager::add( const std::string& name )
{
  int id;
  std::string name_ = name;
	std::transform( name_.begin(), name_.end(), name_.begin(), ::tolower );
	if( names.find( name_ ) != names.end() ) 
	{
		id = names[name_];
		items[id]->addref();
		return id;
	}

  id = nextID();
  Model *model = new Model( name );
  model->finishLoading();
  
  gAsyncLoader->addObject( model );

  do_add( name, id, model );
  return id;
}
void ModelManager::resetAnim()
{
  for( std::map<std::string, int>::iterator it = names.begin( ); it != names.end( ); ++it )
    reinterpret_cast<Model*>( items[it->second] )->animcalc = false;
}

void ModelManager::updateEmitters( float dt )
{
  for( std::map<std::string, int>::iterator it = names.begin( ); it != names.end( ); ++it )
    reinterpret_cast<Model*>( items[it->second] )->updateEmitters( dt );
}
