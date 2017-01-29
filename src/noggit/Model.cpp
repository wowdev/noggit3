// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Log.h>
#include <noggit/Model.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/World.h>
#include <opengl/matrix.hpp>
#include <opengl/scoped.hpp>

#include <algorithm>
#include <cassert>
#include <map>
#include <sstream>
#include <string>

int globalTime = 0;

Model::Model(const std::string& filename)
  : _filename(filename)
{
  memset(&header, 0, sizeof(ModelHeader));

  indices = nullptr;
  origVertices = nullptr;

  showGeosets = nullptr;

  finished = false;

  //! \note hack: we currently would never load them otherwise
  finishLoading();
}

void Model::finishLoading()
{
  MPQFile f(_filename);

  if (f.isEof())
  {
    LogError << "Error loading file \"" << _filename << "\". Aborting to load model." << std::endl;
    finished = true;
    return;
  }

  //LogDebug << "Loading model \"" << _filename << "\"." << std::endl;

  memcpy(&header, f.getBuffer(), sizeof(ModelHeader));

  animated = isAnimated(f);  // isAnimated will set animGeometry and animTextures

  trans = 1.0f;

  vbuf = nbuf = tbuf = 0;

  animtime = 0;
  anim = 0;
  header.nParticleEmitters = 0;      //! \todo  Get Particles to 3.*? ._.
  header.nRibbonEmitters = 0;      //! \todo  Get Particles to 3.*? ._.
  if (header.nGlobalSequences)
  {
    globalSequences.resize (header.nGlobalSequences);
    memcpy(globalSequences.data(), (f.getBuffer() + header.ofsGlobalSequences), header.nGlobalSequences * 4);
  }

  //! \todo  This takes a biiiiiit long. Have a look at this.
  if (animated)
    initAnimated(f);
  else
    initStatic(f);

  f.close();

  finished = true;
}

Model::~Model()
{
  LogDebug << "Unloading model \"" << _filename << "\"." << std::endl;

  _textures.clear();
  _textureFilenames.clear();

  if (showGeosets)
    delete[] showGeosets;

  //! \note if animated geometry, these are a mapped GPU buffer
  if (!animGeometry)
  {
    delete[] vertices;
  }
  delete[] indices;

  if (animated) {
    // unload all sorts of crap
    //delete[] normals;
    if (origVertices)
      delete[] origVertices;
    if (!animGeometry) {
      gl.deleteBuffers(1, &nbuf);
    }
    gl.deleteBuffers(1, &vbuf);
    gl.deleteBuffers(1, &tbuf);
  }
  else {
    gl.deleteLists(ModelDrawList, 1);
  }
}


