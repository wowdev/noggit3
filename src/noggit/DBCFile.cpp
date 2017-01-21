// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/MPQ.h>

#include <string>

DBCFile::DBCFile(const std::string& _filename)
  : filename(_filename)
{}

void DBCFile::open()
{
  MPQFile f (filename);

  if (f.isEof())
  {
    LogError << "The DBC file \"" << filename << "\" could not be opened. This application may crash soon as the file is most likely needed." << std::endl;
    return;
  }
  LogDebug << "Opening DBC \"" << filename << "\"" << std::endl;

  char header[4];

  f.read(header, 4); // Number of records
  assert(header[0] == 'W' && header[1] == 'D' && header[2] == 'B' && header[3] == 'C');
  f.read(&recordCount, 4);
  f.read(&fieldCount, 4);
  f.read(&recordSize, 4);
  f.read(&stringSize, 4);

  if (fieldCount * 4 != recordSize)
  {
    throw std::logic_error ("non four-byte-columns not supported");
  }

  data.resize (recordSize * recordCount);
  f.read (data.data(), data.size());

  stringTable.resize (stringSize);
  f.read (stringTable.data(), stringTable.size());

  f.close();
}
