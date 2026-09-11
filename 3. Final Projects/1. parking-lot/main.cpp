#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory> // for mutex
#include <algorithm>
#include <mutex>
#include <unistd.h> //for sleep()
#include <math.h>
#include <optional> //for optional
#include <ctime>    //for time_t

/*
Entities → stable ->  Vehicle, ParkingSpot, ParkingFloor, Ticket
Strategies → changeable -> payment strategy, pricing strategy, These use interfaces / polymorphism
Orchestrators → flow -> Parking Lot

Locking Level:
parkingLot: Global, all floors locked at once
ParkingFloor: Floor locking, balanced, each floor can work independently
ParkingSpot: Granular, highest lock overhead, overkill. Usefull only when action of parking takes a very long time , in that locking whole dont make sense

*/

using namespace std;

enum class VehicleType
{
    BIKE,
    CAR,
    TRUCK
};

enum class SpotType
{
    BIKE,
    CAR,
    TRUCK
};

enum class PaymentStatus
{
    PENDING,
    SUCCESS,
    FAILED
};

// “Vehicle is a simple data object. It doesn’t know about pricing or parking duration.”
class Vehicle
{
private:
    string vehicleId;
    VehicleType type;

public:
    Vehicle(const string &vid, VehicleType t) : vehicleId(vid), type(t) {}

    VehicleType getType() const
    {
        return type;
    }

    string getVehicleId() const
    {
        return vehicleId;
    }
};

class VehicleFactory
{
public:
    static shared_ptr<Vehicle> createVehicle(
        const string &id,
        VehicleType type)
    {
        return make_shared<Vehicle>(id, type);
    }
};

class ParkingSpot
{
protected:
    string spotId;
    SpotType type;
    bool occupied;
    shared_ptr<Vehicle> vehicle;

public:
    ParkingSpot(string id, SpotType type)
        : spotId(id), type(type), occupied(false) {}

    virtual ~ParkingSpot() = default;

    bool canFitVehicle(VehicleType vtype) const
    {
        return !occupied and static_cast<int>(type) >= static_cast<int>(vtype);
    }

    void parkVehicle(shared_ptr<Vehicle> v)
    {
        occupied = true;
        vehicle = v;
    }

    void removeVehicle()
    {
        occupied = false;
        vehicle.reset();
    }

    bool isOccupied() const
    {
        return occupied;
    }
};

// “Factory isolates object creation and avoids scattering new logic.”
class ParkingSpotFactory
{
public:
    static unique_ptr<ParkingSpot> createSpot(const string &id, SpotType type)
    {
        return make_unique<ParkingSpot>(id, type);
    }
};

class ParkingFloor
{
private:
    int floorNumber;
    // it ows the parking spots, clear ownership defines clear lifecycle management
    vector<unique_ptr<ParkingSpot>> spots;

public:
    ParkingFloor(int number) : floorNumber(number) {}
    void addSpot(unique_ptr<ParkingSpot> spot)
    {
        spots.push_back(move(spot));
    }
    ParkingSpot *findAvailableSpot(VehicleType type)
    {
        for (auto &spot : spots)
        {
            if (spot->canFitVehicle(type))
            {
                // When you call .get(), you are asking the smart pointer for the address of the resource it owns without transferring ownership
                return spot.get(); // return spot not allowed, as copy. return move(spot) will remove spot from spots vector
            }
        }
        return nullptr;
    }
};

class Ticket
{
private:
    string ticketId;
    shared_ptr<Vehicle> vehicle;

    /*
     Why NOT shared_ptr<ParkingSpot>?
        Ticket co-owns the spot ❌
        Spot might live longer than floor ❌
        Lifecycle becomes unclear ❌
    */
    ParkingSpot *spot;
    time_t entryTime;
    time_t exitTime;

public:
    Ticket(string id, shared_ptr<Vehicle> v, ParkingSpot *s)
        : ticketId(id), vehicle(v), spot(s)
    {
        entryTime = time(nullptr); // from ctime, returns current time in seconds since epoch
    }

    void closeTicket()
    {
        exitTime = time(nullptr);
    }
    time_t getEntryTime() const
    {
        return entryTime;
    }

    ParkingSpot *getParkingSpot() const
    {
        return spot;
    }

    time_t getExitTime() const
    {
        return exitTime;
    }

    VehicleType getVehicleType() const
    {
        return vehicle->getType();
    }

    // no destructor to delete spot, as spot is owned by ParkingFloor
};

// Pricing Strategies
class PricingStrategy
{
public:
    virtual double calculatePrice(time_t entry, time_t exit, VehicleType type) = 0;
    virtual ~PricingStrategy() = default;
};

