#include "video.h"
#include "Log.h"

//0 - GL_ALPHA_TEST
//1 - GL_BLEND
//2 - GL_COLOR_MATERIAL
//3 - GL_CULL_FACE
//4 - GL_DEPTH_TEST
//5 - GL_FOG
//6 - GL_FRAGMENT_PROGRAM_ARB
//7 - GL_LIGHTING
//8 - GL_LINE_SMOOTH
//9/10 - GL_TEXTURE_2D
//11 - GL_TEXTURE_GEN_S
//12 - GL_TEXTURE_GEN_T
GLboolean	GLSettings[13];
void SaveGLSettings()
{
	GLSettings[0]=glIsEnabled(GL_ALPHA_TEST);
	GLSettings[1]=glIsEnabled(GL_BLEND);
	GLSettings[2]=glIsEnabled(GL_COLOR_MATERIAL);
	GLSettings[3]=glIsEnabled(GL_CULL_FACE);
	GLSettings[4]=glIsEnabled(GL_DEPTH_TEST);
	GLSettings[5]=glIsEnabled(GL_FOG);
	GLSettings[6]=glIsEnabled(GL_FRAGMENT_PROGRAM_ARB);
	GLSettings[7]=glIsEnabled(GL_LIGHTING);
	GLSettings[8]=glIsEnabled(GL_LINE_SMOOTH);
	glActiveTexture(GL_TEXTURE1);
	GLSettings[10]=glIsEnabled(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	GLSettings[9]=glIsEnabled(GL_TEXTURE_2D);
	GLSettings[11]=glIsEnabled(GL_TEXTURE_GEN_S);
	GLSettings[12]=glIsEnabled(GL_TEXTURE_GEN_T);
}

void compareSetting(GLenum Setting, GLboolean Cur)
{
	if(glIsEnabled(Setting)!=Cur)
	{
		if(Cur==GL_TRUE)
			glEnable(Setting);
		else
			glDisable(Setting);
	}
}

void LoadGLSettings()
{
	compareSetting(GL_ALPHA_TEST,GLSettings[0]);
	compareSetting(GL_BLEND,GLSettings[1]);
	compareSetting(GL_COLOR_MATERIAL,GLSettings[2]);
	compareSetting(GL_CULL_FACE,GLSettings[3]);
	compareSetting(GL_DEPTH_TEST,GLSettings[4]);
	compareSetting(GL_FOG,GLSettings[5]);
	compareSetting(GL_FRAGMENT_PROGRAM_ARB,GLSettings[6]);
	compareSetting(GL_LIGHTING,GLSettings[7]);
	compareSetting(GL_LINE_SMOOTH,GLSettings[8]);
	glActiveTexture(GL_TEXTURE1);
	compareSetting(GL_TEXTURE_2D,GLSettings[10]);
	glActiveTexture(GL_TEXTURE0);
	compareSetting(GL_TEXTURE_2D,GLSettings[9]);
	compareSetting(GL_TEXTURE_GEN_S,GLSettings[11]);
	compareSetting(GL_TEXTURE_GEN_T,GLSettings[12]);
}




Video video;

void Video::resize(int _xres, int _yres)
{
	//! \todo  Implement some minimum size.
	//if(xres < 800.0 || yres<600.0f)
	//{
	//	if(xres < 800.0f) xres = 800.0f;
	//	if(yres < 600.0f) yres = 600.0f;
	//	init(xres, yres, false);
	//	primary = SDL_SetVideoMode(xres, yres, 32, flags);
	//}

	this->xres = _xres;
	this->yres = _yres;
	this->ratio = float( xres ) / float( yres );
	
	// I shouldn't be changing these according to the documentation but it works
	primary->w = xres;
	primary->h = yres;

	SDL_Rect Rect;

	Rect.h = yres;
	Rect.w = xres;
	Rect.x = 0;
	Rect.y = 0;
	SDL_SetClipRect( primary, &Rect );

	glViewport( 0.0f, 0.0f, xres, yres );

	//! \todo  Should we really set to 3D here?
	this->set3D( );
}

bool Video::init( int _xres, int _yres, bool fullscreen_ )
{
	SDL_Init( SDL_INIT_TIMER | SDL_INIT_VIDEO );
	flags = SDL_OPENGL | SDL_HWSURFACE | SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_RESIZABLE;
	if( fullscreen_ )
		flags |= SDL_FULLSCREEN;

	fullscreen = fullscreen_;

	// 32 bits ffs
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
#ifdef _WIN32
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 32 );
	primary = SDL_SetVideoMode( _xres, _yres, 32, flags );
#else
	//nvidia dont support 32bpp on my linux :(
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	primary = SDL_SetVideoMode(_xres, _yres, 24, flags);
#endif
	if( !primary ) 
	{
		LogError << "SDL: " << SDL_GetError( ) << std::endl;
		exit( 1 );
	}

	origX = _xres;
	origY = _yres;

	this->xres = _xres;
	this->yres = _yres;
	this->ratio = float( xres ) / float( yres );

	glViewport( 0.0f, 0.0f, xres, yres );

	//! \todo  Should we really set to 3D here?
	this->set3D( );

	// hmmm...
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	GLenum err = glewInit( );
	if( GLEW_OK != err )
	{
		LogError << "GLEW: " << glewGetErrorString( err ) << std::endl;
		return false;
	}

	mSupportCompression = GLEW_ARB_texture_compression;
	mSupportShaders = GLEW_ARB_vertex_program && GLEW_ARB_fragment_program;

	LogDebug << "GL: Version: " << glGetString( GL_VERSION ) << std::endl;
	LogDebug << "GL: Vendor: " << glGetString( GL_VENDOR ) << std::endl;
	LogDebug << "GL: Renderer: " << glGetString( GL_RENDERER ) << std::endl;
	
	return mSupportCompression;
}

