#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <unordered_map>
using namespace std;

enum class DriverStatus
{
    AVAILABLE,
    BUSY
};

enum class RideStatus
{
    REQUESTED,
    ONGOING,
    COMPLETED
};

class Location
{
public:
    double x; // coordinates
    double y;
    Location(double x, double y) : x(x), y(y) {}
    double distanceTo(const Location &other) const
    {
        double dx = x - other.x;
        double dy = y - other.y;
        return sqrt(dx * dx + dy * dy);
    }
};

class Rider
{
    string riderId;
    string name;
    Location location;

public:
    Rider(const string &riderId, const string &name, const Location &location) : riderId(riderId), name(name), location(location) {}
    const string &getId() const
    {
        return riderId;
    }

    const string &getName() const
    {
        return name;
    }

    const Location &getLocation() const
    {
        return location;
    }

    void updateLocation(const Location &newLocation)
    {
        location = newLocation;
    }
};

class Driver
{
private:
    string driverId;
    string name;
    Location location;
    DriverStatus status;

public:
    Driver(string driverId,
           string name,
           const Location &location)
        : driverId(move(driverId)),
          name(move(name)),
          location(location),
          status(DriverStatus::AVAILABLE) {}
    const string &getId() const
    {
        return driverId;
    }

    const string &getName() const
    {
        return name;
    }

    const Location &getLocation() const
    {
        return location;
    }

    DriverStatus getStatus() const
    {
        return status;
    }

    void updateLocation(const Location &newLocation)
    {
        location = newLocation;
    }

    void setStatus(DriverStatus newStatus)
    {
        status = newStatus;
    }

    bool isAvailable() const
    {
        return status == DriverStatus::AVAILABLE;
    }
};

class Ride
{
private:
    string rideId;
    string riderId;
    string driverId;

    Location source;
    Location destination;

    double fare;
    RideStatus status;

public:
    Ride(string rideId,
         string riderId,
         string driverId,
         const Location &source,
         const Location &destination)
        : rideId(move(rideId)),
          riderId(move(riderId)),
          driverId(move(driverId)),
          source(source),
          destination(destination),
          fare(0.0),
          status(RideStatus::REQUESTED) {}

    const string &getRideId() const
    {
        return rideId;
    }

    const string &getRiderId() const
    {
        return riderId;
    }

    const string &getDriverId() const
    {
        return driverId;
    }

    RideStatus getStatus() const
    {
        return status;
    }

    void setStatus(RideStatus newStatus)
    {
        status = newStatus;
    }

    const Location &getSource() const
    {
        return source;
    }

    const Location &getDestination() const
    {
        return destination;
    }

    void setFare(double newFare)
    {
        fare = newFare;
    }

    double getFare() const
    {
        return fare;
    }
};

class DriverMatchingStrategy
{
public:
    virtual ~DriverMatchingStrategy() = default;

    virtual Driver *findDriver(
        const Location &pickupLocation,
        unordered_map<string, Driver> &drivers) = 0;
};

class NearestDriverMatchingStrategy
    : public DriverMatchingStrategy
{
public:
    Driver *findDriver(
        const Location &pickupLocation,
        unordered_map<string, Driver> &drivers) override
    {

        Driver *selectedDriver = nullptr;
        double minDistance = numeric_limits<double>::max();

        for (auto &d : drivers)
        {
            auto id = d.first;
            Driver &driver = d.second; // here not using & will cause error: bad_alloc because will create a temporary copy for driver
            if (!driver.isAvailable())
            {
                continue;
            }

            double distance =
                driver.getLocation().distanceTo(pickupLocation); // since location is const, A const object can only call const member functions.

            if (distance < minDistance)
            {
                minDistance = distance;
                selectedDriver = &driver;
            }
        }

        return selectedDriver;
    }
};

class FareStrategy
{
public:
    virtual ~FareStrategy() = default;

    virtual double calculateFare(
        const Location &source,
        const Location &destination) = 0;
};

