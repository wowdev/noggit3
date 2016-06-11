// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <algorithm>
#include <cassert>
#include <map>
#include <sstream>
#include <string>

#include <math/vector_2d.h>

#include <noggit/application.h>
#include <noggit/blp_texture.h>
#include <noggit/Log.h>
#include <noggit/Model.h>
#include <noggit/World.h>

#include <opengl/context.h>
#include <opengl/matrix.h>

Model::Model(const std::string& filename, bool _forceAnim)
: forceAnim(_forceAnim)
, _filename( filename )
, _finished_upload(false)
{
  memset( &header, 0, sizeof( ModelHeader ) );

  globalSequences = nullptr;
  indices = nullptr;
  anims = nullptr;
  origVertices = nullptr;

  showGeosets = nullptr;

  _finished = false;

  noggit::app().async_loader().add_object (this);
}

void Model::finish_loading()
{
  noggit::mpq::file f (QString::fromStdString (_filename));

  LogDebug << "Loading model \"" << _filename << "\"." << std::endl;

  memcpy( &header, f.getBuffer(), sizeof( ModelHeader ) );

  animated = isAnimated( f ) || forceAnim;  // isAnimated will set animGeometry and animTextures

  trans = 1.0f;

  vbuf = nbuf = tbuf = 0;

  globalSequences = 0;
  anim = 0;
  header.nParticleEmitters = 0;      //! \todo  Get Particles to 3.*? ._.
  header.nRibbonEmitters = 0;      //! \todo  Get Particles to 3.*? ._.
  if (header.nGlobalSequences)
  {
    globalSequences = new int[header.nGlobalSequences];
    memcpy(globalSequences, (f.getBuffer() + header.ofsGlobalSequences), header.nGlobalSequences * 4);
  }

  //! \todo  This takes a biiiiiit long. Have a look at this.
  initCommon(f);

  if(animated)
    initAnimated(f);

  f.close();

  _finished = true;
}

Model::~Model()
{
  LogDebug << "Unloading model \"" << _filename << "\"." << std::endl;

  if( globalSequences )
    delete[] globalSequences;

  if(showGeosets)
    delete[] showGeosets;

  // unload all sorts of crap
  //delete[] vertices;
  //delete[] normals;
  if (indices)
    delete[] indices;
  if (anims)
    delete[] anims;
  if (origVertices)
    delete[] origVertices;
  if (!animGeometry)
  {
    gl.deleteBuffers(1, &nbuf);
  }
  gl.deleteBuffers(1, &vbuf);
  gl.deleteBuffers(1, &tbuf);
}


