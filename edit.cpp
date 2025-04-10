#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "event.h"
#include <nlohmann/json.hpp>

// Function to edit an event by date using arrays (vector)
void editEvent(const std::string &jsonFile) {
    std::string targetDate;
    std::cout << "Enter the date of the event to edit (dd/mm/yyyy): ";
    std::getline(std::cin, targetDate);

    // Load existing JSON data
    nlohmann::json j;
    std::ifstream inFile(jsonFile);
    if (!inFile.good()) {
        std::cerr << "Error reading file.\n";
        return;
    }
    inFile >> j;
    inFile.close();

    if (!j.contains(targetDate) || !j[targetDate].is_array()) {
        std::cout << "No events found on that date.\n";
        return;
    }

    // Copy events into a vector (array)
    std::vector<Event> events;
    for (const auto& eventJson : j[targetDate]) {
        events.push_back(jsonToEvent(eventJson));
    }

    // Display events with indices for selection
    std::cout << "Events on " << targetDate << ":\n";
    for (size_t i = 0; i < events.size(); ++i) {
        std::cout << i << ": " << events[i].name << " (Priority: " << events[i].priority << ")\n";
    }
    std::cout << "Enter the index of the event you want to edit: ";
    std::string idxStr;
    std::getline(std::cin, idxStr);
    size_t index = 0;
    std::istringstream(idxStr) >> index;
    if (index >= events.size()) {
        std::cout << "Invalid index.\n";
        return;
    }

    // Edit selected event
    std::cout << "Enter new event name (or press Enter to keep unchanged): ";
    std::string newName;
    std::getline(std::cin, newName);
    if (!newName.empty())
        events[index].name = newName;

    std::cout << "Enter new priority (1-5) (or press Enter to keep unchanged): ";
    std::string newPriorityStr;
    std::getline(std::cin, newPriorityStr);
    if (!newPriorityStr.empty()) {
        int newPriority;
        std::istringstream(newPriorityStr) >> newPriority;
        if (newPriority >= 1 && newPriority <= 5)
            events[index].priority = newPriority;
        else {
            std::cout << "Invalid priority. Keeping old value.\n";
        }
    }

    // Write updated events array back to JSON structure
    nlohmann::json newArray = nlohmann::json::array();
    for (const auto &ev : events) {
        newArray.push_back(eventToJson(ev));
    }
    j[targetDate] = newArray;

    std::ofstream outFile(jsonFile);
    if (!outFile) {
        std::cerr << "Error writing to file.\n";
        return;
    }
    outFile << j.dump(4);
    outFile.close();
    std::cout << "Event updated successfully!\n";
}
