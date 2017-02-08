// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/Settings.h>
#include <noggit/mysql.h>

#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>

namespace
{
  std::unique_ptr<sql::Connection> connect (Settings::mysql_connection_info const& info)
  {
    std::unique_ptr<sql::Connection> Con
      (get_driver_instance()->connect (info.Server, info.User, info.Pass));
    Con->setSchema(info.Database);

    return Con;
  }
}

namespace mysql
{
  uint32_t getGUIDFromDB()
  {
    auto Con (connect (*Settings::getInstance()->mysql));
    std::unique_ptr<sql::PreparedStatement> pstmt (Con->prepareStatement ("SELECT UID FROM UIDs"));
    std::unique_ptr<sql::ResultSet> res (pstmt->executeQuery());

    uint32_t highGUID (0);
    while (res->next())
    {
      highGUID = res->getInt (1);
    }

    return highGUID;
  }

  void UpdateUIDInDB(uint32_t NewUID)
  {
    auto Con (connect (*Settings::getInstance()->mysql));
    std::unique_ptr<sql::PreparedStatement> pstmt (Con->prepareStatement ("UPDATE UIDs SET UID=(?)"));
    pstmt->setInt (1, NewUID);
    pstmt->executeUpdate();
  }
}
