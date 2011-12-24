#ifndef __DIRECTORY_H
#define __DIRECTORY_H

#include <string>
#include <map>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class Directory : public boost::enable_shared_from_this<Directory>
{
public:
  typedef std::string File;
  typedef boost::shared_ptr<Directory> Ptr;
  typedef std::map<std::string, Directory::Ptr > Directories;
  typedef std::vector<File> Files;

private:
  Directories _directories;
  Files _files;

public:
  Directory();

  Directory::Ptr addDirectory( std::string name );
  void addFile( const std::string& name );

  Directory::Ptr operator[]( const std::string& name );

  Directories::const_iterator directoriesBegin() const;
  Directories::const_iterator directoriesEnd() const;

  Files::const_iterator filesBegin() const;
  Files::const_iterator filesEnd() const;
};
#endif
