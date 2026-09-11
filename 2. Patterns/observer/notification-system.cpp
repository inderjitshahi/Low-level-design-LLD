#include <bits/stdc++.h>
using namespace std;


class Observer
{
public:
    virtual void notify(const string &message) = 0;
    ~Observer() = default;
};


class Subject
{
public:
    virtual ~Subject() = default;
    virtual void attach(Observer *obs) = 0;
    virtual void detach(Observer *obs) = 0;
    virtual void notifyObserver(const string &msg) = 0;
};

// Instead of duplicating observer list logic in every subject, we create a BaseSubject.
class BaseSubject : public Subject
{
protected: // accessible in derived classes only
    vector<Observer *> observers;

public:
    void attach(Observer *obs) override
    {
        observers.push_back(obs);
    }

    void detach(Observer *obs) override
    {
        observers.erase(remove(observers.begin(), observers.end(), obs), observers.end());
    }

    void notifyObserver(const string &msg)
    {
        for (Observer *obs : observers)
        {
            obs->notify(msg);
        }
    }
};

class OrderService : public BaseSubject
{
public:
    void orderPlace(int orderId)
    {
        notifyObserver("Order placed: Order Id= " + to_string(orderId));
    }

    void orderCancelled(int orderId)
    {
        notifyObserver("Order cancelled: Order Id= " + to_string(orderId));
    }
};

class RideService : public BaseSubject
{
public:
    void rideStarted(int rideId)
    {
        notifyObserver("Ride Started: RideID = " + to_string(rideId));
    }

    void rideCompleted(int rideId)
    {
        notifyObserver("Ride Completed: RideID = " + to_string(rideId));
    }
};

class EmailNotifier : public Observer
{
public:
    void notify(const string &msg) override
    {
        cout << "[EMAIL] " << msg << "\n";
    }
};

class SmsNotifier : public Observer
{
public:
    void notify(const string &msg) override
    {
        cout << "[SMS] " << msg << "\n";
    }
};

class PushNotifier : public Observer
{
public:
    void notify(const string &msg) override
    {
        cout << "[PUSH] " << msg << "\n";
    }
};

int main()
{
    // subjects
    OrderService orderService;
    RideService rideService;

    // Observers
    EmailNotifier email;
    SmsNotifier sms;
    PushNotifier push;

    // subscriptions
    orderService.attach(&email);
    orderService.attach(&sms);

    rideService.attach(&push);
    rideService.attach(&email);

    // Events
    orderService.orderPlace(10);
    rideService.rideStarted(20);

    cout << "---- Unsubscribing Email from Order ----\n";
    orderService.detach(&email);
    orderService.orderCancelled(10);
    rideService.rideCompleted(20);
    return 0;
}