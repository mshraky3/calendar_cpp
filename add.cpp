#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include "event.h"
#include <nlohmann/json.hpp>
#include <sstream>

// Comparator for our priority queue (highest priority first)
struct CompareEvent {
    bool operator()(const Event &a, const Event &b) {
        return a.priority > b.priority; // lower int means higher priority
    }
};

// Function to add a new event into the JSON file using a priority queue
void addEvent(const std::string &jsonFile) {
    std::string inputDate, inputName;
    int inputPriority;

    std::cout << "Enter event date (dd/mm/yyyy): ";
    std::getline(std::cin, inputDate);
    std::cout << "Enter event name: ";
    std::getline(std::cin, inputName);
    std::cout << "Enter event priority (1-5): ";
    std::string prioStr;
    std::getline(std::cin, prioStr);
    std::istringstream(prioStr) >> inputPriority;

    // Basic input validation
    if (inputPriority < 1 || inputPriority > 5) {
        std::cout << "Invalid priority. Must be between 1 and 5.\n";
        return;
    }

    // Create the new event and set default status
    Event newEvent { inputDate, inputName, inputPriority, "upcoming" };

    // Load existing JSON data
    nlohmann::json j;
    std::ifstream inFile(jsonFile);
    if (inFile.good())
        inFile >> j;
    inFile.close();

    // Ensure there is an array for events on this date
    if (!j.contains(inputDate) || !j[inputDate].is_array())
        j[inputDate] = nlohmann::json::array();

    // Use a priority queue to reorder events on this date based on priority
    std::priority_queue<Event, std::vector<Event>, CompareEvent> pq;
    // Push existing events
    for (const auto& eventJson : j[inputDate]) {
        pq.push(jsonToEvent(eventJson));
    }
    // Add the new event
    pq.push(newEvent);

    // Rebuild the array from the priority queue
    nlohmann::json newArray = nlohmann::json::array();
    while (!pq.empty()) {
        newArray.push_back(eventToJson(pq.top()));
        pq.pop();
    }
    j[inputDate] = newArray;

    // Save back to JSON file
    std::ofstream outFile(jsonFile);
    if (!outFile) {
        std::cerr << "Error writing to file.\n";
        return;
    }
    outFile << j.dump(4);
    outFile.close();
    std::cout << "Event added successfully!\n";
}