class DefaultFareStrategy : public FareStrategy
{
private:
    static constexpr double BASE_FARE = 50.0;
    static constexpr double PER_KM_RATE = 12.0;

public:
    double calculateFare(
        const Location &source,
        const Location &destination) override
    {

        double distance = source.distanceTo(destination);

        return BASE_FARE + (distance * PER_KM_RATE);
    }
};

class CabBookingSystem
{
private:
    unordered_map<string, Rider> riders;
    unordered_map<string, Driver> drivers;
    unordered_map<string, Ride> rides;

    unique_ptr<DriverMatchingStrategy> matchingStrategy; // strategies must be pointers to support polymorphism
    unique_ptr<FareStrategy> fareStrategy;

    int rideCounter = 1;

public:
    CabBookingSystem(
        unique_ptr<DriverMatchingStrategy> matchingStrategy,
        unique_ptr<FareStrategy> fareStrategy)
        : matchingStrategy(move(matchingStrategy)),
          fareStrategy(move(fareStrategy)) {}

    void addRider(
        const string &riderId,
        const string &name,
        const Location &location)
    {

        riders.emplace( // emplace prevents rewriting old value if key already exists
            riderId,
            Rider(riderId, name, location));
    }

    void addDriver(
        const string &driverId,
        const string &name,
        const Location &location)
    {

        drivers.emplace(
            driverId,
            Driver(driverId, name, location));
    }

    string requestRide(
        const string &riderId,
        const Location &destination)
    {

        auto riderIt = riders.find(riderId);

        if (riderIt == riders.end())
        {
            throw runtime_error("Rider not found");
        }

        Rider &rider = riderIt->second;

        Driver *driver =
            matchingStrategy->findDriver(
                rider.getLocation(),
                drivers);

        if (!driver)
        {
            throw runtime_error(
                "No available drivers");
        }

        string rideId =
            "RIDE_" + to_string(rideCounter++);

        Ride ride(
            rideId,
            riderId,
            driver->getId(),
            rider.getLocation(),
            destination);

        ride.setStatus(RideStatus::ONGOING);

        double fare =
            fareStrategy->calculateFare(
                rider.getLocation(),
                destination);

        ride.setFare(fare);

        driver->setStatus(DriverStatus::BUSY);

        rides.emplace(rideId, move(ride));

        return rideId;
    }

    void completeRide(const string &rideId)
    {
        auto rideIt = rides.find(rideId);

        if (rideIt == rides.end())
        {
            throw runtime_error("Ride not found");
        }

        Ride &ride = rideIt->second;

        if (ride.getStatus() == RideStatus::COMPLETED)
        {
            return;
        }

        ride.setStatus(RideStatus::COMPLETED);

        auto driverIt =
            drivers.find(ride.getDriverId());

        if (driverIt != drivers.end())
        {
            driverIt->second.setStatus(
                DriverStatus::AVAILABLE);
        }
    }

    void printRide(const string &rideId)
    {
        auto it = rides.find(rideId);

        if (it == rides.end())
        {
            return;
        }

        const Ride &ride = it->second;

        cout << "\nRide Details\n";
        cout << "Ride Id : " << ride.getRideId() << '\n';
        cout << "Rider   : " << ride.getRiderId() << '\n';
        cout << "Driver  : " << ride.getDriverId() << '\n';
        cout << "Fare    : " << ride.getFare() << '\n';
    }
};

int main()
{

    CabBookingSystem cabSystem(
        make_unique<NearestDriverMatchingStrategy>(),
        make_unique<DefaultFareStrategy>());

    cabSystem.addRider(
        "R1",
        "Aman",
        Location(0, 0));

    cabSystem.addDriver(
        "D1",
        "Rahul",
        Location(1, 1));

    cabSystem.addDriver(
        "D2",
        "Karan",
        Location(10, 10));

    string rideId =
        cabSystem.requestRide(
            "R1",
            Location(5, 5));

    cabSystem.printRide(rideId);

    cabSystem.completeRide(rideId);

    return 0;
}