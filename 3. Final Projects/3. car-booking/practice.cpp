#include <iostream>
#include <cmath>
#include <limits>
#include <utility>
#include <memory>
#include <unordered_map>
using namespace std;

enum class DriverStatus
{
    AVAILABLE,
    BUSY
};

enum RideStatus
{
    REQUESTED,
    ONGOING,
    COMPLETED
};

class Location
{
public:
    double x, y;
    Location(double x, double y) : x(x), y(y) {}
    double getDistance(const Location &l) const
    {
        double dx = abs(x - l.x);
        double dy = abs(y - l.y);
        return sqrt(dx * dx + dy * dy);
    }
};

class Rider
{
    string name;
    string id;
    Location location;

public:
    Rider(const string &n, const string &i, const Location &l) : name(n), id(i), location(l) {};
    const string &getName() const
    {
        return name;
    }
    const string &getId() const
    {
        return id;
    }

    const Location &getLocation() const
    {
        return location;
    }

    void updateLocation(const Location &l)
    {
        location = l;
    }
};

class Driver
{
    string name;
    string id;
    Location location;
    DriverStatus status;

public:
    Driver(const string &n, const string &i, const Location &l, DriverStatus s = DriverStatus::AVAILABLE) : name(n), id(i), location(l), status(s) {}
    const string &getId() const
    {
        return id;
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

    bool isAvailable()
    {
        return status == DriverStatus::AVAILABLE;
    }
};

class Ride
{
    string rideId;
    string riderId;
    string driverId;

    Location source;
    Location destination;

    double fare;
    RideStatus status;

public:
    Ride(const string &ri,
         const string &riderId,
         const string &di,
         const Location &s,
         const Location &d)
        : rideId(ri),
          riderId(riderId),
          driverId(di),
          source(s),
          destination(d), fare(0.0),
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

    const double getFare() const
    {
        return fare;
    }
};

class DriverMatchingStrategy
{
public:
    virtual ~DriverMatchingStrategy() = default;
    virtual Driver *findDriver(const Location &location, unordered_map<string, Driver> &drivers) = 0;
};

class NearestDriverMatchingStrategy : public DriverMatchingStrategy
{
public:
    Driver *findDriver(const Location &location, unordered_map<string, Driver> &drivers) override
    {
        double minDistance = numeric_limits<double>::max();
        Driver *selectedDriver = nullptr;
        for (auto &x : drivers)
        {
            Driver &driver = x.second;
            if (!driver.isAvailable())
                continue;

            double dist = location.getDistance(driver.getLocation());
            if (dist < minDistance)
            {
                minDistance = dist;
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
    double BASE_FARE = 50.0;
    double PER_KM_RATE = 12.0;

public:
    double calculateFare(
        const Location &source,
        const Location &destination) override
    {

        double distance = source.getDistance(destination);

        return BASE_FARE + (distance * PER_KM_RATE);
    }
};

class CabBookingSystem
{
    unordered_map<string, Ride> rides;
    unordered_map<string, Rider> riders;
    unordered_map<string, Driver> drivers;

    unique_ptr<DriverMatchingStrategy> matchingStrategy;
    unique_ptr<FareStrategy> fareStrategy;

    int rideCount = 0;

public:
    CabBookingSystem(
        unique_ptr<DriverMatchingStrategy> matchingStrategy,
        unique_ptr<FareStrategy> fareStrategy)
        : matchingStrategy(move(matchingStrategy)),
          fareStrategy(move(fareStrategy)) {}

    void addRider(const string &id, const string &name, const Location &location)
    {
        riders.emplace(id, Rider(id, name, location));
    }

    void addDriver(const string &id, const string &name, const Location &location)
    {
        drivers.emplace(id, Driver(id, name, location));
    }

    string requestRide(const string &riderId, const Location &destination)
    {
        if (riders.count(riderId) == 0)
        {
            throw runtime_error("invalid rider id");
        }

        Rider &rider = riders.at(riderId); // dont use riders[riderId], with a default constructor Rider()= default in the respective class

        Driver *driver = matchingStrategy->findDriver(destination, drivers);

        if (driver == nullptr)
        {
            throw runtime_error("No Driver available");
        }

        const string rideId = "Ride_Id_" + to_string(rideCount++);

        Ride ride = Ride(rideId, riderId, driver->getId(), rider.getLocation(), destination);

        ride.setStatus(RideStatus::ONGOING);

        double fare = fareStrategy->calculateFare(rider.getLocation(), destination);
        ride.setFare(fare);

        driver->setStatus(DriverStatus::BUSY);

        rides.emplace(rideId, ride);
        return rideId;
    }

    // free up driver and mark ride completed
    void completeRide(const string &rideId)
    {
        auto rideIt = rides.find(rideId);
        if (rideIt == rides.end())
        {
            throw runtime_error("Ride not found");
        }

        Ride &ride = rideIt->second;

        ride.setStatus(RideStatus::COMPLETED);

        auto driverIt = drivers.find(ride.getDriverId());
        if (driverIt != drivers.end())
        {
            driverIt->second.setStatus(DriverStatus::AVAILABLE);
        }
    }

    void printRide(const string &rideId)
    {
        auto rideIt = rides.find(rideId);
        if (rideIt == rides.end())
        {
            cout << "Ride not found.\n";
        }

        Ride &ride = rideIt->second;
        cout << "\nRide Details\n";
        cout << "Ride Id : " << ride.getRideId() << '\n';
        cout << "Rider   : " << ride.getRiderId() << '\n';
        cout << "Driver  : " << ride.getDriverId() << '\n';
        cout << "Fare    : " << ride.getFare() << '\n';
        cout << "Ride status    : " << ride.getStatus() << '\n';
    }
};

int main()
{
    CabBookingSystem sys = CabBookingSystem(
        make_unique<NearestDriverMatchingStrategy>(),
        make_unique<DefaultFareStrategy>());

    sys.addRider("r1", "Ram", Location(0.3, 23));
    sys.addRider("r2", "Shyam", Location(2, 23));

    sys.addDriver("d1", "Rahul", Location(2, 4));
    sys.addDriver("d2", "Kunal", Location(5, 4.1));

    string rideId = sys.requestRide("r1", Location(3, 4));
    sys.printRide(rideId);

    sys.completeRide(rideId);

    sys.printRide(rideId);

    return 0;
}