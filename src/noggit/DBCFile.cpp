// DBCFile.cpp is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#include <noggit/DBCFile.h>

#include <noggit/Log.h>

DBCFile::DBCFile (const QString& filename)
  : _filename (filename)
  , data (NULL)
  , recordCount(NULL)
  , fieldCount(NULL)
  , recordSize(NULL)
  , stringSize(NULL)
{
}

void DBCFile::open()
{
  f = new noggit::mpq::file(_filename);
  LogDebug << "Opening DBC " << qPrintable (_filename) << std::endl;
  headerData = new unsigned char[sizeof (header)];
  f->read (headerData, sizeof (header));
  memcpy(&header,headerData,sizeof(header));

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
  //ERROR: this is not c++ conform c array MUS be declared with in compiler time defined constant. Use sExtendableArray. We should pack this into the misc namespace in an own class file!
  char buffer[sizeof(header)+recordSize * recordCount+stringSize];
  memcpy(buffer,headerData,sizeof(header));
  memcpy(buffer+sizeof(header),data,recordSize * recordCount);
  memcpy(buffer+sizeof(header)+recordSize * recordCount,stringTable,stringSize);
  f->setBuffer(buffer,sizeof(buffer));
  f->save_to_disk();
}

DBCFile::~DBCFile()
{
  f->close();
  if( data && stringTable )
  {
    delete[] data;
    delete[] stringTable;
    data = NULL;
    stringTable = NULL;
  }
}


