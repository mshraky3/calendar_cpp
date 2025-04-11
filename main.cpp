#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <list>
#include <ctime>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <functional>

using namespace std;

struct Event
{
    string date;
    string name;
    int priority;
    string status = "active";

    bool operator<(const Event &other) const
    {
        return priority > other.priority; // Min-heap behavior
    }
};

unordered_map<string, vector<Event>> events_map;

// Helper functions
bool validate_date(const string &date)
{
    if (date.size() != 10 || date[2] != '/' || date[5] != '/')
        return false;

    try
    {
        int day = stoi(date.substr(0, 2));
        int month = stoi(date.substr(3, 2));
        int year = stoi(date.substr(6, 4));

        return (year == 2025) && (month >= 1 && month <= 12) &&
               (day >= 1 && day <= 31);
    }
    catch (...)
    {
        return false;
    }
}

string get_month_name(int month)
{
    string names[] = {"January", "February", "March", "April", "May", "June",
                      "July", "August", "September", "October", "November", "December"};
    return names[month - 1];
}

void display_header(int month, int year)
{
    cout << "\033[1;36m" << string(50, '=') << "\n"
         << "                       " << get_month_name(month) << " " << year << "\n"
         << string(50, '=') << "\033[0m\n";
}

void display_day_events(const string &date)
{
    if (!events_map.count(date))
        return;

    for (const auto &event : events_map[date])
    {
        string priority_color;
        switch (event.priority)
        {
        case 1:
            priority_color = "\033[1;31m";
            break; // Red for high priority
        case 2:
            priority_color = "\033[1;35m";
            break; // Magenta
        case 3:
            priority_color = "\033[1;34m";
            break; // Blue
        case 4:
            priority_color = "\033[1;32m";
            break; // Green
        case 5:
            priority_color = "\033[1;37m";
            break; // White for low priority
        }
        cout << priority_color << "  * " << event.name << "\033[0m";
    }
}

void display_calendar(int month, int year)
{
    display_header(month, year);

    vector<string> days = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
    for (const auto &d : days)
        cout << "\033[1;33m" << setw(8) << d << "\033[0m";
    cout << "\n";

    tm time_in = {0, 0, 0, 1, month - 1, year - 1900};
    mktime(&time_in);
    int start_day = time_in.tm_wday;
    int days_in_month = 31; // Simplified

    int day_counter = 1;
    for (int week = 0; week < 6; week++)
    {
        // Display day numbers and events
        for (int day = 0; day < 7; day++)
        {
            if ((week == 0 && day < start_day) || day_counter > days_in_month)
            {
                cout << setw(8) << " ";
                continue;
            }

            stringstream date;
            date << setw(2) << setfill('0') << day_counter << "/"
                 << setw(2) << setfill('0') << month << "/2025";
            string date_str = date.str();

            if (events_map.count(date_str))
            {
                cout << "\033[1;32m" << setw(3) << "[" << day_counter << "]" << "\033[0m";
            }
            else
            {
                cout << setw(4) << day_counter << " ";
            }

            // Display events for this day
            if (events_map.count(date_str))
            {
                display_day_events(date_str);
            }

            cout << setw(8 - (events_map.count(date_str) ? 4 : 0)) << " ";
            day_counter++;
        }
        cout << "\n";
    }
}

void add_event()
{
    Event e;

    while (true)
    {
        cout << "Enter date (dd/mm/2025): ";
        cin >> e.date;
        if (validate_date(e.date))
            break;
        cout << "\033[1;31mInvalid date!\033[0m ";
    }

    cout << "Event name: ";
    cin.ignore();
    getline(cin, e.name);

    while (true)
    {
        cout << "Priority (1-5, 1=highest): ";
        cin >> e.priority;
        if (e.priority >= 1 && e.priority <= 5)
            break;
        cout << "\033[1;31mInvalid priority!\033[0m ";
    }

    priority_queue<Event> pq;

    if (events_map.count(e.date))
    {
        for (const auto &ev : events_map[e.date])
            pq.push(ev);
    }

    pq.push(e);
    events_map[e.date].clear();

    while (!pq.empty())
    {
        events_map[e.date].push_back(pq.top());
        pq.pop();
    }
}

void edit_event()
{
    string date;
    cout << "Enter event date to edit (dd/mm/2025): ";
    cin >> date;

    if (!events_map.count(date))
    {
        cout << "\033[1;31mNo events found!\033[0m\n";
        return;
    }

    cout << "Events on " << date << ":\n";
    for (int i = 0; i < events_map[date].size(); i++)
    {
        cout << i + 1 << ". " << events_map[date][i].name
             << " (Priority: " << events_map[date][i].priority << ")\n";
    }

    int choice;
    cout << "Select event to edit: ";
    cin >> choice;
    choice--;

    if (choice < 0 || choice >= events_map[date].size())
    {
        cout << "\033[1;31mInvalid selection!\033[0m\n";
        return;
    }

    cout << "New name (" << events_map[date][choice].name << "): ";
    cin.ignore();
    string name;
    getline(cin, name);
    if (!name.empty())
        events_map[date][choice].name = name;

    cout << "New priority (" << events_map[date][choice].priority << "): ";
    string prio;
    getline(cin, prio);
    if (!prio.empty())
        events_map[date][choice].priority = stoi(prio);

    sort(events_map[date].begin(), events_map[date].end(),
         [](const Event &a, const Event &b)
         { return a.priority < b.priority; });
}

void cleanup_events()
{
    time_t now = time(nullptr);
    tm *current = localtime(&now);

    struct Node
    {
        Event event;
        Node *next;
    } *head = nullptr, *tail = nullptr;

    for (auto &date_events : events_map)
    {
        for (auto &e : date_events.second)
        {
            Node *newNode = new Node{e};
            if (!head)
                head = tail = newNode;
            else
            {
                tail->next = newNode;
                tail = newNode;
            }
        }
    }

    Node *current_node = head;
    while (current_node)
    {
        string date = current_node->event.date;
        int day = stoi(date.substr(0, 2));
        int month = stoi(date.substr(3, 2));
        int year = stoi(date.substr(6, 4));

        if (year < 2025 || (year == 2025 && month < current->tm_mon + 1) ||
            (year == 2025 && month == current->tm_mon + 1 && day < current->tm_mday))
        {
            current_node->event.status = "expired";
        }

        current_node = current_node->next;
    }

    events_map.clear();
    while (head)
    {
        events_map[head->event.date].push_back(head->event);
        Node *temp = head;
        head = head->next;
        delete temp;
    }
}

int main()
{
    int current_month = 1;
    int current_year = 2025;

    while (true)
    {
        system("clear || cls");
        cleanup_events();
        display_calendar(current_month, current_year);

        cout << "\nOptions:\n"
             << "\033[1;32m[N]\033[0mext \033[1;34m[P]\033[0mprev "
             << "\033[1;35m[A]\033[0mdd \033[1;33m[E]\033[0mdit \033[1;31m[Q]\033[0muit\n> ";

        char choice;
        cin >> choice;

        switch (tolower(choice))
        {
        case 'n':
            current_month = (current_month % 12) + 1;
            break;
        case 'p':
            current_month = (current_month == 1) ? 12 : current_month - 1;
            break;
        case 'a':
            add_event();
            break;
        case 'e':
            edit_event();
            break;
        case 'q':
            return 0;
        default:
            cout << "\033[1;31mInvalid choice!\033[0m\n";
        }
    }
}