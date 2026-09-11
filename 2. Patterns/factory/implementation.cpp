#include <bits/stdc++.h>
using namespace std;

/*
Factory Method( Creational ) Pattern defines an interface for creating an object, but lets subclasses(derived classed like CarFactory) decide which class to instantiate(Carfactory).
In short:
    -Creation logic is separated
    -Client code depends on abstraction, not concrete classes

The real power of this pattern emerges when the Client (the code that uses the vehicle) doesn't know which factory it's getting, see runLogistics function

The main point or goal of implementing this pattern is that all the factory return the same type of vehicle. That is the interface.
*/

class Vehicle // common base so that base class pointer can be used instead concrete class
{
public:
    virtual void drive() = 0;
    virtual ~Vehicle() {}
};

class Car : public Vehicle
{
public:
    void drive() override
    {
        cout << "Driving Car" << "\n";
    }
};

class Bike : public Vehicle
{
public:
    void drive() override
    {
        cout << "Riding Bike" << "\n";
    }
};

// creator (factory interface)
class VehicleFactory // factory interface, so that interface pointer can be used instead concrete factories
{
public:
    virtual Vehicle *createVehicle() = 0; // return vehicle pinter, the creator
    virtual ~VehicleFactory() = default;
};

// concrete factories
class CarFactory : public VehicleFactory
{
public:
    Vehicle *createVehicle() override
    {
        return new Car();
    }
};

class BikeFactory : public VehicleFactory
{
public:
    Vehicle *createVehicle() override
    {
        return new Bike();
    }
};

// This function doesn't care if it's a Car or a Bike!
void runLogistics(VehicleFactory *factory)
{
    Vehicle *v = factory->createVehicle();
    cout << "from independent function: ";
    v->drive();
}

int main()
{
    VehicleFactory *factory = new CarFactory();
    Vehicle *vehicle = factory->createVehicle();
    vehicle->drive();

    factory = new BikeFactory();
    vehicle = factory->createVehicle();
    vehicle->drive();
    // simple idea: a factory interface, and its derived factories
    // the real usage, runLogistic function can run independent of knowing the type of vehicle
    runLogistics(factory); // Now we decide the type in ONE place for the whole app.

    return 0;
}