void Video::close( )
{
	//Crashes if I don't do this, so prolly why they say don't change those
	primary->w = origX;
	primary->h = origY;
	SDL_Quit( );
}

void Video::flip( )
{
	SDL_GL_SwapBuffers( );
}

void Video::clearScreen( )
{
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Video::set3D( )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 45.0f, this->ratio, 1.0f, 1024.0f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
}

void Video::set3D_select()
{
	glMatrixMode( GL_PROJECTION );
	gluPerspective( 45.0f, this->ratio, 1.0f, 1024.0f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
}

void Video::set2D()
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	glOrtho( 0.0f, xres, yres, 0.0f, -1.0f, 1.0f );
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Video::setTileMode()
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	glOrtho( -2.0f * this->ratio, 2.0f * this->ratio, 2.0f, -2.0f, -100.0f, 300.0f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
}

void CheckForGLError( const std::string& pLocation )
{
	int ErrorNum = glGetError( );
	while( ErrorNum )
	{
		switch( ErrorNum )
		{
			case GL_INVALID_ENUM:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_INVALID_ENUM" << std::endl;
		 		break;
			case GL_INVALID_VALUE:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_INVALID_VALUE" << std::endl;
		 		break;
		 	case GL_INVALID_OPERATION:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_INVALID_OPERATION" << std::endl;
		 		break;
		 	case GL_STACK_OVERFLOW:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_STACK_OVERFLOW" << std::endl;
		 		break;
		 	case GL_STACK_UNDERFLOW:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_STACK_UNDERFLOW" << std::endl;
		 		break;
		 	case GL_OUT_OF_MEMORY:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_OUT_OF_MEMORY" << std::endl;
		 		break;
		 	case GL_TABLE_TOO_LARGE:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_TABLE_TOO_LARGE" << std::endl;
		 		break;
		 	case GL_NO_ERROR:
			//! \todo  Add the missing ones.
		 	default:
		 		LogError << "OpenGL: (at " << pLocation << "): GL_NO_ERROR (wat?)" << std::endl;
 		}

		ErrorNum = glGetError( );
	}
}

/*


#pragma pack(push,1)
struct TGAHeader {
   char  idlength;
   char  colourmaptype;
   char  datatypecode;
   short int colourmaporigin;
   short int colourmaplength;
   char  colourmapdepth;
   short int x_origin;
   short int y_origin;
   short width;
   short height;
   char  bitsperpixel;
   char  imagedescriptor;
};
#pragma pack(pop)

GLuint loadTGA(const char *filename, bool mipmaps)
{
	FILE *f = fopen(filename,"rb");
	if(!f)
		return GLuint(0);

	TGAHeader h;
	fread(&h,18,1,f);
	if (h.datatypecode != 2) return 0;////////////////////////////////////
	size_t s = h.width * h.height;
	GLint bppformat;
	GLint format;
	int bypp = h.bitsperpixel / 8;
	if (h.bitsperpixel == 24) {
		s *= 3;
		format = GL_RGB;
		bppformat = GL_RGB8;
	} else if (h.bitsperpixel == 32) {
		s *= 4;
		format = GL_RGBA;
		bppformat = GL_RGBA8;
	} else return 0;

	unsigned char *buf = new unsigned char[s], *buf2;
	//unsigned char *buf2 = new unsigned char[s];
	fread(buf,s,1,f);
	fclose(f);

	buf2 = buf;

	GLuint t;
	glGenTextures(1,&t);
	glBindTexture(GL_TEXTURE_2D, t);

	if (mipmaps) {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps (GL_TEXTURE_2D, bppformat, h.width, h.height, format, GL_UNSIGNED_BYTE, buf2);
	} else {
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, bppformat, h.width, h.height, 0, format, GL_UNSIGNED_BYTE, buf2);
	}
    delete[] buf;
	//delete[] buf2;
	return t;
}*/
