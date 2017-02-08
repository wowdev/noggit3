// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Settings.h>
#include <noggit/mysql.h>

#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>

std::unique_ptr<sql::Connection> connect()
{
  const char* server = Settings::getInstance()->Server.c_str();
  const char* user = Settings::getInstance()->User.c_str();
  const char* password = Settings::getInstance()->Pass.c_str();
  const char* database = Settings::getInstance()->Database.c_str();

  std::unique_ptr<sql::Connection> Con
    (get_driver_instance()->connect (server, user, password));
  Con->setSchema(database);

  return Con;
}

uint32_t Mysql::getGUIDFromDB()
{
  auto Con (connect());
  std::unique_ptr<sql::PreparedStatement> pstmt (Con->prepareStatement ("SELECT UID FROM UIDs"));
  std::unique_ptr<sql::ResultSet> res (pstmt->executeQuery());

  uint32_t highGUID (0);
  while (res->next())
  {
	  highGUID = res->getInt (1);
  }

  return highGUID;
}

void Mysql::UpdateUIDInDB(uint32_t NewUID)
{
  auto Con (connect());
  std::unique_ptr<sql::PreparedStatement> pstmt (Con->prepareStatement ("UPDATE UIDs SET UID=(?)"));
  pstmt->setInt (1, NewUID);
  pstmt->executeUpdate();
}