bool Model::isAnimated(const noggit::mpq::file& f)
{
  // see if we have any animated bones
  ModelBoneDef *bo = reinterpret_cast<ModelBoneDef*>(f.getBuffer() + header.ofsBones);

  animGeometry = false;
  animBones = false;
  mPerInstanceAnimation = false;

  ModelVertex *verts = reinterpret_cast<ModelVertex*>(f.getBuffer() + header.ofsVertices);
  for (size_t i=0; i<header.nVertices && !animGeometry; ++i) {
    for (size_t b=0; b<4; b++) {
      if (verts[i].weights[b]>0) {
        ModelBoneDef& bb = bo[verts[i].bones[b]];
        if (bb.translation.type || bb.rotation.type || bb.scaling.type || (bb.flags & 8)) {
          if (bb.flags & 8) {
            // if we have billboarding, the model will need per-instance animation
            mPerInstanceAnimation = true;
          }
          animGeometry = true;
          break;
        }
      }
    }
  }

  if (animGeometry) animBones = true;
  else {
    for (size_t i=0; i<header.nBones; ++i) {
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
    ModelColorDef *cols = reinterpret_cast<ModelColorDef*>(f.getBuffer() + header.ofsColors);
    for (size_t i=0; i<header.nColors; ++i) {
      if (cols[i].color.type!=0 || cols[i].opacity.type!=0) {
        animMisc = true;
        break;
      }
    }
  }

  // animated opacity
  if (header.nTransparency && !animMisc) {
    ModelTransDef *trs = reinterpret_cast<ModelTransDef*>(f.getBuffer() + header.ofsTransparency);
    for (size_t i=0; i<header.nTransparency; ++i) {
      if (trs[i].trans.type!=0) {
        animMisc = true;
        break;
      }
    }
  }

  // guess not...
  return animGeometry || animTextures || animMisc;
}

::math::vector_3d fixCoordSystem (::math::vector_3d const& v)
{
  static math::matrix_4x4 const mat { 1.0f,  0.0f, 0.0f, 0.0f
                                    , 0.0f,  0.0f, 1.0f, 0.0f
                                    , 0.0f, -1.0f, 0.0f, 0.0f
                                    , 0.0f,  0.0f, 0.0f, 1.0f
                                    };
  return mat * v;
}
math::vector_3d convert_rotation (math::vector_3d const& r)
{
  static math::matrix_4x4 const mat { math::matrix_4x4 {  0.0f, 0.0f, 1.0f, 0.0f
                                                       ,  0.0f, 1.0f, 0.0f, 0.0f
                                                       , -1.0f, 0.0f, 0.0f, 0.0f
                                                       ,  0.0f, 0.0f, 0.0f, 1.0f
                                                       }
                                    * math::matrix_4x4
                                        (math::matrix_4x4::translation, {0.0f, -90.0f, 0.0f})
                                    };

  return mat * r;
}
math::quaternion convert_rotation (math::quaternion const& q)
{
  static math::matrix_4x4 const mat { -1.0f, 0.0f,  0.0f, 0.0f
                                    ,  0.0f, 0.0f, -1.0f, 0.0f
                                    ,  0.0f, 1.0f,  0.0f, 0.0f
                                    ,  0.0f, 0.0f,  0.0f, 1.0f
                                    };
  return mat * q;
}
namespace
{
  ::math::vector_3d fixCoordSystem2 (::math::vector_3d const& v)
  {
    static math::matrix_4x4 const mat { 1.0f, 0.0f, 0.0f, 0.0f
                                      , 0.0f, 0.0f, 1.0f, 0.0f
                                      , 0.0f, 1.0f, 0.0f, 0.0f
                                      , 0.0f, 0.0f, 0.0f, 1.0f
                                      };
    return mat * v;
  }
}

void Model::initCommon(const noggit::mpq::file& f)
{
  origVertices = new ModelVertex[header.nVertices];
  memcpy(origVertices, f.getBuffer() + header.ofsVertices, header.nVertices * sizeof(ModelVertex));

  // assume: origVertices already set
  if (!animGeometry) {
    vertices = new ::math::vector_3d[header.nVertices];
    normals = new ::math::vector_3d[header.nVertices];
  }

  // vertices, normals
  for (size_t i=0; i<header.nVertices; ++i) {
    origVertices[i].pos = fixCoordSystem(origVertices[i].pos);
    origVertices[i].normal = fixCoordSystem(origVertices[i].normal);

    if (!animGeometry) {
      vertices[i] = origVertices[i].pos;
      normals[i] = origVertices[i].normal.normalize();
    }

    float len = origVertices[i].pos.length_squared();
    if (len > rad){
      rad = len;
    }
  }
  rad = std::sqrt(rad);

  // textures
  ModelTextureDef* texdef = reinterpret_cast<ModelTextureDef*>( f.getBuffer() + header.ofsTextures );

  for( size_t i = 0; i < header.nTextures; ++i )
  {
    if( texdef[i].type == 0 )
    {
      _texture_names.emplace_back (std::string (f.getBuffer() + texdef[i].nameOfs, texdef[i].nameLen));
    }
    else
    {
      //! \note special texture - only on characters and such... Noggit should
      //! not even render these. probably should be replaced more intelligent.
      _texture_names.emplace_back ( texdef[i].type == 3
                             ? "item/weapon/armorreflect4.blp"
                             : "tileset/generic/checkers.blp"
                             );
    }
  }

  // init colors
  if (header.nColors) {
    ModelColorDef *colorDefs = reinterpret_cast<ModelColorDef*>(f.getBuffer() + header.ofsColors);
    for (size_t i=0; i<header.nColors; ++i)
    {
      colors.emplace_back (f, colorDefs[i], globalSequences);
    }
  }
  // init transparency
  int16_t *transLookup = reinterpret_cast<int16_t*>(f.getBuffer() + header.ofsTransparencyLookup);
  if (header.nTransparency) {
    ModelTransDef *trDefs = reinterpret_cast<ModelTransDef*>(f.getBuffer() + header.ofsTransparency);
    for (size_t i=0; i<header.nTransparency; ++i)
    {
      transparency.emplace_back (f, trDefs[i], globalSequences);
    }
  }


  // just use the first LOD/view

  if (header.nViews > 0) {
    // indices - allocate space, too
    std::string lodname (_filename.substr (0, _filename.length() - 3));
    lodname.append("00.skin");
    noggit::mpq::file g (QString::fromStdString (lodname));

    ModelViewStruct *view = reinterpret_cast<ModelViewStruct*>(g.getBuffer());

    uint16_t *indexLookup = reinterpret_cast<uint16_t*>(g.getBuffer() + view->ofsIndex);
    uint16_t *triangles = reinterpret_cast<uint16_t*>(g.getBuffer() + view->ofsTris);
    nIndices = view->nTris;
    indices = new uint16_t[nIndices];
    for (size_t i = 0; i<nIndices; ++i) {
      indices[i] = indexLookup[triangles[i]];
    }

    // render ops
    ModelGeoset *ops = reinterpret_cast<ModelGeoset*>(g.getBuffer() + view->ofsSub);
    ModelTexUnit *tex = reinterpret_cast<ModelTexUnit*>(g.getBuffer() + view->ofsTex);
    ModelRenderFlags *renderFlags = reinterpret_cast<ModelRenderFlags*>(f.getBuffer() + header.ofsTexFlags);
    uint16_t *texlookup = reinterpret_cast<uint16_t*>(f.getBuffer() + header.ofsTexLookup);
    uint16_t *texanimlookup = reinterpret_cast<uint16_t*>(f.getBuffer() + header.ofsTexAnimLookup);
    int16_t *texunitlookup = reinterpret_cast<int16_t*>(f.getBuffer() + header.ofsTexUnitLookup);

    showGeosets = new bool[view->nSub];
    for (size_t i=0; i<view->nSub; ++i) {
      showGeosets[i] = true;
    }

    for (size_t j = 0; j<view->nTex; j++) {
      ModelRenderPass pass;

      pass.usetex2 = false;
      pass.useenvmap = false;
      pass.cull = false;
      pass.trans = false;
      pass.unlit = false;
      pass.nozwrite = false;
      pass.billboard = false;

      size_t geoset = tex[j].op;

      pass.geoset = geoset;

      pass.indexStart = ops[geoset].istart;
      pass.indexCount = ops[geoset].icount;
      pass.vertexStart = ops[geoset].vstart;
      pass.vertexEnd = pass.vertexStart + ops[geoset].vcount;

      pass.order = tex[j].shading;

      //TextureID texid = textures[texlookup[tex[j].textureid]];
      //pass.texture = texid;
      pass.tex = texlookup[tex[j].textureid];

      // TODO: figure out these flags properly -_-
      ModelRenderFlags &rf = renderFlags[tex[j].flagsIndex];


      pass.blendmode = rf.blend;
      pass.color = tex[j].colorIndex;
      pass.opacity = transLookup[tex[j].transid];

      enum RenderFlags
      {
        RENDERFLAGS_UNLIT = 1,
        RENDERFLAGS_UNsGED = 2,
        RENDERFLAGS_TWOSIDED = 4,
        RENDERFLAGS_BILLBOARD = 8,
        RENDERFLAGS_ZBUFFERED = 16,
      };

      pass.unlit = (rf.flags & RENDERFLAGS_UNLIT)!=0;
      pass.cull = (rf.flags & RENDERFLAGS_TWOSIDED)==0 && rf.blend==0;

      pass.billboard = (rf.flags & RENDERFLAGS_BILLBOARD) != 0;

      pass.useenvmap = (texunitlookup[tex[j].texunit] == -1) && pass.billboard && rf.blend>2;
      pass.nozwrite = (rf.flags & RENDERFLAGS_ZBUFFERED) != 0;

      // ToDo: Work out the correct way to get the true/false of transparency
      pass.trans = (pass.blendmode>0) && (pass.opacity>0);  // Transparency - not the correct way to get transparency

      pass.p = ops[geoset].BoundingBox[0].x();

      // Texture flags
      enum TextureFlags {
        TEXTURE_WRAPX=1,
        TEXTURE_WRAPY
      };

      pass.swrap = (texdef[pass.tex].flags & TEXTURE_WRAPX) != 0; // Texture wrap X
      pass.twrap = (texdef[pass.tex].flags & TEXTURE_WRAPY) != 0; // Texture wrap Y

      static const int TEXTUREUNIT_STATIC = 16;

      if (animTextures) {
        if (tex[j].flags & TEXTUREUNIT_STATIC) {
          pass.texanim = -1; // no texture animation
        } else {
          pass.texanim = texanimlookup[tex[j].texanimid];
        }
      } else {
        pass.texanim = -1; // no texture animation
      }

      passes.push_back(pass);
    }
    g.close();
    // transparent parts come later
    std::sort(passes.begin(), passes.end());
  }

  // zomg done
}

void Model::initAnimated(const noggit::mpq::file& f)
{
  if (header.nAnimations > 0) {
    anims = new ModelAnimation[header.nAnimations];
    memcpy(anims, f.getBuffer() + header.ofsAnimations, header.nAnimations * sizeof(ModelAnimation));
    for( size_t i = 0; i < header.nAnimations; ++i )
    {
      //! \note Fix for world\kalimdor\diremaul\activedoodads\crystalcorrupter\corruptedcrystalshard.m2 having a zero length for its stand animation.
      anims[i].length = std::max( anims[i].length, 1U );
    }

    animfiles = new noggit::mpq::file*[header.nAnimations];

    std::stringstream tempname;
    for(size_t i=0; i<header.nAnimations; ++i)
    {
      std::string lodname = _filename.substr(0, _filename.length()-3);
      tempname << lodname << anims[i].animID << "-" << anims[i].subAnimID;
      const QString anim_filename (QString::fromStdString (tempname.str()));
      if (noggit::mpq::file::exists (anim_filename))
      {
        animfiles[i] = new noggit::mpq::file (anim_filename);
      }
      else
      {
        animfiles[i] = nullptr;
      }
    }
  }

  if (animBones) {
    // init bones...
    ModelBoneDef *mb = reinterpret_cast<ModelBoneDef*>(f.getBuffer() + header.ofsBones);
    for (size_t i=0; i<header.nBones; ++i) {
      bones.emplace_back (f, mb[i], globalSequences, animfiles);
    }
  }

  if (animTextures) {
    ModelTexAnimDef *ta = reinterpret_cast<ModelTexAnimDef*>(f.getBuffer() + header.ofsTexAnims);
    for (size_t i=0; i<header.nTexAnims; ++i) {
      texanims.emplace_back (f, ta[i], globalSequences);
    }
  }

  // particle systems
  if (header.nParticleEmitters) {
    ModelParticleEmitterDef *pdefs = reinterpret_cast<ModelParticleEmitterDef*>(f.getBuffer() + header.ofsParticleEmitters);
    for (size_t i=0; i<header.nParticleEmitters; ++i) {
      particleSystems.emplace_back (this, f, pdefs[i], globalSequences);
    }
  }

  // ribbons
  if (header.nRibbonEmitters) {
    ModelRibbonEmitterDef *rdefs = reinterpret_cast<ModelRibbonEmitterDef*>(f.getBuffer() + header.ofsRibbonEmitters);
    for (size_t i=0; i<header.nRibbonEmitters; ++i) {
      ribbons.emplace_back (this, f, rdefs[i], globalSequences);
    }
  }

  // just use the first camera, meh
  if (header.nCameras>0) {
    ModelCameraDef *camDefs = reinterpret_cast<ModelCameraDef*>(f.getBuffer() + header.ofsCameras);
    cam = ModelCamera (f, camDefs[0], globalSequences);
  }

  // init lights
  if (header.nLights) {
    ModelLightDef *lDefs = reinterpret_cast<ModelLightDef*>(f.getBuffer() + header.ofsLights);
    for (size_t i=0; i<header.nLights; ++i)
      lights.emplace_back (f, lDefs[i], globalSequences);
  }

  animcalc = false;
}

void Model::calcBones(int _anim, int time)
{
  for (size_t i=0; i<header.nBones; ++i) {
    bones[i].calc = false;
  }

  for (size_t i=0; i<header.nBones; ++i) {
    bones[i].calcMatrix(bones.data(), _anim, time);
  }
}

void Model::animate(int _anim, int time)
{
  anim = _anim;
  ModelAnimation &a = anims[anim];
  int tmax = a.length;
  time %= tmax;

  if (animBones) {
    calcBones(anim, time);
  }

  if (animGeometry) {

    gl.bindBuffer(GL_ARRAY_BUFFER_ARB, vbuf);
    gl.bufferData(GL_ARRAY_BUFFER_ARB, 2*vbufsize, nullptr, GL_STREAM_DRAW_ARB);
    vertices = reinterpret_cast< ::math::vector_3d*>(gl.mapBuffer(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY));

    // transform vertices
    ModelVertex *ov = origVertices;
    for (size_t i=0; i<header.nVertices; ++i,++ov) {
      ::math::vector_3d v(0,0,0), n(0,0,0);

      for (size_t b=0; b<4; ++b) {
        if (ov->weights[b]>0) {
          ::math::vector_3d tv = bones[ov->bones[b]].mat * ov->pos;
          ::math::vector_3d tn = bones[ov->bones[b]].mrot * ov->normal;
          v += tv * (static_cast<float>(ov->weights[b]) / 255.0f);
          n += tn * (static_cast<float>(ov->weights[b]) / 255.0f);
        }
      }

      /*
       vertices[i] = v;
       normals[i] = n;
       */
      vertices[i] = v;
      vertices[header.nVertices + i] = n.normalize(); // shouldn't these be normal by default?
    }

    gl.unmapBuffer(GL_ARRAY_BUFFER_ARB);

  }

  for (size_t i=0; i<header.nLights; ++i) {
    if (lights[i].parent>=0) {
      lights[i].tpos = bones[lights[i].parent].mat * lights[i].pos;
      lights[i].tdir = bones[lights[i].parent].mrot * lights[i].dir;
    }
  }

  for (size_t i=0; i<header.nParticleEmitters; ++i) {
    // random time distribution for teh win ..?
    int pt = (time + static_cast<int>(tmax*particleSystems[i].tofs)) % tmax;
    particleSystems[i].setup(anim, pt);
  }

  for (size_t i=0; i<header.nRibbonEmitters; ++i) {
    ribbons[i].setup(anim, time);
  }

  if (animTextures) {
    for (size_t i=0; i<header.nTexAnims; ++i) {
      texanims[i].calc(anim, time);
    }
  }
}

bool ModelRenderPass::init(Model *m, int animtime)
{
  // May aswell check that we're going to render the geoset before doing all this crap.
  if (m->showGeosets[geoset]) {

    // COLOUR
    // Get the colour and transparency and check that we should even render
    ocol = ::math::vector_4d(1.0f, 1.0f, 1.0f, m->trans);
    ecol = ::math::vector_4d(0.0f, 0.0f, 0.0f, 0.0f);

    //if (m->trans == 1.0f)
    //  return false;

    // emissive colors
    if (color!=-1 && m->colors[color].color.uses(0))
    {
      ::math::vector_3d c (m->colors[color].color.getValue (0, animtime));
      if (m->colors[color].opacity.uses (m->anim))
      {
        ocol.w (m->colors[color].opacity.getValue (m->anim, animtime));
      }

      if (unlit)
      {
        ocol.x (c.x());
        ocol.y (c.y());
        ocol.z (c.z());
      } else {
        ocol.x (ocol.y (ocol.z (0.0f)));
      }

      ecol = ::math::vector_4d (c, ocol.w());
      gl.materialfv (GL_FRONT, GL_EMISSION, ecol);
    }

    // opacity
    if (opacity!=-1)
    {
      if (m->transparency[opacity].trans.uses (0))
      {
        ocol.w ( ocol.w()
               * m->transparency[opacity].trans.getValue (0, animtime)
               );
      }
    }

    // exit and return false before affecting the opengl render state
    if (!((ocol.w() > 0.0f) && (color==-1 || ecol.w() > 0.0f)))
      return false;

    // TEXTURE
    m->_textures[tex]->bind();

    // TODO: Add proper support for multi-texturing.

    // blend mode
    switch (blendmode) {
      case BM_OPAQUE:  // 0
        break;
      case BM_TRANSPARENT: // 1
        gl.enable(GL_ALPHA_TEST);
        gl.alphaFunc(GL_GEQUAL,0.7f);
        break;
      case BM_ALPHA_BLEND: // 2
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
      case BM_ADDITIVE: // 3
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_SRC_COLOR, GL_ONE);
        break;
      case BM_ADDITIVE_ALPHA: // 4
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
      case BM_MODULATE: // 5
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        break;
      case BM_MODULATE2: // 6
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_DST_COLOR,GL_SRC_COLOR);
        break;
      default:
        LogError << "Unknown blendmode: " << blendmode << std::endl;
        gl.enable(GL_BLEND);
        gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    //if (cull)
    //  gl.enable(GL_CULL_FACE);

    // Texture wrapping around the geometry
    if (swrap)
      gl.texParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    if (twrap)
      gl.texParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

    // no writing to the depth buffer.
    if (nozwrite)
      gl.depthMask(GL_FALSE);

    if (unlit) {
      gl.disable(GL_LIGHTING);
    }

    // Environmental mapping, material, and effects
    if (useenvmap) {
      // Turn on the 'reflection' shine, using 18.0f as that is what WoW uses based on the reverse engineering
      // This is now set in InitGL(); - no need to call it every render.
      gl.materialf(GL_FRONT_AND_BACK, GL_SHININESS, 18.0f);

      // env mapping
      gl.enable(GL_TEXTURE_GEN_S);
      gl.enable(GL_TEXTURE_GEN_T);

      const GLint maptype = GL_SPHERE_MAP;
      //const GLint maptype = GL_REFLECTION_MAP_ARB;

      gl.texGeni(GL_S, GL_TEXTURE_GEN_MODE, maptype);
      gl.texGeni(GL_T, GL_TEXTURE_GEN_MODE, maptype);
    }

    if (texanim!=-1) {
      gl.matrixMode(GL_TEXTURE);
      gl.pushMatrix();

      m->texanims[texanim].setup(texanim);
    }

    // color
    gl.color4fv(ocol);
    //gl.materialfv(GL_FRONT, GL_SPECULAR, ocol);

    // don't use lighting on the surface
    if (unlit)
      gl.disable(GL_LIGHTING);

    if (blendmode<=1 && ocol.w()<1.0f)
      gl.enable(GL_BLEND);

    return true;
  }

  return false;
}

