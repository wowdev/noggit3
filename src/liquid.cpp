#include "liquid.h"
#include "world.h"
#include "shaders.h"
#include "dbc.h"
#include "Log.h"
#include "TextureManager.h" // TextureManager, Texture

struct LiquidVertex {
	unsigned char c[4];
	float h;
};


void Liquid::initFromTerrain(MPQFile &f, int flags)
{
	texRepeats = 4.0f;
	/*
	flags:
	8 - ocean
	4 - river
	16 - magma
	*/
	ydir = 1.0f;
	if (flags & 16) {
		// magma:
		//initTextures<1,30>( "XTEXTURES\\LAVA\\lava.%d.blp" );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		type = 0; // not colored
		pType = 2;
		mTransparency = false;
	}
	else if (flags & 4) {
		// river/lake
		//initTextures<1,30>( "XTEXTURES\\river\\lake_a.%d.blp" )
			initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");;
		type = 2; // dynamic color
		pType = 1;
		mTransparency = true;
	}
	else {
		// ocean
		//initTextures<1,30>( "XTEXTURES\\ocean\\ocean_h.%d.blp" );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		type = 2;
		pType = 0;
		mTransparency = true;
	}
	initGeometry(f);
	trans = false;
}

void Liquid::initFromWMO(MPQFile &f, WMOMaterial &mat, bool indoor)
{
	texRepeats = 4.0f;
	ydir = -1.0f;

	initGeometry(f);

	trans = false;

	// tmpflag is the flags value for the last drawn tile
	if (tmpflag & 1) {
		//initTextures<1,30>( "XTEXTURES\\SLIME\\slime.%d.blp" );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		type = 0;
		texRepeats = 2.0f;
		mTransparency = false;
	}
	else if (tmpflag & 2) {
		//initTextures<1,30>( "XTEXTURES\\LAVA\\lava.%d.blp" );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		type = 0;
		mTransparency = false;
	}
	else {
		//initTextures<1,30>( "XTEXTURES\\river\\lake_a.%d.blp" );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		if (indoor) {
			trans = true;
			type = 1;
			col = Vec3D( ((mat.col2&0xFF0000)>>16)/255.0f, ((mat.col2&0xFF00)>>8)/255.0f, (mat.col2&0xFF)/255.0f);
		} else {
			trans = true;
			type = 2; // outdoor water (...?)
		}
		mTransparency = true;
	}

}


void Liquid::initGeometry(MPQFile &f)
{
	// assume: f is at the appropriate starting position

	LiquidVertex *map = (LiquidVertex*) f.getPointer();
	unsigned char *flags = (unsigned char*) (f.getPointer() + (xtiles+1)*(ytiles+1)*sizeof(LiquidVertex));
	
	//waterFlags=new unsigned char[(xtiles+1)*(ytiles+1)];
	//memcpy(waterFlags,flags,(xtiles+1)*(ytiles+1));

	// generate vertices
	Vec3D * lVertices = new Vec3D[(xtiles+1)*(ytiles+1)];
	//color = new unsigned char[(xtiles+1)*(ytiles+1)];
	for (int j=0; j<ytiles+1; j++) {
		for (int i=0; i<xtiles+1; ++i) {
			size_t p = j*(xtiles+1)+i;
			float h = map[p].h;
			if (h > 100000) h = pos.y;
						lVertices[p] = Vec3D(pos.x + tilesize * i, h, pos.z + ydir * tilesize * j);
			//color[p]= map[p].c[0];
//! \todo	if map[p].c[1] != 0, overwrite the type from the flags.
//			gLog( "%i, {%i, %i, %i, %i}: %s\n", flags[p], map[p].c[0], map[p].c[1], map[p].c[2], map[p].c[3], gLiquidTypeDB.getByID( map[p].c[1] != 0 ? map[p].c[1] : pType ).getString( LiquidTypeDB::Name ) );
		}
	}

	mDrawList = new OpenGL::CallList();
	mDrawList->startRecording();

	//! \todo	handle light/dark liquid colors
	glNormal3f(0, 1, 0);
	glBegin(GL_QUADS);
	// draw tiles
	for (int j=0; j<ytiles; j++) {
		for (int i=0; i<xtiles; ++i) {
			unsigned char flag = flags[j*xtiles+i];
			if ((flag&8)==0) {
				tmpflag = flag;
				// 15 seems to be "don't draw"
				size_t p = j*(xtiles+1)+i;

				float c;
		
#ifdef USEBLSFILES
				c=type==2?(float)map[p].c[0]/255.0f:1.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f(i / texRepeats, j / texRepeats);
				glVertex3fv(lVertices[p]);
				
				c=type==2?(float)map[p+1].c[0]/255.0f:1.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, j / texRepeats);
				glVertex3fv(lVertices[p+1]);
				
				c=type==2?(float)map[p+xtiles+1+1].c[0]/255.0f:1.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p+xtiles+1+1]);
				
				c=type==2?(float)map[p+xtiles+1].c[0]/255.0f:1.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f(i / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p+xtiles+1]);
