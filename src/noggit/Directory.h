// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>

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

  Directory::Ptr addDirectory(std::string name);
  void addFile(const std::string& name);

  Directory::Ptr operator[](const std::string& name);

  Directories::const_iterator directoriesBegin() const;
  Directories::const_iterator directoriesEnd() const;

  Files::const_iterator filesBegin() const;
  Files::const_iterator filesEnd() const;
};
