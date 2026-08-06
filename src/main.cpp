#include <crow.h>
#include <iostream>
#include <filesystem>
#include "db/db_manager.hpp"
#include "api/bouts_controller.hpp"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    int port = 5000;
    if (const char* envPort = std::getenv("PORT")) {
        port = std::atoi(envPort);
    }

    std::cout << "[Fencing Tracker C++] Initializing database..." << std::endl;
    DbManager db("fencing.db");
    if (!db.init()) {
        std::cerr << "[Fencing Tracker C++] Database initialization failed!" << std::endl;
        return 1;
    }

    crow::SimpleApp app;

    // Register API endpoints
    registerBoutsRoutes(app, db);

    // Root endpoint -> serve wwwroot/index.html
    CROW_ROUTE(app, "/")( []() {
        crow::response res;
        res.set_static_file_info("wwwroot/index.html");
        return res;
    });

    // Fallback static file route for wwwroot assets (css, js, etc.)
    CROW_ROUTE(app, "/<path>")
    ([](const std::string& path) {
        crow::response res;
        std::string fullPath = "wwwroot/" + path;
        if (fs::exists(fullPath) && !fs::is_directory(fullPath)) {
            res.set_static_file_info(fullPath);
        } else {
            res.code = 404;
            res.write("404 Not Found");
        }
        return res;
    });

    std::cout << "[Fencing Tracker C++] Server running on port " << port << std::endl;
    app.port(port).multithreaded().run();

    return 0;
}