bool Model::isAnimated(const MPQFile& f)
{
  // see if we have any animated bones
  ModelBoneDef *bo = reinterpret_cast<ModelBoneDef*>(f.getBuffer() + header.ofsBones);

  animGeometry = false;
  animBones = false;
  mPerInstanceAnimation = false;

  ModelVertex *verts = reinterpret_cast<ModelVertex*>(f.getBuffer() + header.ofsVertices);
  for (size_t i = 0; i<header.nVertices && !animGeometry; ++i) {
    for (size_t b = 0; b<4; b++) {
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
    for (size_t i = 0; i<header.nBones; ++i) {
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
    for (size_t i = 0; i<header.nColors; ++i) {
      if (cols[i].color.type != 0 || cols[i].opacity.type != 0) {
        animMisc = true;
        break;
      }
    }
  }

  // animated opacity
  if (header.nTransparency && !animMisc) {
    ModelTransDef *trs = reinterpret_cast<ModelTransDef*>(f.getBuffer() + header.ofsTransparency);
    for (size_t i = 0; i<header.nTransparency; ++i) {
      if (trs[i].trans.type != 0) {
        animMisc = true;
        break;
      }
    }
  }

  // guess not...
  return animGeometry || animTextures || animMisc;
}


math::vector_3d fixCoordSystem(math::vector_3d v)
{
  return math::vector_3d(v.x, v.z, -v.y);
}

math::vector_3d fixCoordSystem2(math::vector_3d v)
{
  return math::vector_3d(v.x, v.z, v.y);
}

math::quaternion fixCoordSystemQuat(math::quaternion v)
{
  return math::quaternion(-v.x, -v.z, v.y, v.w);
}


void Model::initCommon(const MPQFile& f)
{
  // assume: origVertices already set
  if (!animGeometry) {
    vertices = new math::vector_3d[header.nVertices];
    normals = new math::vector_3d[header.nVertices];
  }

  // vertices, normals
  for (size_t i = 0; i<header.nVertices; ++i) {
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
  rad = sqrtf(rad);

  // textures
  ModelTextureDef* texdef = reinterpret_cast<ModelTextureDef*>(f.getBuffer() + header.ofsTextures);
  _textureFilenames.resize(header.nTextures);
  _specialTextures.resize(header.nTextures);

  for (size_t i = 0; i < header.nTextures; ++i)
  {
    if (texdef[i].type == 0)
    {
      _specialTextures[i] = -1;
      _textureFilenames[i] = std::string(f.getBuffer() + texdef[i].nameOfs, texdef[i].nameLen);
      _textures.emplace_back (_textureFilenames[i]);
    }
    else
    {
      //! \note special texture - only on characters and such... Noggit should not even render these.
      //! \todo Check if this is actually correct. Or just remove it.
      throw std::exception(); //this just fucks things up, dont load them for now

      _specialTextures[i] = texdef[i].type;

      if (texdef[i].type == 3)
      {
        _textureFilenames[i] = "Item\\ObjectComponents\\Weapon\\ArmorReflect4.BLP";
        // a fix for weapons with type-3 textures.
        _replaceTextures.emplace (texdef[i].type, _textureFilenames[i]);
      }
    }
  }

  // init colors
  if (header.nColors) {
    ModelColorDef *colorDefs = reinterpret_cast<ModelColorDef*>(f.getBuffer() + header.ofsColors);
    for (size_t i = 0; i<header.nColors; ++i) colors.emplace_back(f, colorDefs[i], globalSequences.data());
  }
  // init transparency
  int16_t *transLookup = reinterpret_cast<int16_t*>(f.getBuffer() + header.ofsTransparencyLookup);
  if (header.nTransparency) {
    ModelTransDef *trDefs = reinterpret_cast<ModelTransDef*>(f.getBuffer() + header.ofsTransparency);
    for (size_t i = 0; i<header.nTransparency; ++i) transparency.emplace_back (f, trDefs[i], globalSequences.data());
  }


  // just use the first LOD/view

  if (header.nViews > 0) {
    // indices - allocate space, too
    std::string lodname = _filename.substr(0, _filename.length() - 3);
    lodname.append("00.skin");
    MPQFile g(lodname.c_str());
    if (g.isEof()) {
      LogError << "loading skinfile " << lodname << std::endl;
      g.close();
      return;
    }
    ModelView *view = reinterpret_cast<ModelView*>(g.getBuffer());

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
    for (size_t i = 0; i<view->nSub; ++i) {
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

      //! \todo figure out these flags properly -_-
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
        RENDERFLAGS_ZBUFFERED = 16
      };

      pass.unlit = (rf.flags & RENDERFLAGS_UNLIT) != 0;
      pass.cull = (rf.flags & RENDERFLAGS_TWOSIDED) == 0 && rf.blend == 0;

      pass.billboard = (rf.flags & RENDERFLAGS_BILLBOARD) != 0;

      pass.useenvmap = (texunitlookup[tex[j].texunit] == -1) && pass.billboard && rf.blend>2;
      pass.nozwrite = (rf.flags & RENDERFLAGS_ZBUFFERED) != 0;

      //! \todo Work out the correct way to get the true/false of transparency
      pass.trans = (pass.blendmode>0) && (pass.opacity>0);  // Transparency - not the correct way to get transparency

      pass.p = ops[geoset].BoundingBox[0].x;

      // Texture flags
      enum TextureFlags {
        TEXTURE_WRAPX = 1,
        TEXTURE_WRAPY
      };

      pass.swrap = (texdef[pass.tex].flags & TEXTURE_WRAPX) != 0; // Texture wrap X
      pass.twrap = (texdef[pass.tex].flags & TEXTURE_WRAPY) != 0; // Texture wrap Y

      static const int TEXTUREUNIT_STATIC = 16;

      if (animTextures) {
        if (tex[j].flags & TEXTUREUNIT_STATIC) {
          pass.texanim = -1; // no texture animation
        }
        else {
          pass.texanim = texanimlookup[tex[j].texanimid];
        }
      }
      else {
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

void Model::initStatic(const MPQFile& f)
{
  origVertices = (ModelVertex*)(f.getBuffer() + header.ofsVertices);

  initCommon(f);

  ModelDrawList = gl.genLists(1);
  gl.newList(ModelDrawList, GL_COMPILE);
  drawModel( /*false*/);
  gl.endList();

  // clean up vertices, indices etc
  if (normals)
    delete[] normals;
}

void Model::initAnimated(const MPQFile& f)
{
  origVertices = new ModelVertex[header.nVertices];
  memcpy(origVertices, f.getBuffer() + header.ofsVertices, header.nVertices * sizeof(ModelVertex));

  gl.genBuffers(1, &vbuf);
  gl.genBuffers(1, &tbuf);
  const size_t size = header.nVertices * sizeof(float);
  vbufsize = 3 * size;

  initCommon(f);

  if (header.nAnimations > 0) {
    anims.resize (header.nAnimations);
    memcpy(anims.data(), f.getBuffer() + header.ofsAnimations, header.nAnimations * sizeof(ModelAnimation));
    for (size_t i = 0; i < header.nAnimations; ++i)
    {
      //! \note Fix for world\kalimdor\diremaul\activedoodads\crystalcorrupter\corruptedcrystalshard.m2 having a zero length for its stand animation.
      anims[i].length = std::max(anims[i].length, 1U);
    }

    animfiles = new MPQFile*[header.nAnimations];

    std::stringstream tempname;
    for (size_t i = 0; i<header.nAnimations; ++i)
    {
      std::string lodname = _filename.substr(0, _filename.length() - 3);
      tempname << lodname << anims[i].animID << "-" << anims[i].subAnimID;
      if (MPQFile::exists(tempname.str()))
      {
        animfiles[i] = new MPQFile(tempname.str());
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
    for (size_t i = 0; i<header.nBones; ++i) {
      bones.emplace_back(f, mb[i], globalSequences.data(), animfiles);
    }
  }

  if (!animGeometry) {
    gl.bufferData<GL_ARRAY_BUFFER> (vbuf, vbufsize, vertices, GL_STATIC_DRAW);
    gl.genBuffers(1, &nbuf);
    gl.bufferData<GL_ARRAY_BUFFER> (nbuf, vbufsize, normals, GL_STATIC_DRAW);
    delete[] normals;
  }
  math::vector_2d *texcoords = new math::vector_2d[header.nVertices];
  for (size_t i = 0; i<header.nVertices; ++i)
    texcoords[i] = origVertices[i].texcoords;
  gl.bufferData<GL_ARRAY_BUFFER> (tbuf, 2 * size, texcoords, GL_STATIC_DRAW);
  delete[] texcoords;

  if (animTextures) {
    ModelTexAnimDef *ta = reinterpret_cast<ModelTexAnimDef*>(f.getBuffer() + header.ofsTexAnims);
    for (size_t i = 0; i<header.nTexAnims; ++i) {
      texanims.emplace_back(f, ta[i], globalSequences.data());
    }
  }

  // particle systems
  if (header.nParticleEmitters) {
    ModelParticleEmitterDef *pdefs = reinterpret_cast<ModelParticleEmitterDef*>(f.getBuffer() + header.ofsParticleEmitters);
    for (size_t i = 0; i<header.nParticleEmitters; ++i) {
      particleSystems.emplace_back (this, f, pdefs[i], globalSequences.data());
    }
  }

  // ribbons
  if (header.nRibbonEmitters) {
    ModelRibbonEmitterDef *rdefs = reinterpret_cast<ModelRibbonEmitterDef*>(f.getBuffer() + header.ofsRibbonEmitters);
    for (size_t i = 0; i<header.nRibbonEmitters; ++i) {
      ribbons.emplace_back(this, f, rdefs[i], globalSequences.data());
    }
  }

  // just use the first camera, meh
  if (header.nCameras>0) {
    ModelCameraDef *camDefs = reinterpret_cast<ModelCameraDef*>(f.getBuffer() + header.ofsCameras);
    cam = ModelCamera(f, camDefs[0], globalSequences.data());
  }

  // init lights
  if (header.nLights) {
    ModelLightDef *lDefs = reinterpret_cast<ModelLightDef*>(f.getBuffer() + header.ofsLights);
    for (size_t i = 0; i<header.nLights; ++i)
      lights.emplace_back(f, lDefs[i], globalSequences.data());
  }

  animcalc = false;
}

void Model::calcBones(int _anim, int time)
{
  for (size_t i = 0; i<header.nBones; ++i) {
    bones[i].calc = false;
  }

  for (size_t i = 0; i<header.nBones; ++i) {
    bones[i].calcMatrix(bones.data(), _anim, time);
  }
}

void Model::animate(int _anim)
{
  this->anim = _anim;
  ModelAnimation &a = anims[anim];

  if (anims.empty())
    return;

  int t = globalTime; //(int)(gWorld->animtime /* / a.playSpeed*/);
  int tmax = a.length;
  t %= tmax;
  animtime = t;

  if (animBones) {
    calcBones(anim, t);
  }

  if (animGeometry) {
    opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder (vbuf);
    gl.bufferData (GL_ARRAY_BUFFER, 2 * vbufsize, nullptr, GL_STREAM_DRAW);
    vertices = reinterpret_cast<math::vector_3d*>(gl.mapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    // transform vertices
    ModelVertex *ov = origVertices;
    for (size_t i = 0; i<header.nVertices; ++i, ++ov) {
      math::vector_3d v(0, 0, 0), n(0, 0, 0);

      for (size_t b = 0; b<4; ++b) {
        if (ov->weights[b]>0) {
          math::vector_3d tv = bones[ov->bones[b]].mat * ov->pos;
          math::vector_3d tn = bones[ov->bones[b]].mrot * ov->normal;
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

    gl.unmapBuffer(GL_ARRAY_BUFFER);
  }

  for (size_t i = 0; i<header.nLights; ++i) {
    if (lights[i].parent >= 0) {
      lights[i].tpos = bones[lights[i].parent].mat * lights[i].pos;
      lights[i].tdir = bones[lights[i].parent].mrot * lights[i].dir;
    }
  }

  for (size_t i = 0; i<header.nParticleEmitters; ++i) {
    // random time distribution for teh win ..?
    int pt = (t + static_cast<int>(tmax*particleSystems[i].tofs)) % tmax;
    particleSystems[i].setup(anim, pt);
  }

  for (size_t i = 0; i<header.nRibbonEmitters; ++i) {
    ribbons[i].setup(anim, t);
  }

  if (animTextures) {
    for (size_t i = 0; i<header.nTexAnims; ++i) {
      texanims[i].calc(anim, t);
    }
  }
}

bool ModelRenderPass::init(Model *m)
{
  // May aswell check that we're going to render the geoset before doing all this crap.
  if (m->showGeosets[geoset]) {

    // COLOUR
    // Get the colour and transparency and check that we should even render
    ocol = math::vector_4d(1.0f, 1.0f, 1.0f, m->trans);
    ecol = math::vector_4d(0.0f, 0.0f, 0.0f, 0.0f);

    //if (m->trans == 1.0f)
    //  return false;

    // emissive colors
    if (color != -1 && m->colors[color].color.uses(0)) {
      math::vector_3d c = m->colors[color].color.getValue(0, m->animtime);
      if (m->colors[color].opacity.uses(m->anim)) {
        float o = m->colors[color].opacity.getValue(m->anim, m->animtime);
        ocol.w = o;
      }

      if (unlit) {
        ocol.x = c.x; ocol.y = c.y; ocol.z = c.z;
      }
      else {
        ocol.x = ocol.y = ocol.z = 0;
      }

      ecol = math::vector_4d(c, ocol.w);
      gl.materialfv(GL_FRONT, GL_EMISSION, ecol);
    }

    // opacity
    if (opacity != -1) {
      if (m->transparency[opacity].trans.uses(0))
        ocol.w *= m->transparency[opacity].trans.getValue(0, m->animtime);
    }

    // exit and return false before affecting the opengl render state
    if (!((ocol.w > 0) && (color == -1 || ecol.w > 0)))
      return false;

    // TEXTURE
    if (m->_specialTextures[tex] == -1)
      m->_textures[tex]->bind();
    else
      m->_replaceTextures.at (m->_specialTextures[tex])->bind();

    //! \todo Add proper support for multi-texturing.

    // blend mode
    switch (blendmode) {
    case BM_OPAQUE:  // 0
      break;
    case BM_TRANSPARENT: // 1
      gl.enable(GL_ALPHA_TEST);
      gl.alphaFunc(GL_GEQUAL, 0.7f);
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
      gl.blendFunc(GL_DST_COLOR, GL_SRC_COLOR);
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
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    if (twrap)
      gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
      //const GLint maptype = GL_REFLECTION_MAP_;

      gl.texGeni(GL_S, GL_TEXTURE_GEN_MODE, maptype);
      gl.texGeni(GL_T, GL_TEXTURE_GEN_MODE, maptype);
    }

    if (texanim != -1) {
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

    if (blendmode <= 1 && ocol.w<1.0f)
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
  if (texanim != -1) {
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
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture (0);
  }
  //gl.color4f(1,1,1,1); //???
}

void ModelHighlight(math::vector_4d color)
{
  gl.disable(GL_ALPHA_TEST);
  gl.enable(GL_BLEND);
  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.disable(GL_CULL_FACE);
  opengl::texture::set_active_texture (0);
  opengl::texture::disable_texture();
  opengl::texture::set_active_texture (1);
  opengl::texture::disable_texture();
  gl.color4fv(color);
  gl.materialfv(GL_FRONT, GL_EMISSION, color);
  //  gl.depthMask( GL_FALSE );
}

void ModelUnhighlight()
{
  gl.enable(GL_ALPHA_TEST);
  gl.disable(GL_BLEND);
  gl.enable(GL_CULL_FACE);
  opengl::texture::set_active_texture (0);
  opengl::texture::enable_texture();
  gl.color4fv(math::vector_4d(1, 1, 1, 1));
  //  gl.depthMask( GL_TRUE );
}

void Model::drawModel( /*bool unlit*/)
{
  // assume these client states are enabled: GL_VERTEX_ARRAY, GL_NORMAL_ARRAY, GL_TEXTURE_COORD_ARRAY

  if (animated)
  {
    if (animGeometry)
    {
      opengl::scoped::buffer_binder<GL_ARRAY_BUFFER> const binder (vbuf);
      gl.vertexPointer (3, GL_FLOAT, 0, 0);
      gl.normalPointer (GL_FLOAT, 0, reinterpret_cast<void*>(vbufsize));
    }
    else
    {
      gl.vertexPointer (vbuf, 3, GL_FLOAT, 0, 0);
      gl.normalPointer (nbuf, GL_FLOAT, 0, 0);
    }

    gl.texCoordPointer (tbuf, 2, GL_FLOAT, 0, 0);
  }

  gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl.alphaFunc(GL_GREATER, 0.3f);

  for (size_t i = 0; i<passes.size(); ++i) {
    ModelRenderPass &p = passes[i];

    if (p.init(this)) {
      // we don't want to render completely transparent parts

      // render
      if (animated) {
        //gl.drawElements(GL_TRIANGLES, p.indexCount, GL_UNSIGNED_SHORT, indices + p.indexStart);
        // a GDC OpenGL Performace Tuning paper recommended gl.drawRangeElements over gl.drawElements
        // I can't notice a difference but I guess it can't hurt
        gl.drawRangeElements(GL_TRIANGLES, p.vertexStart, p.vertexEnd, p.indexCount, GL_UNSIGNED_SHORT, indices + p.indexStart);

      }
      else {
        gl.begin(GL_TRIANGLES);
        for (size_t k = 0, b = p.indexStart; k<p.indexCount; ++k, ++b) {
          uint16_t a = indices[b];
          gl.normal3fv(normals[a]);
          gl.texCoord2fv(origVertices[a].texcoords);
          gl.vertex3fv(vertices[a]);
        }
        gl.end();
      }

      p.deinit();
    }

  }
  // done with all render ops

  gl.alphaFunc(GL_GREATER, 0.0f);
  gl.disable(GL_ALPHA_TEST);

  GLfloat czero[4] = { 0, 0, 0, 1 };
  gl.materialfv(GL_FRONT, GL_EMISSION, czero);
  gl.color4f(1, 1, 1, 1);
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
    gl.translatef(tval.x, tval.y, tval.z);
  }
  if (rot.uses(anim)) {
    gl.rotatef(rval.x, 0, 0, 1); // this is wrong, I have no idea what I'm doing here ;)
  }
  if (scale.uses(anim)) {
    gl.scalef(sval.x, sval.y, sval.z);
  }
}

ModelCamera::ModelCamera(const MPQFile& f, const ModelCameraDef &mcd, int *global)
  : nearclip (mcd.nearclip)
  , farclip (mcd.farclip)
  , fov (mcd.fov)
  , pos (fixCoordSystem(mcd.pos))
  , target (fixCoordSystem(mcd.target))
  , tPos (mcd.transPos, f, global)
  , tTarget (mcd.transTarget, f, global)
  , rot (mcd.rot, f, global)
{
  tPos.apply(fixCoordSystem);
  tTarget.apply(fixCoordSystem);
}

void ModelCamera::setup(int time)
{
  video.fov(math::radians (fov / 2));
  video.nearclip(nearclip);
  video.farclip(farclip);

  video.set3D();
  opengl::matrix::look_at ( pos + tPos.getValue( 0, time )
                          , target + tTarget.getValue( 0, time )
                          , {0.0f, 1.0f, 0.0f}
                          );
}

ModelColor::ModelColor(const MPQFile& f, const ModelColorDef &mcd, int *global)
  : color (mcd.color, f, global)
  , opacity(mcd.opacity, f, global)
{}

ModelTransparency::ModelTransparency(const MPQFile& f, const ModelTransDef &mcd, int *global)
  : trans (mcd.trans, f, global)
{}

ModelLight::ModelLight(const MPQFile& f, const ModelLightDef &mld, int *global)
  : tpos (fixCoordSystem(mld.pos))
  , pos (fixCoordSystem(mld.pos))
  , tdir (::math::vector_3d(0,1,0)) // obviously wrong
  , dir (::math::vector_3d(0,1,0))
  , type (mld.type)
  , parent (mld.bone)
  , ambColor (mld.ambColor, f, global)
  , ambIntensity (mld.ambIntensity, f, global)
  , diffColor (mld.color, f, global)
  , diffIntensity (mld.intensity, f, global)
{}

void ModelLight::setup(int time, OpenGL::Light l)
{
  math::vector_4d ambcol(ambColor.getValue(0, time) * ambIntensity.getValue(0, time), 1.0f);
  math::vector_4d diffcol(diffColor.getValue(0, time) * diffIntensity.getValue(0, time), 1.0f);
  math::vector_4d p;

  enum ModelLightTypes {
    MODELLIGHT_DIRECTIONAL = 0,
    MODELLIGHT_POINT
  };

  if (type == MODELLIGHT_DIRECTIONAL) {
    // directional
    p = math::vector_4d(tdir, 0.0f);
  }
  else if (type == MODELLIGHT_POINT) {
    // point
    p = math::vector_4d(tpos, 1.0f);
  }
  else {
    p = math::vector_4d(tpos, 1.0f);
    LogError << "Light type " << type << " is unknown." << std::endl;
  }
  //gLog("Light %d (%f,%f,%f) (%f,%f,%f) [%f,%f,%f]\n", l-GL_LIGHT4, ambcol.x, ambcol.y, ambcol.z, diffcol.x, diffcol.y, diffcol.z, p.x, p.y, p.z);
  gl.lightfv(l, GL_POSITION, p);
  gl.lightfv(l, GL_DIFFUSE, diffcol);
  gl.lightfv(l, GL_AMBIENT, ambcol);
  gl.enable(l);
}

TextureAnim::TextureAnim (const MPQFile& f, const ModelTexAnimDef &mta, int *global)
  : trans (mta.trans, f, global)
  , rot (mta.rot, f, global)
  , scale (mta.scale, f, global)
{}

namespace
{
  //! \todo other billboard types
  static const int MODELBONE_BILLBOARD = 8;
}

Bone::Bone(const MPQFile& f, const ModelBoneDef &b, int *global, MPQFile **animfiles)
  : parent (b.parent)
  , pivot (fixCoordSystem (b.pivot))
  , billboard (b.flags & MODELBONE_BILLBOARD)
  , trans (b.translation, f, global, animfiles)
  , rot (b.rotation, f, global, animfiles)
  , scale (b.scaling, f, global, animfiles)
{
  trans.apply(fixCoordSystem);
  rot.apply(fixCoordSystemQuat);
  scale.apply(fixCoordSystem2);
}

void Bone::calcMatrix(Bone *allbones, int anim, int time)
{
  if (calc) return;

  if (parent != -1)
  {
    allbones[parent].calcMatrix (allbones, anim, time);

    mat = allbones[parent].mat;
    mrot = allbones[parent].mrot;
  }
  else
  {
    mat = math::matrix_4x4::unit;
    mrot = math::matrix_4x4::unit;
  }

  mat = {math::matrix_4x4::translation, pivot};

  if (trans.uses(anim))
  {
    mat *= math::matrix_4x4 (math::matrix_4x4::translation, trans.getValue(anim, time));
  }

  mrot = math::matrix_4x4::unit;

  if (rot.uses(anim)) {
    mrot = math::matrix_4x4 (math::matrix_4x4::rotation, rot.getValue(anim, time));
    mat *= mrot;
  }

  if (scale.uses(anim)) {
    mat *= math::matrix_4x4 (math::matrix_4x4::scale, scale.getValue(anim, time));
  }

  if (billboard) {
    float modelview[16];
    gl.getFloatv(GL_MODELVIEW_MATRIX, modelview);

    math::vector_3d vRight = math::vector_3d(modelview[0], modelview[4], modelview[8]);
    math::vector_3d vUp = math::vector_3d(modelview[1], modelview[5], modelview[9]); // Spherical billboarding
    //math::vector_3d vUp = math::vector_3d(0,1,0); // Cylindrical billboarding
    vRight = vRight * -1;
    mat (0, 2, vRight.x);
    mat(1, 2, vRight.y);
    mat (2, 2, vRight.z);
    mat (0, 1, vUp.x);
    mat (1, 1, vUp.y);
    mat (2, 1, vUp.z);
  }

  mat *= math::matrix_4x4 (math::matrix_4x4::translation, -pivot);

  calc = true;
}


void Model::draw()
{
  if (!finishedLoading())
    return;

  if (gWorld && gWorld->drawfog)
    gl.enable(GL_FOG);
  else
    gl.disable(GL_FOG);

  if (!animated)
  {
    gl.callList(ModelDrawList);
  }
  else
  {
    if (!animcalc || mPerInstanceAnimation)
    {
      animate(0);
      animcalc = true;
    }


    lightsOn(GL_LIGHT4);
    drawModel( /*false*/);
    lightsOff(GL_LIGHT4);


    // draw particle systems & ribbons
    for (size_t i = 0; i < header.nParticleEmitters; ++i)
      particleSystems[i].draw();

    for (size_t i = 0; i < header.nRibbonEmitters; ++i)
      ribbons[i].draw();
  }
}

std::vector<float> Model::intersect (math::ray const& ray)
{
  std::vector<float> results;

  if (finishedLoading())
  {
    if (animated && (!animcalc || mPerInstanceAnimation))
    {
      animate (0);
      animcalc = true;
    }

    for (auto&& pass : passes)
    {
      for (size_t i (pass.indexStart); i < pass.indexStart + pass.indexCount; i += 3)
      {
        if ( auto distance
           = ray.intersect_triangle
               (vertices[indices[i + 0]], vertices[indices[i + 1]], vertices[indices[i + 2]])
           )
        {
          results.emplace_back (*distance);
        }
      }
    }
  }

  return results;
}

void Model::lightsOn(OpenGL::Light lbase)
{
  // setup lights
  for (unsigned int i = 0, l = lbase; i<header.nLights; ++i) lights[i].setup(animtime, l++);
}

void Model::lightsOff(OpenGL::Light lbase)
{
  for (unsigned int i = 0, l = lbase; i<header.nLights; ++i) gl.disable(l++);
}

void Model::updateEmitters(float dt)
{
  for (size_t i = 0; i<header.nParticleEmitters; ++i) {
    particleSystems[i].update(dt);
  }
}
