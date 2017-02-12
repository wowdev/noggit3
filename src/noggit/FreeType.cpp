// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/FreeType.h>

#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_TRIGONOMETRY_H

#include <algorithm>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include <utf8.h>

#include <opengl/scoped.hpp>
#include <noggit/Settings.h>
#include <noggit/MPQ.h>
#include <noggit/Log.h>
#include <noggit/Native.hpp>
#include <noggit/Video.h>

namespace freetype
{
  inline int next_p2(int a)
  {
    int rval(1);
    while (rval < a)
    {
      rval <<= 1;
    }
    return rval;
  }

  void font_data::createGlyph(CharacterCode charCode) const
  {
    if (FT_Load_Glyph(_face, FT_Get_Char_Index(_face, charCode), FT_LOAD_DEFAULT))
    {
      LogError << "FT_Load_Glyph failed" << std::endl;
      throw std::runtime_error("FT_Load_Glyph failed");
    }

    FT_Glyph glyph;
    if (FT_Get_Glyph(_face->glyph, &glyph))
    {
      LogError << "FT_Get_Glyph failed" << std::endl;
      throw std::runtime_error("FT_Get_Glyph failed");
    }

    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    FT_BitmapGlyph bitmap_glyph(reinterpret_cast<FT_BitmapGlyph>(glyph));

    const FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    const int width(next_p2(bitmap.width + 1));
    const int height(next_p2(bitmap.rows + 1));

    const size_t expanded_size(width * height * 2);
    GLubyte* expanded_data = new GLubyte[expanded_size];
    memset(expanded_data, 0, expanded_size);

    for (int j(0); j < height; ++j)
    {
      for (int i(0); i < width; ++i)
      {
        expanded_data[2 * (i + j * width)] = expanded_data[2 * (i + j * width) + 1] =
          (i >= bitmap.width || j >= bitmap.rows) ?
          0 : static_cast<GLubyte>(std::min(static_cast<float>(bitmap.buffer[i + bitmap.width * j]) * 1.5f, 255.0f));
      }
    }

    GlyphData glyphData;
    glyphData._width = _face->glyph->advance.x >> 6;

    glyphData._texture = new opengl::texture();
    glyphData._texture->bind();

    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    gl.texImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

    delete[] expanded_data;
    expanded_data = nullptr;

    glyphData._callList = new opengl::call_list();
    glyphData._callList->start_recording();

    glyphData._texture->bind();

    const float xl((const float)bitmap_glyph->left);
    const float xh(xl + width);
    const float yl((const float)h - bitmap_glyph->top);
    const float yh(yl + height);

    gl.begin(GL_TRIANGLE_STRIP);
    gl.texCoord2f(0.0f, 0.0f);
    gl.vertex2f(xl, yl);
    gl.texCoord2f(0.0f, 1.0f);
    gl.vertex2f(xl, yh);
    gl.texCoord2f(1.0f, 0.0f);
    gl.vertex2f(xh, yl);
    gl.texCoord2f(1.0f, 1.0f);
    gl.vertex2f(xh, yh);
    gl.end();

    gl.translatef(static_cast<float>(glyphData._width), 0.0f, 0.0f);

    glyphData._callList->end_recording();

    _cachedGlyphs[charCode] = glyphData;
  }

  void font_data::init(const std::string& fname, unsigned int _h, bool fromMPQ)
  {
    h = _h;

    if (FT_Init_FreeType(&_library))
    {
      LogError << "FT_Init_FreeType failed" << std::endl;
      throw std::runtime_error("FT_Init_FreeType failed");
    }

    bool failed;
    if (fromMPQ)
    {
      _mpqFile = new MPQFile(fname);
      failed = FT_New_Memory_Face(_library, _mpqFile->get<FT_Byte>(0), _mpqFile->getSize(), 0, &_face) != 0;
    }
    else
    {
      failed = FT_New_Face(_library, fname.c_str(), 0, &_face) != 0;
    }

    if (failed)
    {
		std::string message = "Noggit encountered an error loading fonts required for its UI. ";
		if (fromMPQ) {
			message += "Please ensure that the WoW installation at the path below is valid:\n\n";
			message += Settings::getInstance()->gamePath;
		} else {
			message += "Please ensure that your system has a valid copy of Arial installed.";
		}

		message += "\n\nNoggit will now quit.";

		Native::showAlertDialog("Unable to load fonts", message);

		LogError << "FT_New_Face failed (there is probably a problem with your font file)" << std::endl;
		throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
    }

    // For some twisted reason, Freetype measures font size in terms of 1/64ths of pixels.
    FT_Set_Char_Size(_face, h << 6, h << 6, 72, 72);
  }

