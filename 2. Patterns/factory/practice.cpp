#include <iostream>
using namespace std;

// interface/abstraction vehicle
class Vehicle
{
public:
    virtual void drive() = 0;
    virtual ~Vehicle() {};
};

class Car : public Vehicle
{
public:
    void drive() override
    {
        cout << "Driving car\n";
    }
};

class Bike : public Vehicle
{
public:
    void drive() override
    {
        cout << "Riding bike\n";
    }
};

class VehicleFactory
{
public:
    virtual Vehicle *createVehicle() = 0;
    virtual ~VehicleFactory() = default;
};

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

void runLogistics(VehicleFactory *factory){
    Vehicle* v= factory->createVehicle();
    cout<<"from independent function: ";
    v->drive();
}


int main()
{
    runLogistics(new CarFactory());
    runLogistics(new BikeFactory());
    return 0;
}