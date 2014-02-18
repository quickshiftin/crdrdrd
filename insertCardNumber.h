#ifndef MXN_INSERT_CRD_NUM
#define MXN_INSERT_CRD_NUM
 
#include <sqlite3.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

 /// method prototypes
sqlite3 *initSqlite3DB(char *dbToInit);
int insertCardNumber(char *cardNumber, sqlite3 *db);

#endif
