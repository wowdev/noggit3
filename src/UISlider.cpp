#ifdef _WIN32
#define NOMINMAX
#endif // win32

#include "UISlider.h"

#include <iomanip>
#include <sstream>

#include "FreeType.h" // freetype::
#include "Noggit.h" // app.getArial12()
#include "TextureManager.h" // TextureManager, Texture
#include "Video.h"

UISlider::UISlider( float xPos, float yPos, float w, float s, float o )
: UIFrame( xPos, yPos, w, 10.0f )
, texture( TextureManager::newTexture( "Interface\\Buttons\\UI-SliderBar-Border.blp" ) )
, sliderTexture( TextureManager::newTexture( "Interface\\Buttons\\UI-SliderBar-Button-Horizontal.blp" ) )
, scale( s )
, offset( o )
, func( NULL )
, text( "" )
, value( 0.5f )
{
  clickable( true );
}

UISlider::~UISlider()
{
  TextureManager::delbyname( "Interface\\Buttons\\UI-SliderBar-Border.blp" );
  TextureManager::delbyname( "Interface\\Buttons\\UI-SliderBar-Button-Horizontal.blp" );
}

void UISlider::setFunc( void( *f )( float val ) )
{
  func = f;
}

void UISlider::setValue( float f )
{
  value = std::min( 1.0f, std::max( 0.0f, f ) );
  if( func )
    func( value * scale + offset );
}

void UISlider::setText( const std::string& t )
{
  text = t;
}

UIFrame *UISlider::processLeftClick( float mx, float /*my*/ )
{
  /*if((mx>(width*value-16))&&(mx<(width*value+16)))
        return this;
  return 0;*/

  value = std::min( 1.0f, std::max( 0.0f, mx / width() ) );
  if( func )
    func( value * scale + offset );

  return this;
}

bool UISlider::processLeftDrag( float mx, float /*my*/, float /*xChange*/, float /*yChange*/ )
{
  //! \todo use change?
  float tx,ty;
  parent()->getOffset( &tx, &ty );
  mx -= tx;
  //my -= ty;

  value = std::min( 1.0f, std::max( 0.0f, mx / width() ) );
  if( func )
    func( value * scale + offset );

  return true;
}