void ModelRenderPass::deinit()
{
  switch (blendmode) {
    case BM_OPAQUE:
      break;
    case BM_TRANSPARENT:
      break;
    case BM_ALPHA_BLEND:
      //gl.depthMask(GL_TRUE);
      break;
    default:
      gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default blend func
  }
  if (nozwrite) {
    gl.depthMask(GL_TRUE);
  }
  if (texanim!=-1) {
    gl.popMatrix();
    gl.matrixMode(GL_MODELVIEW);
  }
  if (unlit) {
    gl.enable(GL_LIGHTING);
  }
  if (useenvmap) {
    gl.disable(GL_TEXTURE_GEN_S);
    gl.disable(GL_TEXTURE_GEN_T);
  }
  if (usetex2) {
    gl.disable(GL_TEXTURE_2D);
    gl.activeTexture(GL_TEXTURE0);
  }
  //gl.color4f(1,1,1,1); //???
}

void ModelHighlight( ::math::vector_4d color )
{
  gl.disable( GL_ALPHA_TEST );
  gl.enable( GL_BLEND );
  gl.blendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  gl.disable( GL_CULL_FACE );
  gl.activeTexture( GL_TEXTURE0 );
  gl.disable( GL_TEXTURE_2D);
  gl.activeTexture( GL_TEXTURE1 );
  gl.disable( GL_TEXTURE_2D );
  gl.color4fv( color );
  gl.materialfv( GL_FRONT, GL_EMISSION, color );
  //  gl.depthMask( GL_FALSE );
}

