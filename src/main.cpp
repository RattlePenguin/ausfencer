#include <iostream>

#include "server.hpp"

int main(int argc, char *argv[]) {

  try {
    ServerConfig config{};

    Server server(config);
    server.start();
  } catch (const std::exception &e) {
    std::cout << e.what() << '\n';
  }
}