// isolated and extensible
class HourlyPricingStrategy : public PricingStrategy
{
public:
    double calculatePrice(time_t entry, time_t exit, VehicleType type) override
    {
        double hours = difftime(exit, entry) / 3600.0;
        double rate = 20.0;
        if (type == VehicleType::TRUCK)
            rate = 40;
        else if (type == VehicleType::BIKE)
            rate = 10;
        return ceil(hours) * rate;
    }
};

// Payment Strategy
class PaymentMethod
{
public:
    virtual bool pay(double amount) = 0;
    virtual ~PaymentMethod() = default;
};

class UPIPayment : public PaymentMethod
{
public:
    bool pay(double amount) override
    {
        cout << "Paid " << amount << " via UPI\n";
        return true;
    }
};

// ParkingLot coordinates, doesn’t own logic, concurrency handel via mutex
class ParkingLot
{
private:
    vector<unique_ptr<ParkingFloor>> floors;
    unique_ptr<PricingStrategy> pricingStrategy;

    // ParkingLot OWNS all active tickets
    unordered_map<string, unique_ptr<Ticket>> activeTickets; // so that tickets don't outlive parkingLot
    int ticketNumber = 1;

    // “If contention becomes high, we can move locking to floor-level(ParkingFloor) instead of locking the entire parking lot.”
    // mutex mtx;

public:
    ParkingLot(unique_ptr<PricingStrategy> strategy)
        : pricingStrategy(move(strategy)) {}

    void addFloor(unique_ptr<ParkingFloor> floor)
    {
        floors.push_back(move(floor));
    }

    // ✅ Returns ticketId, not Ticket*
    optional<string> parkVehicle(shared_ptr<Vehicle> vehicle)
    {
        // lock_guard<mutex> lock(mtx);

        for (auto &floor : floors)
        {
            ParkingSpot *spot =
                floor->findAvailableSpot(vehicle->getType());

            if (spot)
            {
                spot->parkVehicle(vehicle);

                string ticketId = "TICKET_" + to_string(ticketNumber++);
                activeTickets[ticketId] =
                    make_unique<Ticket>(ticketId, vehicle, spot);

                return ticketId;
            }
        }
        return nullopt; // parking lot full, Represents "Nothing"
    }

    double unparkVehicle(const string &ticketId,
                         PaymentMethod &payment)
    {
        lock_guard<mutex> lock(mtx);

        auto it = activeTickets.find(ticketId);
        if (it == activeTickets.end())
        {
            return 0.0; // invalid ticket
        }

        // Extract ownership
        unique_ptr<Ticket> ticket = move(it->second); // because unique_ptr can't be copied, we move it to local variable, now ticket owns the Ticket object
        activeTickets.erase(it);

        // Close ticket
        ticket->closeTicket();

        // 🔥 CRUCIAL: free the spot
        ticket->getParkingSpot()->removeVehicle();

        // Calculate price
        double amount = pricingStrategy->calculatePrice(
            ticket->getEntryTime(),
            ticket->getExitTime(),
            ticket->getVehicleType());

        payment.pay(amount);

        // ticket destroyed here automatically
        return amount;
    }
};

int main()
{

    // Use unique_ptr when:
    // There is exactly ONE owner
    // 🔹 Use shared_ptr when:
    // Multiple objects need to refer to it safely

    // 1. create parking lot with floors, spots, and pricing strategy.
    // 2. create vehicle, give entry via ticket, make exit via delete ticket
    // Pricing strategy
    unique_ptr<PricingStrategy> pricing = make_unique<HourlyPricingStrategy>();

    // Parking Lot
    ParkingLot parkingLot(move(pricing));

    // Parking Floor
    unique_ptr<ParkingFloor> floor1 = make_unique<ParkingFloor>(1);
    floor1->addSpot(ParkingSpotFactory::createSpot("S1", SpotType::CAR));
    floor1->addSpot(ParkingSpotFactory::createSpot("S2", SpotType::BIKE));
    floor1->addSpot(ParkingSpotFactory::createSpot("S3", SpotType::TRUCK));
    floor1->addSpot(make_unique<ParkingSpot>("S4", SpotType::TRUCK));

    parkingLot.addFloor(move(floor1));

    auto car = VehicleFactory::createVehicle("Car_101", VehicleType::CAR);
    string ticket = parkingLot.parkVehicle(car);
    if (!ticket.had_value())
    {
        cout << "Parking lot is full\n";
        return 0;
    }

    // simulate sleep
    sleep(2);

    UPIPayment payment;
    double amount = parkingLot.unparkVehicle(ticket, payment);
    cout << "Parking fee: " << amount << "\n";
    return 0;
}