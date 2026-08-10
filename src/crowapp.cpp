#include <crow.h>

int main() {
  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([]() { return "index html"; });

  CROW_ROUTE(app, "/bouts")([]() {
    return "all bouts";
    // return all bouts in chronological order recent first
    // json object, bout id, names, score
  });

  CROW_ROUTE(app, "/bouts/<int>")([](int count) {
    return std::to_string(count);
    // returns bout with full details
    // referee, penalties
  });

  app.port(18080).multithreaded().run();
}
