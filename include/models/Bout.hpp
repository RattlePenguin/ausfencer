#pragma once

#include "Fencer.hpp"

enum Weapon { Foil, Epee, Sabre };

struct Bout {
  int id;
  Fencer left_fencer;
  Fencer right_fencer;

  Weapon weapon;
  int time;

  int left_score;
  int right_score;

  int left_yellow;
  int right_yellow;

  int left_red;
  int right_red;
};
