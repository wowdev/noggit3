#include "WMO.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <sstream>
#include <iomanip>
#include <iostream>

#include "Liquid.h"
#include "Log.h" // LogDebug
#include "ModelManager.h" // ModelManager
#include "Shaders.h"
#include "TextureManager.h" // TextureManager, Texture
#include "World.h"

void WMOHighlight(Vec4D color)
{
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glColor4fv(color);
	glMaterialfv(GL_FRONT, GL_EMISSION, color);
	glDepthMask(GL_FALSE);
}

void WMOUnhighlight()
{
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glColor4fv(Vec4D(1, 1, 1, 1));
	glDepthMask(GL_TRUE);
}

const std::string& WMO::filename() const
{
	return _filename;
}

WMO::WMO(const std::string& filenameArg)
	: ManagedItem()
	, _filename(filenameArg)
{
	MPQFile f(_filename);
	if (f.isEof()) {
		LogError << "Error loading WMO \"" << _filename << "\"." << std::endl;
		return;
	}

	uint32_t fourcc;
	uint32_t size;
	float ff[3];

	char *ddnames = NULL;
	char *groupnames = NULL;

	skybox = 0;

	char *texbuf = 0;

	while (!f.isEof()) {
		f.read(&fourcc, 4);
		f.read(&size, 4);

		size_t nextpos = f.getPos() + size;

		if (fourcc == 'MOHD') {
			unsigned int col;
			// header
			f.read(&nTextures, 4);
			f.read(&nGroups, 4);
			f.read(&nP, 4);
			f.read(&nLights, 4);
			f.read(&nModels, 4);
			f.read(&nDoodads, 4);
			f.read(&nDoodadSets, 4);
			f.read(&col, 4);
			f.read(&nX, 4);
			f.read(ff, 12);
			extents[0] = Vec3D(ff[0], ff[1], ff[2]);
			f.read(ff, 12);
			extents[1] = Vec3D(ff[0], ff[1], ff[2]);

			groups = new WMOGroup[nGroups];
			mat = new WMOMaterial[nTextures];

		}
		else if (fourcc == 'MOTX') {
			// textures
			texbuf = new char[size];
			f.read(texbuf, size);
		}
		else if (fourcc == 'MOMT') {
			// materials
			//WMOMaterialBlock bl;

			for (unsigned int i = 0; i<nTextures; ++i) {
				WMOMaterial *m = &mat[i];
				f.read(m, 0x40);

				std::string texpath(texbuf + m->nameStart);

				m->_texture = TextureManager::newTexture(texpath);
				textures.push_back(texpath);
			}
		}
		else if (fourcc == 'MOGN') {
			groupnames = reinterpret_cast<char*>(f.getPointer());
		}
		else if (fourcc == 'MOGI') {
			// group info - important information! ^_^
			for (unsigned int i = 0; i<nGroups; ++i) {
				groups[i].init(this, &f, i, groupnames);

			}
		}
		else if (fourcc == 'MOLT') {
			// Lights?
			for (unsigned int i = 0; i<nLights; ++i) {
				WMOLight l;
				l.init(&f);
				lights.push_back(l);
			}
		}
		else if (fourcc == 'MODN') {
			// models ...
			// MMID would be relative offsets for MMDX filenames
			if (size) {

				ddnames = reinterpret_cast<char*>(f.getPointer());

				f.seekRelative(size);
			}
		}
		else if (fourcc == 'MODS') {
			for (unsigned int i = 0; i<nDoodadSets; ++i) {
				WMODoodadSet dds;
				f.read(&dds, 32);
				doodadsets.push_back(dds);
			}
		}
		else if (fourcc == 'MODD') {
			nModels = size / 0x28;
			for (unsigned int i = 0; i<nModels; ++i) {
				int ofs;
				f.read(&ofs, 4);
				modelis.push_back(ModelInstance (ddnames + ofs, &f));
			}

		}
		else if (fourcc == 'MOSB')
		{
			if (size>4)
			{
				std::string path = std::string(reinterpret_cast<char*>(f.getPointer()));
				if (path.length())
				{
					LogDebug << "SKYBOX:" << std::endl;

					if (MPQFile::exists(path))
					{
						skybox = ModelManager::add(path);
						skyboxFilename = path;
					}
					else
					{
						skybox = NULL;
					}
				}
			}
		}
		else if (fourcc == 'MOPV') {
			WMOPV p;
			for (unsigned int i = 0; i<nP; ++i) {
				f.read(ff, 12);
				p.a = Vec3D(ff[0], ff[2], -ff[1]);
				f.read(ff, 12);
				p.b = Vec3D(ff[0], ff[2], -ff[1]);
				f.read(ff, 12);
				p.c = Vec3D(ff[0], ff[2], -ff[1]);
				f.read(ff, 12);
				p.d = Vec3D(ff[0], ff[2], -ff[1]);
				pvs.push_back(p);
			}
		}
		else if (fourcc == 'MOPR') {
			int nn = size / 8;
			WMOPR *pr = reinterpret_cast<WMOPR*>(f.getPointer());
			for (int i = 0; i<nn; ++i) {
				prs.push_back(*pr++);
			}
		}
		else if (fourcc == 'MFOG') {
			int nfogs = size / 0x30;
			for (int i = 0; i<nfogs; ++i) {
				WMOFog fog;
				fog.init(&f);
				fogs.push_back(fog);
			}
		}

		f.seek(nextpos);
	}

	f.close();
	if (texbuf)
	{
		delete[] texbuf;
		texbuf = NULL;
	}

	for (unsigned int i = 0; i<nGroups; ++i)
		groups[i].initDisplayList();
}

