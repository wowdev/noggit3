#include "Misc.h"

#include <string>
#include <vector>
#include <map>

namespace misc
{
  void find_and_replace( std::string& source, const std::string& find, const std::string& replace ) 
  {
	size_t found = source.rfind( find );
	while(found!=std::string::npos) //fixed unknown letters replace. Now it works correctly and replace all found symbold instead of only one at previous versions
	{
		source.replace( found, find.length(), replace );
		found = source.rfind( find );
	}
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
	find_and_replace(text,"А","A"); //manually set english version of russian letters to fix encoding troubles
	find_and_replace(text,"Б","B"); //now Noggit shows readable symbols in ruRU client!
	find_and_replace(text,"В","V"); //
	find_and_replace(text,"Г","G"); //
	find_and_replace(text,"Д","D"); //
	find_and_replace(text,"Е","E"); //
	find_and_replace(text,"Ё","E"); //
	find_and_replace(text,"Ж","J"); //
	find_and_replace(text,"З","Z"); //
	find_and_replace(text,"И","I"); //
	find_and_replace(text,"Й","Y"); //
	find_and_replace(text,"К","K"); //
	find_and_replace(text,"Л","L"); //
	find_and_replace(text,"М","M"); //
	find_and_replace(text,"Н","N"); //
	find_and_replace(text,"О","O"); //
	find_and_replace(text,"П","P"); //
	find_and_replace(text,"Р","R"); //
	find_and_replace(text,"С","S"); //
	find_and_replace(text,"Т","T"); //
	find_and_replace(text,"У","U"); //
	find_and_replace(text,"Ф","F"); //
	find_and_replace(text,"Х","H"); //
	find_and_replace(text,"Ц","C"); //
	find_and_replace(text,"Ч","Ch"); //
	find_and_replace(text,"Ш","Sh"); //
	find_and_replace(text,"Щ","Sch"); //
	find_and_replace(text,"Ъ","'"); //
	find_and_replace(text,"Ы","Y"); //
	find_and_replace(text,"Ь",""); //
	find_and_replace(text,"Э","E"); //
	find_and_replace(text,"Ю","Ju"); //
	find_and_replace(text,"Я","Ja"); //
	find_and_replace(text,"а","a"); //
	find_and_replace(text,"б","b"); //
	find_and_replace(text,"в","v"); //
	find_and_replace(text,"г","g"); //
	find_and_replace(text,"д","d"); //
	find_and_replace(text,"е","e"); //
	find_and_replace(text,"ё","e"); //
	find_and_replace(text,"ж","j"); //
	find_and_replace(text,"з","z"); //
	find_and_replace(text,"и","i"); //
	find_and_replace(text,"й","y"); //
	find_and_replace(text,"к","k"); //
	find_and_replace(text,"л","l"); //
	find_and_replace(text,"м","m"); //
	find_and_replace(text,"н","n"); //
	find_and_replace(text,"о","o"); //
	find_and_replace(text,"п","p"); //
	find_and_replace(text,"р","r"); //
	find_and_replace(text,"с","s"); //
	find_and_replace(text,"т","t"); //
	find_and_replace(text,"у","u"); //
	find_and_replace(text,"ф","f"); //
	find_and_replace(text,"х","h"); //
	find_and_replace(text,"ц","c"); //
	find_and_replace(text,"ч","ch"); //
	find_and_replace(text,"ш","sh"); //
	find_and_replace(text,"щ","sch"); //
	find_and_replace(text,"ъ","'"); //
	find_and_replace(text,"ы","i"); //
	find_and_replace(text,"ь",""); //
	find_and_replace(text,"э","e"); //
	find_and_replace(text,"ю","ju"); //
	find_and_replace(text,"я","ja"); //

    return text;
  }

  int getADTCord(float cord)
  {
    return cord / 533.33333f;
  }
}
