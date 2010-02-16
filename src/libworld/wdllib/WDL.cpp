/*
 * WDL.cpp
 *
 *  Created on: 05.08.2009
 *      Author: Bastian
 */
#include "WDL.h"

namespace libworld
{
namespace wdllib
{

WDL::WDL(){
	_TDn("WDL::WDL()");
	mver=MVER();
	maof=MAOF();
	maho_included=false;
	this->buff=Buffer(maof.getRealSize()+mver.getRealSize());

}

WDL::WDL(Buffer buff){
	_TDn("WDL::WDL(Buffer buff)");
	this->buff=buff;
	this->buff.setPosition(0);
	mver=MVER(this->buff);
	int ofs=0;
	ofs+=mver.getRealSize();
	Buffer tBuff=Buffer(this->buff,ofs);
	maof=MAOF(tBuff);
	bool maho_checked=false;
	maho_included=false;
	for(int i=0;i<4096;i++){

		if(maof.getOffset(i)==0){
			mare_exists[i]=false;
			continue;
		}
		mare_exists[i]=true;
		tBuff=Buffer(this->buff,maof.getOffset(i));

		mare[i]=MARE(tBuff);
		if(!maho_checked){
			maho_checked=true;
			tBuff=Buffer(this->buff,maof.getOffset(i)+0x8C0);
			maho[i]=MAHO(tBuff);
			if(!maho[i].MagicCorrect())
				maho_included=false;
			else
				maho_included=true;
		}
		if(maho_included){
			tBuff=Buffer(this->buff,maof.getOffset(i)+0x8C0);
			maho[i]=MAHO(tBuff);
		}
	}
}

void WDL::Render(){
	_TDn("void WDL::Render()");
	buff=Buffer();
	buff=mver.getChunk();
	buff+=maof.getChunk();
	for(int i=0;i<4096;i++){
		if(mare_exists[i]){
			maof.setOffset(buff.getSize(),i);
			buff+=mare[i].getChunk();
			if(maho_included)
				buff+=maho[i].getChunk();
		}
		else{
			maof.setOffset(0,i);
		}
	}
	buff.setPosition(0xC);
	Buffer tbuff=maof.getChunk();
	buff<<tbuff;
}

Buffer WDL::getFileBuff(){
	Render();
	this->buff.setPosition(0);
	return this->buff;
}

/*
 * Returns a int-buff containing the minimap
 */
unsigned int* WDL::createMinimap(){
	/*
	 * adapted from wowmapview
	 */
	_TDn("unsigned int* WDL::createMinimap()");
	unsigned int *texbuf = new unsigned int[512*512];
	for (int j=0; j<64; j++) {
		for (int i=0; i<64; i++) {
			if (mare_exists[j*64+i]) {

				// make minimap
				// for a 512x512 minimap texture, and 64x64 tiles, one tile is 8x8 pixels
				for (int z=0; z<8; z++) {
					for (int x=0; x<8; x++) {
						short hval =mare[j*64+i].getOuterHeight(z,x);

						// make rgb from height value
						unsigned char r,g,b;
						if (hval < 0) {
							// water = blue
							if (hval < -511) hval = -511;
							hval /= -2;
							r = g = 0;
							b = 255 - hval;
						} else {
							// above water = should apply a palette :(
							/*
							float fh = hval / 1600.0f;
							if (fh > 1.0f) fh = 1.0f;
							unsigned char c = (unsigned char) (fh * 255.0f);
							r = g = b = c;
							*/

							// green: 20,149,7		0-600
							// brown: 137, 84, 21	600-1200
							// gray: 96, 96, 96		1200-1600
							// white: 255, 255, 255
							unsigned char r1,r2,g1,g2,b1,b2;
							float t;

							if (hval < 600) {
								r1 = 20;
								r2 = 137;
								g1 = 149;
								g2 = 84;
								b1 = 7;
								b2 = 21;
								t = hval / 600.0f;
							}
							else if (hval < 1200) {
								r2 = 96;
								r1 = 137;
								g2 = 96;
								g1 = 84;
								b2 = 96;
								b1 = 21;
								t = (hval-600) / 600.0f;
							}
							else /*if (hval < 1600)*/ {
								r1 = 96;
								r2 = 255;
								g1 = 96;
								g2 = 255;
								b1 = 96;
								b2 = 255;
								if (hval >= 1600) hval = 1599;
								t = (hval-1200) / 600.0f;
							}

							// TODO: add a regular palette here

							r = (unsigned char)(r2*t + r1*(1.0f-t));
							g = (unsigned char)(g2*t + g1*(1.0f-t));
							b = (unsigned char)(b2*t + b1*(1.0f-t));
						}

						texbuf[(j*8+z)*512 + i*8+x] = (r) | (g<<8) | (b<<16) | (255 << 24);
					}
				}
			}
		}
	}
	return texbuf;
}

WDL::~WDL(){
	_TDn("WDL::~WDL()");
}

}
}