WMO::~WMO()
{
	LogDebug << "Unloading WMO \"" << filename() << "\"." << std::endl;
	if (groups)
	{
		delete[] groups;
		groups = NULL;
	}

	for (std::vector<std::string>::iterator it = textures.begin(); it != textures.end(); ++it) {
		TextureManager::delbyname(*it);
	}

	if (mat)
	{
		delete[] mat;
		mat = NULL;
	}

	if (skybox) {
		//delete skybox;
		ModelManager::delbyname(skyboxFilename);
		skybox = NULL;
	}
}

// model.cpp
void DrawABox(Vec3D pMin, Vec3D pMax, Vec4D pColor, float pLineWidth);

void WMO::draw(int doodadset, const Vec3D &ofs, const float rot, bool boundingbox, bool groupboxes, bool /*highlight*/) const
{
	if (gWorld && gWorld->drawfog)
		glEnable(GL_FOG);
	else
		glDisable(GL_FOG);

	for (unsigned int i = 0; i<nGroups; ++i)
	{
		groups[i].draw(ofs, rot, false);

		if (gWorld->drawdoodads)
		{
			groups[i].drawDoodads(doodadset, ofs, rot);
		}

		groups[i].drawLiquid();
	}

	if (boundingbox)
	{
		glDisable(GL_LIGHTING);

		glDisable(GL_COLOR_MATERIAL);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		for (unsigned int i = 0; i < nGroups; ++i)
			DrawABox(groups[i].BoundingBoxMin, groups[i].BoundingBoxMax, Vec4D(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

		DrawABox(Vec3D(extents[0].x, extents[0].z, -extents[0].y), Vec3D(extents[1].x, extents[1].z, -extents[1].y), Vec4D(1.0f, 0.0f, 0.0f, 1.0f), 2.0f);

		/*glColor4fv( Vec4D( 1.0f, 0.0f, 0.0f, 1.0f ) );
		glBegin( GL_LINES );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( this->header.BoundingBoxMax.x + header.BoundingBoxMax.x / 5.0f, 0.0f, 0.0f );
		glEnd();

		glColor4fv( Vec4D( 0.0f, 1.0f, 0.0f, 1.0f ) );
		glBegin( GL_LINES );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( 0.0f, header.BoundingBoxMax.z + header.BoundingBoxMax.z / 5.0f, 0.0f );
		glEnd();

		glColor4fv( Vec4D( 0.0f, 0.0f, 1.0f, 1.0f ) );
		glBegin( GL_LINES );
		glVertex3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( 0.0f, 0.0f, header.BoundingBoxMax.y + header.BoundingBoxMax.y / 5.0f );
		glEnd();*/

		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);

		glEnable(GL_LIGHTING);

	}

	/*  {
	// draw boundingboxe and axis
	// Turn light off and highlight the following
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_LINE_SMOOTH);
	glLineWidth(1.0);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	glColor4f( 1, 1, 1, 1 );

	glLineWidth(1.0);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	for (int i=0; i<nGroups; ++i)
	{
	WMOGroup &header = groups[i];
	/// Bounding box
	glColor4f( 1, 1, 1, 1 );
	glBegin( GL_LINE_STRIP );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
	glEnd();

	glBegin( GL_LINES );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMax.z );
	glEnd();
	glBegin( GL_LINES );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMin.z );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMin.y, header.BoundingBoxMin.z );
	glEnd();
	glBegin( GL_LINES );
	glVertex3f( header.BoundingBoxMin.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
	glVertex3f( header.BoundingBoxMax.x, header.BoundingBoxMax.y, header.BoundingBoxMax.z );
	glEnd();

	// draw axis
	glColor4fv( Vec4D( 1, 0, 0, 1 ) );
	glBegin( GL_LINES );
	glVertex3f( 0, 0, 0 );
	glVertex3f( header.BoundingBoxMax.x + 6, 0, 0 );
	glEnd();


	glColor4fv( Vec4D( 0, 1, 0, 1 ) );
	glBegin( GL_LINES );
	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, header.BoundingBoxMax.y + 6, 0 );
	glEnd();

	glColor4fv( Vec4D( 0, 0, 1, 1 ) );
	glBegin( GL_LINES );
	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, 0, header.BoundingBoxMax.x + 6 );
	glEnd();



	}
	// Back to normal light rendering
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	} // end bounding  boxes.*/

	if (false && groupboxes)
	{
		//WIP STEFF
		// draw group boundingboxes
		// Turn light off and highlight the following
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(1.0);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		glColor4f(1, 1, 0, 1);

		glLineWidth(1.0);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		for (unsigned int i = 0; i<nGroups; ++i)
		{
			WMOGroup &header = groups[i];
			glBegin(GL_LINE_STRIP);
			//A
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMax.y, header.VertexBoxMin.z);
			//C
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMin.y, header.VertexBoxMin.z);
			//D
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMin.y, header.VertexBoxMin.z);
			//G
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMin.y, header.VertexBoxMax.z);
			//H
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMax.y, header.VertexBoxMax.z);
			//B
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMax.y, header.VertexBoxMin.z);
			//A
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMax.y, header.VertexBoxMin.z);
			//E
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMax.y, header.VertexBoxMax.z);
			//F
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMin.y, header.VertexBoxMax.z);
			//C
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMin.y, header.VertexBoxMin.z);
			glEnd();

			glBegin(GL_LINES);
			// F G
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMin.y, header.VertexBoxMax.z);
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMin.y, header.VertexBoxMax.z);
			glEnd();
			glBegin(GL_LINES);
			// B D
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMax.y, header.VertexBoxMin.z);
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMin.y, header.VertexBoxMin.z);
			glEnd();
			glBegin(GL_LINES);
			// E H
			glVertex3f(header.VertexBoxMin.x, header.VertexBoxMax.y, header.VertexBoxMax.z);
			glVertex3f(header.VertexBoxMax.x, header.VertexBoxMax.y, header.VertexBoxMax.z);
			glEnd();
		}
		// Back to normal light rendering
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	} // end drow groupe boxes.





	/*
	// draw portal relations
	glBegin(GL_LINES);
	for (size_t i=0; i<prs.size(); ++i) {
	WMOPR &pr = prs[i];
	WMOPV &pv = pvs[pr.portal];
	if (pr.dir>0) glColor4f(1,0,0,1);
	else glColor4f(0,0,1,1);
	Vec3D pc = (pv.a+pv.b+pv.c+pv.d)*0.25f;
	Vec3D gc = (groups[pr.group].b1 + groups[pr.group].b2)*0.5f;
	glVertex3fv(pc);
	glVertex3fv(gc);
	}
	glEnd();
	glColor4f(1,1,1,1);
	// draw portals
	for (int i=0; i<nP; ++i) {
	glBegin(GL_LINE_STRIP);
	glVertex3fv(pvs[i].d);
	glVertex3fv(pvs[i].c);
	glVertex3fv(pvs[i].b);
	glVertex3fv(pvs[i].a);
	glEnd();
	}
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	*/
}

