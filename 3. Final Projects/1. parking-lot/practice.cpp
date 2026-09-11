/*
Requirements:
vehicles can enter and exit  iff spot available
multiple floors, each floor have spot for parking
vehicle types supported: currently bike and car (extensible later for more vehicle) // vehicle interface
fee calculation -> can change depending on vehicle or policy -> strategy pattern
payment method for fees -> diff payment (follow ocp principle) -> strategy pattern
ticket is assigned to each vehicle on entering
is this good or anything more i should consider adding in requirement
Also single threaded or concurrent?
any constraint or special rules? like a bike can be parked on car spot?

Entities and relation:
Vehicle(interface) : concrete vehicles(bike, car): id
ParkingSpot : id, isOcucupied, vehicleType
ParkingFloor : have parking spots: composition (parking spots should not exits without floor), check if compatible spot empty , if yes return that spot
ParkingLot have floors : composition
Ticket: id, enter time, exit time, vehicle, spot
ParkingManger : have tickets per vehicle(map for vehicle id to ticket number), create , assign and delete ticket, user pricing stragey and payment methods
PaymentMethod(interface)
PricingMethod(interface)
Spot allocation strategy(interface)
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <ctime>
#include <string>
#include <memory>
#include <utility>
using namespace std;

// enums for vehicle type to avoid naming ambiguity
enum class VehicleType
{
    BIKE = 0, // just standard practice
    CAR
};

enum class PaymentStatus
{
    INPROGRESS = 0, // just standard practice
    SUCCESS,
    FAILED
};

// vehicle interface(abstract class in cpp)
class Vehicle // allows adding new using vehicle while using Vehicle* Liskov substitution principle
{
private:
    VehicleType type;
    int id;

public:
    Vehicle(VehicleType type, int id) : type(type), id(id) {};
    virtual void drive() = 0; // pure virtual function  to make the class abstract so that can't be instantiated4
    VehicleType getVehicleType() const
    {
        return type;
    }
    int getVehicleId() const
    {
        return id;
    }
    virtual ~Vehicle() = default;
};

// concrete vehicles
class Car : public Vehicle
{

public:
    Car(int id) : Vehicle(VehicleType::CAR, id) {}
    void drive() override
    {
        cout << "Driving a car\n";
    }

    ~Car() = default;
};

class Bike : public Vehicle
{

public:
    Bike(int id) : Vehicle(VehicleType::BIKE, id) {}
    void drive() override
    {
        cout << "Driving a bike\n";
    }
    ~Bike() = default;
};

// Parking Spot
class ParkingSpot
{
private:
    int id;
    VehicleType type;
    bool isOccupied; // primary source of truth
    int vehicleId;   // start from 0

public:
    ParkingSpot(int id, VehicleType type, bool isOccupied = false) : id(id), type(type), isOccupied(isOccupied) {}
    bool assignVehicle(int vehicleId)
    {
        isOccupied = true;
        this->vehicleId = vehicleId;
        return true; // signifies assigned successfully
    }
    bool removeVehicle()
    {
        isOccupied = false;
        vehicleId = -1;
        return true; // removed successfully
    }

    VehicleType getSpotType() const
    {
        return type;
    }

    bool isSpotOccupied() const
    {
        return isOccupied == true;
    }
};

// parking floor
class ParkingFloor
{
private:
    int id;
    vector<ParkingSpot *> parkingSpots; // aggregation relation
    // mutex mtx;

public:
    ParkingFloor(int id) : id(id) {}
    bool addParkingSpot(ParkingSpot *spot)
    {
        if (find(parkingSpots.begin(), parkingSpots.end(), spot) == parkingSpots.end())
        {
            parkingSpots.push_back(spot);
        }
        return true;
    }

    bool removeParkingSpot(ParkingSpot *spot)
    {
        parkingSpots.erase(remove(parkingSpots.begin(), parkingSpots.end(), spot), parkingSpots.end());
        return true;
    }

    ParkingSpot *findCompatibleSpot(VehicleType type)
    {
        for (auto spot : parkingSpots)
        {
            if (spot->getSpotType() == type and spot->isSpotOccupied() == false)
            {
                return spot;
            }
        }
        return NULL;
    }

    bool assignSpot(ParkingSpot *spot, int vehicleId)
    {
        // lock_guard<mutex> lock(mtx);
        if (find(parkingSpots.begin(), parkingSpots.end(), spot) != parkingSpots.end() and spot->isSpotOccupied() == false)
        {
            spot->assignVehicle(vehicleId);
            return true;
        }
        return false;
    }

    bool emptySpot(ParkingSpot *spot)
    {
        if (find(parkingSpots.begin(), parkingSpots.end(), spot) != parkingSpots.end())
        {
            spot->removeVehicle();
        }
        return true;
    }

    const int getId() const
    {
        return id;
        return id;
    }

    ~ParkingFloor() = default;
};

// Fee calculation strategies
class IFeeStrategy
{
public:
    virtual double calculateFees(const time_t, const time_t) = 0;
};

class NormalFeeStrategy : public IFeeStrategy
{
    const double BASE_RATE = 10.0;

public:
    double calculateFees(const time_t entry_time, const time_t exit_time) override
    {
        int seconds_passed = difftime(exit_time, entry_time);
        return seconds_passed * BASE_RATE;
    }
};

// Ticket
class Ticket
{
private:
    int id; // starts from 0;
    time_t entry_time;
    time_t exit_time;
    Vehicle *vehicle;
    PaymentStatus paymentStatus;
    ParkingSpot *spot;

public:
    Ticket(int id, Vehicle *vehicle, ParkingSpot *spot) : id(id), vehicle(vehicle), spot(spot)
    {
        entry_time = time(nullptr);
    }
    bool closeTicket(IFeeStrategy *feesStrategy) // pass payment strategy directly here
    {
        // logic for making payment
        exit_time = time(nullptr);
        double fare = feesStrategy->calculateFees(this->entry_time, this->exit_time);
        // make payment for the fare, if success close the ticket
        paymentStatus = PaymentStatus::SUCCESS;
        return true;
    }
    const time_t getExitTime() const
    {
        return exit_time;
    }

    const time_t getEntryTime() const
    {
        return entry_time;
    }
    int getTicketId() const
    {
        return id;
    }
    ParkingSpot *getAssignedSpot() const
    {
        return spot;
    }
};

// Parking lot orchestrator
class ParkingLot
{
private:
    vector<unique_ptr<ParkingFloor>> parkingFloors;
    vector<unique_ptr<Ticket>> tickets;
    int ticketId = 0;

public:
    bool addParkingFloor(unique_ptr<ParkingFloor> parkingFloor)
    {
        parkingFloors.push_back(move(parkingFloor));
        return true;
    }

    bool removeParkingFloor(int floorId)
    {
        auto it = find_if(parkingFloors.begin(), parkingFloors.end(), [floorId](const unique_ptr<ParkingFloor> &floor)
                          { return floor->getId() == floorId; });
        if (it != parkingFloors.end())
        {
            parkingFloors.erase(it);
            return true;
        }
        return false;
    }

    ParkingSpot *getParkingSpot(VehicleType type, int vehicleId)
    {
        for (const auto &parkingFloor : parkingFloors)
        {
            auto spot = parkingFloor->findCompatibleSpot(type);
            if (spot)
            {
                parkingFloor->assignSpot(spot, vehicleId);
                return spot;
            }
        }
        return nullptr;
    }

    string parkVehicle(Vehicle *vehicle)
    {
        ParkingSpot *spot = getParkingSpot(vehicle->getVehicleType(), vehicle->getVehicleId());
        if (!spot)
        {
            return "no parking spot available to park the vehicle";
        }
        ticketId++;
        tickets.push_back(make_unique<Ticket>(ticketId, vehicle, spot));
        return to_string(ticketId);
    }
    string unParkVehicle(int ticketId, IFeeStrategy *pricingStrategy)
    {
        auto it = find_if(tickets.begin(), tickets.end(), [ticketId](const unique_ptr<Ticket> &tkt)
                          { return tkt->getTicketId() == ticketId; });

        if (it == tickets.end())
        {
            return "ticket not found";
        }
        Ticket *ticket = it->get();
        ticket->closeTicket(pricingStrategy);
        ticket->getAssignedSpot()->removeVehicle();
        tickets.erase(it);
        return "Vehicle un-parked successfully";
    }
};

int main()
{
    Vehicle *car1 = new Car(0);
    Vehicle *bike1 = new Bike(1);
    ParkingSpot *spot1 = new ParkingSpot(1, VehicleType::CAR);
    ParkingSpot *spot2 = new ParkingSpot(2, VehicleType::CAR);

    unique_ptr<ParkingFloor> parkingFloor = make_unique<ParkingFloor>(1);
    parkingFloor->addParkingSpot(spot1);
    parkingFloor->addParkingSpot(spot2);
    ParkingLot *parkingLot = new ParkingLot();
    parkingLot->addParkingFloor(move(parkingFloor));

    string ticket_id = parkingLot->parkVehicle(car1);
    int ticketId = stoi(ticket_id);
    cout << ticket_id << "\n";
    string response = parkingLot->parkVehicle(bike1);
    cout << response << "\n";
    return 0;
}