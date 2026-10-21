#pragma once

class DbManager;
int check_tracker_schema(DbManager &db);
void migrate_tracker(DbManager &db);