void WMO::drawSelect(int doodadset, const Vec3D &ofs, const float rot) const
{
	for (unsigned int i = 0; i<nGroups; ++i) {
		groups[i].draw(ofs, rot, true);

		if (gWorld->drawdoodads) {
			groups[i].drawDoodadsSelect(doodadset, ofs, rot);
		}

		groups[i].drawLiquid();
	}
}

void WMO::drawSkybox(Vec3D pCamera, Vec3D pLower, Vec3D pUpper) const
{
	if (skybox && pCamera.IsInsideOf(pLower, pUpper))
	{
		//! \todo  only draw sky if we are "inside" the WMO... ?

		// We need to clear the depth buffer, because the skybox model can (will?)
		// require it *. This is inefficient - is there a better way to do this?
		// * planets in front of "space" in Caverns of Time
		//glClear(GL_DEPTH_BUFFER_BIT);

		// update: skybox models seem to have an explicit renderop ordering!
		// that saves us the depth buffer clear and the depth testing, too

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glPushMatrix();
		Vec3D o = gWorld->camera;
		glTranslatef(o.x, o.y, o.z);
		const float sc = 2.0f;
		glScalef(sc, sc, sc);
		skybox->draw();
		glPopMatrix();
		gWorld->hadSky = true;
		glEnable(GL_DEPTH_TEST);
	}
}