#else
				c=(float)map[p].c[0]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f(i / texRepeats, j / texRepeats);
				glVertex3fv(lVertices[p]);
				
				c=(float)map[p+1].c[0]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, j / texRepeats);
				glVertex3fv(lVertices[p+1]);
				
				c=(float)map[p+xtiles+1+1].c[0]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p+xtiles+1+1]);
				
				c=(float)map[p+xtiles+1].c[0]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f(i / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p+xtiles+1]);
#endif

			}
		}
	}
	glEnd();

	/*
	// debug triangles:
	//glColor4f(1,1,1,1);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glBegin(GL_TRIANGLES);
	for (int j=0; j<ytiles+1; j++) {
		for (int i=0; i<xtiles+1; ++i) {
			size_t p = j*(xtiles+1)+i;
			Vec3D v = verts[p];
			//short s = *( (short*) (f.getPointer() + p*8) );
			//float f = s / 255.0f;
			//glColor4f(f,(1.0f-f),0,1);
			unsigned char c[4];
			c[0] = 255-map[p].c[3];
			c[1] = 255-map[p].c[2];
			c[2] = 255-map[p].c[1];
			c[3] = map[p].c[0];
			glColor4ubv(c);

			glVertex3fv(v + Vec3D(-0.5f, 1.0f, 0));
			glVertex3fv(v + Vec3D(0.5f, 1.0f, 0));
			glVertex3fv(v + Vec3D(0.0f, 2.0f, 0));
		}
	}
	glEnd();
	glColor4f(1,1,1,1);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	*/
	

	/*
	// temp: draw outlines
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_LINE_LOOP);
	Vec3D wx = Vec3D(tilesize*xtiles,0,0);
	Vec3D wy = Vec3D(0,0,tilesize*ytiles*ydir);
	glColor4f(1,0,0,1);
	glVertex3fv(pos);
	glColor4f(1,1,1,1);
	glVertex3fv(pos+wx);
	glVertex3fv(pos+wx+wy);
	glVertex3fv(pos+wy);
	glEnd();
	glEnable(GL_TEXTURE_2D);*/

	mDrawList->endRecording();
	if(lVertices)
	{
		delete[] lVertices;
		lVertices = NULL;
	}
}

void Liquid::initFromMH2O( MH2O_Information *info, MH2O_HeightMask *HeightMap, MH2O_Render *render )
{
	texRepeats = 4.0f;
	ydir = 1.0f;

	try
	{
		DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID( info->LiquidType );
		//initTextures<1,30>( lLiquidTypeRow.getString( LiquidTypeDB::TextureFilenames - 1 ) );
				initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		mLiquidType = lLiquidTypeRow.getInt( LiquidTypeDB::Type );
		mShaderType = lLiquidTypeRow.getInt( LiquidTypeDB::ShaderType );
				mLiquidType = 0;
		mShaderType = 1;
		//! \todo	Get texRepeats too.
	}
	catch( ... )
	{
		// Fallback, when there is no information.
		//initTextures<1,30>( "XTEXTURES\\river\\lake_a.%d.blp" );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		mLiquidType = 0;
		mShaderType = 1;
	}

	mTransparency = mShaderType & 1;
	
	// generate vertices
	//! \todo	Store them somehow else. Maybe an extensible array[][] over the whole ADT?
	Vec3D *lVertices = new Vec3D[info->width * info->height];
	for( int j = 0; j < info->height; j++ ) 
		for( int i = 0; i < info->width; ++i ) 
			if( render->mRender[j * info->width + i] )
				lVertices[j * info->width + i] = Vec3D( pos.x + tilesize * i, HeightMap->mHeightValues[j][i], pos.z + ydir * tilesize * j );

	mDrawList = new OpenGL::CallList();
	mDrawList->startRecording();

	glBegin( GL_QUADS );
	
	glNormal3f( 0.0f, 1.0f, 0.0f );

	// draw tiles
	for( int j = 0; j < info->height; j++ ) 
		for( int i = 0; i < info->width; ++i ) 
			if( render->mRender[j * info->width + i] )
			{
				size_t p = j * info->width + i;
				float c;

				c = (float)HeightMap->mTransparency[j][i]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f(i / texRepeats, j / texRepeats);
				glVertex3fv(lVertices[p]);

				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, j / texRepeats);
				glVertex3fv(lVertices[p]+Vec3D(tilesize,0,0));
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p]+Vec3D(tilesize,0,tilesize));
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f(i / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p]+Vec3D(0,0,tilesize));
				
				/*c = (float)HeightMap->mTransparency[j][i+1]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, j / texRepeats);
				glVertex3fv(lVertices[p+1]);
				
				c = (float)HeightMap->mTransparency[j+1][i+1]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f((i+1) / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p+info->width+1]);
				
				c = (float)HeightMap->mTransparency[j+1][i]/255.0f;
				glMultiTexCoord2f(GL_TEXTURE1,c,c);
				glTexCoord2f(i / texRepeats, (j+1) / texRepeats);
				glVertex3fv(lVertices[p+info->width]);*/
			}

	glEnd();

	mDrawList->endRecording();
	if(lVertices)
	{
		delete[] lVertices;
		lVertices = NULL;
	}
}

