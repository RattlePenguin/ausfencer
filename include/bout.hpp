#include <string>

enum Weapon { Foil, Epee, Sabre };

struct Bout {
  std::string leftFencerName;
  std::string rightFencerName;

  Weapon weapon;

  int leftScore;
  int rightScore;

  int leftYellow;
  int rightYellow;

  int leftReds;
  int rightReds;
};