/*
void WMO::drawPortals()
{
// not used ;)
glBegin(GL_QUADS);
for (int i=0; i<nP; ++i) {
glVertex3fv(pvs[i].d);
glVertex3fv(pvs[i].c);
glVertex3fv(pvs[i].b);
glVertex3fv(pvs[i].a);
}
glEnd();
}
*/

void WMOLight::init(MPQFile* f)
{
	char type[4];
	f->read(&type, 4);
	f->read(&color, 4);
	f->read(pos, 12);
	f->read(&intensity, 4);
	f->read(unk, 4 * 5);
	f->read(&r, 4);

	pos = Vec3D(pos.x, pos.z, -pos.y);

	// rgb? bgr? hm
	float fa = ((color & 0xff000000) >> 24) / 255.0f;
	float fr = ((color & 0x00ff0000) >> 16) / 255.0f;
	float fg = ((color & 0x0000ff00) >> 8) / 255.0f;
	float fb = ((color & 0x000000ff)) / 255.0f;

	fcolor = Vec4D(fr, fg, fb, fa);
	fcolor *= intensity;
	fcolor.w = 1.0f;

	/*
	// light logging
	gLog("Light %08x @ (%4.2f,%4.2f,%4.2f)\t %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f, %4.2f\t(%d,%d,%d,%d)\n",
	color, pos.x, pos.y, pos.z, intensity,
	unk[0], unk[1], unk[2], unk[3], unk[4], r,
	type[0], type[1], type[2], type[3]);
	*/
}

void WMOLight::setup(GLint light)
{
	// not used right now -_-

	GLfloat LightAmbient[] = { 0, 0, 0, 1.0f };
	GLfloat LightPosition[] = { pos.x, pos.y, pos.z, 0.0f };

	glLightfv(light, GL_AMBIENT, LightAmbient);
	glLightfv(light, GL_DIFFUSE, fcolor);
	glLightfv(light, GL_POSITION, LightPosition);

	glEnable(light);
}

void WMOLight::setupOnce(GLint light, Vec3D dir, Vec3D lcol)
{
	Vec4D position(dir, 0);
	//Vec4D position(0,1,0,0);

	Vec4D ambient = Vec4D(lcol * 0.3f, 1);
	//Vec4D ambient = Vec4D(0.101961f, 0.062776f, 0, 1);
	Vec4D diffuse = Vec4D(lcol, 1);
	//Vec4D diffuse = Vec4D(0.439216f, 0.266667f, 0, 1);

	glLightfv(light, GL_AMBIENT, ambient);
	glLightfv(light, GL_DIFFUSE, diffuse);
	glLightfv(light, GL_POSITION, position);

	glEnable(light);
}



void WMOGroup::init(WMO *_wmo, MPQFile* f, int _num, char *names)
{
	this->wmo = _wmo;
	this->num = _num;

	// extract group info from f
	f->read(&flags, 4);
	float ff[3];
	f->read(ff, 12);
	VertexBoxMax = Vec3D(ff[0], ff[1], ff[2]);
	f->read(ff, 12);
	VertexBoxMin = Vec3D(ff[0], ff[1], ff[2]);
	int nameOfs;
	f->read(&nameOfs, 4);

	//! \todo  get proper name from group header and/or dbc?
	if (nameOfs > 0) {
		name = std::string(names + nameOfs);
	}
	else name = "(no name)";

	ddr = 0;
	nDoodads = 0;

	lq = 0;
}


