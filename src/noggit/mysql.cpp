// This file is part of Noggit3, licensed under GNU General Public License (version 3).

 #include <noggit/mysql.h>
 #include <noggit/Settings.h>
 #include <external/mysql/connector/cppconn/driver.h>
 #include <external/mysql/connector/cppconn/prepared_statement.h>

#include <noggit/MPQ.h>
#include <noggit/MapChunk.h>
#include <noggit/MapChunk.h>
#include <noggit/MapTile.h>
#include <noggit/Misc.h>
#include <noggit/Project.h>
#include <noggit/World.h>
#include <noggit/mysql.h>
#include <noggit/map_index.hpp>
#include <noggit/uid_storage.hpp>

#include <boost/range/adaptor/map.hpp>

#include <forward_list>


sql::Driver     *driver;
sql::Connection *Con;
sql::ResultSet  *res;
sql::PreparedStatement *pstmt;

const char* server = Settings::getInstance()->Server.c_str();
const char* user = Settings::getInstance()->User.c_str();
const char* password = Settings::getInstance()->Pass.c_str();
const char* database = Settings::getInstance()->Database.c_str();


uint32_t Mysql::getGUIDFromDB()
{
  uint32_t highGUID = 0;
  driver = get_driver_instance();
  Con = driver->connect(server, user, password);
  Con->setSchema(database);
  pstmt = Con->prepareStatement("SELECT UID FROM UIDs");
  res = pstmt->executeQuery();
  while (res->next())
  {
	  highGUID = res->getInt(1);
  }
  return highGUID;
  Con->close();
  delete res;
  delete pstmt;
  delete Con;
  driver->threadEnd();
}

uint32_t Mysql::UpdateUIDInDB(uint32_t NewUID)
{
  driver = get_driver_instance();
  Con = driver->connect(server, user, password);
  Con->setSchema(database);
  pstmt = Con->prepareStatement("UPDATE UIDs SET UID=(?)");
  pstmt->setInt(1, NewUID);
  pstmt->executeUpdate();
  Con->close();
  delete res;
  delete pstmt;
  delete Con;
  driver->threadEnd();

  return NULL;
}