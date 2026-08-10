#include "sqlite3.h"

int main(int argc, char *argv[]) {
  sqlite3 *db;
  sqlite3_open("db.sqlite", &db);
  sqlite3_close(db);

  // start up the server so phones can send and receive requests
  // and web pages are active
  runServer();

  // open our database for access

  // tap start bout to pull up remote page
  //
}