struct WMOBatch {
	signed char bytes[12];
	uint32_t indexStart;
	uint16_t indexCount, vertexStart, vertexEnd;
	unsigned char flags, texture;
};

void setGLColor(unsigned int col)
{
	//glColor4ubv((GLubyte*)(&col));
	GLubyte r, g, b, a;
	a = (col & 0xFF000000) >> 24;
	r = (col & 0x00FF0000) >> 16;
	g = (col & 0x0000FF00) >> 8;
	b = (col & 0x000000FF);
	glColor4ub(r, g, b, 1);
}

Vec4D colorFromInt(unsigned int col) {
	GLubyte r, g, b, a;
	a = (col & 0xFF000000) >> 24;
	r = (col & 0x00FF0000) >> 16;
	g = (col & 0x0000FF00) >> 8;
	b = (col & 0x000000FF);
	return Vec4D(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}
struct WMOGroupHeader {
	uint32_t nameStart, nameStart2, flags;
	float box1[3], box2[3];
	uint16_t portalStart, portalCount;
	uint16_t batches[4];
	uint8_t fogs[4];
	int32_t unk1, id, unk2, unk3;
};

void WMOGroup::initDisplayList()
{
	Vec3D *vertices = NULL;
	Vec3D *normals = NULL;
	Vec2D *texcoords = NULL;
	uint16_t *indices = NULL;
	struct SMOPoly *materials = NULL;
	WMOBatch *batches = NULL;

	WMOGroupHeader gh;

	uint16_t *useLights = 0;
	int nLR = 0;


	// open group file

	std::stringstream curNum;
	curNum << "_" << std::setw(3) << std::setfill('0') << num;

	std::string fname = wmo->filename();
	fname.insert(fname.find(".wmo"), curNum.str());

	MPQFile gf(fname);
	if (gf.isEof()) {
		LogError << "Error loading WMO \"" << fname << "\"." << std::endl;
		return;
	}

	/*if(!gf.isExternal())
	gLog("    Loading WMO from MPQ %s\n", fname);
	else
	gLog("    Loading WMO from File %s\n", fname);
	*/
	gf.seek(0x14);
	// read header
	gf.read(&gh, sizeof(WMOGroupHeader));
	WMOFog &wf = wmo->fogs[gh.fogs[0]];
	if (wf.r2 <= 0) fog = -1; // default outdoor fog..?
	else fog = gh.fogs[0];

	BoundingBoxMin = Vec3D(gh.box1[0], gh.box1[2], -gh.box1[1]);
	BoundingBoxMax = Vec3D(gh.box2[0], gh.box2[2], -gh.box2[1]);

	gf.seek(0x58); // first chunk

	uint32_t fourcc;
	uint32_t size;

	unsigned int *cv = NULL;
	hascv = false;

	while (!gf.isEof()) {
		gf.read(&fourcc, 4);
		gf.read(&size, 4);

		size_t nextpos = gf.getPos() + size;

		// why copy stuff when I can just map it from memory ^_^

		if (fourcc == 'MOPY') {
			// materials per triangle
			nTriangles = size / 2;
			materials = reinterpret_cast<SMOPoly*>(gf.getPointer());
		}
		else if (fourcc == 'MOVI') {
			// indices
			indices = reinterpret_cast<uint16_t*>(gf.getPointer());
		}
		else if (fourcc == 'MOVT') {
			nVertices = size / 12;
			// let's hope it's padded to 12 bytes, not 16...
			vertices = reinterpret_cast<Vec3D*>(gf.getPointer());
			VertexBoxMin = Vec3D(9999999.0f, 9999999.0f, 9999999.0f);
			VertexBoxMax = Vec3D(-9999999.0f, -9999999.0f, -9999999.0f);
			rad = 0;
			for (size_t i = 0; i<nVertices; ++i) {
				Vec3D v(vertices[i].x, vertices[i].z, -vertices[i].y);
				if (v.x < VertexBoxMin.x) VertexBoxMin.x = v.x;
				if (v.y < VertexBoxMin.y) VertexBoxMin.y = v.y;
				if (v.z < VertexBoxMin.z) VertexBoxMin.z = v.z;
				if (v.x > VertexBoxMax.x) VertexBoxMax.x = v.x;
				if (v.y > VertexBoxMax.y) VertexBoxMax.y = v.y;
				if (v.z > VertexBoxMax.z) VertexBoxMax.z = v.z;
			}
			center = (VertexBoxMax + VertexBoxMin) * 0.5f;
			rad = (VertexBoxMax - center).length() + 300.0f;
		}
		else if (fourcc == 'MONR') {
			normals = reinterpret_cast<Vec3D*>(gf.getPointer());
		}
		else if (fourcc == 'MOTV') {
			texcoords = reinterpret_cast<Vec2D*>(gf.getPointer());
		}
		else if (fourcc == 'MOLR') {
			nLR = size / 2;
			useLights = reinterpret_cast<uint16_t*>(gf.getPointer());
		}
		else if (fourcc == 'MODR') {
			nDoodads = size / 2;
			ddr = new int16_t[nDoodads];
			gf.read(ddr, size);
		}
		else if (fourcc == 'MOBA') {
			nBatches = size / 24;
			batches = reinterpret_cast<WMOBatch*>(gf.getPointer());
		}
		else if (fourcc == 'MOCV') {
			//gLog("CV: %d\n", size);
			hascv = true;
			cv = reinterpret_cast<uint32_t*>(gf.getPointer());
		}
		else if (fourcc == 'MLIQ') {
			// liquids
			WMOLiquidHeader hlq;
			gf.read(&hlq, 0x1E);

			//gLog("WMO Liquid: %dx%d, %dx%d, (%f,%f,%f) %d\n", hlq.X, hlq.Y, hlq.A, hlq.B, hlq.pos.x, hlq.pos.y, hlq.pos.z, hlq.type);

			// Do not even try to render water (nonamed programmer)... I'll try (beket) =))
			lq = new Liquid(hlq.A, hlq.B, Vec3D(hlq.pos.x, hlq.pos.z, -hlq.pos.y));
			lq->initFromWMO(&gf, wmo->mat[hlq.type], (flags & 0x2000) != 0);
		}

		//! \todo  figure out/use MFOG ?

		gf.seek(nextpos);
	}

	// ok, make a display list

	indoor = (flags & 8192) != 0;
	//gLog("Lighting: %s %X\n\n", indoor?"Indoor":"Outdoor", flags);

	initLighting(nLR, useLights);

	//dl = glGenLists(1);
	//glNewList(dl, GL_COMPILE);
	//glDisable(GL_BLEND);
	//glColor4f(1,1,1,1);

	/*
	float xr=0,xg=0,xb=0;
	if (flags & 0x0040) xr = 1;
	if (flags & 0x2000) xg = 1;
	if (flags & 0x8000) xb = 1;
	glColor4f(xr,xg,xb,1);
	*/

	// generate lists for each batch individually instead
	_lists.resize(nBatches);

	// assume that texturing is on, for unit 1

	for (int b = 0; b<nBatches; b++)
	{
		WMOBatch *batch = &batches[b];
		WMOMaterial *mat = &wmo->mat[batch->texture];

		bool overbright = ((mat->flags & 0x10) && !hascv);
		bool spec_shader = (mat->specular && !hascv && !overbright);

		_lists[b].first = new OpenGL::CallList();
		_lists[b].second = spec_shader;

		_lists[b].first->startRecording(GL_COMPILE);

		mat->_texture->bind();

		bool atest = (mat->transparent) != 0;

		if (atest) {
			glEnable(GL_ALPHA_TEST);
			float aval = 0;
			if (mat->flags & 0x80) aval = 0.3f;
			if (mat->flags & 0x01) aval = 0.0f;
			glAlphaFunc(GL_GREATER, aval);
		}

		if (mat->flags & 0x04) glDisable(GL_CULL_FACE);
		else glEnable(GL_CULL_FACE);

		if (spec_shader) {
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, colorFromInt(mat->col2));
		}
		else {
			Vec4D nospec(0, 0, 0, 1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, nospec);
		}

		if (overbright) {
			//! \todo  use emissive color from the WMO Material instead of 1,1,1,1
			GLfloat em[4] = { 1, 1, 1, 1 };
			glMaterialfv(GL_FRONT, GL_EMISSION, em);
		}

		// render
		glBegin(GL_TRIANGLES);
		for (int t = 0, i = batch->indexStart; t<batch->indexCount; t++, ++i) {
			int a = indices[i];
			if (indoor && hascv) {
				setGLColor(cv[a]);
			}
			glNormal3f(normals[a].x, normals[a].z, -normals[a].y);
			glTexCoord2fv(texcoords[a]);
			glVertex3f(vertices[a].x, vertices[a].z, -vertices[a].y);
		}
		glEnd();

		if (overbright) {
			GLfloat em[4] = { 0, 0, 0, 1 };
			glMaterialfv(GL_FRONT, GL_EMISSION, em);
		}

		if (atest) {
			glDisable(GL_ALPHA_TEST);
		}

		_lists[b].first->endRecording();
	}

	indoor = false;
}


