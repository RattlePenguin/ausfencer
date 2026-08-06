#pragma once

#include <crow.h>
#include <nlohmann/json.hpp>
#include "../db/db_manager.hpp"

inline void registerBoutsRoutes(crow::SimpleApp& app, DbManager& db) {

    // POST /api/bouts - Create bout
    CROW_ROUTE(app, "/api/bouts").methods(crow::HTTPMethod::Post)([&db](const crow::request& req) {
        try {
            auto bodyJson = nlohmann::json::parse(req.body);
            Bout bout = bodyJson.get<Bout>();

            if (db.createBout(bout)) {
                nlohmann::json resJson = bout;
                crow::response res(201, resJson.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            } else {
                crow::response res(500, nlohmann::json{{"error", "Failed to create bout in database"}}.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }
        } catch (const std::exception& e) {
            crow::response res(400, nlohmann::json{{"error", std::string("Invalid JSON payload: ") + e.what()}}.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });

    // GET /api/bouts - List all bouts
    CROW_ROUTE(app, "/api/bouts").methods(crow::HTTPMethod::Get)([&db]() {
        std::vector<Bout> bouts = db.getAllBouts();
        nlohmann::json resJson = bouts;
        crow::response res(200, resJson.dump());
        res.set_header("Content-Type", "application/json");
        return res;
    });

    // GET /api/bouts/<id> - Get single bout
    CROW_ROUTE(app, "/api/bouts/<string>").methods(crow::HTTPMethod::Get)([&db](const std::string& id) {
        auto boutOpt = db.getBoutById(id);
        if (boutOpt.has_value()) {
            nlohmann::json resJson = boutOpt.value();
            crow::response res(200, resJson.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        } else {
            crow::response res(404, nlohmann::json{{"error", "Bout not found"}}.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });

    // PUT /api/bouts/<id> - Update bout
    CROW_ROUTE(app, "/api/bouts/<string>").methods(crow::HTTPMethod::Put)([&db](const crow::request& req, const std::string& id) {
        try {
            auto existingOpt = db.getBoutById(id);
            if (!existingOpt.has_value()) {
                crow::response res(404, nlohmann::json{{"error", "Bout not found"}}.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }

            auto bodyJson = nlohmann::json::parse(req.body);
            Bout updated = bodyJson.get<Bout>();
            updated.id = id;

            if (db.updateBout(id, updated)) {
                crow::response res(204);
                return res;
            } else {
                crow::response res(500, nlohmann::json{{"error", "Failed to update bout in database"}}.dump());
                res.set_header("Content-Type", "application/json");
                return res;
            }
        } catch (const std::exception& e) {
            crow::response res(400, nlohmann::json{{"error", std::string("Invalid payload: ") + e.what()}}.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });

    // DELETE /api/bouts/<id> - Delete bout
    CROW_ROUTE(app, "/api/bouts/<string>").methods(crow::HTTPMethod::Delete)([&db](const std::string& id) {
        if (db.deleteBout(id)) {
            crow::response res(204);
            return res;
        } else {
            crow::response res(404, nlohmann::json{{"error", "Bout not found"}}.dump());
            res.set_header("Content-Type", "application/json");
            return res;
        }
    });
}
