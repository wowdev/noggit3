// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <mysql/mysql.h>
#include <noggit/world.h>

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

  bool IsMySQLConfigTrue()
  {
    if (Settings::getInstance()->mysql)
	{
      return true;
	}
	else
	{
      return false;
	}
  }

  bool hasMaxUIDStoredDB(Settings::mysql_connection_info const& info, std::size_t mapID)
  {
	  auto Con(connect(info));
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("SELECT * FROM UIDs WHERE MapId=(?)"));
	  pstmt->setInt(1, mapID);
	  std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
	  if (res->rowsCount() >= 1)
	  {
		  return true;
	  }
	  else
	  {
		  return false;
	  }
  }

  std::uint32_t getGUIDFromDB(Settings::mysql_connection_info const& info, std::size_t mapID)
  {
	  auto Con(connect(info));
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("SELECT UID FROM UIDs WHERE MapId=(?)"));
	  pstmt->setInt(1, mapID);
	  std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

	  std::uint32_t highGUID(0);
	  if (res->rowsCount() == 0) { return 0; }
	  while (res->next())
	  {
		  highGUID = res->getInt(1);
	  }

	  return highGUID;
  }

  void insertUIDinDB(Settings::mysql_connection_info const& info, std::size_t mapID, std::uint32_t NewUID)
  {
	  auto Con(connect(info));
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("INSERT INTO UIDs SET MapId=(?), UID=(?)"));
	  pstmt->setInt(1, mapID);
	  pstmt->setInt(2, NewUID);
	  pstmt->executeUpdate();
  }

  void updateUIDinDB (Settings::mysql_connection_info const& info, std::size_t mapID, std::uint32_t NewUID)
  {
	  auto Con(connect(info));
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("UPDATE UIDs SET UID=(?) WHERE MapId=(?)"));
	  pstmt->setInt(1, NewUID);
	  pstmt->setInt(2, mapID);
	  pstmt->executeUpdate();
  }

}
