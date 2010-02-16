#ifndef ANIMATED_H
#define ANIMATED_H

#include <cassert>
#include <utility>
#include <vector>

#include "vec3d.h"
#include "quaternion.h"
#include "modelheaders.h"

// interpolation functions
template<class T>
inline T interpolate(const float r, const T &v1, const T &v2)
{
	return v1*(1.0f - r) + v2*r;
}

template<class T>
inline T interpolateHermite(const float r, const T &v1, const T &v2, const T &in, const T &out)
{
	// dummy
    //return interpolate<T>(r,v1,v2);

	// basis functions
	float h1 = 2.0f*r*r*r - 3.0f*r*r + 1.0f;
	float h2 = -2.0f*r*r*r + 3.0f*r*r;
	float h3 = r*r*r - 2.0f*r*r + r;
	float h4 = r*r*r - r*r;

	// interpolation
	return v1*h1 + v2*h2 + in*h3 + out*h4;
}

// "linear" interpolation for quaternions should be slerp by default
template<>
inline Quaternion interpolate<Quaternion>(const float r, const Quaternion &v1, const Quaternion &v2)
{
	return Quaternion::slerp(r, v1, v2);
}


typedef std::pair<size_t, size_t> AnimRange;

// global time for global sequences
extern int globalTime;

enum Interpolations {
	INTERPOLATION_NONE,
	INTERPOLATION_LINEAR,
	INTERPOLATION_HERMITE
};

template <class T>
class Identity {
public:
	static const T& conv(const T& t)
	{
		return t;
	}
};

// In WoW 2.0+ Blizzard are now storing rotation data in 16bit values instead of 32bit.
// I don't really understand why as its only a very minor saving in model sizes and adds extra overhead in
// processing the models.  Need this structure to read the data into.
struct PACK_QUATERNION {int16_t x,y,z,w;  }; 

class Quat16ToQuat32 {
public:
	static const Quaternion conv(const PACK_QUATERNION t)
	{
		return Quaternion(
			float(t.x > 0? t.x - 32767 : t.x + 32767)/ 32767.0f, 
			float(t.y > 0? t.y - 32767 : t.y + 32767)/ 32767.0f,
			float(t.z > 0? t.z - 32767 : t.z + 32767)/ 32767.0f,
			float(t.w > 0? t.w - 32767 : t.w + 32767)/ 32767.0f);
	}
};

// Convert opacity values stored as shorts to floating point
// I wonder why Blizzard decided to save 2 bytes by doing this
class ShortToFloat {
public:
	static const float conv(const short t)
	{
		return t/32767.0f;
	}
};

/*
	Generic animated value class:

	T is the data type to animate
	D is the data type stored in the file (by default this is the same as T)
	Conv is a conversion object that defines T conv(D) to convert from D to T
		(by default this is an identity function)
	(there might be a nicer way to do this? meh meh)
*/

template <class T, class D=T, class Conv=Identity<T> >
class Animated {
public:

	bool used;
	int type, seq;
	int *globals;

	std::map<int, std::vector<unsigned int> > times;
	std::map<int, std::vector<T> > data;
	// for nonlinear interpolations:
	std::map<int, std::vector<T> > in, out;
	size_t size; // for fix function

	T getValue(unsigned int anim, unsigned int time)
	{
 // by Flow
		if( !data[anim].size() ) { // HACK
			return T();
		}


		if (type != INTERPOLATION_NONE || data[anim].size()>1) {
			// obtain a time value and a data range
			if (seq>-1) {
				if (globals[seq]==0) 
					time = 0;
				else 
					time = globalTime % globals[seq];
			}

 			if (times[anim].size() > 1) {
				size_t t1, t2;
				size_t pos=0;
				for (size_t i=0; i<times[anim].size()-1; i++) {
					if (time >= times[anim][i] && time < times[anim][i+1]) {
						pos = i;
						break;
					}
				}
				t1 = times[anim][pos];
				t2 = times[anim][pos+1];
				float r = (time-t1)/(float)(t2-t1);

				if (type == INTERPOLATION_LINEAR) 
					return interpolate<T>(r,data[anim][pos],data[anim][pos+1]);
				else if (type == INTERPOLATION_NONE) 
					return data[anim][pos];
				else {
					// INTERPOLATION_HERMITE is only used in cameras afaik?
					return interpolateHermite<T>(r,data[anim][pos],data[anim][pos+1],in[anim][pos],out[anim][pos]);
				}
			} else {
				return data[anim][0];
			}
		} else {
			// default value
			if (data[anim].size() == 0)
				return 0;
			else
				return data[anim][0];
		}
	}

