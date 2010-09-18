#include "shaders.h"
#include "Log.h"

BLSShader::BLSShader( const std::string& pFilename )
{
	mOkay = false;
	if( !MPQFile::exists( pFilename ) )
	{
		LogError << "Failed to get file for shader \"" << pFilename << "\"." << std::endl;
		return;
	}

	int magix;
	int length;
	char * buffer;

	MPQFile lShader( pFilename );
	lShader.read( &magix, sizeof( int ) );
	lShader.seek( 0x18 );
	lShader.read( &length, sizeof( int ) );
	length--;
	buffer = new char[length];
	lShader.read( buffer, length );	

	for( ; buffer[length] <= 0; length-- );

	lShader.seek( 0x1C );
	buffer = new char[length];
	lShader.read( buffer, length );	

	mProgramType = magix == 'GXPS' ? GL_FRAGMENT_PROGRAM_ARB : ( 'GXVS' ? GL_VERTEX_PROGRAM_ARB : -1 );

	glGenProgramsARB( 1, &mShader );
	if( !mShader )
	{
		LogError << "Failed to get program ID for shader \"" << pFilename << "\"." << std::endl;
	}
	else
	{
		GLint errorPos, isNative;

		glBindProgramARB( mProgramType, mShader );
		glProgramStringARB( mProgramType, GL_PROGRAM_FORMAT_ASCII_ARB, length, buffer );
		glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &errorPos );

		glGetProgramiv( mProgramType, GL_PROGRAM_UNDER_NATIVE_LIMITS_ARB, &isNative );
		if( !( errorPos == -1 && isNative == 1 ) )
		{
			LogError << "Shader program \"" << pFilename << "\" failed to load. Reason:" << std::endl;
			if( isNative == 0 )
				LogError << "\t\"This fragment program exceeded the limit.\"" << std::endl;
			
			const GLubyte *stringy;
			stringy = glGetString(GL_PROGRAM_ERROR_STRING_ARB);			//This is only available in ARB
			if( stringy )
				LogError << "\t\"" << reinterpret_cast<const char*>( stringy ) << "\"" << std::endl;
				
			int j = 0;
			char localbuffer[256];
			for( int i = errorPos, j = 0; i < length && j < 128 ; i++, j++ )
			{
				localbuffer[j] = buffer[i];
			}
			localbuffer[j] = 0;
			if( localbuffer[0] )
				LogError << "\tDump:\"" << localbuffer<< "\"" << std::endl;
		}
		else
			mOkay = true;
	}
}


ShaderPair *terrainShaders[4]={0,0,0,0}, *wmoShader=0, *waterShaders[1]={0};

void initShaders( )
{
	if( video.mSupportShaders )
		reloadShaders();
	
	LogDebug << "Shaders are " << ( video.mSupportShaders ? "enabled." : "disabled." ) << std::endl;
} 

void reloadShaders()
{
	for (int i=0; i<4; i++) delete terrainShaders[i];
	delete wmoShader;
	delete waterShaders[0];

	terrainShaders[0] = new ShaderPair(0, "shaders/terrain1.fs", true);
	terrainShaders[1] = new ShaderPair(0, "shaders/terrain2.fs", true);
	terrainShaders[2] = new ShaderPair(0, "shaders/terrain3.fs", true);
	terrainShaders[3] = new ShaderPair(0, "shaders/terrain4.fs", true);
	wmoShader = new ShaderPair(0, "shaders/wmospecular.fs", true);
	waterShaders[0] = new ShaderPair(0, "shaders/wateroutdoor.fs", true);
}

Shader::Shader(GLenum target, const char *program, bool fromFile):id(0),target(target)
{
	if (!program || !strlen(program)) {
		ok = true;
		return;
	}

	const char *progtext;
	if (fromFile) {
    char *buf;
		FILE *f = fopen(program, "rb");
		if (!f) {
			ok = false;
			return;
		}
		fseek(f, 0, SEEK_END);
		size_t len = ftell(f);
		fseek(f, 0, SEEK_SET);

		buf = new char[len+1];
		progtext = buf;
		fread(buf, len, 1, f);
		buf[len]=0;
		fclose(f);
		//gLog("Len: %d\nShader text:\n[%s]\n",len,progtext);
    delete[] buf;
	} else progtext = program;

	glGenProgramsARB(1, &id);
	glBindProgramARB(target, id);
	glProgramStringARB(target, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei)strlen(progtext), progtext);
	if (glGetError() != 0) {
		int errpos;
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errpos);
		LogError << "Error loading shader: " << glGetString(GL_PROGRAM_ERROR_STRING_ARB) << std::endl;
		LogError << "Error position: " << errpos << std::endl;
		ok = false;
	} else ok = true;

}

Shader::~Shader()
{
	if (ok && id) glDeleteProgramsARB(1, &id);
}

void Shader::bind()
{
	glBindProgramARB(target, id);
	glEnable(target);
}

void Shader::unbind()
{
	glDisable(target);
}

ShaderPair::ShaderPair(const char *vprog, const char *fprog, bool fromFile)
{
	if (vprog && strlen(vprog)) {
		vertex = new Shader(GL_VERTEX_PROGRAM_ARB, vprog, fromFile);
		if (!vertex->ok) {
			delete vertex;
			vertex = 0;
		}
	} else vertex = 0;
	if (fprog && strlen(fprog)) {
		fragment = new Shader(GL_FRAGMENT_PROGRAM_ARB, fprog, fromFile);
		if (!fragment->ok) {
			delete fragment;
			fragment = 0;
		}
	} else fragment = 0;
}

void ShaderPair::bind()
{
	/*if (vertex) 
	{
		vertex->bind();
	} else {
		glDisable(GL_VERTEX_PROGRAM_ARB);
	}
	if (fragment) {
		fragment->bind();
	} else {
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}*/
}

void ShaderPair::unbind()
{
	/*
	if (vertex) vertex->unbind();
	if (fragment) fragment->unbind();
	*/
}
