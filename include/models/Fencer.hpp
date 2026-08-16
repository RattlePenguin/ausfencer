#pragma once

#include <string>

struct Fencer {
  int id;
  std::string first_name;
  std::string last_name;
  int birth_year;
};

/**
 * Author note:
 * I think I'm going to store only fencer information here, then when searching
 * for bouts this fencer is involved in I will search all bouts with their id.
 */
