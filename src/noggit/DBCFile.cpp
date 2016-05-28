// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBCFile.h>

#include <noggit/Log.h>

DBCFile::DBCFile (const QString& filename)
  : _filename (filename)
{}

void DBCFile::open()
{
  noggit::mpq::file file (_filename);
  LogDebug << "Opening DBC " << qPrintable (_filename) << std::endl;
  file.read (&header, sizeof (header));

  //! \note Yup, in these files, they store the magic as string, not uint32_t.
  assert (header.magic == 'CBDW');
  assert (header.fieldCount * 4 == header.recordSize);

  data.resize (header.recordSize * header.recordCount);
  stringTable.resize (header.stringSize);
  file.read (data.data(), data.size());
  file.read (stringTable.data(), stringTable.size());
}

void DBCFile::saveToProjectPath()
{
  std::vector<char> buffer (sizeof(header) + header.recordSize * header.recordCount + stringTable.size());
  memcpy (buffer.data(), &header, sizeof(header));
  memcpy (buffer.data() + sizeof(header), data.data(), header.recordSize * header.recordCount);
  memcpy (buffer.data() + sizeof(header) + header.recordSize * header.recordCount, stringTable.data(), stringTable.size());

  noggit::mpq::file file (_filename);
  file.setBuffer (buffer.data(), buffer.size());
  file.save_to_disk();
}
