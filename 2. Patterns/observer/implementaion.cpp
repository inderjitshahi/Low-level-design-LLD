#include <bits/stdc++.h>
using namespace std;

/*
Observer Pattern (Behavioural): defines a one-to-many dependency between objects(subject to observers).

Components:
- Subject: maintains state and notifies observers.
- Observer: interface declaring `update()`.
- ConcreteSubject: stores state and manages observers.
- ConcreteObserver: implements `update()` to react to changes.
- Client: wires observers and subjects (e.g., `main`).

*/

class Observer
{
public:
    virtual void update(int newstate) = 0;
    virtual ~Observer() = default;
};

class Subject
{
public:
    virtual ~Subject() {};
    virtual void notify() = 0;
    virtual void attach(Observer *) = 0;
    virtual void detach(Observer *) = 0;
};

class ConcreteSubject : public Subject
{
    int state;
    vector<Observer *> observers; // aggregation relation
public:
    void attach(Observer *obs) override
    {
        observers.push_back(obs);
    }

    void detach(Observer *obs) override
    {
        observers.erase(
            remove(observers.begin(), observers.end(), obs),
            observers.end());
    }

    void notify() override
    {
        for (Observer *obs : observers)
        {
            obs->update(state);
        }
    }

    void setState(int newState)
    {
        state = newState;
        notify();
    }

    int getState()
    {
        return state;
    }
};

class ConcreteObserver : public Observer
{
    string name;

public:
    ConcreteObserver(const string &n) : name(n) {}

    void update(int newState) override
    {
        cout << "Observer " << name
             << " notified. New state = "
             << newState << '\n';
    }
};

int main()
{
    ConcreteSubject subject;

    ConcreteObserver obs1("A");
    ConcreteObserver obs2("B");

    subject.attach(&obs1);
    subject.attach(&obs2);

    subject.setState(10);
    subject.detach(&obs1);
    subject.setState(20);

    return 0;
}