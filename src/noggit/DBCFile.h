// DBCFile.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
// Beket <snipbeket@mail.ru>
// Bernd Lörwald <bloerwald+noggit@googlemail.com>
// Stephan Biegel <project.modcraft@googlemail.com>
// Tigurius <bstigurius@googlemail.com>

#ifndef DBCFILE_H
#define DBCFILE_H

#include <cassert>
#include <QString>
#include <stdio.h>

class DBCFile
{
public:
  explicit DBCFile(const QString& filename);
  virtual ~DBCFile();

  // Open database. It must be openened before it can be used.
  void open();

  // Database exceptions
  class Exception
  {
  public:
    explicit Exception(const std::string& pmessage): message(pmessage)
    { }
    virtual ~Exception()
    { }
    const std::string& getMessage() {return message;}
  private:
    std::string message;
  };
  class NotFound: public Exception
  {
  public:
    NotFound(): Exception("Key was not found")
    { }
  };
  // Iteration over database
  class Iterator;
  class Record
  {
  public:
    const void setData(size_t field, float value) const
    {
        assert(field < file.fieldCount);
        memset(offset+field*4,0,4);
        unsigned char *fp = (uchar*)&value;
        unsigned char *op = offset+field*4;
        while(op < offset+field*4+4)
        {
             *op++ = *fp++;
        }
    }
    const void setData(size_t field, unsigned char value) const
    {
        assert(field < file.fieldCount);
        memset(offset+field*4,0,4);
        memset(offset+field*4,value,sizeof(value));
    }
    const float& getFloat(size_t field) const
    {
      assert(field < file.fieldCount);
      return *reinterpret_cast<float*>(offset+field*4);
    }
    const unsigned int& getUInt(size_t field) const
    {
      assert(field < file.fieldCount);
      return *reinterpret_cast<unsigned int*>(offset+field*4);
    }
    const int& getInt(size_t field) const
    {
      assert(field < file.fieldCount);
      return *reinterpret_cast<int*>(offset+field*4);
    }
    const char *getString(size_t field) const
    {
      assert(field < file.fieldCount);
      size_t stringOffset = getUInt(field);
      assert(stringOffset < file.stringSize);
      return reinterpret_cast<char*>(file.stringTable + stringOffset);
    }
    const char *getLocalizedString( size_t field, int locale = -1 ) const
    {
      int loc = locale;
      if( locale == -1 )
      {
        assert(field < file.fieldCount -  8 );
        for( loc = 0; loc < 9; loc++ )
        {
          size_t stringOffset = getUInt(field + loc);
          if( stringOffset != 0 )
            break;
        }
      }

      assert( field + loc < file.fieldCount );
      size_t stringOffset = getUInt( field + loc );
      assert( stringOffset < file.stringSize );
      return reinterpret_cast<char*>( file.stringTable + stringOffset );
    }
  private:
    Record(const DBCFile &pfile, unsigned char *poffset): file(pfile), offset(poffset) {}
    const DBCFile &file;
    unsigned char *offset;

    friend class DBCFile;
    friend class DBCFile::Iterator;
  };
  /** Iterator that iterates over records
  */
  class Iterator
  {
  public:
    Iterator(const DBCFile &file, unsigned char *offset):
      record(file, offset) {}
    /// Advance (prefix only)
    Iterator & operator++() {
      record.offset += record.file.recordSize;
      return *this;
    }
    /// Return address of current instance
    Record const & operator*() const { return record; }
    const Record* operator->() const {
      return &record;
    }
    /// Comparison
    bool operator==(const Iterator &b) const
    {
      return record.offset == b.record.offset;
    }
    bool operator!=(const Iterator &b) const
    {
      return record.offset != b.record.offset;
    }
  private:
    Record record;
  };

  inline Record getRecord(size_t id)
  {
    //  assert(data);
    return Record(*this, data + id*recordSize);
  }

  inline Iterator begin()
  {
    //  assert(data);
    return Iterator(*this, data);
  }
  inline Iterator end()
  {
    //  assert(data);
    return Iterator(*this, data+recordCount*recordSize);
  }
  /// Trivial
  inline size_t getRecordCount() const { return recordCount;}
  inline size_t getFieldCount() const { return fieldCount; }
  inline Record getByID( unsigned int id, size_t field = 0 )
  {
    for( Iterator i = begin(); i!=end(); ++i )
    {
      if( i->getUInt( field ) == id )
        return ( *i );
    }
    throw NotFound();
  }

private:
  QString _filename;
  size_t recordSize;
  size_t recordCount;
  size_t fieldCount;
  size_t stringSize;
  unsigned char *data;
  unsigned char *stringTable;
};

#endif
