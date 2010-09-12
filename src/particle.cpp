#include "particle.h"
#include "noggit.h"

static const int MAX_PARTICLES = 10000;

Vec4D fromARGB(uint32_t color)
{
	const float a = ((color & 0xFF000000) >> 24) / 255.0f;
	const float r = ((color & 0x00FF0000) >> 16) / 255.0f;
	const float g = ((color & 0x0000FF00) >>  8) / 255.0f;
	const float b = ((color & 0x000000FF)      ) / 255.0f;
    return Vec4D(r,g,b,a);
}

template<class T>
T lifeRamp(float life, float mid, const T &a, const T &b, const T &c)
{
	if (life<=mid) return interpolate<T>(life / mid,a,b);
	else return interpolate<T>((life-mid) / (1.0f-mid),b,c);
}


void ParticleSystem::init(MPQFile &f, ModelParticleEmitterDef &mta, int *globals)
{
	speed.init	 (mta.Emissionspeed, f, globals);
	variation.init(mta.SpeedVariation, f, globals);
	spread.init	 (mta.VerticalRange, f, globals);
	lat.init	 (mta.HorizontalRange, f, globals);
	gravity.init (mta.Gravity, f, globals);
	lifespan.init(mta.Lifespan, f, globals);
	rate.init	 (mta.EmissionRate, f, globals);
	areal.init	 (mta.EmissionAreaLength, f, globals);
	areaw.init	 (mta.EmissionAreaWidth, f, globals);
	grav2.init	 (mta.Gravity2, f, globals);

	color.init( mta.colors, f, globals );
	size.init( mta.sizes, f, globals );
	opac.init( mta.opacity, f, globals );

	//for (size_t i=0; i<3; i++) {
	//colors[i] = fromARGB(mta.p.colors[i]);
	//sizes[i] = mta.p.sizes[i];// * mta.p.scales[i];
	//}
	mid = 1.0f;//mta.p.mid; Not existant anymore ..
	slowdown = mta.slowdown;
	rotation = mta.rotation;
	pos = fixCoordSystem(mta.pos);
	texture = model->textures[mta.texture];
	blend = mta.blend;
	rows = mta.rows;
	cols = mta.cols;
	type = mta.particletype;
	//order = mta.s2;
	order = mta.particletype>0 ? -1 : 0;
	parent = model->bones + mta.bone;

	switch (mta.emittertype) {
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

	tofs = frand();

	// init tiles
	for (int i=0; i<rows*cols; i++) {
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

	for (int i=0; i<4; i++) {
		tc[(i+4-order) & 3] = otc[i];
	}
}


void ParticleSystem::update(float dt)
{
	float grav = gravity.getValue(manim, mtime);

	// spawn new particles
	if (emitter) {
		float frate = rate.getValue(manim, mtime);
		float flife = 1.0f;
		//flife = lifespan.getValue(manim, mtime);

		float ftospawn = (dt * frate / flife) + rem;
		if (ftospawn < 1.0f) {
			rem = ftospawn;
			if (rem<0) rem = 0;
		}
		else {
			int tospawn = (int)ftospawn;
			rem = ftospawn - (float)tospawn;
			//rem = 0;
			for (int i=0; i<tospawn; i++) {
				Particle p = emitter->newParticle(manim, mtime);
				// sanity check:
				if (particles.size() < MAX_PARTICLES) particles.push_back(p);
			}
		}
	}

	float mspeed = 1.0f;

	for (ParticleList::iterator it = particles.begin(); it != particles.end(); ) {
		Particle &p = *it;
		p.speed += p.down * grav * dt;

		if (slowdown>0) {
			mspeed = expf(-1.0f * slowdown * p.life);
		}
		p.pos += p.speed * mspeed * dt;

		p.life += dt;
		float rlife = p.life / p.maxlife;
		// calculate size and color based on lifetime
		p.size = (size.getValue(manim, mtime).x+size.getValue(manim, mtime).y) / 2;	//! \todo  actually size them right and not only quadratic.
		p.color = Vec4D( color.getValue(manim, mtime).x, color.getValue(manim, mtime).y, color.getValue(manim, mtime).z, opac.getValue(manim, mtime) );

		//lifeRamp<float>(rlife, mid, sizes[0], sizes[1], sizes[2]);
		//lifeRamp<Vec4D>(rlife, mid, colors[0], colors[1], colors[2]);

		// kill off old particles
		if (rlife >= 1.0f) particles.erase(it++);
		else ++it;
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

//	glPushName(texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	Matrix mbb;
	mbb.unit();

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
			glColor4fv(it->color);

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

	glEnable(GL_LIGHTING);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);
	glColor4f(1,1,1,1);
//	glPopName();
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

//	glPushName(texture);
	glBindTexture(GL_TEXTURE_2D, texture);

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
//	glPopName();
}

Particle PlaneParticleEmitter::newParticle(int anim, int time)
{
    Particle p;
	//! \todo  maybe evaluate these outside the spawn function, since they will be common for a given frame?
	float w = sys->areal.getValue(anim, time) * 0.5f;
	float l = sys->areaw.getValue(anim, time) * 0.5f;
	float spd = sys->speed.getValue(anim, time);
	float var = sys->variation.getValue(anim, time);

	p.pos = sys->pos + Vec3D(randfloat(-l,l), 0, randfloat(-w,w));
	p.pos = sys->parent->mat * p.pos;

	Vec3D dir = sys->parent->mrot * Vec3D(0,1,0);
	p.down = Vec3D(0,-1.0f,0); // dir * -1.0f;
	//p.speed = dir.normalize() * randfloat(spd1,spd2);   // ?
	p.speed = dir.normalize() * spd * (1.0f+randfloat(-var,var));

	p.life = 0;
	p.maxlife = sys->lifespan.getValue(anim, time);

	p.origin = p.pos;

	p.tile = randint(0, sys->rows*sys->cols-1);
	return p;
}

Particle SphereParticleEmitter::newParticle(int anim, int time)
{
    Particle p;
	float l = sys->areal.getValue(anim, time);
	float w = sys->areaw.getValue(anim, time);
	float spd = sys->speed.getValue(anim, time);
	float var = sys->variation.getValue(anim, time);

	float t = randfloat(0,2*PI);

	//! \todo  fix shpere emitters to work properly

	//Vec3D bdir(l*cosf(t), 0, w*sinf(t));
	Vec3D bdir(0, l*cosf(t), w*sinf(t));

	/*
	float theta_range = sys->spread.getValue(anim, time);
	float theta = -0.5f* theta_range + randfloat(0, theta_range);
	Vec3D bdir(0, l*cosf(theta), w*sinf(theta));

	float phi_range = sys->lat.getValue(anim, time);
	float phi = randfloat(0, phi_range);
	rotate(0,0, &bdir.z, &bdir.x, phi);
	*/                              

	p.pos = sys->pos + bdir;
	p.pos = sys->parent->mat * p.pos;

	if (bdir.lengthSquared()==0) p.speed = Vec3D(0,0,0);
	else {
		Vec3D dir = sys->parent->mrot * (bdir.normalize());
		p.speed = dir.normalize() * spd * (1.0f+randfloat(-var,var));   // ?
	}

	p.down = sys->parent->mrot * Vec3D(0,-1.0f,0);

	p.life = 0;
	p.maxlife = sys->lifespan.getValue(anim, time);

	p.origin = p.pos;

	p.tile = randint(0, sys->rows*sys->cols-1);
	return p;
}




void RibbonEmitter::init(MPQFile &f, ModelRibbonEmitterDef &mta, int *globals)
{
	color.init(mta.color, f, globals);
	opacity.init(mta.opacity, f, globals);
	above.init(mta.above, f, globals);
	below.init(mta.below, f, globals);

	parent = model->bones + mta.bone;
	int *texlist = (int*)(f.getBuffer() + mta.ofsTextures);
	// just use the first texture for now; most models I've checked only had one
	texture = model->textures[texlist[0]];

	tpos = pos = fixCoordSystem(mta.pos);

	//! \todo  figure out actual correct way to calculate length
	// in BFD, res is 60 and len is 0.6, the trails are very short (too long here)
	// in CoT, res and len are like 10 but the trails are supposed to be much longer (too short here)
	numsegs = (int)mta.res;
	seglen = mta.length;
	length = mta.res * seglen;

	// create first segment
	RibbonSegment rs;
	rs.pos = tpos;
	rs.len = 0;
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
			++it;
		} else {
			segs.erase(it++);
		}
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

//	glPushName(texture);
	glBindTexture(GL_TEXTURE_2D, texture);
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
//	glPopName();
}

