#include "dbcfile.h"
#include "mpq.h"
#include "Log.h"

DBCFile::DBCFile(const std::string& _filename):
	filename(_filename),
	data(0)
{
	
}
void DBCFile::open()
{
	MPQFile f( filename );
	
	if( f.isEof() )
	{
		LogError << "The DBC file \"" << filename << "\" could not be opened. This application may crash soon as the file is most likely needed." << std::endl;
		return;
	}
	LogDebug << "Opening DBC \"" << filename << "\"" << std::endl;
	
	char header[4];
	unsigned int na,nb,es,ss;

	f.read(header,4); // Number of records
	assert(header[0]=='W' && header[1]=='D' && header[2]=='B' && header[3] == 'C');
	f.read(&na,4); // Number of records
	f.read(&nb,4); // Number of fields
	f.read(&es,4); // Size of a record
	f.read(&ss,4); // String size
	
	recordSize = es;
	recordCount = na;
	fieldCount = nb;
	stringSize = ss;
	assert(fieldCount*4 == recordSize);

	data = new unsigned char[recordSize*recordCount+stringSize];
	stringTable = data + recordSize*recordCount;
	f.read(data,recordSize*recordCount+stringSize);
	f.close();
}

DBCFile::~DBCFile()
{
	delete [] data;
}

DBCFile::Record DBCFile::getRecord(size_t id)
{
	assert(data);
	return Record(*this, data + id*recordSize);
}

DBCFile::Iterator DBCFile::begin()
{
	assert(data);
	return Iterator(*this, data);
}
DBCFile::Iterator DBCFile::end()
{
	assert(data);
	return Iterator(*this, stringTable);
}

