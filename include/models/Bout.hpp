#pragma once

#include <ctime>

// Values are persisted as SQLite integers; keep this mapping stable.
enum Weapon { Foil = 0, Epee = 1, Sabre = 2 };

struct Bout {
  int id;
  int left_fencer_id;
  int right_fencer_id;
  time_t timestamp;

  Weapon weapon;
  int time; // seconds

  int left_score; // should be capped at 99
  int right_score;

  // usually more than 1 yellow is not allowed
  int left_yellow;
  int right_yellow;

  int left_red;
  int right_red;
};
