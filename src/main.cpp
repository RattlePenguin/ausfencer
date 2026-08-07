#include "sqlite3.h"

int main(int argc, char *argv[]) {
  sqlite3 *db;
  sqlite3_open("textonly.db", &db);
  sqlite3_close(db);
}
