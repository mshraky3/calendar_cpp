#include <iostream>
#include <fstream>
#include <ctime>
#include "event.h"
#include <nlohmann/json.hpp>

// Simple linked list node for Event
struct EventNode {
    Event data;
    EventNode* next;
};

// Helper function to insert Event into linked list
EventNode* insertEvent(EventNode* head, const Event& ev) {
    EventNode* newNode = new EventNode { ev, nullptr };
    if (!head)
        return newNode;
    EventNode* curr = head;
    while (curr->next) {
        curr = curr->next;
    }
    curr->next = newNode;
    return head;
}

// Dummy function to compare event date with current date (yyyy mm dd comparison)
// In production, implement robust date parsing
bool isPastEvent(const std::string& eventDate) {
    // Expected eventDate format: dd/mm/yyyy
    int day, month, year;
    sscanf(eventDate.c_str(), "%d/%d/%d", &day, &month, &year);

    // Get current date
    time_t now = time(0);
    tm *ltm = localtime(&now);
    int c_year = 1900 + ltm->tm_year;
    int c_month = 1 + ltm->tm_mon;
    int c_day = ltm->tm_mday;

    if (year < c_year) return true;
    if (year == c_year) {
        if (month < c_month) return true;
        if (month == c_month && day < c_day) return true;
    }
    return false;
}

// Function to clean up events (mark passed events) using a linked list
void cleanupEvents(const std::string &jsonFile) {
    nlohmann::json j;
    std::ifstream inFile(jsonFile);
    if (!inFile.good()) {
        std::cerr << "Error reading file for cleanup.\n";
        return;
    }
    inFile >> j;
    inFile.close();

    // For each date in JSON, build a linked list of events and mark passed events
    for (auto& dateEntry : j.items()) {
        // Create linked list head pointer
        EventNode* head = nullptr;
        for (const auto& eventJson : dateEntry.value()) {
            Event ev = jsonToEvent(eventJson);
            head = insertEvent(head, ev);
        }
        // Traverse the linked list and update events if they are in the past
        EventNode* current = head;
        while (current) {
            if (isPastEvent(current->data.date))
                current->data.status = "passed";
            current = current->next;
        }
        // Rebuild the JSON array from the linked list
        nlohmann::json newArray = nlohmann::json::array();
        current = head;
        while (current) {
            newArray.push_back(eventToJson(current->data));
            EventNode* temp = current;
            current = current->next;
            delete temp;
        }
        j[dateEntry.key()] = newArray;
    }

    // Save changes to JSON file
    std::ofstream outFile(jsonFile);
    if (!outFile) {
        std::cerr << "Error writing updated cleanup info to file.\n";
        return;
    }
    outFile << j.dump(4);
    outFile.close();
    std::cout << "Cleanup complete: Past events marked as passed.\n";
}
