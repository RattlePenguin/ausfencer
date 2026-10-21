#include "handlers/tracker.hpp"
#include <charconv>

crow::json::wvalue public_rows(const DbRows &rows) {
  crow::json::wvalue::list result;
  for (const auto &row : rows) {
    crow::json::wvalue record;
    for (const auto &field : row) record[field.first] = field.second;
    result.push_back(std::move(record));
  }
  return crow::json::wvalue(std::move(result));
}

crow::response tracker_response(const std::function<crow::json::wvalue()> &action) {
  crow::response response;
  try { response = crow::response(200,action()); }
  catch (const TrackerNotFound &e) { response = crow::response(404,e.what()); }
  catch (const std::domain_error &e) { response = crow::response(403,e.what()); }
  catch (const std::invalid_argument &e) { response = crow::response(400,e.what()); }
  catch (const std::exception &) { response = crow::response(500,"Unable to complete request"); }
  response.set_header("Cache-Control","no-store");
  response.set_header("X-Content-Type-Options","nosniff");
  return response;
}

void TrackerHandler::register_routes(App &app) {
  app.route_dynamic(basePath_ + "/fencers/<int>").methods(crow::HTTPMethod::GET)([this](const crow::request &req, int id) {
    return tracker_response([&] { return read_profile(id,req); });
  });
  app.route_dynamic(basePath_ + "/fencers").methods(crow::HTTPMethod::GET)([this](const crow::request &req) {
    return tracker_response([&] {
      auto integer = [&](const char *key, int fallback) {
        const char *value = req.url_params.get(key);
        if (!value) return fallback;
        int result; auto end = value + std::char_traits<char>::length(value);
        auto parsed = std::from_chars(value,end,result);
        if (parsed.ec != std::errc{} || parsed.ptr != end) throw std::invalid_argument("Invalid pagination");
        return result;
      };
      crow::json::wvalue result;
      result["demo"] = true;
      result["fencers"] = public_rows(repo_->search(req.url_params.get("q") ? req.url_params.get("q") : "",
                                                    integer("page",1),integer("limit",20)));
      return result;
    });
  });
  app.route_dynamic(basePath_ + "/events").methods(crow::HTTPMethod::GET)([this] {
    return tracker_response([&] { crow::json::wvalue result; result["events"] = public_rows(repo_->events()); return result; });
  });
  app.route_dynamic(basePath_ + "/events/<string>").methods(crow::HTTPMethod::GET)([this](std::string id) {
    return tracker_response([&] {
      crow::json::wvalue result;
      DbRows selected;
      for (const auto &event : repo_->events()) if (event.at("id") == id) selected.push_back(event);
      if (selected.empty()) throw TrackerNotFound("Event not found");
      result["events"] = public_rows(selected); result["entries"] = public_rows(repo_->event_entries(id));
      result["bouts"] = public_rows(repo_->event_bouts(id)); result["demo"] = true;
      return result;
    });
  });
  app.route_dynamic(basePath_ + "/bouts/<int>").methods(crow::HTTPMethod::GET)([this](int id) {
    return tracker_response([&] {
      auto rows = repo_->bout(id);
      if (id <= 0 || rows.empty()) throw TrackerNotFound("Bout not found");
      return public_rows(rows);
    });
  });
}