void Liquid::initFromMH2O( MH2O_Tile pTileInformation )
{
	texRepeats = 4.0f;
	ydir = 1.0f;

	try
	{
		DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID( pTileInformation.mLiquidType );
		//initTextures<1,30>( lLiquidTypeRow.getString( LiquidTypeDB::TextureFilenames - 1 ) );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		
		mLiquidType = lLiquidTypeRow.getInt( LiquidTypeDB::Type );
		mShaderType = lLiquidTypeRow.getInt( LiquidTypeDB::ShaderType );
				mLiquidType = 0;
		mShaderType = 1;
		//! \todo	Get texRepeats too.
	}
	catch( ... )
	{
		// Fallback, when there is no information.
		//initTextures<1,30>( "XTEXTURES\\river\\lake_a.%d.blp" );
		initTextures<1,30>("XTextures\\river\\lake_a.%d.blp");
		mLiquidType = 0;
		mShaderType = 1;
	}

	mTransparency = mShaderType & 1;
	
	// generate vertices
	Vec3D lVertices[9][9];
	for( int j = 0; j < 9; ++j )
		for( int i = 0; i < 9; ++i ) 
			lVertices[j][i] = Vec3D( pos.x + tilesize * i, pTileInformation.mHeightmap[j][i], pos.z + ydir * tilesize * j );
	
	mDrawList = new OpenGL::CallList();
	mDrawList->startRecording();

	glBegin( GL_QUADS );
	
	glNormal3f( 0.0f, 1.0f, 0.0f );

	// draw tiles
	for( int j = 0; j < 8; ++j ) 
		for( int i = 0; i < 8; ++i ) 
			if( pTileInformation.mRender[j][i] )
			{
				float c;
				c = pTileInformation.mDepth[j][i];// / 255.0f;
				glMultiTexCoord2f( GL_TEXTURE1, c, c );
				glTexCoord2f( i / texRepeats, j / texRepeats);
				glVertex3fv( lVertices[j][i] );

				c = pTileInformation.mDepth[j][i + 1];// / 255.0f;
				glMultiTexCoord2f( GL_TEXTURE1, c, c );
				glTexCoord2f( ( i + 1 ) / texRepeats, j / texRepeats);
				glVertex3fv( lVertices[j][i + 1] );
				
				c = pTileInformation.mDepth[j + 1][i + 1];// / 255.0f;
				glMultiTexCoord2f( GL_TEXTURE1, c, c );
				glTexCoord2f( ( i + 1 ) / texRepeats, ( j + 1 ) / texRepeats);
				glVertex3fv( lVertices[j + 1][i + 1] );
				
				c = pTileInformation.mDepth[j + 1][i];// / 255.0f;
				glMultiTexCoord2f( GL_TEXTURE1, c, c );
				glTexCoord2f( i / texRepeats, ( j + 1 ) / texRepeats);
				glVertex3fv( lVertices[j + 1][i] );
			}

	glEnd();

	mDrawList->endRecording();
}

#ifdef USEBLSFILES
	BLSShader * mWaterShader;
	BLSShader * mMagmaShader;
#else
	GLuint	waterShader;
	GLuint	waterFogShader;
#endif

