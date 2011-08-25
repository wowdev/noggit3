#include "Directory.h"

Directory::Directory()
: _directories()
, _files()
{
}

Directory::Ptr Directory::addDirectory( std::string name )
{
  Directory::Ptr lCurrentDir = shared_from_this();
  size_t found;
  found = name.find_last_of( "/\\" );
  while( found != std::string::npos )
  {
    lCurrentDir = lCurrentDir->addDirectory( name.substr( 0, found ) );
    name = name.substr( found + 1 );
    found = name.find_last_of( "/\\" );
  }
  if( ! lCurrentDir->_directories[name] )
    lCurrentDir->_directories[name] = Directory::Ptr(new Directory());
  return lCurrentDir->_directories[name];
}

void Directory::addFile( const std::string& name )
{
  _files.push_back( name );
}

Directory::Ptr Directory::operator[]( const std::string& name )
{
  if( _directories.find(name) != _directories.end() )
    return _directories[name];
  else
    return Directory::Ptr();
}

Directory::Directories::const_iterator Directory::directoriesBegin() const
{
  return _directories.begin();
}
Directory::Directories::const_iterator Directory::directoriesEnd() const
{
  return _directories.end();
}

Directory::Files::const_iterator Directory::filesBegin() const
{
  return _files.begin();
}
Directory::Files::const_iterator Directory::filesEnd() const
{
  return _files.end();
}
