#include <iostream>

#include "server.hpp"

int main() {

  try {
    ServerConfig config{};

    Server server(config);
    server.start();
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