void loadWaterShader()
{
#ifdef USEBLSFILES
	mWaterShader = new BLSShader( "shaders\\pixel\\arbfp1\\psLiquidWater.bls" );
	mMagmaShader = new BLSShader( "shaders\\pixel\\arbfp1\\psLiquidMagma.bls" );
#else
	FILE *shader = fopen( "shaders\\water.ps", "r" );
	if( !shader )
	{
		LogError << "Unable to open water shader \"shaders\\water.ps\"." << std::endl;
	}
	else
	{
		char buffer[8192];
		int length=fread(buffer, 1, 8192, shader);
		fclose(shader);
		glGenProgramsARB(1, &waterShader);
		if(waterShader==0)
			LogError << "Failed to get program ID for water shader \"shaders\\water.ps\"." << std::endl;
		else
		{
			GLint errorPos, isNative;

			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterShader);
			glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

			glGetProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
			if( !(errorPos==-1)&&(isNative==1) )
			{
				int i, j;
				const GLubyte *stringy;
				char localbuffer[256];
				LogError << "Water Shader \"shaders\\water.ps\" Fragment program failed to load \nReason:\n";
				stringy=glGetString(GL_PROGRAM_ERROR_STRING_ARB);	//This is only available in ARB
				LogError << (char *)stringy << std::endl;
				for(i=errorPos, j=0; (i<length)&&(j<128); ++i, j++)
				{
					localbuffer[j]=buffer[i];
				}
				localbuffer[j]=0;
				LogError << "START DUMP :" << std::endl << localbuffer << "END DUMP" << std::endl;
				if(isNative==0)
					LogError << "This fragment program exceeded the limit." << std::endl;
			}
		}
	}

	shader=fopen("shaders\\waterfog.ps", "r");
	if(shader==0)
		LogError << "Unable to open water shader \"shaders/waterfog.ps\"." << std::endl;
	else
	{
		char buffer[8192];
		int length=fread(buffer, 1, 8192, shader);
		fclose(shader);
		glGenProgramsARB(1, &waterFogShader);
		if(waterFogShader==0)
			LogError << "Failed to get program ID for water shader \"shaders/waterfog.ps\"." << std::endl;
		else
		{
			GLint errorPos, isNative;

			glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
			glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer);
			glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);

			glGetProgramiv(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative);
			if( !(errorPos==-1)&&(isNative==1) )
			{
				int i, j;
				const GLubyte *stringy;
				char localbuffer[256];
				LogError << "Water Shader \"shaders/waterfog.ps\" Fragment program failed to load \nReason:\n";
				stringy=glGetString(GL_PROGRAM_ERROR_STRING_ARB);	//This is only available in ARB
				LogError << (char *)stringy << std::endl;
				for(i=errorPos, j=0; (i<length)&&(j<128); ++i, j++)
				{
					localbuffer[j]=buffer[i];
				}
				localbuffer[j]=0;
				LogError << "START DUMP :" << std::endl << localbuffer << "END DUMP" << std::endl;
				if(isNative==0)
					LogError << "This fragment program exceeded the limit." << std::endl;
			}
		}
	}
#endif
}

#ifndef USEBLSFILES
void enableWaterShader()
{
	if(glIsEnabled(GL_FOG)==GL_TRUE)
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterFogShader);
	else
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, waterShader);
}
#endif

void Liquid::draw()
{
	CheckForGLError( "before Liquid:draw" );

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

#ifdef USEBLSFILES
	if( type == 2 && mWaterShader->IsOkay() )
		mWaterShader->EnableShader();
	if( type == 0 && mMagmaShader->IsOkay() )
		mMagmaShader->EnableShader();
#else
	enableWaterShader();
#endif

	Vec3D col2;
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	size_t texidx = (size_t)(gWorld->animtime / 60.0f) % textures.size();
	


	//glActiveTexture(GL_TEXTURE0);
	//glDisable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, textures[texidx]);
	
	const float tcol = mTransparency ? 0.75f : 1.0f;
	
	if( mTransparency ) 
	{
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
	}

	if (type==0) 
		glColor4f(0,0,0,0.8);
	else 
	{
		if (type==2) 
		{
			// dynamic color lookup! ^_^
			col = gWorld->skies->colorSet[WATER_COLOR_LIGHT]; //! \todo	add variable water color
			col2 = gWorld->skies->colorSet[WATER_COLOR_DARK];
		}
		glColor4f(col.x, col.y, col.z, tcol);
		glProgramLocalParameter4fARB(GL_FRAGMENT_PROGRAM_ARB,0,col2.x,col2.y,col2.z,tcol);
#ifdef USEBLSFILES
		glSecondaryColor3f(col2.x,col2.y,col2.z);
#endif
		//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); //! \todo	check if ARB_texture_env_add is supported? :(
	}

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[texidx]);
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);


	if( mDrawList )
	{
		mDrawList->render();
	}


	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	
	glColor4f(1,1,1,0.4f);
	if( mTransparency ) 
	{
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	

	CheckForGLError( "after Liquid:draw" );
}

template<int pFirst, int pLast>
void Liquid::initTextures( const std::string& pFilename )
{
	char buf[1024];
	for( int i = pFirst; i <= pLast; ++i ) 
	{
		sprintf( buf, pFilename.c_str(), i );
		textures.push_back( TextureManager::add( buf )) ;
	}
}


Liquid::~Liquid()
{
	if( mDrawList )
	{
		delete mDrawList;
		mDrawList = NULL;
	}
	for( size_t i=0; i<textures.size(); ++i ) 
	{
		TextureManager::del( textures[i] );
	}
}