void ModelUnhighlight()
{
  gl.enable( GL_ALPHA_TEST );
  gl.disable( GL_BLEND );
  gl.enable( GL_CULL_FACE );
  gl.activeTexture( GL_TEXTURE0 );
  gl.enable( GL_TEXTURE_2D );
  gl.color4fv( ::math::vector_4d( 1, 1, 1, 1 ) );
  //  gl.depthMask( GL_TRUE );
}

void Model::drawModel(int animtime)
{
  // assume these client states are enabled: GL_VERTEX_ARRAY, GL_NORMAL_ARRAY, GL_TEXTURE_COORD_ARRAY

  if (animGeometry)
  {
    gl.bindBuffer(GL_ARRAY_BUFFER, vbuf);
    gl.vertexPointer(3, GL_FLOAT, 0, 0);
    gl.normalPointer(GL_FLOAT, 0, reinterpret_cast<void*>(vbufsize));
  }
  else
  {
    gl.bindBuffer(GL_ARRAY_BUFFER, vbuf);
    gl.vertexPointer(3, GL_FLOAT, 0, 0);
    gl.bindBuffer(GL_ARRAY_BUFFER, nbuf);
    gl.normalPointer(GL_FLOAT, 0, 0);
  }

  gl.bindBuffer(GL_ARRAY_BUFFER, tbuf);
  gl.texCoordPointer(2, GL_FLOAT, 0, 0);

  gl.blendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  gl.alphaFunc( GL_GREATER, 0.3f );

  for (size_t i = 0; i < passes.size(); ++i)
  {
    ModelRenderPass& p = passes[i];

    if (p.init(this, animtime))
    {
      // we don't want to render completely transparent parts

      //gl.drawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_SHORT, indices + p.indexStart);
      // a GDC OpenGL Performace Tuning paper recommended gl.drawRangeElements over gl.drawElements
      // I can't notice a difference but I guess it can't hurt
      gl.drawRangeElements(GL_TRIANGLES, p.vertexStart, p.vertexEnd, p.indexCount, GL_UNSIGNED_SHORT, indices + p.indexStart);

      p.deinit();
    }
  }
  // done with all render ops

  gl.alphaFunc (GL_GREATER, 0.0f);
  gl.disable (GL_ALPHA_TEST);

  GLfloat czero[4] = {0,0,0,1};
  gl.materialfv(GL_FRONT, GL_EMISSION, czero);
  gl.color4f(1,1,1,1);
  gl.depthMask(GL_TRUE);
}