void WMOGroup::initLighting(int /*nLR*/, uint16_t* /*useLights*/)
{
	//dl_light = 0;
	// "real" lighting?
	if ((flags & 0x2000) && hascv) {

		Vec3D dirmin(1, 1, 1);
		float lenmin;
		int lmin;

		for (int i = 0; i<nDoodads; ++i) {
			lenmin = 999999.0f*999999.0f;
			lmin = 0;
			ModelInstance &mi = wmo->modelis[ddr[i]];
			for (unsigned int j = 0; j<wmo->nLights; j++) {
				WMOLight &l = wmo->lights[j];
				Vec3D dir = l.pos - mi.pos;
				float ll = dir.lengthSquared();
				if (ll < lenmin) {
					lenmin = ll;
					dirmin = dir;
					lmin = j;
				}
			}
			mi.ldir = dirmin;
		}

		outdoorLights = false;
	}
	else {
		outdoorLights = true;
	}
}

void WMOGroup::draw(const Vec3D& ofs, const float rot, bool selection)
{
	visible = false;
	// view frustum culling

	Vec3D pos = center + ofs;

	rotate(ofs.x, ofs.z, &pos.x, &pos.z, rot*(float)PI / 180.0f);

	if (!gWorld->frustum.intersectsSphere(pos, rad)) return;

	float dist = (pos - gWorld->camera).length() - rad;
	if (dist >= gWorld->culldistance) return;
	visible = true;
	setupFog();

	if (hascv) {
		glDisable(GL_LIGHTING);
		gWorld->outdoorLights(false);
	}
	else
	{

		if (gWorld->skies->hasSkies())
		{
			gWorld->outdoorLights(true);
		}
		else glDisable(GL_LIGHTING);
	}


	//glCallList(dl);
	glDisable(GL_BLEND);
	glColor4f(1, 1, 1, 1);
	for (int i = 0; i<nBatches; ++i)
	{
		if (video.mSupportShaders && _lists[i].second && wmoShader)
		{
			wmoShader->bind();
			_lists[i].first->render();
			wmoShader->unbind();
		}
		else
		{
			_lists[i].first->render();
		}
	}

	glColor4f(1, 1, 1, 1);
	glEnable(GL_CULL_FACE);

	if (hascv && !selection && gWorld->lighting)
		glEnable(GL_LIGHTING);


}

