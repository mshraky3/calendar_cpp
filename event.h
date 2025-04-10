#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// Event structure definition
struct Event {
    std::string date;    // Format: dd/mm/yyyy
    std::string name;
    int priority;        // 1 (highest) to 5 (lowest)
    std::string status;  // "upcoming" or "passed"
};

// Convert Event to JSON object
inline nlohmann::json eventToJson(const Event& ev) {
    return nlohmann::json{
        {"date", ev.date},
        {"name", ev.name},
        {"priority", ev.priority},
        {"status", ev.status}
    };
}

// Convert JSON object to Event (assumes valid input)
inline Event jsonToEvent(const nlohmann::json& j) {
    Event ev;
    ev.date = j["date"].get<std::string>();
    ev.name = j["name"].get<std::string>();
    ev.priority = j["priority"].get<int>();
    ev.status = j.value("status", "upcoming");
    return ev;
}

#endif