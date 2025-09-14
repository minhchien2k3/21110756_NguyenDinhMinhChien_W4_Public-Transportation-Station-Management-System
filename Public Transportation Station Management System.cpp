// File: main.cpp
// Public Transportation Station Management System
// Code identifiers in English. Demonstration + test cases in main().

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <iomanip>

using namespace std;

// Forward declarations
class Vehicle;
class Station;
class Passenger;

// -------------------- Schedule --------------------
struct Schedule {
    shared_ptr<Vehicle> vehicle; // vehicle scheduled
    string time;                 // simple time string (e.g., "09:30")
    bool isArrival;              // true = arrival, false = departure

    Schedule(shared_ptr<Vehicle> v, const string& t, bool arr)
        : vehicle(v), time(t), isArrival(arr) {
    }
};

// -------------------- Vehicle (base) --------------------
class Vehicle {
protected:
    string id;
    string route;
    int capacity;
    double speed; // km/h (default baseline)
    bool onTime;  // true = on-time, false = delayed
    vector<Passenger*> bookedPassengers;
    Station* assignedStation = nullptr;

public:
    Vehicle(const string& id_, const string& route_, int cap_, double speed_)
        : id(id_), route(route_), capacity(cap_), speed(speed_), onTime(true) {
        cout << "[Vehicle created] " << id << " | route: " << route << " | capacity: " << capacity << "\n";
    }

    virtual ~Vehicle() {
        cout << "[Vehicle destroyed] " << id << "\n";
    }

    // Accessors
    string getId() const { return id; }
    string getRoute() const { return route; }
    int getCapacity() const { return capacity; }
    double getSpeed() const { return speed; }
    bool isOnTime() const { return onTime; }

    // Virtual method to allow override in derived classes
    virtual double calculateTravelTime(double distanceKm) const {
        if (speed <= 0) return -1.0;
        return distanceKm / speed; // hours
    }

    virtual void displayInfo() const {
        cout << "Vehicle ID: " << id
            << " | Route: " << route
            << " | Capacity: " << capacity
            << " | Booked: " << bookedPassengers.size()
            << " | Speed: " << speed << " km/h"
            << " | Status: " << (onTime ? "On-time" : "Delayed") << "\n";
    }

    // Booking management
    bool addPassenger(Passenger* p);
    bool removePassenger(Passenger* p);

    // Station assignment
    void setAssignedStation(Station* s) { assignedStation = s; }
    Station* getAssignedStation() const { return assignedStation; }

    void setStatus(bool onTime_) { onTime = onTime_; }
};

// -------------------- ExpressBus (derived) --------------------
class ExpressBus : public Vehicle {
private:
    int stopsCount; // fewer stops for express

public:
    ExpressBus(const string& id_, const string& route_, int cap_, double speed_, int stops_)
        : Vehicle(id_, route_, cap_, speed_), stopsCount(stops_) {
        cout << "[ExpressBus created] " << id << " | stops: " << stopsCount << "\n";
    }

    ~ExpressBus() override {
        cout << "[ExpressBus destroyed] " << id << "\n";
    }

    // Express buses take 20% less time for the same distance
    double calculateTravelTime(double distanceKm) const override {
        double baseTime = Vehicle::calculateTravelTime(distanceKm);
        if (baseTime < 0) return -1.0;
        return baseTime * 0.8; // 20% faster
    }

    void displayInfo() const override {
        cout << "Express ";
        Vehicle::displayInfo();
        cout << "   (stops: " << stopsCount << ")\n";
    }
};

// -------------------- Passenger --------------------
class Passenger {
private:
    string name;
    string id;
    vector<string> bookedVehicleIds;

public:
    Passenger(const string& name_, const string& id_) : name(name_), id(id_) {
        cout << "[Passenger created] " << name << " (" << id << ")\n";
    }

    string getId() const { return id; }
    string getName() const { return name; }

    // Attempts to book ride on vehicle (vehicle handles capacity)
    bool bookRide(shared_ptr<Vehicle> vehicle) {
        if (!vehicle) return false;
        if (vehicle->addPassenger(this)) {
            bookedVehicleIds.push_back(vehicle->getId());
            cout << "[Booked] " << name << " booked " << vehicle->getId() << "\n";
            return true;
        }
        else {
            cout << "[Booking failed] " << name << " could not book " << vehicle->getId() << "\n";
            return false;
        }
    }

    bool cancelRide(shared_ptr<Vehicle> vehicle) {
        if (!vehicle) return false;
        if (vehicle->removePassenger(this)) {
            auto it = find(bookedVehicleIds.begin(), bookedVehicleIds.end(), vehicle->getId());
            if (it != bookedVehicleIds.end()) bookedVehicleIds.erase(it);
            cout << "[Cancelled] " << name << " cancelled " << vehicle->getId() << "\n";
            return true;
        }
        cout << "[Cancel failed] " << name << " not on " << vehicle->getId() << "\n";
        return false;
    }

    void displayInfo() const {
        cout << "Passenger: " << name << " (ID: " << id << ") | Booked: ";
        if (bookedVehicleIds.empty()) cout << "none";
        else {
            for (size_t i = 0; i < bookedVehicleIds.size(); ++i) {
                if (i) cout << ", ";
                cout << bookedVehicleIds[i];
            }
        }
        cout << "\n";
    }
};