void Model::drawModelSelect (int animtime)
{
  // assume these client states are enabled: GL_VERTEX_ARRAY, GL_NORMAL_ARRAY, GL_TEXTURE_COORD_ARRAY

  if (animGeometry)
  {
    gl.bindBuffer(GL_ARRAY_BUFFER, vbuf);
    gl.vertexPointer(3, GL_FLOAT, 0, 0);
    gl.normalPointer(GL_FLOAT, 0, reinterpret_cast<void*>(vbufsize));
  }
  else
  {
    gl.bindBuffer(GL_ARRAY_BUFFER, vbuf);
    gl.vertexPointer(3, GL_FLOAT, 0, 0);
    gl.bindBuffer(GL_ARRAY_BUFFER, nbuf);
    gl.normalPointer(GL_FLOAT, 0, 0);
  }

  gl.bindBuffer(GL_ARRAY_BUFFER, tbuf);
  gl.texCoordPointer(2, GL_FLOAT, 0, 0);

  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.alphaFunc(GL_GREATER, 0.3f);

  for (size_t i = 0; i < passes.size(); ++i)
  {
    ModelRenderPass& p = passes[i];

    if (p.init(this, animtime))
    {
      // we don't want to render completely transparent parts

      //gl.drawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_SHORT, indices + p.indexStart);
      // a GDC OpenGL Performace Tuning paper recommended gl.drawRangeElements over gl.drawElements
      // I can't notice a difference but I guess it can't hurt
      gl.drawRangeElements(GL_TRIANGLES, p.vertexStart, p.vertexEnd, p.indexCount, GL_UNSIGNED_SHORT, indices + p.indexStart);

      p.deinit();
    }
  }
  // done with all render ops

  gl.alphaFunc (GL_GREATER, 0.0f);
  gl.disable (GL_ALPHA_TEST);

  GLfloat czero[4] = {0,0,0,1};
  gl.materialfv(GL_FRONT, GL_EMISSION, czero);
  gl.color4f(1,1,1,1);
  gl.depthMask(GL_TRUE);

}