  font_data::~font_data()
  {
    FT_Done_Face(_face);
    FT_Done_FreeType(_library);

    delete _mpqFile;

    typedef std::map<CharacterCode, GlyphData> GlyphListType;

    for (GlyphListType::iterator it(_cachedGlyphs.begin()), end(_cachedGlyphs.end()); it != end; ++it)
    {
      delete it->second._callList;
      delete it->second._texture;
    }
    _cachedGlyphs.clear();
  }

  const GlyphData& font_data::getGlyphData(CharacterCode charCode) const
  {
    if (!_cachedGlyphs.count(charCode))
    {
      createGlyph(charCode);
    }
    return _cachedGlyphs[charCode];
  }

  void font_data::print(float x, float y, const std::string& text, float colorR, float colorG, float colorB) const
  {
    float height(h / 0.90f);

    typedef std::vector<std::string> linesType;
    linesType lines;

    boost::split(lines, text, boost::is_any_of("\n\r"));

    gl.color3f(colorR, colorG, colorB);

    gl.pushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    gl.disable(GL_LIGHTING);
    opengl::texture::enable_texture();
    gl.disable(GL_DEPTH_TEST);
    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int verticalPosition((int)y);

    for (linesType::const_iterator line(lines.begin()), end(lines.end()); line != end; ++line)
    {
      opengl::scoped::matrix_pusher matrix;
      gl.translatef(x, (float)verticalPosition, 0.0f);

      const char* lineBegin = line->c_str();
      const char* lineEnd = lineBegin + line->length();

      try
      {
        for (utf8::iterator<const char*> itr(lineBegin, lineBegin, lineEnd), itrEnd(lineEnd, lineBegin, lineEnd); itr != itrEnd; ++itr)
        {
          getGlyphData(*itr).render();
        }
      }
      catch (const utf8::invalid_utf8& e)
      {
        LogError << "Invalid UTF8 in string \"" << *line << "(" << &e << ")\"" << std::endl;
      }

      verticalPosition += (int)height;
    }

    gl.popAttrib();
  }

  void font_data::shprint(float x, float y, const std::string& text, float colorR, float colorG, float colorB) const
  {
    print(x + 2.0f, y + 2.0f, text, 0.0f, 0.0f, 0.0f);
    print(x, y, text, colorR, colorG, colorB);
  }

  int font_data::width(const std::string& text) const
  {
    typedef std::vector<std::string> linesType;
    linesType lines;

    boost::split(lines, text, boost::is_any_of("\n\r"));

    int maximumWidth(0);

    for (linesType::const_iterator line(lines.begin()), end(lines.end()); line != end; ++line)
    {
      int currentWidth(0);

      const char* lineBegin = line->c_str();
      const char* lineEnd = lineBegin + line->length();

      try
      {
        for (utf8::iterator<const char*> itr(lineBegin, lineBegin, lineEnd), itrEnd(lineEnd, lineBegin, lineEnd); itr != itrEnd; ++itr)
        {
          currentWidth += getGlyphData(*itr).width();
        }
      }
      catch (const utf8::invalid_utf8& e)
      {
        LogError << "Invalid UTF8 in string \"" << *line << "(" << &e << ")\"" << std::endl;
      }

      maximumWidth = std::max(maximumWidth, currentWidth);
    }

    return maximumWidth;
  }
}
