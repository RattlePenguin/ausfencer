#pragma once

enum Weapon { Foil, Epee, Sabre };

struct Bout {
  int id;
  int left_fencer_id;
  int right_fencer_id;

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