void TextureAnim::calc(int anim, int time)
{
  if (trans.uses(anim)) {
    tval = trans.getValue(anim, time);
  }
  if (rot.uses(anim)) {
        rval = rot.getValue(anim, time);
  }
  if (scale.uses(anim)) {
        sval = scale.getValue(anim, time);
  }
}

void TextureAnim::setup(int anim)
{
  gl.loadIdentity();
  if (trans.uses(anim)) {
    gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::translation, tval).transposed());
  }
  if (rot.uses(anim)) {
    // this is wrong, I have no idea what I'm doing here ;)
    gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::rotation, math::vector_3d {rval.x(), 0.0f, 0.0f}).transposed());
  }
  if (scale.uses(anim)) {
    gl.multMatrixf (math::matrix_4x4 (math::matrix_4x4::scale, sval));
  }
}

ModelCamera::ModelCamera(const noggit::mpq::file& f, const ModelCameraDef &mcd, int *global)
  : pos (fixCoordSystem(mcd.pos))
  , target (fixCoordSystem(mcd.target))
  , nearclip (mcd.nearclip)
  , farclip (mcd.farclip)
  , fov (mcd.fov)
  , tPos (mcd.transPos, f, global)
  , tTarget (mcd.transTarget, f, global)
  , rot (mcd.rot, f, global)
{
  tPos.apply(fixCoordSystem);
  tTarget.apply(fixCoordSystem);
}