void WMOGroup::drawDoodads(unsigned int doodadset, const Vec3D& ofs, const float rot)
{

	if (!visible) return;
	if (nDoodads == 0) return;

	gWorld->outdoorLights(outdoorLights);
	setupFog();

	/*
	float xr=0,xg=0,xb=0;
	if (flags & 0x0040) xr = 1;
	//if (flags & 0x0008) xg = 1;
	if (flags & 0x8000) xb = 1;
	glColor4f(xr,xg,xb,1);
	*/

	// draw doodads
	glColor4f(1, 1, 1, 1);
	for (int i = 0; i<nDoodads; ++i) {
		int16_t dd = ddr[i];
		if (!(wmo->doodadsets.size() < doodadset))
			if ((dd >= wmo->doodadsets[doodadset].start) && (dd < (wmo->doodadsets[doodadset].start + wmo->doodadsets[doodadset].size))) {

			ModelInstance &mi = wmo->modelis[dd];

			if (!outdoorLights) {
				WMOLight::setupOnce(GL_LIGHT2, mi.ldir, mi.lcol);
			}
			setupFog();
			wmo->modelis[dd].draw2(ofs, rot);
			}
	}

	glDisable(GL_LIGHT2);

	glColor4f(1, 1, 1, 1);

}


void WMOGroup::drawDoodadsSelect(unsigned int doodadset, const Vec3D& ofs, const float rot)
{
	if (!visible) return;
	if (nDoodads == 0) return;


	gWorld->outdoorLights(outdoorLights);
	setupFog();

	/*
	float xr=0,xg=0,xb=0;
	if (flags & 0x0040) xr = 1;
	//if (flags & 0x0008) xg = 1;
	if (flags & 0x8000) xb = 1;
	glColor4f(xr,xg,xb,1);
	*/

	// draw doodads
	glColor4f(1, 1, 1, 1);
	for (int i = 0; i<nDoodads; ++i) {
		int16_t dd = ddr[i];
		if (!(wmo->doodadsets.size() < doodadset))
			if ((dd >= wmo->doodadsets[doodadset].start) && (dd < (wmo->doodadsets[doodadset].start + wmo->doodadsets[doodadset].size))) {

			ModelInstance &mi = wmo->modelis[dd];

			if (!outdoorLights) {
				WMOLight::setupOnce(GL_LIGHT2, mi.ldir, mi.lcol);
			}

			wmo->modelis[dd].draw2Select(ofs, rot);
			}
	}

	glDisable(GL_LIGHT2);

	glColor4f(1, 1, 1, 1);

}

