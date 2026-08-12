#pragma once

#include "Fencer.hpp"

struct Bout {
  int id;
  Fencer left_fencer;
  Fencer right_fencer;

  int weapon;
  int time;

  int left_score;
  int right_score;

  int left_yellow;
  int right_yellow;

  int left_red;
  int right_red;
};