void ModelCamera::setup( int time )
{
  //! \todo This really needs to be done differently.
/*
  video.fov( fov * 34.5f );
  video.nearclip( nearclip );
  video.farclip( farclip );
  video.updateProjectionMatrix();

  opengl::matrix::look_at ( pos + tPos.getValue( 0, time )
                          , target + tTarget.getValue( 0, time )
                          , {0.0f, 1.0f, 0.0f}
                          );
  */
}

ModelColor::ModelColor(const noggit::mpq::file& f, const ModelColorDef &mcd, int *global)
  : color(mcd.color, f, global)
  , opacity(mcd.opacity, f, global)
{}

ModelTransparency::ModelTransparency(const noggit::mpq::file& f, const ModelTransDef &mcd, int *global)
  : trans(mcd.trans, f, global)
{}

ModelLight::ModelLight(const noggit::mpq::file& f, const ModelLightDef &mld, int *global)
  : type (mld.type)
  , parent (mld.bone)
  , pos (fixCoordSystem(mld.pos))
  , tpos (fixCoordSystem(mld.pos))
  , dir (::math::vector_3d(0,1,0))
  , tdir (::math::vector_3d(0,1,0)) // obviously wrong
  , diffColor (mld.color, f, global)
  , ambColor (mld.ambColor, f, global)
  , diffIntensity (mld.intensity, f, global)
  , ambIntensity (mld.ambIntensity, f, global)
{}

void ModelLight::setup(int time, opengl::light l)
{
  ::math::vector_4d ambcol(ambColor.getValue(0, time) * ambIntensity.getValue(0, time), 1.0f);
  ::math::vector_4d diffcol(diffColor.getValue(0, time) * diffIntensity.getValue(0, time), 1.0f);
  ::math::vector_4d p;

  enum ModelLightTypes {
    MODELLIGHT_DIRECTIONAL=0,
    MODELLIGHT_POINT
  };

  if (type==MODELLIGHT_DIRECTIONAL) {
    // directional
    p = ::math::vector_4d(tdir, 0.0f);
  } else if (type==MODELLIGHT_POINT) {
    // point
    p = ::math::vector_4d(tpos, 1.0f);
  } else {
    p = ::math::vector_4d(tpos, 1.0f);
    LogError << "Light type " << type << " is unknown." << std::endl;
  }
  //gLog("Light %d (%f,%f,%f) (%f,%f,%f) [%f,%f,%f]\n", l-GL_LIGHT4, ambcol.x, ambcol.y, ambcol.z, diffcol.x, diffcol.y, diffcol.z, p.x, p.y, p.z);
  gl.lightfv(l, GL_POSITION, p);
  gl.lightfv(l, GL_DIFFUSE, diffcol);
  gl.lightfv(l, GL_AMBIENT, ambcol);
  gl.enable(l);
}

TextureAnim::TextureAnim(const noggit::mpq::file& f, const ModelTexAnimDef &mta, int *global)
  : trans (mta.trans, f, global)
  , rot (mta.rot, f, global)
  , scale (mta.scale, f, global)
{}

namespace
{
  static const int MODELBONE_BILLBOARD = 8;
}
Bone::Bone(const noggit::mpq::file& f, const ModelBoneDef &b, int *global, noggit::mpq::file **animfiles)
  : trans (b.translation, f, global, animfiles)
  , rot (b.rotation, f, global, animfiles)
  , scale (b.scaling, f, global, animfiles)
  , pivot (fixCoordSystem (b.pivot))
  , parent (b.parent)
  , billboard (b.flags & MODELBONE_BILLBOARD)
  , mat (math::matrix_4x4::uninitialized)
  , mrot (math::matrix_4x4::uninitialized)
{
  trans.apply(fixCoordSystem);
  rot.apply(convert_rotation);
  scale.apply(fixCoordSystem2);
}

