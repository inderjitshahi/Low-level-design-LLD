#include <iostream>
#include <cmath>
#include <unordered_map>
#include <memory>
#include <utility>
#include <limits>
using namespace std;

enum class RideStatus
{
    REQUESTED = 0,
    ONGOING,
    COMPLETED
};

enum class DriverStatus
{
    AVAILABLE = 0,
    BUSY
};

class Location
{
private:
    double x, y; // coordinates

public:
    Location(double x, double y) : x(x), y(y) {}

    const double getX() const
    {
        return x;
    }

    const double getY() const
    {
        return y;
    }

    double getDistance(const Location &l) const
    {
        double dx = abs(x - l.getX());
        double dy = abs(y - l.getY());

        return sqrt(dx * dx + dy * dy);
    }
};

class Rider
{
    string id;
    string name;
    Location location;

public:
    Rider(const string &id,
          const string &name,
          const Location &location)
        : id(id),
          name(name),
          location(location) {}

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
};

class Driver
{
private:
    string id;
    string name;
    Location location;
    DriverStatus status;

public:
    Driver(const string &id, const string &name, const Location &location) : id(id), name(name), location(location), status(DriverStatus::AVAILABLE) {}

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

    void setStatus(DriverStatus s)
    {
        status = s;
    }

    DriverStatus getStatus() const
    {
        return status;
    }
};

class Ride
{
    string rideId;
    string riderId;
    string driverId;

    Location source, destination;

    double fare;

    RideStatus status;

public:
    Ride(const string &rideId,
         const string &riderId,
         const string &driverId,
         const Location &source,
         const Location &destination)
        : rideId(rideId),
          riderId(riderId),
          driverId(driverId),
          source(source),
          destination(destination),
          fare(0.0),
          status(RideStatus::REQUESTED) {}

    const string &getId() const
    {
        return rideId;
    }
    const string &getDriverId() const
    {
        return driverId;
    }
    const string &getRiderId() const
    {
        return riderId;
    }

    const Location &getSource() const
    {
        return source;
    }
    const Location &getDestination() const
    {
        return destination;
    }

    RideStatus getStatus() const
    {
        return status;
    }

    void setFare(double f)
    {
        fare = f;
    }

    void setStatus(RideStatus s)
    {
        status = s;
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
        Driver *selectedDriver = nullptr;
        double minDist = numeric_limits<double>::max();

        for (auto &x : drivers)
        {
            Driver &driver = x.second;

            double dist = driver.getLocation().getDistance(location);
            if (dist < minDist)
            {
                minDist = dist;
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
    virtual double calculateFare(const Location &s, const Location &d) = 0;
};

class NormalFareStrategy : public FareStrategy
{
private:
    double base_fare = 100.0;
    double per_km_fare = 10.2;

public:
    double calculateFare(const Location &s, const Location &d) override
    {
        double dist = s.getDistance(d);
        return base_fare + per_km_fare * dist;
    }
};

class CabBookingSystem
{
private:
    unordered_map<string, Rider> riders;
    unordered_map<string, Driver> drivers;
    unordered_map<string, Ride> rides;

    int rideCount = 0;

    unique_ptr<DriverMatchingStrategy> matchingStrategy;
    unique_ptr<FareStrategy> fareStrategy;

public:
    CabBookingSystem(unique_ptr<DriverMatchingStrategy> matchingStrategy,
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
        auto riderIt = riders.find(riderId);
        if (riderIt == riders.end())
        {
            throw runtime_error("rider not found");
        }

        Rider &rider = riderIt->second;

        Driver *driver = matchingStrategy->findDriver(destination, drivers);
        if (driver == nullptr)
        {
            throw runtime_error("no driver found");
        }

        string rideId = "Ride_Id_" + to_string(rideCount++);

        Ride ride = Ride(rideId, riderId, driver->getId(), rider.getLocation(), destination);

        double fare = fareStrategy->calculateFare(rider.getLocation(), destination);
        ride.setFare(fare);

        ride.setStatus(RideStatus::ONGOING);
        rides.emplace(rideId, ride);

        return rideId;
    }

    void completeRide(const string &rideId)
    {
        auto rideIt = rides.find(rideId);
        if (rideIt == rides.end())
        {
            cout << "Ride not found\n";
            return;
        }
        Ride &ride = rideIt->second;

        auto driverIt = drivers.find(ride.getDriverId());
        if (driverIt != drivers.end())
        {
            driverIt->second.setStatus(DriverStatus::AVAILABLE);
        }
        ride.setStatus(RideStatus::COMPLETED);
        cout << "Ride completed successfully!\n";
    }
    void printRide(const string &rideId)
    {
        auto rideIt = rides.find(rideId);
        if (rideIt == rides.end())
        {
            cout << "Ride not found\n";
        }
        Ride &ride = rideIt->second;
        cout << "\nRide Details\n";
        cout << "Ride Id : " << ride.getId() << '\n';
        cout << "Rider   : " << ride.getRiderId() << '\n';
        cout << "Driver  : " << ride.getDriverId() << '\n';
        cout << "Fare    : " << ride.getFare() << "\n\n";
    }
};

int main()
{
    CabBookingSystem sys = CabBookingSystem(
        make_unique<NearestDriverMatchingStrategy>(),
        make_unique<NormalFareStrategy>());

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