void WMOGroup::drawLiquid()
{
	if (!visible) return;

	// draw liquid
	//! \todo  culling for liquid boundingbox or something
	if (lq) {
		setupFog();
		if (outdoorLights) {
			gWorld->outdoorLights(true);
		}
		else {
			//! \todo  setup some kind of indoor lighting... ?
			gWorld->outdoorLights(false);
			glEnable(GL_LIGHT2);
			glLightfv(GL_LIGHT2, GL_AMBIENT, Vec4D(0.1f, 0.1f, 0.1f, 1));
			glLightfv(GL_LIGHT2, GL_DIFFUSE, Vec4D(0.8f, 0.8f, 0.8f, 1));
			glLightfv(GL_LIGHT2, GL_POSITION, Vec4D(0, 1, 0, 0));
		}
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDepthMask(GL_TRUE);
		glColor4f(1, 1, 1, 1);
		lq->draw();
		glDisable(GL_LIGHT2);
	}
}

void WMOGroup::setupFog()
{
	if (outdoorLights || fog == -1) {
		gWorld->setupFog();
	}
	else {
		wmo->fogs[fog].setup();
	}
}



WMOGroup::~WMOGroup()
{
	//if (dl) glDeleteLists(dl, 1);
	//if (dl_light) glDeleteLists(dl_light, 1);
	for (std::vector< std::pair<OpenGL::CallList*, bool> >::iterator it = _lists.begin(); it != _lists.end(); ++it)
	{
		delete it->first;
	}
	_lists.clear();

	if (nDoodads)
	{
		delete[] ddr;
		ddr = NULL;
	}
	if (lq)
	{
		delete lq;
		lq = NULL;
	}
}


void WMOFog::init(MPQFile* f)
{
	f->read(this, 0x30);
	color = Vec4D(((color1 & 0x00FF0000) >> 16) / 255.0f, ((color1 & 0x0000FF00) >> 8) / 255.0f,
		(color1 & 0x000000FF) / 255.0f, ((color1 & 0xFF000000) >> 24) / 255.0f);
	float temp;
	temp = pos.y;
	pos.y = pos.z;
	pos.z = -temp;
	fogstart = fogstart * fogend * 1.5f;
	fogend *= 1.5;
}

void WMOFog::setup()
{
	/*if (gWorld->drawfog) {
	glFogfv(GL_FOG_COLOR, color);
	glFogf(GL_FOG_START, fogstart);
	glFogf(GL_FOG_END, fogend);

	glEnable(GL_FOG);
	} else {
	glDisable(GL_FOG);
	}*/
	glDisable(GL_FOG);
}

WMOManager::mapType WMOManager::items;

void WMOManager::report()
{
	std::string output = "Still in the WMO manager:\n";
	for (mapType::iterator t = items.begin(); t != items.end(); ++t)
	{
		output += "- " + t->first + "\n";
	}
	LogDebug << output;
}

WMO* WMOManager::add(std::string name)
{
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	if (items.find(name) == items.end())
	{
		items[name] = new WMO(name);
		//! \todo Uncomment this, if loading is threaded.
		//items[name]->finishLoading();
		//app.loader()->addObject( items[name] );
	}

	items[name]->addReference();
	return items[name];
}

void WMOManager::delbyname(std::string name)
{
	std::transform(name.begin(), name.end(), name.begin(), ::tolower);

	if (items.find(name) != items.end())
	{
		items[name]->removeReference();

		if (items[name]->hasNoReferences())
		{
			delete items[name];
			items.erase(items.find(name));
		}
	}
}