void Bone::calcMatrix(Bone *allbones, int anim, int time)
{
  if (calc) return;
  ::math::matrix_4x4 m (math::matrix_4x4::uninitialized);
  ::math::quaternion q;

  bool tr = rot.uses(anim) || scale.uses(anim) || trans.uses(anim) || billboard;
  if (tr) {
    m = math::matrix_4x4 (math::matrix_4x4::translation, pivot);

    if (trans.uses(anim)) {
      m *= ::math::matrix_4x4 (math::matrix_4x4::translation, trans.getValue (anim, time));
    }
    if (rot.uses(anim)) {
      q = rot.getValue(anim, time);
      m *= ::math::matrix_4x4 (math::matrix_4x4::rotation, q);
    }
    if (scale.uses(anim)) {
      m *= ::math::matrix_4x4 (math::matrix_4x4::scale, scale.getValue(anim, time));
    }
    if (billboard) {
      math::matrix_4x4 const modelview (opengl::matrix::model_view());

      ::math::vector_3d const vRight (-modelview.column<0>().xyz());
      ::math::vector_3d const vUp (modelview.column<1>().xyz()); // Spherical billboarding
      //::math::vector_3d vUp (0,1,0); // Cylindrical billboarding
      m (0, 1, vUp.x());
      m (1, 1, vUp.y());
      m (2, 1, vUp.z());
      m (0, 2, vRight.x());
      m (1, 2, vRight.y());
      m (2, 2, vRight.z());
    }

    m *= ::math::matrix_4x4 (math::matrix_4x4::translation, pivot*-1.0f);

  }
  else
  {
    m = math::matrix_4x4::unit;
  }

  if (parent>=0) {
    allbones[parent].calcMatrix(allbones, anim, time);
    mat = allbones[parent].mat * m;
  } else mat = m;

  // transform matrix for normal vectors ... ??
  if (rot.uses(anim)) {
    if (parent>=0) {
      mrot = allbones[parent].mrot * ::math::matrix_4x4 (math::matrix_4x4::rotation, q);
    } else mrot = ::math::matrix_4x4 (math::matrix_4x4::rotation, q);
  }
  else
  {
    mrot = math::matrix_4x4::unit;
  }

  transPivot = mat * pivot;

  calc = true;
}


void Model::draw (bool draw_fog, size_t time)
{
  if(!finished_loading())
    return;

  if (!_finished_upload) {
    upload();
    return;
  }

  if (draw_fog)
    gl.enable( GL_FOG );
  else
    gl.disable( GL_FOG );

  if (animated && (!animcalc || mPerInstanceAnimation))
  {
    animate(0, time);
    animcalc = true;
  }


  lightsOn(GL_LIGHT4, time);
  drawModel(time);
  lightsOff(GL_LIGHT4);


  // draw particle systems & ribbons
  for (size_t i = 0; i < header.nParticleEmitters; ++i)
    particleSystems[i].draw();

  for (size_t i = 0; i < header.nRibbonEmitters; ++i)
    ribbons[i].draw();
}

void Model::drawSelect (size_t time)
{
  if (!finished_loading())
    return;

  if (animated && (!animcalc || mPerInstanceAnimation))
  {
    animate(0, time);
    animcalc = true;
  }

  drawModelSelect(time);
}

void Model::lightsOn(opengl::light lbase, int animtime)
{
  // setup lights
  for (unsigned int i=0, l=lbase; i<header.nLights; ++i) lights[i].setup(animtime, l++);
}

void Model::lightsOff(opengl::light lbase)
{
  for (unsigned int i=0, l=lbase; i<header.nLights; ++i) gl.disable(l++);
}

void Model::upload()
{
  for (std::string texture : _texture_names)
    _textures.emplace_back(texture);

  gl.genBuffers(1, &vbuf);
  gl.genBuffers(1, &tbuf);
  const size_t size = header.nVertices * sizeof(float);
  vbufsize = 3 * size;

  if (!animGeometry)
  {
    gl.bindBuffer(GL_ARRAY_BUFFER_ARB, vbuf);
    gl.bufferData(GL_ARRAY_BUFFER_ARB, vbufsize, vertices, GL_STATIC_DRAW_ARB);

    gl.genBuffers(1, &nbuf);
    gl.bindBuffer(GL_ARRAY_BUFFER_ARB, nbuf);
    gl.bufferData(GL_ARRAY_BUFFER_ARB, vbufsize, normals, GL_STATIC_DRAW_ARB);

    delete[] vertices;
    delete[] normals;
  }

  ::math::vector_2d* texcoords = new ::math::vector_2d[header.nVertices];

  for (size_t i = 0; i < header.nVertices; ++i)
    texcoords[i] = origVertices[i].texcoords;

  gl.bindBuffer(GL_ARRAY_BUFFER_ARB, tbuf);
  gl.bufferData(GL_ARRAY_BUFFER_ARB, 2 * size, texcoords, GL_STATIC_DRAW_ARB);

  delete[] texcoords;

  _finished_upload = true;
}

void Model::updateEmitters(float dt)
{
  for (size_t i=0; i<header.nParticleEmitters; ++i) {
    particleSystems[i].update(dt);
  }
}
