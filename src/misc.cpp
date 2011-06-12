#include <string>
#include "misc.h"
#include <vector>
#include <map>

namespace misc
{
  void find_and_replace( std::string& source, const std::string& find, const std::string& replace ) 
  {
    size_t found = source.rfind( find );
    if( found != std::string::npos )
      source.replace( found, find.length(), replace );
  }

  //dirty hack
  int FtoIround(float d)
  {
    return d<0 ? d-.5f : d+.5f;
  }

  char roundc( float a )
  {
    if( a < 0 )
      a -= 0.5f;
    if( a > 0 )
      a += 0.5f;
    if( a < -127 )
      a = -127;
    else if( a > 127 )
      a = 127;
    return static_cast<char>( a );
  }

  float frand()
  {
    return rand() / static_cast<float>( RAND_MAX );
  }

  float randfloat(float lower, float upper)
  {
    return lower + ( upper - lower ) * frand();
  }

  int randint(int lower, int upper)
  {
    return lower + static_cast<int>( ( upper + 1 - lower ) * frand() );
  }

  std::string replaceSpecialChars(const std::string& input)
  {
    std::string text = input;
    find_and_replace(text,"Ã´","o");  //ô
    find_and_replace(text,"Å¡3","s");  //š
    find_and_replace(text,"Ã¶","o");  //ö
    find_and_replace(text,"Ãº","u");  //ú
    find_and_replace(text,"Ã¼","u");  //ü
    find_and_replace(text,"Ã½","y");  //ý
    find_and_replace(text,"Å®","U");  //Ù
    find_and_replace(text,"Ä‚","A");  //Ã
    find_and_replace(text,"Å¯","u");  //ù
    find_and_replace(text,"Äƒ","a");  //ã
    find_and_replace(text,"Å°","U");  //Û
    find_and_replace(text,"Å±","u");  //û
    find_and_replace(text,"Ä†","AE");  //Æ
    find_and_replace(text,"Åº","Y");  //Ÿ
    find_and_replace(text,"Ä‡","ae");  //æ
    find_and_replace(text,"ÄOE","E");  //È
    find_and_replace(text,"Âμ","u");  //μ
    find_and_replace(text,"Ä?","e");  //è
    find_and_replace(text,"Å½","Z");  //Ž
    find_and_replace(text,"ÄŽ","I");  //Ï
    find_and_replace(text,"Å¾","z");  //ž
    find_and_replace(text,"Ä?","i");  //ï
    find_and_replace(text,"Ä?","D");  //Ð
    find_and_replace(text,"Ë˜","c");  //¢
    find_and_replace(text,"Ë™","y");  //ÿ 
    find_and_replace(text,"Ã?","A");  //Á
    find_and_replace(text,"Ä˜","E");  //Ê
    find_and_replace(text,"Ã‚","A");  //Â
    find_and_replace(text,"Ä™","e");  //ê
    find_and_replace(text,"Ã„","Ae");  //Ä
    find_and_replace(text,"Äš","I");  //Ì
    find_and_replace(text,"Ã‡","C");  //Ç
    find_and_replace(text,"Ä›","i");  //ì
    find_and_replace(text,"Ã‰","E");  //É
    find_and_replace(text,"Ä¹","A");  //Å
    find_and_replace(text,"Ã‹","E");  //Ë
    find_and_replace(text,"Äº","a");  //å
    find_and_replace(text,"Ã?","I");  //Í 
    find_and_replace(text,"ÃŽ","I");  //Î
    find_and_replace(text,"Ã“","O");  //Ó
    find_and_replace(text,"Ã”","O");  //Ô
    find_and_replace(text,"Ã–","Oe");  //Ö
    find_and_replace(text,"Åƒ","N");  //Ñ
    find_and_replace(text,"â€","t");  //†
    find_and_replace(text,"Ã—","x");  //×
    find_and_replace(text,"Å„","n");  //ñ
    find_and_replace(text,"Ãš","U");  //Ú
    find_and_replace(text,"Å‡","O");  //Ò
    find_and_replace(text,"Ãoe","Ue");  //Ü
    find_and_replace(text,"Åˆ","o");  //ò
    find_and_replace(text,"Ã?","Y");  //Ý
    find_and_replace(text,"Å?","O");  //Õ
    find_and_replace(text,"ÃŸ","ss");  //ß
    find_and_replace(text,"Å‘","o");  //õ
    find_and_replace(text,"Ã¡","a");  //á
    find_and_replace(text,"Å”","A");  //À
    find_and_replace(text,"Ã¢","a");  //â
    find_and_replace(text,"Å•","a");  //à
    find_and_replace(text,"â‚¬","euro");  //€
    find_and_replace(text,"Ã¤","ae");  //ä
    find_and_replace(text,"Å˜","O");  //Ø
    find_and_replace(text,"Ã§","c");  //ç
    find_and_replace(text,"Å™","o");  //ø
    find_and_replace(text,"Ã©","e");  //é
    find_and_replace(text,"Åš","OE");  //OE
    find_and_replace(text,"Ã«","e");  //ë
    find_and_replace(text,"Å›","oe");  //oe
    find_and_replace(text,"Ã-","i");  //í
    find_and_replace(text,"Åž","a");  //ª
    find_and_replace(text,"Ã®","i");  //î
    find_and_replace(text,"Ã³","o");  //ó
    find_and_replace(text,"Å","S");    //Š
    find_and_replace(text,"Ã¤","ae");  //ä
    find_and_replace(text,"Ã¶","oe");  //ö
    find_and_replace(text,"Ã¼","ue");  //ü
    find_and_replace(text,"ÃŸ","ss");  //?
    find_and_replace(text,"Ã„","Ae");  //Ä
    find_and_replace(text,"Ã–","Oe");  //Ö
    find_and_replace(text,"Ãoe","Ue");  //Ü

    return text;
  }

  int getADTCord(float cord)
  {
    return cord / 533.33333f;
  }
}
