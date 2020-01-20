// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <mysql/mysql.h>
#include <noggit/world.h>

#include <QtCore/QSettings>

#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>

namespace
{
  std::unique_ptr<sql::Connection> connect()
  {
	QSettings settings;
    std::unique_ptr<sql::Connection> Con
      (get_driver_instance()->connect 
	    ( settings.value("project/mysql/server").toString().toStdString()
		, settings.value("project/mysql/user").toString().toStdString()
		, settings.value("project/mysql/pwd").toString().toStdString()
		)
	  );
    Con->setSchema(settings.value("project/mysql/db").toString().toStdString());

    return Con;
  }
}

namespace mysql
{
  bool hasMaxUIDStoredDB(std::size_t mapID)
  {
	  auto Con(connect());
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("SELECT * FROM UIDs WHERE MapId=(?)"));
	  pstmt->setInt(1, mapID);
	  std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
	  return res->rowsCount();
  }

  std::uint32_t getGUIDFromDB(std::size_t mapID)
  {
	  auto Con(connect());
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("SELECT UID FROM UIDs WHERE MapId=(?)"));
	  pstmt->setInt(1, mapID);
	  std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

	  std::uint32_t highGUID(0);
	  if (res->rowsCount() == 0) 
    { 
      return 0; 
    }
	  while (res->next())
	  {
		  highGUID = res->getInt(1);
	  }

	  return highGUID;
  }

  void insertUIDinDB(std::size_t mapID, std::uint32_t NewUID)
  {
	  auto Con(connect());
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("INSERT INTO UIDs SET MapId=(?), UID=(?)"));
	  pstmt->setInt(1, mapID);
	  pstmt->setInt(2, NewUID);
	  pstmt->executeUpdate();
  }

  void updateUIDinDB (std::size_t mapID, std::uint32_t NewUID)
  {
	  auto Con(connect());
	  std::unique_ptr<sql::PreparedStatement> pstmt(Con->prepareStatement("UPDATE UIDs SET UID=(?) WHERE MapId=(?)"));
	  pstmt->setInt(1, NewUID);
	  pstmt->setInt(2, mapID);
	  pstmt->executeUpdate();
  }
}
