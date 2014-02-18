/***
 * insertCardNumber.c
 *
 *	5/7/2007
 *	Moxune
 *	Nathan Nobbe

 *	This is a down-and-dirty file to insert a card number into an sqlite database.
 */

#include "mxnLogger.h"
#include "insertCardNumber.h"

/**
 * initSqlite3DB()
 *		this method will create a connection to an sqlite3 database specified by dbToInit
 *		upon success the handle to the open database will be set to the OpenDB
 *	RETURN:
 *		sqlite3 handle on success
 *		NULL on failure
 */
sqlite3 *initSqlite3DB(char *dbToInit) {
  sqlite3 *db;
  mxnLog(MXN_INFO,  "--> Entering initSqlite3DB(); dbToInit: %s", dbToInit);

  /// attempt to create a connection to the database
  if (sqlite3_open(dbToInit, &db)!=SQLITE_OK) {
    mxnLog(MXN_CRIT, "Failed to open the %s database", dbToInit);
    return NULL;
  }

  mxnLog(MXN_INFO, "<-- Exiting initSqlite3DB()); CONNECTION SUCCESS");
  return db;
}

/**
 * insertCardNumber()
 *		this method takes a handle to an open sqlite3 database and inserts the card number specified by
 *		cardNumber.
 *		if the insert is a failure a critical message is logged.
 *		the method returns 0 upon failure and 1 upon success
 */
int insertCardNumber(char* cardNumber, sqlite3 *db) {
  int insertSuccess = 1;					// method success
  int execResult = -1;						// result of query
  char *zErrMsg = 0;						// an error message prepared by sqlite3
  char *insertCardNumberQuery;				// insert query string


  /// prepare the query statement
  insertCardNumberQuery = sqlite3_mprintf("INSERT INTO CARD_SWIPE_HISTORY (card_number, datetime_swiped) VALUES (%Q, %i);", cardNumber, time(NULL));

  mxnLog(MXN_DEBUG,"%s", insertCardNumberQuery);

  execResult = sqlite3_exec(db, insertCardNumberQuery, 0, 0, &zErrMsg);
  sqlite3_free(insertCardNumberQuery);

  if(execResult != 0) {			// assume query failed
    mxnLog(MXN_CRIT, "Failed to insert card number: %s", cardNumber);
    mxnLog(MXN_CRIT, "Sqlite3 error:%s\n", sqlite3_errmsg(db));
    insertSuccess = 0;
  }

  //There is no reason to close the database after every insert.
  return insertSuccess;
}
