// DBCFile.cpp is part of Noggit3, licensed via GNU General Public License (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/DBCFile.h>

#include <noggit/Log.h>

DBCFile::DBCFile (const QString& filename)
  : _filename (filename)
  , f (NULL)
  , recordSize (0)
  , recordCount (0)
  , fieldCount (0)
  , stringSize (0)
  , data (NULL)
  , stringTable (NULL)
  , headerData (NULL)
{
}

void DBCFile::open()
{
  f = new noggit::mpq::file (_filename);
  LogDebug << "Opening DBC " << qPrintable (_filename) << std::endl;
  headerData = new unsigned char[sizeof (header)];
  f->read (headerData, sizeof (header));
  memcpy (&header, headerData, sizeof(header));

  //! \note Yup, in these files, they store the magic as string, not uint32_t.
  assert (header.magic == 'CBDW');
  assert (fieldCount * 4 == recordSize);

  recordCount = header.recordCount;
  fieldCount = header.fieldCount;
  recordSize = header.recordSize;
  stringSize = header.stringSize;

  data = new unsigned char[recordSize * recordCount];
  stringTable = new unsigned char[stringSize];
  f->read (data, recordSize * recordCount);
  f->read (stringTable, stringSize);
}

void DBCFile::saveToProjectPath()
{
  //sizeof(header)+recordSize*recordCount+stringSize

  char buffer[50000];//WRONG: Use extendeble array like in ADT save.
  memcpy (buffer, headerData, sizeof(header));
  memcpy (buffer + sizeof(header), data,recordSize * recordCount);
  memcpy (buffer + sizeof(header) + recordSize * recordCount, stringTable, stringSize);
  f->setBuffer (buffer, sizeof(buffer));
  f->save_to_disk();
}

DBCFile::~DBCFile()
{
  delete f;
  f = NULL;

  delete[] data;
  data = NULL;

  delete[] stringTable;
  stringTable = NULL;
}


