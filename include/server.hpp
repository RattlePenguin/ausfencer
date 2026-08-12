#include <cstdint>

struct ServerConfig {
  uint16_t port{8080};
  uint16_t threads{2};
};

class Server {
  ServerConfig config_;

  bool is_running_{false};

public:
  explicit Server(const ServerConfig &config = ServerConfig()) {}
  ~Server() = default;

  void setup();
  void start();
};
