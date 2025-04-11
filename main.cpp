#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

struct Event
{
    string date;
    string name;
    int priority;
    string status = "active";

    bool operator<(const Event &other) const
    {
        return priority > other.priority;
    }
};

struct HashTable
{
    static const int TABLE_SIZE = 31;
    vector<Event> table[TABLE_SIZE];

    int hash(const string &date)
    {

        int day = stoi(date.substr(0, 2));
        int month = stoi(date.substr(3, 2));
        int year = stoi(date.substr(6, 4));

        return ((day * 31 + month) * 31 + year) % TABLE_SIZE;
    }

    void insert(const Event &event)
    {
        int index = hash(event.date);

        for (const auto &e : table[index])
        {
            if (e.date == event.date && e.name == event.name)
            {
                return;
            }
        }
        table[index].push_back(event);
    }

    vector<Event> &get(const string &date)
    {
        return table[hash(date)];
    }

    bool contains(const string &date)
    {
        int index = hash(date);
        for (const auto &e : table[index])
        {
            if (e.date == date)
            {
                return true;
            }
        }
        return false;
    }

    void clear()
    {
        for (int i = 0; i < TABLE_SIZE; i++)
        {
            table[i].clear();
        }
    }
};

HashTable events_map;

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
    if (!events_map.contains(date))
        return;

    for (const auto &event : events_map.get(date))
    {
        string priority_color;
        switch (event.priority)
        {
        case 1:
            priority_color = "\033[1;31m";
            break;
        case 2:
            priority_color = "\033[1;35m";
            break;
        case 3:
            priority_color = "\033[1;34m";
            break;
        case 4:
            priority_color = "\033[1;32m";
            break;
        case 5:
            priority_color = "\033[1;37m";
            break;
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
    int days_in_month = 31;

    int day_counter = 1;
    for (int week = 0; week < 6; week++)
    {

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

            if (events_map.contains(date_str))
            {
                cout << "\033[1;32m" << setw(3) << "[" << day_counter << "]" << "\033[0m";
            }
            else
            {
                cout << setw(4) << day_counter << " ";
            }

            if (events_map.contains(date_str))
            {
                display_day_events(date_str);
            }

            cout << setw(8 - (events_map.contains(date_str) ? 4 : 0)) << " ";
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
        cout << "Enter date (dd/mm/2025) : ";
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

    if (events_map.contains(e.date))
    {
        for (const auto &ev : events_map.get(e.date))
            pq.push(ev);
    }

    pq.push(e);
    events_map.get(e.date).clear();

    while (!pq.empty())
    {
        events_map.get(e.date).push_back(pq.top());
        pq.pop();
    }
}

void edit_event()
{
    string date;
    cout << "Enter event date to edit (dd/mm/2025): ";
    cin >> date;

    if (!events_map.contains(date))
    {
        cout << "\033[1;31mNo events found!\033[0m\n";
        return;
    }

    vector<Event> events = events_map.get(date);
    Event events_array[100];
    int event_count = 0;

    for (const auto &e : events)
    {
        events_array[event_count++] = e;
    }

    cout << "Events on " << date << ":\n";
    for (int i = 0; i < event_count; i++)
    {
        cout << i + 1 << ". " << events_array[i].name
             << " (Priority: " << events_array[i].priority << ")\n";
    }

    int choice;
    cout << "Select event to edit: ";
    cin >> choice;
    choice--;

    if (choice < 0 || choice >= event_count)
    {
        cout << "\033[1;31mInvalid selection!\033[0m\n";
        return;
    }

    cout << "New name (" << events_array[choice].name << "): ";
    cin.ignore();
    string name;
    getline(cin, name);
    if (!name.empty())
        events_array[choice].name = name;

    cout << "New priority (" << events_array[choice].priority << "): ";
    string prio;
    getline(cin, prio);
    if (!prio.empty())
        events_array[choice].priority = stoi(prio);

    events_map.get(date).clear();
    for (int i = 0; i < event_count; i++)
    {
        events_map.get(date).push_back(events_array[i]);
    }
}

void delete_event()
{
    string date;
    cout << "Enter event date to delete from (dd/mm/2025): ";
    cin >> date;

    if (!events_map.contains(date))
    {
        cout << "\033[1;31mNo events found!\033[0m\n";
        return;
    }

    vector<Event> events = events_map.get(date);
    Event events_array[100];
    int event_count = 0;

    for (const auto &e : events)
    {
        events_array[event_count++] = e;
    }

    cout << "Events on " << date << ":\n";
    for (int i = 0; i < event_count; i++)
    {
        cout << i + 1 << ". " << events_array[i].name
             << " (Priority: " << events_array[i].priority << ")\n";
    }

    int choice;
    cout << "Select event to delete (0 to cancel): ";
    cin >> choice;

    if (choice == 0)
    {
        cout << "Deletion cancelled.\n";
        return;
    }

    choice--;

    if (choice < 0 || choice >= event_count)
    {
        cout << "\033[1;31mInvalid selection!\033[0m\n";
        return;
    }

    Event new_events_array[100];
    int new_count = 0;
    for (int i = 0; i < event_count; i++)
    {
        if (i != choice)
        {
            new_events_array[new_count++] = events_array[i];
        }
    }

    events_map.get(date).clear();
    for (int i = 0; i < new_count; i++)
    {
        events_map.get(date).push_back(new_events_array[i]);
    }

    cout << "\033[1;32mEvent deleted successfully!\033[0m\n";
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

    for (int i = 0; i < HashTable::TABLE_SIZE; i++)
    {
        for (const auto &e : events_map.table[i])
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
    current_node = head;
    while (current_node)
    {
        events_map.insert(current_node->event);
        Node *temp = current_node;
        current_node = current_node->next;
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
             << "\033[1;35m[A]\033[0mdd \033[1;33m[E]\033[0mdit "
             << "\033[1;31m[D]\033[0melete \033[1;36m[Q]\033[0muit\n> ";

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
        case 'd':
            delete_event();
            break;
        case 'q':
            return 0;
        default:
            cout << "\033[1;31mInvalid choice!\033[0m\n";
        }
    }
}