void UISlider::render() const
{
  if( hidden() )
    return;

  glPushMatrix();
  glTranslatef( x(), y(), 0.0f );

  glColor3f( 1.0f, 1.0f, 1.0f );

  std::stringstream temp;
  temp << text << std::fixed << std::setprecision( 2 ) << ( value * scale + offset );
  const std::string tempStr = temp.str();
  app.getArial12().shprint( width() / 2.0f - app.getArial12().width( tempStr ) / 2.0f, -16.0f, tempStr );

  glPushMatrix();

  OpenGL::Texture::setActiveTexture();
  OpenGL::Texture::enableTexture();

  texture->bind();

  const float height_plus = height() + 4.0f;
  const float height_minus = height() - 4.0f;
  const float width_plus = width() + 1.0f;
  const float width_minus = width() - 7.0f;

  //Draw Bottom left Corner First
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.75f, 1.0f );
  glVertex2f( -1.0f, height_plus );
  glTexCoord2f( 0.875f, 1.0f );
  glVertex2f( 7.0f, height_plus );
  glTexCoord2f( 0.75f, 0.0f );
  glVertex2f( -1.0f, height_minus );
  glTexCoord2f( 0.875f, 0.0f );
  glVertex2f( 7.0f, height_minus );
  glEnd();

  //Draw Bottom Right Corner
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.875f, 1.0f );
  glVertex2f( width_minus, height_plus );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( width_plus, height_plus );
  glTexCoord2f( 0.875f, 0.0f );
  glVertex2f( width_minus, height_minus );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( width_plus, height_minus );
  glEnd();

  //Draw Top Left Corner
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.5f, 1.0f );
  glVertex2f( -1.0f, 4.0f );
  glTexCoord2f( 0.625f, 1.0f );
  glVertex2f( 7.0f, 4.0f );
  glTexCoord2f( 0.5f, 0.0f );
  glVertex2f( -1.0f, -4.0f );
  glTexCoord2f( 0.625f, 0.0f );
  glVertex2f( 7.0f, -4.0f );
  glEnd();

  //Draw Top Right Corner
  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.625f, 1.0f );
  glVertex2f( width_minus, 4.0f );
  glTexCoord2f( 0.75f, 1.0f );
  glVertex2f( width_plus, 4.0f );
  glTexCoord2f( 0.625f, 0.0f );
  glVertex2f( width_minus, -4.0f );
  glTexCoord2f( 0.75f, 0.0f );
  glVertex2f( width_plus, -4.0f );
  glEnd();

  if( height() > 8.0f )
  {
    //Draw Left Side
    glBegin( GL_TRIANGLE_STRIP );
    glTexCoord2f( 0.0f, 1.0f );
    glVertex2f( -1.0f, height_minus );
    glTexCoord2f( 0.125f, 1.0f );
    glVertex2f( 7.0f, height_minus );
    glTexCoord2f( 0.0f, 0.0f );
    glVertex2f( -1.0f, 4.0f );
    glTexCoord2f( 0.125f, 0.0f );
    glVertex2f( 7.0f, 4.0f );
    glEnd();

    //Draw Right Side
    glBegin( GL_TRIANGLE_STRIP );
    glTexCoord2f( 0.125f, 1.0f );
    glVertex2f( width_minus, height_minus );
    glTexCoord2f( 0.25f, 1.0f );
    glVertex2f( width_plus, height_minus );
    glTexCoord2f( 0.125f, 0.0f );
    glVertex2f( width_minus, 4.0f );
    glTexCoord2f( 0.25f, 0.0f );
    glVertex2f( width_plus, 4.0f );
    glEnd();
  }

  if( width() > 14.0f )
  {
    //Draw Top Side
    glBegin( GL_TRIANGLE_STRIP );
    glTexCoord2f( 0.5f, 1.0f );
    glVertex2f( 7.0f, height_plus );
    glTexCoord2f( 0.5f, 0.0f );
    glVertex2f( width_minus, height_plus );
    glTexCoord2f( 0.375f, 1.0f );
    glVertex2f( 7.0f, height_minus );
    glTexCoord2f( 0.375f, 0.0f );
    glVertex2f( width_minus, height_minus );
    glEnd();

    //Draw Bottom Side
    glBegin( GL_TRIANGLE_STRIP );
    glTexCoord2f( 0.375f, 1.0f );
    glVertex2f( 7.0f, 4.0f );
    glTexCoord2f( 0.375f, 0.0f );
    glVertex2f( width_minus, 4.0f );
    glTexCoord2f( 0.25f, 1.0f );
    glVertex2f( 7.0f, -4.0f );
    glTexCoord2f( 0.25f, 0.0f );
    glVertex2f( width_minus, -4.0f );
    glEnd();
  }

  glPopMatrix();

  sliderTexture->bind();

  const float sliderpos_x = width() * value;
  const float sliderpos_y = height() / 2.0f;

  glBegin( GL_TRIANGLE_STRIP );
  glTexCoord2f( 0.0f, 0.0f );
  glVertex2f( sliderpos_x - 16.0f, sliderpos_y - 16.0f );
  glTexCoord2f( 1.0f, 0.0f );
  glVertex2f( sliderpos_x + 16.0f, sliderpos_y - 16.0f );
  glTexCoord2f( 0.0f, 1.0f );
  glVertex2f( sliderpos_x - 16.0f, sliderpos_y + 16.0f );
  glTexCoord2f( 1.0f, 1.0f );
  glVertex2f( sliderpos_x + 16.0f, sliderpos_y + 16.0f );
  glEnd();

  OpenGL::Texture::disableTexture();


  glPopMatrix();
}