	void init(AnimationBlock &b, MPQFile &f, int *gs)
	{
		globals = gs;
		type = b.type;
		seq = b.seq;
		if (seq!=-1) {
			assert(gs);
		}

		// Old method
		//used = (type != INTERPOLATION_NONE) || (seq != -1);
		// New method suggested by Cryect
		used = (b.nKeys > 0);


		// times
		assert(b.nTimes == b.nKeys);
	// by Flow
		size = b.nTimes;
		if( b.nTimes == 0 )
			return;

		for(size_t j=0; j < b.nTimes; j++) {
			AnimationBlockHeader* pHeadTimes = (AnimationBlockHeader*)(f.getBuffer() + b.ofsTimes + j*sizeof(AnimationBlockHeader));
		
			uint32_t *ptimes = (uint32_t*)(f.getBuffer() + pHeadTimes->ofsEntrys);
			for (size_t i=0; i < pHeadTimes->nEntrys; i++)
				times[j].push_back(ptimes[i]);
		}

		// keyframes
		for(size_t j=0; j < b.nKeys; j++) {
			AnimationBlockHeader* pHeadKeys = (AnimationBlockHeader*)(f.getBuffer() + b.ofsKeys + j*sizeof(AnimationBlockHeader));

			D *keys = (D*)(f.getBuffer() + pHeadKeys->ofsEntrys);
			switch (type) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) 
						data[j].push_back(Conv::conv(keys[i]));
					break;
				case INTERPOLATION_HERMITE:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						data[j].push_back(Conv::conv(keys[i*3]));
						in[j].push_back(Conv::conv(keys[i*3+1]));
						out[j].push_back(Conv::conv(keys[i*3+2]));
					}
					break;
			}
		}
	}

	void init(FakeAnimationBlock &b, MPQFile &f, int *gs)
	{
		globals = gs;
		type = 0;
		seq = -1;
		if (seq!=-1) {
			assert(gs);
		}

		// Old method
		//used = (type != INTERPOLATION_NONE) || (seq != -1);
		// New method suggested by Cryect
		used = (b.nKeys > 0);


		// times
		assert(b.nTimes == b.nKeys);
	// by Flow
		size = b.nTimes;
		if( b.nTimes == 0 )
			return;

		for(size_t j=0; j < b.nTimes; j++) {
			AnimationBlockHeader* pHeadTimes = (AnimationBlockHeader*)(f.getBuffer() + b.ofsTimes + j*sizeof(AnimationBlockHeader));
		
			uint32_t *ptimes = (uint32_t*)(f.getBuffer() + pHeadTimes->ofsEntrys);
			for (size_t i=0; i < pHeadTimes->nEntrys; i++)
				times[j].push_back(ptimes[i]);
		}

		// keyframes
		for(size_t j=0; j < b.nKeys; j++) {
			AnimationBlockHeader* pHeadKeys = (AnimationBlockHeader*)(f.getBuffer() + b.ofsKeys + j*sizeof(AnimationBlockHeader));

			D *keys = (D*)(f.getBuffer() + pHeadKeys->ofsEntrys);
			switch (type) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) 
						data[j].push_back(Conv::conv(keys[i]));
					break;
				case INTERPOLATION_HERMITE:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						data[j].push_back(Conv::conv(keys[i*3]));
						in[j].push_back(Conv::conv(keys[i*3+1]));
						out[j].push_back(Conv::conv(keys[i*3+2]));
					}
					break;
			}
		}
	}

	void init(AnimationBlock &b, MPQFile &f, int *gs, MPQFile *animfiles)
	{
		globals = gs;
		type = b.type;
		seq = b.seq;
		if (seq!=-1) {
			assert(gs);
		}

		// Old method
		//used = (type != INTERPOLATION_NONE) || (seq != -1);
		// New method suggested by Cryect
		used = (b.nKeys > 0);

		// times
		assert(b.nTimes == b.nKeys);
		size = b.nTimes;
		if( b.nTimes == 0 )
			return;

		for(size_t j=0; j < b.nTimes; j++) {
			AnimationBlockHeader* pHeadTimes = (AnimationBlockHeader*)(f.getBuffer() + b.ofsTimes + j*sizeof(AnimationBlockHeader));
			uint32_t *ptimes;
			if (animfiles[j].getSize() > 0)
				ptimes = (uint32_t*)(animfiles[j].getBuffer() + pHeadTimes->ofsEntrys);
			else
				ptimes = (uint32_t*)(f.getBuffer() + pHeadTimes->ofsEntrys);
			for (size_t i=0; i < pHeadTimes->nEntrys; i++)
				times[j].push_back(ptimes[i]);

		}

		// keyframes
		for(size_t j=0; j < b.nKeys; j++) {
			AnimationBlockHeader* pHeadKeys = (AnimationBlockHeader*)(f.getBuffer() + b.ofsKeys + j*sizeof(AnimationBlockHeader));
			assert((D*)(f.getBuffer() + pHeadKeys->ofsEntrys));
			D *keys;
			if (animfiles[j].getSize() > 0)
				keys = (D*)(animfiles[j].getBuffer() + pHeadKeys->ofsEntrys);
			else 
				keys = (D*)(f.getBuffer() + pHeadKeys->ofsEntrys);
			switch (type) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) 
						data[j].push_back(Conv::conv(keys[i]));
					break;
				case INTERPOLATION_HERMITE:
					for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						data[j].push_back(Conv::conv(keys[i*3]));
						in[j].push_back(Conv::conv(keys[i*3+1]));
						out[j].push_back(Conv::conv(keys[i*3+2]));
					}
					break;
			}
		}
	}

	void fix(T fixfunc(const T))
	{
		switch (type) {
			case INTERPOLATION_NONE:
			case INTERPOLATION_LINEAR:
				for (size_t i=0; i<size; i++) {
					for (size_t j=0; j<data[i].size(); j++) {
						data[i][j] = fixfunc(data[i][j]);
					}
				}
				break;
			case INTERPOLATION_HERMITE:
				for (size_t i=0; i<size; i++) {
					for (size_t j=0; j<data[i].size(); j++) {
						data[i][j] = fixfunc(data[i][j]);
						in[i][j] = fixfunc(in[i][j]);
						out[i][j] = fixfunc(out[i][j]);
					}
				}
				break;
		}
	}

};

typedef Animated<float,short,ShortToFloat> AnimatedShort;

#endif