// Implement Vehicle passenger methods
bool Vehicle::addPassenger(Passenger* p) {
    if ((int)bookedPassengers.size() >= capacity) {
        cout << "[Vehicle full] " << id << " cannot accept passenger " << p->getName() << "\n";
        return false;
    }
    if (find(bookedPassengers.begin(), bookedPassengers.end(), p) != bookedPassengers.end()) {
        cout << "[Already booked] " << p->getName() << " already on " << id << "\n";
        return false;
    }
    bookedPassengers.push_back(p);
    return true;
}

bool Vehicle::removePassenger(Passenger* p) {
    auto it = find(bookedPassengers.begin(), bookedPassengers.end(), p);
    if (it == bookedPassengers.end()) return false;
    bookedPassengers.erase(it);
    return true;
}

// -------------------- Station --------------------
class Station {
private:
    string name;
    string location;
    string type; // "bus" or "train"
    vector<Schedule> schedules;
    const size_t MAX_SCHEDULES = 10;

public:
    Station(const string& name_, const string& location_, const string& type_)
        : name(name_), location(location_), type(type_) {
        cout << "[Station created] " << name << " (" << type << ") at " << location << "\n";
    }

    ~Station() {
        cout << "[Station destroyed] " << name << "\n";
    }

    // Add schedule; enforce max limit
    bool addSchedule(shared_ptr<Vehicle> v, const string& time, bool isArrival) {
        if (schedules.size() >= MAX_SCHEDULES) {
            cout << "[Schedule limit reached] Station " << name << " cannot accept more schedules.\n";
            return false;
        }
        schedules.emplace_back(v, time, isArrival);
        if (v) v->setAssignedStation(this);
        cout << "[Schedule added] " << (isArrival ? "Arrival" : "Departure")
            << " | Vehicle: " << (v ? v->getId() : string("null"))
            << " | Time: " << time << " at station " << name << "\n";
        return true;
    }

    bool removeScheduleByVehicleId(const string& vehicleId) {
        auto it = find_if(schedules.begin(), schedules.end(), [&](const Schedule& s) {
            return s.vehicle && s.vehicle->getId() == vehicleId;
            });
        if (it == schedules.end()) {
            cout << "[Remove schedule] Vehicle " << vehicleId << " not found at " << name << "\n";
            return false;
        }
        schedules.erase(it);
        cout << "[Schedule removed] Vehicle " << vehicleId << " removed from " << name << "\n";
        return true;
    }

    void displayInfo() const {
        cout << "Station: " << name << " | Location: " << location << " | Type: " << type << "\n";
        if (schedules.empty()) {
            cout << "  No schedules.\n";
            return;
        }
        for (size_t i = 0; i < schedules.size(); ++i) {
            cout << "  [" << i + 1 << "] " << (schedules[i].isArrival ? "Arrival " : "Departure ")
                << "| Vehicle: " << (schedules[i].vehicle ? schedules[i].vehicle->getId() : string("null"))
                << " | Route: " << (schedules[i].vehicle ? schedules[i].vehicle->getRoute() : string("N/A"))
                << " | Time: " << schedules[i].time << "\n";
        }
    }
};

// -------------------- Main / Tests --------------------
int main() {
    cout << "=== Public Transportation Station Management System Demo ===\n\n";

    // Create stations
    Station busStation("Downtown Bus Hub", "12 Main St", "bus");
    Station trainStation("Central Train", "1 Station Rd", "train");

    // Create vehicles
    auto v1 = make_shared<Vehicle>("BUS101", "A->B", 2, 45.0);       // capacity 2 for test
    auto v2 = make_shared<Vehicle>("BUS202", "C->D", 3, 50.0);
    auto exp1 = make_shared<ExpressBus>("EXP301", "X->Y Express", 4, 80.0, 3);

    cout << "\n-- Scheduling tests (max 10 per station) --\n";
    // Add 10 schedules to busStation (should accept)
    for (int i = 0; i < 10; ++i) {
        string t = string("08:") + (i < 10 ? "0" : "") + to_string(10 + i); // <-- fixed
        busStation.addSchedule(v1, t, false);
    }
    // 11th should fail
    busStation.addSchedule(v2, "11:30", true);

    cout << "\n-- Display schedules at busStation --\n";
    busStation.displayInfo();

    cout << "\n-- Booking tests (capacity checks) --\n";
    Passenger pA("Alice", "P100");
    Passenger pB("Bob", "P101");
    Passenger pC("Carol", "P102");

    pA.bookRide(v1); // success
    pB.bookRide(v1); // success
    pC.bookRide(v1); // should fail (full)

    cout << "\n-- Vehicle info after attempted bookings --\n";
    v1->displayInfo();

    cout << "\n-- Cancel and retry booking --\n";
    pB.cancelRide(v1);
    pC.bookRide(v1); // should succeed now

    v1->displayInfo();

    cout << fixed << setprecision(2);
    double distanceKm = 120.0;
    cout << "\n-- Travel time comparison (distance " << distanceKm << " km) --\n";
    cout << "BUS202 time (hrs): " << v2->calculateTravelTime(distanceKm) << "\n";
    cout << "EXP301 time (hrs): " << exp1->calculateTravelTime(distanceKm) << " (20% faster)\n";

    cout << "\n-- Schedule express bus at trainStation --\n";
    trainStation.addSchedule(exp1, "09:45", true);
    trainStation.displayInfo();

    cout << "\n-- Remove schedule example --\n";
    busStation.removeScheduleByVehicleId("BUS101");
    busStation.displayInfo();

    cout << "\n-- Passenger info --\n";
    pA.displayInfo();
    pB.displayInfo();
    pC.displayInfo();

    cout << "\n=== Demo complete ===\n";
    return 0;
}
